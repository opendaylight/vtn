/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.LOG;

import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.PortMapping;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.PortMappingBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.VlanMapping;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code PortMapRegistry} describes a context for registering port mapping.
 */
public final class PortMapRegistry extends VirtualMapRegistry<PortMapCleaner> {
    /**
     * The identifier for the target virtual interface.
     */
    private final VInterfaceIdentifier<?>  targetId;

    /**
     * The identifier for the MAC mapping invalidated by a new port mapping.
     */
    private MacMapIdentifier  macMapId;

    /**
     * Set {@code true} if all the hosts mapped by the MAC mapping specified by
     * {@link #macMapId} were inactivated.
     */
    private boolean  macMapDown;

    /**
     * A set of {@link MacVlan} instances which represents invalidated
     * MAC mappings.
     */
    private Set<MacVlan>  macMappedHosts;

    /**
     * A {@link SalPort} instance corresponding to a switch port mapped by
     * a new port mapping.
     */
    private SalPort  mappedPort;

    /**
     * Construct a new instance.
     *
     * @param ctx    A runtime context for transaction task.
     * @param ident  The identifier for the target virtual interface.
     */
    public PortMapRegistry(TxContext ctx, VInterfaceIdentifier<?> ident) {
        super(ctx);
        targetId = ident;
    }

    /**
     * Register a new port mapping.
     *
     * @param pvlan  A {@link PortVlan} instances which represents VLAN to be
     *               mapped by a new port mapping.
     * @param purge  If {@code true} is specified, network caches corresponding
     *               to VLAN superseded by a new port mapping will be purged.
     * @throws VTNException  An error occurred.
     */
    public void register(PortVlan pvlan, boolean purge)
        throws VTNException {
        // Determine whether the specified VLAN is mapped by port mapping
        // or not.
        TxContext ctx = getContext();
        InstanceIdentifier<PortMapping> path = MappingRegistry.getPath(pvlan);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<PortMapping> opt = DataStoreUtils.read(tx, oper, path);
        if (opt.isPresent()) {
            // Try to supersede existing mapping.
            supersede(ctx, opt.get(), pvlan, purge);
        }

        // Register new mapping.
        PortMapping pmap = new PortMappingBuilder().
            setIdentifier(pvlan.toString()).
            setMapping(targetId.toString()).
            build();
        tx.put(oper, path, pmap, true);
        mappedPort = pvlan.getPort();

        if (purge) {
            // Network caches for a VLAN mapping superseded by a new port
            // mapping need to be purged.
            int vid = pvlan.getVlanId();
            NodeVlan nvlan = new NodeVlan(mappedPort.getSalNode(), vid);
            Optional<VlanMapping> vopt = MappingRegistry.read(tx, nvlan);
            if (!vopt.isPresent()) {
                // Try to wildcard VLAN mapping.
                nvlan = new NodeVlan(null, vid);
                vopt = MappingRegistry.read(tx, nvlan);
            }
            if (vopt.isPresent()) {
                VNodeIdentifier vmapId =
                    VNodeIdentifier.create(vopt.get().getMapping());
                setMapCleaner(pvlan, vmapId);
            }
        }

        cleanUp();
    }

    /**
     * Create a {@link PortMapCleaner} instance which represents VLAN
     * superseded by a new port mapping.
     *
     * @param pvlan  A {@link PortVlan} instance which represents VLAN mapped
     *               by a new port mapping.
     * @param ident  The identifier for the virtual mapping superseded by a new
     *               port mapping.
     */
    private void setMapCleaner(PortVlan pvlan, VNodeIdentifier<?> ident) {
        String tname = ident.getTenantNameString();
        PortMapCleaner cleaner = getMapCleaner(tname);
        if (cleaner == null) {
            cleaner = new PortMapCleaner();
            addMapCleaner(tname, cleaner);
        }

        cleaner.add(pvlan);
    }

    /**
     * Supersede MAC mappings by a new port mapping.
     *
     * @param ctx    A runtime context for transaction task.
     * @param pmap   The port mapping or MAC mapping that maps the specified
     *               VLAN.
     * @param pvlan  A {@link PortVlan} instances that specifies the VLAN.
     * @param purge  If {@code true} is specified, network caches corresponding
     *               to VLAN superseded by a new port mapping will be purged.
     * @throws VTNException  An error occurred.
     */
    private void supersede(TxContext ctx, PortMapping pmap, PortVlan pvlan,
                           boolean purge) throws VTNException {
        VNodeIdentifier ident = VNodeIdentifier.create(pmap.getMapping());
        if (ident instanceof VInterfaceIdentifier<?>) {
            // Conflicted with another port mapping.
            throw RpcException.getDataExistsException(
                "Specified port is mapped to " + ident);
        }

        if (!(ident instanceof MacMapIdentifier)) {
            // This should never happen.
            throw new VTNException(
                "Unexpected mapping: pvlan=" + pvlan + ", ident=" +
                ident);
        }

        // Port mapping always supersedes existing MAC mapping.
        // So we need to inactivate MAC mappings on the specified port.
        MacMapIdentifier mmapId = (MacMapIdentifier)ident;
        macMapId = mmapId;

        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = reader.get(mmapId);
        macMappedHosts = vmst.inactivate(pvlan);
        macMapDown = !vmst.hasMapping();
        if (purge) {
            setMapCleaner(pvlan, mmapId);
        }
    }

    // VirtualMapRegistry

    /**
     * {@inheritDoc}
     */
    @Override
    protected void cleanUpImpl(TxContext ctx) throws VTNException {
        if (macMapId != null) {
            if (macMapDown) {
                // The status of the vBridge that contains the MAC mapping
                // specified macMapId needs to be changed to DOWN.
                BridgeIdentifier<Vbridge> vbrId =
                    macMapId.getBridgeIdentifier();
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                VBridge vbr = new VBridge(vbrId, vbrId.fetch(tx));
                vbr.putState(ctx, VnodeState.DOWN);
            }

            // Record trace logs that notify inactivated MAC mappings.
            if (macMappedHosts != null && LOG.isTraceEnabled()) {
                for (MacVlan mvlan: macMappedHosts) {
                    LOG.trace("{}: MAC mapping has been inactivated: " +
                              "host={}, port={}", macMapId, mvlan, mappedPort);
                }
            }
        }
    }
}
