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
import java.util.HashMap;
import java.util.Timer;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

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
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
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
    private ConcurrentMap<String, VTNManagerImpl>  vtnManagers =
        new ConcurrentHashMap<String, VTNManagerImpl>();

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
     * A base class for classes which represents the result of configuration
     * change.
     */
    protected abstract class MapResult {
        /**
         * A map which keeps {@link MapCleaner} instances used to purge
         * network caches.
         */
        private final Map<ObjectPair<String, String>, MapCleaner>
            mapCleaners = new HashMap<ObjectPair<String, String>, MapCleaner>();

        /**
         * Return a {@link MapCleaner} instance associated with the
         * specified {@link ObjectPair} instance.
         *
         * @param name  A {@link ObjectPair} instance which keeps a pair of
         *              container and tenant name.
         * @return  A {@link MapCleaner} instance if found.
         *          {@code null} is returned if not found.
         */
        protected final MapCleaner
            getMapCleaner(ObjectPair<String, String> name) {
            return mapCleaners.get(name);
        }

        /**
         * Associate the specified {@link MapCleaner} instance with the
         * specified {@link ObjectPair} instance.
         *
         * @param name  A {@link ObjectPair} instance which keeps a pair of
         *              container and tenant name.
         * @param cl    A {@link MapCleaner} instance.
         */
        protected final void addMapCleaner(ObjectPair<String, String> name,
                                           MapCleaner cl) {
            mapCleaners.put(name, cl);
        }

        /**
         * Create a key for {@link MapCleaner} to be associated with
         * the specified mapping.
         *
         * @param ref  A reference to a virtual mapping.
         * @return  A {@link ObjectPair} instance which keeps a pair of
         *          container and tenant name.
         */
        protected final ObjectPair<String, String>
            getMapCleanerKey(MapReference ref) {
            String containerName = ref.getContainerName();
            String tenantName = ref.getPath().getTenantName();
            return new ObjectPair<String, String>(containerName, tenantName);
        }

        /**
         * Clean up transaction of configuration change.
         *
         * @param mgr         A VTN Manager service which manages the target
         *                    tenant.
         * @param tenantName  The name of the virtual tenant which includes
         *                    a new mapping.
         */
        protected final void cleanUp(VTNManagerImpl mgr, String tenantName) {
            String containerName = mgr.getContainerName();
            ObjectPair<String, String> name =
                new ObjectPair<String, String>(containerName, tenantName);
            MapCleaner cl = mapCleaners.remove(name);
            if (cl != null) {
                // Network caches in the virtual tenant which includes
                // a new mapping need to be purged immediately.
                cl.purge(mgr, tenantName);
            }

            // Purge network caches in other tenants.
            for (Map.Entry<ObjectPair<String, String>, MapCleaner> entry:
                     mapCleaners.entrySet()) {
                name = entry.getKey();
                final MapCleaner cleaner = entry.getValue();

                String container = name.getLeft();
                final String tenant = name.getRight();
                final VTNManagerImpl cmgr = vtnManagers.get(container);
                if (cmgr == null) {
                    // This should never happen.
                    LOG.warn("{}: VTN Manager service not found.", container);
                    continue;
                }

                // Purge network caches in the container.
                cmgr.purge(cleaner, tenant);
            }

            cleanUpImpl();
        }

        /**
         * Clean up transaction of configuration change.
         */
        protected abstract void cleanUpImpl();
    }

    /**
     * An instance of this class represents the result of
     * {@link #changePortMap(PortVlan, MapReference, PortVlan, boolean)} call.
     */
    private final class PortMapResult extends MapResult {
        /**
         * A reference to existing port mapping which conflicts the specified
         * mapping.
         */
        private MapReference  conflicted;

        /**
         * A {@link NodeConnector} corresponding to a switch port mapped by
         * a new port mapping.
         */
        private NodeConnector  mappedPort;

        /**
         * Create a {@link PortMapCleaner} instance which represents VLAN
         * network superseded by a new port mapping.
         *
         * @param pvlan  A {@link PortVlan} instance which represents VLAN
         *               network mapped by a new port mapping.
         * @param ref    A reference to a virtual mapping superseded by a new
         *               port mapping.
         */
        private void setMapCleaner(PortVlan pvlan, MapReference ref) {
            ObjectPair<String, String> name = getMapCleanerKey(ref);
            PortMapCleaner cl = (PortMapCleaner)getMapCleaner(name);
            if (cl == null) {
                cl = new PortMapCleaner();
                addMapCleaner(name, cl);
            }

            cl.add(pvlan);
        }

        /**
         * Try to register a new port mapping.
         *
         * @param pvlan  A {@link PortVlan} instances which represents VLAN
         *               network mapped by a new port mapping.
         * @param ref    A {@link MapReference} instance which points a new
         *               port mapping.
         * @param purge  If {@code true} is specified, network caches
         *               corresponding to VLAN network superseded by a new
         *               port mapping will be purged.
         */
        private void register(PortVlan pvlan, MapReference ref,
                              boolean purge) {
            MapReference old = portMaps.putIfAbsent(pvlan, ref);
            if (old != null) {
                // Conflicted with another port mapping.
                assert old.getMapType() == MapType.PORT;
                conflicted = old;
                return;
            }

            if (purge) {
                // Network caches created by a VLAN mapping superseded by a
                // new port mapping must be purged.
                NodeConnector port = pvlan.getNodeConnector();
                short vlan = pvlan.getVlan();
                NodeVlan nvlan = new NodeVlan(port.getNode(), vlan);
                MapReference vref = vlanMaps.get(nvlan);
                if (vref == null) {
                    nvlan = new NodeVlan(null, vlan);
                    vref = vlanMaps.get(nvlan);
                }
                if (vref != null) {
                    setMapCleaner(pvlan, vref);
                }
            }
        }

        /**
         * Return a reference to a port mapping which conflicted a new port
         * mapping.
         *
         * @return  A reference to a port mapping which conflicted a new port
         *          mapping is returned.
         *          {@code null} is returned if a new port mapping was
         *          successfully confgured.
         */
        private MapReference getConflicted() {
            return conflicted;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void cleanUpImpl() {
        }
    }

    /**
     * An instance of this class represents the result of VLAN mapping
     * configuration change.
     */
    private final class VlanMapResult extends MapResult {
        /**
         * A reference to existing VLAN mapping which conflicts the specified
         * mapping.
         */
        private MapReference  conflicted;

        /**
         * Create a {@link VlanMapCleaner} instance which represents VLAN
         * network superseded by a new VLAN mapping.
         *
         * @param nvlan  A {@link NodeVlan} instance which represents VLAN
         *               network mapped by a new VLAN mapping.
         * @param ref    A reference to a virtual mapping superseded by a new
         *               VLAN mapping.
         */
        private void setMapCleaner(NodeVlan nvlan, MapReference ref) {
            ObjectPair<String, String> name = getMapCleanerKey(ref);
            VlanMapCleaner cl = (VlanMapCleaner)getMapCleaner(name);
            if (cl == null) {
                cl = new VlanMapCleaner();
                addMapCleaner(name, cl);
            }

            cl.add(nvlan);
        }

        /**
         * Return a reference to a VLAN mapping which conflicted a new VLAN
         * mapping.
         *
         * @return  A reference to a VLAN mapping which conflicted a new VLAN
         *          mapping is returned.
         *          {@code null} is returned if a new VLAN mapping was
         *          successfully confgured.
         */
        private MapReference getConflicted() {
            return conflicted;
        }

        /**
         * Try to register a new VLAN mapping.
         *
         * @param nvlan  A {@link NodeVlan} instances which represents VLAN
         *               network mapped by a new VLAN mapping.
         * @param ref    A {@link MapReference} instance which points a new
         *               VLAN mapping.
         * @param purge  If {@code true} is specified, network caches
         *               corresponding to VLAN network superseded by a new
         *               VLAN mapping will be purged.
         */
        private void register(NodeVlan nvlan, MapReference ref,
                              boolean purge) {
            MapReference old = vlanMaps.putIfAbsent(nvlan, ref);
            if (old != null) {
                // Conflicted with another VLAN mapping.
                conflicted = old;
                return;
            }

            if (purge && nvlan.getNode() != null) {
                // Check to see if a new VLAN mappnig supersedes existing
                // VLAN mapping.
                NodeVlan key = new NodeVlan(null, nvlan.getVlan());
                MapReference vref = vlanMaps.get(key);
                if (vref != null) {
                    setMapCleaner(nvlan, vref);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void cleanUpImpl() {
        }
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
     * @param ref    A reference to the VLAN mapping to be removed.
     * @param nvlan  A {@link NodeVlan} object which specifies the VLAN
     *               network.
     * @throws VTNException  A fatal error occurred.
     */
    private void removeVlanMap(MapReference ref, NodeVlan nvlan)
        throws VTNException {
        // Remove the VLAN mapping.
        MapReference old = vlanMaps.remove(nvlan);
        if (!ref.equals(old)) {
            StringBuilder builder = new StringBuilder(ref.toString());
            builder.append(": Trying to unmap unexpected VLAN: nvlan=").
                append(nvlan).append(", old=").append(old);
            throw new VTNException(StatusCode.NOTFOUND, builder.toString());
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
     * @param purge  If {@code true} is specified, network caches corresponding
     *               to VLAN network superseded by a new port mapping will be
     *               purged.
     * @return  An instance of {@link PortMapResult} is returned.
     * @throws VTNException  A fatal error occurred.
     */
    private PortMapResult changePortMap(PortVlan pvlan, MapReference ref,
                                        PortVlan rmlan, boolean purge)
        throws VTNException {
        if (rmlan != null) {
            // Remove the port mapping which maps rmlan.
            MapReference old = portMaps.remove(rmlan);
            if (!ref.equals(old)) {
                // This should never happen.
                StringBuilder builder = new StringBuilder(ref.toString());
                builder.append(": Trying to unmap unexpected port: rmlan=").
                    append(rmlan).append(", old=").append(old);
                throw new VTNException(StatusCode.NOTFOUND,
                                       builder.toString());
            }
        }

        PortMapResult result = new PortMapResult();
        if (pvlan != null) {
            result.register(pvlan, ref, purge);
        }

        return result;
    }

    /**
     * Purge network caches originated by the specified virtual mapping.
     *
     * @param mgr  VTN Manager service.
     * @param ref  A reference to the virtual mapping.
     */
    private void purge(VTNManagerImpl mgr, MapReference ref) {
        String container = mgr.getContainerName();
        VBridgePath path = ref.getPath();
        PathMapCleaner cleaner = new PathMapCleaner(path);
        cleaner.purge(mgr, null);
    }

    /**
     * Purge network caches originated by the specified port mapping.
     *
     * @param mgr    VTN Manager service.
     * @param path   A path to the vBridge which maps the specified network.
     * @param pvlan  A {@link PortVlan} instance which specifies the
     *               VLAN network on a switch port.
     */
    private void purge(VTNManagerImpl mgr, VBridgePath path, PortVlan pvlan) {
        String container = mgr.getContainerName();
        NodeConnector port = pvlan.getNodeConnector();
        short vlan = pvlan.getVlan();

        // Remove MAC addresses detected on the specified port.
        MacAddressTable table = mgr.getMacAddressTable(path);
        if (table != null) {
            table.flush(port, vlan);
        }

        // Remove flow entries previously mapped by the specified port mapping.
        String tenantName = path.getTenantName();
        VTNThreadData.removeFlows(mgr, tenantName, port, vlan);
    }

    /**
     * Return a reference to virtual network mapping which maps the specified
     * host.
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
        // Note that we must examine VLAN mapping with a specific node first.
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
     * @return  {@code true} is returned if at least one entry was removed.
     *          {@code false} is returned if the specified map was not changed.
     */
    private <T> boolean removeMapReferences(Map<T, MapReference> map,
                                            String containerName) {
        Set<T> keys = new HashSet<T>();
        for (Map.Entry<T, MapReference> entry: map.entrySet()) {
            MapReference ref = entry.getValue();
            if (containerName.equals(ref.getContainerName())) {
                T key = entry.getKey();
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
        String cname = mgr.getContainerName();
        if (vtnManagers.putIfAbsent(cname, mgr) != null) {
            LOG.trace("{}: Add VTN Manager: {}", cname, mgr);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void removeManager(VTNManagerImpl mgr) {
        String cname = mgr.getContainerName();
        if (vtnManagers.remove(cname) != null) {
            LOG.trace("{}: Remove VTN Manager: {}", cname, mgr);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MapReference registerVlanMap(VTNManagerImpl mgr, VlanMapPath path,
                                        final NodeVlan nvlan,
                                        final boolean purge)
        throws VTNException {
        final MapReference ref = mgr.getMapReference(path);

        // Register VLAN mapping in a cluster cache transaction.
        ConfigTransaction<VlanMapResult> xact =
            new ConfigTransaction<VlanMapResult>(mgr.getVTNConfig()) {
            @Override
            protected VlanMapResult update() throws VTNException {
                VlanMapResult r = new VlanMapResult();
                r.register(nvlan, ref, purge);
                if (r.getConflicted() == null) {
                    setChanged();
                }

                return r;
            }
        };

        VlanMapResult result = xact.execute();
        result.cleanUp(mgr, path.getTenantName());

        return result.getConflicted();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void unregisterVlanMap(VTNManagerImpl mgr, VlanMapPath path,
                                  final NodeVlan nvlan, boolean purge)
        throws VTNException {
        final MapReference ref = mgr.getMapReference(path);

        // Unregister VLAN mapping in a cluster cache transaction.
        ConfigTransaction<Object> xact =
            new ConfigTransaction<Object>(mgr.getVTNConfig()) {
            @Override
            protected Object update() throws VTNException {
                removeVlanMap(ref, nvlan);
                setChanged();
                return null;
            }
        };

        xact.execute();
        if (purge) {
            purge(mgr, ref);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MapReference registerPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                                        final PortVlan pvlan,
                                        final PortVlan rmlan,
                                        final boolean purge)
        throws VTNException {
        if (pvlan == null && rmlan == null) {
            return null;
        }

        // Change port mappings in a cluster cache transaction.
        final MapReference ref = mgr.getMapReference(path);
        ConfigTransaction<PortMapResult> xact =
            new ConfigTransaction<PortMapResult>(mgr.getVTNConfig()) {
            @Override
            protected PortMapResult update() throws VTNException {
                PortMapResult r = changePortMap(pvlan, ref, rmlan, purge);
                if (r.getConflicted() == null) {
                    setChanged();
                }

                return r;
            }
        };

        PortMapResult result = xact.execute();
        MapReference conflicted = result.getConflicted();
        if (conflicted == null) {
            result.cleanUp(mgr, path.getTenantName());
            if (rmlan != null) {
                purge(mgr, path, rmlan);
            }
        }

        return conflicted;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void unregisterPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                                  PortVlan pvlan, boolean purge)
        throws VTNException {
        registerPortMap(mgr, path, null, pvlan, purge);
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
            for (VTNManagerImpl mgr: vtnManagers.values()) {
                mgr.clusterNodeChanged(added, removed);
            }
        }
    }
}
