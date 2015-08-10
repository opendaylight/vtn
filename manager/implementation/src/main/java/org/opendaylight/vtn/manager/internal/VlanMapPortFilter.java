/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;

/**
 * {@link PortFilter} implementation which accepts {@link NodeConnector}
 * instances corresponding to switch ports mapped by the specified
 * VLAN mapping.
 */
public abstract class VlanMapPortFilter implements PortFilter {
    /**
     * VTN resource manager service.
     */
    private final IVTNResourceManager  resMgr;

    /**
     * A VLAN ID configured in the VLAN mapping.
     */
    private final short  targetVlan;

    /**
     * A set of {@link PortVlan} instances which indicates switch ports
     * to be ignored.
     */
    private final Set<PortVlan>  ignorePorts;

    /**
     * Create a new port filter which accepts switch ports mapped by the
     * specified VLAN mapping.
     *
     * @param rmgr    VTN Resource manager service.
     * @param node    A {@link Node} instance configured in the VLAN mapping.
     * @param vlan    A VLAN ID configured in the VLAN mapping.
     * @param ignore  A set of {@link PortVlan} instances which represents
     *                switch ports to be ingored.
     *                {@code null} means all ports should be accepted.
     * @return  A {@link VlanMapPortFilter} instance is returned.
     */
    public static VlanMapPortFilter create(IVTNResourceManager rmgr, Node node,
                                           short vlan, Set<PortVlan> ignore) {
        return (node == null)
            ? new GlobalMapFilter(rmgr, vlan, ignore)
            : new NodeMapFilter(rmgr, node, vlan, ignore);
    }

    /**
     * Construct a new instance.
     *
     * @param rmgr    VTN Resource manager service.
     * @param vlan    A VLAN ID configured in the VLAN mapping.
     * @param ignore  A set of {@link PortVlan} instances which represents
     *                switch ports to be ingored.
     *                {@code null} means all ports should be accepted.
     */
    private VlanMapPortFilter(IVTNResourceManager rmgr, short vlan,
                              Set<PortVlan> ignore) {
        resMgr = rmgr;
        targetVlan = vlan;
        ignorePorts = ignore;
    }

    /**
     * Test if the specified switch port is mapped by the specified
     * VLAN mapping.
     *
     * @param port   A {@link NodeConnector} object corresponding to the
     *               switch port to be tested.
     * @param vport  Unused.
     * @return  {@code true} if the specified port is mapped by the
     *          VLAN mapping. Otherwise {@code false} is returned.
     */
    @Override
    public final boolean accept(NodeConnector port, VtnPort vport) {
        PortVlan pvlan = new PortVlan(port, targetVlan);
        if (ignorePorts != null && ignorePorts.contains(pvlan)) {
            return false;
        }

        if (resMgr.isPortMapped(pvlan)) {
            // This switch port is mapped to virtual interface by port mapping.
            return false;
        }

        return isMapped(resMgr, port, targetVlan);
    }

    /**
     * Test if the specified switch port is mapped by the specified
     * VLAN mapping.
     *
     * @param rmgr  VTN Resource manager service.
     * @param port  A {@link NodeConnector} object corresponding to the
     *              switch port to be tested.
     * @param vlan  A VLAN ID configured in the VLAN mapping.
     * @return  {@code true} if the specified port is mapped by the
     *          VLAN mapping. Otherwise {@code false} is returned.
     */
    protected abstract boolean isMapped(IVTNResourceManager rmgr,
                                        NodeConnector port, short vlan);

    /**
     * {{@link VlanMapPortFilter} implementation for the VLAN mapping which
     * maps all switches in the container.
     */
    private static final class GlobalMapFilter extends VlanMapPortFilter {
        /**
         * A map which keeps a boolean value returned by
         * {@link IVTNResourceManager#isVlanMapped(NodeVlan)} call.
         */
        private final Map<Node, Boolean>  vlanMapped =
            new HashMap<Node, Boolean>();

        /**
         * Construct a new instance.
         *
         * @param rmgr    VTN Resource manager service.
         * @param vlan    A VLAN ID configured in the VLAN mapping.
         * @param ignore  A set of {@link PortVlan} instances which represents
         *                switch ports to be ingored.
         *                {@code null} means all ports should be accepted.
         */
        private GlobalMapFilter(IVTNResourceManager rmgr, short vlan,
                                Set<PortVlan> ignore) {
            super(rmgr, vlan, ignore);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isMapped(IVTNResourceManager rmgr,
                                   NodeConnector port, short vlan) {
            // Avoid cluster cache read as possible for better performance.
            Node node = port.getNode();
            Boolean result = vlanMapped.get(node);
            if (result != null) {
                return result.booleanValue();
            }

            // VLAN mappings which maps a specific node are prior to the
            // global VLAN mapping. So we need eliminate switches mapped by
            // other VLAN mappings.
            NodeVlan nvlan = new NodeVlan(node, vlan);
            boolean ret = !rmgr.isVlanMapped(nvlan);
            result = Boolean.valueOf(ret);
            vlanMapped.put(node, result);

            return ret;
        }
    }

    /**
     * {{@link VlanMapPortFilter} implementation for the VLAN mapping which
     * maps VLAN network on a specific switch.
     */
    private static final class NodeMapFilter extends VlanMapPortFilter {
        /**
         * A {@link Node} instance configured in the VLAN mapping.
         */
        private final Node  targetNode;

        /**
         * Construct a new instance.
         *
         * @param rmgr    VTN Resource manager service.
         * @param node    A {@link Node} instance configured in the VLAN
         *                mapping.
         * @param vlan    A VLAN ID configured in the VLAN mapping.
         * @param ignore  A set of {@link PortVlan} instances which represents
         *                switch ports to be ingored.
         *                {@code null} means all ports should be accepted.
         */
        private NodeMapFilter(IVTNResourceManager rmgr, Node node, short vlan,
                              Set<PortVlan> ignore) {
            super(rmgr, vlan, ignore);
            targetNode = node;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isMapped(IVTNResourceManager rmgr,
                                   NodeConnector port, short vlan) {
            return targetNode.equals(port.getNode());
        }
    }
}
