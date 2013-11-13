/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Iterator;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.FlowRemoveEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.sal.connection.ConnectionLocality;

/**
 * This class implements flow programming task which uninstalls VTN flows.
 */
public class FlowRemoveTask extends RemoteFlowModTask {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(FlowRemoveTask.class);

    /**
     * Divisor used to derive ingress flow remove timeout.
     */
    private static final long  INGRESS_TIMEOUT_DIVISOR = 4;

    /**
     * Set of VTN flow group identifier to be removed.
     */
    private final Set<FlowGroupId>  groupSet;

    /**
     * A list of ingress flow entries.
     */
    private final List<FlowEntry>  ingressFlows;

    /**
     * A list of flow entries except for ingress flows.
     */
    private final List<FlowEntry>  flowEntries;

    /**
     * The maximum number of milliseconds to wait for completion of this task.
     */
    private long  taskTimeout;

    /**
     * Construct a new flow task to uninstall flow entries in a VTN flow.
     *
     * @param mgr      VTN Manager service.
     * @param gid      Identifier of the VTN flow.
     * @param ingress  A ingress flow of a VTN flow.
     *                 {@code null} can be specified if there is no need to
     *                 uninstall ingress flow.
     * @param it       A list iterator which contains flow entries except for
     *                 ingress flow.
     */
    FlowRemoveTask(VTNManagerImpl mgr, FlowGroupId gid, FlowEntry ingress,
                   Iterator<FlowEntry> it) {
        super(mgr);

        groupSet = new HashSet<FlowGroupId>();
        groupSet.add(gid);

        ingressFlows = new ArrayList<FlowEntry>();
        if (ingress != null) {
            ingressFlows.add(ingress);
        }

        flowEntries = new ArrayList<FlowEntry>();
        while (it.hasNext()) {
            FlowEntry fent = it.next();
            flowEntries.add(fent);
        }

        VTNConfig vc = mgr.getVTNConfig();
        taskTimeout = (long)vc.getRemoteFlowModTimeout();
    }

    /**
     * Construct a new flow task to uninstall bulk flow entries.
     *
     * @param mgr      VTN Manager service.
     * @param gidset   A set of VTN flow identifiers to be removed.
     * @param ingress  A list of ingress flows.
     * @param entries  A list of flow entries except for ingress flow.
     */
    FlowRemoveTask(VTNManagerImpl mgr, Set<FlowGroupId> gidset,
                   List<FlowEntry> ingress, List<FlowEntry> entries) {
        super(mgr);
        groupSet = new HashSet<FlowGroupId>(gidset);
        ingressFlows = new ArrayList<FlowEntry>(ingress);
        flowEntries = new ArrayList<FlowEntry>(entries);
        VTNConfig vc = mgr.getVTNConfig();
        taskTimeout = (long)vc.getRemoteBulkFlowModTimeout();
    }

    /**
     * Wait for completion of this task.
     *
     * @return  {@link FlowModResult} which indicates the result of this task
     *          is returned.
     */
    FlowModResult getResult() {
        long abs = System.currentTimeMillis() + taskTimeout;
        FlowModResult ret = getResultAbs(abs);
        if (ret == FlowModResult.SUCCEEDED) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: VTN flow remove task has completed: {}",
                          getVTNManager().getContainerName(), groupSet);
            }
        } else {
            LOG.error("{}: Failed to remove VTN flows: result={}, group={}",
                      getVTNManager().getContainerName(), ret, groupSet);
        }

        return ret;
    }

    /**
     * Execute flow uninstallation task.
     *
     * @return  {@code true} is returned if this task completed successfully.
     *          Otherwise {@code false} is returned.
     */
    @Override
    protected boolean execute() {
        // Remove VTN flows from the cluster cache.
        VTNManagerImpl mgr = getVTNManager();
        ConcurrentMap<FlowGroupId, VTNFlow> db = mgr.getFlowDB();
        for (FlowGroupId gid: groupSet) {
            try {
                db.remove(gid);
            } catch (Exception e) {
                LOG.error(mgr.getContainerName() +
                          ": Failed to remove VTN flow from the cache: " +
                          gid, e);
            }
        }

        // Unintall ingress flows.
        long cur = System.currentTimeMillis();
        long limit = cur + taskTimeout;
        long inLimit = cur + (taskTimeout / INGRESS_TIMEOUT_DIVISOR);
        boolean ret = uninstall(ingressFlows, inLimit);

        // Uninstall rest of flow entries.
        if (!uninstall(flowEntries, limit)) {
            ret = false;
        }

        if (LOG.isDebugEnabled() && ret) {
            LOG.debug("{}: Flow entries have been removed",
                      mgr.getContainerName());
        }

        return ret;
    }

    /**
     * Return a logger object for this class.
     *
     * @return  A logger object.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Initiate request for flow modification to remote cluster nodes.
     *
     * @param entries  A list of flow entries to be modified.
     */
    @Override
    protected void sendRequest(List<FlowEntry> entries) {
        // Send flow remove event.
        postEvent(new FlowRemoveEvent(entries));
    }

    /**
     * Uninstall the given flow entries.
     *
     * @param entries  A list of flow entries to be uninstalled.
     * @param limit    System absolute time in milliseconds which represents
     *                 deadline of the wait.
     * @return  {@code true} is returned if all of the given flow entries
     *          were successfully uninstalled.
     *          Otherwise {@code false} is returned.
     */
    private boolean uninstall(List<FlowEntry> entries, long limit) {
        ArrayList<FlowEntry> remote = new ArrayList<FlowEntry>();
        List<LocalFlowRemoveTask> local = uninstall(entries, remote);

        boolean ret = true;
        // Direct remote cluster nodes to uninstall flow entries.
        if (!remote.isEmpty() && !modifyRemoteFlow(remote, true, limit)) {
            ret = false;
        }

        // Ensure that all local flows were successfully uninstalled.
        for (LocalFlowRemoveTask task: local) {
            if (task.waitFor() != FlowModResult.SUCCEEDED) {
                ret = false;
            }
        }

        return ret;
    }

    /**
     * Uninstall all flow entries in the specified list from local node.
     *
     * <p>
     *   This method tries to uninstall local flows in background.
     * </p>
     *
     * @param entries  A list of flow entries to be uninstalled.
     * @param remote   Flow entry list to set remote flows.
     *                 This method appends all flow entries to be uninstalled
     *                 by remote cluster node to this list.
     * @return  A list of {@link LocalFlowRemoveTask} that uninstalls local
     *          flows is returned.
     */
    private List<LocalFlowRemoveTask> uninstall(List<FlowEntry> entries,
                                                List<FlowEntry> remote) {
        VTNManagerImpl mgr = getVTNManager();
        IConnectionManager cnm = mgr.getConnectionManager();
        ArrayList<LocalFlowRemoveTask> local =
            new ArrayList<LocalFlowRemoveTask>();
        for (FlowEntry fent: entries) {
            ConnectionLocality cl = cnm.getLocalityStatus(fent.getNode());
            if (cl == ConnectionLocality.LOCAL) {
                LocalFlowRemoveTask task =
                    new LocalFlowRemoveTask(mgr, fent);
                local.add(task);
                mgr.postAsync(task);
            } else if (cl == ConnectionLocality.NOT_LOCAL) {
                remote.add(fent);
            } else {
                LOG.warn("{}: Target node of flow entry is not connected: {}",
                         mgr.getConnectionManager(), fent);
            }
        }

        return local;
    }
}
