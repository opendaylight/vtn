/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.isPortMapped;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.isVlanMapped;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.inventory.port.PortFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;

/**
 * An implementation of {@link PortFilter} that accepts {@link SalPort}
 * instances corresponding to switch ports mapped by the specified
 * VLAN mapping.
 */
public abstract class VlanMapPortFilter implements PortFilter {
    /**
     * Context for MD-SAL datastore transaction.
     */
    private final TxContext  context;

    /**
     * A VLAN ID configured in the VLAN mapping.
     */
    private final int  targetVlan;

    /**
     * A set of {@link PortVlan} instances which indicates switch ports
     * to be ignored.
     */
    private final Set<PortVlan>  ignorePorts;

    /**
     * Create a new port filter which accepts switch ports mapped by the
     * specified VLAN mapping.
     *
     * @param ctx     A runtime context for transaction task.
     * @param snode   A {@link SalNode} instance configured in the VLAN
     *                mapping.
     * @param vid     A VLAN ID configured in the VLAN mapping.
     * @param ignore  A set of {@link PortVlan} instances which represents
     *                switch ports to be ingored.
     *                {@code null} means all ports should be accepted.
     * @return  A {@link VlanMapPortFilter} instance is returned.
     */
    public static VlanMapPortFilter create(TxContext ctx, SalNode snode,
                                           int vid, Set<PortVlan> ignore) {
        return (snode == null)
            ? new GlobalMapFilter(ctx, vid, ignore)
            : new NodeMapFilter(ctx, snode, vid, ignore);
    }

    /**
     * Construct a new instance.
     *
     * @param ctx     A runtime context for transaction task.
     * @param vid     A VLAN ID configured in the VLAN mapping.
     * @param ignore  A set of {@link PortVlan} instances which represents
     *                switch ports to be ingored.
     *                {@code null} means all ports should be accepted.
     */
    private VlanMapPortFilter(TxContext ctx, int vid, Set<PortVlan> ignore) {
        context = ctx;
        targetVlan = vid;
        ignorePorts = ignore;
    }

    /**
     * Test if the specified switch port is mapped by the specified
     * VLAN mapping.
     *
     * @param sport  A {@link SalPort} instance corresponding to the switch
     *               port to be tested.
     * @param vport  Unused.
     * @return  {@code true} if the specified port is mapped by the
     *          VLAN mapping. Otherwise {@code false} is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public final boolean accept(SalPort sport, VtnPort vport)
        throws VTNException {
        PortVlan pvlan = new PortVlan(sport, targetVlan);
        if (ignorePorts != null && ignorePorts.contains(pvlan)) {
            return false;
        }

        if (isPortMapped(context, pvlan)) {
            // This switch port is mapped to virtual interface by port mapping.
            return false;
        }

        return isMapped(context, sport, targetVlan);
    }

    /**
     * Test if the specified switch port is mapped by the specified
     * VLAN mapping.
     *
     * @param ctx    A runtime context for transaction task.
     * @param sport  A {@link SalPort} instance corresponding to the switch
     *               port to be tested.
     * @param vid    A VLAN ID configured in the VLAN mapping.
     * @return  {@code true} if the specified port is mapped by the
     *          VLAN mapping. Otherwise {@code false} is returned.
     * @throws VTNException  An error occurred.
     */
    protected abstract boolean isMapped(TxContext ctx, SalPort sport,
                                        int vid) throws VTNException;

    /**
     * {{@link VlanMapPortFilter} implementation for the VLAN mapping which
     * maps all switches in the container.
     */
    private static final class GlobalMapFilter extends VlanMapPortFilter {
        /**
         * A map which keeps a boolean value returned by
         * {@link MappingRegistry#isVlanMapped(TxContext, NodeVlan)} call.
         */
        private final Map<SalNode, Boolean>  vlanMapped =
            new HashMap<SalNode, Boolean>();

        /**
         * Construct a new instance.
         *
         * @param ctx     A runtime context for transaction task.
         * @param vid     A VLAN ID configured in the VLAN mapping.
         * @param ignore  A set of {@link PortVlan} instances which represents
         *                switch ports to be ingored.
         *                {@code null} means all ports should be accepted.
         */
        private GlobalMapFilter(TxContext ctx, int vid, Set<PortVlan> ignore) {
            super(ctx, vid, ignore);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isMapped(TxContext ctx, SalPort sport, int vid)
            throws VTNException {
            // Avoid cluster cache read as possible for better performance.
            SalNode node = sport.getSalNode();
            Boolean res = vlanMapped.get(node);
            boolean result;
            if (res != null) {
                result = res.booleanValue();
            } else {
                // VLAN mappings which maps a specific node are prior to the
                // global VLAN mapping. So we need eliminate switches mapped by
                // other VLAN mappings.
                NodeVlan nvlan = new NodeVlan(node, vid);
                result = !isVlanMapped(ctx, nvlan);
                vlanMapped.put(node, result);
            }

            return result;
        }
    }

    /**
     * {{@link VlanMapPortFilter} implementation for the VLAN mapping which
     * maps VLAN network on a specific switch.
     */
    private static final class NodeMapFilter extends VlanMapPortFilter {
        /**
         * A {@link SalNode} instance configured in the VLAN mapping.
         */
        private final SalNode  targetNode;

        /**
         * Construct a new instance.
         *
         * @param ctx     A runtime context for transaction task.
         * @param snode   A {@link SalNode} instance configured in the VLAN
         *                mapping.
         * @param vid     A VLAN ID configured in the VLAN mapping.
         * @param ignore  A set of {@link PortVlan} instances which represents
         *                switch ports to be ingored.
         *                {@code null} means all ports should be accepted.
         */
        private NodeMapFilter(TxContext ctx, SalNode snode, int vid,
                              Set<PortVlan> ignore) {
            super(ctx, vid, ignore);
            targetNode = snode;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isMapped(TxContext ctx, SalPort sport, int vid) {
            return (targetNode.getNodeNumber() == sport.getNodeNumber());
        }
    }
}
