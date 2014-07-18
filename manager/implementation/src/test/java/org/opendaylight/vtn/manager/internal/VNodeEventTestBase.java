/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.List;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock.ReadLock;

import org.apache.felix.dm.impl.ComponentImpl;

import org.junit.Before;

import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;

/**
 * Base class for test of classes that inherit
 * {@link org.opendaylight.vtn.manager.internal.cluster.VNodeEvent}.
 */
public class VNodeEventTestBase extends TestUseVTNManagerBase {

    /**
     * Construct a new instance.
     */
    public VNodeEventTestBase() {
        super(3);
    }

    @Before
    @Override
    public void before() {
        setupStartupDir();

        vtnMgr = new VTNManagerImpl();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStubCluster(stubMode);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        resMgr.setClusterGlobalService(stubObj);
        resMgr.init(c);
        vtnMgr.setResourceManager(resMgr);
        vtnMgr.setClusterContainerService(stubObj);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        vtnMgr.setDataPacketService(stubObj);
        vtnMgr.setRouting(stubObj);
        vtnMgr.setHostTracker(stubObj);
        vtnMgr.setForwardingRuleManager(stubObj);
        vtnMgr.setConnectionManager(stubObj);
        vtnMgr.setContainerManager(stubObj);
        startVTNManager(c);
    }

    /**
     * Get posted {@link ClusterEvent} toward cluster event cache.
     *
     * @return A list of {@link ClusterEvent}.
     */
    public List<ClusterEvent> getClusterEvent() {
        IClusterContainerServices cs = vtnMgr.getClusterContainerService();
        ConcurrentMap<ClusterEventId, ClusterEvent> clsEvents =
            (ConcurrentMap<ClusterEventId, ClusterEvent>)cs
            .getCache(VTNManagerImpl.CACHE_EVENT);
        if (clsEvents == null || clsEvents.size() == 0) {
            return null;
        }
        return new ArrayList<ClusterEvent>(clsEvents.values());
    }

    /**
     * Clear {@link ClusterEvent} in cluster event cache.
     */
    public void clearClusterEvent() {
        IClusterContainerServices cs = vtnMgr.getClusterContainerService();
        ConcurrentMap<ClusterEventId, ClusterEvent> clsEvents =
            (ConcurrentMap<ClusterEventId, ClusterEvent>)cs
            .getCache(VTNManagerImpl.CACHE_EVENT);
        if (clsEvents != null) {
            clsEvents.clear();
        }
    }

    /**
     * Flush enqueued event in {@code clusterEventQueue}.
     */
    public void forceFlushEventQueue() {
        ReentrantReadWriteLock rwlock = new ReentrantReadWriteLock();
        ReadLock lock = rwlock.readLock();
        lock.lock();
        vtnMgr.unlock(lock, false);
    }
}
