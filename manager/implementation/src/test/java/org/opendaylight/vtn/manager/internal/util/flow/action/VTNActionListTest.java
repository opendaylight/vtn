/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createDropAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createOutputAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createPopVlanAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createPushVlanAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetDlDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetDlSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetNwDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetNwSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetNwTosAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetTpDstAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetTpSrcAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetVlanIdAction;
import static org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtilsTest.createSetVlanPcpAction;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.util.OrderedComparator;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Instructions;

/**
 * JUnit test for {@link VTNActionList}.
 */
public class VTNActionListTest extends TestBase {
    /**
     * Test case for flow action that drops every packet.
     */
    @Test
    public void testDrop() {
        VTNActionList actions = new VTNActionList();
        List<Action> expected = new ArrayList<>();
        expected.add(createDropAction(0));

        Instructions insts = actions.toInstructions();
        assertEquals(expected, FlowActionUtils.getActions(insts, null));

        insts = actions.addAll(null).toInstructions();
        assertEquals(expected, FlowActionUtils.getActions(insts, null));

        insts = actions.addAll(Collections.<VTNFlowAction>emptySet()).
            toInstructions();
        assertEquals(expected, FlowActionUtils.getActions(insts, null));

        // addVlanAction() should do nothing if the same VLAN IDs are
        // specified.
        int[] vids = {0, 3, 100, 4095};
        for (int vid: vids) {
            actions.addVlanAction(vid, vid);
            insts = actions.toInstructions();
            assertEquals(expected, FlowActionUtils.getActions(insts, null));
        }
    }

    /**
     * Test case for flow action that forwards packet.
     */
    @Test
    public void testForward() {
        EtherAddress dlSrc = new EtherAddress(0x1122334455L);
        EtherAddress dlDst = new EtherAddress(0xabcdef01234L);
        Ip4Network nwSrc = new Ip4Network("192.168.33.55");
        Ip4Network nwDst = new Ip4Network("10.20.30.40");
        short dscp = 60;
        short pcp = 7;
        int portSrc = 1234;
        int portDst = 60000;
        List<VTNFlowAction> vactions = new ArrayList<>();
        Collections.addAll(
            vactions,
            new VTNSetDlSrcAction(dlSrc),
            new VTNSetDlDstAction(dlDst),
            new VTNSetInetSrcAction(nwSrc),
            new VTNSetInetDstAction(nwDst),
            new VTNSetInetDscpAction(dscp),
            new VTNSetVlanPcpAction(pcp),
            new VTNSetPortSrcAction(portSrc),
            new VTNSetPortDstAction(portDst));

        List<Action> base = new ArrayList<>();
        Collections.addAll(
            base,
            createSetDlSrcAction(0, dlSrc.getMacAddress()),
            createSetDlDstAction(1, dlDst.getMacAddress()),
            createSetNwSrcAction(2, nwSrc.getMdAddress()),
            createSetNwDstAction(3, nwDst.getMdAddress()),
            createSetNwTosAction(4, dscp),
            createSetVlanPcpAction(5, pcp),
            createSetTpSrcAction(6, portSrc),
            createSetTpDstAction(7, portDst));

        OrderedComparator comp = new OrderedComparator();
        String[] ports = {
            "openflow:1:2",
            "openflow:12302652056939934532:67",
        };

        // Forward packet without changing VLAN ID.
        int[] vids = {0, 3, 100, 4095};
        for (int vid: vids) {
            for (String port: ports) {
                SalPort sport = SalPort.create(port);
                VTNActionList actions = new VTNActionList().
                    addAll(vactions).
                    addVlanAction(vid, vid).
                    addOutputAction(sport);
                List<Action> expected = new ArrayList<>(base);
                expected.add(createOutputAction(base.size(), port));

                Instructions insts = new VTNActionList().
                    addAll(vactions).
                    addVlanAction(vid, vid).
                    addOutputAction(sport).
                    toInstructions();
                List<Action> results = FlowActionUtils.getActions(insts, comp);
                assertEquals(expected, results);
            }
        }

        vids = new int[]{1, 3, 128, 3333, 4095};
        for (int vid: vids) {
            for (String port: ports) {
                // Add VLAN tag.
                SalPort sport = SalPort.create(port);
                List<Action> expected = new ArrayList<>(base);
                int order = base.size();
                Collections.addAll(
                    expected,
                    createPushVlanAction(order++),
                    createSetVlanIdAction(order++, vid),
                    createOutputAction(order++, port));
                Instructions insts = new VTNActionList().
                    addAll(vactions).
                    addVlanAction(0, vid).
                    addOutputAction(sport).
                    toInstructions();
                List<Action> results = FlowActionUtils.getActions(insts, comp);
                assertEquals(expected, results);

                // Strip VLAN tag.
                expected = new ArrayList<>(base);
                order = base.size();
                Collections.addAll(
                    expected,
                    createPopVlanAction(order++),
                    createOutputAction(order++, port));
                insts = new VTNActionList().
                    addAll(vactions).
                    addVlanAction(vid, 0).
                    addOutputAction(sport).
                    toInstructions();
                results = FlowActionUtils.getActions(insts, comp);
                assertEquals(expected, results);

                // Set VLAN ID into existing VLAN tag.
                int oldVid = (vid == 4095) ? 1 : vid + 1;
                expected = new ArrayList<>(base);
                order = base.size();
                Collections.addAll(
                    expected,
                    createSetVlanIdAction(order++, vid),
                    createOutputAction(order++, port));
                insts = new VTNActionList().
                    addAll(vactions).
                    addVlanAction(oldVid, vid).
                    addOutputAction(sport).
                    toInstructions();
                results = FlowActionUtils.getActions(insts, comp);
                assertEquals(expected, results);
            }
        }
    }
}
