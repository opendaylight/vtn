/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.VBridge.DEFAULT_AGE_INTERVAL;

import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;

import org.opendaylight.controller.md.sal.common.api.clustering.CandidateAlreadyRegisteredException;
import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipCandidateRegistration;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeConfig;

/**
 * {@code VBridgeEntity} describes the entity of the vBridge in a cluster.
 *
 * <p>
 *   This class is used to associate only one MAC address aging task with
 *   a vBridge in a cluster.
 * </p>
 */
public final class VBridgeEntity {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VBridgeEntity.class);

    /**
     * Threshold of aging interval to determine whether a canceled task should
     * be purged from the timer task queue or not.
     *
     * <p>
     *   If the value of {@link #ageInterval} is greater than this value,
     *   an aging timer task will be purged when it is canceled.
     * </p>
     */
    private static final int  AGING_PURGE_THRESHOLD = 3600000;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * The timer thread to run MAC address aging timer task.
     *
     * <p>
     *   {@code null} is set when this entity is destroyed.
     * </p>
     */
    private Timer  agingTimer;

    /**
     * The identifier for the target vBridge.
     */
    private final VBridgeIdentifier  identifier;

    /**
     * The timer task for MAC address aging.
     */
    private AgeTimerTask  agingTask;

    /**
     * The number of milliseconds between MAC address aging.
     */
    private long  ageInterval;

    /**
     * Registration of the entity ownership candidate for the target vBridge.
     */
    private EntityOwnershipCandidateRegistration  entityRegistration;

    /**
     * {@code AgeTxTask} descrbies a task that ages MAC address table entries
     * learned in the target vBridge.
     */
    private final class AgeTxTask extends AbstractTxTask<Void> {
        /**
         * Construct a new instance.
         */
        private AgeTxTask() {
        }

        // AbstractTxTask

        /**
         * Scan MAC addresses learned in the MAC address table associated with
         * the target vBridge.
         *
         * @param ctx  A runtime context for transaction task.
         * @return  {@code null}.
         * @throws VTNException  An error occurred.
         */
        @Override
        protected Void execute(TxContext ctx) throws VTNException {
            new MacEntryAger(ctx).scan(ctx, identifier);
            return null;
        }
    }

    /**
     * {@code AgeTimerTask} describes a timer task that implements aging for
     * MAC addresses learned in the vBridge.
     */
    private final class AgeTimerTask extends TimerTask {
        /**
         * The system time when the latest aging task was scheduled.
         */
        private long  latestTime;

        /**
         * Construct a new instance with specifying the latest execution time.
         *
         * @param time  The system time when the latest aging task was
         *              scheduled.
         */
        private AgeTimerTask(long time) {
            latestTime = time;
        }

        /**
         * Return the system time when the lastet aging task was scheduled.
         *
         * @return  The number of milliseconds from the epoch.
         */
        private long getLatestTime() {
            return latestTime;
        }

        // Runnable

        /**
         * Run the MAC address aging process.
         */
        @Override
        public void run() {
            latestTime = System.currentTimeMillis();
            vtnProvider.post(new AgeTxTask());
        }
    }

    /**
     * Determine the number of milliseconds to be inserted before the first
     * execution of the MAC address aging task.
     *
     * @param age     The current interval between MAC address aging in
     *                milliseconds.
     * @param latest  The system time when the latest aging task was scheduled.
     *                Zero implies the aging task has never been scheduled.
     * @return  The number of milliseconds to be inserted before the first
     *          execution of the MAC address aging task.
     */
    static long getDelay(long age, long latest) {
        long delay;

        if (latest == 0L) {
            // The previous task has never been scheduled.
            delay = age;
        } else {
            // Try to schedule the next aging task when the specified number
            // of milliseconds has passed from the latest execution.
            long next = latest + age;
            delay = Math.max(next - System.currentTimeMillis(), 0L);
        }

        return delay;
    }

    /**
     * Return the value of age-interval in the given vBridge configuration.
     *
     * @param config  The vBridge configuration.
     * @return  The value of age-interval in milliseconds.
     */
    static long getAgeInterval(VtnVbridgeConfig config) {
        Integer age = config.getAgeInterval();
        long sec;
        if (age == null) {
            // This should never happen.
            sec = DEFAULT_AGE_INTERVAL;
        } else {
            sec = age.longValue();
        }

        return TimeUnit.SECONDS.toMillis(sec);
    }

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     * @param ident     The identifier for the target vBridge.
     * @param config    The configuration of the target vBridge.
     */
    public VBridgeEntity(VTNManagerProvider provider,
                         VBridgeIdentifier ident, VtnVbridgeConfig config) {
        vtnProvider = provider;
        agingTimer = vtnProvider.getTimer();
        identifier = ident;
        ageInterval = getAgeInterval(config);
    }

    /**
     * Start the MAC address aging task for the target vBridge.
     */
    public synchronized void startAging() {
        if (agingTimer != null && agingTask == null) {
            startAging(System.currentTimeMillis(), ageInterval);
            LOG.info("{}: MAC address aging timer has been started.",
                     identifier);
        }
    }

    /**
     * Stop the MAC address aging task associated with the target vBridge.
     */
    public synchronized void stopAging() {
        AgeTimerTask task = agingTask;
        if (task != null) {
            agingTask = null;
            cancelAging(task, ageInterval);
            LOG.info("{}: MAC address aging timer has been stopped.",
                     identifier);
        }
    }

    /**
     * Apply new configuration of the vBridge.
     *
     * @param config    The configuration of the target vBridge.
     */
    public synchronized void apply(VtnVbridgeConfig config) {
        if (agingTimer != null) {
            long age = getAgeInterval(config);
            long oldAge = ageInterval;
            if (age != oldAge) {
                // Change the interval between MAC address aging.
                ageInterval = age;

                AgeTimerTask task = agingTask;
                if (task != null) {
                    // Need to restart aging timer task.
                    cancelAging(task, oldAge);
                    long latest = task.getLatestTime();
                    long delay = getDelay(age, latest);
                    startAging(latest, delay);
                    LOG.info("{}: MAC address aging timer has been changed: " +
                             "interval=({} -> {}), delay={}",
                             identifier, oldAge, age, delay);
                }
            }
        }
    }

    /**
     * Register a candidate for ownership of this vBridge entity.
     *
     * @param ent  The entity that specifies the target vBridge.
     * @return  {@code true} on success. {@code false} on failure.
     */
    public boolean register(Entity ent) {
        boolean result = true;
        try {
            entityRegistration = vtnProvider.registerEntity(ent);
            LOG.debug("{}: Entity has been registered.", identifier);
        } catch (CandidateAlreadyRegisteredException | RuntimeException e) {
            String msg = identifier.toString() +
                ": Unable to register entity: " + e;
            LOG.error(msg, e);
            result = false;
        }

        return result;
    }

    /**
     * Destroy this vBridge entity.
     */
    public void destroy() {
        synchronized (this) {
            // Stop MAC address aging timer.
            stopAging();

            // Clear the timer thead for MAC address aging.
            agingTimer = null;
        }

        // Clear the registration of the entity.
        EntityOwnershipCandidateRegistration reg = entityRegistration;
        if (reg != null) {
            entityRegistration = null;
            reg.close();
            LOG.debug("{}: Entity has been unregistered.", identifier);
        }
    }

    /**
     * Start a new MAC address aging task.
     *
     * @param latest  The system time when the latest aging task was scheduled.
     * @param delay   The number of milliseconds to be inserted before the
     *                first execution of the task.
     */
    private synchronized void startAging(long latest, long delay) {
        AgeTimerTask task = new AgeTimerTask(latest);
        agingTask = task;
        agingTimer.scheduleAtFixedRate(task, delay, ageInterval);
    }

    /**
     * Cancel the current MAC address aging timer task.
     *
     * @param task  The aging timer task to be canceled.
     * @param age   The interval between MAC address table aging.
     */
    private synchronized void cancelAging(AgeTimerTask task, long age) {
        task.cancel();
        if (age > AGING_PURGE_THRESHOLD) {
            agingTimer.purge();
        }
    }
}
