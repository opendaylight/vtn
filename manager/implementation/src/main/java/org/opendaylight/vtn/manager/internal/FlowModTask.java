/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.TimerTask;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.util.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.SalNode;
import org.opendaylight.vtn.manager.internal.util.concurrent.AbstractVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.TimeoutCounter;

import org.opendaylight.controller.forwardingrulesmanager.
    IForwardingRulesManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class represents an abstract task to be executed on VTN flow programmer
 * thread.
 *
 * @see VTNFlowDatabase
 */
public abstract class FlowModTask extends AbstractVTNFuture<FlowModResult>
    implements Runnable {
    /**
     * VTN Manager service.
     */
    private final VTNManagerImpl  vtnManager;

    /**
     * A MD-SAL datastore transaction context.
     */
    private final TxContext  txContext;

    /**
     * Result of this task.
     */
    private FlowModResult  taskResult;

    /**
     * Construct a new task.
     *
     * @param mgr  VTN Manager service.
     * @param ctx  MD-SAL datastore transaction context.
     */
    protected FlowModTask(VTNManagerImpl mgr, TxContext ctx) {
        vtnManager = mgr;
        txContext = ctx;
    }

    /**
     * Return VTN Manager service.
     *
     * @return  VTN Manager service.
     */
    protected VTNManagerImpl getVTNManager() {
        return vtnManager;
    }

    /**
     * Return MD-SAL datastore transaction context.
     *
     * @return  A {@link TxContext} instance.
     */
    protected TxContext getTxContext() {
        return txContext;
    }

    /**
     * Determine whether the given node is present or not.
     *
     * @param node  A {@link Node} instance.
     * @return  {@code true} if present. Otherwise {@code false}.
     */
    protected boolean exists(Node node) {
        InventoryReader reader = txContext.getInventoryReader();
        try {
            return reader.exists(SalNode.create(node));
        } catch (Exception e) {
            String msg = MiscUtils.join(
                vtnManager.getContainerName(),
                "Failed to read node information", node);
            getLogger().error(msg, e);
        }

        return false;
    }

    /**
     * Post a cluster event.
     *
     * @param cev  A cluster event.
     */
    protected void postEvent(ClusterEvent cev) {
        vtnManager.postEvent(cev);
    }

    /**
     * Wait for completion of this task.
     *
     * @param timeout  The maximum time to wait in milliseconds.
     *                 Note that zero must not be passed.
     * @return  {@link FlowModResult} which indicates the result of this task
     *          is returned.
     */
    synchronized FlowModResult getResult(long timeout) {
        try {
            return get(timeout, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            Logger logger = getLogger();
            logger.error(vtnManager.getContainerName() +
                         ": getResult: Interrupted", e);
            setTaskResult(FlowModResult.INTERRUPTED);
        } catch (TimeoutException e) {
            Logger logger = getLogger();
            logger.error("{}: {} timed out", vtnManager.getContainerName(),
                         getClass().getSimpleName());
            setTaskResult(FlowModResult.TIMEDOUT);
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
        try {
            return get();
        } catch (InterruptedException e) {
            Logger logger = getLogger();
            logger.error(vtnManager.getContainerName() +
                         ": waitFor: Interrupted", e);
            setTaskResult(FlowModResult.INTERRUPTED);
        }
        return taskResult;
    }

    /**
     * Set result of this task.
     *
     * @param result  Result of this task.
     */
    protected void setResult(boolean result) {
        FlowModResult res = (result)
            ? FlowModResult.SUCCEEDED : FlowModResult.FAILED;
        setTaskResult(res);
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
            Node node = fent.getNode();
            if (exists(node)) {
                if (status.getCode() == StatusCode.UNDEFINED) {
                    logger.error("{}: Failed to install flow entry: " +
                                 "Timed Out: entry={}",
                                 vtnManager.getContainerName(), fent);
                } else {
                    logger.error("{}: Failed to install flow entry: " +
                                 "status={}, entry={}",
                                 vtnManager.getContainerName(), status, fent);
                }
            } else if (logger.isTraceEnabled()) {
                logger.error("{}: Failed to install flow entry: No node: " +
                             "entry={}", vtnManager.getContainerName(), fent);
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
            Node node = fent.getNode();
            if (exists(node)) {
                if (status.getCode() == StatusCode.UNDEFINED) {
                    logger.error("{}: Failed to uninstall flow entry: " +
                                 "Timed Out: entry={}",
                                 vtnManager.getContainerName(), fent);
                } else {
                    logger.error("{}: Failed to uninstall flow entry: " +
                                 "status={}, entry={}",
                                 vtnManager.getContainerName(), status, fent);
                }
            } else if (logger.isTraceEnabled()) {
                logger.trace("{}: Failed to uninstall flow entry: No node: " +
                             "entry={}", vtnManager.getContainerName(), fent);
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
     * Set the result of this task.
     *
     * @param result  The result of this task.
     */
    private void setTaskResult(FlowModResult result) {
        synchronized (this) {
            if (taskResult == null) {
                taskResult = result;
                notifyAll();
            }
        }
        done();
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
            txContext.cancelTransaction();
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

    // Remarks: Implement VTNFuture tentatively.

    // Future

    /**
     * Attempt to cancel execution of the task.
     *
     * @param intr  {@code true} if the task runner thread should be
     *              interrupted. Otherwise in-progress task are allowed
     *              to complete.
     * @return  {@code true} only if the task was canceled.
     */
    @Override
    public boolean cancel(boolean intr) {
        // Cancellation is not supported.
        return false;
    }

    /**
     * Determine whether the task was canceled before it completed normally.
     *
     * @return  {@code true} only if the task was canceled.
     */
    @Override
    public boolean isCancelled() {
        // Cancellation is not supported.
        return false;
    }

    /**
     * Determine whether the task completed or not.
     *
     * @return  {@code true} only if the task completed.
     */
    @Override
    public synchronized boolean isDone() {
        return (taskResult != null);
    }

    /**
     * Wait for the task to complete, and then return the result of the task.
     *
     * @return  The result of the task.
     * @throws InterruptedException
     *    The current thread was interrupted while waiting.
     */
    @Override
    public synchronized FlowModResult get() throws InterruptedException {
        while (taskResult == null) {
            wait();
        }
        return taskResult;
    }

    /**
     * Wait for the task to complete, and then return the result of the task.
     *
     * @param timeout  The maximum time to wait
     * @param unit     The time unit of {@code timeout}.
     * @return  The result of the task.
     * @throws InterruptedException
     *    The current thread was interrupted while waiting.
     * @throws TimeoutException
     *    The task did not complete within the given timeout.
     */
    @Override
    public synchronized FlowModResult get(long timeout, TimeUnit unit)
        throws InterruptedException, TimeoutException {
        TimeoutCounter tc = TimeoutCounter.newTimeout(timeout, unit);
        while (taskResult == null) {
            tc.await(this);
        }

        return taskResult;
    }
}
