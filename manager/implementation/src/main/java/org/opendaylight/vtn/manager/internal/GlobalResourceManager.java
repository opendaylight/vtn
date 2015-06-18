/*
 * Copyright (c) 2013-2015 NEC Corporation
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
import java.util.ArrayList;
import java.util.Set;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentSkipListSet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.osgi.framework.Bundle;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.Version;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacMapState;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
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
    public static final int  API_VERSION = 2;

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
     * The name of the cluster cache which keeps MAC mappings.
     */
    private static final String  CACHE_MACMAP = "vtn.macmap";

    /**
     * The name of the cluster cache which keeps VLAN networks to be eliminated
     * from MAC mapping.
     */
    private static final String  CACHE_MACMAP_DENY = "vtn.macmap.deny";

    /**
     * The name of the cluster cache which keeps runtime state of
     * MAC mapping.
     */
    private static final String  CACHE_MACMAP_STATE = "vtn.macmap.state";

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
     *
     * <p>
     *   Note that this map is also used to maintain VLAN network on
     *   switch port owned by MAC mappings.
     * </p>
     */
    private ConcurrentMap<PortVlan, MapReference> portMaps;

    /**
     * A set of layer 2 hosts to be mapped by MAC mapping.
     */
    private ConcurrentMap<MacVlan, MapReference> macMapAllowed;

    /**
     * A set of layer 2 hosts not to be mapped by MAC mapping.
     *
     * <p>
     *   This map takes a {@link MacVlan} instance corresponding to the
     *   layer 2 host as key, and a set of {@link MapReference} instances
     *   corresponding to MAC mapping.
     * </p>
     */
    private ConcurrentMap<MacVlan, Set<MapReference>> macMapDenied;

    /**
     * A set of runtime state of MAC mappings.
     */
    private ConcurrentMap<MapReference, MacMapState> macMapStates;

    /**
     * Cluster container service instance.
     */
    private IClusterGlobalServices  clusterService;

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
    private abstract class ConfigTrans<T> extends CacheTransaction<T> {
        /**
         * Determine whether the configuration was changed or not.
         */
        private boolean  changed = false;

        /**
         * Construct a new instance.
         *
         * @param config  A {@link VTNConfig} object.
         */
        private ConfigTrans(VTNConfig config) {
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

                throw new CacheRetryException(e);
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
        private final Map<ObjectPair<String, String>, MapCleaner> mapCleaners =
            new HashMap<ObjectPair<String, String>, MapCleaner>();

        /**
         * Return a {@link MapCleaner} instance associated with the
         * specified {@link ObjectPair} instance.
         *
         * @param name  A {@link ObjectPair} instance which keeps a pair of
         *              container and tenant name.
         * @return  A {@link MapCleaner} instance if found.
         *          {@code null} is returned if not found.
         */
        protected final MapCleaner getMapCleaner(
            ObjectPair<String, String> name) {
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
        protected final ObjectPair<String, String> getMapCleanerKey(
            MapReference ref) {
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
                MapCleaner cleaner = entry.getValue();
                String container = name.getLeft();
                String tenant = name.getRight();
                VTNManagerImpl cmgr = getVTNManager(container);
                if (cmgr != null) {
                    // Purge network caches in the container.
                    cmgr.purge(cleaner, tenant);
                }
            }

            cleanUpImpl(mgr);
        }

        /**
         * Clean up transaction of configuration change.
         *
         * @param mgr  VTN Manager service.
         */
        protected abstract void cleanUpImpl(VTNManagerImpl mgr);
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
         * A reference to MAC mapping invalidated by a new port mapping.
         */
        private MapReference  macMap;

        /**
         * A set of {@link MacVlan} instances which represent invalidated
         * MAC mappings.
         */
        private Set<MacVlan>  macMappedHosts;

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
                MapType type = old.getMapType();
                if (type != MapType.MAC) {
                    // Conflicted with another port mapping.
                    conflicted = old;
                    return;
                }

                // Port mapping always supersedes existing MAC mapping.
                portMaps.put(pvlan, ref);
                macMap = old;
                mappedPort = pvlan.getNodeConnector();

                // Inactivate mappings on the specified port.
                MacMapState mst = getMacMapStateForUpdate(old);
                if (mst != null) {
                    macMappedHosts = mst.inactivate(pvlan);
                    putMacMapState(old, mst);
                }
                if (purge) {
                    setMapCleaner(pvlan, old);
                }
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
        protected void cleanUpImpl(VTNManagerImpl mgr) {
            if (macMap != null) {
                String container = macMap.getContainerName();
                if (!container.equals(mgr.getContainerName())) {
                    // A new port mapping superseded a MAC mapping in
                    // another container. In this case the internal state of
                    // vBridge in which the MAC mapping is configured needs to
                    // be updated.
                    VTNManagerImpl cmgr = getVTNManager(container);
                    if (cmgr != null) {
                        cmgr.updateBridgeState((VBridgePath)macMap.getPath());
                    }
                }
            }

            // Record trace logs that notify inactivated MAC mappings.
            if (macMappedHosts != null && LOG.isTraceEnabled()) {
                for (MacVlan mvlan: macMappedHosts) {
                    MapLog mlog =
                        new MacMapInactivatedLog(macMap, mvlan, mappedPort);
                    mlog.log(LOG);
                }
            }
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
        protected void cleanUpImpl(VTNManagerImpl mgr) {
        }
    }

    /**
     * An instance of this class represents the result of
     * {@link #changeMacMap(MapReference, MacMapChange)}.
     */
    private final class MacMapResult extends MapResult {
        /**
         * A set of {@link MacVlan} instances which represents hosts already
         * mapped by this MAC mapping.
         */
        private final Set<MacVlan>  alreadyMapped = new HashSet<MacVlan>();

        /**
         * Runtime state of the target MAC mapping.
         */
        private final MacMapState  mapState;

        /**
         * Set {@code true} if the target MAC mapping is going to be removed.
         */
        private final boolean  removing;

        /**
         * A set of VLAN IDs corresponding to unmapped VLAN network.
         */
        private final Set<Short>  unmappedVlans = new HashSet<Short>();

        /**
         * A boolean value whicih indicates obsolete network caches need to
         * be purged or not.
         */
        private final boolean  doPurge;

        /**
         * A list of trace level logs.
         */
        private final List<MapLog> logList;

        /**
         * Construct a new instance.
         *
         * @param ref     A reference to the target MAC mapping.
         * @param change  A {@link MacMapChange} instance which keeps
         *                differences to be applied.
         */
        private MacMapResult(MapReference ref, MacMapChange change) {
            // Get runtime state of MAC mapping.
            // Note that we need to modify clone of MacMapState instance
            // in case of transaction abort.
            removing = change.isRemoving();
            MacMapState mst;
            if (removing) {
                mst = macMapStates.remove(ref);
                if (mst != null) {
                    mst = mst.clone();
                }
            } else {
                mst = createMacMapState(ref);
            }
            mapState = mst;
            doPurge = !(removing || change.dontPurge());
            logList = (LOG.isTraceEnabled()) ? new ArrayList<MapLog>() : null;

            if (!removing) {
                // Determine hosts that are already mapped by this MAC mapping.
                for (MacVlan mvlan: change.getAllowAddedSet()) {
                    if (mvlan.getMacAddress() == MacVlan.UNDEFINED) {
                        continue;
                    }
                    MapReference mref = getMacMapReference(mvlan);
                    if (ref.equals(mref)) {
                        alreadyMapped.add(mvlan);
                    }
                }
            }
        }

        /**
         * Return a {@link MacMapCleaner} instance.
         *
         * @param ref    A reference to the target MAC mapping.
         * @return  A {@link MacMapCleaner} instance to clean up obsolete
         *          network caches.
         */
        private MacMapCleaner getMapCleaner(MapReference ref) {
            ObjectPair<String, String> name = getMapCleanerKey(ref);
            MacMapCleaner cl = (MacMapCleaner)getMapCleaner(name);
            if (cl == null) {
                cl = new MacMapCleaner(GlobalResourceManager.this, ref);
                addMapCleaner(name, cl);
            }

            return cl;
        }

        /**
         * Create a {@link MacMapCleaner} instance which represents VLAN
         * network superseded by a new MAC mapping.
         *
         * @param ref    A reference to the target MAC mapping.
         * @param mvlan  A {@link MacVlan} instance which specifies the
         *               mapped host.
         */
        private void addMappedHost(MapReference ref, MacVlan mvlan) {
            if (doPurge) {
                MacMapCleaner cl = getMapCleaner(ref);
                cl.addMappedHost(mvlan);
            }
        }

        /**
         * Create a {@link MacMapCleaner} instance which represents VLAN
         * network detached from the MAC mapping.
         *
         * @param ref    A reference to the target MAC mapping.
         * @param mvlan  A {@link MacVlan} instance which specifies the
         *               unmapped host.
         */
        private void addUnmappedHost(MapReference ref, MacVlan mvlan) {
            if (doPurge) {
                MacMapCleaner cl = getMapCleaner(ref);
                cl.addUnmappedHost(mvlan);
            }
        }

        /**
         * Inactivate MAC mapping which maps the specified host.
         *
         * @param mst    A {@link MacMapState} instance which contains runtime
         *               information about the MAC mapping.
         * @param ref    A reference to a MAC mapping.
         * @param mvlan  A {@link MacVlan} instance.
         * @throws VTNException  An error occurred.
         */
        private void inactivate(MacMapState mst, MapReference ref,
                                MacVlan mvlan)
            throws VTNException {
            Set<PortVlan> rels = new HashSet<PortVlan>();
            NodeConnector oldPort = mst.inactivate(mvlan, rels);
            if (oldPort == null) {
                return;
            }

            if (logList != null) {
                MapLog mlog = new MacMapInactivatedLog(ref, mvlan, oldPort);
                logList.add(mlog);
            }

            // Release switch port reserved by MAC mapping.
            releasePort(rels, ref);
        }

        /**
         * Inactivate MAC mapping which maps the specified host.
         *
         * @param ref    A reference to a MAC mapping.
         * @param mvlan  A {@link MacVlan} instance.
         * @throws VTNException  An error occurred.
         */
        private void inactivate(MapReference ref, MacVlan mvlan)
            throws VTNException {
            MacMapState mst = getMacMapStateForUpdate(ref);
            if (mst != null) {
                inactivate(mst, ref, mvlan);
                putMacMapState(ref, mst);
            }
        }

        /**
         * Get a set of {@link MapReference} instances associated with the
         * specified host in the denied host set.
         *
         * <p>
         *   This method must be used if you want to modify the denied host
         *   set.
         * </p>
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  A set of {@link MapReference} instances associated with
         *          the specified host in the denied host set.
         *          An empty set is returned if there was no mapping for the
         *          specified host.
         */
        private Set<MapReference> getDenied(MacVlan mvlan) {
            Set<MapReference> set = macMapDenied.get(mvlan);
            if (set == null) {
                return new ConcurrentSkipListSet<MapReference>();
            }

            // Return a copy of the set for succeeding modification.
            return ((ConcurrentSkipListSet<MapReference>)set).clone();
        }

        /**
         * Remove a map entry associated with the specified host in the
         * denied host set.
         *
         * @param mvlan  A {@link MacVlan} instance.
         * @return  A previous value associated with the specified host.
         *          {@code null} if there was no mapping for the host.
         */
        private Set<MapReference> removeDenied(MacVlan mvlan) {
            Set<MapReference> set = macMapDenied.remove(mvlan);
            if (set == null) {
                return set;
            }

            // Return a copy of the set for succeeding modification.
            return ((ConcurrentSkipListSet<MapReference>)set).clone();
        }

        /**
         * Remove the specified host from the allowed host set.
         *
         * @param ref    A reference to the target MAC mapping.
         * @param mvlan  A {@link MacVlan} instance.
         * @throws VTNException  An error occurred.
         */
        private void removeAllowed(MapReference ref, MacVlan mvlan)
            throws VTNException {
            if (macMapAllowed.remove(mvlan) == null) {
                StringBuilder bulder =
                    new StringBuilder("Trying to unmap unexpected host: ");
                mvlan.appendContents(bulder);
                throw new VTNException(StatusCode.NOTFOUND, bulder.toString());
            }

            addUnmappedHost(ref, mvlan);
            MacMapState mst = mapState;
            if (mst != null) {
                if (mvlan.getMacAddress() == MacVlan.UNDEFINED) {
                    // MAC mapping mapped by wildcard map will be invalidated
                    // by collectUnmapped().
                    unmappedVlans.add(Short.valueOf(mvlan.getVlan()));
                } else {
                    inactivate(mst, ref, mvlan);
                }
            }
        }

        /**
         * Add the specified host to the allowed host set.
         *
         * @param ref     A reference to the target MAC mapping.
         * @param mvlan   A {@link MacVlan} instance.
         * @throws MacMapConflictException
         *    The specified host is already mapped by the specified
         *    MAC mapping.
         * @throws VTNException  An error occurred.
         */
        private void addAllowed(MapReference ref, MacVlan mvlan)
            throws VTNException {
            MapReference old = macMapAllowed.putIfAbsent(mvlan, ref);
            if (old != null) {
                if (old.equals(ref)) {
                    // Already added.
                    return;
                }
                throw new MacMapConflictException(mvlan, old);
            }

            if (alreadyMapped.contains(mvlan)) {
                // No need to purge network caches for this host because
                // this host is already mapped by this MAC mapping.
                return;
            }

            addMappedHost(ref, mvlan);
            if (mvlan.getMacAddress() == MacVlan.UNDEFINED) {
                return;
            }

            // Check to see if the specified host is mapped by wildcard
            // MAC mapping.
            short vlan = mvlan.getVlan();
            MacVlan key = new MacVlan(MacVlan.UNDEFINED, vlan);
            MapReference mref = macMapAllowed.get(key);
            if (mref != null) {
                inactivate(mref, mvlan);
            }
        }

        /**
         * Remove the specified host from the allowed host set.
         *
         * @param ref    A reference to the target MAC mapping.
         * @param mvlan  A {@link MacVlan} instance.
         * @throws VTNException  An error occurred.
         */
        private void removeDenied(MapReference ref, MacVlan mvlan)
            throws VTNException {
            Set<MapReference> set = removeDenied(mvlan);
            if (set == null || !set.remove(ref)) {
                StringBuilder bulder = new StringBuilder("Host(");
                mvlan.appendContents(bulder);
                bulder.append(") is not found in the denied set");
                throw new VTNException(StatusCode.NOTFOUND,
                                       bulder.toString());
            }
            if (!set.isEmpty()) {
                macMapDenied.put(mvlan, set);
            }

            // Check to see if the specified host is mapped to the target
            // MAC mapping.
            MapReference mref = macMapAllowed.get(mvlan);
            MacVlan key = new MacVlan(MacVlan.UNDEFINED, mvlan.getVlan());
            MapReference wcref = macMapAllowed.get(key);
            if (ref.equals(mref)) {
                // The specified host exists in the allowed host set.
                addMappedHost(ref, mvlan);
                if (wcref != null && !wcref.equals(ref)) {
                    // The specified host may be mapped by another MAC mapping.
                    // It must be inactivated here.
                    inactivate(wcref, mvlan);
                }
            } else if (mref == null && ref.equals(wcref)) {
                // The specified host will be mapped to the target MAC mapping
                // by wildcard MAC address entry.
                // Network caches for the host should be purged because it may
                // be mapped to another vBridge by VLAN mapping.
                addMappedHost(ref, mvlan);
            }
        }

        /**
         * Add the specified host to the denied host set.
         *
         * @param ref    A reference to the target MAC mapping.
         * @param mvlan  A {@link MacVlan} instance.
         * @throws VTNException  An error occurred.
         */
        private void addDenied(MapReference ref, MacVlan mvlan)
            throws VTNException {
            Set<MapReference> set = getDenied(mvlan);
            if (!set.add(ref)) {
                // Already added.
                return;
            }

            macMapDenied.put(mvlan, set);
            addUnmappedHost(ref, mvlan);
            MacMapState mst = mapState;
            if (mst != null) {
                inactivate(mst, ref, mvlan);
            }
        }

        /**
         * Fix up changes for MAC mappings configuration.
         *
         * @param ref  A reference to the target MAC mapping.
         * @throws VTNException  An error occurred.
         */
        private void fixUp(MapReference ref) throws VTNException {
            MacMapState mst = mapState;
            if (!(mst == null || unmappedVlans.isEmpty())) {
                // Inactivate MAC mappings which are no longer valid.
                Set<PortVlan> rels = new HashSet<PortVlan>();
                Map<MacVlan, NodeConnector> unmapped =
                    mst.inactivate(macMapAllowed, ref, unmappedVlans, rels);
                releasePort(rels, ref);
                if (logList != null) {
                    for (Map.Entry<MacVlan, NodeConnector> entry:
                             unmapped.entrySet()) {
                        MacVlan mvlan = entry.getKey();
                        NodeConnector port = entry.getValue();
                        MapLog mlog = new MacMapInactivatedLog(ref, mvlan,
                                                               port);
                        logList.add(mlog);
                    }
                }
            }

            // Update runtime state.
            if (!removing) {
                putMacMapState(ref, mst);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void cleanUpImpl(VTNManagerImpl mgr) {
            if (logList != null) {
                for (MapLog mlog: logList) {
                    mlog.log(LOG);
                }
            }
        }
    }

    /**
     * An instance of this class is used to inactivate MAC mappings related
     * to switch ports accepted by the specified port filter.
     */
    private final class MacMapPurgeResult {
        /**
         * Determine whether the target MAC mapping is still active or not.
         */
        private boolean  active;

        /**
         * A map which keeps unmapped MAC mappings.
         */
        private Map<MacVlan, NodeConnector>  unmapped;

        /**
         * Inactivate MAC mappings for hosts detected on the specified switch.
         *
         * @param ref     A reference to the target MAC mapping.
         * @param filter  A {@link PortFilter} instance which selects
         *                switch ports.
         * @return  {@code true} is returned if at least one MAC mapping was
         *          inactivated. Otherwise {@code false} is returned.
         * @throws VTNException  A fatal error occurred.
         */
        private boolean inactivate(MapReference ref, PortFilter filter)
            throws VTNException {
            MacMapState mst = getMacMapStateForUpdate(ref);
            if (mst == null) {
                return false;
            }

            Set<PortVlan> rels = new HashSet<PortVlan>();
            unmapped = mst.inactivate(filter, rels);
            releasePort(rels, ref);
            active = mst.hasMapping();

            return putMacMapState(ref, mst);
        }

        /**
         * Determine whether the target MAC mapping is still active or not.
         *
         * @return  {@code true} is returned if at least one host is still
         *          mapped by the MAC mapping.
         *          {@code false} is returned the specified MAC mapping is
         *          no longer active.
         */
        private boolean isActive() {
            return active;
        }

        /**
         * Clean up cluster cache transaction.
         *
         * @param ref   A reference to the target MAC mapping.
         */
        private void cleanUp(MapReference ref) {
            if (unmapped != null && LOG.isTraceEnabled()) {
                for (Map.Entry<MacVlan, NodeConnector> entry:
                         unmapped.entrySet()) {
                    MacVlan mvlan = entry.getKey();
                    NodeConnector port = entry.getValue();
                    MapLog mlog = new MacMapInactivatedLog(ref, mvlan, port);
                    mlog.log(LOG);
                }
            }
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
        if (service != null && service.equals(clusterService)) {
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
            macMapAllowed = new ConcurrentHashMap<MacVlan, MapReference>();
            macMapDenied = new ConcurrentHashMap<MacVlan, Set<MapReference>>();
            macMapStates = new ConcurrentHashMap<MapReference, MacMapState>();

            // Use loopback address as controller's address.
            controllerAddress = InetAddress.getLoopbackAddress();
        } else {
            Set<IClusterServices.cacheMode> cmode =
                EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL);
            createCache(cluster, CACHE_CONFREVISION, cmode);
            createCache(cluster, CACHE_VLANMAP, cmode);
            createCache(cluster, CACHE_PORTMAP, cmode);
            createCache(cluster, CACHE_MACMAP, cmode);
            createCache(cluster, CACHE_MACMAP_DENY, cmode);
            createCache(cluster, CACHE_MACMAP_STATE, cmode);

            configRevision = (ConcurrentMap<MapType, Integer>)
                getCache(cluster, CACHE_CONFREVISION);
            vlanMaps = (ConcurrentMap<NodeVlan, MapReference>)
                getCache(cluster, CACHE_VLANMAP);
            portMaps = (ConcurrentMap<PortVlan, MapReference>)
                getCache(cluster, CACHE_PORTMAP);
            macMapAllowed = (ConcurrentMap<MacVlan, MapReference>)
                getCache(cluster, CACHE_MACMAP);
            macMapDenied = (ConcurrentMap<MacVlan, Set<MapReference>>)
                getCache(cluster, CACHE_MACMAP_DENY);
            macMapStates = (ConcurrentMap<MapReference, MacMapState>)
                getCache(cluster, CACHE_MACMAP_STATE);

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
            LOG.warn("Cache already exists: name=" + cacheName, e);
        } catch (CacheConfigException e) {
            String msg = "Invalid cache configuration: name=" + cacheName +
                ", mode=" + cmode;
            LOG.error(msg, e);
        } catch (Exception e) {
            String msg = "Failed to create cache: name=" + cacheName;
            LOG.error(msg, e);
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
     * Release the specified VLAN network on a switch port reserved by
     * the MAC mapping.
     *
     * @param pvlan  A {@link PortVlan} instance which represents a VLAN
     *               network on a specific switch port.
     * @param ref    A reference to the MAC mapping.
     * @throws VTNException  An error occurred.
     */
    private void releasePort(PortVlan pvlan, MapReference ref)
        throws VTNException {
        if (!portMaps.remove(pvlan, ref)) {
            StringBuilder builder = new StringBuilder(
                "MAC mapping did not reserve port: map=");
            builder.append(ref.toString()).
                append(", port=").append(pvlan.toString());
            throw new VTNException(StatusCode.INTERNALERROR,
                                   builder.toString());
        }
    }

    /**
     * Release the specified VLAN network on a switch port reserved by
     * the MAC mapping.
     *
     * @param rels  A set of {@link PortVlan} instances corresponding to
     *              VLAN networks on switch port.
     * @param ref   A reference to the MAC mapping.
     * @throws VTNException  An error occurred.
     */
    private void releasePort(Set<PortVlan> rels, MapReference ref)
        throws VTNException {
        for (PortVlan pvlan: rels) {
            releasePort(pvlan, ref);
        }
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
        VNodePath path = ref.getPath();
        PathMapCleaner cleaner = new PathMapCleaner(path);
        cleaner.purge(mgr, null);
    }

    /**
     * Purge network caches originated by the specified port mapping.
     *
     * @param mgr    VTN Manager service.
     * @param path   A path to the virtual interface which maps the specified
     *               network.
     * @param pvlan  A {@link PortVlan} instance which specifies the
     *               VLAN network on a switch port.
     */
    private void purge(VTNManagerImpl mgr, VInterfacePath path,
                       PortVlan pvlan) {
        NodeConnector port = pvlan.getNodeConnector();
        short vlan = pvlan.getVlan();

        if (path instanceof VBridgePath) {
            // Remove MAC addresses detected on the specified port.
            MacAddressTable table = mgr.getMacAddressTable((VBridgePath)path);
            if (table != null) {
                table.flush(port, vlan);
            }
        }

        // Remove flow entries previously mapped by the specified port mapping.
        String tenantName = path.getTenantName();
        VTNThreadData.removeFlows(mgr, tenantName, port, vlan);
    }

    /**
     * Return runtime state of the specified MAC mapping to be updated.
     *
     * @param ref  A reference to the MAC mapping.
     * @return  A {@link MacMapState} instance if found.
     *          {@code null} if not found.
     */
    private MacMapState getMacMapStateForUpdate(MapReference ref) {
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? null : mst.clone();
    }

    /**
     * Return runtime state of the specified MAC mapping to be updated.
     *
     * <p>
     *   Unlike {@link #getMacMapStateForUpdate(MapReference)}, this method
     *   creates a new {@link MacMapState} instance if the runtime state of
     *   the specified MAC mapping is not found.
     * </p>
     *
     * @param ref  A reference to the MAC mapping.
     * @return  A {@link MacMapState} instance.
     */
    private MacMapState createMacMapState(MapReference ref) {
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? new MacMapState() : mst.clone();
    }

    /**
     * Update runtime state of the specified MAC mapping.
     *
     * @param ref  A reference to the MAC mapping.
     * @param mst  A {@link MacMapState} instance associated with the specified
     *             MAC mapping.
     * @return  {@code true} is returned if the state was actually updated.
     *          Otherwise {@code false} is returned.
     */
    private boolean putMacMapState(MapReference ref, MacMapState mst) {
        boolean changed = mst.isDirty();
        if (changed) {
            macMapStates.put(ref, mst);
        }

        return changed;
    }

    /**
     * Change MAC mapping configuration.
     *
     * @param ref     A reference to the target MAC mapping.
     * @param change  A {@link MacMapChange} instance which keeps differences
     *                to be applied.
     * @return  A {@link MacMapResult} instance.
     * @throws MacMapConflictException
     *   At least one host configured in {@code allowAdded} is already mapped
     *   to vBridge by MAC mapping.
     * @throws VTNException  A fatal error occurred.
     */
    private MacMapResult changeMacMap(MapReference ref, MacMapChange change)
        throws VTNException {
        MacMapResult result = new MacMapResult(ref, change);

        // Remove hosts from the allowed host set.
        for (MacVlan mvlan: change.getAllowRemovedSet()) {
            result.removeAllowed(ref, mvlan);
        }

        // Add hosts to the allowed host set.
        for (MacVlan mvlan: change.getAllowAddedSet()) {
            result.addAllowed(ref, mvlan);
        }

        // Remove hosts from the denied host set.
        for (MacVlan mvlan: change.getDenyRemovedSet()) {
            result.removeDenied(ref, mvlan);
        }

        // Append hosts to the denied host set.
        for (MacVlan mvlan: change.getDenyAddedSet()) {
            result.addDenied(ref, mvlan);
        }

        // Fix up change of MAC mapping configuration.
        result.fixUp(ref);

        return result;
    }

    /**
     * Activate the specified host in the MAC mapping.
     *
     * @param ref    A reference to the MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port where the specified host is detected.
     * @return  A {@link MacMapActivation} instance is returned if the host
     *          was actually activated.
     *          {@code null} is returned if the MAC mapping for the specified
     *          host is already activated.
     * @throws MacMapGoneException
     *    The specified host is no longer mapped by the target MAC mapping.
     * @throws MacMapPortBusyException
     *    The specified VLAN network on a switch port is reserved by
     *    another virtual mapping.
     * @throws MacMapDuplicateException
     *    The same MAC address as {@code mvlan} is already mapped to the
     *    same vBridge.
     * @throws VTNException  A fatal error occurred.
     */
    private MacMapActivation activateMacMapImpl(MapReference ref,
                                                MacVlan mvlan,
                                                NodeConnector port)
        throws VTNException {
        // Verify the current MAC mapping configuration.
        MacVlan key = getMacMapKey(ref, mvlan);
        if (key == null) {
            throw new MacMapGoneException(mvlan, ref);
        }

        // Reserve the VLAN network on the specified switch port.
        short vlan = mvlan.getVlan();
        PortVlan pvlan = new PortVlan(port, vlan);
        MapReference old = portMaps.putIfAbsent(pvlan, ref);
        MapReference vlanMap = null;
        if (old != null) {
            if (!old.equals(ref)) {
                throw new MacMapPortBusyException(mvlan, ref, old);
            }
        } else {
            // We need to purge obsolete network caches originated by
            // VLAN mapping.
            vlanMap = getVlanMapReference(port, vlan);
        }

        // Activate the MAC mapping.
        MacMapState mst = createMacMapState(ref);
        MacMapActivation result = mst.activate(ref, mvlan, port);
        if (result != null) {
            PortVlan released = result.getReleasedNetwork();
            if (released != null) {
                // Release the switch port which is no longer used by
                // this MAC mapping.
                releasePort(released, ref);
            }
            if (vlanMap != null) {
                // Need to purge network caches superseded by the MAC mapping.
                result.setObsoleteVlanMap(vlanMap, pvlan);
            }
            putMacMapState(ref, mst);
        } else if (old == null) {
            // This should never happen.
            StringBuilder builder = new StringBuilder(
                "Port is reserved by MAC mapping unexpectedly: map=");
            builder.append(ref.toString()).append(", host={");
            mvlan.appendContents(builder);
            builder.append("}, pvlan=").append(pvlan.toString());
            throw new VTNException(StatusCode.INTERNALERROR,
                                   builder.toString());
        }

        return result;
    }

    /**
     * Return a reference to virtual network mapping which maps the specified
     * host by MAC mapping.
     *
     * <p>
     *   This method only sees the MAC mapping configuration.
     *   It never checks whether the MAC mapping is actually activated or not.
     * </p>
     *
     * @param mvlan  A {@link MacVlan} instance which specifies L2 host.
     * @return       A {@link MapReference} object is returned if found.
     *               {@code null} is returned if not found.
     */
    private MapReference getMacMapReference(MacVlan mvlan) {
        MapReference ref = macMapAllowed.get(mvlan);
        if (ref == null) {
            // Check to see if the specified VLAN is mapped by MAC mapping.
            MacVlan key = new MacVlan(MacVlan.UNDEFINED, mvlan.getVlan());
            ref = macMapAllowed.get(key);
            if (ref == null) {
                return null;
            }
        }

        Set<MapReference> deniedSet = macMapDenied.get(mvlan);
        if (deniedSet != null && deniedSet.contains(ref)) {
            // This MAC address is denied by configuration.
            return null;
        }

        return ref;
    }

    /**
     * Return the key of {@link #macMapAllowed} which maps the specified
     * host.
     *
     * @param mref   A reference to the MAC mapping.
     * @param mvlan  A {@link MacVlan} instance corresponding to the host
     *               to be mapped by the MAC mapping.
     * @return  If the specified host is mapped by the specified MAC mapping,
     *          a {@link MacVlan} instance which maps the host is returned.
     *          {@code null} is returned if the specified host is not mapped
     *          by the specified MAC mapping.
     */
    private MacVlan getMacMapKey(MapReference mref, MacVlan mvlan) {
        MacVlan key = mvlan;
        MapReference ref = macMapAllowed.get(key);
        if (ref == null) {
            // Check to see if the specified VLAN is mapped by MAC mapping.
            key = new MacVlan(MacVlan.UNDEFINED, mvlan.getVlan());
            ref = macMapAllowed.get(key);
            if (ref == null) {
                return null;
            }
        }

        Set<MapReference> deniedSet = macMapDenied.get(mvlan);
        if (deniedSet != null && deniedSet.contains(ref)) {
            // This MAC address is denied by configuration.
            return null;
        }

        return (mref.equals(ref)) ? key : null;
    }

    /**
     * Return a reference to the VLAN mapping which maps the specified VLAN
     * network on a switch port.
     *
     * @param port  A {@link NodeConnector} instance corresponding to a
     *              switch port.
     * @param vlan  A VLAN ID.
     * @return  A reference to the VLAN mapping if found.
     *          {@code null} if not found.
     */
    private MapReference getVlanMapReference(NodeConnector port, short vlan) {
        // Note that we must examine VLAN mapping with a specific node first.
        NodeVlan nvlan = new NodeVlan(port.getNode(), vlan);
        MapReference ref = vlanMaps.get(nvlan);
        if (ref == null) {
            // Check the VLAN mapping which maps all switches.
            nvlan = new NodeVlan(null, vlan);
            ref = vlanMaps.get(nvlan);
        }

        return ref;
    }

    /**
     * Return a reference to virtual network mapping which maps the specified
     * host.
     *
     * @param mac   A byte array which represents the MAC address.
     * @param nc    A node connector corresponding to the switch port.
     *              Specifying {@code null} results in undefined behavior.
     * @param vlan  A VLAN ID.
     * @param logs  A list of {@link MapLog} to store trace log records.
     *              No trace log is recorded if {@code null} is specified.
     * @return      A {@link MapReference} object is returned if found.
     *              {@code null} is returned if not found.
     */
    private MapReference getMapReferenceImpl(byte[] mac, NodeConnector nc,
                                             short vlan, List<MapLog> logs) {
        // Examine port mapping at first.
        PortVlan pvlan = new PortVlan(nc, vlan);
        MapReference pref = portMaps.get(pvlan);
        if (pref != null && pref.getMapType() == MapType.PORT) {
            return pref;
        }

        // Examine MAC mapping.
        MacVlan mvlan = new MacVlan(mac, vlan);
        MapReference ref = getMacMapReference(mvlan);
        if (ref != null && checkMacMapping(ref, mvlan, nc, pref, logs)) {
            return ref;
        }

        // If the incoming VLAN network is reserved by port or MAC mapping,
        // VLAN mapping should ignore all incoming packets from the VLAN
        // network.
        if (pref != null) {
            return null;
        }

        // Examine VLAN mapping.
        ref = getVlanMapReference(nc, vlan);

        assert ref == null || ref.getMapType() == MapType.VLAN;
        return ref;
    }

    /**
     * Determine whether the MAC mapping for the specified host is available
     * or not.
     *
     * @param ref    A reference to the target MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which specifies the host
     *               to be mapped.
     * @param port   A {@link NodeConnector} instance corresponding to a switch
     *               port where the host was detected.
     * @param pref   A reference to the virtual mapping which reserves the
     *               specified VLAN network on a switch port.
     * @param logs  A list of {@link MapLog} to store trace log records.
     *              No trace log is recorded if {@code null} is specified.
     * @return  {@code true} is returned only if the specified host can be
     *          mapped by the specified MAC mapping.
     */
    private boolean checkMacMapping(MapReference ref, MacVlan mvlan,
                                    NodeConnector port, MapReference pref,
                                    List<MapLog> logs) {
        // Ensure that the specified VLAN network on a switch port is not
        // reserved by another virtual mapping.
        if (pref != null && !pref.equals(ref)) {
            if (logs != null) {
                MapLog mlog = new MacMapPortBusyLog(ref, mvlan, port, pref);
                logs.add(mlog);
            }
            return false;
        }

        // Ensure that the specified MAC address on another VLAN network is
        // not mapped to the same vBridge.
        MacMapState mst = macMapStates.get(ref);
        if (mst != null) {
            MacVlan dup = mst.getDuplicate(mvlan);
            if (dup != null) {
                if (logs != null) {
                    MapLog mlog = new MacMapDuplicateLog(ref, mvlan, dup);
                    logs.add(mlog);
                }
                return false;
            }
        }

        return true;
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
        ConfigTrans<VlanMapResult> xact = new ConfigTrans<VlanMapResult>(
            mgr.getVTNConfig()) {
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
        ConfigTrans<Object> xact = new ConfigTrans<Object>(mgr.getVTNConfig()) {
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
    public MapReference registerPortMap(VTNManagerImpl mgr,
                                        VInterfacePath path,
                                        final PortVlan pvlan,
                                        final PortVlan rmlan,
                                        final boolean purge)
        throws VTNException {
        if (pvlan == null && rmlan == null) {
            return null;
        }

        // Change port mappings in a cluster cache transaction.
        final MapReference ref = mgr.getMapReference(path);
        ConfigTrans<PortMapResult> xact = new ConfigTrans<PortMapResult>(
            mgr.getVTNConfig()) {
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
    public void unregisterPortMap(VTNManagerImpl mgr, VInterfacePath path,
                                  PortVlan pvlan, boolean purge)
        throws VTNException {
        registerPortMap(mgr, path, null, pvlan, purge);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void registerMacMap(VTNManagerImpl mgr, MacMapPath path,
                               final MacMapChange change)
        throws VTNException {
        // Change MAC mapping configuration in a cluster cache transaction.
        final MapReference ref = mgr.getMapReference(path);
        ConfigTrans<MacMapResult> xact = new ConfigTrans<MacMapResult>(
            mgr.getVTNConfig()) {
            @Override
            protected MacMapResult update() throws VTNException {
                MacMapResult r = changeMacMap(ref, change);
                setChanged();
                return r;
            }
        };

        MacMapResult result = xact.execute();
        result.cleanUp(mgr, path.getTenantName());
        if (change.isRemoving() && !change.dontPurge()) {
            purge(mgr, ref);
        }
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
    public  MapReference getMapReference(byte[] mac, NodeConnector nc,
                                         short vlan) {
        Integer current;
        MapReference ref;
        List<MapLog> logList = (LOG.isTraceEnabled())
            ? new ArrayList<MapLog>() : null;

        do {
            // Read current revision.
            current = getConfigRevision();

            // Search for a virtual mapping which maps the given network.
            if (logList != null) {
                logList.clear();
            }
            ref = getMapReferenceImpl(mac, nc, vlan, logList);

            // Ensure that the configuration is consistent.
        } while (!current.equals(getConfigRevision()));

        if (logList != null) {
            for (MapLog mlog: logList) {
                mlog.log(LOG);
            }
        }

        return ref;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MapReference getMapReference(PortVlan pvlan) {
        return portMaps.get(pvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MapReference getMapReference(MacVlan mvlan) {
        Integer current;
        MapReference ref;

        do {
            // Read current revision.
            current = getConfigRevision();

            // Search for a MAC mapping which maps the specified host.
            ref = getMacMapReference(mvlan);

            // Ensure that the configuration is consistent.
        } while (!current.equals(getConfigRevision()));

        return ref;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<PortVlan> getMacMappedNetworks(VTNManagerImpl mgr,
                                              MacMapPath path) {
        MapReference ref = mgr.getMapReference(path);
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? null : mst.getNetworks();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public PortVlan getMacMappedNetwork(VTNManagerImpl mgr, MacMapPath path,
                                        long mac) {
        MapReference ref = mgr.getMapReference(path);
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? null : mst.getPortVlan(mac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Map<MacVlan, NodeConnector> getMacMappedHosts(VTNManagerImpl mgr,
                                                         MacMapPath path) {
        MapReference ref = mgr.getMapReference(path);
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? null : mst.getActiveHosts();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public NodeConnector getMacMappedPort(VTNManagerImpl mgr, MacMapPath path,
                                          MacVlan mvlan) {
        MapReference ref = mgr.getMapReference(path);
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? null : mst.getPort(mvlan);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean hasMacMappedHost(VTNManagerImpl mgr, MacMapPath path) {
        MapReference ref = mgr.getMapReference(path);
        MacMapState mst = macMapStates.get(ref);
        return (mst == null) ? false : mst.hasMapping();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean activateMacMap(VTNManagerImpl mgr, MacMapPath path,
                                  final MacVlan mvlan,
                                  final NodeConnector port)
        throws VTNException {
        // Activate MAC mapping in a cluster cache transaction.
        final MapReference ref = mgr.getMapReference(path);
        ConfigTrans<MacMapActivation> xact = new ConfigTrans<MacMapActivation>(
            mgr.getVTNConfig()) {
            @Override
            protected MacMapActivation update() throws VTNException {
                MacMapActivation r = activateMacMapImpl(ref, mvlan, port);
                if (r != null) {
                    setChanged();
                }
                return r;
            }
        };

        MacMapActivation result = xact.execute();
        if (result == null) {
            // The specified host on the port is already activated.
            return false;
        }

        result.cleanUp(LOG, mgr, ref, mvlan, port);

        return result.isActivated();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean inactivateMacMap(VTNManagerImpl mgr, MacMapPath path,
                                    final PortFilter filter)
        throws VTNException {
        // Inactivate MAC mappings in a cluster cache transaction.
        final MapReference ref = mgr.getMapReference(path);
        ConfigTrans<MacMapPurgeResult> x = new ConfigTrans<MacMapPurgeResult>(
            mgr.getVTNConfig()) {
            @Override
            protected MacMapPurgeResult update() throws VTNException {
                MacMapPurgeResult r = new MacMapPurgeResult();
                if (r.inactivate(ref, filter)) {
                    setChanged();
                }
                return r;
            }
        };

        MacMapPurgeResult result = x.execute();
        result.cleanUp(ref);

        return result.isActive();
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
    public VTNManagerImpl getVTNManager(String containerName) {
        VTNManagerImpl mgr = vtnManagers.get(containerName);
        if (mgr == null) {
            // This should never happen.
            LOG.warn("{}: VTN Manager service was not found.", containerName);
        }
        return mgr;
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
