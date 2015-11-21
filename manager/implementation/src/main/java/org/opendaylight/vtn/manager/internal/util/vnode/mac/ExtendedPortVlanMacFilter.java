/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.inventory.port.PortFilter;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * {@code ExtendedPortVlanMacFilter} describes a MAC address table entry filter
 * that selects entries detected on the VLAN specified by a pair of physical
 * switch port and VLAN ID.
 *
 * <p>
 *   This class selects physical switch ports by {@link PortFilter}.
 *   Note that {@code null} is always passed to the call of
 *   {@link PortFilter#accept(SalPort, org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort)}
 *   as port property.
 * </p>
 */
public class ExtendedPortVlanMacFilter implements MacEntryFilter {
    /**
     * A {@link PortFilter} instance that selects physical switch ports.
     */
    private final PortFilter  portFilter;

    /**
     * The target VLAN ID.
     *
     * <p>
     *   {@code null} means the VLAN ID is not specified.
     * </p>
     */
    private final Integer  vlanId;

    /**
     * Construct a new instance.
     *
     * @param filter  A {@link PortFilter} instance that selects physical
     *                switch port.
     * @param vid     A VLAN ID.
     *                {@code null} means the VLAN ID is not specified.
     */
    public ExtendedPortVlanMacFilter(PortFilter filter, Integer vid) {
        portFilter = filter;
        vlanId = vid;
    }

    // NodeMacFilter

    /**
     * Test if the specified MAC address table entry should be accepted or not.
     *
     * @param ment  A {@link MacTableEntry} instance to be tested.
     * @return  {@code true} only if the specified entry was detected on the
     *          specified VLAN at the switch port selected by
     *          {@link PortFilter}.
     * @throws VTNException  An error occurred.
     */
    @Override
    public boolean accept(MacTableEntry ment) throws VTNException {
        boolean ret = (vlanId == null ||
                       ment.getVlanId().intValue() == vlanId.intValue());
        if (ret) {
            ret = portFilter.accept(SalPort.create(ment), null);
        }

        return ret;
    }
}
