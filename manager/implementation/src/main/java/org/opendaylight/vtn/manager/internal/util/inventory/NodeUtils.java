/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * {@code NodeUtils} class is a collection of utility class methods related to
 * node-id and node-connector-id, aka switch and switch port identifier.
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
     * Return a new {@link RpcException} which indicates the location of the
     * switch port is not specified.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullPortLocationException() {
        return RpcException.getNullArgumentException("Port location");
    }

    /**
     * Return a new {@link RpcException} that indicates the port descriptor
     * is null.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullPortDescException() {
        return RpcException.getNullArgumentException("vtn-port-desc");
    }

    /**
     * Check whether the given parameters specifies a physical switch port
     * or not.
     *
     * @param snode     A {@link SalNode} instance which specifies a physical
     *                  switch.
     * @param portId    A string representation of a switch port ID.
     * @param portName  The name of the switch port.
     * @return  If a valid port identifier is passed to {@code portId},
     *          A {@link SalPort} instance converted from {@code snode} and
     *          {@code portId} is returned. {@code null} otherwise.
     * @throws RpcException
     *    The specified instance is invalid.
     */
    public static SalPort checkPortLocation(SalNode snode, String portId,
                                            String portName)
        throws RpcException {
        SalPort sport = checkPortLocationImpl(snode, portId, portName);
        if (portId == null && portName == null) {
            throw RpcException.getBadArgumentException(
                "Target port name or ID must be specified");
        }

        return sport;
    }

    /**
     * Check whether the given {@link VtnPortDesc} instance is valid or not.
     *
     * @param vdesc  A {@link VtnPortDesc} instance to be tested.
     * @throws RpcException
     *    The specified instance is invalid.
     */
    public static void checkVtnPortDesc(VtnPortDesc vdesc)
        throws RpcException {
        String value = (vdesc == null) ? null : vdesc.getValue();
        if (value == null) {
            throw getNullPortDescException();
        }

        int idx = value.indexOf(PORT_DESC_SEPARATOR);
        if (idx >= 0) {
            String nodeId = value.substring(0, idx);
            int idStart = idx + 1;
            int idEnd = value.indexOf(PORT_DESC_SEPARATOR, idStart);
            if (idEnd >= 0) {
                String portId = (idStart == idEnd)
                    ? null : value.substring(idStart, idEnd);
                int nameStart = idEnd + 1;
                String portName = (nameStart >= value.length())
                    ? null : value.substring(nameStart);
                try {
                    SalNode snode = SalNode.checkedCreate(nodeId);
                    checkPortLocationImpl(snode, portId, portName);
                    return;
                } catch (RpcException e) {
                    String msg = "Invalid vtn-port-desc: " + value + ": " +
                        e.getMessage();
                    throw RpcException.getBadArgumentException(msg, e);
                }
            }
        }

        throw RpcException.getBadArgumentException(
            "Invalid vtn-port-desc format: " + value);
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
     * Check whether the given parameters specifies a valid switch port or not.
     *
     * @param snode     A {@link SalNode} instance which specifies a physical
     *                  switch.
     * @param portId    A string representation of switch port identifier.
     * @param portName  The name of the switc port.
     * @return  If a valid port identifier is passed to {@code portId},
     *          A {@link SalPort} instance converted from {@code snode} and
     *          {@code portId} is returned. {@code null} otherwise.
     * @throws RpcException
     *    The given parameters does not specify a valid switch port.
     */
    private static SalPort checkPortLocationImpl(SalNode snode, String portId,
                                                 String portName)
        throws RpcException {
        SalPort sport;
        if (portId == null) {
            sport = null;
        } else {
            sport = SalPort.create(snode.getNodeNumber(), portId);
            if (sport == null) {
                throw RpcException.getBadArgumentException(
                    "Invalid port ID: " + portId);
            }
        }

        if (portName != null && portName.isEmpty()) {
            throw RpcException.getBadArgumentException(
                "Port name cannot be empty");
        }

        return sport;
    }
}
