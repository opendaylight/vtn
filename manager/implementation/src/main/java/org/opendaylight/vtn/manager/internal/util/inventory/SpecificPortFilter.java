/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;

/**
 * An implementation of {@link PortFilter} that accepts {@link SalPort}
 * instance which equals to the specified one.
 */
public class SpecificPortFilter implements PortFilter {
    /**
     * The target {@link SalPort} instance.
     */
    private final SalPort  targetPort;

    /**
     * Construct a new instance.
     *
     * @param sport The target {@link SalPort} instance.
     *              Specifying {@code null} results in undefined behavior.
     */
    public SpecificPortFilter(SalPort sport) {
        targetPort = sport;
    }

    /**
     * Test if the specified switch port is identical to the target port.
     *
     * @param sport  A {@link SalPort} instance corresponding to the switch
     *               port to be tested.
     * @param vport  Unused.
     * @return  {@code true} if the specified port is identical to the
     *          target port. Otherwise {@code false} is returned.
     */
    @Override
    public boolean accept(SalPort sport, VtnPort vport) {
        return targetPort.equalsPort(sport);
    }

    // Object

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        return "port-filter[port=" + targetPort + "]";
    }
}
