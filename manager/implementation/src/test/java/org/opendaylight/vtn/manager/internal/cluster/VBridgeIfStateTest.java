/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * JUnit test for {@link VBridgeIfState}.
 */
public class VBridgeIfStateTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            VBridgeIfState ist = new VBridgeIfState(state);
            assertSame(state, ist.getState());
            assertSame(VNodeState.UNKNOWN, ist.getPortState());
            assertNull(ist.getMappedPort());

            for (NodeConnector nc: createNodeConnectors(50)) {
                for (VNodeState pstate: states) {
                    ist = new VBridgeIfState(state, pstate, nc);
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
        VBridgeIfState ist = new VBridgeIfState(VNodeState.UNKNOWN);
        assertSame(VNodeState.UNKNOWN, ist.getState());
        assertSame(VNodeState.UNKNOWN, ist.getPortState());
        assertFalse(ist.isDirty());

        // Change interface state.
        VNodeState[] states = {
            VNodeState.UP,
            VNodeState.DOWN,
            VNodeState.UNKNOWN,
        };
        for (VNodeState state: states) {
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
        for (VNodeState state: states) {
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
        ist.setPortState(VNodeState.UP);
        assertSame(VNodeState.UP, ist.getPortState());
        assertTrue(ist.isDirty());
        assertFalse(ist.isDirty());
        ist.setMappedPort(null);
        assertNull(ist.getMappedPort());
        assertSame(VNodeState.UNKNOWN, ist.getPortState());
        assertTrue(ist.isDirty());
        assertFalse(ist.isDirty());
    }

    /**
     * Test case for {@link VBridgeIfState#equals(Object)} and
     * {@link VBridgeIfState#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        VNodeState[] states = VNodeState.values();
        List<NodeConnector> connectors = createNodeConnectors(50);
        for (VNodeState state: states) {
            for (VNodeState pstate: states) {
                for (NodeConnector nc: connectors) {
                    VBridgeIfState ist1 =
                        new VBridgeIfState(state, pstate, nc);
                    VBridgeIfState ist2 =
                        new VBridgeIfState(state, pstate, copy(nc));
                    testEquals(set, ist1, ist2);
                }
            }
        }

        int required = states.length * states.length * connectors.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VBridgeIfState#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VBridgeIfState[";
        String suffix = "]";
        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            for (VNodeState pstate: states) {
                for (NodeConnector nc: createNodeConnectors(50)) {
                    VBridgeIfState ist = new VBridgeIfState(state, pstate, nc);
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
     * Ensure that {@link VBridgeIfState} is serializable.
     */
    @Test
    public void testSerialize() {
        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            for (VNodeState pstate: states) {
                for (NodeConnector nc: createNodeConnectors(50)) {
                    VBridgeIfState ist = new VBridgeIfState(state, pstate, nc);
                    serializeTest(ist);
                }
            }
        }
    }
}
