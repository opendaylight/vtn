/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.VTNException;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.junit.Assert;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.vtn.manager.internal.TestBase;

 /**
  * Unit Test for VtnNodeEvent.
  */
public class VtnNodeEventTest extends TestBase {

    /**
     * Static Instance of VTNManagerImpl to perform unit testing.
     */
    private static VTNManagerImpl vtnManagerImpl;
    /**
     * Static Instance of SalNode to perform unit testing.
     */
    private static SalNode salNode;
    /**
     * Static Instance of VtnNode to perform unit testing.
     */
    private static VtnNode vnode;
    /**
     * Static Instance of VtnNodeEvent to perform unit testing.
     */
    private static VtnNodeEvent vtnNodeEvent, vtnNodeEvent1;
    /**
     * Static Instance of VtnUpdateType to perform unit testing.
     */
    private static VtnUpdateType vtnUpdateType;

    /**
     * This method creates the required objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        vtnManagerImpl = Mockito.mock(VTNManagerImpl.class);
        salNode = new SalNode(5L);
        vnode = new VtnNodeBuilder().setId(new NodeId("openflow:1")).build();
        vtnUpdateType = VtnUpdateType.CREATED;
        vtnNodeEvent1 = new VtnNodeEvent(vtnManagerImpl, vnode, vtnUpdateType);
        vtnNodeEvent = new VtnNodeEvent(vtnManagerImpl, vtnNodeEvent1);

    }

    /**
     * Test method for
     * {@link VtnNodeEvent#getSalNode()}.
     */
    @Test
    public void testgetSalNode() {
        Assert.assertTrue(vtnNodeEvent.getSalNode() instanceof SalNode);

    }

    /**
     * Test method for
     * {@link VtnNodeEvent#getVtnNode()}.
     */
    @Test
    public void testgetVtnNode() {
        assertEquals(vnode, vtnNodeEvent.getVtnNode());

    }

    /**
     * Test method for
     * {@link VtnNodeEvent#getUpdateType()}.
     */
    @Test
    public void testgetUpdateType() {
        assertEquals(vtnUpdateType, vtnNodeEvent.getUpdateType());

    }

    /**
     * Test method for
     * {@link VtnNodeEvent#getUpdateType()}.
     */
    @Test
    public void testnotifyEvent() {

        try {
            vtnNodeEvent.notifyEvent();
        } catch (Exception ex) {

            Assert.assertFalse(ex instanceof VTNException);
        }

    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {
        vtnManagerImpl = null;
        salNode = null;
        vnode = null;
        vtnNodeEvent = null;
    }
}
