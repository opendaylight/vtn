/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.Assert;
import org.mockito.Mockito;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.vtn.manager.internal.TestBase;


 /**
  * Unit Test for VtnPortEvent.
  */
public class VtnPortEventTest extends TestBase {
    /**
     * Static Instance of VTNManagerImpl to perform unit testing.
     */
    private static VTNManagerImpl vtnManagerImpl;
    /**
     * Static Instance of SalPort to perform unit testing.
     */
    private static SalPort salPort;
    /**
     * Static Instance of VtnPort to perform unit testing.
     */
    private static VtnPort vport;
    /**
     * Static Instance of VtnPortEvent to perform unit testing.
     */
    private static VtnPortEvent vtnPortEvent, vtnPortEvent1;
    /**
     * Static Instance of VtnUpdateType to perform unit testing.
     */
    private static VtnUpdateType vtnUpdateType;
    /**
     * Static boolean variable to perform unit testing.
     */
    private static boolean state;


    /**
     * This method creates the required objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        vtnManagerImpl = Mockito.mock(VTNManagerImpl.class);
        salPort = new SalPort(5L, 2L);
        state = true;
        vport = createVtnPortBuilder(salPort).build();
        vtnUpdateType = VtnUpdateType.CREATED;
        vtnPortEvent1 = new VtnPortEvent(vtnManagerImpl, vport, state, vtnUpdateType);
        vtnPortEvent = new VtnPortEvent(vtnManagerImpl, vtnPortEvent1);

    }
    /**
     * Test method for
     * {@link VtnPortEvent#getSalPort()}.
     */
    @Test
    public void testGetSalPort() {
        assertEquals(salPort, vtnPortEvent.getSalPort());

    }

    /**
     * Test method for
     * {@link VtnPortEvent#getVtnPort()}.
     */
    @Test
    public void testGetVtnPort() {
        assertEquals(vport, vtnPortEvent.getVtnPort());

    }

     /**
     * Test method for
     * {@link VtnPortEvent#getInterSwitchLinkChange()}.
     */
    @Test
    public void testGetInterSwitchLinkChange() {

        assertEquals(state, vtnPortEvent.getInterSwitchLinkChange());

    }

    /**
     * Test method for
     * {@link VtnPortEvent#getUpdateType()}.
     */
    @Test
    public void testgetUpdateType() {
        assertEquals(vtnUpdateType, vtnPortEvent.getUpdateType());

    }

    /**
     * Test method for
     * {@link VtnPortEvent#notifyEvent()}.
     */
    @Test
    public void testnotifyEvent() {

        try {
            vtnPortEvent.notifyEvent();
        } catch (Exception e) {

            Assert.assertFalse(e instanceof VTNException);
        }


    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {
        vtnManagerImpl = null;
        salPort = null;
        vport = null;
        vtnPortEvent = null;
    }
}
