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
 * {@code PortVlanMacFilter} describes a MAC address table entry filter that
 * selects entries detected on the VLAN specified by a pair of physical switch
 * port and VLAN ID.
 */
public class PortVlanMacFilter extends PortMacFilter {
    /**
     * The target VLAN ID.
     */
    private final int  vlanId;

    /**
     * Construct a new instance.
     *
     * @param sport  A {@link SalPort} instance which specifies the target
     *               physical switch port.
     * @param vid    A VLAN ID.
     */
    public PortVlanMacFilter(SalPort sport, int vid) {
        super(sport);
        vlanId = vid;
    }

    // NodeMacFilter

    /**
     * Test if the specified MAC address table entry should be accepted or not.
     *
     * @param ment  A {@link MacTableEntry} instance to be tested.
     * @return  {@code true} only if the specified entry was detected on the
     *          specified VLAN.
     */
    @Override
    public boolean accept(MacTableEntry ment) {
        return (super.accept(ment) && ment.getVlanId().intValue() == vlanId);
    }
}
