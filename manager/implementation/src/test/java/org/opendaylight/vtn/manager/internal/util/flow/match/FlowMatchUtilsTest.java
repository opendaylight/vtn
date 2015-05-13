/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.L2Host;
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
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowMatchUtils#getSourceHost(Match)}</li>
     *   <li>{@link FlowMatchUtils#getSourceMacAddress(Match)}</li>
     *   <li>{@link FlowMatchUtils#getVlanId(Match)}</li>
     * </ul>
     */
    @Test
    public void testSourceHost() {
        // No Ethernet match.
        MatchBuilder mb = new MatchBuilder();
        Match match = mb.build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Only destination address is specified.
        MacAddress dst = new EtherAddress(0x12345678L).getMacAddress();
        EthernetDestination edst = new EthernetDestinationBuilder().
            setAddress(dst).build();
        EthernetMatchBuilder emb = new EthernetMatchBuilder().
            setEthernetDestination(edst);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Set the ingress port.
        SalPort port = new SalPort(1L, 10L);
        match = mb.setInPort(port.getNodeConnectorId()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Source address is specified.
        EtherAddress eaddr = new EtherAddress(0xaabbccddeeL);
        MacAddress src = eaddr.getMacAddress();
        EthernetSource esrc = new EthernetSourceBuilder().
            setAddress(src).build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
        assertEquals(src, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Set an empty VLAN match.
        VlanMatchBuilder vmb = new VlanMatchBuilder();
        match = mb.setVlanMatch(vmb.build()).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
        assertEquals(src, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(null, FlowMatchUtils.getVlanId(match));

        // Specify untagged frame.
        VlanId vid = new VlanId(0);
        L2Host host = new L2Host(eaddr.getAddress(), (short)0,
                                 port.getAdNodeConnector());
        VlanIdBuilder vib = new VlanIdBuilder().setVlanIdPresent(false);
        match = mb.setVlanMatch(vmb.setVlanId(vib.build()).build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));
        assertEquals(src, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(vid, FlowMatchUtils.getVlanId(match));

        // Specify VLAN ID 4095.
        vid = new VlanId(4095);
        host = new L2Host(eaddr.getAddress(), (short)4095,
                          port.getAdNodeConnector());
        vib.setVlanIdPresent(true).setVlanId(vid);
        match = mb.setVlanMatch(vmb.setVlanId(vib.build()).build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));
        assertEquals(src, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(vid, FlowMatchUtils.getVlanId(match));

        // Clear source address.
        host = new L2Host((MacAddress)null, 4095, port);
        esrc = new EthernetSourceBuilder().build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).build();
        assertEquals(host, FlowMatchUtils.getSourceHost(match));
        assertEquals(null, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(vid, FlowMatchUtils.getVlanId(match));

        // Set the source MAC address again, and clear VLAN match.
        esrc = new EthernetSourceBuilder().setAddress(src).build();
        emb.setEthernetSource(esrc);
        match = mb.setEthernetMatch(emb.build()).setVlanMatch(null).build();
        assertEquals(null, FlowMatchUtils.getSourceHost(match));
        assertEquals(src, FlowMatchUtils.getSourceMacAddress(match));
        assertEquals(null, FlowMatchUtils.getVlanId(match));
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
