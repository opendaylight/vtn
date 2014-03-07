/*
 * Copyright (c) 2013-2014 NEC Corporation
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
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.clustering.services.ICoordinatorChangeAware;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.StatusCode;

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
     * The name of the cluster cache which keeps revision identifier of
     * configuration per mapping type.
     */
    private static final String CACHE_CONFREVISION = "vtn.confrev";

    /**
     * The name of the cluster cache which keeps VLAN mappings.
     */
    private static final String  CACHE_VLANMAP = "vtn.vlanmap";

    /**
     * The name of the cluster cache which keeps port mappings.
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
     * Initial revision number of the global configuration.
     */
    private static final int  CONFIG_REV_INIT = 0;

    /**
     * A map which keeps the current revision number for the global
     * configuration of virtual mappings.
     *
     * <p>
     *   This is used to avoid expensive {@link ConcurrentMap} methods.
     *   Only {@link MapType#ALL} is used as map key.
     * </p>
     */
    private ConcurrentMap<MapType, Integer> configRevision;

    /**
     * A set of VLAN mappings established in all containers.
     */
    private ConcurrentMap<NodeVlan, MapReference> vlanMaps;

    /**
     * A set of port mappings established in all containers.
     */
    private ConcurrentMap<PortVlan, MapReference> portMaps;

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
     * This class is used to update virtual network mapping in a cluster
     * cache transaction.
     *
     * @param <T>  The type of the object returned by this operation.
     */
    private abstract class ConfigTransaction<T> extends CacheTransaction<T> {
        /**
         * Determine whether the configuration was changed or not.
         */
        private boolean  changed = false;

        /**
         * Construct a new instance.
         *
         * @param config  A {@link VTNConfig} object.
         */
        private ConfigTransaction(VTNConfig config) {
            super(clusterService, config.getCacheTransactionTimeout());
        }

        /**
         * Execute a procedure in a cluster cache transaction.
         *
         * @return  An object returned by {@link #update()}.
         * @throws CacheRetryException
         *   Cluster cache operation should be retried from scratch.
         * @throws VTNException
         *   A fatal error occurred.
         */
        @Override
        protected final T executeImpl() throws VTNException {
            Integer ini = Integer.valueOf(CONFIG_REV_INIT);
            Integer current = configRevision.putIfAbsent(MapType.ALL, ini);
            if (current == null) {
                current = ini;
            }

            T ret;
            try {
                ret = update();
            } catch (VTNException e) {
                // This exception may be caused because of cluster cache
                // inconsistency. So we should check current revision number.
                if (current.equals(configRevision.get(MapType.ALL))) {
                    throw e;
                }

                throw new CacheRetryException();
            }

            if (changed) {
                // Update revision number.
                Integer rev = Integer.valueOf(current.intValue() + 1);
                if (configRevision.replace(MapType.ALL, current, rev)) {
                    return ret;
                }
            } else {
                // Abort the current transaction.
                abort();

                // Ensure that revision number is not changed.
                if (current.equals(configRevision.get(MapType.ALL))) {
                    return ret;
                }
            }

            throw new CacheRetryException();
        }

        /**
         * Notify that the configuration has been changed successfully.
         */
        protected final void setChanged() {
            changed = true;
        }

        /**
         * Update the virtual mapping.
         *
         * <p>
         *   {@link #setChanged()} must be called in this method if the
         *   configuration has been changed.
         * </p>
         *
         * @return  An arbitrary object.
         * @throws VTNException
         *   A fatal error occurred.
         */
        protected abstract T update() throws VTNException;
    }

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
        LOG.trace("Set cluster service: {}", service);
        clusterService = service;
    }

    /**
     * Invoked when a cluster global service is unregistered.
     *
     * @param service  Cluster global service.
     */
    void unsetClusterGlobalService(IClusterGlobalServices service) {
        if (clusterService == service) {
            LOG.trace("Unset cluster service: ", service);
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
            configRevision = new ConcurrentHashMap<MapType, Integer>();
            vlanMaps = new ConcurrentHashMap<NodeVlan, MapReference>();
            portMaps = new ConcurrentHashMap<PortVlan, MapReference>();

            // Use loopback address as controller's address.
            controllerAddress = InetAddress.getLoopbackAddress();
        } else {
            Set<IClusterServices.cacheMode> cmode =
                EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL);
            createCache(cluster, CACHE_CONFREVISION, cmode);
            createCache(cluster, CACHE_VLANMAP, cmode);
            createCache(cluster, CACHE_PORTMAP, cmode);

            configRevision = (ConcurrentMap<MapType, Integer>)
                getCache(cluster, CACHE_CONFREVISION);
            vlanMaps = (ConcurrentMap<NodeVlan, MapReference>)
                getCache(cluster, CACHE_VLANMAP);
            portMaps = (ConcurrentMap<PortVlan, MapReference>)
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

    /**
     * Return the current revision number of the global configuration.
     *
     * @return  An {@link Integer} object which represents the current
     *          revision number.
     */
    private Integer getConfigRevision() {
        Integer rev = configRevision.get(MapType.ALL);
        if (rev == null) {
            rev = Integer.valueOf(CONFIG_REV_INIT);
        }

        return rev;
    }

    /**
     * Remove the VLAN mapping which maps the network specified by
     * {@link NodeVlan}.
     *
     * @param nvlan  A {@link NodeVlan} object which specifies the VLAN
     *               network.
     * @throws VTNException  A fatal error occurred.
     */
    private void removeVlanMap(NodeVlan nvlan) throws VTNException {
        // Remove the VLAN mapping.
        MapReference old = vlanMaps.remove(nvlan);
        if (old == null) {
            throw new VTNException(StatusCode.NOTFOUND,
                                   "Trying to unmap unexpected VLAN: " +
                                   nvlan);
        }
    }

    /**
     * Change port mapping configuration.
     *
     * @param pvlan  A {@link PortVlan} object which specifies the VLAN network
     *               to be mapped. No port mapping is added if {@code null} is
     *               specified.
     * @param ref    A reference to the port mapping to be registered.
     * @param rmlan  A {@link PortVlan} object which specifies the VLAN network
     *               to be unmapped. No port mapping is removed if {@code null}
     *               is specified.
     * @return  {@code null} is returned on success.
     *          On failure, a reference to the port mapping which maps the
     *          VLAN network specified by {@code pvlan} is returned.
     * @throws VTNException  A fatal error occurred.
     */
    private MapReference changePortMap(PortVlan pvlan, MapReference ref,
                                       PortVlan rmlan) throws VTNException {
        if (rmlan != null) {
            // Remove the port mapping which maps rmlan.
            if (portMaps.remove(rmlan) == null) {
                // This should never happen.
                throw new VTNException(StatusCode.NOTFOUND,
                                       "Trying to unmap unexpected port: " +
                                       rmlan);
            }
        }

        return (pvlan != null)
            ? portMaps.putIfAbsent(pvlan, ref)
            : null;
    }

    /**
     * Return a reference to virtual network mapping which maps the VLAN
     * network specified by the switch port and the VLAN ID.
     *
     * @param nc    A node connector corresponding to the switch port.
     *              Specifying {@code null} results in undefined behavior.
     * @param vlan  A VLAN ID.
     * @return      A {@link MapReference} object is returned if found.
     *              {@code null} is returned if not found.
     */
    private MapReference getMapReferenceImpl(NodeConnector nc, short vlan) {
        // Examine port mapping at first.
        PortVlan pvlan = new PortVlan(nc, vlan);
        MapReference ref = portMaps.get(pvlan);
        if (ref != null) {
            assert ref.getMapType() == MapType.PORT;
            return ref;
        }

        // Examine VLAN mapping.
        // Node that we must examine VLAN mapping with a specific node first.
        NodeVlan nvlan = new NodeVlan(nc.getNode(), vlan);
        ref = vlanMaps.get(nvlan);
        if (ref == null) {
            // Check the VLAN mapping which maps all switches.
            nvlan = new NodeVlan(null, vlan);
            ref = vlanMaps.get(nvlan);
        }

        assert ref == null || ref.getMapType() == MapType.VLAN;
        return ref;
    }

    /**
     * Remove all {@link MapReference} objects associated with the specified
     * container from the map.
     *
     * @param map            A {@link Map} which contains {@link MapReference}
     *                       objects as values.
     * @param containerName  The name of the container.
     * @param <T>            The type of keys in {@code map}.
     * @return  {@code true} is returned at least one entry was removed.
     *          {@code false} is returned if the specified map was not changed.
     */
    private <T> boolean removeMapReferences(Map<T, MapReference> map,
                                            String containerName) {
        Set<T> keys = new HashSet<T>();
        for (Map.Entry<T, MapReference> entry: map.entrySet()) {
            T key = entry.getKey();
            MapReference ref = entry.getValue();
            if (containerName.equals(ref.getContainerName())) {
                keys.add(key);
            }
        }

        if (keys.isEmpty()) {
            return false;
        }

        for (T key: keys) {
            map.remove(key);
        }

        return true;
    }

    /**
     * Clean up resources associated with the given container.
     *
     * @param containerName  The name of the container.
     * @return  {@code true} is returned the configuration was changed.
     *          {@code false} is returned if not changed.
     */
    private boolean cleanUpImpl(String containerName) {
        boolean ret = removeMapReferences(vlanMaps, containerName);
        return (removeMapReferences(portMaps, containerName) || ret);
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
     *          {@code null} is returned if the VTN Manager is not loaded by
     *          an OSGi bundle class loader.
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
     * {@inheritDoc}
     */
    @Override
    public void addManager(VTNManagerImpl mgr) {
        if (vtnManagers.addIfAbsent(mgr)) {
            LOG.trace("{}: Add VTN Manager: {}", mgr.getContainerName(), mgr);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeManager(VTNManagerImpl mgr) {
        if (vtnManagers.remove(mgr)) {
            LOG.trace("{}: Remove VTN Manager: {}", mgr.getContainerName(),
                      mgr);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MapReference registerVlanMap(VTNManagerImpl mgr, VlanMapPath path,
                                        final NodeVlan nvlan)
        throws VTNException {
        final MapReference ref =
            new MapReference(MapType.VLAN, mgr.getContainerName(), path);
        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            // This code is only for unit test.
            return vlanMaps.putIfAbsent(nvlan, ref);
        }

        // Register VLAN mapping in a cluster cache transaction.
        ConfigTransaction<MapReference> xact =
            new ConfigTransaction<MapReference>(mgr.getVTNConfig()) {
            @Override
            protected MapReference update() throws VTNException {
                MapReference old = vlanMaps.putIfAbsent(nvlan, ref);
                if (old == null) {
                    setChanged();
                }

                return old;
            }
        };

        return xact.execute();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void unregisterVlanMap(VTNManagerImpl mgr, final NodeVlan nvlan)
        throws VTNException {
        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            // This code is only for unit test.
            removeVlanMap(nvlan);
            return;
        }

        // Unregister VLAN mapping in a cluster cache transaction.
        ConfigTransaction<Object> xact =
            new ConfigTransaction<Object>(mgr.getVTNConfig()) {
            @Override
            protected Object update() throws VTNException {
                removeVlanMap(nvlan);
                setChanged();
                return null;
            }
        };

        xact.execute();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MapReference registerPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                                        final PortVlan pvlan,
                                        final PortVlan rmlan)
        throws VTNException {
        if (pvlan == null && rmlan == null) {
            return null;
        }

        final MapReference ref = (pvlan == null)
            ? null
            : new MapReference(MapType.PORT, mgr.getContainerName(), path);
        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            // This code is only for unit test.
            return changePortMap(pvlan, ref, rmlan);
        }

        // Change port mappings in a cluster cache transaction.
        ConfigTransaction<MapReference> xact =
            new ConfigTransaction<MapReference>(mgr.getVTNConfig()) {
            @Override
            protected MapReference update() throws VTNException {
                MapReference old = changePortMap(pvlan, ref, rmlan);
                if (old == null) {
                    setChanged();
                }

                return old;
            }
        };

        return xact.execute();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void unregisterPortMap(VTNManagerImpl mgr, final PortVlan pvlan)
        throws VTNException {
        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            // This code is only for unit test.
            changePortMap(null, null, pvlan);
            return;
        }

        // Unregister port mapping in a cluster cache transaction.
        ConfigTransaction<MapReference> xact =
            new ConfigTransaction<MapReference>(mgr.getVTNConfig()) {
            @Override
            protected MapReference update() throws VTNException {
                changePortMap(null, null, pvlan);
                setChanged();
                return null;
            }
        };

        xact.execute();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isVlanMapped(NodeVlan nvlan) {
        return vlanMaps.containsKey(nvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isPortMapped(PortVlan pvlan) {
        return portMaps.containsKey(pvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public  MapReference getMapReference(NodeConnector nc, short vlan) {
        Integer current;
        MapReference ref;

        do {
            // Read current revision.
            current = getConfigRevision();

            // Search for a virtual mapping which maps the given network.
            ref = getMapReferenceImpl(nc, vlan);

            // Ensure that the configuration is consistent.
        } while (!current.equals(getConfigRevision()));

        return ref;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Timer getTimer() {
        return globalTimer;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean executeAsync(Runnable command) {
        return asyncThreadPool.execute(command);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InetAddress getControllerAddress() {
        return controllerAddress;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getRemoteClusterSize() {
        synchronized (remoteClusterNodes) {
            return remoteClusterNodes.size();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean isRemoteClusterAddress(InetAddress addr) {
        synchronized (remoteClusterNodes) {
            return remoteClusterNodes.contains(addr);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void cleanUp(final String containerName) {
        LOG.trace("{}: Clean up resources", containerName);

        IClusterGlobalServices cluster = clusterService;
        if (cluster == null) {
            // This code is only for unit test.
            cleanUpImpl(containerName);
            return;
        }

        // Clean up caches in a cluster cache transaction.
        String root = GlobalConstants.STARTUPHOME.toString();
        VTNConfig config = new VTNConfig(root, containerName);
        ConfigTransaction<Object> xact =
            new ConfigTransaction<Object>(config) {
            @Override
            protected Object update() {
                if (cleanUpImpl(containerName)) {
                    setChanged();
                }
                return null;
            }
        };

        try {
            xact.execute();
        } catch (Exception e) {
            LOG.error(containerName + ": Failed to clean up resource", e);
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
