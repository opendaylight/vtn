/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.TimerTask;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;

import org.opendaylight.controller.forwardingrulesmanager.
    IForwardingRulesManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class represents an abstract task to be executed on VTN flow programmer
 * thread.
 *
 * @see VTNFlowProgrammer
 */
public abstract class FlowModTask implements Runnable {
    /**
     * VTN Manager service.
     */
    protected final VTNManagerImpl  vtnManager;

    /**
     * Result of this task.
     */
    private FlowModResult  taskResult;

    /**
     * Construct a new task.
     *
     * @param mgr  VTN Manager service.
     */
    protected FlowModTask(VTNManagerImpl mgr) {
        vtnManager = mgr;
    }

    /**
     * Wait for completion of this task.
     *
     * @param timeout  The maximum time to wait in milliseconds.
     *                 Note that zero must not be passed.
     * @return  {@link FlowModResult} which indicates the result of this task
     *          is returned.
     */
    FlowModResult getResult(long timeout) {
        return getResultAbs(System.currentTimeMillis() + timeout);
    }

    /**
     * Wait for completion of this task.
     *
     * @param limit  System absolute time in milliseconds which represents
     *               the deadline of this task.
     * @return  {@link FlowModResult} which indicates the result of this task
     *          is returned.
     */
    synchronized FlowModResult getResultAbs(long limit) {
        if (taskResult == null) {
            do {
                long timeout = limit - System.currentTimeMillis();
                if (timeout <= 0) {
                    Logger logger = getLogger();
                    logger.error("{}: {} timed out",
                                 vtnManager.getContainerName(),
                                 getClass().getSimpleName());
                    taskResult = FlowModResult.TIMEDOUT;
                    break;
                }

                try {
                    wait(timeout);
                } catch (InterruptedException e) {
                    Logger logger = getLogger();
                    logger.error(vtnManager.getContainerName() +
                                 ": getResultAbs: Interrupted", e);
                    taskResult = FlowModResult.INTERRUPTED;
                    break;
                }
            } while (taskResult == null);
        }

        return taskResult;
    }

    /**
     * Wait for this task to complete with specifying infinite timeout.
     *
     * @return  {@link FlowModResult} which indicates the result of this task
     *          is returned.
     */
    synchronized FlowModResult waitFor() {
        while (taskResult == null) {
            try {
                wait();
            } catch (InterruptedException e) {
                Logger logger = getLogger();
                logger.error(vtnManager.getContainerName() +
                             ": waitFor: Interrupted", e);
                taskResult = FlowModResult.INTERRUPTED;
                break;
            }
        }
        return taskResult;
    }

    /**
     * Set result of this task.
     *
     * @param result  Result of this task.
     */
    protected synchronized void setResult(boolean result) {
        if (taskResult == null) {
            taskResult = (result)
                ? FlowModResult.SUCCEEDED : FlowModResult.FAILED;
            notifyAll();
        }
    }

    /**
     * Install a single flow entry to local node.
     *
     * @param fent  A flow entry to be installed.
     * @return  {@code true} is returned if the given flow entry was installed
     *          successfully.
     */
    protected boolean installLocal(FlowEntry fent) {
        IForwardingRulesManager frm = vtnManager.getForwardingRuleManager();
        return installLocal(frm, fent);
    }

    /**
     * Install a single flow entry to local node.
     *
     * @param frm  Forwarding rule manager service.
     * @param fent  A flow entry to be installed.
     * @return  {@code true} is returned if the given flow entry was installed
     *          successfully.
     */
    protected boolean installLocal(IForwardingRulesManager frm,
                                   FlowEntry fent) {
        Status status;
        TimerTask alarm = vtnManager.setFlowModAlarm();
        try {
            status = frm.installFlowEntry(fent);
        } finally {
            vtnManager.cancelAlarm(alarm);
        }

        Logger logger = getLogger();
        if (!status.isSuccess()) {
            if (status.getCode() == StatusCode.UNDEFINED) {
                logger.error("{}: Failed to install flow entry: Timed Out:" +
                             "entry={}", vtnManager.getContainerName(),
                             fent);
            } else {
                logger.error("{}: Failed to install flow entry: " +
                             "status={}, entry={}",
                             vtnManager.getContainerName(), status, fent);
            }
            return false;
        }

        if (logger.isTraceEnabled()) {
            logger.trace("{}: Installed flow entry: entry={}",
                         vtnManager.getContainerName(), fent);
        }
        return true;
    }

    /**
     * Uninstall a single flow entry from local node.
     *
     * @param fent  A flow entry to be uninstalled.
     * @return  {@code true} is returned if the given flow entry was
     *          uninstalled successfully.
     */
    protected boolean uninstallLocal(FlowEntry fent) {
        IForwardingRulesManager frm = vtnManager.getForwardingRuleManager();
        return uninstallLocal(frm, fent);
    }

    /**
     * Uninstall a single flow entry from local node.
     *
     * @param frm   Forwarding rule manager service.
     * @param fent  A flow entry to be uninstalled.
     * @return  {@code true} is returned if the given flow entry was
     *          uninstalled successfully.
     */
    protected boolean uninstallLocal(IForwardingRulesManager frm,
                                     FlowEntry fent) {
        Status status;
        TimerTask alarm = vtnManager.setFlowModAlarm();
        try {
            status = frm.uninstallFlowEntry(fent);
        } finally {
            vtnManager.cancelAlarm(alarm);
        }

        Logger logger = getLogger();
        if (!status.isSuccess()) {
            if (status.getCode() == StatusCode.UNDEFINED) {
                logger.error("{}: Failed to uninstall flow entry: " +
                             "Timed Out: entry={}",
                             vtnManager.getContainerName(), status, fent);
            } else {
                logger.error("{}: Failed to uninstall flow entry: " +
                             "status={}, entry={}",
                             vtnManager.getContainerName(), status, fent);
            }
            return false;
        }

        if (logger.isTraceEnabled()) {
            logger.trace("{}: Uninstalled flow entry: entry={}",
                         vtnManager.getContainerName(), fent);
        }
        return true;
    }

    /**
     * Execute this task.
     */
    @Override
    public final void run() {
        try {
            setResult(execute());
        } finally {
            // Make this task fail if the task result is not set.
            setResult(false);
        }
    }

    /**
     * Execute this task.
     *
     * @return  {@code true} is returned if this task completed successfully.
     *          Otherwise {@code false} is returned.
     */
    protected abstract boolean execute();

    /**
     * Return a logger object for this class.
     *
     * @return  A logger object.
     */
    protected abstract Logger getLogger();
}
