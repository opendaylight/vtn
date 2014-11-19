/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;

/**
 * {@code NodeUtils} class is a collection of utility class methods
 * related to {@link Node} and {@link NodeConnector}.
 */
public final class NodeUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private NodeUtils() {}

    /**
     * Check the specified node.
     *
     * @param node  Node to be tested.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNode(Node node) throws VTNException {
        // Currently only OpenFlow node is supported.
        checkNode(node, true);
    }

    /**
     * Check the specified node.
     *
     * @param node       Node to be tested.
     * @param checkType  {@code true} means that this method should check
     *                   whether the type of the given node is supported or
     *                   not.
     *                   {@code false} means that the caller does not care
     *                   about node type.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNode(Node node, boolean checkType)
        throws VTNException {
        if (node == null) {
            Status status = MiscUtils.argumentIsNull("Node");
            throw new VTNException(status);
        }

        String type = node.getType();
        Object id = node.getID();
        if (type == null || id == null) {
            String msg = "Broken node is specified";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        if (checkType) {
            checkNodeType(type);
        }
    }

    /**
     * Check the specified node connector.
     *
     * @param nc  Node connector to be tested.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNodeConnector(NodeConnector nc)
        throws VTNException {
        // Currently only OpenFlow node is supported.
        checkNodeConnector(nc, true);
    }

    /**
     * Check the specified node connector.
     *
     * @param nc         Node connector to be tested.
     * @param checkType  {@code true} means that this method should check
     *                   whether the type of the given node connector is
     *                   supported or not.
     *                   {@code false} means that the caller does not care
     *                   about node connector type.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNodeConnector(NodeConnector nc, boolean checkType)
        throws VTNException {
        if (nc == null) {
            Status status = MiscUtils.argumentIsNull("Node connector");
            throw new VTNException(status);
        }

        String type = nc.getType();
        Object id = nc.getID();
        if (type == null || id == null) {
            String msg = "Broken node connector is specified";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        if (checkType) {
            checkNodeConnectorType(type);
        }

        checkNode(nc.getNode(), checkType);
    }

    /**
     * Check whether the given node type is supported by the VTN Manager.
     *
     * @param type  The node type to be tested.
     * @throws VTNException  The specified node type is not supported.
     */
    public static void checkNodeType(String type)
        throws VTNException {
        // Only OpenFlow switch is supported.
        if (!Node.NodeIDType.OPENFLOW.equals(type)) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "Unsupported node: type=" + type);
        }
    }

    /**
     * Check whether the given node connector type is supported by the
     * VTN Manager.
     *
     * @param type  The node connector type to be tested.
     * @throws VTNException  The specified node connector type is not
     *                       supported.
     */
    public static void checkNodeConnectorType(String type)
        throws VTNException {
        if (!NodeConnector.NodeConnectorIDType.OPENFLOW.equals(type)) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "Unsupported node connector: type=" + type);
        }
    }

    /**
     * Check whether the given {@link SwitchPort} instance is valid or not.
     *
     * @param port  A {@link SwitchPort} instance to be tested.
     * @param node  A {@link Node} instance corresponding to a switch port.
     * @throws VTNException  The specified instance contains invalid value.
     */
    public static void checkSwitchPort(SwitchPort port, Node node)
        throws VTNException {
        if (port == null) {
            Status status = MiscUtils.argumentIsNull("Switch port");
            throw new VTNException(status);
        }

        String type = port.getType();
        if (type != null) {
            String id = port.getId();
            if (id == null) {
                String msg = "Port type must be specified with port ID";
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }

            // Currently only OpenFlow node connector is supported.
            if (!NodeConnector.NodeConnectorIDType.OPENFLOW.equals(type)) {
                String msg = "Unsupported node connector type";
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }

            // Ensure that we can construct a NodeConnector instance.
            NodeConnector nc = NodeConnector.fromStringNoNode(type, id, node);
            if (nc == null) {
                String msg = "Broken node connector is specified";
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        } else if (port.getId() != null) {
            String msg = "Port ID must be specified with port type";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        String name = port.getName();
        if (name == null) {
            if (type == null) {
                String msg = "Switch port cannot be empty";
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        } else if (name.isEmpty()) {
            String msg = "Port name cannot be empty";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Check whether the given {@link PortLocation} instance is valid or not.
     *
     * <p>
     *   Note that this method accepts a {@link PortLocation} instance
     *   which does not contain a {@link SwitchPort} instance.
     * </p>
     *
     * @param ploc  A {@link PortLocation} instance to be tested.
     * @throws VTNException  The specified instance contains invalid value.
     */
    public static void checkPortLocation(PortLocation ploc)
        throws VTNException {
        if (ploc == null) {
            Status status = MiscUtils.argumentIsNull("Port location");
            throw new VTNException(status);
        }

        Node node = ploc.getNode();
        SwitchPort sw = ploc.getPort();
        checkNode(node, true);
        if (sw != null) {
            checkSwitchPort(sw, node);
        }
    }

    /**
     * Find a {@link NodeConnector} instance that meets the specified
     * condition.
     *
     * @param mgr      VTN Manager service.
     * @param node     A {@link Node} instance corresponding to a physical
     *                 switch.
     * @param port     A {@link SwitchPort} instance which specifies the
     *                 condition to select switch port.
     * @return  A {@link NodeConnector} instance if found.
     *          {@code null} is returned if not found.
     */
    public static NodeConnector findPort(VTNManagerImpl mgr, Node node,
                                         SwitchPort port) {
        if (!mgr.exists(node)) {
            return null;
        }

        ISwitchManager swMgr = mgr.getSwitchManager();
        NodeConnector target = null;
        String type = port.getType();
        String id = port.getId();
        if (type != null && id != null) {
            // Try to construct a NodeConnector instance.
            // This returns null if invalid parameter is specified.
            target = NodeConnector.fromStringNoNode(type, id, node);
            if (target == null) {
                return null;
            }
        }

        String name = port.getName();
        if (name != null) {
            // Search for a switch port by its name.
            NodeConnector nc = swMgr.getNodeConnector(node, name);
            if (nc == null || swMgr.isSpecial(nc)) {
                return null;
            }
            if (target != null && !target.equals(nc)) {
                nc = null;
            }
            return nc;
        }

        // Ensure that the detected NodeConnector exists.
        if (target != null && !mgr.exists(target)) {
            target = null;
        }
        return target;
    }
}
