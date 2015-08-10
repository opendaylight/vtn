/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Destination;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.DestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Source;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.SourceBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkBuilder;

/**
 * JUnit test for {@link LinkEdge}.
 */
public class LinkEdgeTest extends TestBase {
    /**
     * Test case for constructor and getter methods.
     *
     * <ul>
     *   <li>{@link LinkEdge#LinkEdge(VtnLink)}</li>
     *   <li>{@link LinkEdge#getSourcePort()}</li>
     *   <li>{@link LinkEdge#getDestinationPort()}</li>
     *   <li>{@link LinkEdge#toString()}</li>
     * </ul>
     */
    @Test
    public void testVtnLink() {
        NodeConnectorId nc = new NodeConnectorId("openflow:1:2");
        NodeConnectorId[] invalid = {
            null,
            new NodeConnectorId("invalid"),
            new NodeConnectorId("unknown_proto:1:2"),
        };

        // Invalid source.
        for (NodeConnectorId inv: invalid) {
            VtnLink vlink = new VtnLinkBuilder().setSource(inv).
                setDestination(nc).build();
            try {
                new LinkEdge(vlink);
                unexpected();
            } catch (IllegalArgumentException e) {
                String expected = "Source port is not configured: " + vlink;
                assertEquals(expected, e.getMessage());
            }
        }

        // Invalid destination.
        for (NodeConnectorId inv: invalid) {
            VtnLink vlink = new VtnLinkBuilder().setSource(nc).
                setDestination(inv).build();
            try {
                new LinkEdge(vlink);
                unexpected();
            } catch (IllegalArgumentException e) {
                String expected = "Destination port is not configured: " +
                    vlink;
                assertEquals(expected, e.getMessage());
            }
        }

        NodeConnectorId[] source = {
            nc,
            new NodeConnectorId("openflow:10:33"),
            new NodeConnectorId("openflow:10:34"),
            new NodeConnectorId("openflow:11:123"),
        };
        NodeConnectorId[] destination = {
            new NodeConnectorId("openflow:2:1"),
            new NodeConnectorId("openflow:2:56"),
            new NodeConnectorId("openflow:12:345"),
            new NodeConnectorId("openflow:333:444"),
        };

        for (int i = 0; i < source.length; i++) {
            NodeConnectorId src = source[i];
            NodeConnectorId dst = destination[i];
            VtnLink vlink = new VtnLinkBuilder().setSource(src).
                setDestination(dst).build();
            LinkEdge le = new LinkEdge(vlink);
            SalPort srcPort = SalPort.create(src.getValue());
            SalPort dstPort = SalPort.create(dst.getValue());
            assertNotNull(srcPort);
            assertNotNull(dstPort);
            assertEquals(srcPort, le.getSourcePort());
            assertEquals(dstPort, le.getDestinationPort());
            toStringTest(le, src.getValue(), dst.getValue());

            vlink = new VtnLinkBuilder().setSource(dst).
                setDestination(src).build();
            le = new LinkEdge(vlink);
            assertEquals(dstPort, le.getSourcePort());
            assertEquals(srcPort, le.getDestinationPort());
            toStringTest(le, dst.getValue(), src.getValue());
        }
    }

    /**
     * Test case for constructor and getter methods.
     *
     * <ul>
     *   <li>{@link LinkEdge#LinkEdge(Link)}</li>
     *   <li>{@link LinkEdge#getSourcePort()}</li>
     *   <li>{@link LinkEdge#getDestinationPort()}</li>
     *   <li>{@link LinkEdge#toString()}</li>
     * </ul>
     */
    @Test
    public void testLink() {
        TpId tpid = new TpId("openflow:1:2");
        TpId[] invalid = {
            null,
            new TpId("invalid"),
            new TpId("unknown_proto:1:2"),
        };

        List<Source> sourceList = new ArrayList<>();
        List<Destination> destinationList = new ArrayList<>();
        sourceList.add(null);
        sourceList.add(new SourceBuilder().build());
        destinationList.add(null);
        destinationList.add(new DestinationBuilder().build());
        for (TpId inv: invalid) {
            Source s = new SourceBuilder().setSourceTp(inv).build();
            sourceList.add(s);

            Destination d = new DestinationBuilder().setDestTp(inv).build();
            destinationList.add(d);
        }

        // Invalid source.
        for (Source src: sourceList) {
            Destination d = new DestinationBuilder().setDestTp(tpid).build();
            Link link = new LinkBuilder().setSource(src).setDestination(d).
                build();
            try {
                new LinkEdge(link);
                unexpected();
            } catch (IllegalArgumentException e) {
                String expected = "Source port is not configured: " + link;
                assertEquals(expected, e.getMessage());
            }
        }

        // Invalid destination.
        for (Destination dst: destinationList) {
            Source s = new SourceBuilder().setSourceTp(tpid).build();
            Link link = new LinkBuilder().setSource(s).setDestination(dst).
                build();
            try {
                new LinkEdge(link);
                unexpected();
            } catch (IllegalArgumentException e) {
                String expected = "Destination port is not configured: " +
                    link;
                assertEquals(expected, e.getMessage());
            }
        }

        TpId[] source = {
            tpid,
            new TpId("openflow:10:33"),
            new TpId("openflow:10:34"),
            new TpId("openflow:11:123"),
        };
        TpId[] destination = {
            new TpId("openflow:2:1"),
            new TpId("openflow:2:56"),
            new TpId("openflow:12:345"),
            new TpId("openflow:333:444"),
        };

        for (int i = 0; i < source.length; i++) {
            TpId src = source[i];
            TpId dst = destination[i];
            Source s = new SourceBuilder().setSourceTp(src).build();
            Destination d = new DestinationBuilder().setDestTp(dst).build();
            Link link = new LinkBuilder().setSource(s).setDestination(d).
                build();
            LinkEdge le = new LinkEdge(link);
            SalPort srcPort = SalPort.create(src.getValue());
            SalPort dstPort = SalPort.create(dst.getValue());
            assertNotNull(srcPort);
            assertNotNull(dstPort);
            assertEquals(srcPort, le.getSourcePort());
            assertEquals(dstPort, le.getDestinationPort());
            toStringTest(le, src.getValue(), dst.getValue());

            s = new SourceBuilder().setSourceTp(dst).build();
            d = new DestinationBuilder().setDestTp(src).build();
            link = new LinkBuilder().setSource(s).setDestination(d).build();
            le = new LinkEdge(link);
            assertEquals(dstPort, le.getSourcePort());
            assertEquals(srcPort, le.getDestinationPort());
            toStringTest(le, dst.getValue(), src.getValue());
        }
    }

    /**
     * Test for {@link LinkEdge#equals(Object)} and
     * {@link LinkEdge#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        NodeConnectorId[] source = {
            new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:1:2"),
            new NodeConnectorId("openflow:1:3"),
            new NodeConnectorId("openflow:10:33"),
            new NodeConnectorId("openflow:10:34"),
            new NodeConnectorId("openflow:10:35"),
            new NodeConnectorId("openflow:11:123"),
            new NodeConnectorId("openflow:11:124"),
            new NodeConnectorId("openflow:11:125"),
        };
        NodeConnectorId[] destination = {
            new NodeConnectorId("openflow:1:4"),
            new NodeConnectorId("openflow:1:5"),
            new NodeConnectorId("openflow:1:6"),
            new NodeConnectorId("openflow:2:33"),
            new NodeConnectorId("openflow:2:34"),
            new NodeConnectorId("openflow:2:35"),
            new NodeConnectorId("openflow:30:40"),
            new NodeConnectorId("openflow:30:41"),
            new NodeConnectorId("openflow:30:42"),
        };

        for (int i = 0; i < source.length; i++) {
            NodeConnectorId src = source[i];
            NodeConnectorId dst = destination[i];
            VtnLink vlink = new VtnLinkBuilder().setSource(src).
                setDestination(dst).build();
            LinkEdge le1 = new LinkEdge(vlink);
            vlink = new VtnLinkBuilder().setSource(new NodeConnectorId(src)).
                setDestination(new NodeConnectorId(dst)).build();
            LinkEdge le2 = new LinkEdge(vlink);
            testEquals(set, le1, le2);

            vlink = new VtnLinkBuilder().setSource(dst).
                setDestination(src).build();
            le1 = new LinkEdge(vlink);
            vlink = new VtnLinkBuilder().setSource(new NodeConnectorId(dst)).
                setDestination(new NodeConnectorId(src)).build();
            le2 = new LinkEdge(vlink);
            testEquals(set, le1, le2);
        }

        assertEquals(source.length * 2, set.size());
    }

    /**
     * Verify results of {@link SalPort#toString()}.
     *
     * @param le   A {@link LinkEdge} instance to be tested.
     * @param src  The source port identifier.
     * @param dst  The destination port identifier.
     */
    private void toStringTest(LinkEdge le, String src, String dst) {
        String expected = "LinkEdge[" + src + " -> " + dst + "]";
        assertEquals(expected, le.toString());
    }
}
