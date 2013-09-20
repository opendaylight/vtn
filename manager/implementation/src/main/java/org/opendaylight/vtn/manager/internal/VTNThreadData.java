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
import java.util.ArrayList;
import java.util.concurrent.locks.Lock;

import org.opendaylight.vtn.manager.VTenantPath;

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
     * Ongoing flow remove tasks to wait for completion.
     */
    private List<FlowRemoveTask>  flowRemoveTaskList;

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
        THREAD_LOCAL.set(data);
        try {
            lock.lock();
            lock = null;
            return data;
        } finally {
            if (lock != null) {
                THREAD_LOCAL.remove();
            }
        }
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
     */
    public static void removeFlows(VTNManagerImpl mgr, VTenantPath path) {
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(path.getTenantName());
        if (fdb != null) {
            addTask(fdb.removeFlows(mgr, path));
        }
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
     * @param mgr  VTN Manager service.
     * @param fdb  VTN flow database object associated with the virtual tenant.
     */
    public static void removeFlows(VTNManagerImpl mgr, VTNFlowDatabase fdb) {
        addTask(fdb.clear(mgr));
    }


    /**
     * Add the specified flow remove task to the task list to wait.
     *
     * @param task  A flow remove task to wait.
     */
    private static void addTask(FlowRemoveTask task) {
        if (task != null) {
            VTNThreadData data = THREAD_LOCAL.get();
            if (data != null) {
                List<FlowRemoveTask> list = data.flowRemoveTaskList;
                if (list == null) {
                    list = new ArrayList<FlowRemoveTask>();
                    data.flowRemoveTaskList = list;
                }
                list.add(task);
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

        if (flowRemoveTaskList != null) {
            // Wait for completion of background tasks.
            for (FlowRemoveTask task: flowRemoveTaskList) {
                task.getResult();
            }
        }
    }
}
