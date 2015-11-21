/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory.port;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;

/**
 * {@link PortFilter} provides interfaces to be implemented by classes which
 * filter switch ports.
 */
public interface PortFilter {
    /**
     * Test if the specified switch port should be accepted or not.
     *
     * @param sport  A {@link SalPort} instance corresponding to the switch
     *               port to be tested.
     * @param vport  A {@link VtnPort} object  which contains properties of
     *               the switch port.
     * @return  {@code true} if the specified port should be accepted.
     *          {@code false} if the specified port should be filtered out.
     * @throws VTNException  An error occurred.
     */
    boolean accept(SalPort sport, VtnPort vport) throws VTNException;
}
