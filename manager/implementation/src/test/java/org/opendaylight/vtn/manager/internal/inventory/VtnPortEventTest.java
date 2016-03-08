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

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link VtnPortEvent}.
 */
public class VtnPortEventTest extends TestBase {
    /**
     * Test method for {@link VtnPortEvent#getSalPort()}.
     */
    @Test
    public void testGetSalPort() {
        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(100L, 12345L),
            new SalPort(33333L, 55555L),
        };

        for (SalPort sport: sports) {
            VtnPort vport = createVtnPortBuilder(sport).build();
            VtnPortEvent ev1 = new VtnPortEvent(
                vport, null, null, VtnUpdateType.CREATED);
            assertEquals(sport, ev1.getSalPort());

            VTNInventoryListener l = mock(VTNInventoryListener.class);
            VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
            assertSame(ev1.getSalPort(), ev2.getSalPort());
            verifyZeroInteractions(l);
        }
    }

    /**
     * Test method for {@link VtnPortEvent#getVtnPort()}.
     */
    @Test
    public void testGetVtnPort() {
        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 2L),
            new SalPort(100L, 12345L),
            new SalPort(33333L, 55555L),
        };

        for (SalPort sport: sports) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            VtnPort vport = createVtnPortBuilder(sport).build();
            VtnPortEvent ev1 = new VtnPortEvent(
                vport, null, null, VtnUpdateType.CREATED);
            VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
            assertSame(vport, ev1.getVtnPort());
            assertSame(vport, ev2.getVtnPort());
            verifyZeroInteractions(l);
        }
    }

    /**
     * Test method for {@link VtnPortEvent#getStateChange()}.
     */
    @Test
    public void testGetStateChange() {
        SalPort sport = new SalPort(10L, 20L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};

        for (Boolean state: bools) {
            for (VtnUpdateType type: VtnUpdateType.values()) {
                VTNInventoryListener l = mock(VTNInventoryListener.class);
                VtnPortEvent ev1 = new VtnPortEvent(vport, state, null, type);
                VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
                if (type == VtnUpdateType.REMOVED) {
                    // State change should be ignored on REMOVED event.
                    assertEquals(null, ev1.getStateChange());
                    assertEquals(null, ev2.getStateChange());
                } else {
                    assertSame(state, ev1.getStateChange());
                    assertSame(state, ev2.getStateChange());
                }
                verifyZeroInteractions(l);
            }
        }
    }

    /**
     * Test method for {@link VtnPortEvent#getInterSwitchLinkChange()}.
     */
    @Test
    public void testGetInterSwitchLinkChange() {
        SalPort sport = new SalPort(10L, 20L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};

        for (Boolean isl: bools) {
            for (VtnUpdateType type: VtnUpdateType.values()) {
                VTNInventoryListener l = mock(VTNInventoryListener.class);
                VtnPortEvent ev1 = new VtnPortEvent(vport, null, isl, type);
                VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
                if (type == VtnUpdateType.REMOVED) {
                    // ISL change should be ignored on REMOVED event.
                    assertEquals(null, ev1.getInterSwitchLinkChange());
                    assertEquals(null, ev2.getInterSwitchLinkChange());
                } else {
                    assertSame(isl, ev1.getInterSwitchLinkChange());
                    assertSame(isl, ev2.getInterSwitchLinkChange());
                }
                verifyZeroInteractions(l);
            }
        }
    }

    /**
     * Test method for {@link VtnPortEvent#getUpdateType()}.
     */
    @Test
    public void testGetUpdateType() {
        SalPort sport = new SalPort(10L, 20L);
        VtnPort vport = createVtnPortBuilder(sport).build();

        for (VtnUpdateType type: VtnUpdateType.values()) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            VtnPortEvent ev1 = new VtnPortEvent(vport, null, null, type);
            VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
            assertSame(type, ev1.getUpdateType());
            assertSame(type, ev2.getUpdateType());
            verifyZeroInteractions(l);
        }
    }

    /**
     * Test method for {@link VtnPortEvent#isDisabled()}.
     */
    @Test
    public void testIsDisabled() {
        SalPort sport = new SalPort(10L, 20L);
        Boolean[] bools = {null, Boolean.TRUE, Boolean.FALSE};

        for (Boolean en: bools) {
            for (VtnUpdateType type: VtnUpdateType.values()) {
                VtnPort vport = createVtnPortBuilder(sport).
                    setEnabled(en).build();
                VTNInventoryListener l = mock(VTNInventoryListener.class);
                VtnPortEvent ev1 = new VtnPortEvent(vport, null, null, type);
                VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
                boolean disabled = (type == VtnUpdateType.REMOVED ||
                                    !Boolean.TRUE.equals(en));
                assertEquals(disabled, ev1.isDisabled());
                assertEquals(disabled, ev2.isDisabled());
                verifyZeroInteractions(l);
            }
        }
    }

    /**
     * Test method for {@link VtnPortEvent#notifyEvent()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNotifyEvent() throws Exception {
        SalPort sport = new SalPort(10L, 20L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        VtnPortEvent ev1 = new VtnPortEvent(
            vport, Boolean.FALSE, Boolean.FALSE, VtnUpdateType.CREATED);
        try {
            ev1.notifyEvent();
            unexpected();
        } catch (NullPointerException e) {
        }

        VTNInventoryListener l = mock(VTNInventoryListener.class);
        VtnPortEvent ev2 = new VtnPortEvent(l, ev1);
        ev2.notifyEvent();
        verify(l).notifyVtnPort(ev2);
        verifyNoMoreInteractions(l);
    }
}
