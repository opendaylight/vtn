/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

/**
 * {@code SalPortGenerator} generates a {@link SalPort} instances with
 * specifying the fixed node identifier.
 */
public final class SalPortGenerator {
    /**
     * Node number.
     */
    private final long  nodeNumber;

    /**
     * Port number.
     */
    private long  portNumber;

    /**
     * Construct a new instance.
     *
     * @param dpid  A node number.
     */
    public SalPortGenerator(long dpid) {
        nodeNumber = dpid;
    }

    /**
     * Construct a new {@link SalPort} instance.
     *
     * @return  A {@link SalPort} instance.
     */
    public SalPort newInstance() {
        return new SalPort(nodeNumber, ++portNumber);
    }
}
