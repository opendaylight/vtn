/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.apache.commons.lang3.tuple.ImmutablePair;
import org.apache.commons.lang3.tuple.Pair;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgePortFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.inventory.port.PortFilter;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.PortVlanMacFilter;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappings;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.map.reference.list.MapReferenceList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.map.reference.list.MapReferenceListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapAllowed;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapAllowedKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapDenied;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapDeniedKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.PortMapping;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.PortMappingKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.VlanMapping;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.VlanMappingKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHostKey;

/**
 * A collection of utility class methods for virtual mapping management.
 */
public final class MappingRegistry {
    /**
     * A logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(MappingRegistry.class);

    /**
     * Private constructor that protects this class from instantiating.
     */
    private MappingRegistry() {}

    /**
     * Register mapping between vBridge and VLAN.
     *
     * <ul>
     *   <li>
     *     If {@code nvlan} specifies a VLAN network on a specific node and
     *     {@code true} is specified to {@code purge}, this method purges
     *     network caches corresponding to the network superseded by a new
     *     VLAN mapping.
     *   </li>
     * </ul>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param mapId  The identifier for the VLAN mapping.
     * @param nvlan  A {@link NodeVlan} instance that specifies the VLAN
     *               to be mapped.
     * @param purge  If {@code true}, this method purges obsolete network
     *               caches as appropriate.
     * @throws VTNException  An error occurred.
     */
    public static void registerVlanMap(
        TxContext ctx, VlanMapIdentifier mapId, NodeVlan nvlan, boolean purge)
        throws VTNException {
        VlanMapRegistry reg = new VlanMapRegistry(ctx, mapId);
        reg.register(nvlan, purge);
    }

    /**
     * Unregister the VLAN mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param mapId  The identifier for the VLAN mapping to be unregistered.
     * @param nvlan  A {@link NodeVlan} object which specifies the VLAN
     *               to be unmapped.
     * @param purge  If {@code true} is specified, this method purges caches
     *               corresponding to the unmapped VLAN.
     * @throws VTNException  A fatal error occurred.
     */
    public static void unregisterVlanMap(
        TxContext ctx, VlanMapIdentifier mapId, NodeVlan nvlan, boolean purge)
        throws VTNException {
        // Remove VLAN mapping.
        InstanceIdentifier<VlanMapping> path = getPath(nvlan);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        VlanMapping vmap = DataStoreUtils.delete(tx, path);
        if (vmap == null || !mapId.toString().equals(vmap.getMapping())) {
            // This should never happen.
            String msg = mapId.toString() +
                ": Attempting to unmap unexpected VLAN: nvlan=" +
                nvlan + ", vmap=" + vmap;
            throw new RpcException(msg);
        }

        if (purge) {
            purge(ctx, mapId);
        }
    }

    /**
     * Register mapping between physical switch port and virtual interface.
     *
     * <ul>
     *   <li>
     *     If a non-{@code null} value is specified to {@code oldPv},
     *     the port mapping that maps the VLAN specified by {@code oldPv} is
     *     removed in the same transaction. In this case the VLAN specified by
     *     {@code oldPv} must be currently mapped to the virtual interface
     *     specified by {@code ifId}.
     *   </li>
     *   <li>
     *     If {@code true} is specified to {@code purge}, this method purges
     *     network caches corresponding to obsolete mapping.
     *   </li>
     * </ul>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param ifId   The identifier for the virtual interface which maps the
     *               specified VLAN.
     * @param newPv  A pair of physical switch port and VLAN ID to be mapped.
     *               No port mapping is registered if {@code null} is
     *               specified.
     * @param oldPv  A pair of physical switch port and VLAN ID to be unmapped.
     *               No port mapping is removed if {@code null} is specified.
     * @param purge  If {@code true}, this method purges obsolete network
     *               caches as appropriate.
     * @return  {@code true} if the port mapping was changed.
     *          {@code false} if not changed.
     * @throws VTNException  An error occurred.
     */
    public static boolean registerPortMap(
        TxContext ctx, VInterfaceIdentifier<?> ifId, PortVlan newPv,
        PortVlan oldPv, boolean purge) throws VTNException {
        boolean changed = !Objects.equals(newPv, oldPv);
        if (changed) {
            if (oldPv != null) {
                // Remove the port mapping that maps oldPv.
                InstanceIdentifier<PortMapping> path = getPath(oldPv);
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                PortMapping pmap = DataStoreUtils.delete(tx, path);
                if (pmap == null ||
                    !ifId.toString().equals(pmap.getMapping())) {
                    // This should never happen.
                    String msg = ifId.toString() +
                        ": Attempting to unmap unexpected port mapping: " +
                        "pvlan=" + oldPv + ", pmap=" + pmap;
                    throw new RpcException(msg);
                }

                purge(ctx, ifId, oldPv);
            }

            if (newPv != null) {
                // Register new port mapping.
                PortMapRegistry reg = new PortMapRegistry(ctx, ifId);
                reg.register(newPv, purge);
            }
        }

        return changed;
    }

    /**
     * Register MAC mapping configuration.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param mapId   The identifier for the MAC mapping.
     * @param change  A {@link MacMapChange} instance which keeps differences
     *                to be applied.
     * @throws org.opendaylight.vtn.manager.internal.util.vnode.MacMapConflictException
     *    At least one host configured in the allowed host set is already
     *    mapped to vBridge by MAC mapping.
     * @throws VTNException  An error occurred.
     */
    public static void registerMacMap(TxContext ctx, MacMapIdentifier mapId,
                                      MacMapChange change)
        throws VTNException {
        MacMapRegistry reg = new MacMapRegistry(ctx, mapId, change);
        reg.apply();

        if (change.isRemoving() && !change.dontPurge()) {
            purge(ctx, mapId);
        }
    }

    /**
     * Determine whether the given pair of switch port and VLAN ID is mapped
     * to the virtual interface or not.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param pv   A pair of the switch port and VLAN ID.
     * @return  {@code true} if the given port is mapped to the virtual
     *          interface. {@code false} otherwise.
     * @throws VTNException  An error occurred.
     */
    public static boolean isPortMapped(TxContext ctx, PortVlan pv)
        throws VTNException {
        return read(ctx.getReadWriteTransaction(), pv).isPresent();
    }

    /**
     * Determine whether the given VLAN on a switch is mapped by VLAN mapping
     * or not.
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param nv   A pair of the switch and VLAN ID.
     * @return  {@code true} if the given port is mapped to the virtual
     *          interface. {@code false} otherwise.
     * @throws VTNException  An error occurred.
     */
    public static boolean isVlanMapped(TxContext ctx, NodeVlan nv)
        throws VTNException {
        return read(ctx.getReadWriteTransaction(), nv).isPresent();
    }

    /**
     * Return the identifier for the virtual network mapping which maps the
     * specified host.
     *
     * <p>
     *   This method checks port mapping, MAC mapping, and VLAN mapping
     *   configurations in order, and returns the identifier for the virtual
     *   mapping found first.
     * </p>
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param eaddr  An {@link EtherAddress} instance that specifies the host.
     *               Specifying {@code null} results in undefined behavior.
     * @param sport  A {@link SalPort} instance corresponding to the switch
     *               port where the specified host was detected.
     *               Specifying {@code null} results in undefined behavior.
     * @param vid    A VLAN ID.
     * @return       A {@link TenantNodeIdentifier} instance that specifies
     *               the virtual mapping if found.
     *               {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public static TenantNodeIdentifier<?, ?> getMapping(
        TxContext ctx, EtherAddress eaddr, SalPort sport, int vid)
        throws VTNException {
        // Examine port mapping at first.
        PortVlan pv = new PortVlan(sport, vid);
        ReadTransaction rtx = ctx.getTransaction();
        Optional<PortMapping> popt = read(rtx, pv);
        TenantNodeIdentifier<?, ?> pident;
        if (popt.isPresent()) {
            pident = VNodeIdentifier.create(popt.get().getMapping(),
                                            TenantNodeIdentifier.class);
            if (pident.getType().isInterface()) {
                return pident;
            }
        } else {
            pident = null;
        }

        // Examine MAC mapping.
        MacVlan mv = new MacVlan(eaddr.getAddress(), vid);
        MacMapIdentifier mapId = getMacMapping(rtx, mv);
        if (mapId != null && checkMacMapping(ctx, mapId, mv, sport, pident)) {
            return mapId;
        }

        // Examine VLAN mapping.
        // If the incoming VLAN is reserved by port or MAC mapping,
        // VLAN mapping should ignore all the incoming packets from the
        // reserved VLAN.
        return (pident == null)
            ? getVlanMapping(rtx, sport, vid)
            : null;
    }

    /**
     * Return the identifier for the virtual network mapping which reserves the
     * specified VLAN on a swtich port.
     *
     * <p>
     *   This method searches for a port or MAC mapping which reserves the
     *   specified VLAN.
     * </p>
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param pv   A {@link PortVlan} instance which represents a VLAN on a
     *             specific switch port.
     * @return       A {@link VNodeIdentifier} instance that specifies the
     *               virtual mapping if found.
     *               {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public static VNodeIdentifier<?> getMapping(TxContext ctx, PortVlan pv)
        throws VTNException {
        ReadTransaction rtx = ctx.getTransaction();
        Optional<PortMapping> opt = read(rtx, pv);
        return (opt.isPresent())
            ? VNodeIdentifier.create(opt.get().getMapping())
            : null;
    }

    /**
     * Return the identifier for the MAC mapping which maps the specified host.
     *
     * <p>
     *   Note that this method only sees the MAC mapping configuration.
     *   It never checks whether the MAC mapping is actually activated or not.
     * </p>
     *
     * @param ctx  MD-SAL datastore transaction context.
     * @param mv   A {@link MacVlan} instance which specifies the L2 host.
     * @return  A {@link MacMapIdentifier} instance that maps the the specified
     *          host if found. {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public static MacMapIdentifier getMapping(TxContext ctx, MacVlan mv)
        throws VTNException {
        return getMacMapping(ctx.getTransaction(), mv);
    }

    /**
     * Return an entry of the mac-map-allowed list that maps the specified host
     * by MAC mapping.
     *
     * <p>
     *   This method only sees the MAC mapping configuration.
     *   It never checks whether the MAC mapping is actually activated or not.
     * </p>
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @param mv   A {@link MacVlan} instance which specifies the L2 host.
     * @return  A pair of {@link MacVlan} instance and a string representation
     *          of {@link MacMapIdentifier} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public static Pair<MacVlan, String> getMacMapAllowed(
        ReadTransaction rtx, MacVlan mv) throws VTNException {
        MacVlan key = mv;
        Optional<MacMapAllowed> aopt = readAllowed(rtx, key);
        if (!aopt.isPresent()) {
            // Check to see if the specified VLAN is mapped by MAC mapping.
            key = new MacVlan(MacVlan.UNDEFINED, mv.getVlanId());
            aopt = readAllowed(rtx, key);
            if (!aopt.isPresent()) {
                return null;
            }
        }

        // Check to see if the specified host is denied by configuration.
        String id = aopt.get().getMapping();
        Optional<MapReferenceList> dopt = readDenied(rtx, mv, id);
        return (dopt.isPresent())
            ? null : new ImmutablePair<MacVlan, String>(key, id);
    }

    /**
     * Return the identifier for the VLAN mapping which maps the specified
     * VLAN on a switch port.
     *
     * @param rtx    A read-only transaction for the MD-SAL datastore.
     * @param sport  A {@link SalPort} instance corresponding to a switch port.
     * @param vid    A VLAN ID.
     * @return  A {@link VlanMapIdentifier} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public static VlanMapIdentifier getVlanMapping(
        ReadTransaction rtx, SalPort sport, int vid) throws VTNException {
        // Note that we must examine VLAN mapping with a specific node first.
        NodeVlan nv = new NodeVlan(sport.getSalNode(), vid);
        Optional<VlanMapping> opt = read(rtx, nv);
        if (!opt.isPresent()) {
            // Check the VLAN mapping which maps all switches.
            nv = new NodeVlan(null, vid);
            opt = read(rtx, nv);
            if (!opt.isPresent()) {
                return null;
            }
        }

        return VNodeIdentifier.create(opt.get().getMapping(),
                                      VlanMapIdentifier.class);
    }

    /**
     * Inactivate all the MAC mappings detected on switch ports specified by
     * the port filter.
     *
     * <p>
     *   Note that this method never purges network caches corresponding to
     *   invalidated mappings, such as flow entry.
     * </p>
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param mapId   The identifier for the target MAC mapping.
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @return  {@code true} is returned if at least one host is still mapped
     *          by the MAC mapping.
     *          {@code false} is returned the specified MAC mapping is
     *          no longer active.
     * @throws VTNException  A fatal error occurred.
     */
    public static boolean inactivateMacMap(
        TxContext ctx, MacMapIdentifier mapId, PortFilter filter)
        throws VTNException {
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = reader.get(mapId);
        Set<PortVlan> rels = new HashSet<>();
        Map<MacVlan, SalPort> unmapped = vmst.inactivate(filter, rels);
        releasePort(ctx, rels, mapId);

        if (LOG.isTraceEnabled()) {
            for (Entry<MacVlan, SalPort> entry: unmapped.entrySet()) {
                MacVlan mv = entry.getKey();
                SalPort sport = entry.getValue();
                logMacMapInactivated(mapId, mv, sport);
            }
        }

        return vmst.hasMapping();
    }

    /**
     * Return path to the port mapping associated with the given pair of
     * switch port and VLAN ID.
     *
     * @param pv   A pair of the switch port and VLAN ID.
     * @return  Path to the port mapping associated with the given pair of
     *          switch port and VLAN ID.
     */
    public static InstanceIdentifier<PortMapping> getPath(PortVlan pv) {
        return InstanceIdentifier.builder(VtnMappings.class).
            child(PortMapping.class, new PortMappingKey(pv.toString())).
            build();
    }

    /**
     * Return path to the VLAN mapping associated with the given pair of
     * switch and VLAN ID.
     *
     * @param nv   A pair of the switch and VLAN ID.
     * @return  Path to the VLAN mapping associated with the given pair of
     *          switch and VLAN ID.
     */
    public static InstanceIdentifier<VlanMapping> getPath(NodeVlan nv) {
        return InstanceIdentifier.builder(VtnMappings.class).
            child(VlanMapping.class, new VlanMappingKey(nv.toString())).
            build();
    }

    /**
     * Return path to the MAC mapping that maps the specified L2 host.
     *
     * @param mv  A {@link MacVlan} instance that specifies the L2 host.
     * @return  Path to the MAC mapping that maps the specified L2 host.
     */
    public static InstanceIdentifier<MacMapAllowed> getAllowedPath(MacVlan mv) {
        return InstanceIdentifier.builder(VtnMappings.class).
            child(MacMapAllowed.class, new MacMapAllowedKey(mv.toString())).
            build();
    }

    /**
     * Return path to the set of MAC mappings that denies the specified
     * L2 host.
     *
     * @param mv  A {@link MacVlan} instance that specifies the L2 host.
     * @return  Path to the set of MAC mappings that denies the specified
     *          L2 host.
     */
    public static InstanceIdentifier<MacMapDenied> getDeniedPath(MacVlan mv) {
        return InstanceIdentifier.builder(VtnMappings.class).
            child(MacMapDenied.class, new MacMapDeniedKey(mv.toString())).
            build();
    }

    /**
     * Return path to the MAC mapping that denies the specified L2 host.
     * L2 host.
     *
     * @param mv  A {@link MacVlan} instance that specifies the L2 host.
     * @param id  A string representation of the MAC mapping identifier.
     * @return  Path to the MAC mapping that denies the specified L2 host.
     */
    public static InstanceIdentifier<MapReferenceList> getDeniedPath(
        MacVlan mv, String id) {
        return InstanceIdentifier.builder(VtnMappings.class).
            child(MacMapDenied.class, new MacMapDeniedKey(mv.toString())).
            child(MapReferenceList.class, new MapReferenceListKey(id)).
            build();
    }

    /**
     * Read the port-mapping container associated with the given pair of
     * switch port and VLAN ID.
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @param pv   A pair of the switch port and VLAN ID.
     * @return  An {@link Optional} instance that contains the result of the
     *          read operation.
     * @throws VTNException  An error occurred.
     */
    public static Optional<PortMapping> read(ReadTransaction rtx, PortVlan pv)
        throws VTNException {
        return DataStoreUtils.read(rtx, getPath(pv));
    }

    /**
     * Read the vlan-mapping container associated with the given pair of
     * switch and VLAN ID.
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @param nv   A pair of the switch and VLAN ID.
     * @return  An {@link Optional} instance that contains the result of the
     *          read operation.
     * @throws VTNException  An error occurred.
     */
    public static Optional<VlanMapping> read(ReadTransaction rtx, NodeVlan nv)
        throws VTNException {
        return DataStoreUtils.read(rtx, getPath(nv));
    }

    /**
     * Read the mac-map-allowed container associated with the given L2 host.
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @param mv   A {@link MacVlan} instance that specifies the L2 host.
     * @return  An {@link Optional} instance that contains the result of the
     *          read operation.
     * @throws VTNException  An error occurred.
     */
    public static Optional<MacMapAllowed> readAllowed(
        ReadTransaction rtx, MacVlan mv) throws VTNException {
        return DataStoreUtils.read(rtx, getAllowedPath(mv));
    }

    /**
     * Read a set of L2 hosts present in the the mac-map-allowed container.
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @return  A set of {@link MacVlan} instances that indicates a set of
     *          keys present in the mac-map-allowed container.
     * @throws VTNException  An error occurred.
     */
    public static Set<MacVlan> readAllowedSet(ReadTransaction rtx)
        throws VTNException {
        InstanceIdentifier<VtnMappings> path =
            InstanceIdentifier.create(VtnMappings.class);
        Optional<VtnMappings> opt = DataStoreUtils.read(rtx, path);
        Set<MacVlan> set = null;
        if (opt.isPresent()) {
            List<MacMapAllowed> list = opt.get().getMacMapAllowed();
            if (list != null) {
                set = new HashSet<>();
                for (MacMapAllowed allowed: list) {
                    set.add(new MacVlan(allowed.getIdentifier()));
                }
            }
        }

        if (set == null) {
            set = Collections.<MacVlan>emptySet();
        }

        return set;
    }

    /**
     * Read the map-reference-list container that indicates the denied host
     * in the MAC mapping configuration.
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @param mv   A {@link MacVlan} instance that specifies the L2 host.
     * @param id   A string representation of the MAC mapping identifier.
     * @return  An {@link Optional} instance that contains the result of the
     *          read operation.
     * @throws VTNException  An error occurred.
     */
    public static Optional<MapReferenceList> readDenied(
        ReadTransaction rtx, MacVlan mv, String id) throws VTNException {
        return DataStoreUtils.read(rtx, getDeniedPath(mv, id));
    }

    /**
     * Release the specified VLAN on a switch port reserved by the MAC mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param pv     A {@link PortVlan} instance which represents a VLAN on a
     *               specific switch port.
     * @param ident  The identifier for the MAC mapping.
     * @throws VTNException  An error occurred.
     */
    public static void releasePort(TxContext ctx, PortVlan pv,
                                   MacMapIdentifier ident)
        throws VTNException {
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        InstanceIdentifier<PortMapping> path = getPath(pv);
        PortMapping old = DataStoreUtils.delete(tx, path);
        if (old == null || !ident.toString().equals(old.getMapping())) {
            // This should never happen.
            String msg = ident.toString() +
                ": MAC mapping did not reserve port: " + pv;
            throw new RpcException(msg);
        }
    }

    /**
     * Release the specified VLAN on a switch port reserved by the MAC mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param rels  A set of {@link PortVlan} instances corresponding to
     *              VLANs on switch port.
     * @param ident  The identifier for the MAC mapping.
     * @throws VTNException  An error occurred.
     */
    public static void releasePort(TxContext ctx, Set<PortVlan> rels,
                                   MacMapIdentifier ident)
        throws VTNException {
        for (PortVlan pv: rels) {
            releasePort(ctx, pv, ident);
        }
    }

    /**
     * Record a trace log that notifies the specified MAC mapping has been
     * inactivated.
     *
     * @param ident  The identifier for the MAC mapping.
     * @param mv     A {@link MacVlan} instance that specifies the unmapped
     *               L2 host.
     * @param sport  A {@link SalPort} instance that specifies a switch port.
     */
    public static void logMacMapInactivated(
        MacMapIdentifier ident, MacVlan mv, SalPort sport) {
        LOG.trace("{}: MAC mapping has been inactivated: host={}, port={}",
                  ident, mv, sport);
    }

    /**
     * Purge network caches originated by the specified virtual mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param ident  The identifier for the target virtual mapping.
     * @throws VTNException  An error occurred.
     */
    private static void purge(TxContext ctx, VNodeIdentifier<?> ident)
        throws VTNException {
        new VNodeMapCleaner(ident).purge(ctx, null);
    }

    /**
     * Purge network caches originated by the specified port mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param ifId   The identifier for the the virtual interface which maps
     *               the specified network.
     * @param pvlan  A {@link PortVlan} instance which specifies the VLAN
     *               mapped by the port mapping.
     * @throws VTNException  An error occurred.
     */
    private static void purge(TxContext ctx, VInterfaceIdentifier<?> ifId,
                              PortVlan pvlan) throws VTNException {
        SalPort sport = pvlan.getPort();
        int vid = pvlan.getVlanId();

        if (ifId.getType() == VNodeType.VBRIDGE_IF) {
            // Remove MAC addresses detected on the specified port.
            VBridgeIdentifier vbrId = new VBridgeIdentifier(
                ifId.getTenantName(), ifId.getBridgeName());
            PortVlanMacFilter filter = new PortVlanMacFilter(sport, vid);
            new MacEntryRemover(filter).scan(ctx, vbrId);
        }

        // Remove flow entries previously mapped by the specified port mapping.
        String tname = ifId.getTenantNameString();
        ctx.getSpecific(FlowRemoverQueue.class).
            enqueue(new EdgePortFlowRemover(tname, sport, vid));
    }

    /**
     * Return the identifier for the MAC mapping that maps the specified host.
     *
     * <p>
     *   This method only sees the MAC mapping configuration.
     *   It never checks whether the MAC mapping is actually activated or not.
     * </p>
     *
     * @param rtx  A read-only transaction for the MD-SAL datastore.
     * @param mv   A {@link MacVlan} instance which specifies the L2 host.
     * @return  A {@link MacMapIdentifier} instance if found.
     *          {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    private static MacMapIdentifier getMacMapping(
        ReadTransaction rtx, MacVlan mv) throws VTNException {
        Pair<MacVlan, String> allowed = getMacMapAllowed(rtx, mv);
        return (allowed == null)
            ? null
            : VNodeIdentifier.create(allowed.getRight(),
                                     MacMapIdentifier.class);
    }

    /**
     * Determine whether the MAC mapping for the specified host is available
     * or not.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param mapId   The identifier for the target MAC mapping.
     * @param mv      A {@link MacVlan} instance which specifies the host
     *                to be mapped.
     * @param sport   A {@link SalPort} instance corresponding to a switch port
     *                where the host was detected.
     * @param pident  A string representation of the virtual mapping which
     *                reserves the specified VLAN network on a switch port.
     * @return  {@code true} is returned only if the specified host can be
     *          mapped by the specified MAC mapping.
     * @throws VTNException  An error occurred.
     */
    private static boolean checkMacMapping(
        TxContext ctx, MacMapIdentifier mapId, MacVlan mv, SalPort sport,
        VNodeIdentifier<?> pident) throws VTNException {
        // Ensure that the specified VLAN network on a switch port is not
        // reserved by another virtual mapping.
        boolean valid = (pident == null || pident.equals(mapId));
        if (valid) {
            // Ensure that the specified MAC address on another VLAN network
            // is not mapped to the same vBridge.
            valid = checkDuplicateMap(ctx, mapId, mv);
        } else {
            LOG.trace("{}: MAC mapping is unavailable: switch port is " +
                      "reserved: host={}, port={}, map={}",
                      mapId, mv, sport, pident);
        }

        return valid;
    }

    /**
     * Ensure that the specified MAC address is not mapped by the specified
     * MAC mapping.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param mapId   The identifier for the target MAC mapping.
     * @param mv      A {@link MacVlan} instance which specifies the host
     *                to be mapped.
     * @return  {@code true} if the specified MAC address is not mapped.
     *          {@code false} otherwise.
     * @throws VTNException  An error occurred.
     */
    private static boolean checkDuplicateMap(
        TxContext ctx, MacMapIdentifier mapId, MacVlan mv)
        throws VTNException {
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = reader.getCached(mapId);
        MacVlan dup;
        if (vmst == null) {
            // Read status from the MD-SAL datastore.
            MappedHostKey key = new MappedHostKey(mv.getMacAddress());
            InstanceIdentifier<MappedHost> path = mapId.getStatusPath().
                child(MappedHost.class, key);
            ReadTransaction rtx = ctx.getTransaction();
            Optional<MappedHost> opt = DataStoreUtils.read(rtx, path);
            if (opt.isPresent()) {
                int vid = opt.get().getVlanId().getValue().intValue();
                dup = (vid == mv.getVlanId())
                    ? null
                    : new MacVlan(mv.getMacAddress(), vid);
            } else {
                dup = null;
            }
        } else {
            // Used cached status.
            dup = vmst.getDuplicate(mv);
        }

        boolean result = (dup == null);
        if (!result) {
            LOG.trace("{}: MAC mapping is unavailable: Same MAC address is " +
                      "already mapped: host={}, duplicate={}",
                      mapId, mv, dup);
        }

        return result;
    }
}
