/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory.port;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;

/**
 * An implementation of {@link PortFilter} that accepts {@link SalPort}
 * instances in the specified switch.
 */
public class NodePortFilter implements PortFilter {
    /**
     * The target {@link SalNode} instance.
     */
    private final SalNode  targetNode;

    /**
     * Construct a new instance.
     *
     * @param snode  The target {@link SalNode} instance.
     *               Specifying {@code null} results in undefined behavior.
     */
    public NodePortFilter(SalNode snode) {
        targetNode = snode;
    }

    /**
     * Test if the specified switch port is in the target node.
     *
     * @param sport  A {@link SalPort} instance corresponding to the switch
     *               port to be tested.
     * @param vport  Unused.
     * @return  {@code true} if the specified port is in the target node.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public boolean accept(SalPort sport, VtnPort vport) {
        return targetNode.equalsNode(sport);
    }

    // Object

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        return "port-filter[node=" + targetNode + "]";
    }
}
