/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;
import java.util.HashSet;
import java.util.HashMap;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;

/**
 * {@code RemoteFlowRequest} class maintains the state of request for
 *  modification of flow entries controlled by remote cluster nodes.
 */
public class RemoteFlowRequest {
    /**
     * A set of flow entries being modified by remote cluster node.
     */
    private final HashSet<String>  requestSet = new HashSet<String>();

    /**
     * A set of flow entries successfully modified.
     */
    private final HashSet<String>  succeededSet = new HashSet<String>();

    /**
     * A set of flow entries failed to modify.
     */
    private final HashSet<String>  failedSet = new HashSet<String>();

    /**
     * A set of flow entries that handled by no one.
     */
    private final HashSet<String> orphanSet = new HashSet<String>();

    /**
     * This map keeps the number of remote cluster nodes that ignores
     * flow modification request.
     */
    private HashMap<String, Integer> ignoredFlows =
        new HashMap<String, Integer>();

    /**
     * Result of this request.
     */
    private FlowModResult  requestResult;

    /**
     * Construct a new remote flow request.
     *
     * @param entries  A list of flow entries.
     */
    RemoteFlowRequest(List<FlowEntry> entries) {
        for (FlowEntry fent: entries) {
            requestSet.add(fent.getFlowName());
        }
    }

    /**
     * Called when a remote cluster node reports a result of flow modification.
     *
     * @param name         The name of flow entry.
     * @param result       Result of flow modification.
     *                     {@code true} means successful completion.
     * @param remoteNodes  The number of remote cluster nodes.
     * @return  {@code true} is returned only if there is no more request
     *          to wait.
     */
    synchronized boolean setResult(String name, FlowModResult result,
                                   int remoteNodes) {
        if (result == FlowModResult.IGNORED) {
            handleIgnored(name, remoteNodes);
        } else if (requestSet.remove(name)) {
            if (result == FlowModResult.SUCCEEDED) {
                succeededSet.add(name);
            } else {
                failedSet.add(name);
                notifyAll();
            }
        }

        boolean ret = requestSet.isEmpty();
        if (ret) {
            notifyAll();
        }

        return ret;
    }

    /**
     * Wait for completion of flow modification on remote cluster node.
     *
     * @param limit    System absolute time in milliseconds which represents
     *                 deadline of the wait.
     * @param all      If {@code true} is passed, this method waits for all
     *                 requests of flow entry modification to complete.
     *                 If {@code false} is passed, this method returns
     *                 immediately when at least one modification failure
     *                 is detected.
     * @return  {@code true} is returned if all flow entries were modified
     *          successfully. Otherwise {@code false} is returned.
     */
    synchronized boolean getResultAbs(long limit, boolean all) {
        if (requestResult == null) {
            boolean interrupted = false;
            do {
                long timeout = limit - System.currentTimeMillis();
                if (timeout <= 0) {
                    requestResult = FlowModResult.TIMEDOUT;
                    return false;
                }

                try {
                    wait(timeout);
                } catch (InterruptedException e) {
                    interrupted = true;
                    break;
                }
                if (!all && (!failedSet.isEmpty() || !orphanSet.isEmpty())) {
                    requestResult = FlowModResult.FAILED;
                    return false;
                }
            } while (!requestSet.isEmpty());

            if (requestSet.isEmpty() && failedSet.isEmpty() &&
                orphanSet.isEmpty()) {
                requestResult = FlowModResult.SUCCEEDED;
            } else if (interrupted) {
                requestResult = FlowModResult.INTERRUPTED;
            } else {
                requestResult = FlowModResult.FAILED;
            }
        }

        return (requestResult == FlowModResult.SUCCEEDED);
    }

    /**
     * Record a log message that remote flow modification request was
     * failed.
     *
     * @param logger  A logger instance.
     */
    void logError(Logger logger) {
        // Monitor does not need to be acquired because this method is
        // called after this request is removed from VTN Manager.
        logger.error("Request for remote flow modification failed: " +
                     "result={}, succeeded={}, failed={}, orphan={}, " +
                     "incomplete={}, ignored={}", requestResult,
                     succeededSet, failedSet, orphanSet, requestSet,
                     ignoredFlows);
    }

    /**
     * Handle flow modification result which means that the flow modification
     * request was ignored by remote cluster node.
     *
     * @param name         The name of flow entry.
     * @param remoteNodes  The number of remote cluster nodes.
     */
    private synchronized void handleIgnored(String name, int remoteNodes) {
        if (!requestSet.contains(name)) {
            return;
        }

        Integer count = ignoredFlows.get(name);
        int c;
        if (count == null) {
            c = 1;
        } else {
            c = count.intValue() + 1;
        }

        if (c < remoteNodes) {
            ignoredFlows.put(name, new Integer(c));
        } else {
            // The specified flow entry was ignored by all remote cluster
            // nodes. So no one can modify the flow.
            ignoredFlows.remove(name);
            orphanSet.add(name);
            requestSet.remove(name);
            notifyAll();
        }
    }
}
