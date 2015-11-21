/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.inventory.port.SpecificPortFilter;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * JUnit test for {@link ExtendedPortVlanMacFilter}.
 */
public class ExtendedPortVlanMacFilterTest extends TestBase {
    /**
     * Test case for {@link ExtendedPortVlanMacFilter#accept(MacTableEntry)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAccept() throws Exception {
        long nodeNumber = 2L;
        long portNumber = 5L;
        int targetVid = 3491;
        SalPort target = new SalPort(nodeNumber, portNumber);
        SpecificPortFilter portFilter = new SpecificPortFilter(target);

        // Filter without specifying VLAN ID.
        ExtendedPortVlanMacFilter filter =
            new ExtendedPortVlanMacFilter(portFilter, null);

        // Filter with specifying VLAN ID.
        ExtendedPortVlanMacFilter vlanFilter =
            new ExtendedPortVlanMacFilter(portFilter, targetVid);

        Integer[] vids = {0, 1, 34, 481, 1034, targetVid, 4095};

        for (long dpid = 1L; dpid < 10L; dpid++) {
            for (long port = 1L; port <= 5L; port++) {
                String pnum = String.valueOf(port);
                SalPort sport = new SalPort(dpid, port);
                boolean portMatch = sport.equals(target);
                for (Integer vid: vids) {
                    boolean vlanMatch = (vid.intValue() == targetVid);
                    MacTableEntry mtent = mock(MacTableEntry.class);
                    when(mtent.getNode()).thenReturn(sport.getNodeId());
                    when(mtent.getPortId()).thenReturn(pnum);
                    when(mtent.getVlanId()).thenReturn(vid);

                    assertEquals(portMatch, filter.accept(mtent));
                    verify(mtent).getNode();
                    verify(mtent).getPortId();
                    verifyNoMoreInteractions(mtent);

                    assertEquals(portMatch && vlanMatch,
                                 vlanFilter.accept(mtent));
                    verify(mtent).getVlanId();
                    if (vlanMatch) {
                        verify(mtent, times(2)).getNode();
                        verify(mtent, times(2)).getPortId();
                    }
                    verifyNoMoreInteractions(mtent);
                }
            }
        }
    }
}
