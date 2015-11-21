/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * JUnit test for {@link PortVlanMacFilter}.
 */
public class PortVlanMacFilterTest extends TestBase {
    /**
     * Test case for {@link PortVlanMacFilter#accept(MacTableEntry)}.
     */
    @Test
    public void testAccept() {
        long nodeNumber = 3L;
        long portNumber = 2L;
        int targetVid = 113;
        SalPort target = new SalPort(nodeNumber, portNumber);
        PortVlanMacFilter filter = new PortVlanMacFilter(target, targetVid);

        Integer[] vids = {0, 1, 34, targetVid, 481, 1034, 2987, 4095};

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

                    assertEquals(portMatch && vlanMatch, filter.accept(mtent));
                    verify(mtent).getNode();
                    if (dpid == nodeNumber) {
                        verify(mtent).getPortId();
                        if (port == portNumber) {
                            verify(mtent).getVlanId();
                        }
                    }
                    verifyNoMoreInteractions(mtent);
                }
            }
        }
    }
}
