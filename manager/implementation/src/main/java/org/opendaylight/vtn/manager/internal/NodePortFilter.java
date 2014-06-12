/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.PortProperty;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@link PortFilter} implementation which accepts {@link NodeConnector}
 * instances in the specified switch.
 */
public class NodePortFilter implements PortFilter {
    /**
     * The target {@link Node} instance.
     */
    private final Node  targetNode;

    /**
     * Construct a new instance.
     *
     * @param node  The target {@link Node} instance.
     *              Specifying {@code null} results in undefined behavior.
     */
    public NodePortFilter(Node node) {
        targetNode = node;
    }

    /**
     * Test if the specified switch port is in the target node.
     *
     * @param port  A {@link NodeConnector} object corresponding to the
     *              switch port to be tested.
     * @param prop  Unused.
     * @return  {@code true} if the specified port is in the target node.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public boolean accept(NodeConnector port, PortProperty prop) {
        return targetNode.equals(port.getNode());
    }
}
