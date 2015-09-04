/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePortsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortKey;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Destination;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.DestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Source;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.SourceBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkKey;

/**
 * JUnit test for {@link LinkUpdateContext}.
 */
public class LinkUpdateContextTest extends TestBase {

    /**
     * Create a new {@link StaticSwitchLinks} instance.
     *
     * @param swl  An array of {@link StaticSwitchLink} instances to be
     *             configured in a new {@link StaticSwitchLinks} instance.
     * @return  A {@link StaticSwitchLinks} instance which contains the given
     *          configuration.
     */
    public static StaticSwitchLinks newStaticSwitchLinks(
        StaticSwitchLink ... swl) {
        List<StaticSwitchLink> swlinks = new ArrayList<>();
        Collections.addAll(swlinks, swl);

        return new StaticSwitchLinksBuilder().
            setStaticSwitchLink(swlinks).build();
    }

    /**
     * Create a new {@link StaticEdgePorts} instance.
     *
     * @param edges  An array of {@link SalPort} instances to be configured
     *               in a new {@link StaticEdgePorts} instance.
     * @return  A {@link StaticEdgePorts} instance which contains the given
     *          configuration.
     */
    public static StaticEdgePorts newStaticEdgePorts(SalPort ... edges) {
        List<StaticEdgePort> eplist = new ArrayList<>();
        for (SalPort sport: edges) {
            StaticEdgePort ep = new StaticEdgePortBuilder().
                setPort(sport.getNodeConnectorId()).build();
            eplist.add(ep);
        }

        return new StaticEdgePortsBuilder().
            setStaticEdgePort(eplist).build();
    }

    /**
     * Create a new MD-SAL link.
     *
     * @param lid   Link identifier.
     * @param snid  Source node idenfifier.
     * @param src   Source port identifier.
     * @param dnid  Destination node identifier.
     * @param dst   Destination port identifier.
     * @return  A {@link Link} instance.
     */
    public static Link newMdLink(String lid, String snid, String src,
                                 String dnid, String dst) {
        Source source = new SourceBuilder().
            setSourceNode(new NodeId(snid)).
            setSourceTp(new TpId(src)).
            build();
        Destination destination = new DestinationBuilder().
            setDestNode(new NodeId(dnid)).
            setDestTp(new TpId(dst)).
            build();

        return new LinkBuilder().
            setLinkId(new LinkId(lid)).
            setSource(source).
            setDestination(destination).
            build();
    }

    /**
     * Create a new MD-SAL link.
     *
     * @param src  A {@link SalPort} instance which specifies the source port
     *             of the link.
     * @param dst  A {@link SalPort} instance which specifies the destination
     *             port of the link.
     * @return  A {@link Link} instance.
     */
    public static Link newMdLink(SalPort src, SalPort dst) {
        String srcNode = src.toNodeString();
        String srcId = src.toString();
        String dstNode = dst.toNodeString();
        String dstId = dst.toString();
        return newMdLink(srcId, srcNode, srcId, dstNode, dstId);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#addVtnLink(LinkId,SalPort,SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddVtnLink() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        SalPort src = new SalPort(1L, 2L);
        SalPort dst = new SalPort(10L, 20L);
        LinkId lid = new LinkId(src.toString());
        InstanceIdentifier<IgnoredLink> ipath = InstanceIdentifier.
            builder(IgnoredLinks.class).
            child(IgnoredLink.class, new IgnoredLinkKey(lid)).build();
        IgnoredLink ignored = new IgnoredLinkBuilder().
            setLinkId(lid).setSource(src.getNodeConnectorId()).
            setDestination(dst.getNodeConnectorId()).build();
        InstanceIdentifier<VtnLink> vpath = InstanceIdentifier.
            builder(VtnTopology.class).
            child(VtnLink.class, new VtnLinkKey(lid)).build();
        VtnLink vlink = new VtnLinkBuilder().
            setLinkId(lid).setSource(src.getNodeConnectorId()).
            setDestination(dst.getNodeConnectorId()).build();
        InstanceIdentifier<PortLink> srcPath = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, src.getVtnNodeKey()).
            child(VtnPort.class, src.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid)).build();
        PortLink srcLink = new PortLinkBuilder().
            setLinkId(lid).setPeer(dst.getNodeConnectorId()).build();
        InstanceIdentifier<PortLink> dstPath = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, dst.getVtnNodeKey()).
            child(VtnPort.class, dst.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid)).build();
        PortLink dstLink = new PortLinkBuilder().
            setLinkId(lid).setPeer(src.getNodeConnectorId()).build();

        InstanceIdentifier<StaticEdgePort> srcEdgePath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class,
                  new StaticEdgePortKey(src.getNodeConnectorId())).
            build();
        InstanceIdentifier<StaticEdgePort> dstEdgePath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class,
                  new StaticEdgePortKey(dst.getNodeConnectorId())).
            build();
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        StaticEdgePort nullEdge = null;
        when(tx.read(cstore, srcEdgePath)).thenReturn(getReadResult(nullEdge));
        when(tx.read(cstore, dstEdgePath)).thenReturn(getReadResult(nullEdge));

        InstanceIdentifier<StaticSwitchLink> swlinkPath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            child(StaticSwitchLink.class,
                  new StaticSwitchLinkKey(src.getNodeConnectorId())).
            build();
        StaticSwitchLink nullLink = null;
        when(tx.read(cstore, swlinkPath)).thenReturn(getReadResult(nullLink));

        // Both source and destination ports are not present.
        reader.prefetch(src, (VtnPort)null);
        reader.prefetch(dst, (VtnPort)null);
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);
        assertEquals(null, luctx.addVtnLink(lid, src, dst));
        verify(tx).put(oper, ipath, ignored, true);
        verify(tx, never()).
            put(eq(oper), eq(vpath), eq(vlink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(srcPath), eq(srcLink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(dstPath), eq(dstLink), anyBoolean());
        reset(tx);

        Set<IgnoredLink> expectedIgnored = new HashSet<>();
        Set<IgnoredLink> emptyIgnored = Collections.<IgnoredLink>emptySet();
        List<VtnLink> emptyLink = Collections.<VtnLink>emptyList();
        Set<SalPort> emptyPort = Collections.<SalPort>emptySet();
        expectedIgnored.add(ignored);
        assertEquals(expectedIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        String causeFmt = "{}: {}: {} -> {}: cause={}";
        String imsg = "Ignore inter-switch link";
        verify(log).info(causeFmt, imsg, lid.getValue(), src.toString(),
                         dst.toString(), "Source port is not present.");
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);

        // Source port is not present.
        VtnPort dstPort = createVtnPortBuilder(dst, null, true).build();
        reader.prefetch(dst, dstPort);
        luctx = new LinkUpdateContext(tx, reader);
        assertEquals(null, luctx.addVtnLink(lid, src, dst));
        verify(tx).put(oper, ipath, ignored, true);
        verify(tx, never()).
            put(eq(oper), eq(vpath), eq(vlink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(srcPath), eq(srcLink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(dstPath), eq(dstLink), anyBoolean());
        reset(tx);
        assertEquals(expectedIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).info(causeFmt, imsg, lid.getValue(), src.toString(),
                         dst.toString(), "Source port is not present.");
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);

        // Destination port is not present.
        VtnPort srcPort = createVtnPortBuilder(src, null, true).build();
        reader.prefetch(src, srcPort);
        reader.prefetch(dst, (VtnPort)null);
        luctx = new LinkUpdateContext(tx, reader);
        assertEquals(null, luctx.addVtnLink(lid, src, dst));
        verify(tx).put(oper, ipath, ignored, true);
        verify(tx, never()).
            put(eq(oper), eq(vpath), eq(vlink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(srcPath), eq(srcLink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(dstPath), eq(dstLink), anyBoolean());
        reset(tx);
        assertEquals(expectedIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).info(causeFmt, imsg, lid.getValue(), src.toString(),
                         dst.toString(), "Destination port is not present.");
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);

        // Both ports are present.
        reader.prefetch(dst, dstPort);
        luctx = new LinkUpdateContext(tx, reader);
        assertEquals(vlink, luctx.addVtnLink(lid, src, dst));
        verify(tx, never()).
            put(eq(oper), eq(ipath), eq(ignored), anyBoolean());
        verify(tx).put(oper, vpath, vlink, true);
        verify(tx).put(oper, srcPath, srcLink, true);
        verify(tx).put(oper, dstPath, dstLink, true);
        reset(tx);
        assertEquals(emptyIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);

        // Source port is configured as an edge port.
        reader = new InventoryReader(tx);
        reader.prefetch(src, srcPort);
        reader.prefetch(dst, dstPort);
        StaticEdgePort edge = new StaticEdgePortBuilder().
            setPort(src.getNodeConnectorId()).build();
        when(tx.read(cstore, srcEdgePath)).thenReturn(getReadResult(edge));
        when(tx.read(cstore, dstEdgePath)).thenReturn(getReadResult(nullEdge));
        when(tx.read(cstore, swlinkPath)).thenReturn(getReadResult(nullLink));
        luctx = new LinkUpdateContext(tx, reader);
        assertEquals(null, luctx.addVtnLink(lid, src, dst));
        verify(tx).put(oper, ipath, ignored, true);
        verify(tx, never()).
            put(eq(oper), eq(vpath), eq(vlink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(srcPath), eq(srcLink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(dstPath), eq(dstLink), anyBoolean());
        reset(tx);
        assertEquals(expectedIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).info(causeFmt, imsg, lid.getValue(), src.toString(),
                         dst.toString(),
                         "Source port is configured as static edge port.");
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);

        // Source port is configured as an edge port.
        reader = new InventoryReader(tx);
        reader.prefetch(src, srcPort);
        reader.prefetch(dst, dstPort);
        edge = new StaticEdgePortBuilder().
            setPort(dst.getNodeConnectorId()).build();
        when(tx.read(cstore, srcEdgePath)).thenReturn(getReadResult(nullEdge));
        when(tx.read(cstore, dstEdgePath)).thenReturn(getReadResult(edge));
        when(tx.read(cstore, swlinkPath)).thenReturn(getReadResult(nullLink));
        luctx = new LinkUpdateContext(tx, reader);
        assertEquals(null, luctx.addVtnLink(lid, src, dst));
        verify(tx).put(oper, ipath, ignored, true);
        verify(tx, never()).
            put(eq(oper), eq(vpath), eq(vlink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(srcPath), eq(srcLink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(dstPath), eq(dstLink), anyBoolean());
        reset(tx);
        assertEquals(expectedIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).info(causeFmt, imsg, lid.getValue(), src.toString(),
                         dst.toString(),
                         "Destination port is configured as static edge port.");
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);

        // Static inter-switch linik is configured in the source port.
        reader = new InventoryReader(tx);
        reader.prefetch(src, srcPort);
        reader.prefetch(dst, dstPort);
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(src.getNodeConnectorId()).
            setDestination(dst.getNodeConnectorId()).
            build();
        when(tx.read(cstore, srcEdgePath)).thenReturn(getReadResult(nullEdge));
        when(tx.read(cstore, dstEdgePath)).thenReturn(getReadResult(nullEdge));
        when(tx.read(cstore, swlinkPath)).thenReturn(getReadResult(swlink));
        luctx = new LinkUpdateContext(tx, reader);
        assertEquals(null, luctx.addVtnLink(lid, src, dst));
        verify(tx).put(oper, ipath, ignored, true);
        verify(tx, never()).
            put(eq(oper), eq(vpath), eq(vlink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(srcPath), eq(srcLink), anyBoolean());
        verify(tx, never()).
            put(eq(oper), eq(dstPath), eq(dstLink), anyBoolean());
        reset(tx);
        assertEquals(expectedIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).info(causeFmt, imsg, lid.getValue(), src.toString(),
                         dst.toString(),
                         "Static inter-switch link is configured.");
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(log);
    }

    /**
     * Test environment for {@link #testResolveIgnoredLinks()}.
     */
    private final class ResolveLinkEnv {
        /**
         * The mock-up of MD-SAL datastore transaction.
         */
        private final ReadWriteTransaction  transaction =
            mock(ReadWriteTransaction.class);

        /**
         * The mock-up of logger instance.
         */
        private final Logger  logger = mock(Logger.class);

        /**
         * Inventory reader.
         */
        private final InventoryReader  reader =
            new InventoryReader(transaction);

        /**
         * A link update context.
         */
        private final LinkUpdateContext  linkUpdater =
            new LinkUpdateContext(transaction, reader);

        /**
         * A list of {@link IgnoredLink} instances.
         */
        private final List<IgnoredLink>  ignoredLinks = new ArrayList<>();

        /**
         * VTN ignored links to be removed.
         */
        private final Set<InstanceIdentifier<IgnoredLink>> removedLinks =
            new HashSet<>();

        /**
         * VTN links to be created.
         */
        private final Map<InstanceIdentifier<VtnLink>, VtnLink> vtnLinks =
            new HashMap<>();

        /**
         * VTN port links to be created.
         */
        private final Map<InstanceIdentifier<PortLink>, PortLink> portLinks =
            new HashMap<>();

        /**
         * String which indicates the cause why link is not resolved.
         */
        private final Map<InstanceIdentifier<IgnoredLink>, String> causes =
            new HashMap<>();

        /**
         * Construct a new instance.
         */
        private ResolveLinkEnv() {
            when(logger.isDebugEnabled()).thenReturn(true);
        }

        /**
         * Return the mock-up of MD-SAL transaction.
         *
         * @return  A {@link ReadWriteTransaction} instance.
         */
        private ReadWriteTransaction getTransaction() {
            return transaction;
        }

        /**
         * Return the mock-up of logger instance.
         *
         * @return  A {@link Logger} instance.
         */
        private Logger getLogger() {
            return logger;
        }

        /**
         * Return the inventory reader for test.
         *
         * @return  An {@link InventoryReader} instance.
         */
        private InventoryReader getInventoryReader() {
            return reader;
        }

        /**
         * Return the link update context for test.
         *
         * @return  A {@link LinkUpdateContext} instance.
         */
        private LinkUpdateContext getLinkUpdateContext() {
            return linkUpdater;
        }

        /**
         * Add the given link information.
         *
         * @param src         The source port.
         * @param srcPresent  {@code true} means that the source port is
         *                    present.
         * @param dst         The destination port.
         * @param dstPresent  {@code true} means that the destination port is
         *                    present.
         * @throws Exception  An error occurred.
         */
        private void add(SalPort src, boolean srcPresent, SalPort dst,
                         boolean dstPresent) throws Exception {
            LinkId lid = new LinkId(src.toString());
            IgnoredLink ilink = new IgnoredLinkBuilder().
                setLinkId(lid).setSource(src.getNodeConnectorId()).
                setDestination(dst.getNodeConnectorId()).build();
            ignoredLinks.add(ilink);

            VtnPort vport;
            if (srcPresent) {
                vport = createVtnPortBuilder(src, null, true).build();
            } else {
                vport = null;
            }
            reader.prefetch(src, vport);

            if (dstPresent) {
                vport = createVtnPortBuilder(dst, null, true).build();
            } else {
                vport = null;
            }
            reader.prefetch(dst, vport);

            InstanceIdentifier<IgnoredLink> ipath = InstanceIdentifier.
                builder(IgnoredLinks.class).
                child(IgnoredLink.class, new IgnoredLinkKey(lid)).build();
            InstanceIdentifier<VtnLink> vpath = InstanceIdentifier.
                builder(VtnTopology.class).
                child(VtnLink.class, new VtnLinkKey(lid)).build();
            InstanceIdentifier<PortLink> spath = InstanceIdentifier.
                builder(VtnNodes.class).
                child(VtnNode.class, src.getVtnNodeKey()).
                child(VtnPort.class, src.getVtnPortKey()).
                child(PortLink.class, new PortLinkKey(lid)).build();
            InstanceIdentifier<PortLink> dpath = InstanceIdentifier.
                builder(VtnNodes.class).
                child(VtnNode.class, dst.getVtnNodeKey()).
                child(VtnPort.class, dst.getVtnPortKey()).
                child(PortLink.class, new PortLinkKey(lid)).build();
            assertEquals(false, vtnLinks.containsKey(vpath));
            assertEquals(false, portLinks.containsKey(spath));
            assertEquals(false, portLinks.containsKey(dpath));

            VtnLink vlink = null;
            PortLink splink = null;
            PortLink dplink = null;
            if (reader.isStaticEdgePort(src)) {
                causes.put(ipath, "Source port is configured as static " +
                           "edge port.");
            } else if (reader.isStaticEdgePort(dst)) {
                causes.put(ipath, "Destination port is configured as static " +
                           "edge port.");
            } else if (reader.getStaticLink(src) != null) {
                causes.put(ipath, "Static inter-switch link is configured.");
            } else if (srcPresent) {
                if (dstPresent) {
                    assertEquals(true, removedLinks.add(ipath));
                    NodeConnectorId sncid = src.getNodeConnectorId();
                    NodeConnectorId dncid = dst.getNodeConnectorId();
                    vlink = new VtnLinkBuilder().setLinkId(lid).
                        setSource(sncid).setDestination(dncid).build();
                    splink = new PortLinkBuilder().
                        setLinkId(lid).setPeer(dncid).build();
                    dplink = new PortLinkBuilder().
                        setLinkId(lid).setPeer(sncid).build();
                } else {
                    causes.put(ipath, "Destination port is not present.");
                }
            } else {
                causes.put(ipath, "Source port is not present.");
            }

            assertEquals(null, vtnLinks.put(vpath, vlink));
            assertEquals(null, portLinks.put(spath, splink));
            assertEquals(null, portLinks.put(dpath, dplink));
        }

        /**
         * Add configuration for static edge port.
         *
         * @param sport  A {@link SalPort} instance.
         * @param edge   {@code true} if the given port should be configured
         *               as an edge port.
         */
        private void addStaticEdgePort(SalPort sport, boolean edge) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            StaticEdgePort ep = (edge)
                ? null
                : new StaticEdgePortBuilder().setPort(ncId).build();
            LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
            when(transaction.read(cstore, path)).
                thenReturn(getReadResult(ep));
        }

        /**
         * Add configuration for static inter-switch link.
         *
         * @param src  The source port of the link.
         * @param dst  The destination port of the link.
         * @param st  {@code true} if the given link should be configured
         *             statically.
         */
        private void addStaticSwitchLink(SalPort src, SalPort dst,
                                         boolean st) {
            NodeConnectorId srcId = src.getNodeConnectorId();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            StaticSwitchLink swlink;
            if (st) {
                NodeConnectorId dstId = dst.getNodeConnectorId();
                swlink = new StaticSwitchLinkBuilder().
                    setSource(srcId).setDestination(dstId).build();
            } else {
                swlink = null;
            }
            LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
            when(transaction.read(cstore, path)).
                thenReturn(getReadResult(swlink));
        }

        /**
         * Run the test.
         *
         * @throws Exception  An error occurred.
         */
        private void runTest() throws Exception {
            ReadWriteTransaction tx = transaction;
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            LinkUpdateContext luctx = linkUpdater;
            InstanceIdentifier<IgnoredLinks> irpath =
                InstanceIdentifier.create(IgnoredLinks.class);
            IgnoredLinks iroot = new IgnoredLinksBuilder().
                setIgnoredLink(ignoredLinks).build();
            when(tx.read(oper, irpath)).
                thenReturn(getReadResult(iroot));
            luctx.resolveIgnoredLinks();
            List<VtnLink> expected = new ArrayList<>();
            Map<IgnoredLink, String> skipped = new HashMap<>();

            for (IgnoredLink ilink: ignoredLinks) {
                LinkId lid = ilink.getLinkId();
                InstanceIdentifier<IgnoredLink> ipath = InstanceIdentifier.
                    builder(IgnoredLinks.class).
                    child(IgnoredLink.class, new IgnoredLinkKey(lid)).build();
                String linkId = lid.getValue();
                SalPort src = SalPort.create(ilink.getSource());
                SalPort dst = SalPort.create(ilink.getDestination());
                String dmsg = "Skip ignored-link: {}: {} -> {}: cause={}";
                if (removedLinks.contains(ipath)) {
                    verify(tx).delete(oper, ipath);
                    VtnLink vlink = new VtnLinkBuilder(ilink).build();
                    expected.add(vlink);
                } else {
                    verify(tx, never()).delete(oper, ipath);
                    skipped.put(ilink, causes.get(ipath));
                }
            }

            Set<IgnoredLink> emptyIgnored = Collections.<IgnoredLink>emptySet();
            List<VtnLink> emptyLink = Collections.<VtnLink>emptyList();
            Set<SalPort> emptyPort = Collections.<SalPort>emptySet();
            assertEquals(emptyIgnored, luctx.getIgnoredLinks());
            assertEquals(skipped.keySet(), luctx.getUnresolvedLinks());
            assertEquals(expected, luctx.getResolvedLinks());
            assertEquals(emptyLink, luctx.getMovedLinks());
            assertEquals(emptyPort, luctx.getStaticEdgePorts());

            luctx.recordLogs(logger);
            verify(logger).isDebugEnabled();

            String fmt = "{}: {}: {} -> {}";
            String msg = "Inter-switch link has been resolved";
            for (VtnLink vlink: expected) {
                verify(logger).
                    info(fmt, msg, vlink.getLinkId().getValue(),
                         vlink.getSource().getValue(),
                         vlink.getDestination().getValue());
            }

            String causeFmt = "{}: {}: {} -> {}: cause={}";
            String skipMsg = "Skip ignored-link";
            for (Map.Entry<IgnoredLink, String> entry: skipped.entrySet()) {
                IgnoredLink ilink = entry.getKey();
                String cause = entry.getValue();
                verify(logger).
                    debug(causeFmt, skipMsg, ilink.getLinkId().getValue(),
                          ilink.getSource().getValue(),
                          ilink.getDestination().getValue(), cause);
            }
            verifyNoMoreInteractions(logger);

            // In case where debug log is disabled.
            reset(logger);
            when(logger.isDebugEnabled()).thenReturn(false);
            luctx.recordLogs(logger);

            for (VtnLink vlink: expected) {
                verify(logger).
                    info(fmt, msg, vlink.getLinkId().getValue(),
                         vlink.getSource().getValue(),
                         vlink.getDestination().getValue());
            }
            verify(logger).isDebugEnabled();
            verifyNoMoreInteractions(logger);

            for (Map.Entry<InstanceIdentifier<VtnLink>, VtnLink> entry:
                     vtnLinks.entrySet()) {
                InstanceIdentifier<VtnLink> vpath = entry.getKey();
                VtnLink vlink = entry.getValue();
                if (vlink == null) {
                    verify(tx, never()).
                        put(any(LogicalDatastoreType.class),
                              eq(vpath), any(VtnLink.class), anyBoolean());
                } else {
                    verify(tx).put(oper, vpath, vlink, true);
                }
            }

            for (Map.Entry<InstanceIdentifier<PortLink>, PortLink> entry:
                     portLinks.entrySet()) {
                InstanceIdentifier<PortLink> ppath = entry.getKey();
                PortLink plink = entry.getValue();
                if (plink == null) {
                    verify(tx, never()).
                        put(any(LogicalDatastoreType.class),
                              eq(ppath), any(PortLink.class), anyBoolean());
                } else {
                    verify(tx).put(oper, ppath, plink, true);
                }
            }
        }
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#resolveIgnoredLinks()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testResolveIgnoredLinks() throws Exception {
        ResolveLinkEnv env = new ResolveLinkEnv();
        ReadWriteTransaction tx = env.getTransaction();
        Logger log = env.getLogger();
        LinkUpdateContext luctx = env.getLinkUpdateContext();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Set<IgnoredLink> emptyIgnored = Collections.<IgnoredLink>emptySet();
        List<VtnLink> emptyLink = Collections.<VtnLink>emptyList();
        Set<SalPort> emptyPort = Collections.<SalPort>emptySet();

        // Root container is not present.
        InstanceIdentifier<IgnoredLinks> irpath =
            InstanceIdentifier.create(IgnoredLinks.class);
        IgnoredLinks iroot = null;
        when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
        luctx.resolveIgnoredLinks();
        assertEquals(emptyIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        verify(tx).read(oper, irpath);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(tx, log);
        when(log.isDebugEnabled()).thenReturn(true);

        // Ignored link list is null.
        iroot = new IgnoredLinksBuilder().build();
        when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
        luctx.resolveIgnoredLinks();
        assertEquals(emptyIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        verify(tx).read(oper, irpath);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(tx, log);
        when(log.isDebugEnabled()).thenReturn(true);

        // Ignored link list is empty.
        List<IgnoredLink> ilinks = new ArrayList<>();
        iroot = new IgnoredLinksBuilder().setIgnoredLink(ilinks).build();
        when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
        luctx.resolveIgnoredLinks();
        assertEquals(emptyIgnored, luctx.getIgnoredLinks());
        assertEquals(emptyIgnored, luctx.getUnresolvedLinks());
        assertEquals(emptyLink, luctx.getResolvedLinks());
        assertEquals(emptyLink, luctx.getMovedLinks());
        assertEquals(emptyPort, luctx.getStaticEdgePorts());

        verify(tx).read(oper, irpath);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
        reset(tx, log);
        when(log.isDebugEnabled()).thenReturn(true);

        SalPort staticDst = new SalPort(12345L, 10L);
        SalPortGenerator[] generators = {
            new SalPortGenerator(10L),
            new SalPortGenerator(11L),
            new SalPortGenerator(20L),
        };

        for (SalPortGenerator gen1: generators) {
            for (SalPortGenerator gen2: generators) {
                if (gen1.equals(gen2)) {
                    continue;
                }

                // Both source and destination ports are not present.
                SalPort src = gen1.newInstance();
                SalPort dst = gen2.newInstance();
                env.addStaticEdgePort(src, false);
                env.addStaticEdgePort(dst, false);
                env.addStaticSwitchLink(src, staticDst, false);
                env.add(src, false, dst, false);

                // Only source port is present.
                src = gen1.newInstance();
                dst = gen2.newInstance();
                env.addStaticEdgePort(src, false);
                env.addStaticEdgePort(dst, false);
                env.addStaticSwitchLink(src, staticDst, false);
                env.add(src, true, dst, false);

                // Only destination port is present.
                src = gen1.newInstance();
                dst = gen2.newInstance();
                env.addStaticEdgePort(src, false);
                env.addStaticEdgePort(dst, false);
                env.addStaticSwitchLink(src, staticDst, false);
                env.add(src, false, dst, true);

                // Both source and destination ports are present.
                src = gen1.newInstance();
                dst = gen2.newInstance();
                env.addStaticEdgePort(src, false);
                env.addStaticEdgePort(dst, false);
                env.addStaticSwitchLink(src, staticDst, false);
                env.add(src, true, dst, true);

                // Source port is configured as an edge port.
                src = gen1.newInstance();
                dst = gen2.newInstance();
                env.addStaticEdgePort(src, true);
                env.addStaticEdgePort(dst, false);
                env.addStaticSwitchLink(src, staticDst, false);
                env.add(src, true, dst, true);

                // Destination port is configured as an edge port.
                src = gen1.newInstance();
                dst = gen2.newInstance();
                env.addStaticEdgePort(src, false);
                env.addStaticEdgePort(dst, true);
                env.addStaticSwitchLink(src, staticDst, false);
                env.add(src, true, dst, true);

                // Static inter-switch link is configured.
                src = gen1.newInstance();
                dst = gen2.newInstance();
                env.addStaticEdgePort(src, false);
                env.addStaticEdgePort(dst, false);
                env.addStaticSwitchLink(src, staticDst, true);
                env.add(src, true, dst, true);
            }
        }

        env.runTest();
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   The given port is in DOWN state.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyDown() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        SalPort stSrc = new SalPort(100L, 201L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InterSwitchLink isl3 = new InterSwitchLink(stSrc, sport, true);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink(),
                           isl3.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport, null, true).
            setPortLink(plinks).build();

        // All the links in the given port should be removed.
        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path3 = isl3.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));
        when(tx.read(oper, path3)).
            thenReturn(getReadResult(isl3.getVtnLink()));

        InstanceIdentifier<PortLink> spath1 = isl1.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath2 = isl2.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath3 = isl3.getSourceLinkPath();
        when(tx.read(oper, spath2)).
            thenReturn(getReadResult(isl2.getSourceLink()));
        when(tx.read(oper, spath3)).
            thenReturn(getReadResult(isl3.getSourceLink()));

        InstanceIdentifier<PortLink> dpath1 = isl1.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath2 = isl2.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath3 = isl3.getDestinationLinkPath();
        when(tx.read(oper, dpath1)).
            thenReturn(getReadResult(isl1.getDestinationLink()));

        luctx.updateStaticTopology(sport, vport);

        verify(tx).read(oper, path1);
        verify(tx).read(oper, path2);
        verify(tx).read(oper, path3);
        verify(tx).delete(oper, path1);
        verify(tx).delete(oper, path2);
        verify(tx).delete(oper, path3);

        verify(tx).read(oper, spath2);
        verify(tx).read(oper, spath3);
        verify(tx).delete(oper, spath1);
        verify(tx).delete(oper, spath2);
        verify(tx).delete(oper, spath3);

        verify(tx).read(oper, dpath1);
        verify(tx).delete(oper, dpath1);
        verify(tx).delete(oper, dpath2);
        verify(tx).delete(oper, dpath3);
        verifyNoMoreInteractions(tx);

        // No message should be logged.
        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);

        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);

        // In case where no link is configured.
        reset(tx, log);
        vport = createVtnPortBuilder(sport, null, true).build();
        luctx.updateStaticTopology(sport, vport);
        verifyZeroInteractions(tx);

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   The given port is configured as a static edge port.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyEdge() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        SalPort stSrc = new SalPort(100L, 201L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InterSwitchLink isl3 = new InterSwitchLink(stSrc, sport, true);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink(),
                           isl3.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport).
            setPortLink(plinks).build();

        // All the links in the given port should be removed.
        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path3 = isl3.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));
        when(tx.read(oper, path3)).
            thenReturn(getReadResult(isl3.getVtnLink()));

        InstanceIdentifier<PortLink> spath1 = isl1.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath2 = isl2.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath3 = isl3.getSourceLinkPath();
        when(tx.read(oper, spath2)).
            thenReturn(getReadResult(isl2.getSourceLink()));
        when(tx.read(oper, spath3)).
            thenReturn(getReadResult(isl3.getSourceLink()));

        InstanceIdentifier<PortLink> dpath1 = isl1.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath2 = isl2.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath3 = isl3.getDestinationLinkPath();
        when(tx.read(oper, dpath1)).
            thenReturn(getReadResult(isl1.getDestinationLink()));

        // Configure the static network topology.
        // Note that static inter-switch link configuration should be ignored
        // because static edge port configuration always precedes static
        // inter-switch link configuration.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(
                newStaticSwitchLinks(
                    isl1.getStaticSwitchLink(),
                    isl2.getStaticSwitchLink())).
            setStaticEdgePorts(newStaticEdgePorts(sport)).
            build();
        reader.prefetch(vstopo);

        luctx.updateStaticTopology(sport, vport);

        verify(tx).read(oper, path1);
        verify(tx).read(oper, path2);
        verify(tx).read(oper, path3);
        verify(tx).delete(oper, path1);
        verify(tx).delete(oper, path2);
        verify(tx).delete(oper, path3);

        verify(tx).read(oper, spath2);
        verify(tx).read(oper, spath3);
        verify(tx).delete(oper, spath1);
        verify(tx).delete(oper, spath2);
        verify(tx).delete(oper, spath3);

        verify(tx).read(oper, dpath1);
        verify(tx).delete(oper, dpath1);
        verify(tx).delete(oper, dpath2);
        verify(tx).delete(oper, dpath3);

        // Dynamic links should be moved to ignored-links container, and they
        // should be logged.
        verify(tx).
            put(oper, isl1.getIgnoredLinkPath(), isl1.getIgnoredLink(), true);
        verify(tx).
            put(oper, isl2.getIgnoredLinkPath(), isl2.getIgnoredLink(), true);

        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);

        String msg = "Inter-switch link has been superseded by static topology";
        VtnLink vlink1 = isl1.getVtnLink();
        VtnLink vlink2 = isl2.getVtnLink();
        verify(log).isDebugEnabled();
        verify(log).
            info("{}: {}: {} -> {}", msg,
                 vlink1.getLinkId().getValue(),
                 vlink1.getSource().getValue(),
                 vlink1.getDestination().getValue());
        verify(log).
            info("{}: {}: {} -> {}", msg,
                 vlink2.getLinkId().getValue(),
                 vlink2.getSource().getValue(),
                 vlink2.getDestination().getValue());

        verify(log).info("{}: Port has been updated as a static edge port.",
                         sport);
        verifyNoMoreInteractions(log);

        // In case where no link is configured.
        reset(tx, log);
        vport = createVtnPortBuilder(sport).build();
        luctx = new LinkUpdateContext(tx, reader);
        luctx.updateStaticTopology(sport, vport);
        verifyZeroInteractions(tx);

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   No static network topology is configured.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyEmpty() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport).
            setPortLink(plinks).build();

        // No static network topology is configured.
        reader.prefetch((VtnStaticTopology)null);

        // All the links in the given port should never be changed.
        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));

        luctx.updateStaticTopology(sport, vport);

        verify(tx).read(oper, path1);
        verify(tx).read(oper, path2);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);

        // In case where no link is configured.
        reset(tx, log);
        vport = createVtnPortBuilder(sport).build();
        luctx.updateStaticTopology(sport, vport);
        verifyZeroInteractions(tx);

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   Static link should be removed if its configuration is removed.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyLinkUnconfig() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        SalPort stSrc = new SalPort(100L, 201L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InterSwitchLink isl3 = new InterSwitchLink(stSrc, sport, true);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink(),
                           isl3.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport).
            setPortLink(plinks).build();

        // No static network topology is configured.
        reader.prefetch((VtnStaticTopology)null);

        // Only a static link should be removed.
        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path3 = isl3.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));
        when(tx.read(oper, path3)).
            thenReturn(getReadResult(isl3.getVtnLink()));

        InstanceIdentifier<PortLink> spath3 = isl3.getSourceLinkPath();
        when(tx.read(oper, spath3)).
            thenReturn(getReadResult(isl3.getSourceLink()));

        luctx.updateStaticTopology(sport, vport);

        verify(tx).read(oper, path1);
        verify(tx).read(oper, path2);
        verify(tx).read(oper, path3);
        verify(tx).delete(oper, path3);

        verify(tx).read(oper, spath3);
        verify(tx).delete(oper, spath3);

        InstanceIdentifier<PortLink> dpath3 = isl3.getDestinationLinkPath();
        verify(tx).delete(oper, dpath3);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);

        // In case where no link is configured.
        reset(tx, log);
        vport = createVtnPortBuilder(sport).build();
        luctx = new LinkUpdateContext(tx, reader);
        luctx.updateStaticTopology(sport, vport);
        verifyZeroInteractions(tx);

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   Static link should be removed if the peer port is in DOWN state.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyLinkPeerDown1() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        SalPort stSrc = new SalPort(100L, 201L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InterSwitchLink isl3 = new InterSwitchLink(stSrc, sport, true);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink(),
                           isl3.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport).
            setPortLink(plinks).build();

        List<PortLink> stSrcLinks = new ArrayList<>();
        stSrcLinks.add(isl3.getSourceLink());
        VtnPort stSrcPort = createVtnPortBuilder(stSrc, false, true).
            setEnabled(false).setPortLink(stSrcLinks).build();
        reader.prefetch(stSrc, stSrcPort);

        // stSrcPort in inventory reader will be purged by
        // removeObsoleteStaticLink().
        VtnPort stSrcPort1 = createVtnPortBuilder(stSrc, false, true).build();
        InstanceIdentifier<VtnPort> stSrcPath = stSrc.getVtnPortIdentifier();
        when(tx.read(oper, stSrcPath)).thenReturn(getReadResult(stSrcPort1));

        // Configure the static link.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(
                newStaticSwitchLinks(isl3.getStaticSwitchLink())).
            build();
        reader.prefetch(vstopo);

        // Only a static link should be removed.
        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path3 = isl3.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));
        when(tx.read(oper, path3)).
            thenReturn(getReadResult(isl3.getVtnLink()));

        InstanceIdentifier<PortLink> spath3 = isl3.getSourceLinkPath();
        when(tx.read(oper, spath3)).
            thenReturn(getReadResult(isl3.getSourceLink()));

        luctx.updateStaticTopology(sport, vport);

        verify(tx).read(oper, path1);
        verify(tx).read(oper, path2);
        verify(tx).read(oper, path3);
        verify(tx).delete(oper, path3);

        verify(tx).read(oper, spath3);
        verify(tx).delete(oper, spath3);

        InstanceIdentifier<PortLink> dpath3 = isl3.getDestinationLinkPath();
        verify(tx).delete(oper, dpath3);

        verify(tx).read(oper, stSrcPath);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);

        // In case where no link is configured.
        reset(tx, log);
        vport = createVtnPortBuilder(sport).build();
        luctx = new LinkUpdateContext(tx, reader);
        luctx.updateStaticTopology(sport, vport);
        verifyZeroInteractions(tx);

        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   Static link configuration should be ignored if the peer port is
     *   in DOWN state.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyLinkPeerDown2() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer, true);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport, true);
        VtnPort vport = createVtnPortBuilder(sport).build();
        VtnPort vpeer = createVtnPortBuilder(peer, false, true).build();
        reader.prefetch(peer, vpeer);

        // Configure the static link.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(
                newStaticSwitchLinks(
                    isl1.getStaticSwitchLink(),
                    isl2.getStaticSwitchLink())).
            build();
        reader.prefetch(vstopo);

        // Static link configuration should be ignored.
        verifyZeroInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   Static link configuration should be ignored if the peer port is
     *   not present.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyLinkNoPeer() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer, true);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport, true);
        VtnPort vport = createVtnPortBuilder(sport).build();
        reader.prefetch(peer, (VtnPort)null);

        // Configure the static link.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(
                newStaticSwitchLinks(
                    isl1.getStaticSwitchLink(),
                    isl2.getStaticSwitchLink())).
            build();
        reader.prefetch(vstopo);

        // Static link configuration should be ignored.
        verifyZeroInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   Should do nothing if the static link is already established.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyAlreadyLinked() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        SalPort dsrc = new SalPort(100L, 201L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer, true);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport, true);
        InterSwitchLink isl3 = new InterSwitchLink(dsrc, sport);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink(),
                           isl3.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport).
            setPortLink(plinks).build();
        reader.prefetch(sport, vport);

        List<PortLink> peerLinks = new ArrayList<>();
        Collections.addAll(peerLinks, isl1.getDestinationLink(),
                           isl2.getSourceLink());
        VtnPort vpeer = createVtnPortBuilder(peer).
            setPortLink(peerLinks).build();
        reader.prefetch(peer, vpeer);

        // Configure the static link.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(
                newStaticSwitchLinks(
                    isl1.getStaticSwitchLink(),
                    isl2.getStaticSwitchLink())).
            build();
        reader.prefetch(vstopo);

        // No change should be made.
        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path3 = isl3.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));
        when(tx.read(oper, path3)).
            thenReturn(getReadResult(isl3.getVtnLink()));

        luctx.updateStaticTopology(sport, vport);

        verify(tx).read(oper, path1);
        verify(tx).read(oper, path2);
        verify(tx).read(oper, path3);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#updateStaticTopology(SalPort, VtnPort)}.
     *
     * <p>
     *   Establish new static links.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateStaticTopologyNewLink() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        SalPort peer = new SalPort(200L, 300L);
        SalPort stPeer = new SalPort(201L, 123L);
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InterSwitchLink isl3 = new InterSwitchLink(sport, stPeer, true);
        InterSwitchLink isl4 = new InterSwitchLink(stPeer, sport, true);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl1.getSourceLink(),
                           isl2.getDestinationLink());
        VtnPort vport = createVtnPortBuilder(sport).
            setPortLink(plinks).build();
        reader.prefetch(sport, vport);

        List<PortLink> peerLinks = new ArrayList<>();
        Collections.addAll(peerLinks, isl1.getDestinationLink(),
                           isl2.getSourceLink());
        VtnPort vpeer = createVtnPortBuilder(peer).
            setPortLink(peerLinks).build();
        reader.prefetch(peer, vpeer);

        VtnPort vstPeer = createVtnPortBuilder(stPeer).build();
        reader.prefetch(stPeer, vstPeer);

        // addStaticLink() for isl3 will purge inventory caches.
        plinks = new ArrayList<>(plinks);
        plinks.add(isl4.getDestinationLink());
        VtnPort vportNew = createVtnPortBuilder(sport).
            setPortLink(plinks).build();
        InstanceIdentifier<VtnPort> vportPath = sport.getVtnPortIdentifier();
        when(tx.read(oper, vportPath)).thenReturn(getReadResult(vportNew));

        List<PortLink> vstPeerLink = new ArrayList<>();
        vstPeerLink.add(isl4.getSourceLink());
        VtnPort vstPeerNew = createVtnPortBuilder(stPeer).
            setPortLink(vstPeerLink).build();
        InstanceIdentifier<VtnPort> stPeerPath = stPeer.getVtnPortIdentifier();
        when(tx.read(oper, stPeerPath)).thenReturn(getReadResult(vstPeerNew));

        InstanceIdentifier<VtnLink> path1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path2 = isl2.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path3 = isl3.getVtnLinkPath();
        InstanceIdentifier<VtnLink> path4 = isl4.getVtnLinkPath();
        when(tx.read(oper, path1)).
            thenReturn(getReadResult(isl1.getVtnLink()));
        when(tx.read(oper, path2)).
            thenReturn(getReadResult(isl2.getVtnLink()));
        when(tx.read(oper, path4)).
            thenReturn(getReadResult((VtnLink)null));

        InstanceIdentifier<PortLink> spath1 = isl1.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath3 = isl3.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath4 = isl4.getSourceLinkPath();
        when(tx.read(oper, spath1)).
            thenReturn(getReadResult(isl1.getSourceLink()));

        InstanceIdentifier<PortLink> dpath1 = isl1.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath3 = isl3.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath4 = isl4.getDestinationLinkPath();
        when(tx.read(oper, dpath1)).
            thenReturn(getReadResult(isl1.getDestinationLink()));

        // Configure the static inter-switch links.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(
                newStaticSwitchLinks(
                    isl3.getStaticSwitchLink(),
                    isl4.getStaticSwitchLink())).
            build();
        reader.prefetch(vstopo);

        luctx.updateStaticTopology(sport, vport);

        // Operations in removeObsoleteStaticLink().
        // path1 will also be read in addStaticLink().
        verify(tx, times(2)).read(oper, path1);
        verify(tx).read(oper, path2);

        // Operations in addStaticLink() which establishes isl4.
        verify(tx).read(oper, path4);
        verify(tx).put(oper, path4, isl4.getVtnLink(), true);
        verify(tx).put(oper, spath4, isl4.getSourceLink(), true);
        verify(tx).put(oper, dpath4, isl4.getDestinationLink(), true);

        // Operations in addStaticLink() which establishes isl3.
        // This should move isl1 to ignored-links container.
        verify(tx).read(oper, vportPath);
        verify(tx).read(oper, stPeerPath);
        verify(tx).delete(oper, path1);
        verify(tx).read(oper, spath1);
        verify(tx).delete(oper, spath1);
        verify(tx).read(oper, dpath1);
        verify(tx).delete(oper, dpath1);
        verify(tx).
            put(oper, isl1.getIgnoredLinkPath(), isl1.getIgnoredLink(), true);

        verify(tx).put(oper, path3, isl3.getVtnLink(), true);
        verify(tx).put(oper, spath3, isl3.getSourceLink(), true);
        verify(tx).put(oper, dpath3, isl3.getDestinationLink(), true);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);

        String msg = "Inter-switch link has been superseded by static topology";
        VtnLink vlink1 = isl1.getVtnLink();
        verify(log).isDebugEnabled();
        verify(log).
            info("{}: {}: {} -> {}", msg,
                 vlink1.getLinkId().getValue(),
                 vlink1.getSource().getValue(),
                 vlink1.getDestination().getValue());
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   No MD-SAL link is preserved.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksNoLink() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        LinkId linkId = new LinkId(sport.toString());
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, new LinkKey(linkId)).
            build();
        Link link = null;
        when(tx.read(oper, lpath)).thenReturn(getReadResult(link));

        assertSame(vport, luctx.restoreVtnLinks(sport, vport));
        verify(tx).read(oper, lpath);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   Peer port ID in the MD-SAL link is invalid.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksInvalidPeer() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        String srcId = sport.toString();
        String srcNode = sport.toNodeString();
        Link link = newMdLink(srcId, srcNode, srcId, "unknown:1",
                              "unknown:1:2");
        LinkId linkId = link.getLinkId();
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, link.getKey()).
            build();
        when(tx.read(oper, lpath)).thenReturn(getReadResult(link));

        assertSame(vport, luctx.restoreVtnLinks(sport, vport));
        verify(tx).read(oper, lpath);
        verifyNoMoreInteractions(tx);

        Logger log = mock(Logger.class);
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   Peer port is not present or in DOWN state.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksPeerDown() throws Exception {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        SalPort peer = new SalPort(300L, 400L);
        VtnPort[] vpeers = {
            null,
            createVtnPortBuilder(peer, false, true).build(),
        };
        Link link = newMdLink(sport, peer);
        LinkId linkId = link.getLinkId();
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, link.getKey()).
            build();
        InstanceIdentifier<VtnPort> peerPath = peer.getVtnPortIdentifier();
        for (VtnPort vpeer: vpeers) {
            ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
            InventoryReader reader = new InventoryReader(tx);
            LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);
            when(tx.read(oper, lpath)).thenReturn(getReadResult(link));
            when(tx.read(oper, peerPath)).thenReturn(getReadResult(vpeer));

            assertSame(vport, luctx.restoreVtnLinks(sport, vport));
            verify(tx).read(oper, lpath);
            verify(tx).read(oper, peerPath);
            verifyNoMoreInteractions(tx);

            Logger log = mock(Logger.class);
            when(log.isDebugEnabled()).thenReturn(true);
            luctx.recordLogs(log);
            verify(log).isDebugEnabled();
            verifyNoMoreInteractions(log);
        }
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   No reverse MD-SAL link is present.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksNoReverse() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        SalPort peer = new SalPort(300L, 400L);
        VtnPort vpeer = createVtnPortBuilder(peer).build();
        reader.prefetch(sport, vport);
        reader.prefetch(peer, vpeer);

        InstanceIdentifier<VtnPort> peerPath = peer.getVtnPortIdentifier();
        when(tx.read(oper, peerPath)).thenReturn(getReadResult(vpeer));

        // No static topology configuration.
        reader.prefetch((VtnStaticTopology)null);

        Link link = newMdLink(sport, peer);
        LinkId linkId = link.getLinkId();
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, link.getKey()).
            build();
        when(tx.read(oper, lpath)).thenReturn(getReadResult(link));

        Link rlink = newMdLink(peer, sport);
        LinkId rlinkId = rlink.getLinkId();
        InstanceIdentifier<Link> rlpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, rlink.getKey()).
            build();
        rlink = null;
        when(tx.read(oper, rlpath)).thenReturn(getReadResult(rlink));

        // Only one link should be restored.
        InterSwitchLink isl = new InterSwitchLink(sport, peer);
        InstanceIdentifier<VtnLink> vlpath = isl.getVtnLinkPath();
        InstanceIdentifier<PortLink> spath = isl.getSourceLinkPath();
        InstanceIdentifier<PortLink> dpath = isl.getDestinationLinkPath();
        List<PortLink> plinks = Collections.singletonList(isl.getSourceLink());
        VtnPort expected = createVtnPortBuilder(sport).
            setPortLink(plinks).build();
        InstanceIdentifier<VtnPort> ppath = sport.getVtnPortIdentifier();
        when(tx.read(oper, ppath)).thenReturn(getReadResult(expected));

        assertEquals(expected, luctx.restoreVtnLinks(sport, vport));
        verify(tx).read(oper, lpath);
        verify(tx).read(oper, peerPath);
        verify(tx).read(oper, rlpath);
        verify(tx).read(oper, ppath);
        verify(tx).put(oper, vlpath, isl.getVtnLink(), true);
        verify(tx).put(oper, spath, isl.getSourceLink(), true);
        verify(tx).put(oper, dpath, isl.getDestinationLink(), true);
        verifyNoMoreInteractions(tx);

        Map<SalPort, VtnPort> cache = reader.getCachedPorts();
        assertEquals(null, cache.get(sport));
        assertEquals(null, cache.get(peer));

        Logger log = mock(Logger.class);
        String msg = "VTN link has been restored from the MD-SAL " +
            "network topology: {} -> {}";
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verify(log).info(msg, sport, peer);
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   Reverse MD-SAL link does not point to the target port.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksReverseBroken() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        SalPort peer = new SalPort(300L, 400L);
        VtnPort vpeer = createVtnPortBuilder(peer).build();
        reader.prefetch(sport, vport);
        reader.prefetch(peer, vpeer);

        InstanceIdentifier<VtnPort> peerPath = peer.getVtnPortIdentifier();
        when(tx.read(oper, peerPath)).thenReturn(getReadResult(vpeer));

        // No static topology configuration.
        reader.prefetch((VtnStaticTopology)null);

        Link link = newMdLink(sport, peer);
        LinkId linkId = link.getLinkId();
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, link.getKey()).
            build();
        when(tx.read(oper, lpath)).thenReturn(getReadResult(link));

        Link rlink = newMdLink(peer, new SalPort(100L, 201L));
        LinkId rlinkId = rlink.getLinkId();
        InstanceIdentifier<Link> rlpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, rlink.getKey()).
            build();
        when(tx.read(oper, rlpath)).thenReturn(getReadResult(rlink));

        // Only one link should be restored.
        InterSwitchLink isl = new InterSwitchLink(sport, peer);
        InstanceIdentifier<VtnLink> vlpath = isl.getVtnLinkPath();
        InstanceIdentifier<PortLink> spath = isl.getSourceLinkPath();
        InstanceIdentifier<PortLink> dpath = isl.getDestinationLinkPath();
        List<PortLink> plinks = Collections.singletonList(isl.getSourceLink());
        VtnPort expected = createVtnPortBuilder(sport).
            setPortLink(plinks).build();
        InstanceIdentifier<VtnPort> ppath = sport.getVtnPortIdentifier();
        when(tx.read(oper, ppath)).thenReturn(getReadResult(expected));

        assertEquals(expected, luctx.restoreVtnLinks(sport, vport));
        verify(tx).read(oper, lpath);
        verify(tx).read(oper, peerPath);
        verify(tx).read(oper, rlpath);
        verify(tx).read(oper, ppath);
        verify(tx).put(oper, vlpath, isl.getVtnLink(), true);
        verify(tx).put(oper, spath, isl.getSourceLink(), true);
        verify(tx).put(oper, dpath, isl.getDestinationLink(), true);
        verifyNoMoreInteractions(tx);

        Map<SalPort, VtnPort> cache = reader.getCachedPorts();
        assertEquals(null, cache.get(sport));
        assertEquals(null, cache.get(peer));

        Logger log = mock(Logger.class);
        String msg = "VTN link has been restored from the MD-SAL " +
            "network topology: {} -> {}";
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verify(log).info(msg, sport, peer);
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   Both links are restored to vtn-topology.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksRestored() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        SalPort peer = new SalPort(300L, 400L);
        VtnPort vpeer = createVtnPortBuilder(peer).build();
        reader.prefetch(sport, vport);
        reader.prefetch(peer, vpeer);

        InstanceIdentifier<VtnPort> peerPath = peer.getVtnPortIdentifier();
        when(tx.read(oper, peerPath)).thenReturn(getReadResult(vpeer));

        // No static topology configuration.
        reader.prefetch((VtnStaticTopology)null);

        Link link = newMdLink(sport, peer);
        LinkId linkId = link.getLinkId();
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, link.getKey()).
            build();
        when(tx.read(oper, lpath)).thenReturn(getReadResult(link));

        Link rlink = newMdLink(peer, sport);
        LinkId rlinkId = rlink.getLinkId();
        InstanceIdentifier<Link> rlpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, rlink.getKey()).
            build();
        when(tx.read(oper, rlpath)).thenReturn(getReadResult(rlink));

        // Both links should be restored.
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InstanceIdentifier<VtnLink> vlpath1 = isl1.getVtnLinkPath();
        InstanceIdentifier<VtnLink> vlpath2 = isl2.getVtnLinkPath();
        InstanceIdentifier<PortLink> spath1 = isl1.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath2 = isl2.getSourceLinkPath();
        InstanceIdentifier<PortLink> dpath1 = isl1.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath2 = isl2.getDestinationLinkPath();
        List<PortLink> plinks = Arrays.asList(
            isl1.getSourceLink(), isl2.getDestinationLink());
        VtnPort expected = createVtnPortBuilder(sport).
            setPortLink(plinks).build();
        InstanceIdentifier<VtnPort> ppath = sport.getVtnPortIdentifier();
        when(tx.read(oper, ppath)).thenReturn(getReadResult(expected));

        assertEquals(expected, luctx.restoreVtnLinks(sport, vport));
        verify(tx).read(oper, lpath);
        verify(tx).read(oper, peerPath);
        verify(tx).read(oper, rlpath);
        verify(tx).read(oper, ppath);
        verify(tx).put(oper, vlpath1, isl1.getVtnLink(), true);
        verify(tx).put(oper, spath1, isl1.getSourceLink(), true);
        verify(tx).put(oper, dpath1, isl1.getDestinationLink(), true);
        verify(tx).put(oper, vlpath2, isl2.getVtnLink(), true);
        verify(tx).put(oper, spath2, isl2.getSourceLink(), true);
        verify(tx).put(oper, dpath2, isl2.getDestinationLink(), true);
        verifyNoMoreInteractions(tx);

        Map<SalPort, VtnPort> cache = reader.getCachedPorts();
        assertEquals(null, cache.get(sport));
        assertEquals(null, cache.get(peer));

        Logger log = mock(Logger.class);
        String msg = "VTN link has been restored from the MD-SAL " +
            "network topology: {} -> {}";
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verify(log).info(msg, sport, peer);
        verify(log).info(msg, peer, sport);
        verifyNoMoreInteractions(log);
    }

    /**
     * Test case for
     * {@link LinkUpdateContext#restoreVtnLinks(SalPort, VtnPort)}.
     *
     * <p>
     *   Both links are restored to ignored-links.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRestoreVtnLinksIgnored() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        SalPort peer = new SalPort(300L, 400L);
        VtnPort vpeer = createVtnPortBuilder(peer).build();
        reader.prefetch(sport, vport);
        reader.prefetch(peer, vpeer);

        InstanceIdentifier<VtnPort> peerPath = peer.getVtnPortIdentifier();
        when(tx.read(oper, peerPath)).thenReturn(getReadResult(vpeer));

        // Peer is configured as static edge port.
        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticEdgePorts(newStaticEdgePorts(peer)).
            build();
        reader.prefetch(vstopo);

        Link link = newMdLink(sport, peer);
        LinkId linkId = link.getLinkId();
        InstanceIdentifier<Link> lpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, link.getKey()).
            build();
        when(tx.read(oper, lpath)).thenReturn(getReadResult(link));

        Link rlink = newMdLink(peer, sport);
        LinkId rlinkId = rlink.getLinkId();
        InstanceIdentifier<Link> rlpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class, rlink.getKey()).
            build();
        when(tx.read(oper, rlpath)).thenReturn(getReadResult(rlink));

        // Both links should be restored to ignored-links.
        InterSwitchLink isl1 = new InterSwitchLink(sport, peer);
        InterSwitchLink isl2 = new InterSwitchLink(peer, sport);
        InstanceIdentifier<IgnoredLink> igpath1 = isl1.getIgnoredLinkPath();
        InstanceIdentifier<IgnoredLink> igpath2 = isl2.getIgnoredLinkPath();
        InstanceIdentifier<VtnPort> ppath = sport.getVtnPortIdentifier();
        when(tx.read(oper, ppath)).thenReturn(getReadResult(vport));

        assertEquals(vport, luctx.restoreVtnLinks(sport, vport));
        verify(tx).read(oper, lpath);
        verify(tx).read(oper, peerPath);
        verify(tx).read(oper, rlpath);
        verify(tx).read(oper, ppath);
        verify(tx).put(oper, igpath1, isl1.getIgnoredLink(), true);
        verify(tx).put(oper, igpath2, isl2.getIgnoredLink(), true);
        verifyNoMoreInteractions(tx);

        Map<SalPort, VtnPort> cache = reader.getCachedPorts();
        assertEquals(null, cache.get(sport));
        assertEquals(null, cache.get(peer));

        Logger log = mock(Logger.class);
        String msg = "VTN link has been restored from the " +
            "MD-SAL network topology: {} -> {}";
        when(log.isDebugEnabled()).thenReturn(true);
        luctx.recordLogs(log);
        verify(log).isDebugEnabled();
        verify(log).info(msg, sport, peer);
        verify(log).info(msg, peer, sport);

        msg = "Ignore inter-switch link";
        String cause = "Destination port is configured as static edge port.";
        VtnLink vlink1 = isl1.getVtnLink();
        VtnLink vlink2 = isl2.getVtnLink();
        verify(log).
            info("{}: {}: {} -> {}: cause={}", msg,
                 vlink1.getLinkId().getValue(),
                 vlink1.getSource().getValue(),
                 vlink1.getDestination().getValue(), cause);
        cause = "Source port is configured as static edge port.";
        verify(log).
            info("{}: {}: {} -> {}: cause={}", msg,
                 vlink2.getLinkId().getValue(),
                 vlink2.getSource().getValue(),
                 vlink2.getDestination().getValue(), cause);
        verifyNoMoreInteractions(log);
    }
}
