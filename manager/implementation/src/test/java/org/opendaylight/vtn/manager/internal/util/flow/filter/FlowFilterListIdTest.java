/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTerminalIfIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit tset for {@link FlowFilterListId}.
 */
public class FlowFilterListIdTest extends TestBase {

    /**
     * Test case for {@link FlowFilterListId#getFlowDirectionName(boolean)}.
     */
    @Test
    public void testGetFlowDirectionName() {
        assertEquals("OUT", FlowFilterListId.getFlowDirectionName(true));
        assertEquals("IN", FlowFilterListId.getFlowDirectionName(false));
    }

    /**
     * Test case for {@link FlowFilterListId#getIdentifier()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetIdentifier() throws Exception {
        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers,
            VTenantIdentifier.create("vtn_1", false),
            VBridgeIdentifier.create("vtn_1", "vbr_1", false),
            VBridgeIfIdentifier.create("vtn_2", "vbr_2", "if_1", false),
            VTerminalIfIdentifier.create("vtn_2", "vterm_2", "if_2", false));
        for (VNodeIdentifier<?> ident: identifiers) {
            FlowFilterListId flid = new FlowFilterListId(ident, true);
            assertEquals(ident, flid.getIdentifier());
        }
    }

    /**
     * Test case for {@link FlowFilterListId#isOutput()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIsOutput() throws Exception {
        VNodeIdentifier<?> ident = VTenantIdentifier.create("vtn_1", false);
        boolean[] bools = {true, false};
        for (boolean out: bools) {
            FlowFilterListId flid = new FlowFilterListId(ident, out);
            assertEquals(out, flid.isOutput());
        }
    }

    /**
     * Test case for {@link FlowFilterListId#getFilterId(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetFieldId() throws Exception {
        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers,
            VTenantIdentifier.create("vtn_1", false),
            VBridgeIdentifier.create("vtn_1", "vbr_1", false),
            VBridgeIfIdentifier.create("vtn_2", "vbr_2", "if_1", false),
            VTerminalIfIdentifier.create("vtn_2", "vterm_2", "if_2", false));
        boolean[] bools = {true, false};
        Integer[] indices = {1, 22, 55, 44444, 65535};

        for (boolean out: bools) {
            String dir = (out) ? "OUT" : "IN";
            for (VNodeIdentifier<?> ident: identifiers) {
                FlowFilterListId flid = new FlowFilterListId(ident, out);
                for (int index: indices) {
                    String expected = ident.toString() + "%" + dir + "." +
                        index;
                    assertEquals(expected, flid.getFilterId(index));
                }
            }
        }
    }

    /**
     * Test case for {@link FlowFilterListId#equals(Object)} and
     * {@link FlowFilterListId#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        String[] tnames = {"vtn_1", "vtn_2"};
        String[] bnames = {"bridge_1", "bridge_2"};
        String[] inames = {"if_1", "if_2"};
        boolean[] bools = {true, false};

        int count = 0;
        for (boolean out: bools) {
            for (String tname: tnames) {
                VNodeIdentifier<?> ident1 =
                    VTenantIdentifier.create(tname, false);
                VNodeIdentifier<?> ident2 =
                    VTenantIdentifier.create(tname, false);
                FlowFilterListId flid1 = new FlowFilterListId(ident1, out);
                FlowFilterListId flid2 = new FlowFilterListId(ident2, out);
                testEquals(set, flid1, flid2);
                count++;

                for (String bname: bnames) {
                    ident1 = VBridgeIdentifier.create(tname, bname, false);
                    ident2 = VBridgeIdentifier.create(tname, bname, false);
                    flid1 = new FlowFilterListId(ident1, out);
                    flid2 = new FlowFilterListId(ident2, out);
                    testEquals(set, flid1, flid2);
                    count++;

                    for (String iname: inames) {
                        ident1 = VBridgeIfIdentifier.
                            create(tname, bname, iname, false);
                        ident2 = VBridgeIfIdentifier.
                            create(tname, bname, iname, false);
                        flid1 = new FlowFilterListId(ident1, out);
                        flid2 = new FlowFilterListId(ident2, out);
                        testEquals(set, flid1, flid2);
                        count++;

                        ident1 = VTerminalIfIdentifier.
                            create(tname, bname, iname, false);
                        ident2 = VTerminalIfIdentifier.
                            create(tname, bname, iname, false);
                        flid1 = new FlowFilterListId(ident1, out);
                        flid2 = new FlowFilterListId(ident2, out);
                        testEquals(set, flid1, flid2);
                        count++;
                    }
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link FlowFilterListId#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers,
            VTenantIdentifier.create("vtn_1", false),
            VBridgeIdentifier.create("vtn_1", "vbr_1", false),
            VBridgeIfIdentifier.create("vtn_2", "vbr_2", "if_1", false),
            VTerminalIfIdentifier.create("vtn_2", "vterm_2", "if_2", false));
        boolean[] bools = {true, false};

        for (boolean out: bools) {
            String dir = (out) ? "OUT" : "IN";
            for (VNodeIdentifier<?> ident: identifiers) {
                FlowFilterListId flid = new FlowFilterListId(ident, out);
                String expected = ident.toString() + "%" + dir;
                String id = flid.toString();
                assertEquals(expected, id);

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(id, flid.toString());
                }
            }
        }
    }
}
