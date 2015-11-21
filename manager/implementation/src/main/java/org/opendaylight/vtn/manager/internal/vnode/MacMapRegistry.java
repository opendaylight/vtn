/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.LOG;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.logMacMapInactivated;

import java.util.HashSet;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapChange;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapConflictException;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMapReferenceField;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.map.reference.list.MapReferenceList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.map.reference.list.MapReferenceListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapAllowed;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapAllowedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.MacMapDenied;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * {@code MacMapRegistry} describes a context for registering MAC mapping.
 */
public final class MacMapRegistry extends VirtualMapRegistry<MacMapCleaner> {
    /**
     * The identifier for the target MAC mapping.
     */
    private final MacMapIdentifier  targetId;

    /**
     * Changes to be applied to the MAC mapping configuration.
     */
    private final MacMapChange  configChange;

    /**
     * A set of {@link MacVlan} instances which represents hosts already
     * mapped by this MAC mapping.
     */
    private final Set<MacVlan>  alreadyMapped = new HashSet<>();

    /**
     * MAC mappin status reader.
     */
    private final MacMapStatusReader  statusReader;

    /**
     * Runtime status of the target MAC mapping.
     */
    private final VTNMacMapStatus  mapStatus;

    /**
     * A set of VLAN IDs corresponding to unmapped VLAN network.
     */
    private final Set<Integer>  unmappedVlans = new HashSet<>();

    /**
     * A set of vBridge identifiers to change their state to DOWN.
     */
    private final Set<VBridgeIdentifier>  downBridges = new HashSet<>();

    /**
     * Set {@code true} if the target MAC mapping is going to be removed.
     */
    private final boolean  removing;

    /**
     * A boolean value whicih indicates obsolete network caches need to be
     * purged or not.
     */
    private final boolean  doPurge;

    /**
     * Construct a new instance.
     *
     * @param ctx     A runtime context for transaction task.
     * @param ident   The identifier for the target MAC mapping.
     * @param change  A {@link MacMapChange} instance that contains changes
     *                to the MAC mapping configuration.
     * @throws VTNException  An error occurred.
     */
    public MacMapRegistry(TxContext ctx, MacMapIdentifier ident,
                          MacMapChange change) throws VTNException {
        super(ctx);
        targetId = ident;
        configChange = change;
        removing = change.isRemoving();
        doPurge = !(removing || change.dontPurge());

        // Set up cache for the MAC mapping status.
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        statusReader = reader;
        mapStatus = reader.get(ident);

        if (!removing) {
            // Determine hosts that are already mapped by this MAC mapping.
            for (MacVlan mv: change.getAllowAddedSet()) {
                if (mv.getAddress() != MacVlan.UNDEFINED) {
                    MacMapIdentifier id = MappingRegistry.getMapping(ctx, mv);
                    if (ident.equals(id)) {
                        alreadyMapped.add(mv);
                    }
                }
            }
        }
    }

    /**
     * Apply new configuration for the MAC mapping.
     *
     * @throws MacMapConflictException
     *    At least one host configured in the allowed host set is already
     *    mapped by the target MAC mapping.
     * @throws VTNException  An error occurred.
     */
    public void apply() throws VTNException {
        // Remove hosts from the allowed host set.
        MacMapChange change = configChange;
        for (MacVlan mv: change.getAllowRemovedSet()) {
            removeAllowed(mv);
        }

        // Add hosts to the allowed host set.
        for (MacVlan mv: change.getAllowAddedSet()) {
            addAllowed(mv);
        }

        // Remove hosts from the denied host set.
        for (MacVlan mv: change.getDenyRemovedSet()) {
            removeDenied(mv);
        }

        // Append hosts to the denied host set.
        for (MacVlan mv: change.getDenyAddedSet()) {
            addDenied(mv);
        }

        cleanUp();
    }

    /**
     * Return a {@link MacMapCleaner} instance.
     *
     * @param ident  The identifier for the MAC mapping.
     * @return  A {@link MacMapCleaner} instance to clean up obsolete
     *          network caches.
     */
    private MacMapCleaner getMapCleaner(MacMapIdentifier ident) {
        String tname = ident.getTenantNameString();
        MacMapCleaner cleaner = getMapCleaner(tname);
        if (cleaner == null) {
            cleaner = new MacMapCleaner(getContext(), targetId);
            addMapCleaner(tname, cleaner);
        }

        return cleaner;
    }

    /**
     * Create a {@link MacMapCleaner} instance which represents VLAN
     * superseded by a new MAC mapping.
     *
     * @param ident  The identifier for the MAC mapping.
     * @param mv     A {@link MacVlan} instance which specifies the mapped
     *               host.
     */
    private void addMappedHost(MacMapIdentifier ident, MacVlan mv) {
        if (doPurge) {
            getMapCleaner(ident).addMappedHost(mv);
        }
    }

    /**
     * Create a {@link MacMapCleaner} instance which represents VLAN
     * detached from the MAC mapping.
     *
     * @param ident  The identifier for the MAC mapping.
     * @param mv     A {@link MacVlan} instance which specifies the unmapped
     *               host.
     */
    private void addUnmappedHost(MacMapIdentifier ident, MacVlan mv) {
        if (doPurge) {
            getMapCleaner(ident).addUnmappedHost(mv);
        }
    }

    /**
     * Inactivate mapping in the target MAC mapping which maps the specified
     * host.
     *
     * @param mv  A {@link MacVlan} instance.
     * @throws VTNException  An error occurred.
     */
    private void inactivate(MacVlan mv) throws VTNException {
        inactivate(targetId, mapStatus, mv);
    }

    /**
     * Inactivate mapping in the specified MAC mapping which maps the
     * specified host.
     *
     * @param ident  The identifier for the MAC mapping.
     * @param mv     A {@link MacVlan} instance.
     * @throws VTNException  An error occurred.
     */
    private void inactivate(MacMapIdentifier ident, MacVlan mv)
        throws VTNException {
        boolean target = targetId.equals(ident);
        VTNMacMapStatus vmst = (target) ? mapStatus : statusReader.get(ident);
        inactivate(ident, vmst, mv);

        if (!target && !vmst.hasMapping()) {
            downBridges.add(ident.getBridgeIdentifier());
        }
    }

    /**
     * Inactivate MAC mapping which maps the specified host.
     *
     * @param ident  The identifier for the MAC mapping.
     * @param vmst   The status of the specified MAC mapping.
     * @param mv     A {@link MacVlan} instance.
     * @throws VTNException  An error occurred.
     */
    private void inactivate(MacMapIdentifier ident, VTNMacMapStatus vmst,
                            MacVlan mv) throws VTNException {
        Set<PortVlan> rels = new HashSet<>();
        SalPort oldPort = vmst.inactivate(mv, rels);
        if (oldPort != null) {
            TxContext ctx = getContext();
            if (LOG.isTraceEnabled()) {
                logMacMapInactivated(ident, mv, oldPort);
            }

            // Release switch port reserved by MAC mapping.
            MappingRegistry.releasePort(ctx, rels, ident);
        }
    }

    /**
     * Remove the specified host from the allowed host set.
     *
     * @param mv  A {@link MacVlan} instance.
     * @throws VTNException  An error occurred.
     */
    private void removeAllowed(MacVlan mv) throws VTNException {
        InstanceIdentifier<MacMapAllowed> path = MappingRegistry.
            getAllowedPath(mv);
        TxContext ctx = getContext();
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        DataStoreUtils.delete(tx, path);

        addUnmappedHost(targetId, mv);
        if (mv.getAddress() == MacVlan.UNDEFINED) {
            // MAC mapping mapped by wildcard map will be invalidated by
            // cleanUpImpl().
            unmappedVlans.add(mv.getVlanId());
        } else {
            inactivate(mv);
        }
    }

    /**
     * Add the specified host to the allowed host set.
     *
     * @param mv  A {@link MacVlan} instance.
     * @throws MacMapConflictException
     *    The specified host is already mapped by the specified MAC mapping.
     * @throws VTNException  An error occurred.
     */
    private void addAllowed(MacVlan mv) throws VTNException {
        InstanceIdentifier<MacMapAllowed> path = MappingRegistry.
            getAllowedPath(mv);
        TxContext ctx = getContext();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<MacMapAllowed> opt = DataStoreUtils.read(tx, oper, path);
        String idStr = targetId.toString();
        if (opt.isPresent()) {
            String ref = opt.get().getMapping();
            if (ref.equals(idStr)) {
                // Already added.
                return;
            }

            MacMapIdentifier ident =
                VNodeIdentifier.create(ref, MacMapIdentifier.class);
            throw new MacMapConflictException(mv, ident);
        }

        // Add the given host to the allowed host set.
        MacMapAllowed allowed = new MacMapAllowedBuilder().
            setIdentifier(mv.toString()).
            setMapping(idStr).
            build();
        tx.put(oper, path, allowed, true);

        // Purge network caches corresponding to the specified host if it is
        // not yet mapped by the MAC mapping.
        if (!alreadyMapped.contains(mv)) {
            addMappedHost(targetId, mv);

            if (mv.getAddress() != MacVlan.UNDEFINED) {
                // Check to see if the specified host is mapped by wildcard
                // MAC mapping.
                MacVlan key = new MacVlan(MacVlan.UNDEFINED, mv.getVlanId());
                opt = MappingRegistry.readAllowed(tx, key);
                if (opt.isPresent()) {
                    MacMapIdentifier ident = VNodeIdentifier.create(
                        opt.get().getMapping(), MacMapIdentifier.class);
                    inactivate(ident, mv);
                    addUnmappedHost(ident, mv);
                }
            }
        }
    }

    /**
     * Remove the specified host from the allowed host set.
     *
     * @param mv  A {@link MacVlan} instance.
     * @throws VTNException  An error occurred.
     */
    private void removeDenied(MacVlan mv) throws VTNException {
        String id = targetId.toString();
        InstanceIdentifier<MapReferenceList> path = MappingRegistry.
            getDeniedPath(mv, id);
        TxContext ctx = getContext();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        DataStoreUtils.delete(tx, path);

        // Remove mac-map-denied if it becomes empty.
        InstanceIdentifier<MacMapDenied> dpath = MappingRegistry.
            getDeniedPath(mv);
        Optional<MacMapDenied> dopt = DataStoreUtils.read(tx, oper, dpath);
        if (dopt.isPresent() &&
            MiscUtils.isEmpty(dopt.get().getMapReferenceList())) {
            tx.delete(oper, dpath);
        }

        // Check to see if the specified host is mapped by the target
        // MAC mapping.
        String mref = getMapping(MappingRegistry.readAllowed(tx, mv));
        MacVlan key = new MacVlan(MacVlan.UNDEFINED, mv.getVlanId());
        String wcref = getMapping(MappingRegistry.readAllowed(tx, key));
        if (id.equals(mref)) {
            // The specified host exists in the allowed host set.
            addMappedHost(targetId, mv);
            if (wcref != null && !wcref.equals(id)) {
                // The specified host may be mapped by another MAC mapping.
                // It must be inactivated here.
                MacMapIdentifier ident =
                    VNodeIdentifier.create(wcref, MacMapIdentifier.class);
                inactivate(ident, mv);
                addUnmappedHost(ident, mv);
            }
        } else if (mref == null && id.equals(wcref)) {
            // The specified host will be mapped by the wildcard MAC address
            // entry in the target MAC mapping.
            // Network caches for the host should be purged because it may
            // be mapped to another vBridge by VLAN mapping.
            addMappedHost(targetId, mv);
        }
    }

    /**
     * Add the specified host to the denied host set.
     *
     * @param mv  A {@link MacVlan} instance.
     * @throws VTNException  An error occurred.
     */
    private void addDenied(MacVlan mv) throws VTNException {
        String id = targetId.toString();
        InstanceIdentifier<MapReferenceList> path = MappingRegistry.
            getDeniedPath(mv, id);
        TxContext ctx = getContext();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<MapReferenceList> opt = DataStoreUtils.read(tx, oper, path);
        if (!opt.isPresent()) {
            // Add the specified host to the denied host set.
            MapReferenceList mref = new MapReferenceListBuilder().
                setMapping(id).build();
            tx.put(oper, path, mref, true);
            addUnmappedHost(targetId, mv);
            inactivate(mv);
        }
    }

    /**
     * Return the mapping identifier in the given field.
     *
     * @param opt  An {@link Optional} instance that contains a
     *             vtn-map-reference-field.
     * @return  A string representation of the mapping identifier if present.
     *          {@code null} otherwise.
     */
    private String getMapping(Optional<? extends VtnMapReferenceField> opt) {
        return (opt.isPresent()) ? opt.get().getMapping() : null;
    }

    // VirtualMapRegistry

    /**
     * {@inheritDoc}
     */
    @Override
    protected void cleanUpImpl(TxContext ctx) throws VTNException {
        VTNMacMapStatus vmst = mapStatus;
        if (!unmappedVlans.isEmpty()) {
            // Inactivate MAC mappings which are no longer valid.
            Set<PortVlan> rels = new HashSet<>();
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            Set<MacVlan> allowed = MappingRegistry.readAllowedSet(tx);
            Map<MacVlan, SalPort> unmapped =
                vmst.inactivate(targetId, allowed, unmappedVlans, rels);
            MappingRegistry.releasePort(ctx, rels, targetId);

            if (LOG.isTraceEnabled()) {
                for (Entry<MacVlan, SalPort> entry: unmapped.entrySet()) {
                    MacVlan mv = entry.getKey();
                    SalPort sport = entry.getValue();
                    logMacMapInactivated(targetId, mv, sport);
                }
            }
        }

        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        for (VBridgeIdentifier vbrId: downBridges) {
            // Change vBridge status to DOWN.
            VBridge vbr = new VBridge(vbrId, vbrId.fetch(tx));
            vbr.putState(ctx, VnodeState.DOWN);
        }
    }
}
