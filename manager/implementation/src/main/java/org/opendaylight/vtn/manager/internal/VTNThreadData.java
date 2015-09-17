/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgeHostFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgeNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgePortFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.TenantFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.VNodeFlowRemover;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code VTNThreadData} class describes a thread-local variable used for
 * the VTN Manager.
 *
 * <p>
 *   This class is designed to be used by a single thread.
 * </p>
 */
public final class VTNThreadData {
    /**
     * Thread-local variables used for the VTN Manager.
     */
    private static final ThreadLocal<VTNThreadData>  THREAD_LOCAL =
        new ThreadLocal<VTNThreadData>();

    /**
     * A lock object held by the thread.
     */
    private final Lock  theLock;

    /**
     * A list of futures associated with ongoing tasks.
     */
    private List<VTNFuture<?>>  futureList;

    /**
     * Create a thread-local variable for the calling thread.
     *
     * <p>
     *   This method acquires the specified lock object. It will be released
     *   by the calling of {@link #cleanUp(VTNManagerImpl)}.
     * </p>
     *
     * @param lock  A lock object to be held.
     * @return  A {@code VTNThreadData} object bound to the calling thread.
     */
    static VTNThreadData create(Lock lock) {
        VTNThreadData data = new VTNThreadData(lock);
        boolean succeeded = false;
        THREAD_LOCAL.set(data);
        try {
            lock.lock();
            succeeded = true;
            return data;
        } finally {
            if (!succeeded) {
                THREAD_LOCAL.remove();
            }
        }
    }

    /**
     * Remove all VTN flows determined by the given flow remover.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param provider  VTN Manager provider service.
     * @param remover   A {@link FlowRemover} instance.
     */
    public static void removeFlows(VTNManagerProvider provider,
                                   FlowRemover remover) {
        if (provider != null) {
            addTask(provider.removeFlows(remover));
        }
    }

    /**
     * Remove all VTN flows determined by the given flow remover.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param remover  A {@link FlowRemover} instance.
     */
    public static void removeFlows(VTNManagerImpl mgr, FlowRemover remover) {
        removeFlows(mgr.getVTNProvider(), remover);
    }

    /**
     * Remove all VTN flows which depend on the given virtual node.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the virtual node.
     * @see VNodeFlowRemover
     */
    public static void removeFlows(VTNManagerImpl mgr, VTenantPath path) {
        removeFlows(mgr, new VNodeFlowRemover(path));
    }

    /**
     * Remove all VTN flows related to the given edge network.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param tname   The name of the virtual tenant.
     * @param node    A {@link Node} instance corresponding to the target
     *                switch.
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @param vlan        A VLAN ID.
     * @see EdgeNodeFlowRemover
     */
    public static void removeFlows(VTNManagerImpl mgr, String tname,
                                   Node node, PortFilter filter, short vlan) {
        SalNode snode = SalNode.create(node);
        removeFlows(mgr,
                    new EdgeNodeFlowRemover(tname, snode, filter, (int)vlan));
    }

    /**
     * Remove all VTN flows related to the given edge network.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param tname  The name of the virtual tenant.
     * @param port   A node connector associated with a switch port.
     * @param vlan   A VLAN ID.
     * @see EdgePortFlowRemover
     */
    public static void removeFlows(VTNManagerImpl mgr, String tname,
                                   NodeConnector port, short vlan) {
        SalPort sport = SalPort.create(port);
        removeFlows(mgr, new EdgePortFlowRemover(tname, sport, (int)vlan));
    }

    /**
     * Remove all VTN flows related to the given layer 2 host entry.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param tname  The name of the virtual tenant.
     * @param mvlan  A pair of MAC address and VLAN ID.
     * @param port   A node connector associated with a switch port.
     * @see EdgeHostFlowRemover
     */
    public static void removeFlows(VTNManagerImpl mgr, String tname,
                                   MacVlan mvlan, NodeConnector port) {
        SalPort sport = SalPort.create(port);
        removeFlows(mgr, new EdgeHostFlowRemover(tname, mvlan, sport));
    }

    /**
     * Remove all VTN flows in the virtual tenant.
     *
     * <p>
     *   Flow removing will be executed in background.
     *   If a {@code VTNThreadData} is bound to the calling thread,
     *   the calling thread will wait for completion of flow removing
     *   when {@link #cleanUp(VTNManagerImpl)} is called.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param tname  The name of the target VTN.
     */
    public static void removeFlows(VTNManagerImpl mgr, String tname) {
        removeFlows(mgr, new TenantFlowRemover(tname));
    }

    /**
     * Add the specified future to the task list to wait.
     *
     * @param future  A future associated with the task to wait.
     */
    private static void addTask(VTNFuture<?> future) {
        if (future != null) {
            VTNThreadData data = THREAD_LOCAL.get();
            if (data != null) {
                List<VTNFuture<?>> list = data.futureList;
                if (list == null) {
                    list = new ArrayList<>();
                    data.futureList = list;
                }
                list.add(future);
            }
        }
    }

    /**
     * Construct a new context.
     *
     * @param lock  A lock object held by the calling thread.
     */
    private VTNThreadData(Lock lock) {
        theLock = lock;
    }

    /**
     * Clean up this thread-local variable.
     *
     * @param mgr   VTN Manager service.
     */
    void cleanUp(VTNManagerImpl mgr) {
        try {
            // Unlock the lock, and flush pending events.
            mgr.unlock(theLock);
        } finally {
            // Unbind this object from the calling thread.
            THREAD_LOCAL.remove();
        }

        if (futureList != null) {
            // Wait for completion of background tasks.
            for (VTNFuture<?> future: futureList) {
                try {
                    future.checkedGet();
                } catch (VTNException e) {
                    // Ignore any error.
                }
            }
        }
    }
}
