/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
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
