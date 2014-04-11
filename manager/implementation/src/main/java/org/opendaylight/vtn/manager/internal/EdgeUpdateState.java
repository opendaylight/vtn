/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.cluster.PortProperty;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An instance of this class represents an edge update event reported
 * by the controller.
 */
public class EdgeUpdateState implements PortFilter {
    /**
     * A map which keeps switch ports which was reporeted by an edge update
     * event.
     *
     * <ul>
     *   <li>
     *     If the port associated with the map key was changed to ISL port,
     *     {@link Boolean#TRUE} is associated as value.
     *   </li>
     *   <li>
     *     {@link Boolean#FALSE} is associated as value if the port was changed
     *      to edge port.
     *   </li>
     * </ul>
     */
    private final Map<NodeConnector, Boolean>  islMap;

    /**
     * A set which keeps {@code Node} instances configured in
     * {@link #islMap}.
     */
    private Set<Node>  islNodes;

    /**
     * A map which caches the result of
     * {@link VTNManagerImpl#hasEdgePort(Node)} call.
     */
    private Map<Node, Boolean>  edgePortMap;

    /**
     * Construct a new instance.
     *
     * @param islMap  A map which keeps actual changes to ISL ports.
     */
    public EdgeUpdateState(Map<NodeConnector, Boolean> islMap) {
        this.islMap = islMap;
    }

    /**
     * Determine whether the link state of the specified port was changed
     * event or not.
     *
     * @param port  A {@link NodeConnector} instance.
     * @return  {@link Boolean#TRUE} is returned if the specified port was
     *          changed to ISL port.
     *          {@link Boolean#FALSE} is returned if the specified port was
     *          changed to edge port.
     *          {@code null} is returned if the link state of the specified
     *          port was not changed.
     */
    public Boolean getPortState(NodeConnector port) {
        return islMap.get(port);
    }

    /**
     * Determine whether at least one port in the specified node is reporeted
     * by an edge update event or not.
     *
     * @param node  A {@link Node} instance.
     * @return  {@code true} is returned if at least one switch port in the
     *          specified node is reported by an edge update event.
     *          Otherwise {@code false} is returned.
     */
    public boolean contains(Node node) {
        Set<Node> nset = islNodes;
        if (nset == null) {
            nset = new HashSet<Node>();
            for (NodeConnector port: islMap.keySet()) {
                nset.add(port.getNode());
            }
            islNodes = nset;
        }

        return nset.contains(node);
    }

    /**
     * Determine whether the given node has at least one edge port in up state
     * or not.
     *
     * <p>
     *   This method must be called with holding the manager lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param node  A {@link Node} instance associated with SDN switch.
     * @return  {@code true} is returned if the given node has at least one
     *          edge port in up state. Otherwise {@code false} is returned.
     */
    public boolean hasEdgePort(VTNManagerImpl mgr, Node node) {
        if (edgePortMap != null) {
            Boolean result = edgePortMap.get(node);
            if (result != null) {
                // Return a cached result.
                return result.booleanValue();
            }
        } else {
            edgePortMap = new HashMap<Node, Boolean>();
        }

        boolean ret = mgr.hasEdgePort(node);
        edgePortMap.put(node, Boolean.valueOf(ret));

        return ret;
    }

    // PortFilter

    /**
     * Test if the specified switch port should be accepted or not.
     *
     * <p>
     *   This method accepts if the specified port was changed to ISL port.
     * </p>
     *
     * @param port  A {@link NodeConnector} object corresponding to the
     *              switch port to be tested.
     * @param prop  Unused.
     * @return  {@code true} if the specified port should be accepted.
     *          {@code false} if the specified port should be filtered out.
     */
    @Override
    public boolean accept(NodeConnector port, PortProperty prop) {
        Boolean isl = islMap.get(port);
        return (isl != null && isl.booleanValue());
    }
}
