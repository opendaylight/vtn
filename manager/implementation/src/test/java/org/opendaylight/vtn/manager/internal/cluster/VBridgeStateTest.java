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
import java.util.Set;

import org.junit.Test;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * Junit test for {@link VBridgeState}.
 */
public class VBridgeStateTest extends TestBase {
    private Set<ObjectPair<Node, Node>> fp = new HashSet<ObjectPair<Node, Node>>();
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VNodeState[] states = VNodeState.values();

        for (VNodeState state: states) {
            VBridgeState bst = new VBridgeState(state);
            assertSame(state, bst.getState());
            assertEquals(fp, bst.getFaultedPaths());
        }
    }

    /**
     * Test case for {@link VBridgeState#equals(object)} and {@link VBridgeState#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        VNodeState[] states = VNodeState.values();

        for (VNodeState state: states) {
            VBridgeState bst1 = new VBridgeState(state);
            VBridgeState bst2 = new VBridgeState(state);

            testEquals(set, bst1, bst2);
        }

        int required = states.length;
        assertEquals(required, set.size());

    }

    /**
     * Test case for {@link VBridgeState#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VBridgeState[";
        String suffix = "]";

        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            VBridgeState bst = new VBridgeState(state);
            String s = "state=" + state.toString();
            String f = "faulted=" + fp.toString();
            String required = joinStrings(prefix, suffix, ",", s, f);

            assertEquals(required, bst.toString());
        }
    }

    /**
     * Ensure that {@link VBridgeState} is serializable.
     */
    @Test
    public void testSerialize() {
        VNodeState[] states = VNodeState.values();
        for (VNodeState state: states) {
            VBridgeState bst = new VBridgeState(state);
            serializeTest(bst);
        }
    }
}
