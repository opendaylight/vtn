/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link VtnNodeEvent}.
 */
public class VtnNodeEventTest extends TestBase {
    /**
     * Test method for {@link VtnNodeEvent#getSalNode()}.
     */
    @Test
    public void testGetSalNode() {
        SalNode[] snodes = {
            new SalNode(1L),
            new SalNode(2L),
            new SalNode(1000L),
            new SalNode(1234567890L),
            SalNode.create("openflow:18446744073709551615"),
        };

        for (SalNode snode: snodes) {
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).build();
            VtnNodeEvent ev1 = new VtnNodeEvent(vnode, VtnUpdateType.CREATED);
            assertEquals(snode, ev1.getSalNode());

            VTNInventoryListener l = mock(VTNInventoryListener.class);
            VtnNodeEvent ev2 = new VtnNodeEvent(l, ev1);
            assertSame(ev1.getSalNode(), ev2.getSalNode());
            verifyZeroInteractions(l);
        }
    }

    /**
     * Test method for {@link VtnNodeEvent#getVtnNode()}.
     */
    @Test
    public void testGetVtnNode() {
        SalNode[] snodes = {
            new SalNode(1L),
            new SalNode(2L),
            new SalNode(1000L),
            new SalNode(1234567890L),
            SalNode.create("openflow:18446744073709551615"),
        };

        for (SalNode snode: snodes) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).build();
            VtnNodeEvent ev1 = new VtnNodeEvent(vnode, VtnUpdateType.CREATED);
            VtnNodeEvent ev2 = new VtnNodeEvent(l, ev1);
            assertSame(vnode, ev1.getVtnNode());
            assertSame(vnode, ev2.getVtnNode());
            verifyZeroInteractions(l);
        }
    }

    /**
     * Test method for {@link VtnNodeEvent#getUpdateType()}.
     */
    @Test
    public void testGetUpdateType() {
        SalNode snode = new SalNode(12345L);
        VtnNode vnode = new VtnNodeBuilder().setId(snode.getNodeId()).build();

        for (VtnUpdateType type: VtnUpdateType.values()) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            VtnNodeEvent ev1 = new VtnNodeEvent(vnode, type);
            VtnNodeEvent ev2 = new VtnNodeEvent(l, ev1);
            assertSame(type, ev1.getUpdateType());
            assertSame(type, ev2.getUpdateType());
            verifyZeroInteractions(l);
        }
    }

    /**
     * Test method for {@link VtnNodeEvent#notifyEvent()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testnotifyEvent() throws Exception {
        SalNode snode = new SalNode(12345L);
        VtnNode vnode = new VtnNodeBuilder().setId(snode.getNodeId()).build();
        VtnNodeEvent ev1 = new VtnNodeEvent(vnode, VtnUpdateType.CREATED);
        try {
            ev1.notifyEvent();
            unexpected();
        } catch (NullPointerException e) {
        }

        VTNInventoryListener l = mock(VTNInventoryListener.class);
        VtnNodeEvent ev2 = new VtnNodeEvent(l, ev1);
        ev2.notifyEvent();
        verify(l).notifyVtnNode(ev2);
        verifyNoMoreInteractions(l);
    }
}
