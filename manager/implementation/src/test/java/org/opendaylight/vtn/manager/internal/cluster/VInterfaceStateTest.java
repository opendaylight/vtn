/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * JUnit test for {@link VInterfaceState}.
 */
public class VInterfaceStateTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VnodeState[] states = VnodeState.values();
        for (VnodeState state: states) {
            VInterfaceState ist = new VInterfaceState(state);
            assertSame(state, ist.getState());
            assertSame(VnodeState.UNKNOWN, ist.getPortState());
            assertNull(ist.getMappedPort());

            for (NodeConnector nc: createNodeConnectors(50)) {
                for (VnodeState pstate: states) {
                    ist = new VInterfaceState(state, pstate, nc);
                    assertSame(state, ist.getState());
                    assertSame(pstate, ist.getPortState());
                    assertEquals(nc, ist.getMappedPort());
                }
            }
        }
    }

    /**
     * Test case for dirty flag.
     */
    @Test
    public void testDirty() {
        VInterfaceState ist = new VInterfaceState(VnodeState.UNKNOWN);
        assertSame(VnodeState.UNKNOWN, ist.getState());
        assertSame(VnodeState.UNKNOWN, ist.getPortState());
        assertFalse(ist.isDirty());

        // Change interface state.
        VnodeState[] states = {
            VnodeState.UP,
            VnodeState.DOWN,
            VnodeState.UNKNOWN,
        };
        for (VnodeState state: states) {
            ist.setState(state);
            String emsg = ist.toString();
            assertSame(emsg, state, ist.getState());
            assertTrue(emsg, ist.isDirty());
            assertFalse(emsg, ist.isDirty());

            ist.setState(state);
            emsg = ist.toString();
            assertSame(emsg, state, ist.getState());
            assertFalse(emsg, ist.isDirty());
        }

        // Change port state.
        for (VnodeState state: states) {
            ist.setPortState(state);
            String emsg = ist.toString();
            assertSame(emsg, state, ist.getPortState());
            assertTrue(emsg, ist.isDirty());
            assertFalse(emsg, ist.isDirty());

            ist.setPortState(state);
            emsg = ist.toString();
            assertSame(emsg, state, ist.getPortState());
            assertFalse(emsg, ist.isDirty());
        }

        // Changing mapped port should not affect the dirty flag.
        for (NodeConnector nc: createNodeConnectors(50, false)) {
            ist.setMappedPort(nc);
            String emsg = ist.toString();
            assertEquals(emsg, nc, ist.getMappedPort());
            assertFalse(emsg, ist.isDirty());
        }

        // Set null to mapped port.
        // This should change port state to UNKNOWN.
        assertNotNull(ist.getMappedPort());
        ist.setPortState(VnodeState.UP);
        assertSame(VnodeState.UP, ist.getPortState());
        assertTrue(ist.isDirty());
        assertFalse(ist.isDirty());
        ist.setMappedPort(null);
        assertNull(ist.getMappedPort());
        assertSame(VnodeState.UNKNOWN, ist.getPortState());
        assertTrue(ist.isDirty());
        assertFalse(ist.isDirty());
    }

    /**
     * Test case for {@link VInterfaceState#equals(Object)} and
     * {@link VInterfaceState#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        VnodeState[] states = VnodeState.values();
        List<NodeConnector> connectors = createNodeConnectors(50);
        for (VnodeState state: states) {
            for (VnodeState pstate: states) {
                for (NodeConnector nc: connectors) {
                    VInterfaceState ist1 =
                        new VInterfaceState(state, pstate, nc);
                    VInterfaceState ist2 =
                        new VInterfaceState(state, pstate, copy(nc));
                    testEquals(set, ist1, ist2);
                }
            }
        }

        int required = states.length * states.length * connectors.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VInterfaceState#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VInterfaceState[";
        String suffix = "]";
        VnodeState[] states = VnodeState.values();
        for (VnodeState state: states) {
            for (VnodeState pstate: states) {
                for (NodeConnector nc: createNodeConnectors(50)) {
                    VInterfaceState ist = new VInterfaceState(state, pstate, nc);
                    String m = (nc == null) ? null : "mapped=" + nc;
                    String s = "state=" + state.toString();
                    String p = "portState=" + pstate.toString();
                    String required = joinStrings(prefix, suffix, ",",
                                                  m, s, p);
                    assertEquals(required, ist.toString());
                }
            }
        }
    }

    /**
     * Ensure that {@link VInterfaceState} is serializable.
     */
    @Test
    public void testSerialize() {
        VnodeState[] states = VnodeState.values();
        for (VnodeState state: states) {
            for (VnodeState pstate: states) {
                for (NodeConnector nc: createNodeConnectors(50)) {
                    VInterfaceState ist = new VInterfaceState(state, pstate, nc);
                    serializeTest(ist);
                }
            }
        }
    }
}
