/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.IdentifiableItem;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.Item;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkKey;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnectorBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.State;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.StateBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * JUnit test for {@link InventoryUtils}.
 */
public class InventoryUtilsTest extends TestBase {
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
    public void testVtnPortToString() {
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
     * Test case for {@link InventoryUtils#toVtnNodeBuilder(Node)} and
     * {@link InventoryUtils#getVtnPorts(List, Node)}.
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
     * Test case for {@link InventoryUtils#toVtnPortBuilder(NodeConnector)}.
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
                NodeConnectorBuilder ncBuilder = new NodeConnectorBuilder().
                    setId(ncid);
                NodeConnector nc = ncBuilder.build();
                VtnPort vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(pid, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                FlowCapableNodeConnectorBuilder fcb =
                    new FlowCapableNodeConnectorBuilder();
                FlowCapableNodeConnector fcnc = fcb.build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(pid, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                // Set port name.
                String name = "switch-" + dpid + "-port-" + port;
                fcnc = fcb.setName(name).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
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
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                pcf = pcfb.setNoFwd(Boolean.TRUE).setNoPacketIn(Boolean.TRUE).
                    setNoRecv(Boolean.TRUE).setPortDown(Boolean.TRUE).build();
                fcnc = fcb.setConfiguration(pcf).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                StateBuilder stb = new StateBuilder();
                State st = stb.build();
                fcnc = fcb.setState(st).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                st = stb.setBlocked(Boolean.TRUE).setLive(Boolean.TRUE).
                    setLinkDown(Boolean.TRUE).build();
                fcnc = fcb.setState(st).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                st = stb.setLinkDown(Boolean.FALSE).build();
                fcnc = fcb.setState(st).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                pcf = pcfb.setPortDown(Boolean.FALSE).build();
                st = stb.setLinkDown(Boolean.TRUE).build();
                fcnc = fcb.setState(st).setConfiguration(pcf).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.FALSE, vport.isEnabled());
                assertEquals(defCost, vport.getCost());

                st = stb.setLinkDown(Boolean.FALSE).build();
                fcnc = fcb.setState(st).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
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
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
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
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
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
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
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
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
                assertEquals(ncid, vport.getId());
                assertEquals(key, vport.getKey());
                assertEquals(name, vport.getName());
                assertEquals(null, vport.getPortLink());
                assertEquals(Boolean.TRUE, vport.isEnabled());
                assertEquals(cost, vport.getCost());

                speed = (long)0xffffffffL;
                cost = Long.valueOf(base / (speed * 1000L));
                fcnc = fcb.setCurrentSpeed(speed).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                vport = InventoryUtils.toVtnPortBuilder(nc).build();
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
     * Test case for {@link InventoryUtils#getOpenflowVersion(NodeConnector)}.
     */
    @Test
    public void testGetOpenflowVersion() {
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            for (long port = 1L; port <= 10L; port++) {
                String pid = "openflow:" + dpid + ":" + port;
                NodeConnectorId ncid = new NodeConnectorId(pid);
                NodeConnectorBuilder ncBuilder = new NodeConnectorBuilder().
                    setId(ncid);
                NodeConnector nc = ncBuilder.build();
                assertEquals(VtnOpenflowVersion.OF10,
                             InventoryUtils.getOpenflowVersion(nc));

                FlowCapableNodeConnectorBuilder fcb =
                    new FlowCapableNodeConnectorBuilder();
                FlowCapableNodeConnector fcnc = fcb.build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                assertEquals(VtnOpenflowVersion.OF10,
                             InventoryUtils.getOpenflowVersion(nc));

                fcnc = fcb.setCurrentSpeed(100000L).build();
                nc = ncBuilder.
                    addAugmentation(FlowCapableNodeConnector.class, fcnc).
                    build();
                assertEquals(VtnOpenflowVersion.OF13,
                             InventoryUtils.getOpenflowVersion(nc));
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
}
