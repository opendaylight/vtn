/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.inventory.L2Host;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSourceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.vlan.match.fields.VlanIdBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link FlowMatchUtils}.
 */
public class FlowMatchUtilsTest extends TestBase {
    /**
     * Test case for {@link FlowMatchUtils#getSourceHost(Match)}.
     */
    @Test
    public void testSourceHost() {
        // No Ethernet match.
        MatchBuilder mb = new MatchBuilder();
        Match match = mb.build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));

        // Only destination address is specified.
        MacAddress dst = new EtherAddress(0x12345678L).getMacAddress();
        EthernetDestination edst = new EthernetDestinationBuilder().
            setAddress(dst).build();
        EthernetMatchBuilder emb = new EthernetMatchBuilder().
            setEthernetDestination(edst);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));

        // Set the ingress port.
        SalPort port = new SalPort(1L, 10L);
        match = mb.setInPort(port.getNodeConnectorId()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));

        // Source address with mask is specified.
        EtherAddress eaddr = new EtherAddress(0xaabbccddeeL);
        MacAddress src = eaddr.getMacAddress();
        MacAddress mask = new MacAddress("ff:ff:ff:ff:00:00");
        EthernetSource esrc = new EthernetSourceBuilder().
            setAddress(src).setMask(mask).build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));

        // Set an empty VLAN match.
        VlanMatchBuilder vmb = new VlanMatchBuilder();
        match = mb.setVlanMatch(vmb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));

        // Specify untagged frame.
        VlanId vid = new VlanId(0);
        L2Host host = new L2Host((MacAddress)null, 0, port);
        VlanIdBuilder vib = new VlanIdBuilder().setVlanIdPresent(false);
        match = mb.setVlanMatch(vmb.setVlanId(vib.build()).build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));

        // Clear MAC address mask.
        esrc = new EthernetSourceBuilder().
            setAddress(src).build();
        emb.setEthernetSource(esrc);
        host = new L2Host(eaddr.getAddress(), 0, port);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));

        // Specify VLAN ID 4095.
        vid = new VlanId(4095);
        host = new L2Host(eaddr.getAddress(), 4095, port);
        vib.setVlanIdPresent(true).setVlanId(vid);
        match = mb.setVlanMatch(vmb.setVlanId(vib.build()).build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));

        // Clear source address.
        host = new L2Host((MacAddress)null, 4095, port);
        esrc = new EthernetSourceBuilder().build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));

        // Set the source MAC address again, and clear VLAN match.
        esrc = new EthernetSourceBuilder().setAddress(src).build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).setVlanMatch(null).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
    }

    /**
     * Test case for {@link FlowMatchUtils#getSourceMacAddress(Match)}.
     */
    @Test
    public void testGetSourceMacAddress() {
        // No Ethernet match.
        MatchBuilder mb = new MatchBuilder();
        Match match = mb.build();
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));

        // Only destination address is specified.
        MacAddress dst = new EtherAddress(0x777777777L).getMacAddress();
        EthernetDestination edst = new EthernetDestinationBuilder().
            setAddress(dst).build();
        EthernetMatchBuilder emb = new EthernetMatchBuilder().
            setEthernetDestination(edst);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));

        // Source address with mask is specified.
        MacAddress src = new EtherAddress(0xdeadbeef1234L).getMacAddress();
        MacAddress mask = new MacAddress("ff:ff:ff:ff:ff:ff");
        EthernetSource esrc = new EthernetSourceBuilder().
            setAddress(src).setMask(mask).build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));

        // Clear MAC address mask.
        esrc = new EthernetSourceBuilder().
            setAddress(src).build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(src, FlowMatchUtils.getSourceMacAddress(match));
    }

    /**
     * Test case for {@link FlowMatchUtils#getDestinationMacAddress(Match)}.
     */
    @Test
    public void testGetDestinationMacAddress() {
        // No Ethernet match.
        MatchBuilder mb = new MatchBuilder();
        Match match = mb.build();
        assertEquals(null, FlowMatchUtils.getDestinationMacAddress(match));

        // Only source address is specified.
        MacAddress src = new EtherAddress(0x3141592L).getMacAddress();
        EthernetSource esrc = new EthernetSourceBuilder().
            setAddress(src).build();
        EthernetMatchBuilder emb = new EthernetMatchBuilder().
            setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getDestinationMacAddress(match));

        // Destination address with mask is specified.
        MacAddress dst = new EtherAddress(0x112233445566L).getMacAddress();
        MacAddress mask = new MacAddress("ff:ff:ff:ff:ff:f0");
        EthernetDestination edst = new EthernetDestinationBuilder().
            setAddress(dst).setMask(mask).build();
        emb.setEthernetDestination(edst);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getDestinationMacAddress(match));

        // Clear MAC address mask.
        edst = new EthernetDestinationBuilder().
            setAddress(dst).build();
        emb.setEthernetDestination(edst);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(dst, FlowMatchUtils.getDestinationMacAddress(match));
    }

    /**
     * Test case for {@link FlowMatchUtils#getVlanId(Match)}.
     */
    @Test
    public void testGetVlanId() {
        // No VLAN match.
        MatchBuilder mb = new MatchBuilder();
        Match match = mb.build();
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Empty VLAN match.
        VlanMatchBuilder vmb = new VlanMatchBuilder();
        match = mb.setVlanMatch(vmb.build()).build();
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Specify untagged frame.
        VlanId vid = new VlanId(0);
        VlanIdBuilder vib = new VlanIdBuilder().setVlanIdPresent(false);
        match = mb.setVlanMatch(vmb.setVlanId(vib.build()).build()).build();
        assertEquals(vid, FlowMatchUtils.getVlanId(match));

        // Set VLAN ID.
        int[] vids = {1, 3, 100, 500, 3000, 4094, 4095};
        for (int vlan: vids) {
            vid = new VlanId(vlan);
            vib.setVlanIdPresent(true).setVlanId(vid);
            match = mb.setVlanMatch(vmb.setVlanId(vib.build()).build()).
                build();
            assertEquals(vid, FlowMatchUtils.getVlanId(match));
        }
    }

    /**
     * Test case for {@link FlowMatchUtils#getIngressPort(Match)}.
     */
    @Test
    public void testGetIngressPort() {
        assertEquals(null, FlowMatchUtils.getIngressPort((Match)null));

        MatchBuilder mb = new MatchBuilder();
        assertEquals(null, FlowMatchUtils.getIngressPort(mb.build()));

        NodeConnectorId phys = new NodeConnectorId("openflow:1:2");
        mb.setInPhyPort(phys);
        assertEquals(null, FlowMatchUtils.getIngressPort(mb.build()));

        NodeConnectorId in = new NodeConnectorId("openflow:123:456");
        mb.setInPort(in);
        assertEquals(in, FlowMatchUtils.getIngressPort(mb.build()));
    }
}
