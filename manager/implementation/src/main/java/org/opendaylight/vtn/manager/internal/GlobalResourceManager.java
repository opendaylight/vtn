/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Set;
import java.util.EnumSet;
import java.util.Iterator;
import java.util.Timer;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices;

/**
 * {@code GlobalResourceManager} class manages global resources used by the
 * VTN Manager.
 */
public class GlobalResourceManager implements IVTNResourceManager {
    /**
     * Logger instance.
     */
    private final static Logger  LOG =
        LoggerFactory.getLogger(GlobalResourceManager.class);

    /**
     * A character which separates the container name from the map key.
     */
    private final static char  MAPKEY_SEPARATOR = ':';

    /**
     * Cluster cache name associated with {@link #vlanMaps}.
     */
    private final static String  CACHE_VLANMAP = "vtn.vlanmap";

    /**
     * Cluster cache name associated with {@link #portMaps}.
     */
    private final static String  CACHE_PORTMAP = "vtn.portmap";

    /**
     * VLAN IDs mapped to the virtual L2 bridge.
     */
    private ConcurrentMap<Short, String> vlanMaps;

    /**
     * Physical switch ports mapped to the virtual L2 bridge interface.
     */
    private ConcurrentMap<PortVlan, String> portMaps;

    /**
     * Cluster container service instance.
     */
    private IClusterGlobalServices  clusterService;

    /**
     * The global timer.
     */
    private Timer  globalTimer;

    /**
     * Function called by the dependency manager when all the required
     * dependencies are satisfied.
     *
     * @param c  Dependency manager component.
     */
    void init(Component c) {
        createCaches();
        globalTimer = new Timer("VTN Global Timer");
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     */
    void destroy() {
        if (globalTimer != null) {
            globalTimer.cancel();
        }
    }

    /**
     * Invoked when a cluster global service is registered.
     *
     * @param service  Cluster global service.
     */
    void setClusterGlobalService(IClusterGlobalServices service) {
        LOG.debug("Set cluster service");
        clusterService = service;
    }

    /**
     * Invoked when a cluster global service is unregistered.
     *
     * @param service  Cluster global service.
     */
    void unsetClusterGlobalService(IClusterGlobalServices service) {
        if (clusterService == service) {
            LOG.debug("Unset cluster service");
            clusterService = null;
        }
    }

    /**
     * Create global cluster caches for VTN.
     */
    private void createCaches() {
        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            LOG.error("Cluster service is not yet registered.");
            return;
        }

        Set<IClusterServices.cacheMode> cmode =
            EnumSet.of(IClusterServices.cacheMode.NON_TRANSACTIONAL);
        createCache(cluster, CACHE_VLANMAP, cmode);
        createCache(cluster, CACHE_PORTMAP, cmode);

        vlanMaps = (ConcurrentMap<Short, String>)
            getCache(cluster, CACHE_VLANMAP);
        portMaps = (ConcurrentMap<PortVlan, String>)
            getCache(cluster, CACHE_PORTMAP);

        LOG.debug("Created global VTN caches.");
    }

    /**
     * Create a named cluster cache.
     *
     * @param cs         Cluster container service.
     * @param cacheName  The name of cluster cache.
     * @param cmode      Set of cache modes.
     */
    private void createCache(IClusterGlobalServices cs, String cacheName,
                             Set<IClusterServices.cacheMode> cmode) {
        try {
            cs.createCache(cacheName, cmode);
        } catch (CacheExistException e) {
            LOG.warn("{}: Cache already exists", cacheName);
        } catch (CacheConfigException e) {
            LOG.error("{}: Invalid cache configuration: {}", cacheName, cmode);
        } catch (Exception e) {
            if (LOG.isErrorEnabled()) {
                String msg = cacheName + ": Failed to create cache";
                LOG.error(msg, e);
            }
        }
    }

    /**
     * Retrieve cluster cache associated with the given name.
     *
     * @param cs         Cluster global service.
     * @param cacheName  The name of cluster cache.
     * @return           Cluster cache, or {@code null} on failure.
     */
    private ConcurrentMap<?, ?> getCache(IClusterGlobalServices cs,
                                         String cacheName) {
        ConcurrentMap<?, ?> cache = cs.getCache(cacheName);
        if (cache == null) {
            LOG.error("{}: Cache not found", cacheName);
        }

        return cache;
    }

    /**
     * Register VLAN ID for VLAN mapping.
     *
     * @param containerName  The name of the container.
     * @param path           Path to the virtual bridge.
     * @param vlan           VLAN ID to map.
     * @return  {@code null} is returned on success.
     *          On failure, fully-qualified name of the bridge which maps
     *          the specified VLAN is returned.
     */
    @Override
    public String registerVlanMap(String containerName, VBridgePath path,
                                  short vlan) {
        synchronized (vlanMaps) {
            String name = vlanMaps.get(vlan);
            if (name == null) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(MAPKEY_SEPARATOR).append(path.toString());
                String bname = builder.toString();
                vlanMaps.put(vlan, bname);
            }
            return name;
        }
    }

    /**
     * Unregister VLAN mapping.
     *
     * @param vlan  VLAN ID.
     */
    @Override
    public void unregisterVlanMap(short vlan) {
        synchronized (vlanMaps) {
            if (vlanMaps.remove(vlan) == null) {
                // This should never happen.
                String msg = "Trying to unmap unexpected VLAN: " + vlan;
                throw new IllegalStateException(msg);
            }
        }
    }

    /**
     * Register mapping between physical switch port and virtual bridge
     * interface.
     *
     * @param containerName  The name of the container.
     * @param path           Path to the virtual bridge interface.
     * @param pvlan          Identifier of the mapped switch port.
     * @return  {@code null} is returned on success.
     *          On failure, fully-qualified name of the bridge interface
     *          which maps the specified physical switch port is returned.
     */
    @Override
    public String registerPortMap(String containerName, VBridgeIfPath path,
                                  PortVlan pvlan) {
        synchronized (portMaps) {
            String name = portMaps.get(pvlan);
            if (name == null) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(MAPKEY_SEPARATOR).append(path.toString());
                String ifname = builder.toString();
                portMaps.put(pvlan, ifname);
            }
            return name;
        }
    }

    /**
     * Unregister port mapping.
     *
     * @param pvlan  Identifier of the mapped switch port.
     */
    @Override
    public void unregisterPortMap(PortVlan pvlan) {
        synchronized (portMaps) {
            if (portMaps.remove(pvlan) == null) {
                // This should never happen.
                String msg = "Trying to unmap unexpected port: " + pvlan;
                throw new IllegalStateException(msg);
            }
        }
    }


    /**
     * Return the global timer.
     *
     * @return  The global timer.
     */
    @Override
    public Timer getTimer() {
        return globalTimer;
    }

    /**
     * Clean up resources associated with the given container.
     *
     * @param containerName  The name of the container.
     */
    @Override
    public void cleanUp(String containerName) {
        LOG.debug("{}: Clean up resources", containerName);

        StringBuilder builder = new StringBuilder(containerName);
        String prefix = builder.append(MAPKEY_SEPARATOR).toString();
        synchronized (vlanMaps) {
            for (Iterator<String> it = vlanMaps.values().iterator();
                 it.hasNext();) {
                String name = it.next();
                if (name.startsWith(prefix)) {
                    it.remove();
                }
            }
        }

        synchronized (portMaps) {
            for (Iterator<String> it = portMaps.values().iterator();
                 it.hasNext();) {
                String name = it.next();
                if (name.startsWith(prefix)) {
                    it.remove();
                }
            }
        }
    }
}
