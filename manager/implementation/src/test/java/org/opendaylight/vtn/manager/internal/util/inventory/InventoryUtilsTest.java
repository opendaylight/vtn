/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.IdentifiableItem;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.Item;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchLink;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnectorBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.State;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.StateBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkKey;

/**
 * JUnit test for {@link InventoryUtils}.
 */
public class InventoryUtilsTest extends TestBase {
    /**
     * Network topology identifier.
     */
    private static final String TOPOLOGY_ID = "flow:1";

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link InventoryUtils#hasPortLink(VtnPort)}</li>
     *   <li>{@link InventoryUtils#isEdge(VtnPort)}</li>
     *   <li>{@link InventoryUtils#isEnabled(VtnPort)}</li>
     *   <li>{@link InventoryUtils#isEnabledEdge(VtnPort)}</li>
     * </ul>
     */
    @Test
    public void testVtnPort() {
        Boolean enabled = null;
        boolean edge = false;

        for (long dpid = 1L; dpid <= 5L; dpid++) {
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                VtnPortBuilder builder =
                    createVtnPortBuilder(sport, enabled, edge);
                VtnPort vport = builder.build();
                boolean up = Boolean.TRUE.equals(enabled);
                assertEquals(!edge, InventoryUtils.hasPortLink(vport));
                assertEquals(edge, InventoryUtils.isEdge(vport));
                assertEquals(up, InventoryUtils.isEnabled(vport));
                assertEquals(up && edge, InventoryUtils.isEnabledEdge(vport));

                if (edge) {
                    // Empty port link list should be ignored.
                    List<PortLink> links = new ArrayList<>();
                    vport = builder.setPortLink(links).build();
                    assertEquals(!edge, InventoryUtils.hasPortLink(vport));
                    assertEquals(edge, InventoryUtils.isEdge(vport));
                    assertEquals(up, InventoryUtils.isEnabled(vport));
                    assertEquals(up && edge,
                                 InventoryUtils.isEnabledEdge(vport));
                }

                edge = !edge;
                enabled = triState(enabled);
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#hasEdgePort(VtnNode)}.
     */
    @Test
    public void testHasEdgePort() {
        assertEquals(false, InventoryUtils.hasEdgePort(null));

        // No port in the node.
        VtnNode vnode = new VtnNodeBuilder().build();
        assertEquals(false, InventoryUtils.hasEdgePort(vnode));
        List<VtnPort> portList = new ArrayList<>();
        vnode = new VtnNodeBuilder().setVtnPort(portList).build();
        assertEquals(false, InventoryUtils.hasEdgePort(vnode));

        // All ports are disabled.
        long dpid = 1L;
        Boolean enabled = null;
        boolean edge = false;
        portList = new ArrayList<VtnPort>();
        for (long port = 1L; port <= 20; port++) {
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, enabled, edge).
                build();
            portList.add(vport);

            edge = !edge;
            if (enabled == null) {
                enabled = Boolean.FALSE;
            } else {
                enabled = null;
            }
        }
        vnode = new VtnNodeBuilder().setVtnPort(portList).build();
        assertEquals(false, InventoryUtils.hasEdgePort(vnode));

        // All ports are linked to other switches.
        enabled = null;
        portList = new ArrayList<VtnPort>();
        for (long port = 1L; port <= 20; port++) {
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, enabled, false).
                build();
            portList.add(vport);

            enabled = triState(enabled);
        }
        vnode = new VtnNodeBuilder().setVtnPort(portList).build();
        assertEquals(false, InventoryUtils.hasEdgePort(vnode));

        // Only one edge port is in up state.
        edge = false;
        enabled = null;
        portList = new ArrayList<VtnPort>();
        for (long port = 1L; port <= 20; port++) {
            if (port == 9L) {
                edge = true;
                enabled = Boolean.TRUE;
            } else {
                edge = !edge;
                enabled = (edge) ? Boolean.FALSE : triState(enabled);
            }
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, enabled, edge).
                build();
            portList.add(vport);
        }
        vnode = new VtnNodeBuilder().setVtnPort(portList).build();
        assertEquals(true, InventoryUtils.hasEdgePort(vnode));

        // All ports are in up state, but only one port has ISL links.
        enabled = Boolean.TRUE;
        portList = new ArrayList<VtnPort>();
        for (long port = 1L; port <= 20; port++) {
            if (port == 11L) {
                edge = false;
            } else {
                edge = true;
            }
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, enabled, edge).
                build();
            portList.add(vport);
        }
        vnode = new VtnNodeBuilder().setVtnPort(portList).build();
        assertEquals(true, InventoryUtils.hasEdgePort(vnode));

        // All ports are in up state and no ISL link.
        edge = true;
        enabled = Boolean.TRUE;
        portList = new ArrayList<VtnPort>();
        for (long port = 1L; port <= 20; port++) {
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, enabled, edge).
                build();
            portList.add(vport);
        }
        vnode = new VtnNodeBuilder().setVtnPort(portList).build();
        assertEquals(true, InventoryUtils.hasEdgePort(vnode));
    }

    /**
     * Test case for {@link InventoryUtils#toString(VtnPort)}.
     */
    @Test
    public void testToStringVtnPort() {
        VtnPort vport = new VtnPortBuilder().build();
        String expected =
            "{id=null, name=null, enabled=null, cost=null, links=none}";
        assertEquals(expected, InventoryUtils.toString(vport));

        NodeConnectorId[] ids = {
            null, new NodeConnectorId(""), new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:123456789:123456"),
        };
        String[] names = {null, "", "port-1", "port-name-1"};
        Boolean[] enabled = {null, Boolean.TRUE, Boolean.FALSE};
        Long[] costs = {
            null, Long.valueOf(1L), Long.valueOf(1000L),
            Long.valueOf(10000000L),
        };

        for (NodeConnectorId id: ids) {
            for (String name: names) {
                for (Boolean en: enabled) {
                    for (Long cost: costs) {
                        VtnPortBuilder builder = new VtnPortBuilder().
                            setId(id).setName(name).setEnabled(en).
                            setCost(cost);
                        vport = builder.build();
                        String idstr = (id == null) ? null : id.getValue();
                        String base = "{id=" + idstr + ", name=" + name +
                            ", enabled=" + en + ", cost=" + cost + ", links=";
                        expected = base + "none}";
                        assertEquals(expected, InventoryUtils.toString(vport));

                        List<PortLink> list = new ArrayList<>();
                        vport = builder.setPortLink(list).build();
                        assertEquals(expected, InventoryUtils.toString(vport));

                        // Add one port link.
                        list = new ArrayList<PortLink>();
                        PortLinkBuilder pbuilder = new PortLinkBuilder();
                        String lid1 = "isl-1";
                        String peerId1 = "openflow:33:44";
                        pbuilder.setLinkId(new LinkId(lid1)).
                            setPeer(new NodeConnectorId(peerId1));
                        list.add(pbuilder.build());
                        vport = builder.setPortLink(list).build();
                        String lstr1 = "{id=" + lid1 + ", peer=" + peerId1 +
                            "}";
                        expected = base + "[" + lstr1 + "]}";
                        vport = builder.setPortLink(list).build();
                        assertEquals(expected, InventoryUtils.toString(vport));

                        // Add one more port link.
                        list = new ArrayList<PortLink>(list);
                        pbuilder = new PortLinkBuilder();
                        String lid2 = "isl-2";
                        String peerId2 = "openflow:55:66";
                        pbuilder.setLinkId(new LinkId(lid2)).
                            setPeer(new NodeConnectorId(peerId2));
                        list.add(pbuilder.build());
                        vport = builder.setPortLink(list).build();
                        String lstr2 = "{id=" + lid2 + ", peer=" + peerId2 +
                            "}";
                        expected = base +
                            joinStrings("[", "]", ", ", lstr1, lstr2) + "}";
                        vport = builder.setPortLink(list).build();
                        assertEquals(expected, InventoryUtils.toString(vport));

                        // Add an invalid link.
                        list = new ArrayList<PortLink>(list);
                        pbuilder = new PortLinkBuilder();
                        list.add(pbuilder.build());
                        vport = builder.setPortLink(list).build();
                        String lstr3 = "{id=null, peer=null}";
                        expected = base +
                            joinStrings("[", "]", ", ", lstr1, lstr2, lstr3) +
                            "}";
                        vport = builder.setPortLink(list).build();
                        assertEquals(expected, InventoryUtils.toString(vport));
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#toString(VtnSwitchLink)}.
     */
    @Test
    public void testToStringVtnSwitchLink() {
        VtnSwitchLink vslink = new StaticSwitchLinkBuilder().build();
        assertEquals("{null -> null}", InventoryUtils.toString(vslink));

        NodeConnectorId[] srcs = {
            null,
            new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:123:456"),
            new NodeConnectorId("unknown:1:2"),
        };
        NodeConnectorId[] dsts = {
            null,
            new NodeConnectorId("openflow:2:1"),
            new NodeConnectorId("openflow:789:012"),
            new NodeConnectorId("unknown:2:3"),
        };

        for (NodeConnectorId src: srcs) {
            String srcId = (src == null) ? "null" : src.getValue();
            for (NodeConnectorId dst: dsts) {
                String dstId = (dst == null) ? "null" : dst.getValue();
                vslink = new StaticSwitchLinkBuilder().
                    setSource(src).setDestination(dst).build();
                String expected = "{" + srcId + " -> " + dstId + "}";
                assertEquals(expected, InventoryUtils.toString(vslink));
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#toString(VtnLink)}.
     */
    @Test
    public void testToStringVtnLink() {
        LinkId[] ids = {
            null,
            new LinkId("link:1"),
            new LinkId("link:2"),
            new LinkId("link:3"),
        };
        NodeConnectorId[] srcs = {
            null,
            new NodeConnectorId("openflow:1:1"),
            new NodeConnectorId("openflow:123:456"),
            new NodeConnectorId("unknown:1:2"),
        };
        NodeConnectorId[] dsts = {
            null,
            new NodeConnectorId("openflow:2:1"),
            new NodeConnectorId("openflow:789:012"),
            new NodeConnectorId("unknown:2:3"),
        };

        for (LinkId lid: ids) {
            String linkId = (lid == null) ? "null" : lid.getValue();
            for (NodeConnectorId src: srcs) {
                String srcId = (src == null) ? "null" : src.getValue();
                for (NodeConnectorId dst: dsts) {
                    String dstId = (dst == null) ? "null" : dst.getValue();
                    VtnLinkBuilder vb = new VtnLinkBuilder().
                        setLinkId(lid).setSource(src).setDestination(dst);
                    String exp = "{id=" + linkId + ", src=" + srcId +
                        ", dst=" + dstId + ", static=";
                    String expected = exp + "false}";
                    VtnLink vlink = vb.build();
                    assertEquals(expected, InventoryUtils.toString(vlink));

                    vlink = vb.setStaticLink(Boolean.FALSE).build();
                    assertEquals(expected, InventoryUtils.toString(vlink));

                    expected = exp + "true}";
                    vlink = vb.setStaticLink(Boolean.TRUE).build();
                    assertEquals(expected, InventoryUtils.toString(vlink));
                }
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#toVtnLinkIdentifier(LinkId)}.
     */
    @Test
    public void testToVtnLinkIdentifier() {
        String[] ids = {
            "link-1", "link-2", "openflow:1", "openflow:1234567",
        };
        for (String id: ids) {
            LinkId lid = new LinkId(id);
            InstanceIdentifier<VtnLink> path = InventoryUtils.
                toVtnLinkIdentifier(lid);
            assertEquals(VtnLink.class, path.getTargetType());
            assertEquals(false, path.isWildcarded());

            Iterator<PathArgument> it = path.getPathArguments().iterator();
            assertTrue(it.hasNext());
            PathArgument pa = it.next();
            assertEquals(VtnTopology.class, pa.getType());
            assertTrue(pa instanceof Item);

            assertTrue(it.hasNext());
            pa = it.next();
            assertEquals(VtnLink.class, pa.getType());
            assertTrue(pa instanceof IdentifiableItem);
            IdentifiableItem<?, ?> item = (IdentifiableItem<?, ?>)pa;
            VtnLinkKey key = new VtnLinkKey(lid);
            assertEquals(key, item.getKey());

            assertFalse(it.hasNext());
        }
    }
    /**
     * Test case for {@link InventoryUtils#toIgnoredLinkIdentifier(LinkId)}.
     */
    @Test
    public void testToIgnoredLinkIdentifier() {
        String[] ids = {
            "link-1", "link-2", "openflow:1", "openflow:1234567",
        };
        for (String id: ids) {
            LinkId lid = new LinkId(id);
            InstanceIdentifier<IgnoredLink> path = InventoryUtils.
                toIgnoredLinkIdentifier(lid);
            assertEquals(IgnoredLink.class, path.getTargetType());
            assertEquals(false, path.isWildcarded());

            Iterator<PathArgument> it = path.getPathArguments().iterator();
            assertTrue(it.hasNext());
            PathArgument pa = it.next();
            assertEquals(IgnoredLinks.class, pa.getType());
            assertTrue(pa instanceof Item);

            assertTrue(it.hasNext());
            pa = it.next();
            assertEquals(IgnoredLink.class, pa.getType());
            assertTrue(pa instanceof IdentifiableItem);
            IdentifiableItem<?, ?> item = (IdentifiableItem<?, ?>)pa;
            IgnoredLinkKey key = new IgnoredLinkKey(lid);
            assertEquals(key, item.getKey());

            assertFalse(it.hasNext());
        }
    }

    /**
     * Test case for
     * {@link InventoryUtils#toStaticSwitchLinkIdentifier(SalPort)}.
     */
    @Test
    public void testToStaticSwitchLinkIdentifier() {
        for (long dpid = 1L; dpid <= 5L; dpid++) {
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<StaticSwitchLink> path = InventoryUtils.
                    toStaticSwitchLinkIdentifier(sport);
                assertEquals(StaticSwitchLink.class, path.getTargetType());
                assertEquals(false, path.isWildcarded());

                Iterator<PathArgument> it = path.getPathArguments().iterator();
                assertTrue(it.hasNext());
                PathArgument pa = it.next();
                assertEquals(VtnStaticTopology.class, pa.getType());
                assertTrue(pa instanceof Item);

                assertTrue(it.hasNext());
                pa = it.next();
                assertEquals(StaticSwitchLinks.class, pa.getType());
                assertTrue(pa instanceof Item);

                assertTrue(it.hasNext());
                pa = it.next();
                assertEquals(StaticSwitchLink.class, pa.getType());
                assertTrue(pa instanceof IdentifiableItem);
                IdentifiableItem<?, ?> item = (IdentifiableItem<?, ?>)pa;
                StaticSwitchLinkKey key =
                    new StaticSwitchLinkKey(sport.getNodeConnectorId());
                assertEquals(key, item.getKey());

                assertFalse(it.hasNext());
            }
        }
    }

    /**
     * Test case for
     * {@link InventoryUtils#toStaticEdgePortIdentifier(SalPort)}.
     */
    @Test
    public void testToStaticEdgePortIdentifier() {
        for (long dpid = 1L; dpid <= 5L; dpid++) {
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<StaticEdgePort> path = InventoryUtils.
                    toStaticEdgePortIdentifier(sport);
                assertEquals(StaticEdgePort.class, path.getTargetType());
                assertEquals(false, path.isWildcarded());

                Iterator<PathArgument> it = path.getPathArguments().iterator();
                assertTrue(it.hasNext());
                PathArgument pa = it.next();
                assertEquals(VtnStaticTopology.class, pa.getType());
                assertTrue(pa instanceof Item);

                assertTrue(it.hasNext());
                pa = it.next();
                assertEquals(StaticEdgePorts.class, pa.getType());
                assertTrue(pa instanceof Item);

                assertTrue(it.hasNext());
                pa = it.next();
                assertEquals(StaticEdgePort.class, pa.getType());
                assertTrue(pa instanceof IdentifiableItem);
                IdentifiableItem<?, ?> item = (IdentifiableItem<?, ?>)pa;
                StaticEdgePortKey key =
                    new StaticEdgePortKey(sport.getNodeConnectorId());
                assertEquals(key, item.getKey());

                assertFalse(it.hasNext());
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link InventoryUtils#toVtnNodeBuilder(Node)}</li>
     *   <li>{@link InventoryUtils#toVtnNodeBuilder(NodeId)}</li>
     *   <li>{@link InventoryUtils#getVtnPorts(List, Node)}</li>
     * </ul>
     */
    @Test
    public void testToVtnNodeBuilder() {
        NodeBuilder builder = new NodeBuilder();
        Node node = builder.build();
        List<VtnPort> vportList = new ArrayList<>();
        assertEquals(null, InventoryUtils.getVtnPorts(vportList, node));
        VtnNodeBuilder vbuilder = InventoryUtils.toVtnNodeBuilder(node);
        assertEquals(null, vbuilder.getId());
        assertEquals(null, vbuilder.getVtnPort());
        assertEquals(null, vbuilder.getOpenflowVersion());
        assertTrue(vportList.isEmpty());

        List<NodeConnector> badNcList = new ArrayList<>();
        node = builder.setNodeConnector(badNcList).build();
        assertEquals(null, InventoryUtils.getVtnPorts(vportList, node));
        vbuilder = InventoryUtils.toVtnNodeBuilder(node);
        assertEquals(null, vbuilder.getId());
        assertEquals(null, vbuilder.getVtnPort());
        assertEquals(null, vbuilder.getOpenflowVersion());
        assertTrue(vportList.isEmpty());

        // Unsupported node connectors should be ignored.
        NodeConnector nc = new NodeConnectorBuilder().
            setId(new NodeConnectorId("unknown:1:2")).build();
        badNcList.add(nc);
        nc = new NodeConnectorBuilder().
            setId(new NodeConnectorId("openflow:1:LOCAL")).build();
        badNcList.add(nc);
        node = builder.setNodeConnector(badNcList).build();
        assertEquals(null, InventoryUtils.getVtnPorts(vportList, node));
        vbuilder = InventoryUtils.toVtnNodeBuilder(node);
        assertEquals(null, vbuilder.getId());
        assertEquals(null, vbuilder.getVtnPort());
        assertEquals(null, vbuilder.getOpenflowVersion());
        assertTrue(vportList.isEmpty());

        PortConfig pcf = new PortConfigBuilder().setPortDown(Boolean.FALSE).
            build();
        State st = new StateBuilder().setLinkDown(Boolean.FALSE).build();
        long base = 10000000000000L;
        long speed = 10000000L;
        Long cost = Long.valueOf(base / speed);
        PortFeatures pf = new PortFeaturesBuilder().setTenMbFd(Boolean.TRUE).
            build();
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            String nid = "openflow:" + dpid;
            NodeId nodeId = new NodeId(nid);
            builder.setId(nodeId);

            vbuilder = InventoryUtils.toVtnNodeBuilder(nodeId);
            assertEquals(nodeId, vbuilder.getId());
            assertEquals(null, vbuilder.getVtnPort());
            assertEquals(null, vbuilder.getOpenflowVersion());

            List<NodeConnector> nclist10 = new ArrayList<>(badNcList);
            List<NodeConnector> nclist13 = new ArrayList<>(badNcList);
            List<VtnPort> vports10 = new ArrayList<>();
            List<VtnPort> vports13 = new ArrayList<>();
            for (long port = 1L; port <= 10L; port++) {
                String pid = nid + ":" + port;
                NodeConnectorId ncid = new NodeConnectorId(pid);
                String name = "of-" + dpid + "-port-" + port;
                FlowCapableNodeConnectorBuilder fcb =
                    new FlowCapableNodeConnectorBuilder();

                // OpenFlow 1.0
                FlowCapableNodeConnector fcnc = fcb.setConfiguration(pcf).
                    setState(st).setName(name).setCurrentFeature(pf).build();
                nc = new NodeConnectorBuilder().
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    setId(ncid).build();
                nclist10.add(nc);
                VtnPort vport = new VtnPortBuilder().setName(name).
                    setCost(cost).setEnabled(Boolean.TRUE).setId(ncid).build();
                vports10.add(vport);
                node = builder.setNodeConnector(nclist10).build();
                vportList.clear();
                assertEquals(VtnOpenflowVersion.OF10,
                             InventoryUtils.getVtnPorts(vportList, node));
                assertEquals(vports10, vportList);

                vbuilder = InventoryUtils.toVtnNodeBuilder(node);
                assertEquals(nodeId, vbuilder.getId());
                assertEquals(vports10, vbuilder.getVtnPort());
                assertEquals(VtnOpenflowVersion.OF10,
                             vbuilder.getOpenflowVersion());

                // OpenFlow 1.3
                fcnc = new FlowCapableNodeConnectorBuilder().
                    setConfiguration(pcf).setState(st).setName(name).
                    setCurrentFeature(pf).
                    setCurrentSpeed(Long.valueOf(speed / 1000L)).build();
                nc = new NodeConnectorBuilder().
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    setId(ncid).build();
                nclist13.add(nc);
                vport = new VtnPortBuilder().setName(name).
                    setCost(cost).setEnabled(Boolean.TRUE).setId(ncid).build();
                vports13.add(vport);
                node = builder.setNodeConnector(nclist13).build();
                vportList.clear();
                assertEquals(VtnOpenflowVersion.OF13,
                             InventoryUtils.getVtnPorts(vportList, node));
                assertEquals(vports13, vportList);

                vbuilder = InventoryUtils.toVtnNodeBuilder(node);
                assertEquals(nodeId, vbuilder.getId());
                assertEquals(vports13, vbuilder.getVtnPort());
                assertEquals(VtnOpenflowVersion.OF13,
                             vbuilder.getOpenflowVersion());
            }
        }
    }

    /**
     * Test case for
     * {@link InventoryUtils#toVtnPortBuilder(NodeConnectorId, FlowCapableNodeConnector)}.
     */
    @Test
    public void testToVtnPortBuilder() {
        long base = 10000000000000L;
        Long defCost = Long.valueOf(base / 1000000000L);
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            for (long port = 1L; port <= 20L; port++) {
                String pid = "openflow:" + dpid + ":" + port;
                NodeConnectorId ncid = new NodeConnectorId(pid);
                VtnPortKey key = new VtnPortKey(ncid);

                FlowCapableNodeConnectorBuilder fcb =
                    new FlowCapableNodeConnectorBuilder();
                FlowCapableNodeConnector fcnc = fcb.build();
                VtnPort vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).
                    build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(pid, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                // Set port name.
                String name = "switch-" + dpid + "-port-" + port;
                fcnc = fcb.setName(name).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                // Test for link state.
                PortConfigBuilder pcfb = new PortConfigBuilder();
                PortConfig pcf = pcfb.build();
                fcnc = fcb.setConfiguration(pcf).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                pcf = pcfb.setNoFwd(Boolean.TRUE).setNoPacketIn(Boolean.TRUE).
                    setNoRecv(Boolean.TRUE).setPortDown(Boolean.TRUE).build();
                fcnc = fcb.setConfiguration(pcf).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                StateBuilder stb = new StateBuilder();
                State st = stb.build();
                fcnc = fcb.setState(st).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                st = stb.setBlocked(Boolean.TRUE).setLive(Boolean.TRUE).
                    setLinkDown(Boolean.TRUE).build();
                fcnc = fcb.setState(st).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                st = stb.setLinkDown(Boolean.FALSE).build();
                fcnc = fcb.setState(st).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                pcf = pcfb.setPortDown(Boolean.FALSE).build();
                st = stb.setLinkDown(Boolean.TRUE).build();
                fcnc = fcb.setState(st).setConfiguration(pcf).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                st = stb.setLinkDown(Boolean.FALSE).build();
                fcnc = fcb.setState(st).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                // Test for cost for packet transmission.
                PortFeaturesBuilder pfb = new PortFeaturesBuilder();
                PortFeatures pf = pfb.setTenMbHd(Boolean.TRUE).build();
                Long cost = Long.valueOf(base / 10000000L);
                fcnc = fcb.setCurrentFeature(pf).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(cost, vport.getCost());

                pf = pfb.setTenMbHd(Boolean.FALSE).setTenGbFd(Boolean.TRUE).
                    build();
                cost = Long.valueOf(base / 10000000000L);
                fcnc = fcb.setCurrentFeature(pf).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(cost, vport.getCost());

                pf = pfb.setTenGbFd(Boolean.FALSE).setOneTbFd(Boolean.TRUE).
                    build();
                cost = Long.valueOf(base / 1000000000000L);
                fcnc = fcb.setCurrentFeature(pf).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(cost, vport.getCost());

                // current-speed should precede port-features.
                long speed = 100000000L;
                cost = Long.valueOf(base / speed);
                fcnc = fcb.setCurrentSpeed(speed / 1000L).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(cost, vport.getCost());

                speed = (long)0xffffffffL;
                cost = Long.valueOf(base / (speed * 1000L));
                fcnc = fcb.setCurrentSpeed(speed).build();
                vport = InventoryUtils.toVtnPortBuilder(ncid, fcnc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(cost, vport.getCost());
            }
        }
    }

    /**
     * Test case for
     * {@link InventoryUtils#toVtnLinkBuilder(LinkId, SalPort, SalPort)} and
     * {@link InventoryUtils#toIgnoredLinkBuilder(LinkId, SalPort, SalPort)}.
     */
    @Test
    public void testToVtnLinkBuilder() {
        int link = 1;
        for (long srcDpid = 1L; srcDpid <= 5; srcDpid++) {
            String srcNode = "openflow:" + srcDpid;
            for (long srcPort = 1L; srcPort <= 5L; srcPort++) {
                SalPort src = new SalPort(srcDpid, srcPort);
                NodeConnectorId srcId =
                    new NodeConnectorId(srcNode + ":" + srcPort);
                for (long dstDpid = 100L; dstDpid <= 105L; dstDpid++) {
                    String dstNode = "openflow:" + dstDpid;
                    for (long dstPort = 1L; dstPort <= 5L; dstPort++) {
                        SalPort dst = new SalPort(dstDpid, dstPort);
                        NodeConnectorId dstId =
                            new NodeConnectorId(dstNode + ":" + dstPort);
                        LinkId lid = new LinkId("link-" + link);
                        VtnLinkBuilder builder = InventoryUtils.
                            toVtnLinkBuilder(lid, src, dst);
                        assertEquals(lid, builder.getLinkId());
                        assertEquals(srcId, builder.getSource());
                        assertEquals(dstId, builder.getDestination());

                        IgnoredLinkBuilder ibuilder = InventoryUtils.
                            toIgnoredLinkBuilder(lid, src, dst);
                        assertEquals(lid, ibuilder.getLinkId());
                        assertEquals(srcId, ibuilder.getSource());
                        assertEquals(dstId, ibuilder.getDestination());
                        link++;
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#toPortLinkBuilder(LinkId, SalPort)}.
     */
    @Test
    public void testToPortLinkBuilder() {
        int link = 1;
        for (long dpid = 1L; dpid <= 10; dpid++) {
            String nodeId = "openflow:" + dpid;
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                NodeConnectorId ncId =
                    new NodeConnectorId(nodeId + ":" + port);
                LinkId lid = new LinkId("link-" + link);
                PortLinkBuilder builder = InventoryUtils.
                    toPortLinkBuilder(lid, sport);
                assertEquals(lid, builder.getLinkId());
                assertEquals(ncId, builder.getPeer());
                link++;
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#getOpenflowVersion(FlowCapableNodeConnector)}.
     */
    @Test
    public void testGetOpenflowVersion() {
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            for (long port = 1L; port <= 10L; port++) {
                String pid = "openflow:" + dpid + ":" + port;
                FlowCapableNodeConnector fcnc = null;
                assertEquals(VtnOpenflowVersion.OF10,
                             InventoryUtils.getOpenflowVersion(fcnc));

                FlowCapableNodeConnectorBuilder fcb =
                    new FlowCapableNodeConnectorBuilder();
                fcnc = fcb.build();
                assertEquals(VtnOpenflowVersion.OF10,
                             InventoryUtils.getOpenflowVersion(fcnc));

                fcnc = fcb.setCurrentSpeed(100000L).build();
                assertEquals(VtnOpenflowVersion.OF13,
                             InventoryUtils.getOpenflowVersion(fcnc));
            }
        }
    }

    /**
     * Test case for {@link InventoryUtils#getLinkSpeed(PortFeatures)}.
     */
    @Test
    public void testGetLinkSpeed() {
        final long spd1m = 1000000L;
        final long spd10m = spd1m * 10L;
        final long spd100m = spd1m * 100L;
        final long spd1g = spd1m * 1000L;
        final long spd10g = spd1g * 10L;
        final long spd40g = spd1g * 40L;
        final long spd100g = spd1g * 100L;
        final long spd1t = spd1g * 1000L;

        // Default is 1G.
        PortFeaturesBuilder pfb = new PortFeaturesBuilder();
        PortFeatures pf = pfb.build();
        assertEquals(spd1g, InventoryUtils.getLinkSpeed(null));
        assertEquals(spd1g, InventoryUtils.getLinkSpeed(pf));

        // 10M
        pf = pfb.setTenMbHd(Boolean.TRUE).build();
        assertEquals(spd10m, InventoryUtils.getLinkSpeed(pf));
        pf = pfb.setTenMbHd(Boolean.FALSE).setTenMbFd(Boolean.TRUE).build();
        assertEquals(spd10m, InventoryUtils.getLinkSpeed(pf));

        // 100M
        pf = pfb.setTenMbFd(Boolean.FALSE).setHundredMbHd(Boolean.TRUE).
            build();
        assertEquals(spd100m, InventoryUtils.getLinkSpeed(pf));
        pf = pfb.setHundredMbHd(Boolean.FALSE).setHundredMbFd(Boolean.TRUE).
            build();
        assertEquals(spd100m, InventoryUtils.getLinkSpeed(pf));

        // 1G
        pf = pfb.setHundredMbFd(Boolean.FALSE).setOneGbHd(Boolean.TRUE).
            build();
        assertEquals(spd1g, InventoryUtils.getLinkSpeed(pf));
        pf = pfb.setOneGbHd(Boolean.FALSE).setOneGbFd(Boolean.TRUE).build();
        assertEquals(spd1g, InventoryUtils.getLinkSpeed(pf));

        // 10G
        pf = pfb.setOneGbFd(Boolean.FALSE).setTenGbFd(Boolean.TRUE).build();
        assertEquals(spd10g, InventoryUtils.getLinkSpeed(pf));

        // 40G
        pf = pfb.setTenGbFd(Boolean.FALSE).setFortyGbFd(Boolean.TRUE).build();
        assertEquals(spd40g, InventoryUtils.getLinkSpeed(pf));

        // 100G
        pf = pfb.setFortyGbFd(Boolean.FALSE).setHundredGbFd(Boolean.TRUE).
            build();
        assertEquals(spd100g, InventoryUtils.getLinkSpeed(pf));

        // 1T
        pf = pfb.setHundredGbFd(Boolean.FALSE).setOneTbFd(Boolean.TRUE).
            build();
        assertEquals(spd1t, InventoryUtils.getLinkSpeed(pf));
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link InventoryUtils#getNodeId(InstanceIdentifier)}</li>
     *   <li>{@link InventoryUtils#getNodeConnectorId(InstanceIdentifier)}</li>
     *   <li>{@link InventoryUtils#getLinkId(InstanceIdentifier)}</li>
     * </ul>
     */
    @Test
    public void testGetKey() {
        TopologyKey topoKey = new TopologyKey(new TopologyId(TOPOLOGY_ID));
        for (long dpid = 1L; dpid <= 5L; dpid++) {
            String nid = "openflow:" + dpid;
            NodeId nodeId = new NodeId(nid);
            NodeKey nodeKey = new NodeKey(nodeId);
            InstanceIdentifier<Node> path = InstanceIdentifier.
                builder(Nodes.class).
                child(Node.class, nodeKey).build();
            assertEquals(nodeId, InventoryUtils.getNodeId(path));
            assertEquals(null, InventoryUtils.getNodeConnectorId(path));
            assertEquals(null, InventoryUtils.getLinkId(path));

            for (long port = 1L; port <= 5L; port++) {
                String pid = nid + ";" + port;
                NodeConnectorId ncId = new NodeConnectorId(pid);
                NodeConnectorKey ncKey = new NodeConnectorKey(ncId);
                InstanceIdentifier<NodeConnector> ncPath = InstanceIdentifier.
                    builder(Nodes.class).
                    child(Node.class, nodeKey).
                    child(NodeConnector.class, ncKey).build();
                assertEquals(nodeId, InventoryUtils.getNodeId(ncPath));
                assertEquals(ncId, InventoryUtils.getNodeConnectorId(ncPath));
                assertEquals(null, InventoryUtils.getLinkId(ncPath));

                LinkId lid = new LinkId(pid);
                LinkKey lkey = new LinkKey(lid);
                InstanceIdentifier<Link> lpath = InstanceIdentifier.
                    builder(NetworkTopology.class).
                    child(Topology.class, topoKey).
                    child(Link.class, lkey).build();
                assertEquals(null, InventoryUtils.getNodeId(lpath));
                assertEquals(null, InventoryUtils.getNodeConnectorId(lpath));
                assertEquals(lid, InventoryUtils.getLinkId(lpath));
            }
        }
    }

    /**
     * VTN topology for test.
     */
    private final class TopologyLists {
        /**
         * A list of {@link VtnLink} instances.
         */
        private final List<VtnLink>  vtnLinks = new ArrayList<>();

        /**
         * A list of {@link IgnoredLink} instances.
         */
        private final List<IgnoredLink>  ignoredLinks = new ArrayList<>();

        /**
         * A map that keeps paths to {@link VtnLink} instances.
         *
         * <p>
         *   {@link Boolean#TRUE} means the VTN link to be removed.
         * </p>
         */
        private Map<InstanceIdentifier<VtnLink>, Boolean> vtnLinkPaths;

        /**
         * A map that keeps paths to {@link PortLink} intances.
         *
         * <p>
         *   {@link Boolean#TRUE} means the port link to be removed.
         * </p>
         */
        private Map<InstanceIdentifier<PortLink>, Boolean>  portLinkPaths;

        /**
         * A map that keeps paths to {@link IgnoredLink} instances.
         *
         * <p>
         *   {@link Boolean#TRUE} means the ignored link to be removed.
         * </p>
         */
        private Map<InstanceIdentifier<IgnoredLink>, Boolean> ignoredLinkPaths;

        /**
         * Return a list of {@link VtnLink} instances.
         *
         * @return  A list of {@link VtnLink} instances.
         */
        private List<VtnLink> getVtnLinks() {
            return vtnLinks;
        }

        /**
         * Return a list of {@link IgnoredLink} instances.
         *
         * @return  A list of {@link IgnoredLink} instances.
         */
        private List<IgnoredLink> getIgnoredLinks() {
            return ignoredLinks;
        }

        /**
         * Add a {@link VtnLink} instance.
         *
         * @param src  The source port.
         * @param dst  The destination port.
         */
        private void addVtnLink(SalPort src, SalPort dst) {
            LinkId lid = new LinkId(src.toString());
            VtnLink vlink = new VtnLinkBuilder().
                setLinkId(lid).setSource(src.getNodeConnectorId()).
                setDestination(dst.getNodeConnectorId()).build();
            vtnLinks.add(vlink);
        }

        /**
         * Add an {@link IgnoredLink} instance.
         *
         * @param src  The source port.
         * @param dst  The destination port.
         */
        private void addIgnoredLink(SalPort src, SalPort dst) {
            LinkId lid = new LinkId(src.toString());
            IgnoredLink ilink = new IgnoredLinkBuilder().
                setLinkId(lid).setSource(src.getNodeConnectorId()).
                setDestination(dst.getNodeConnectorId()).build();
            ignoredLinks.add(ilink);
        }

        /**
         * Set up the mock-up of MD-SAL datastore transaction.
         *
         * @param tx     A mock-up of {@link ReadWriteTransaction}.
         * @param snode  A {@link SalNode} to be removed.
         */
        private void setUp(ReadWriteTransaction tx, SalNode snode) {
            Map<InstanceIdentifier<PortLink>, Boolean> plinks =
                new HashMap<>();
            Map<InstanceIdentifier<VtnLink>, Boolean> rmLinks =
                new HashMap<>();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            long dpid = snode.getNodeNumber();
            for (VtnLink vlink: vtnLinks) {
                SalPort src = SalPort.create(vlink.getSource());
                SalPort dst = SalPort.create(vlink.getDestination());
                LinkId lid = vlink.getLinkId();
                boolean rmLink = false;
                boolean rmSrc = false;
                boolean rmDst = false;
                if (src.getNodeNumber() == dpid) {
                    rmLink = true;
                    if (dst.getNodeNumber() != dpid) {
                        rmDst = true;
                    }
                } else if (dst.getNodeNumber() == dpid) {
                    rmLink = true;
                    rmSrc = true;
                }

                InstanceIdentifier<VtnLink> vpath = InstanceIdentifier.
                    builder(VtnTopology.class).
                    child(VtnLink.class, new VtnLinkKey(lid)).build();
                assertEquals(null, rmLinks.put(vpath, rmLink));

                InstanceIdentifier<PortLink> ppath = InstanceIdentifier.
                    builder(VtnNodes.class).
                    child(VtnNode.class, src.getVtnNodeKey()).
                    child(VtnPort.class, src.getVtnPortKey()).
                    child(PortLink.class, new PortLinkKey(lid)).build();
                assertEquals(null, plinks.put(ppath, rmSrc));
                if (rmSrc) {
                    PortLink plink = new PortLinkBuilder().
                        setLinkId(lid).setPeer(dst.getNodeConnectorId()).
                        build();
                    when(tx.read(oper, ppath)).
                        thenReturn(getReadResult(plink));
                }

                ppath = InstanceIdentifier.builder(VtnNodes.class).
                    child(VtnNode.class, dst.getVtnNodeKey()).
                    child(VtnPort.class, dst.getVtnPortKey()).
                    child(PortLink.class, new PortLinkKey(lid)).build();
                assertEquals(null, plinks.put(ppath, rmDst));
                if (rmDst) {
                    PortLink plink = new PortLinkBuilder().
                        setLinkId(lid).setPeer(src.getNodeConnectorId()).
                        build();
                    when(tx.read(oper, ppath)).
                        thenReturn(getReadResult(plink));
                }
            }

            InstanceIdentifier<VtnTopology> rpath =
                InstanceIdentifier.create(VtnTopology.class);
            VtnTopology root = new VtnTopologyBuilder().
                setVtnLink(vtnLinks).build();
            when(tx.read(oper, rpath)).thenReturn(getReadResult(root));

            vtnLinkPaths = rmLinks;
            portLinkPaths = plinks;

            Map<InstanceIdentifier<IgnoredLink>, Boolean> ilinks =
                new HashMap<>();
            for (IgnoredLink ilink: ignoredLinks) {
                SalPort src = SalPort.create(ilink.getSource());
                SalPort dst = SalPort.create(ilink.getDestination());
                LinkId lid = ilink.getLinkId();
                boolean rmLink = (src.getNodeNumber() == dpid ||
                                  dst.getNodeNumber() == dpid);
                InstanceIdentifier<IgnoredLink> ipath = InstanceIdentifier.
                    builder(IgnoredLinks.class).
                    child(IgnoredLink.class, new IgnoredLinkKey(lid)).build();
                assertEquals(null, ilinks.put(ipath, rmLink));
            }

            InstanceIdentifier<IgnoredLinks> irpath =
                InstanceIdentifier.create(IgnoredLinks.class);
            IgnoredLinks iroot = new IgnoredLinksBuilder().
                setIgnoredLink(ignoredLinks).build();
            when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
            ignoredLinkPaths = ilinks;
        }

        /**
         * Verify results of
         * {@link InventoryUtils#removeVtnTopologyLink(ReadWriteTransaction,SalNode)}.
         *
         * @param tx       A mock-up of {@link ReadWriteTransaction}.
         * @param invoked  {@code true} means that the method was invoked.
         */
        private void verifyVtnLink(ReadWriteTransaction tx, boolean invoked) {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            InstanceIdentifier<VtnTopology> rpath =
                InstanceIdentifier.create(VtnTopology.class);
            if (invoked) {
                verify(tx).read(oper, rpath);
            } else {
                verify(tx, never()).read(oper, rpath);
            }

            for (Map.Entry<InstanceIdentifier<VtnLink>, Boolean> entry:
                     vtnLinkPaths.entrySet()) {
                InstanceIdentifier<VtnLink> vpath = entry.getKey();
                verify(tx, never()).read(oper, vpath);

                Boolean removed = entry.getValue();
                if (invoked && Boolean.TRUE.equals(removed)) {
                    verify(tx).delete(oper, vpath);
                } else {
                    verify(tx, never()).delete(oper, vpath);
                }
            }

            for (Map.Entry<InstanceIdentifier<PortLink>, Boolean> entry:
                     portLinkPaths.entrySet()) {
                InstanceIdentifier<PortLink> ppath = entry.getKey();
                Boolean removed = entry.getValue();
                if (invoked && Boolean.TRUE.equals(removed)) {
                    verify(tx).read(oper, ppath);
                    verify(tx).delete(oper, ppath);
                } else {
                    verify(tx, never()).read(oper, ppath);
                    verify(tx, never()).delete(oper, ppath);
                }
            }
        }

        /**
         * Verify results of
         * {@link InventoryUtils#removeIgnoredLink(ReadWriteTransaction,SalNode)}.
         *
         * @param tx     A mock-up of {@link ReadWriteTransaction}.
         * @param invoked  {@code true} means that the method was invoked.
         */
        private void verifyIgnoredLink(ReadWriteTransaction tx,
                                       boolean invoked) {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            InstanceIdentifier<IgnoredLinks> irpath =
                InstanceIdentifier.create(IgnoredLinks.class);
            if (invoked) {
                verify(tx).read(oper, irpath);
            } else {
                verify(tx, never()).read(oper, irpath);
            }

            for (Map.Entry<InstanceIdentifier<IgnoredLink>, Boolean> entry:
                     ignoredLinkPaths.entrySet()) {
                InstanceIdentifier<IgnoredLink> ipath = entry.getKey();
                verify(tx, never()).read(oper, ipath);

                Boolean removed = entry.getValue();
                if (invoked && Boolean.TRUE.equals(removed)) {
                    verify(tx).delete(oper, ipath);
                } else {
                    verify(tx, never()).delete(oper, ipath);
                }
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link InventoryUtils#removeVtnLink(ReadWriteTransaction,SalNode)}</li>
     *   <li>{@link InventoryUtils#removeVtnTopologyLink(ReadWriteTransaction,SalNode)}</li>
     *   <li>{@link InventoryUtils#removeIgnoredLink(ReadWriteTransaction,SalNode)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveVtnLinkByNode() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Root containers are not present.
        long dpid = 10L;
        SalNode snode = new SalNode(dpid);
        InstanceIdentifier<VtnTopology> rpath =
            InstanceIdentifier.create(VtnTopology.class);
        InstanceIdentifier<IgnoredLinks> irpath =
            InstanceIdentifier.create(IgnoredLinks.class);
        VtnTopology root = null;
        IgnoredLinks iroot = null;
        when(tx.read(oper, rpath)).thenReturn(getReadResult(root));
        when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
        InventoryUtils.removeVtnLink(tx, snode);
        verify(tx).read(oper, rpath);
        verify(tx).read(oper, irpath);

        InventoryUtils.removeVtnTopologyLink(tx, snode);
        verify(tx, times(2)).read(oper, rpath);
        verify(tx).read(oper, irpath);

        InventoryUtils.removeIgnoredLink(tx, snode);
        verify(tx, times(2)).read(oper, rpath);
        verify(tx, times(2)).read(oper, irpath);
        reset(tx);

        // Root containers contain null list.
        root = new VtnTopologyBuilder().build();
        iroot = new IgnoredLinksBuilder().build();
        when(tx.read(oper, rpath)).thenReturn(getReadResult(root));
        when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
        InventoryUtils.removeVtnLink(tx, snode);
        verify(tx).read(oper, rpath);
        verify(tx).read(oper, irpath);

        InventoryUtils.removeVtnTopologyLink(tx, snode);
        verify(tx, times(2)).read(oper, rpath);
        verify(tx).read(oper, irpath);

        InventoryUtils.removeIgnoredLink(tx, snode);
        verify(tx, times(2)).read(oper, rpath);
        verify(tx, times(2)).read(oper, irpath);
        reset(tx);

        // Root containers contain empty list.
        List<VtnLink> vlinks = new ArrayList<>();
        List<IgnoredLink> ilinks = new ArrayList<>();
        root = new VtnTopologyBuilder().setVtnLink(vlinks).build();
        iroot = new IgnoredLinksBuilder().setIgnoredLink(ilinks).build();
        when(tx.read(oper, rpath)).thenReturn(getReadResult(root));
        when(tx.read(oper, irpath)).thenReturn(getReadResult(iroot));
        InventoryUtils.removeVtnLink(tx, snode);
        verify(tx).read(oper, rpath);
        verify(tx).read(oper, irpath);

        InventoryUtils.removeVtnTopologyLink(tx, snode);
        verify(tx, times(2)).read(oper, rpath);
        verify(tx).read(oper, irpath);

        InventoryUtils.removeIgnoredLink(tx, snode);
        verify(tx, times(2)).read(oper, rpath);
        verify(tx, times(2)).read(oper, irpath);
        reset(tx);

        SalPortGenerator gen = new SalPortGenerator(dpid);
        SalPortGenerator gen1 = new SalPortGenerator(dpid + 1L);
        SalPortGenerator gen2 = new SalPortGenerator(dpid + 2L);
        SalPortGenerator[] peers = {
            gen1, gen2,
        };
        TopologyLists topo = new TopologyLists();
        for (int i = 0; i < 5; i++) {
            SalPort src = gen.newInstance();
            SalPort dst = gen.newInstance();
            topo.addVtnLink(src, dst);

            src = gen.newInstance();
            dst = gen.newInstance();
            topo.addIgnoredLink(src, dst);

            for (SalPortGenerator pgen: peers) {
                src = gen.newInstance();
                dst = pgen.newInstance();
                topo.addVtnLink(src, dst);

                src = gen.newInstance();
                dst = pgen.newInstance();
                topo.addIgnoredLink(src, dst);

                src = pgen.newInstance();
                dst = gen.newInstance();
                topo.addVtnLink(src, dst);

                src = pgen.newInstance();
                dst = gen.newInstance();
                topo.addIgnoredLink(src, dst);
            }

            src = gen1.newInstance();
            dst = gen2.newInstance();
            topo.addVtnLink(src, dst);

            src = gen1.newInstance();
            dst = gen2.newInstance();
            topo.addIgnoredLink(src, dst);

            src = gen2.newInstance();
            dst = gen1.newInstance();
            topo.addVtnLink(src, dst);

            src = gen2.newInstance();
            dst = gen1.newInstance();
            topo.addIgnoredLink(src, dst);
        }

        topo.setUp(tx, snode);
        InventoryUtils.removeVtnLink(tx, snode);
        topo.verifyVtnLink(tx, true);
        topo.verifyIgnoredLink(tx, true);
        reset(tx);

        topo.setUp(tx, snode);
        InventoryUtils.removeVtnTopologyLink(tx, snode);
        topo.verifyVtnLink(tx, true);
        topo.verifyIgnoredLink(tx, false);
        reset(tx);

        topo.setUp(tx, snode);
        InventoryUtils.removeIgnoredLink(tx, snode);
        topo.verifyVtnLink(tx, false);
        topo.verifyIgnoredLink(tx, true);
        reset(tx);
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link InventoryUtils#removeVtnLink(ReadWriteTransaction,VtnPort)}</li>
     *   <li>{@link InventoryUtils#removePortLink(ReadWriteTransaction,SalPort,LinkId)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemoveVtnLinkByPort() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Port link is null.
        VtnPort vport = new VtnPortBuilder().build();
        InventoryUtils.removeVtnLink(tx, vport);
        verifyZeroInteractions(tx);

        // Port link is empty.
        List<PortLink> plist = new ArrayList<>();
        vport = new VtnPortBuilder().setPortLink(plist).build();
        InventoryUtils.removeVtnLink(tx, vport);
        verifyZeroInteractions(tx);

        Map<InstanceIdentifier<VtnLink>, VtnLink> vtnLinks = new HashMap<>();
        Map<InstanceIdentifier<PortLink>, PortLink> peerLinks =
            new HashMap<>();
        Set<InstanceIdentifier<PortLink>> portLinks = new HashSet<>();
        long dpid = 10L;
        final SalPort sport = new SalPort(dpid, 3L);
        final SalPort peer = new SalPort(dpid + 10L, 1L);

        LinkId lid1 = new LinkId(sport.toString());
        InstanceIdentifier<PortLink> portPath1 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, sport.getVtnNodeKey()).
            child(VtnPort.class, sport.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid1)).build();
        PortLink portLink1 = new PortLinkBuilder().
            setLinkId(lid1).setPeer(peer.getNodeConnectorId()).build();
        InstanceIdentifier<PortLink> peerPath1 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, peer.getVtnNodeKey()).
            child(VtnPort.class, peer.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid1)).build();
        InstanceIdentifier<VtnLink> vpath1 = InstanceIdentifier.
            builder(VtnTopology.class).
            child(VtnLink.class, new VtnLinkKey(lid1)).build();

        LinkId lid2 = new LinkId(peer.toString());
        InstanceIdentifier<PortLink> portPath2 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, sport.getVtnNodeKey()).
            child(VtnPort.class, sport.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid2)).build();
        PortLink portLink2 = new PortLinkBuilder().
            setLinkId(lid2).setPeer(peer.getNodeConnectorId()).build();
        InstanceIdentifier<PortLink> peerPath2 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, peer.getVtnNodeKey()).
            child(VtnPort.class, peer.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid2)).build();
        InstanceIdentifier<VtnLink> vpath2 = InstanceIdentifier.
            builder(VtnTopology.class).
            child(VtnLink.class, new VtnLinkKey(lid2)).build();
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, portLink1, portLink2);
        vport = new VtnPortBuilder().setId(sport.getNodeConnectorId()).
            setPortLink(plinks).build();

        // No link information is present.
        when(tx.read(oper, peerPath1)).
            thenReturn(getReadResult((PortLink)null));
        when(tx.read(oper, peerPath2)).
            thenReturn(getReadResult((PortLink)null));
        when(tx.read(oper, vpath1)).
            thenReturn(getReadResult((VtnLink)null));
        when(tx.read(oper, vpath2)).
            thenReturn(getReadResult((VtnLink)null));

        InventoryUtils.removeVtnLink(tx, vport);
        verify(tx).read(oper, peerPath1);
        verify(tx).read(oper, peerPath2);
        verify(tx).read(oper, vpath1);
        verify(tx).read(oper, vpath2);
        verify(tx, never()).delete(oper, peerPath1);
        verify(tx, never()).delete(oper, peerPath2);
        verify(tx, never()).delete(oper, vpath1);
        verify(tx, never()).delete(oper, vpath2);
        reset(tx);

        // Link information is present.
        PortLink peerLink1 = new PortLinkBuilder().
            setLinkId(lid1).setPeer(sport.getNodeConnectorId()).build();
        PortLink peerLink2 = new PortLinkBuilder().
            setLinkId(lid2).setPeer(sport.getNodeConnectorId()).build();
        VtnLink vlink1 = new VtnLinkBuilder().
            setLinkId(lid1).setSource(sport.getNodeConnectorId()).
            setDestination(peer.getNodeConnectorId()).build();
        VtnLink vlink2 = new VtnLinkBuilder().
            setLinkId(lid2).setSource(peer.getNodeConnectorId()).
            setDestination(sport.getNodeConnectorId()).build();
        when(tx.read(oper, peerPath1)).thenReturn(getReadResult(peerLink1));
        when(tx.read(oper, peerPath2)).thenReturn(getReadResult(peerLink2));
        when(tx.read(oper, vpath1)).thenReturn(getReadResult(vlink1));
        when(tx.read(oper, vpath2)).thenReturn(getReadResult(vlink2));

        InventoryUtils.removeVtnLink(tx, vport);
        verify(tx).read(oper, peerPath1);
        verify(tx).read(oper, peerPath2);
        verify(tx).read(oper, vpath1);
        verify(tx).read(oper, vpath2);
        verify(tx).delete(oper, peerPath1);
        verify(tx).delete(oper, peerPath2);
        verify(tx).delete(oper, vpath1);
        verify(tx).delete(oper, vpath2);
    }

    /**
     * Test case for
     * {@link InventoryUtils#clearPortLink(ReadWriteTransaction,SalPort,VtnPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClearPortLink() throws Exception {
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        SalPort sport = new SalPort(10L, 20L);

        // Port link is null.
        VtnPort vport = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).build();
        InventoryUtils.clearPortLink(tx, sport, vport);
        verifyZeroInteractions(tx);

        // Port link is empty.
        List<PortLink> plist = new ArrayList<>();
        vport = new VtnPortBuilder().setId(sport.getNodeConnectorId()).
            setPortLink(plist).build();
        InventoryUtils.clearPortLink(tx, sport, vport);
        verifyZeroInteractions(tx);

        Map<InstanceIdentifier<VtnLink>, VtnLink> vtnLinks = new HashMap<>();
        Map<InstanceIdentifier<PortLink>, PortLink> peerLinks =
            new HashMap<>();
        Set<InstanceIdentifier<PortLink>> portLinks = new HashSet<>();
        final SalPort peer = new SalPort(20L, 30L);

        LinkId lid1 = new LinkId(sport.toString());
        InstanceIdentifier<PortLink> portPath1 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, sport.getVtnNodeKey()).
            child(VtnPort.class, sport.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid1)).build();
        PortLink portLink1 = new PortLinkBuilder().
            setLinkId(lid1).setPeer(peer.getNodeConnectorId()).build();
        InstanceIdentifier<PortLink> peerPath1 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, peer.getVtnNodeKey()).
            child(VtnPort.class, peer.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid1)).build();
        InstanceIdentifier<VtnLink> vpath1 = InstanceIdentifier.
            builder(VtnTopology.class).
            child(VtnLink.class, new VtnLinkKey(lid1)).build();

        LinkId lid2 = new LinkId(peer.toString());
        InstanceIdentifier<PortLink> portPath2 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, sport.getVtnNodeKey()).
            child(VtnPort.class, sport.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid2)).build();
        PortLink portLink2 = new PortLinkBuilder().
            setLinkId(lid2).setPeer(peer.getNodeConnectorId()).build();
        InstanceIdentifier<PortLink> peerPath2 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, peer.getVtnNodeKey()).
            child(VtnPort.class, peer.getVtnPortKey()).
            child(PortLink.class, new PortLinkKey(lid2)).build();
        InstanceIdentifier<VtnLink> vpath2 = InstanceIdentifier.
            builder(VtnTopology.class).
            child(VtnLink.class, new VtnLinkKey(lid2)).build();
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, portLink1, portLink2);
        vport = new VtnPortBuilder().setId(sport.getNodeConnectorId()).
            setPortLink(plinks).build();

        // No link information is present.
        when(tx.read(oper, peerPath1)).
            thenReturn(getReadResult((PortLink)null));
        when(tx.read(oper, peerPath2)).
            thenReturn(getReadResult((PortLink)null));
        when(tx.read(oper, vpath1)).
            thenReturn(getReadResult((VtnLink)null));
        when(tx.read(oper, vpath2)).
            thenReturn(getReadResult((VtnLink)null));

        InventoryUtils.clearPortLink(tx, sport, vport);
        verify(tx).read(oper, peerPath1);
        verify(tx).read(oper, peerPath2);
        verify(tx).read(oper, vpath1);
        verify(tx).read(oper, vpath2);
        verify(tx).delete(oper, portPath1);
        verify(tx).delete(oper, portPath2);
        verifyNoMoreInteractions(tx);
        reset(tx);

        // Link information is present.
        PortLink peerLink1 = new PortLinkBuilder().
            setLinkId(lid1).setPeer(sport.getNodeConnectorId()).build();
        PortLink peerLink2 = new PortLinkBuilder().
            setLinkId(lid2).setPeer(sport.getNodeConnectorId()).build();
        VtnLink vlink1 = new VtnLinkBuilder().
            setLinkId(lid1).setSource(sport.getNodeConnectorId()).
            setDestination(peer.getNodeConnectorId()).build();
        VtnLink vlink2 = new VtnLinkBuilder().
            setLinkId(lid2).setSource(peer.getNodeConnectorId()).
            setDestination(sport.getNodeConnectorId()).build();
        when(tx.read(oper, peerPath1)).thenReturn(getReadResult(peerLink1));
        when(tx.read(oper, peerPath2)).thenReturn(getReadResult(peerLink2));
        when(tx.read(oper, vpath1)).thenReturn(getReadResult(vlink1));
        when(tx.read(oper, vpath2)).thenReturn(getReadResult(vlink2));

        InventoryUtils.clearPortLink(tx, sport, vport);
        verify(tx).read(oper, peerPath1);
        verify(tx).read(oper, peerPath2);
        verify(tx).read(oper, vpath1);
        verify(tx).read(oper, vpath2);
        verify(tx).delete(oper, portPath1);
        verify(tx).delete(oper, portPath2);
        verify(tx).delete(oper, peerPath1);
        verify(tx).delete(oper, peerPath2);
        verify(tx).delete(oper, vpath1);
        verify(tx).delete(oper, vpath2);
        verifyNoMoreInteractions(tx);
    }
}
