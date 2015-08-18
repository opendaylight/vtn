/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.mockito.Mockito;

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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
                Mockito.when(rtx.read(oper, path)).
                    thenReturn(getReadResult(vport));
            }

            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).setVtnPort(portList).build();
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
            assertNull(expected.put(snode, vnode));
        }

        for (long dpid = 100L; dpid <= 110L; dpid++) {
            SalNode snode = new SalNode(dpid);
            negative.add(snode);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = null;
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
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
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);

            for (VtnPort vport: vnode.getVtnPort()) {
                SalPort sport = SalPort.create(vport.getId());
                assertNotNull(sport);
                InstanceIdentifier<VtnPort> ppath =
                    sport.getVtnPortIdentifier();
                Mockito.verify(rtx, Mockito.never()).read(oper, ppath);
            }
        }
        for (SalNode snode: negative) {
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
                Mockito.when(rtx.read(oper, path)).
                    thenReturn(getReadResult(vport));
                assertNull(expected.put(sport, vport));
            }

            for (long port = 20L; port <= 30L; port++) {
                SalPort sport = new SalPort(dpid, port);
                InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
                VtnPort vport = null;
                Mockito.when(rtx.read(oper, path)).
                    thenReturn(getReadResult(vport));
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
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        }
        for (SalPort sport: negative) {
            InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
        Mockito.when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

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

        Mockito.verify(rtx, Mockito.times(1)).read(oper, root);

        for (Map.Entry<SalNode, VtnNode> entry: expectedNodes.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNode vnode = entry.getValue();
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            Mockito.verify(rtx, Mockito.never()).read(oper, path);

            for (VtnPort vport: vnode.getVtnPort()) {
                SalPort sport = SalPort.create(vport.getId());
                assertNotNull(sport);
                InstanceIdentifier<VtnPort> ppath =
                    sport.getVtnPortIdentifier();
                Mockito.verify(rtx, Mockito.never()).read(oper, ppath);
            }
        }

        assertEquals(expectedNodes, reader.getCachedNodes());
        assertEquals(expectedPorts, reader.getCachedPorts());

        // Test with an empty VtnNodes.
        rtx = Mockito.mock(ReadTransaction.class);
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
        Mockito.when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

        for (int i = 0; i < 10; i++) {
            assertTrue(reader.getVtnNodes().isEmpty());
        }

        Mockito.verify(rtx, Mockito.times(1)).read(oper, root);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
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
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
                Mockito.when(rtx.read(oper, path)).
                    thenReturn(getReadResult(vport));
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
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
            assertNull(nodeMap.put(snode, vnode));
        }

        Set<SalNode> notPresent = new HashSet<>();
        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = null;
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
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
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        }

        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
            assertNull(nodeMap.put(snode, vnode));
        }

        Set<SalNode> notPresent = new HashSet<>();
        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            VtnNode vnode = null;
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vnode));
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
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        }

        for (long dpid = 10L; dpid <= 15L; dpid++) {
            SalNode snode = new SalNode(dpid);
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
        Mockito.when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

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

        Mockito.verify(rtx, Mockito.times(1)).read(oper, root);

        // Test with an empty VtnNodes.
        rtx = Mockito.mock(ReadTransaction.class);
        reader = new InventoryReader(rtx);
        assertSame(rtx, reader.getReadTransaction());
        vnodes = null;
        Mockito.when(rtx.read(oper, root)).thenReturn(getReadResult(vnodes));

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

        Mockito.verify(rtx, Mockito.times(1)).read(oper, root);
    }
}
