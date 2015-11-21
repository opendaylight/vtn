/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * {@code PortMacFilter} describes a MAC address table entry filter that
 * selects entries detected on the specified physical switch port.
 */
public class PortMacFilter extends NodeMacFilter {
    /**
     * A string representation of the target switch port number.
     */
    private final String  portNumber;

    /**
     * Construct a new instance.
     *
     * @param sport  A {@link SalPort} instance which specifies the target
     *               physical switch port.
     */
    public PortMacFilter(SalPort sport) {
        super(sport);
        portNumber = String.valueOf(sport.getPortNumber());
    }

    // NodeMacFilter

    /**
     * Test if the specified MAC address table entry should be accepted or not.
     *
     * @param ment  A {@link MacTableEntry} instance to be tested.
     * @return  {@code true} only if the specified entry was detected on the
     *          specified physical switch port.
     */
    @Override
    public boolean accept(MacTableEntry ment) {
        return (super.accept(ment) && portNumber.equals(ment.getPortId()));
    }
}
