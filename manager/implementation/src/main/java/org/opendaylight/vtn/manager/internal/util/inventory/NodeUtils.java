/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortLocation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

/**
 * {@code NodeUtils} class is a collection of utility class methods
 * related to {@link Node} and {@link NodeConnector}.
 */
public final class NodeUtils {
    /**
     * Field separator of {@link VtnPortDesc}.
     */
    private static final char  PORT_DESC_SEPARATOR = ',';

    /**
     * Private constructor that protects this class from instantiating.
     */
    private NodeUtils() {}

    /**
     * Return a new {@link RpcException} which indicates the
     * {@link PortLocation} is {@code null}.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullPortLocationException() {
        return RpcException.getNullArgumentException("Port location");
    }

    /**
     * Check the specified node.
     *
     * @param node  Node to be tested.
     * @throws RpcException  The specified node is invalid.
     */
    public static void checkNode(Node node) throws RpcException {
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
     * @throws RpcException  The specified node is invalid.
     */
    public static void checkNode(Node node, boolean checkType)
        throws RpcException {
        if (node == null) {
            throw RpcException.getNullArgumentException("Node");
        }

        String type = node.getType();
        Object id = node.getID();
        if (type == null || id == null) {
            String msg = "Broken node is specified";
            throw RpcException.getBadArgumentException(msg);
        }

        if (checkType) {
            checkNodeType(type);
        }
    }

    /**
     * Check the specified node connector.
     *
     * @param nc  Node connector to be tested.
     * @throws RpcException  The specified node is invalid.
     */
    public static void checkNodeConnector(NodeConnector nc)
        throws RpcException {
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
     * @throws RpcException  The specified node is invalid.
     */
    public static void checkNodeConnector(NodeConnector nc, boolean checkType)
        throws RpcException {
        if (nc == null) {
            throw RpcException.getNullArgumentException("Node connector");
        }

        String type = nc.getType();
        Object id = nc.getID();
        if (type == null || id == null) {
            String msg = "Broken node connector is specified";
            throw RpcException.getBadArgumentException(msg);
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
     * @throws RpcException  The specified node type is not supported.
     */
    public static void checkNodeType(String type)
        throws RpcException {
        // Only OpenFlow switch is supported.
        if (!Node.NodeIDType.OPENFLOW.equals(type)) {
            String msg = "Unsupported node: type=" + type;
            throw RpcException.getBadArgumentException(msg);
        }
    }

    /**
     * Check whether the given node connector type is supported by the
     * VTN Manager.
     *
     * @param type  The node connector type to be tested.
     * @throws RpcException  The specified node connector type is not
     *                       supported.
     */
    public static void checkNodeConnectorType(String type)
        throws RpcException {
        if (!NodeConnector.NodeConnectorIDType.OPENFLOW.equals(type)) {
            String msg = "Unsupported node connector: type=" + type;
            throw RpcException.getBadArgumentException(msg);
        }
    }

    /**
     * Check whether the given {@link SwitchPort} instance is valid or not.
     *
     * @param port  A {@link SwitchPort} instance to be tested.
     * @param node  A {@link Node} instance corresponding to a switch.
     * @throws RpcException  The specified instance contains invalid value.
     */
    public static void checkSwitchPort(SwitchPort port, Node node)
        throws RpcException {
        if (port == null) {
            throw RpcException.getNullArgumentException("Switch port");
        }

        // Check a pair of the port type and ID.
        String type = port.getType();
        String id = port.getId();
        String msg = checkPortTypeId(type, id, node);
        if (msg == null) {
            // Check the port name.
            String name = port.getName();
            if (name == null) {
                if (type == null) {
                    msg = "Switch port cannot be empty";
                }
            } else if (name.isEmpty()) {
                msg = "Port name cannot be empty";
            }
        }

        if (msg != null) {
            throw RpcException.getBadArgumentException(msg);
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
     * @throws RpcException  The specified instance contains invalid value.
     */
    public static void checkPortLocation(PortLocation ploc)
        throws RpcException {
        if (ploc == null) {
            throw getNullPortLocationException();
        }

        Node node = ploc.getNode();
        SwitchPort sw = ploc.getPort();
        checkNode(node, true);
        if (sw != null) {
            checkSwitchPort(sw, node);
        }
    }

    /**
     * Create an array of {@link VtnPortDesc} instances that represents the
     * search condition for physical switch port.
     *
     * @param sport  A {@link SalPort} instance.
     * @param vport  A {@link VtnPort} instance.
     * @return  An array of {@link VtnPortDesc} instances.
     */
    public static VtnPortDesc[] createPortDescArray(SalPort sport,
                                                    VtnPort vport) {
        String portId = Long.toString(sport.getPortNumber());
        String portName = vport.getName();
        if (portName == null) {
            // Port name is unavailable. Port descriptors which specify the
            // port name should not match.
            return new VtnPortDesc[] {
                createPortDesc(sport, portId, null),
                createPortDesc(sport, null, null),
            };
        }

        return new VtnPortDesc[] {
            // Search for a port descriptor by port ID and name.
            createPortDesc(sport, portId, portName),

            // Search for a port descriptor by port ID.
            createPortDesc(sport, portId, null),

            // Search for a port descriptor by port name.
            createPortDesc(sport, null, portName),

            // Search for a port descriptor that does not specify the port.
            createPortDesc(sport, null, null),
        };
    }

    /**
     * Create a {@link VtnPortDesc} instance.
     *
     * @param snode  A {@link SalNode} instance.
     * @param id     A string implementation of the port identifier.
     * @param name   The name of the switch port.
     * @return  A {@link VtnPortDesc} instance.
     */
    public static VtnPortDesc createPortDesc(SalNode snode, String id,
                                             String name) {
        StringBuilder builder = new StringBuilder(snode.toNodeString()).
            append(PORT_DESC_SEPARATOR);
        if (id != null) {
            builder.append(id);
        }

        builder.append(PORT_DESC_SEPARATOR);
        if (name != null) {
            builder.append(name);
        }

        return new VtnPortDesc(builder.toString());
    }

    /**
     * Convert the given {@link PortLocation} instance into a
     * {@link VtnPortDesc} instance.
     *
     * @param ploc  A {@link PortLocation} instance.
     * @return  A {@link VtnPortDesc} instance or {@code null}.
     * @throws RpcException
     *    The given value contains an invalid value.
     */
    public static VtnPortDesc toVtnPortDesc(PortLocation ploc)
        throws RpcException {
        checkPortLocation(ploc);

        SalNode snode = SalNode.create(ploc.getNode());
        if (snode == null) {
            // This should never happen.
            String msg = "Invalid node in port location: " + ploc;
            throw RpcException.getBadArgumentException(msg);
        }

        SwitchPort swport = ploc.getPort();
        String id;
        String name;
        if (swport == null) {
            id = null;
            name = null;
        } else {
            id = swport.getId();
            name = swport.getName();
        }

        try {
            return createPortDesc(snode, id, name);
        } catch (IllegalArgumentException e) {
            // This should never happen.
            String msg = "Unable to convert PortLocation into VtnPortDesc: " +
                ploc;
            throw new RpcException(msg, e);
        }
    }

    /**
     * Convert the given {@link VtnPortDesc} instance into a
     * {@link PortLocation} instance.
     *
     * @param vdesc  A {@link VtnPortDesc} instance.
     * @return  A {@link PortLocation} instance or {@code null}.
     */
    public static PortLocation toPortLocation(VtnPortDesc vdesc) {
        if (vdesc != null) {
            // Parse node-id field.
            String value = vdesc.getValue();
            int idx = value.indexOf(PORT_DESC_SEPARATOR);
            if (idx >= 0) {
                String nodeId = value.substring(0, idx);
                SalNode snode = SalNode.create(nodeId);
                if (snode != null) {
                    return createPortLocation(snode.getAdNode(), value, idx);
                }
            }
        }

        return null;
    }

    /**
     * Convert the given {@link VtnPortLocation} instance into a
     * {@link PortLocation} instance.
     *
     * <p>
     *   Note that this method assumes that the given {@link VtnPortLocation}
     *   contains valid values.
     * </p>
     *
     * @param vpl  A {@link VtnPortLocation} instance.
     * @return  A {@link PortLocation} instance.
     *          {@code null} if {@code vpl} is {@code null}.
     */
    public static PortLocation toPortLocation(VtnPortLocation vpl) {
        if (vpl == null) {
            return null;
        }

        SalNode snode = SalNode.create(vpl.getNode());
        SwitchPort swport = toSwitchPort(vpl);
        return new PortLocation(snode.getAdNode(), swport);
    }

    /**
     * Convert the given {@link VtnSwitchPort} instance into a
     * {@link SwitchPort} instance.
     *
     * @param vswp  A {@link VtnSwitchPort} instance.
     * @return  A {@link PortLocation} instance or {@code null}.
     */
    public static SwitchPort toSwitchPort(VtnSwitchPort vswp) {
        if (vswp == null) {
            return null;
        }

        String id = vswp.getPortId();
        String name = vswp.getPortName();
        String type;

        if (id == null) {
            if (name == null) {
                return null;
            }
            type = null;
        } else {
            // AD-SAL node connector type is fixed to OPENFLOW.
            type = NodeConnectorIDType.OPENFLOW;
        }

        return new SwitchPort(name, type, id);
    }

    /**
     * Verify a pair of switch port type and ID.
     *
     * @param type  The type of the switch port.
     * @param id    The identifier of the switch port.
     * @param node  A {@link Node} instance corresponding to a switch.
     * @return  {@code null} on success.
     *          Otherwise an error message which indicates the cause of
     *          failure.
     */
    private static String checkPortTypeId(String type, String id, Node node) {
        if (type != null) {
            if (id == null) {
                return "Port type must be specified with port ID";
            }

            // Currently only OpenFlow node connector is supported.
            if (!NodeConnector.NodeConnectorIDType.OPENFLOW.equals(type)) {
                return "Unsupported node connector type";
            }

            // Ensure that we can construct a NodeConnector instance.
            NodeConnector nc = NodeConnector.fromStringNoNode(type, id, node);
            if (nc == null) {
                return "Broken node connector is specified";
            }
        } else if (id != null) {
            return "Port ID must be specified with port type";
        }

        return null;
    }

    /**
     * Create a new {@link PortLocation} instance.
     *
     * @param node     A {@link Node} instance corresponding to a switch port.
     * @param vdesc    A string configured in a {@link VtnPortDesc} instance.
     * @param portIdx  A index into {@code vdesc} which specifies the start
     *                 of port ID field in {@code vdesc}.
     * @return  A {@link PortLocation} instance on success.
     *          {@code null} on failure.
     */
    private static PortLocation createPortLocation(Node node, String vdesc,
                                                   int portIdx) {
        // Parse port ID field.
        int idStart = portIdx + 1;
        if (idStart >= vdesc.length()) {
            return null;
        }
        int idEnd = vdesc.indexOf(PORT_DESC_SEPARATOR, idStart);
        if (idEnd < 0) {
            // This should never happen.
            return null;
        }

        String id;
        String type;
        if (idStart == idEnd) {
            id = null;
            type = null;
        } else {
            id = vdesc.substring(idStart, idEnd);

            // We can use AD-SAL node type string as AD-SAL port type.
            type = node.getType();
        }

        // Parse port name field.
        int nameStart = idEnd + 1;
        String name = (nameStart >= vdesc.length())
            ? null : vdesc.substring(nameStart);

        SwitchPort swport = (id == null && name == null)
            ? null : new SwitchPort(name, type, id);
        return new PortLocation(node, swport);
    }
}
