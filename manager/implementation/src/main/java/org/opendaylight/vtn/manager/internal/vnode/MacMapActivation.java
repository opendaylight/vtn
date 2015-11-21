/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.LOG;

import com.google.common.base.Optional;

import org.apache.commons.lang3.tuple.Pair;
import org.apache.commons.lang3.tuple.Triple;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.flow.remove.EdgeHostFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.FlowRemoverQueue;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapGoneException;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapPortBusyException;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNMacMapStatus;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.PortMapping;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.vtn.mappings.PortMappingBuilder;

/**
 * {@code MacMapActivation} describes a runtime context for activating the
 * MAC mapping.
 */
public final class MacMapActivation {
    /**
     * The identifier for the target MAC mapping.
     */
    private final MacMapIdentifier  targetId;

    /**
     * The L2 host to be mapped by the MAC mapping.
     */
    private final MacVlan  targetHost;

    /**
     * The physical switch port where the target host is detected.
     */
    private final SalPort  targetPort;

    /**
     * A reference to the VLAN mapping which maps the VLAN specified by
     * {@link #reservedNetwork}.
     */
    private VlanMapIdentifier  vlanMap;

    /**
     * A {@link PortVlan} instance which represents the VLAN reserved by the
     * MAC mapping.
     */
    private PortVlan  reservedNetwork;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the target MAC mapping.
     * @param mv     A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param sport  A {@link SalPort} instance corresponding to a switch port
     *               where the specified host is detected.
     */
    public MacMapActivation(MacMapIdentifier ident, MacVlan mv,
                            SalPort sport) {
        targetId = ident;
        targetHost = mv;
        targetPort = sport;
    }

    /**
     * Activate the specified host in the MAC mapping.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @return  {@code true} is returned if the specified MAC mapping was
     *          activated. In other words, {@code true} is returned if the
     *          specified host was activated, and it is the only active host
     *          in the MAC mapping. Otherwise {@code false} is returned.
     * @throws MacMapGoneException
     *    The specified host is no longer mapped by the target MAC mapping.
     * @throws MacMapPortBusyException
     *    The specified VLAN network on a switch port is reserved by
     *    another virtual mapping.
     * @throws org.opendaylight.vtn.manager.internal.util.vnode.MacMapDuplicateException
     *    The same MAC address as {@code mv} is already mapped to the same
     *    vBridge.
     * @throws VTNException  A fatal error occurred.
     */
    public boolean activate(TxContext ctx) throws VTNException {
        // Ensure that the specified host is mapped by the specified
        // MAC mapping.
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        checkMacMapping(tx);

        // Reserve the VLAN on the specified switch port for the MAC mapping.
        int vid = targetHost.getVlanId();
        PortVlan pv = new PortVlan(targetPort, vid);
        boolean reserved = reservePort(tx, pv);

        // Activate the MAC mapping.
        boolean activated;
        Triple<Boolean, SalPort, PortVlan> result =
            activate(ctx, pv, reserved);
        if (result == null) {
            // The specified host is already activated.
            activated = false;
        } else {
            activated = result.getLeft().booleanValue();
            cleanUp(ctx, result.getMiddle());
        }

        return activated;
    }

    /**
     * Ensure that the specified host is mapped by the specified MAC mapping.
     *
     * @param tx  A transaction for the MD-SAL datastore.
     * @throws MacMapGoneException
     *    The specified host is no longer mapped by the target MAC mapping.
     * @throws VTNException  A fatal error occurred.
     */
    private void checkMacMapping(ReadWriteTransaction tx) throws VTNException {
        String id = targetId.toString();
        Pair<MacVlan, String> allowed =
            MappingRegistry.getMacMapAllowed(tx, targetHost);
        if (allowed == null || !id.equals(allowed.getRight())) {
            throw new MacMapGoneException(targetHost, targetId);
        }
    }

    /**
     * Reserve the specified VLAN on a switch port for the specified
     * MAC mapping.
     *
     * @param tx  A MD-SAL datastore transaction.
     * @param pv  A {@link PortVlan} instance that indicates the VLAN to be
     *            reserved.
     * @return  {@code true} if the given port has been reserved successfully.
     *          {@code false} if the given port is already reserved.
     * @throws MacMapPortBusyException
     *    The specified VLAN network on a switch port is reserved by another
     *    virtual mapping.
     * @throws VTNException  An error occurred.
     */
    private boolean reservePort(ReadWriteTransaction tx, PortVlan pv)
        throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<PortMapping> path = MappingRegistry.getPath(pv);
        Optional<PortMapping> opt = DataStoreUtils.read(tx, oper, path);
        boolean present = opt.isPresent();
        if (present) {
            String map = opt.get().getMapping();
            if (!targetId.toString().equals(map)) {
                // The specified port is reserved by a port mapping.
                VNodeIdentifier<?> ident =  VNodeIdentifier.create(map);
                throw new MacMapPortBusyException(targetHost, targetId, ident);
            }
        } else {
            // Reserve the specified port for the MAC mapping.
            PortMapping pmap = new PortMappingBuilder().
                setIdentifier(pv.toString()).
                setMapping(targetId.toString()).
                build();
            tx.put(oper, path, pmap, true);
        }

        return !present;
    }

    /**
     * Activate the target MAC mapping.
     *
     * @param ctx       MD-SAL datastore transaction context.
     * @param pv        A {@link PortVlan} instance that indicates the VLAN to
     *                  be reserved.
     * @param reserved  {@code true} means that the target VLAN on the switch
     *                  port is reserved.
     * @return
     *   <p>
     *     A {@link Triple} instance is returned if the MAC mapping was
     *     successfully activated.
     *   </p>
     *   <ul>
     *     <li>
     *       A boolean value that indicates the result is set to the left.
     *       {@code true} means that the target MAC mapping has been activated.
     *       {@code false} means that the target MAC mapping is already
     *       activated.
     *     </li>
     *     <li>
     *       A {@link SalPort} instance that specifies the physical switch port
     *       previously associated with the L2 host in the MAC mapping is set
     *       to the middle.
     *     </li>
     *     <li>
     *       A {@link PortVlan} instance that specifies the VLAN to be released
     *       is set to the right.
     *     </li>
     *   </ul>
     *   <p>
     *     {@code null} is returned if the MAC mapping for the specified
     *     host is already activated.
     *   </p>
     * @throws org.opendaylight.vtn.manager.internal.util.vnode.MacMapDuplicateException
     *    The specified MAC address is already mapped by this MAC mapping.
     * @throws VTNException  A fatal error occurred.
     */
    private Triple<Boolean, SalPort, PortVlan> activate(
        TxContext ctx, PortVlan pv, boolean reserved) throws VTNException {
        MacMapStatusReader reader = ctx.getSpecific(MacMapStatusReader.class);
        VTNMacMapStatus vmst = reader.get(targetId);
        Triple<Boolean, SalPort, PortVlan> result =
            vmst.activate(targetId, targetHost, targetPort);
        if (result != null) {
            PortVlan released = result.getRight();
            if (released != null) {
                // Release the switch port which is no longer used by the
                // target MAC mapping.
                MappingRegistry.releasePort(ctx, released, targetId);
            }
            if (reserved) {
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                VlanMapIdentifier vmapId = MappingRegistry.
                    getVlanMapping(tx, pv.getPort(), pv.getVlanId());
                if (vmapId != null) {
                    // Need to purge network caches superseded by the
                    // MAC mapping.
                    vlanMap = vmapId;
                    reservedNetwork = pv;
                }
            }
        } else if (reserved) {
            // This should never happen.
            String msg = "Port is reserved by MAC mapping unexpectedly: " +
                "map=" + targetId + ", host=" + targetHost + ", port=" +
                targetPort;
            throw new VTNException(msg);
        }

        return result;
    }

    /**
     * Clean up the transaction for activating the MAC mapping.
     *
     * @param ctx      MD-SAL datastore transaction context.
     * @param oldPort  A {@link SalPort} instance that was previously
     *                 associated with the target host in the MAC mapping.
     * @throws VTNException  A fatal error occurred.
     */
    private void cleanUp(TxContext ctx, SalPort oldPort) throws VTNException {
        if (vlanMap != null) {
            // Purge obsolete network caches originated by the VLAN mapping.
            PortMapCleaner cleaner = new PortMapCleaner();
            cleaner.add(reservedNetwork);
            cleaner.purge(ctx, vlanMap.getTenantNameString());
        }

        if (oldPort == null) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: MAC mapping has been activated: host={}, " +
                          "port={}", targetId, targetHost, targetPort);
            }
        } else {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: MAC mapped host has been moved: host={}, " +
                          "from={}, to={}", targetId, targetHost, oldPort,
                          targetPort);
            }

            // Remove MAC addresses registered by old MAC mapping.
            VBridgeIdentifier vbrId = targetId.getBridgeIdentifier();
            MacEntryRemover.remove(ctx, vbrId, targetHost.getMacAddress());

            // Remove flow entries registered by old MAC mapping.
            String tname = targetId.getTenantNameString();
            EdgeHostFlowRemover remover =
                new EdgeHostFlowRemover(tname, targetHost, targetPort);
            ctx.getSpecific(FlowRemoverQueue.class).enqueue(remover);
        }
    }
}
