/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Map;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.core.Bandwidth;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.ISwitchManager;

/**
 * {@code NodeUtils} class is a collection of utility class methods
 * related to {@link Node} and {@link NodeConnector}.
 */
public final class NodeUtils {
    /**
     * Type identifier of {@link Node} and {@link NodeConnector} that will be
     * assigned to inventories detected by MD-SAL protocol plugins.
     */
    private static final String  MD_SAL_TYPE = "MD_SAL";

    /**
     * Protocol plugin identifier embedded into inventory identifier.
     */
    private static final String  OF_TYPE = "openflow:";

    /**
     * Private constructor that protects this class from instantiating.
     */
    private NodeUtils() {}

    /**
     * Determine whether the given node type is supported by the VTN Manager.
     *
     * @param type  The node type to be tested.
     * @param id    The node identifier to be tested.
     * @return      {@code true} is returned if the given node type is
     *              supported by the VTN Manager.
     *              Otherwise {@code false} is returned.
     */
    public static boolean isNodeSupported(String type, Object id) {
        if (Node.NodeIDType.OPENFLOW.equals(type)) {
            // OpenFlow node detected by AD-SAL protocol plugin.
            return true;
        }

        return (MD_SAL_TYPE.equals(type) && isOpenFlowIdentifier(id));
    }

    /**
     * Determine whether the given node connector type is supported by the
     * VTN Manager.
     *
     * @param type  The node connector type to be tested.
     * @param id    The node connector identifier to be tested.
     * @return      {@code true} is returned if the given node connector type
     *              is supported by the VTN Manager.
     *              Otherwise {@code false} is returned.
     */
    public static boolean isNodeConnectorSupported(String type, Object id) {
        if (NodeConnector.NodeConnectorIDType.OPENFLOW.equals(type)) {
            // OpenFlow node connector detected by AD-SAL protocol plugin.
            return true;
        }

        return (MD_SAL_TYPE.equals(type) && isOpenFlowIdentifier(id));
    }

    /**
     * Check whether the given node type is supported by the VTN Manager.
     *
     * @param type  The node type to be tested.
     * @param id    The node identifier to be tested.
     * @throws VTNException  The specified node type is not supported.
     */
    public static void checkNodeType(String type, Object id)
        throws VTNException {
        if (!isNodeSupported(type, id)) {
            StringBuilder builder =
                new StringBuilder("Unsupported node: type=");
            builder.append(type).append(", id=").append(id);
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }
    }

    /**
     * Check whether the given node connector type is supported by the
     * VTN Manager.
     *
     * @param type  The node connector type to be tested.
     * @param id    The node connector identifier to be tested.
     * @throws VTNException  The specified node connector type is not
     *                       supported.
     */
    public static void checkNodeConnectorType(String type, Object id)
        throws VTNException {
        if (!isNodeConnectorSupported(type, id)) {
            StringBuilder builder =
                new StringBuilder("Unsupported node connector: type=");
            builder.append(type).append(", id=").append(id);
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }
    }

    /**
     * Determine whether the given node connector is a special node port or
     * not.
     *
     * @param swMgr    {@link ISwitchManager} service instance.
     * @param nc       {@link NodeConnector} object to be tested.
     * @param propMap  A set of properties for the given node connector.
     *                 If {@code null} is specified, this method may retrieve
     *                 port property from {@code swMgr}.
     * @return         {@code true} is returned if the given node connector
     *                 is a special node port.
     *                 Otherwise {@code false} is returned.
     */
    public static boolean isSpecial(ISwitchManager swMgr, NodeConnector nc,
                                    Map<String, Property> propMap) {
        String type = nc.getType();
        if (!MD_SAL_TYPE.equals(type)) {
            return swMgr.isSpecial(nc);
        }

        // Check to see if the given node connector has bandwidth property.
        // Bandwidth property is expected to be set to physical port only.
        if (propMap != null) {
            return !propMap.containsKey(Bandwidth.BandwidthPropName);
        }

        Property bw =
            swMgr.getNodeConnectorProp(nc, Bandwidth.BandwidthPropName);
        return (bw == null);
    }

    /**
     * Determine whether the given object represents {@link Node} or
     * {@link NodeConnector} identifier detected by MD-SAL OpenFlow protocol
     * plugin or not.
     *
     * @param id  The identifier object to be tested.
     * @return    {@code true} is returned if the given object is an inventory
     *            identifier generated by MD-SAL OpenFlow protocol plugin.
     *            Otherwise {@code false} is returned.
     */
    private static boolean isOpenFlowIdentifier(Object id) {
        if (id instanceof String) {
            String str = (String)id;
            return str.startsWith(OF_TYPE);
        }

        return false;
    }
}
