/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Map;
import java.util.Timer;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.osgi.framework.Bundle;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.Version;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.clustering.services.ICoordinatorChangeAware;

/**
 * {@code GlobalResourceManager} class manages global resources used by the
 * VTN Manager.
 */
public class GlobalResourceManager
    implements IVTNGlobal, IVTNResourceManager, ICoordinatorChangeAware {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(GlobalResourceManager.class);

    /**
     * Current API version of the VTN Manager.
     */
    public static final int  API_VERSION = 1;

    /**
     * A character which separates the container name from the map key.
     */
    private static final char  MAPKEY_SEPARATOR = ':';

    /**
     * Cluster cache name associated with {@link #vlanMaps}.
     */
    private static final String  CACHE_VLANMAP = "vtn.vlanmap";

    /**
     * Cluster cache name associated with {@link #portMaps}.
     */
    private static final String  CACHE_PORTMAP = "vtn.portmap";

    /**
     * The maximum number of threads in the thread pool for asynchronous tasks.
     */
    private static final int  THREAD_POOL_MAXSIZE = 8;

    /**
     * The number of milliseconds to keep threads in the thread pool for
     * asynchronous tasks.
     */
    private static final int  THREAD_POOL_KEEPALIVE = 10000;

    /**
     * The number of milliseconds to wait for completion of thread pool
     * shutdown.
     */
    private static final long  THREAD_POOL_SHUTDOWN = 5000;

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
     * The global thread pool to execute commands.
     */
    private VTNThreadPool  asyncThreadPool;

    /**
     * The IP address of the controller in the cluster.
     */
    private InetAddress  controllerAddress;

    /**
     * Set of IP addresses of remote cluster nodes.
     */
    private Set<InetAddress>  remoteClusterNodes = new HashSet<InetAddress>();

    /**
     * A list of VTN Manager services.
     */
    private CopyOnWriteArrayList<VTNManagerImpl>  vtnManagers =
        new CopyOnWriteArrayList<VTNManagerImpl>();

    /**
     * Function called by the dependency manager when all the required
     * dependencies are satisfied.
     *
     * @param c  Dependency manager component.
     */
    void init(Component c) {
        initCluster();

        globalTimer = new Timer("VTN Global Timer");
        asyncThreadPool =
            new VTNThreadPool("VTN Async Thread", THREAD_POOL_MAXSIZE,
                              THREAD_POOL_KEEPALIVE);
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     */
    void destroy() {
        if (globalTimer != null) {
            globalTimer.cancel();
            globalTimer = null;
        }

        if (asyncThreadPool != null) {
            asyncThreadPool.shutdown();
            if (!asyncThreadPool.join(THREAD_POOL_SHUTDOWN)) {
                LOG.error("Async thread pool did not terminate within {} msec",
                          THREAD_POOL_SHUTDOWN);
                asyncThreadPool.terminate();
            }
            asyncThreadPool = null;
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
     * Initialize clustering service.
     */
    private void initCluster() {
        // Clear remote cluster addresses.
        remoteClusterNodes.clear();

        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            // Create dummy caches.
            vlanMaps = new ConcurrentHashMap<Short, String>();
            portMaps = new ConcurrentHashMap<PortVlan, String>();

            // Use loopback address as controller's address.
            controllerAddress = InetAddress.getLoopbackAddress();
        } else {
            Set<IClusterServices.cacheMode> cmode =
                EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL);
            createCache(cluster, CACHE_VLANMAP, cmode);
            createCache(cluster, CACHE_PORTMAP, cmode);

            vlanMaps = (ConcurrentMap<Short, String>)
                getCache(cluster, CACHE_VLANMAP);
            portMaps = (ConcurrentMap<PortVlan, String>)
                getCache(cluster, CACHE_PORTMAP);

            controllerAddress = cluster.getMyAddress();
            if (controllerAddress == null) {
                controllerAddress = InetAddress.getLoopbackAddress();
            }

            List<InetAddress> addrs = cluster.getClusteredControllers();
            if (addrs != null) {
                for (InetAddress addr: addrs) {
                    if (!addr.equals(controllerAddress)) {
                        remoteClusterNodes.add(addr);
                    }
                }
            }
            LOG.debug("Remote controller addresses: {}", remoteClusterNodes);
        }

        ClusterEventId.setLocalAddress(controllerAddress);
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

    // IVTNGlobal

    /**
     * Return the API version of the VTN Manager.
     *
     * <p>
     *   The API version will be incremented when changes which breaks
     *   compatibility is made to the API of VTN Manager.
     * </p>
     *
     * @return  The API version of the VTN Manager.
     */
    @Override
    public int getApiVersion() {
        return API_VERSION;
    }

    /**
     * Return the version information of the OSGi bundle which implements
     * the VTN Manager.
     *
     * @return  A {@link BundleVersion} object which represents the version
     *          of the OSGi bundle which implements the VTN Manager.
     *          {@code null} is returned if the VTN Manager is loaded by
     *          a OSGi bundle class loader.
     */
    @Override
    public BundleVersion getBundleVersion() {
        Bundle bundle = FrameworkUtil.getBundle(GlobalResourceManager.class);
        if (bundle == null) {
            return null;
        }

        Version ver = bundle.getVersion();
        return new BundleVersion(ver.getMajor(), ver.getMinor(),
                                 ver.getMicro(), ver.getQualifier());
    }

    // IVTNResourceManager

    /**
     * Add the VTN Manager service.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void addManager(VTNManagerImpl mgr) {
        if (vtnManagers.addIfAbsent(mgr)) {
            LOG.debug("{}: Add VTN Manager: {}", mgr.getContainerName(), mgr);
        }
    }

    /**
     * Remove the VTN Manager service.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void removeManager(VTNManagerImpl mgr) {
        if (vtnManagers.remove(mgr)) {
            LOG.debug("{}: Remove VTN Manager: {}", mgr.getContainerName(),
                      mgr);
        }
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
        StringBuilder builder = new StringBuilder(containerName);
        builder.append(MAPKEY_SEPARATOR).append(path.toString());
        String bname = builder.toString();
        return vlanMaps.putIfAbsent(vlan, bname);
    }

    /**
     * Unregister VLAN mapping.
     *
     * @param vlan  VLAN ID.
     */
    @Override
    public void unregisterVlanMap(short vlan) {
        if (vlanMaps.remove(vlan) == null) {
            // This should never happen.
            String msg = "Trying to unmap unexpected VLAN: " + vlan;
            throw new IllegalStateException(msg);
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
        StringBuilder builder = new StringBuilder(containerName);
        builder.append(MAPKEY_SEPARATOR).append(path.toString());
        String ifname = builder.toString();
        return portMaps.putIfAbsent(pvlan, ifname);
    }

    /**
     * Unregister port mapping.
     *
     * @param pvlan  Identifier of the mapped switch port.
     */
    @Override
    public void unregisterPortMap(PortVlan pvlan) {
        if (portMaps.remove(pvlan) == null) {
            // This should never happen.
            String msg = "Trying to unmap unexpected port: " + pvlan;
            throw new IllegalStateException(msg);
        }
    }

    /**
     * Determine whether the given switch port is mapped to the virtual
     * interface or not.
     *
     * @param pvlan  A pair of the switch port and the VLAN ID.
     * @return  {@code true} is returned only if the given switch port is
     *          mapped to the virtual interface.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public boolean isPortMapped(PortVlan pvlan) {
        return portMaps.containsKey(pvlan);
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
     * Run the given command asynchronously.
     *
     * <p>
     *   Note that this method exucutes the given command using a thread pool
     *   which has multiple threads. So the order of the command execution is
     *   unspecified.
     * </p>
     *
     * @param command  The command to be executed.
     * @return  {@code true} is returned if the specified task was submitted.
     *          {@code false} is returned if the specified tas was rejected.
     */
    @Override
    public boolean executeAsync(Runnable command) {
        return asyncThreadPool.execute(command);
    }

    /**
     * Return the IP address of the controller in the cluster.
     *
     * @return  The IP address of the controller.
     */
    @Override
    public InetAddress getControllerAddress() {
        return controllerAddress;
    }

    /**
     * Return the number of remote cluster nodes.
     *
     * <p>
     *   Zero is returned if no remote node was found in the cluster.
     * </p>
     *
     * @return  The number of remote cluster nodes.
     */
    @Override
    public int getRemoteClusterSize() {
        synchronized (remoteClusterNodes) {
            return remoteClusterNodes.size();
        }
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
        Set<Short> vlanSet = new HashSet<Short>();
        for (Map.Entry<Short, String> entry: vlanMaps.entrySet()) {
            String name = entry.getValue();
            if (name.startsWith(prefix)) {
                vlanSet.add(entry.getKey());
            }
        }
        for (Short vlan: vlanSet) {
            vlanMaps.remove(vlan);
        }

        Set<PortVlan> pvSet = new HashSet<PortVlan>();
        for (Map.Entry<PortVlan, String> entry: portMaps.entrySet()) {
            String name = entry.getValue();
            if (name.startsWith(prefix)) {
                pvSet.add(entry.getKey());
            }
        }
        for (PortVlan pvlan: pvSet) {
            portMaps.remove(pvlan);
        }
    }

    // ICoordinatorChangeAware

    /**
     * Function that will be called when there is the event of
     * coordinator change in the cluster.
     */
    @Override
    public void coordinatorChanged() {
        List<InetAddress> addrs = clusterService.getClusteredControllers();
        Set<InetAddress> newset = new HashSet<InetAddress>(addrs);
        Set<InetAddress> added = new HashSet<InetAddress>();
        Set<InetAddress> removed = new HashSet<InetAddress>();
        int size;
        synchronized (remoteClusterNodes) {
            // Eliminate removed addresses.
            for (Iterator<InetAddress> it = remoteClusterNodes.iterator();
                 it.hasNext();) {
                InetAddress addr = it.next();
                if (!newset.contains(addr)) {
                    removed.add(addr);
                    it.remove();
                }
            }

            // Set remote controller's addresses.
            for (InetAddress addr: addrs) {
                if (!addr.equals(controllerAddress) &&
                    remoteClusterNodes.add(addr)) {
                    added.add(addr);
                }
            }
            size = remoteClusterNodes.size();
        }

        // Invoke cluster node change event listeners.
        if (!added.isEmpty() || !removed.isEmpty()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("Remote cluster node changed: " +
                          "size={}, added={}, removed={}",
                          size, added, removed);
            }
            for (VTNManagerImpl mgr: vtnManagers) {
                mgr.clusterNodeChanged(added, removed);
            }
        }
    }
}
