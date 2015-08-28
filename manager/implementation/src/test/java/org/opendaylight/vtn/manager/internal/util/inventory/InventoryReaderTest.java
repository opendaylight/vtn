/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.vtn.manager.internal.PortFilter;
import org.opendaylight.vtn.manager.internal.SpecificPortFilter;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link InventoryReader}.
 */
public class InventoryReaderTest extends TestBase {
    /**
     * Test case for {@link InventoryReader#get(SalNode)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetVtnNode() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        assertEquals(null, reader.get((SalNode)null));

        Map<SalNode, VtnNode> expected = new HashMap<>();
        List<SalNode> negative = new ArrayList<>();
        List<SalPort> negativePorts = new ArrayList<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        final long nports = 10L;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            List<VtnPort> portList = new ArrayList<>();
            for (long port = 1L; port <= nports; port++) {
                // Create negative cache entry for this port.
                SalPort sport = new SalPort(dpid, port);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
                portList.add(createVtnPortBuilder(sport).build());
            }
            for (long port = 100L; port <= 110L; port++) {
                SalPort sport = new SalPort(dpid, port);
                negativePorts.add(sport);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = null;
                when(rtx.read(oper, path)).thenReturn(getReadResult(vport));
            }

            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).setVtnPort(portList).build();
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
            assertNull(expected.put(snode, vnode));
        }

        for (long dpid = 100L; dpid <= 110L; dpid++) {
            SalNode snode = new SalNode(dpid);
            negative.add(snode);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = null;
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
        }

        Map<SalPort, VtnPort> cachedPorts = new HashMap<>();
        for (int i = 0; i < 10; i++) {
            for (Map.Entry<SalNode, VtnNode> entry: expected.entrySet()) {
                SalNode snode = entry.getKey();
                VtnNode vnode = entry.getValue();
                assertEquals(vnode, reader.get(snode));
                for (VtnPort vport: vnode.getVtnPort()) {
                    SalPort sport = SalPort.create(vport.getId());
                    assertNotNull(sport);
                    assertEquals(vport, reader.get(sport));
                    cachedPorts.put(sport, vport);
                }
            }
            for (SalNode snode: negative) {
                assertEquals(null, reader.get(snode));
            }
            for (SalPort sport: negativePorts) {
                assertEquals(null, reader.get(sport));
                cachedPorts.put(sport, null);
            }
        }

        Map<SalNode, VtnNode> cached = new HashMap<>(expected);
        for (Map.Entry<SalNode, VtnNode> entry: expected.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNode vnode = entry.getValue();
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);

            for (VtnPort vport: vnode.getVtnPort()) {
                SalPort sport = SalPort.create(vport.getId());
                assertNotNull(sport);
                InstanceIdentifier<VtnPort> ppath =
                    sport.getVtnPortIdentifier();
                verify(rtx, never()).read(oper, ppath);
            }
        }
        for (SalNode snode: negative) {
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);
            assertNull(cached.put(snode, null));
        }

        assertEquals(cached, reader.getCachedNodes());
        assertEquals(cachedPorts, reader.getCachedPorts());
    }

    /**
     * Test case for {@link InventoryReader#get(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetVtnPort() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        assertEquals(null, reader.get((SalPort)null));

        Map<SalPort, VtnPort> expected = new HashMap<>();
        List<SalPort> negative = new ArrayList<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            for (long port = 1L; port <= 10L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = createVtnPortBuilder(sport).build();
                when(rtx.read(oper, path)).thenReturn(getReadResult(vport));
                assertNull(expected.put(sport, vport));
            }

            for (long port = 20L; port <= 30L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = null;
                when(rtx.read(oper, path)).thenReturn(getReadResult(vport));
                negative.add(sport);
            }
        }

        for (int i = 0; i < 10; i++) {
            for (Map.Entry<SalPort, VtnPort> entry: expected.entrySet()) {
                SalPort sport = entry.getKey();
                VtnPort vport = entry.getValue();
                assertEquals(vport, reader.get(sport));
            }
            for (SalPort sport: negative) {
                assertEquals(null, reader.get(sport));
            }
        }

        // Results of read request should be cached.
        Map<SalPort, VtnPort> cached = new HashMap<>(expected);
        for (SalPort sport: expected.keySet()) {
            InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }
        for (SalPort sport: negative) {
            InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
            verify(rtx, times(1)).read(oper, path);
            assertNull(cached.put(sport, null));
        }

        assertTrue(reader.getCachedNodes().isEmpty());
        assertEquals(cached, reader.getCachedPorts());
    }

    /**
     * Test case for {@link InventoryReader#getVtnNodes()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetVtnNodes() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());

        // Create negative cache entries.
        Map<SalNode, VtnNode> expectedNodes = new HashMap<>();
        Map<SalPort, VtnPort> expectedPorts = new HashMap<>();
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            SalNode snode = new SalNode(dpid);
            reader.prefetch(snode, (VtnNode)null);
            assertEquals(null, reader.get(snode));
            assertNull(expectedNodes.put(snode, null));
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
                assertNull(expectedPorts.put(sport, null));
            }
        }

        assertEquals(expectedNodes, reader.getCachedNodes());
        assertEquals(expectedPorts, reader.getCachedPorts());

        expectedNodes.clear();
        expectedPorts.clear();
        Set<VtnNode> expected = new HashSet<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        final long nports = 10L;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            List<VtnPort> portList = new ArrayList<>();
            for (long port = 1L; port <= nports; port++) {
                SalPort sport = new SalPort(dpid, port);
                VtnPort vport = createVtnPortBuilder(sport).build();
                portList.add(vport);
                assertNull(expectedPorts.put(sport, vport));
            }

            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).setVtnPort(portList).build();
            assertNull(expectedNodes.put(snode, vnode));
            assertTrue(expected.add(vnode));
        }

        VtnNodes vnodes = new VtnNodesBuilder().
            setVtnNode(new ArrayList<VtnNode>(expected)).build();
        InstanceIdentifier<VtnNodes> root = InstanceIdentifier.
            create(VtnNodes.class);
        when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

        for (int i = 0; i < 10; i++) {
            assertEquals(expected, new HashSet<VtnNode>(reader.getVtnNodes()));

            for (Map.Entry<SalNode, VtnNode> entry: expectedNodes.entrySet()) {
                SalNode snode = entry.getKey();
                VtnNode vnode = entry.getValue();
                assertEquals(vnode, reader.get(snode));
                for (VtnPort vport: vnode.getVtnPort()) {
                    SalPort sport = SalPort.create(vport.getId());
                    assertNotNull(sport);
                    assertEquals(vport, reader.get(sport));
                }
            }
        }

        verify(rtx, times(1)).read(oper, root);

        for (Map.Entry<SalNode, VtnNode> entry: expectedNodes.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNode vnode = entry.getValue();
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, never()).read(oper, path);

            for (VtnPort vport: vnode.getVtnPort()) {
                SalPort sport = SalPort.create(vport.getId());
                assertNotNull(sport);
                InstanceIdentifier<VtnPort> ppath =
                    sport.getVtnPortIdentifier();
                verify(rtx, never()).read(oper, ppath);
            }
        }

        assertEquals(expectedNodes, reader.getCachedNodes());
        assertEquals(expectedPorts, reader.getCachedPorts());

        // Test with an empty VtnNodes.
        rtx = mock(ReadTransaction.class);
        reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            SalNode snode = new SalNode(dpid);
            reader.prefetch(snode, (VtnNode)null);
            assertEquals(null, reader.get(snode));
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
            }
        }

        vnodes = null;
        when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

        for (int i = 0; i < 10; i++) {
            assertTrue(reader.getVtnNodes().isEmpty());
        }

        verify(rtx, times(1)).read(oper, root);
        assertTrue(reader.getCachedNodes().isEmpty());
        assertTrue(reader.getCachedPorts().isEmpty());
    }

    /**
     * Test case for prefetching {@link VtnNode}.
     *
     * <ul>
     *   <li>{@link InventoryReader#prefetch(SalNode, VtnNode)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPrefetchVtnNode() throws Exception {
        InventoryReader reader = new InventoryReader(null);
        Map<SalNode, VtnNode> expected = new HashMap<>();
        final long nports = 10L;
        long additional = 0;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            List<VtnPort> portList = new ArrayList<>();
            for (long port = 1L; port <= nports; port++) {
                // Create negative cache entry for this port.
                SalPort sport = new SalPort(dpid, port);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
                portList.add(createVtnPortBuilder(sport).build());
            }
            for (long port = 100L; port <= 110L; port++) {
                SalPort sport = new SalPort(dpid, port);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
                additional++;
            }

            SalNode snode = new SalNode(dpid);
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).setVtnPort(portList).build();
            assertNull(expected.put(snode, vnode));
        }

        assertTrue(reader.getCachedNodes().isEmpty());
        assertEquals(expected.size() * nports + additional,
                     reader.getCachedPorts().size());

        List<SalNode> negative = new ArrayList<>();
        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            negative.add(snode);
            reader.prefetch(snode, (VtnNode)null);
            assertEquals(null, reader.get(snode));
        }

        assertEquals(negative.size(), reader.getCachedNodes().size());
        assertEquals(expected.size() * nports + additional,
                     reader.getCachedPorts().size());

        for (Map.Entry<SalNode, VtnNode> entry: expected.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNode vnode = entry.getValue();
            reader.prefetch(snode, vnode);
        }

        // Cached ports related to prefetched nodes should be purged.
        assertEquals(negative.size() + expected.size(),
                     reader.getCachedNodes().size());
        assertEquals(expected.size() * nports,
                     reader.getCachedPorts().size());

        for (Map.Entry<SalNode, VtnNode> entry: expected.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNode vnode = entry.getValue();
            assertEquals(vnode, reader.get(snode));
            for (VtnPort vport: vnode.getVtnPort()) {
                SalPort sport = SalPort.create(vport.getId());
                assertNotNull(sport);
                assertEquals(vport, reader.get(sport));
            }
        }
    }

    /**
     * Test case for prefetching {@link VtnPort}.
     *
     * <ul>
     *   <li>{@link InventoryReader#prefetch(Collection)}</li>
     *   <li>{@link InventoryReader#prefetch(VtnPort)}</li>
     *   <li>{@link InventoryReader#prefetch(SalPort, VtnPort)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPrefetchVtnPort() throws Exception {
        InventoryReader reader = new InventoryReader(null);
        reader.prefetch((Collection<VtnPort>)null);
        reader.prefetch((VtnPort)null);
        assertTrue(reader.getCachedPorts().isEmpty());

        Map<SalPort, VtnPort> expected = new HashMap<>();
        List<SalPort> negative = new ArrayList<>();
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            for (long port = 1L; port <= 10L; port++) {
                SalPort sport = new SalPort(dpid, port);
                VtnPort vport = createVtnPortBuilder(sport).build();
                assertNull(expected.put(sport, vport));
            }
            for (long port = 20L; port <= 30L; port++) {
                SalPort sport = new SalPort(dpid, port);
                negative.add(sport);
                reader.prefetch(sport, (VtnPort)null);
                assertEquals(null, reader.get(sport));
            }
        }

        assertEquals(negative.size(), reader.getCachedPorts().size());
        reader.prefetch(expected.values());
        assertEquals(expected.size() + negative.size(),
                     reader.getCachedPorts().size());

        for (Map.Entry<SalPort, VtnPort> entry: expected.entrySet()) {
            SalPort sport = entry.getKey();
            VtnPort vport = entry.getValue();
            assertEquals(vport, reader.get(sport));
        }
        for (SalPort sport: negative) {
            assertEquals(null, reader.get(sport));

            // Update negative cache.
            VtnPort vport = createVtnPortBuilder(sport).build();
            reader.prefetch(sport, vport);
            assertEquals(vport, reader.get(sport));
        }
    }

    /**
     * Test case for {@link InventoryReader#exists(SalNode)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExists() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        assertFalse(reader.exists((SalNode)null));

        Map<SalNode, Boolean> expected = new HashMap<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        boolean result = true;
        for (long dpid = 1L; dpid <= 30L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = (result)
                ? new VtnNodeBuilder().setId(snode.getNodeId()).build()
                : null;
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
            assertNull(expected.put(snode, Boolean.valueOf(result)));
            result = !result;
        }

        for (int i = 0; i < 10; i++) {
            for (Map.Entry<SalNode, Boolean> entry: expected.entrySet()) {
                SalNode snode = entry.getKey();
                Boolean res = entry.getValue();
                assertEquals(res.booleanValue(), reader.exists(snode));
            }
        }

        for (SalNode snode: expected.keySet()) {
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }
    }

    /**
     * Test case for {@link InventoryReader#isEnabled(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIsEnabled() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        assertFalse(reader.isEnabled((SalPort)null));

        Map<SalPort, Boolean> expected = new HashMap<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Boolean enabled = Boolean.TRUE;
        for (long dpid = 1L; dpid <= 10L; dpid++) {
            for (long port = 1L; port <= 20L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = createVtnPortBuilder(sport, enabled, true).
                    build();
                when(rtx.read(oper, path)).thenReturn(getReadResult(vport));
                Boolean en = Boolean.valueOf(Boolean.TRUE.equals(enabled));
                assertNull(expected.put(sport, en));
                enabled = triState(enabled);
            }
        }

        for (int i = 0; i < 10; i++) {
            for (Map.Entry<SalPort, Boolean> entry: expected.entrySet()) {
                SalPort sport = entry.getKey();
                Boolean result = entry.getValue();
                assertEquals(result.booleanValue(), reader.isEnabled(sport));
            }
        }

        for (SalPort sport: expected.keySet()) {
            InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }
    }

    /**
     * Test case for {@link InventoryReader#findPort(Node, SwitchPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFindPort1() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        assertEquals(null, reader.findPort((Node)null, (SwitchPort)null));

        Map<SalNode, VtnNode> nodeMap = new HashMap<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            List<VtnPort> portList = new ArrayList<>();
            for (long port = 1L; port <= 10L; port++) {
                SalPort sport = new SalPort(dpid, port);
                VtnPort vport = createVtnPortBuilder(sport).build();
                portList.add(vport);
            }

            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).setVtnPort(portList).build();
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
            assertNull(nodeMap.put(snode, vnode));
        }

        Set<SalNode> notPresent = new HashSet<>();
        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = null;
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
            assertTrue(notPresent.add(snode));
        }

        final String type = NodeConnectorIDType.OPENFLOW;
        final String badType = "UNKNOWN";
        for (int i = 0; i < 10; i++) {
            for (Map.Entry<SalNode, VtnNode> entry: nodeMap.entrySet()) {
                SalNode snode = entry.getKey();
                VtnNode vnode = entry.getValue();
                Node node = snode.getAdNode();
                for (VtnPort vport: vnode.getVtnPort()) {
                    // Search by port name.
                    SalPort sport = SalPort.create(vport.getId());
                    String name = vport.getName();
                    String id = String.valueOf(sport.getPortNumber());
                    SwitchPort swport = new SwitchPort(name);
                    assertEquals(sport, reader.findPort(node, swport));
                    for (int j = 0; j < 4; j++) {
                        String nm = name + "-" + j;
                        swport = new SwitchPort(nm);
                        assertEquals(null, reader.findPort(node, swport));

                        swport = new SwitchPort(nm, type, id);
                        assertEquals(null, reader.findPort(node, swport));
                    }

                    // Search by port type and ID.
                    swport = new SwitchPort(type, id);
                    assertEquals(sport, reader.findPort(node, swport));

                    swport = new SwitchPort(badType, id);
                    assertEquals(null, reader.findPort(node, swport));
                    for (int j = 0; j < 4; j++) {
                        String badId = String.valueOf(1000 + j);
                        swport = new SwitchPort(type, badId);
                        assertEquals(null, reader.findPort(node, swport));

                        swport = new SwitchPort(name, type, badId);
                        assertEquals(null, reader.findPort(node, swport));
                    }

                    // Seaerch by port name, type, and ID.
                    swport = new SwitchPort(name, type, id);
                    assertEquals(sport, reader.findPort(node, swport));
                    swport = new SwitchPort(name, badType, id);
                    assertEquals(null, reader.findPort(node, swport));
                }
            }

            for (long dpid = 10L; dpid <= 15L; dpid++) {
                SalNode snode = new SalNode(dpid);
                assertEquals(null, reader.findPort(snode.getAdNode(), null));
            }
        }

        for (SalNode snode: nodeMap.keySet()) {
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }

        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }
    }

    /**
     * Test case for {@link InventoryReader#findPort(SalNode, VtnSwitchPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFindPort2() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        assertEquals(null, reader.findPort((SalNode)null, (VtnSwitchPort)null));

        Map<SalNode, VtnNode> nodeMap = new HashMap<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            List<VtnPort> portList = new ArrayList<>();
            for (long port = 1L; port <= 10L; port++) {
                SalPort sport = new SalPort(dpid, port);
                VtnPort vport = createVtnPortBuilder(sport).build();
                portList.add(vport);
            }

            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).setVtnPort(portList).build();
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
            assertNull(nodeMap.put(snode, vnode));
        }

        Set<SalNode> notPresent = new HashSet<>();
        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = null;
            when(rtx.read(oper, path)).thenReturn(getReadResult(vnode));
            assertTrue(notPresent.add(snode));
        }

        for (int i = 0; i < 10; i++) {
            for (Map.Entry<SalNode, VtnNode> entry: nodeMap.entrySet()) {
                SalNode snode = entry.getKey();
                VtnNode vnode = entry.getValue();
                for (VtnPort vport: vnode.getVtnPort()) {
                    // Search by port name.
                    SalPort sport = SalPort.create(vport.getId());
                    String name = vport.getName();
                    String id = String.valueOf(sport.getPortNumber());
                    VtnSwitchPort vswp = new DataFlowPortBuilder().
                        setPortName(name).build();
                    assertEquals(sport, reader.findPort(snode, vswp));
                    for (int j = 0; j < 4; j++) {
                        String nm = name + "-" + j;
                        vswp = new DataFlowPortBuilder().
                            setPortName(nm).build();
                        assertEquals(null, reader.findPort(snode, vswp));

                        vswp = new DataFlowPortBuilder().
                            setPortName(nm).setPortId(id).build();
                        assertEquals(null, reader.findPort(snode, vswp));
                    }

                    // Search by port ID.
                    vswp = new DataFlowPortBuilder().setPortId(id).build();
                    assertEquals(sport, reader.findPort(snode, vswp));

                    for (int j = 0; j < 4; j++) {
                        String badId = String.valueOf(1000 + j);
                        vswp = new DataFlowPortBuilder().
                            setPortId(badId).build();
                        assertEquals(null, reader.findPort(snode, vswp));

                        vswp = new DataFlowPortBuilder().
                            setPortName(name).setPortId(badId).build();
                        assertEquals(null, reader.findPort(snode, vswp));
                    }

                    // Seaerch by port name and type.
                    vswp = new DataFlowPortBuilder().
                        setPortName(name).setPortId(id).build();
                    assertEquals(sport, reader.findPort(snode, vswp));
                }
            }

            for (long dpid = 10L; dpid <= 15L; dpid++) {
                SalNode snode = new SalNode(dpid);
                assertEquals(null, reader.findPort(snode, null));
            }
        }

        for (SalNode snode: nodeMap.keySet()) {
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }

        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            verify(rtx, times(1)).read(oper, path);
        }
    }

    /**
     * Test case for {@link InventoryReader#collectUpEdgePorts(Set)} and
     * {@link InventoryReader#collectUpEdgePorts(Set, PortFilter)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCollectUpEdgePorts() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());

        Set<NodeConnector> expected = new HashSet<>();
        List<VtnNode> nodeList = new ArrayList<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // DPID 1: No port.
        long dpid = 1L;
        SalNode snode = new SalNode(dpid);
        InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
        VtnNode vnode = new VtnNodeBuilder().setId(snode.getNodeId()).build();
        nodeList.add(vnode);

        // DPID 2: All ports are down.
        List<VtnPort> portList = new ArrayList<>();
        dpid = 2L;
        snode = new SalNode(dpid);
        path = snode.getVtnNodeIdentifier();
        for (long port = 1L; port <= 10L; port++) {
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, Boolean.FALSE, true).
                build();
            portList.add(vport);
        }
        vnode = new VtnNodeBuilder().setId(snode.getNodeId()).
            setVtnPort(portList).build();
        nodeList.add(vnode);

        // DPID 3: Only one edge port is up.
        portList = new ArrayList<VtnPort>();
        dpid = 3L;
        snode = new SalNode(dpid);
        path = snode.getVtnNodeIdentifier();
        boolean edge = true;
        for (long port = 1L; port <= 10L; port++) {
            SalPort sport = new SalPort(dpid, port);
            Boolean en;
            if (port == 4L) {
                en = Boolean.TRUE;
                edge = true;
            } else {
                en = Boolean.FALSE;
            }
            VtnPort vport = createVtnPortBuilder(sport, en, edge).build();
            portList.add(vport);
            if (en.booleanValue()) {
                assertTrue(expected.add(sport.getAdNodeConnector()));
            }
            edge = !edge;
        }
        vnode = new VtnNodeBuilder().setId(snode.getNodeId()).
            setVtnPort(portList).build();
        nodeList.add(vnode);

        // DPID 4: No edge port.
        portList = new ArrayList<>();
        dpid = 4L;
        snode = new SalNode(dpid);
        path = snode.getVtnNodeIdentifier();
        for (long port = 1L; port <= 10L; port++) {
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, Boolean.TRUE, false).
                build();
            portList.add(vport);
        }
        vnode = new VtnNodeBuilder().setId(snode.getNodeId()).
            setVtnPort(portList).build();
        nodeList.add(vnode);

        // DPID 5: 10 edge ports are present.
        portList = new ArrayList<>();
        dpid = 4L;
        snode = new SalNode(dpid);
        path = snode.getVtnNodeIdentifier();
        for (long port = 1L; port <= 20L; port++) {
            edge = ((port & 1L) == 0L);
            SalPort sport = new SalPort(dpid, port);
            VtnPort vport = createVtnPortBuilder(sport, Boolean.TRUE, edge).
                build();
            portList.add(vport);
            if (edge) {
                assertTrue(expected.add(sport.getAdNodeConnector()));
            }
        }
        vnode = new VtnNodeBuilder().setId(snode.getNodeId()).
            setVtnPort(portList).build();
        nodeList.add(vnode);

        VtnNodes vnodes = new VtnNodesBuilder().setVtnNode(nodeList).build();
        InstanceIdentifier<VtnNodes> root = InstanceIdentifier.
            create(VtnNodes.class);
        when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

        for (int i = 0; i < 10; i++) {
            Set<NodeConnector> result = new HashSet<>();
            reader.collectUpEdgePorts(result);
            assertEquals(expected, result);

            for (NodeConnector nc: expected) {
                Set<NodeConnector> res = new HashSet<>();
                Set<NodeConnector> exp = new HashSet<>();
                exp.add(nc);

                PortFilter filter = new SpecificPortFilter(nc);
                reader.collectUpEdgePorts(res, filter);
                assertEquals(exp, res);
            }
        }

        verify(rtx, times(1)).read(oper, root);

        // Test with an empty VtnNodes.
        rtx = mock(ReadTransaction.class);
        reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        vnodes = null;
        when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

        for (int i = 0; i < 10; i++) {
            Set<NodeConnector> result = new HashSet<>();
            reader.collectUpEdgePorts(result);
            assertTrue(result.isEmpty());

            for (NodeConnector nc: expected) {
                PortFilter filter = new SpecificPortFilter(nc);
                reader.collectUpEdgePorts(result, filter);
                assertTrue(result.isEmpty());
            }
        }

        verify(rtx, times(1)).read(oper, root);
    }

    /**
     * Test case for {@link InventoryReader#getStaticLink(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetStaticLink() throws Exception {
        List<StaticSwitchLink> swlinks = new ArrayList<>();

        Collections.addAll(
            swlinks,
            // Valid inter-switch links.
            newStaticSwitchLink("openflow:1:1", "openflow:2:2"),
            newStaticSwitchLink("openflow:2:2", "openflow:1:1"),
            newStaticSwitchLink("openflow:1:2", "openflow:3:1"),
            newStaticSwitchLink("openflow:3:1", "openflow:1:2"),
            newStaticSwitchLink("openflow:2:1", "openflow:5:9"),
            newStaticSwitchLink("openflow:5:9", "openflow:2:1"),
            newStaticSwitchLink("openflow:10:12", "openflow:4:11"),
            newStaticSwitchLink("openflow:11:3", "openflow:9:5"),
            newStaticSwitchLink("openflow:12:34", "openflow:11:1"),

            // Invalid inter-switch links.
            newStaticSwitchLink("openflow:1:3", null),
            newStaticSwitchLink("openflow:1:234", null),
            newStaticSwitchLink("openflow:2:4", "openflow:2:4"),
            newStaticSwitchLink("openflow:3:111", "openflow:3:111"),
            newStaticSwitchLink("openflow:3:5", "unknown:10:1"),
            newStaticSwitchLink("openflow:11:44", "unknown:1:5"));

        // Static links to be ignored due to static edge configuration.
        List<StaticSwitchLink> srcEdgeLinks = new ArrayList<>();
        Collections.addAll(
            srcEdgeLinks,
            newStaticSwitchLink("openflow:40:41", "openflow:2:90"),
            newStaticSwitchLink("openflow:14:3", "openflow:11:9876"));

        List<StaticSwitchLink> dstEdgeLinks = new ArrayList<>();
        Collections.addAll(
            dstEdgeLinks,
            newStaticSwitchLink("openflow:39:41", "openflow:88:9999"),
            newStaticSwitchLink("openflow:1:123", "openflow:2:456"));

        SalPort[] edges = {
            new SalPort(1L, 4L),
            new SalPort(1L, 5L),
            new SalPort(2L, 3L),
            new SalPort(3L, 10L),
            new SalPort(5L, 1L),
            new SalPort(11L, 34L),
            new SalPort(12L, 999L),
        };

        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        StaticEdgePort nullEdge = null;

        for (StaticSwitchLink swlink: swlinks) {
            NodeConnectorId srcId = swlink.getSource();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            when(rtx.read(cstore, path)).thenReturn(getReadResult(swlink));

            SalPort sport = SalPort.create(swlink.getSource());
            if (sport != null) {
                NodeConnectorId ncId = sport.getNodeConnectorId();
                InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                    builder(VtnStaticTopology.class).
                    child(StaticEdgePorts.class).
                    child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                    build();
                when(rtx.read(cstore, epath)).
                    thenReturn(getReadResult(nullEdge));
            }

            sport = SalPort.create(swlink.getDestination());
            if (sport != null) {
                NodeConnectorId ncId = sport.getNodeConnectorId();
                InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                    builder(VtnStaticTopology.class).
                    child(StaticEdgePorts.class).
                    child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                    build();
                when(rtx.read(cstore, epath)).
                    thenReturn(getReadResult(nullEdge));
            }
        }
        for (StaticSwitchLink swlink: srcEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            when(rtx.read(cstore, path)).thenReturn(getReadResult(swlink));

            SalPort sport = SalPort.create(swlink.getSource());
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            StaticEdgePort ep = newStaticEdgePort(sport.toString());
            when(rtx.read(cstore, epath)).thenReturn(getReadResult(ep));

            sport = SalPort.create(swlink.getDestination());
            ncId = sport.getNodeConnectorId();
            epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            when(rtx.read(cstore, epath)).thenReturn(getReadResult(nullEdge));
        }
        for (StaticSwitchLink swlink: dstEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            when(rtx.read(cstore, path)).thenReturn(getReadResult(swlink));

            SalPort sport = SalPort.create(swlink.getSource());
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            when(rtx.read(cstore, epath)).thenReturn(getReadResult(nullEdge));

            sport = SalPort.create(swlink.getDestination());
            ncId = sport.getNodeConnectorId();
            epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            StaticEdgePort ep = newStaticEdgePort(sport.toString());
            when(rtx.read(cstore, epath)).thenReturn(getReadResult(ep));
        }

        for (SalPort sport: edges) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(ncId)).
                build();
            StaticSwitchLink swlink = null;
            when(rtx.read(cstore, path)).thenReturn(getReadResult(swlink));
        }

        for (StaticSwitchLink swlink: swlinks) {
            NodeConnectorId srcId = swlink.getSource();
            NodeConnectorId dstId = swlink.getDestination();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            SalPort src = SalPort.create(srcId);
            SalPort dst = SalPort.create(dstId);
            SalPort expected = (dst == null || src.equals(dst))
                ? null : dst;
            assertEquals(expected, reader.getStaticLink(src));
            verify(rtx).read(cstore, path);

            if (expected != null) {
                InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                    builder(VtnStaticTopology.class).
                    child(StaticEdgePorts.class).
                    child(StaticEdgePort.class, new StaticEdgePortKey(srcId)).
                    build();
                verify(rtx).read(cstore, epath);

                epath = InstanceIdentifier.
                    builder(VtnStaticTopology.class).
                    child(StaticEdgePorts.class).
                    child(StaticEdgePort.class, new StaticEdgePortKey(dstId)).
                    build();
                verify(rtx).read(cstore, epath);
            }
        }

        for (StaticSwitchLink swlink: srcEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            SalPort src = SalPort.create(srcId);
            assertEquals(null, reader.getStaticLink(src));
            verify(rtx).read(cstore, path);

            InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(srcId)).
                build();
            verify(rtx).read(cstore, epath);
        }

        for (StaticSwitchLink swlink: dstEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            NodeConnectorId dstId = swlink.getSource();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            SalPort src = SalPort.create(srcId);
            assertEquals(null, reader.getStaticLink(src));
            verify(rtx).read(cstore, path);

            InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(srcId)).
                build();
            verify(rtx).read(cstore, epath);

            epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(dstId)).
                build();
            verify(rtx).read(cstore, epath);
        }

        for (SalPort sport: edges) {
            NodeConnectorId srcId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            assertEquals(null, reader.getStaticLink(sport));
            verify(rtx).read(cstore, path);
        }

        // Results should be cached.
        reset(rtx);

        for (StaticSwitchLink swlink: swlinks) {
            NodeConnectorId srcId = swlink.getSource();
            NodeConnectorId dstId = swlink.getDestination();
            SalPort src = SalPort.create(srcId);
            SalPort dst = SalPort.create(dstId);
            SalPort expected = (dst == null || src.equals(dst))
                ? null : dst;
            assertEquals(expected, reader.getStaticLink(src));
        }

        for (StaticSwitchLink swlink: srcEdgeLinks) {
            SalPort src = SalPort.create(swlink.getSource());
            assertEquals(null, reader.getStaticLink(src));
        }

        for (StaticSwitchLink swlink: dstEdgeLinks) {
            SalPort src = SalPort.create(swlink.getSource());
            assertEquals(null, reader.getStaticLink(src));
        }

        for (SalPort sport: edges) {
            assertEquals(null, reader.getStaticLink(sport));
        }
        verifyZeroInteractions(rtx);

        // In case where all static links are cached.
        List<StaticSwitchLink> allLinks = new ArrayList<>();
        allLinks.addAll(swlinks);
        allLinks.addAll(srcEdgeLinks);
        allLinks.addAll(dstEdgeLinks);
        List<StaticEdgePort> allEdges = new ArrayList<>();
        for (StaticSwitchLink swlink: srcEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            allEdges.add(newStaticEdgePort(srcId.getValue()));
        }
        for (StaticSwitchLink swlink: dstEdgeLinks) {
            NodeConnectorId dstId = swlink.getDestination();
            allEdges.add(newStaticEdgePort(dstId.getValue()));
        }

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(new StaticSwitchLinksBuilder().
                                 setStaticSwitchLink(allLinks).build()).
            setStaticEdgePorts(new StaticEdgePortsBuilder().
                               setStaticEdgePort(allEdges).build()).
            build();
        InstanceIdentifier<VtnStaticTopology> rootPath =
            InstanceIdentifier.create(VtnStaticTopology.class);
        when(rtx.read(cstore, rootPath)).thenReturn(getReadResult(vstopo));

        reader = new InventoryReader(rtx);
        reader.getReverseStaticLinks(new SalPort(10L, 10L));
        verify(rtx).read(cstore, rootPath);
        verifyNoMoreInteractions(rtx);

        for (StaticSwitchLink swlink: swlinks) {
            NodeConnectorId srcId = swlink.getSource();
            NodeConnectorId dstId = swlink.getDestination();
            SalPort src = SalPort.create(srcId);
            SalPort dst = SalPort.create(dstId);
            SalPort expected = (dst == null || src.equals(dst))
                ? null : dst;
            assertEquals(expected, reader.getStaticLink(src));
        }

        for (StaticSwitchLink swlink: srcEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            SalPort src = SalPort.create(srcId);
            assertEquals(null, reader.getStaticLink(src));
        }

        for (StaticSwitchLink swlink: dstEdgeLinks) {
            NodeConnectorId srcId = swlink.getSource();
            SalPort src = SalPort.create(srcId);
            assertEquals(null, reader.getStaticLink(src));
        }

        for (SalPort sport: edges) {
            assertEquals(null, reader.getStaticLink(sport));
        }

        // In case where no static link is configured.
        StaticSwitchLink nullLink = null;
        reader = new InventoryReader(rtx);
        for (StaticSwitchLink swlink: allLinks) {
            NodeConnectorId srcId = swlink.getSource();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            when(rtx.read(cstore, path)).thenReturn(getReadResult(nullLink));
            SalPort src = SalPort.create(srcId);
            assertEquals(null, reader.getStaticLink(src));
            verify(rtx).read(cstore, path);
        }
        for (SalPort sport: edges) {
            NodeConnectorId srcId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticSwitchLinks.class).
                child(StaticSwitchLink.class, new StaticSwitchLinkKey(srcId)).
                build();
            when(rtx.read(cstore, path)).thenReturn(getReadResult(nullLink));
            assertEquals(null, reader.getStaticLink(sport));
            verify(rtx).read(cstore, path);
        }
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vstopo = null;
        when(rtx.read(cstore, rootPath)).thenReturn(getReadResult(vstopo));
        reader = new InventoryReader(rtx);
        reader.getReverseStaticLinks(new SalPort(10L, 10L));

        for (StaticSwitchLink swlink: allLinks) {
            NodeConnectorId srcId = swlink.getSource();
            SalPort src = SalPort.create(srcId);
            assertEquals(null, reader.getStaticLink(src));
        }

        for (SalPort sport: edges) {
            assertEquals(null, reader.getStaticLink(sport));
        }

        verify(rtx).read(cstore, rootPath);
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link InventoryReader#getReverseStaticLinks(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetReverseStaticLinks() throws Exception {
        List<StaticSwitchLink> swlinks = new ArrayList<>();
        Map<SalPort, Set<SalPort>> revMap = new HashMap<>();
        Set<SalPort> edges = new HashSet<>();
        Collections.addAll(
            edges,
            SalPort.create("openflow:15:16"),
            SalPort.create("openflow:1:15"),
            SalPort.create("openflow:10:2"));

        ReadTransaction rtx = mock(ReadTransaction.class);
        String nullStr = null;
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:1:1",
                             "openflow:2:1");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:1:2",
                             "openflow:2:2", "openflow:3:1");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:15:16",
                             "openflow:23:45", "openflow:3:999");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:10:123",
                             "openflow:5:9", "openflow:2:25", "openflow:10:2");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:123:45",
                             "openflow:123:45", "openflow:3:2", "openflow:4:1",
                             "openflow:1:15", "unknown:1:1", "openflow:13:4");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "bad:1:3",
                             "openflow:12:99", "openflow:3:19", "openflow:8:3",
                             "openflow:1:16", "unknown:1:34", "openflow:13:8");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, nullStr,
                             "openflow:12:88", "openflow:7:4", "openflow:6:12",
                             "openflow:1:7", "bad:15:67");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:99:88",
                             "openflow:12:32", "openflow:99:88", "bad:11:22",
                             nullStr, "openflow:99:87");

        SalPort[] notFound = {
            SalPort.create("openflow:1:19"),
            SalPort.create("openflow:2:33"),
            SalPort.create("openflow:4:100"),
            SalPort.create("openflow:12:3456"),
        };

        List<StaticEdgePort> allEdges = new ArrayList<>();
        for (SalPort sport: edges) {
            allEdges.add(newStaticEdgePort(sport.toString()));
        }

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(new StaticSwitchLinksBuilder().
                                 setStaticSwitchLink(swlinks).build()).
            setStaticEdgePorts(new StaticEdgePortsBuilder().
                               setStaticEdgePort(allEdges).build()).
            build();
        InstanceIdentifier<VtnStaticTopology> path =
            InstanceIdentifier.create(VtnStaticTopology.class);
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        when(rtx.read(cstore, path)).thenReturn(getReadResult(vstopo));
        InventoryReader reader = new InventoryReader(rtx);
        boolean first = true;
        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            if (dst == null) {
                continue;
            }

            Set<SalPort> expected = revMap.get(dst);
            if (expected == null) {
                expected = Collections.<SalPort>emptySet();
            }
            assertEquals(expected, reader.getReverseStaticLinks(dst));
            if (first) {
                // All static links should be cached.
                verify(rtx).read(cstore, path);
                verifyNoMoreInteractions(rtx);
                reset(rtx);
                first = false;
            }
        }

        assertEquals(revMap,
                     getFieldValue(reader, Map.class, "reverseStaticLinks"));

        Set<SalPort> expected = Collections.<SalPort>emptySet();
        for (SalPort sport: notFound) {
            assertEquals(expected, reader.getReverseStaticLinks(sport));
        }

        // In case where no static link configuration is present.
        vstopo = null;
        when(rtx.read(cstore, path)).thenReturn(getReadResult(vstopo));
        reader = new InventoryReader(rtx);
        for (StaticSwitchLink swlink: swlinks) {
            SalPort dst = SalPort.create(swlink.getDestination());
            if (dst != null) {
                assertEquals(expected, reader.getReverseStaticLinks(dst));
            }
        }
        for (SalPort sport: notFound) {
            assertEquals(expected, reader.getReverseStaticLinks(sport));
        }
        verify(rtx).read(cstore, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        StaticSwitchLinks swlctr = new StaticSwitchLinksBuilder().
            setStaticSwitchLink(Collections.<StaticSwitchLink>emptyList()).
            build();
        vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlctr).
            build();
        when(rtx.read(cstore, path)).thenReturn(getReadResult(vstopo));
        reader = new InventoryReader(rtx);
        for (StaticSwitchLink swlink: swlinks) {
            SalPort dst = SalPort.create(swlink.getDestination());
            if (dst != null) {
                assertEquals(expected, reader.getReverseStaticLinks(dst));
            }
        }
        for (SalPort sport: notFound) {
            assertEquals(expected, reader.getReverseStaticLinks(sport));
        }
        verify(rtx).read(cstore, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        swlctr = new StaticSwitchLinksBuilder().build();
        vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlctr).
            build();
        when(rtx.read(cstore, path)).thenReturn(getReadResult(vstopo));
        reader = new InventoryReader(rtx);
        for (StaticSwitchLink swlink: swlinks) {
            SalPort dst = SalPort.create(swlink.getDestination());
            if (dst != null) {
                assertEquals(expected, reader.getReverseStaticLinks(dst));
            }
        }
        for (SalPort sport: notFound) {
            assertEquals(expected, reader.getReverseStaticLinks(sport));
        }
        verify(rtx).read(cstore, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);
    }

    /**
     * Test case for {@link InventoryReader#isStaticEdgePort(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIsStaticEdgePort() throws Exception {
        List<StaticEdgePort> edgePorts = new ArrayList<>();
        SalPort[] edges = {
            new SalPort(1L, 2L),
            new SalPort(1L, 3L),
            new SalPort(2L, 2L),
            new SalPort(2L, 3L),
            new SalPort(10L, 20L),
            new SalPort(123L, 456L),
        };
        SalPort[] nonEdges = {
            new SalPort(1L, 1L),
            new SalPort(1L, 10L),
            new SalPort(2L, 1L),
            new SalPort(2L, 10L),
            new SalPort(10L, 21L),
            new SalPort(123L, 999L),
        };

        ReadTransaction rtx = mock(ReadTransaction.class);
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        for (SalPort sport: edges) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            StaticEdgePort ep = newStaticEdgePort(sport.toString());
            edgePorts.add(ep);
            when(rtx.read(cstore, path)).thenReturn(getReadResult(ep));
        }

        for (SalPort sport: nonEdges) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            StaticEdgePort ep = null;
            when(rtx.read(cstore, path)).thenReturn(getReadResult(ep));
        }

        InventoryReader reader = new InventoryReader(rtx);
        for (SalPort sport: edges) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            assertEquals(true, reader.isStaticEdgePort(sport));
            verify(rtx).read(cstore, path);
        }

        for (SalPort sport: nonEdges) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            InstanceIdentifier<StaticEdgePort> path = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            assertEquals(false, reader.isStaticEdgePort(sport));
            verify(rtx).read(cstore, path);
        }

        // Results should be cached.
        reset(rtx);
        for (SalPort sport: edges) {
            assertEquals(true, reader.isStaticEdgePort(sport));
        }

        for (SalPort sport: nonEdges) {
            assertEquals(false, reader.isStaticEdgePort(sport));
        }
        verifyZeroInteractions(rtx);
    }

    /**
     * Test case for {@link InventoryReader#prefetch(VtnStaticTopology)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPrefetchVtnStaticTopology() throws Exception {
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;

        assertEquals(null, getFieldValue(reader, Map.class,
                                         "reverseStaticLinks"));

        SalPort[] notFound = {
            SalPort.create("openflow:1:19"),
            SalPort.create("openflow:2:33"),
            SalPort.create("openflow:4:100"),
            SalPort.create("openflow:12:3456"),
        };

        // Cache some configurations.
        Map<SalPort, Set<SalPort>> revMap = new HashMap<>();
        StaticEdgePort nullEdge = null;
        List<StaticSwitchLink> swlinks = new ArrayList<>();
        Collections.addAll(
            swlinks,
            newStaticSwitchLink("openflow:100:100", "openflow:200:200"),
            newStaticSwitchLink("openflow:300:300", "openflow:400:400"));
        List<StaticEdgePort> allEdges = new ArrayList<>();
        Collections.addAll(
            allEdges,
            newStaticEdgePort("openflow:12345:12345"),
            newStaticEdgePort("openflow:23456:23456"));

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(new StaticSwitchLinksBuilder().
                                 setStaticSwitchLink(swlinks).build()).
            setStaticEdgePorts(new StaticEdgePortsBuilder().
                               setStaticEdgePort(allEdges).build()).
            build();
        InstanceIdentifier<VtnStaticTopology> rootPath =
            InstanceIdentifier.create(VtnStaticTopology.class);
        when(rtx.read(cstore, rootPath)).thenReturn(getReadResult(vstopo));

        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            Set<SalPort> srcSet = Collections.singleton(src);
            revMap.put(dst, srcSet);
            assertEquals(srcSet, reader.getReverseStaticLinks(dst));
            assertEquals(dst, reader.getStaticLink(src));
        }

        for (StaticEdgePort ep: allEdges) {
            SalPort sport = SalPort.create(ep.getPort());
            assertEquals(true, reader.isStaticEdgePort(sport));
        }

        Set<SalPort> emptyPort = Collections.<SalPort>emptySet();
        for (SalPort sport: notFound) {
            assertEquals(false, reader.isStaticEdgePort(sport));
            assertEquals(null, reader.getStaticLink(sport));
            assertEquals(emptyPort, reader.getReverseStaticLinks(sport));
        }

        verify(rtx).read(cstore, rootPath);
        verifyNoMoreInteractions(rtx);

        assertEquals(revMap, getFieldValue(reader, Map.class,
                                           "reverseStaticLinks"));

        // Update the static topology cache with null topology.
        reset(rtx);
        vstopo = null;
        revMap = new HashMap<>();
        reader.prefetch(vstopo);
        verifyZeroInteractions(rtx);
        assertEquals(revMap, getFieldValue(reader, Map.class,
                                           "reverseStaticLinks"));

        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            assertEquals(emptyPort, reader.getReverseStaticLinks(dst));
            assertEquals(null, reader.getStaticLink(src));
        }

        for (StaticEdgePort ep: allEdges) {
            SalPort sport = SalPort.create(ep.getPort());
            assertEquals(false, reader.isStaticEdgePort(sport));
        }

        for (SalPort sport: notFound) {
            assertEquals(false, reader.isStaticEdgePort(sport));
            assertEquals(null, reader.getStaticLink(sport));
            assertEquals(emptyPort, reader.getReverseStaticLinks(sport));
        }

        // Update the static topology cache with an empty topology.
        reset(rtx);
        vstopo = new VtnStaticTopologyBuilder().build();
        reader.prefetch(vstopo);
        verifyZeroInteractions(rtx);
        assertEquals(revMap, getFieldValue(reader, Map.class,
                                           "reverseStaticLinks"));

        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            assertEquals(emptyPort, reader.getReverseStaticLinks(dst));
            assertEquals(null, reader.getStaticLink(src));
        }

        for (StaticEdgePort ep: allEdges) {
            SalPort sport = SalPort.create(ep.getPort());
            assertEquals(false, reader.isStaticEdgePort(sport));
        }

        for (SalPort sport: notFound) {
            assertEquals(false, reader.isStaticEdgePort(sport));
            assertEquals(null, reader.getStaticLink(sport));
            assertEquals(emptyPort, reader.getReverseStaticLinks(sport));
        }

        // Update the static topology cache with another configuration.
        String nullStr = null;
        Set<SalPort> edges = new HashSet<>();
        Collections.addAll(
            edges,
            SalPort.create("openflow:15:16"),
            SalPort.create("openflow:1:15"),
            SalPort.create("openflow:10:2"),
            SalPort.create("openflow:99999:99999"));

        swlinks.clear();
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:1:1",
                             "openflow:2:1");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:1:2",
                             "openflow:2:2", "openflow:3:1");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:15:16",
                             "openflow:23:45", "openflow:3:999");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:10:123",
                             "openflow:5:9", "openflow:2:25", "openflow:10:2");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:123:45",
                             "openflow:123:45", "openflow:3:2", "openflow:4:1",
                             "openflow:1:15", "unknown:1:1", "openflow:13:4");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "bad:1:3",
                             "openflow:12:99", "openflow:3:19", "openflow:8:3",
                             "openflow:1:16", "unknown:1:34", "openflow:13:8");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, nullStr,
                             "openflow:12:88", "openflow:7:4", "openflow:6:12",
                             "openflow:1:7", "bad:15:67");
        setStaticSwitchLinks(rtx, swlinks, revMap, edges, "openflow:99:88",
                             "openflow:12:32", "openflow:99:88", "bad:11:22",
                             nullStr, "openflow:99:87");

        allEdges.clear();
        for (SalPort sport: edges) {
            allEdges.add(newStaticEdgePort(sport.toString()));
        }

        vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(new StaticSwitchLinksBuilder().
                                 setStaticSwitchLink(swlinks).build()).
            setStaticEdgePorts(new StaticEdgePortsBuilder().
                               setStaticEdgePort(allEdges).build()).
            build();
        reader.prefetch(vstopo);
        verifyZeroInteractions(rtx);
        assertEquals(revMap, getFieldValue(reader, Map.class,
                                           "reverseStaticLinks"));

        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            boolean de = edges.contains(dst);
            if (src != null) {
                boolean se = edges.contains(src);
                assertEquals(se, reader.isStaticEdgePort(src));
                if (dst == null || dst.equals(src) || se || de) {
                    assertEquals(null, reader.getStaticLink(src));
                } else {
                    assertEquals(dst, reader.getStaticLink(src));
                }
            }

            if (dst != null) {
                assertEquals(de, reader.isStaticEdgePort(dst));

                Set<SalPort> expected = revMap.get(dst);
                if (expected == null) {
                    expected = Collections.<SalPort>emptySet();
                }
                assertEquals(expected, reader.getReverseStaticLinks(dst));
            }
        }

        for (SalPort sport: edges) {
            assertEquals(true, reader.isStaticEdgePort(sport));
        }
        for (SalPort sport: notFound) {
            assertEquals(false, reader.isStaticEdgePort(sport));
            assertEquals(null, reader.getStaticLink(sport));
            assertEquals(emptyPort, reader.getReverseStaticLinks(sport));
        }

        // Update the static topology cache with empty static link list and
        // empty edge port list.
        StaticSwitchLinks swlctr = new StaticSwitchLinksBuilder().
            setStaticSwitchLink(new ArrayList<StaticSwitchLink>()).
            build();
        StaticEdgePorts edgectr = new StaticEdgePortsBuilder().
            setStaticEdgePort(new ArrayList<StaticEdgePort>()).
            build();
        reset(rtx);
        vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlctr).
            setStaticEdgePorts(edgectr).
            build();
        revMap = new HashMap<>();
        reader.prefetch(vstopo);
        verifyZeroInteractions(rtx);
        assertEquals(revMap, getFieldValue(reader, Map.class,
                                           "reverseStaticLinks"));

        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            assertEquals(emptyPort, reader.getReverseStaticLinks(dst));
            assertEquals(null, reader.getStaticLink(src));
        }

        for (SalPort sport: edges) {
            assertEquals(false, reader.isStaticEdgePort(sport));
        }

        for (SalPort sport: notFound) {
            assertEquals(false, reader.isStaticEdgePort(sport));
            assertEquals(null, reader.getStaticLink(sport));
            assertEquals(emptyPort, reader.getReverseStaticLinks(sport));
        }

        swlctr = new StaticSwitchLinksBuilder().build();
        edgectr = new StaticEdgePortsBuilder().build();
        reset(rtx);
        vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlctr).
            setStaticEdgePorts(edgectr).
            build();
        revMap = new HashMap<>();
        reader.prefetch(vstopo);
        verifyZeroInteractions(rtx);
        assertEquals(revMap, getFieldValue(reader, Map.class,
                                           "reverseStaticLinks"));

        for (StaticSwitchLink swlink: swlinks) {
            SalPort src = SalPort.create(swlink.getSource());
            SalPort dst = SalPort.create(swlink.getDestination());
            assertEquals(emptyPort, reader.getReverseStaticLinks(dst));
            assertEquals(null, reader.getStaticLink(src));
        }

        for (SalPort sport: edges) {
            assertEquals(false, reader.isStaticEdgePort(sport));
        }

        for (SalPort sport: notFound) {
            assertEquals(false, reader.isStaticEdgePort(sport));
            assertEquals(null, reader.getStaticLink(sport));
            assertEquals(emptyPort, reader.getReverseStaticLinks(sport));
        }
    }

    /**
     * Test case for {@link InventoryReader#purge(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPurgePort() throws Exception {
        ReadTransaction rtx = mock(ReadTransaction.class);
        InventoryReader reader = new InventoryReader(rtx);

        Map<SalPort, VtnPort> expected = new HashMap<>();
        Map<SalPort, VtnPort> expectedPurged = new HashMap<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            for (long port = 1L; port <= 10L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort edge = createVtnPortBuilder(sport).build();
                VtnPort isl = createVtnPortBuilder(sport, true, false).build();
                when(rtx.read(oper, path)).thenReturn(getReadResult(edge)).
                    thenReturn(getReadResult(isl));
                assertNull(expected.put(sport, edge));
                assertNull(expectedPurged.put(sport, isl));
            }

            for (long port = 20L; port <= 30L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = null;
                VtnPort vportNew = createVtnPortBuilder(sport, false, true).
                    build();
                when(rtx.read(oper, path)).thenReturn(getReadResult(vport)).
                    thenReturn(getReadResult(vportNew));
                assertNull(expected.put(sport, vport));
                assertNull(expectedPurged.put(sport, vportNew));
            }

            for (long port = 40L; port <= 50L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = createVtnPortBuilder(sport).build();
                VtnPort vportNew = null;
                when(rtx.read(oper, path)).thenReturn(getReadResult(vport)).
                    thenReturn(getReadResult(vportNew));
                assertNull(expected.put(sport, vport));
                assertNull(expectedPurged.put(sport, vportNew));
            }
        }

        // Read port information.
        for (Map.Entry<SalPort, VtnPort> entry: expected.entrySet()) {
            SalPort sport = entry.getKey();
            VtnPort vport = entry.getValue();

            // Result should be cached.
            for (int i = 0; i < 5; i++) {
                assertEquals(vport, reader.get(sport));
            }
        }
        assertEquals(expected, reader.getCachedPorts());

        List<SalPort> ports = new ArrayList<>(expected.keySet());
        for (SalPort sport: ports) {
            // Purge cache for this port.
            reader.purge(sport);
            expected.remove(sport);
            assertEquals(expected, reader.getCachedPorts());

            // New result should be cached.
            VtnPort newPort = expectedPurged.get(sport);
            for (int i = 0; i < 5; i++) {
                assertEquals(newPort, reader.get(sport));
            }

            expected.put(sport, newPort);
            assertEquals(expected, reader.getCachedPorts());
        }
    }

    /**
     * Create a static inter-switch link configuration.
     *
     * @param src  The source port of the link.
     * @param dst  The destination port of the link.
     * @return  A {@link StaticSwitchLink} instance.
     */
    private StaticSwitchLink newStaticSwitchLink(String src, String dst) {
        StaticSwitchLinkBuilder builder = new StaticSwitchLinkBuilder();
        if (src != null) {
            builder.setSource(new NodeConnectorId(src));
        }
        if (dst != null) {
            builder.setDestination(new NodeConnectorId(dst));
        }

        return builder.build();
    }

    /**
     * Create a static inter-switch link configuration for
     * {@link InventoryReader#getReverseStaticLinks(SalPort)} test.
     *
     * @param rtx      A mock-up of {@link ReadTransaction}.
     * @param swlinks  A list to store {@link StaticSwitchLink} instances.
     * @param revMap   A map to store inter-switch links indexed by
     *                 destination port
     * @param edges    A set that specifies static edge ports.
     * @param dst      The destination port of the link.
     * @param srcs     An array of source ports.
     */
    private void setStaticSwitchLinks(ReadTransaction rtx,
                                      List<StaticSwitchLink> swlinks,
                                      Map<SalPort, Set<SalPort>> revMap,
                                      Set<SalPort> edges,
                                      String dst, String ... srcs) {
        Set<SalPort> srcSet = new HashSet<>();
        SalPort dport = SalPort.create(dst);
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;

        StaticEdgePort dstEp = null;
        if (dport != null) {
            NodeConnectorId ncId = new NodeConnectorId(dst);
            InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                builder(VtnStaticTopology.class).
                child(StaticEdgePorts.class).
                child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                build();
            if (edges.contains(dport)) {
                dstEp = newStaticEdgePort(dst);
            }
            when(rtx.read(cstore, epath)).thenReturn(getReadResult(dstEp));
        }

        for (String src: srcs) {
            swlinks.add(newStaticSwitchLink(src, dst));

            SalPort sport = SalPort.create(src);
            if (sport != null) {
                NodeConnectorId ncId = new NodeConnectorId(src);
                InstanceIdentifier<StaticEdgePort> epath = InstanceIdentifier.
                    builder(VtnStaticTopology.class).
                    child(StaticEdgePorts.class).
                    child(StaticEdgePort.class, new StaticEdgePortKey(ncId)).
                    build();
                StaticEdgePort srcEp = (edges.contains(sport))
                    ? newStaticEdgePort(src)
                    : null;
                when(rtx.read(cstore, epath)).thenReturn(getReadResult(srcEp));

                if (dport != null && !sport.equals(dport) && srcEp == null &&
                    dstEp == null) {
                    srcSet.add(sport);
                }
            }
        }

        if (!srcSet.isEmpty()) {
            revMap.put(dport, srcSet);
        }
    }

    /**
     * Create a static edge port configuration.
     *
     * @param port  The port identifier.
     * @return  A {@link StaticEdgePort} instance.
     */
    private StaticEdgePort newStaticEdgePort(String port) {
        NodeConnectorId ncId = new NodeConnectorId(port);
        return new StaticEdgePortBuilder().setPort(ncId).build();
    }
}
