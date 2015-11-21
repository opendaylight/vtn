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

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

/**
 * JUnit test for {@link NodeMacFilterTest}.
 */
public class NodeMacFilterTest extends TestBase {
    /**
     * Test case for {@link NodeMacFilter#accept(MacTableEntry)}.
     */
    @Test
    public void testAccept() {
        long nodeNumber = 4L;
        SalNode target = new SalNode(nodeNumber);
        NodeMacFilter filter = new NodeMacFilter(target);

        for (long dpid = 1L; dpid < 10L; dpid++) {
            SalNode snode = new SalNode(dpid);
            MacTableEntry mtent = mock(MacTableEntry.class);
            when(mtent.getNode()).thenReturn(snode.getNodeId());
            assertEquals(dpid == nodeNumber, filter.accept(mtent));
            verify(mtent).getNode();
            verifyNoMoreInteractions(mtent);
        }
    }
}
