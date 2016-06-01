/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_NOT_FOUND;
import static java.net.HttpURLConnection.HTTP_OK;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfigTest.DEFAULT_BRIDGE_NAME;
import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfigTest.DEFAULT_PORT_NAME;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.google.common.base.Optional;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.PortBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntryBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbFailModeSecure;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeProtocolOpenflow13;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ConnectionInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ConnectionInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntryBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Address;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;

/**
 * JUnit test for {@link OVSDBEventHandler}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({OVSDBEventHandler.class, VTNManagerService.class,
                 MdsalUtils.class})
public final class OVSDBEventHandlerTest extends TestBase {
    /**
     * Mock-up of {@link MdsalUtils}.
     */
    private MdsalUtils utils;

    /**
     * VTNManagerService instance.
     */
    private VTNManagerService service;

    /**
     * An {@link OVSDBEventHandler} instance for test.
     */
    private OVSDBEventHandler handler;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        utils = PowerMockito.mock(MdsalUtils.class);
        service = PowerMockito.mock(VTNManagerService.class);
        NeutronConfig cfg = new NeutronConfig();
        handler = PowerMockito.spy(new OVSDBEventHandler(cfg, utils, service));
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeAdded(Node,OvsdbNodeAugmentation)}.
     *
     * <ul>
     *   <li>No node key.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNodeAdded1() throws Exception {
        Node node = mock(Node.class);
        OvsdbNodeAugmentation ovnode = mock(OvsdbNodeAugmentation.class);
        handler.nodeAdded(node, ovnode);
        verify(node).getKey();
        verifyNoMoreInteractions(node, ovnode, utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeAdded(Node,OvsdbNodeAugmentation)}.
     *
     * <ul>
     *   <li>Bridge node is already present.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNodeAdded2() throws Exception {
        String bridgeStr = "ovsdb:node1/bridge/" + DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bridgeRef = newNodePath(bridgeStr);
        ManagedNodeEntry mnent = new ManagedNodeEntryBuilder().
            setBridgeRef(new OvsdbBridgeRef(bridgeRef)).
            build();
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            setManagedNodeEntry(Collections.singletonList(mnent)).
            build();
        NodeId nodeId = new NodeId("ovsdb:node1");
        Node node = new NodeBuilder().
            setNodeId(nodeId).
            build();
        handler.nodeAdded(node, ovnode);
        verifyZeroInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeAdded(Node,OvsdbNodeAugmentation)}.
     *
     * <ul>
     *   <li>No connection info.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNodeAdded3() throws Exception {
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        NodeId nodeId = new NodeId("ovsdb:node1");
        Node node = new NodeBuilder().
            setNodeId(nodeId).
            build();
        handler.nodeAdded(node, ovnode);
        verifyZeroInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeRemoved(Node)}.
     */
    @Test
    public void testNodeRemoved() {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        for (String nid: nodeIds) {
            boolean[] bools = {true, false};
            NodeId nodeId = new NodeId(nid);
            Node bnode = new NodeBuilder().
                setNodeId(nodeId).
                build();
            String mngNodeIdStr = nid + "/bridge/" + DEFAULT_BRIDGE_NAME;
            InstanceIdentifier<Node> path = newNodePath(mngNodeIdStr);
            LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
            for (boolean result: bools) {
                when(utils.delete(store, path)).thenReturn(result);
                handler.nodeRemoved(bnode);
                verify(utils).delete(store, path);
                verifyNoMoreInteractions(utils, service);
                reset(utils);
            }
        }
    }

    /**
     * Test case for {@link OVSDBEventHandler#deleteBridge(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeleteBridge() throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        for (String nid: nodeIds) {
            boolean[] bools = {true, false};
            NodeId nodeId = new NodeId(nid);
            Node bnode = new NodeBuilder().
                setNodeId(nodeId).
                build();
            String mngNodeIdStr = nid + "/bridge/" + DEFAULT_BRIDGE_NAME;
            InstanceIdentifier<Node> path = newNodePath(mngNodeIdStr);
            LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
            for (boolean result: bools) {
                when(utils.delete(store, path)).thenReturn(result);
                assertEquals(result, Whitebox.invokeMethod(
                                 handler, "deleteBridge", bnode,
                                 DEFAULT_BRIDGE_NAME));
                verify(utils).delete(store, path);
                verifyNoMoreInteractions(utils, service);
                reset(utils);
            }
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#extractTerminationPointAugmentation(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExtractTerminationPointAugmentation() throws Exception {
        // In case where no bridge augmentation is present.
        NodeId nodeId = new NodeId("ovsdb:node-1");
        Node node = new NodeBuilder().
            setNodeId(nodeId).
            build();
        assertNull(Whitebox.invokeMethod(
                       handler, "extractTerminationPointAugmentation",
                       node, DEFAULT_PORT_NAME));

        // In case where the specified port is not found.
        OvsdbBridgeAugmentation ovbr =
            new OvsdbBridgeAugmentationBuilder().
            build();
        List<TerminationPoint> list = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            OvsdbTerminationPointAugmentation ovtp =
                new OvsdbTerminationPointAugmentationBuilder().
                setName("ovsdb-tp:" + i).
                build();
            TerminationPoint tp = new TerminationPointBuilder().
                setTpId(new TpId("tp" + i + "-" + i)).
                addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
                build();
            list.add(tp);
        }
        node = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(list).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertNull(Whitebox.invokeMethod(
                         handler, "extractTerminationPointAugmentation",
                         node, DEFAULT_PORT_NAME));

        // In case where the specified port is found.
        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().
            setName(DEFAULT_PORT_NAME).
            build();
        TerminationPoint tp = new TerminationPointBuilder().
            setTpId(new TpId("tp")).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        list.add(tp);
        node = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(list).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        assertEquals(ovtp, Whitebox.invokeMethod(
                         handler, "extractTerminationPointAugmentation",
                         node, DEFAULT_PORT_NAME));
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#extractTerminationPointAugmentations(Node)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExtractTerminationPointAugmentations() throws Exception {
        // In case where termination point list is null.
        NodeId nodeId = new NodeId("ovsdb:node-1");
        Node node = new NodeBuilder().
            setNodeId(nodeId).
            build();
        List<OvsdbTerminationPointAugmentation> expected =
            Collections.<OvsdbTerminationPointAugmentation>emptyList();
        assertEquals(expected, Whitebox.invokeMethod(
                         handler, "extractTerminationPointAugmentations",
                         node));

        // In case where termination point list is empty.
        List<TerminationPoint> list = new ArrayList<>();
        node = new NodeBuilder().
            setNodeId(nodeId).
            setTerminationPoint(list).
            build();
        assertEquals(expected, Whitebox.invokeMethod(
                         handler, "extractTerminationPointAugmentations",
                         node));

        // In case where termination point augmentation is present.
        expected = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            TerminationPoint tp = new TerminationPointBuilder().
                setTpId(new TpId("tp" + i)).
                build();
            list.add(tp);

            OvsdbTerminationPointAugmentation ovtp =
                new OvsdbTerminationPointAugmentationBuilder().
                setName("ovsdb-tp:" + i).
                build();
            tp = new TerminationPointBuilder().
                setTpId(new TpId("tp" + i + "-" + i)).
                addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
                build();
            list.add(tp);
            expected.add(ovtp);
        }
        assertEquals(expected, Whitebox.invokeMethod(
                         handler, "extractTerminationPointAugmentations",
                         node));
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addPortToBridge(NodeKey,String,String)}.
     *
     * <ul>
     *   <li>Successful completion.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddPortToBridge1() throws Exception {
        NodeId nodeId = new NodeId("ovsdb:node-1");
        NodeKey nodeKey = new NodeKey(nodeId);
        String mngNodeIdStr = nodeId.getValue() + "/bridge/" +
            DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bnodePath = newNodePath(mngNodeIdStr);
        Node bnode = new NodeBuilder().
            setNodeId(new NodeId(mngNodeIdStr)).
            build();
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        when(utils.read(store, bnodePath)).thenReturn(Optional.of(bnode));

        TerminationPointKey tpKey =
            new TerminationPointKey(new TpId(DEFAULT_PORT_NAME));
        InstanceIdentifier<TerminationPoint> tpPath = bnodePath.
            child(TerminationPoint.class, tpKey);
        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().
            setName(DEFAULT_PORT_NAME).
            build();
        TerminationPoint btp = new TerminationPointBuilder().
            setKey(tpKey).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        when(utils.put(store, tpPath, btp)).thenReturn(true);

        assertEquals(true, Whitebox.invokeMethod(
                         handler, "addPortToBridge", nodeKey,
                         DEFAULT_BRIDGE_NAME, DEFAULT_PORT_NAME));
        verify(utils).read(store, bnodePath);
        verify(utils).put(store, tpPath, btp);
        verifyNoMoreInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addPortToBridge(NodeKey,String,String)}.
     *
     * <ul>
     *   <li>No bridge node.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddPortToBridge2() throws Exception {
        NodeId nodeId = new NodeId("ovsdb:node-1");
        NodeKey nodeKey = new NodeKey(nodeId);
        String mngNodeIdStr = nodeId.getValue() + "/bridge/" +
            DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bnodePath = newNodePath(mngNodeIdStr);
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        when(utils.read(store, bnodePath)).thenReturn(Optional.<Node>absent());
        assertEquals(false, Whitebox.invokeMethod(
                         handler, "addPortToBridge", nodeKey,
                         DEFAULT_BRIDGE_NAME, DEFAULT_PORT_NAME));
        verify(utils).read(store, bnodePath);
        verifyNoMoreInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addPortToBridge(NodeKey,String,String)}.
     *
     * <ul>
     *   <li>Bridge port is already present.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddPortToBridge3() throws Exception {
        NodeId nodeId = new NodeId("ovsdb:node-1");
        NodeKey nodeKey = new NodeKey(nodeId);
        String mngNodeIdStr = nodeId.getValue() + "/bridge/" +
            DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bnodePath = newNodePath(mngNodeIdStr);
        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().
            setName(DEFAULT_PORT_NAME).
            build();
        TerminationPoint btp = new TerminationPointBuilder().
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        OvsdbBridgeAugmentation ovbr = new OvsdbBridgeAugmentationBuilder().
            build();
        Node bnode = new NodeBuilder().
            setNodeId(new NodeId(mngNodeIdStr)).
            setTerminationPoint(Collections.singletonList(btp)).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        when(utils.read(store, bnodePath)).thenReturn(Optional.of(bnode));
        assertEquals(true, Whitebox.invokeMethod(
                         handler, "addPortToBridge", nodeKey,
                         DEFAULT_BRIDGE_NAME, DEFAULT_PORT_NAME));
        verify(utils).read(store, bnodePath);
        verifyNoMoreInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addPortToBridge(NodeKey,String,String)}.
     *
     * <ul>
     *   <li>Unable to create termination point.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddPortToBridge4() throws Exception {
        NodeId nodeId = new NodeId("ovsdb:node-1");
        NodeKey nodeKey = new NodeKey(nodeId);
        String mngNodeIdStr = nodeId.getValue() + "/bridge/" +
            DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bnodePath = newNodePath(mngNodeIdStr);
        Node bnode = new NodeBuilder().
            setNodeId(new NodeId(mngNodeIdStr)).
            build();
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        when(utils.read(store, bnodePath)).thenReturn(Optional.of(bnode));

        TerminationPointKey tpKey =
            new TerminationPointKey(new TpId(DEFAULT_PORT_NAME));
        InstanceIdentifier<TerminationPoint> tpPath = bnodePath.
            child(TerminationPoint.class, tpKey);
        OvsdbTerminationPointAugmentation ovtp =
            new OvsdbTerminationPointAugmentationBuilder().
            setName(DEFAULT_PORT_NAME).
            build();
        TerminationPoint btp = new TerminationPointBuilder().
            setKey(tpKey).
            addAugmentation(OvsdbTerminationPointAugmentation.class, ovtp).
            build();
        when(utils.put(store, tpPath, btp)).thenReturn(false);

        assertEquals(false, Whitebox.invokeMethod(
                         handler, "addPortToBridge", nodeKey,
                         DEFAULT_BRIDGE_NAME, DEFAULT_PORT_NAME));
        verify(utils).read(store, bnodePath);
        verify(utils).put(store, tpPath, btp);
        verifyNoMoreInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setManagedByForBridge(OvsdbBridgeAugmentationBuilder,NodeKey)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetManagedByForBridge() throws Exception {
        OvsdbBridgeAugmentationBuilder builder =
            new OvsdbBridgeAugmentationBuilder();
        NodeId nodeId = new NodeId("ovsdb-node");
        NodeKey nodeKey = new NodeKey(nodeId);
        InstanceIdentifier<Node> path = newNodePath(nodeId);
        OvsdbNodeRef nref = new OvsdbNodeRef(path);
        Whitebox.invokeMethod(
            handler, "setManagedByForBridge", builder, nodeKey);
        assertEquals(nref, builder.getManagedBy());
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createTerminationPointInstanceIdentifier(Node, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateTerminationPointInstanceIdentifier()
        throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        String[] portNames = {DEFAULT_PORT_NAME, "port-1", "port-2"};
        for (String nid: nodeIds) {
            NodeId nodeId = new NodeId(nid);
            Node node = new NodeBuilder().setNodeId(nodeId).build();
            for (String pname: portNames) {
                TerminationPointKey tpKey = new TerminationPointKey(
                    new TpId(pname));
                InstanceIdentifier<TerminationPoint> path = newNodePath(nodeId).
                    child(TerminationPoint.class, tpKey);
                assertEquals(path, Whitebox.invokeMethod(
                                 handler,
                                 "createTerminationPointInstanceIdentifier",
                                 node, pname));
            }
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addTerminationPoint(Node, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddTerminationPoint() throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        String[] portNames = {DEFAULT_PORT_NAME, "port-1", "port-2"};
        boolean[] bools = {true, false};
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        for (String nid: nodeIds) {
            NodeId nodeId = new NodeId(nid);
            Node node = new NodeBuilder().setNodeId(nodeId).build();
            for (String pname: portNames) {
                TerminationPointKey tpKey = new TerminationPointKey(
                    new TpId(pname));
                InstanceIdentifier<TerminationPoint> path = newNodePath(nodeId).
                    child(TerminationPoint.class, tpKey);
                OvsdbTerminationPointAugmentation ovtp =
                    new OvsdbTerminationPointAugmentationBuilder().
                    setName(pname).
                    build();
                TerminationPoint tp = new TerminationPointBuilder().
                    setKey(tpKey).
                    addAugmentation(OvsdbTerminationPointAugmentation.class,
                                    ovtp).
                    build();
                for (boolean result: bools) {
                    when(utils.put(store, path, tp)).thenReturn(result);
                    assertEquals(result, Whitebox.invokeMethod(
                                     handler, "addTerminationPoint", node,
                                     pname));
                    verify(utils).put(store, path, tp);
                    verifyNoMoreInteractions(utils, service);
                    reset(utils);
                }
            }
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createControllerEntries(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateControllerEntries() throws Exception {
        String[] targets = {
            "tcp:1.2.3.4:6653",
            "tcp:192.168.10.20:6653",
            "tcp:255.255.255.255:6653",
        };
        for (String target: targets) {
            ControllerEntry cent = new ControllerEntryBuilder().
                setTarget(new Uri(target)).
                build();
            List<ControllerEntry> expected = Collections.singletonList(cent);
            assertEquals(expected, Whitebox.invokeMethod(
                             handler, "createControllerEntries", target));
            verifyZeroInteractions(utils, service);
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createMdsalProtocols()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateMdsalProtocols() throws Exception {
        ProtocolEntry pent = new ProtocolEntryBuilder().
            setProtocol(OvsdbBridgeProtocolOpenflow13.class).
            build();
        List<ProtocolEntry> expected = Collections.singletonList(pent);
        assertEquals(expected, Whitebox.invokeMethod(
                         handler, "createMdsalProtocols"));
        verifyZeroInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#getNodeId(InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetNodeId() throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        for (String nid: nodeIds) {
            NodeId nodeId = new NodeId(nid);
            InstanceIdentifier<Node> path = newNodePath(nodeId);
            assertEquals(nodeId, Whitebox.invokeMethod(
                             handler, "getNodeId", path));
            verifyZeroInteractions(utils, service);
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMap(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>No physical port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMap1() throws Exception {
        Uuid tenantId = new Uuid("03e46195-f9de-4802-a573-a037635fa4aa");
        Uuid networkId = new Uuid("0b998c41-6856-4720-94b2-ae727b6cb01b");
        Uuid uuid = new Uuid("59079c03-1ddf-41ab-8b3b-a62bbf179404");
        Port port = new PortBuilder().
            setTenantId(tenantId).
            setNetworkId(networkId).
            setUuid(uuid).
            build();
        OfNode node = new OfNode(12345L);

        handler.setPortMap(port, node, null, null);
        verifyZeroInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMap(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>Physical port ID is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMap2() throws Exception {
        Uuid tenantId = new Uuid("8f3bb555-c97f-4c6d-9f74-f07995605d42");
        Uuid networkId = new Uuid("445208ae-96ae-459e-879b-0594ec05bf83");
        Uuid uuid = new Uuid("5d58739b-e42e-4525-b34c-94104fa08d5a");
        Port port = new PortBuilder().
            setTenantId(tenantId).
            setNetworkId(networkId).
            setUuid(uuid).
            build();
        OfNode node = new OfNode(12345L);
        Long portId = 15L;

        String tname = "8f3bb555c97fc6d9f74f07995605d42";
        String bname = "445208ae96ae59e879b0594ec05bf83";
        String iname = "5d58739be42e525b34c94104fa08d5a";
        SetPortMapInput input = new SetPortMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            setNode(node.getNodeId()).
            setVlanId(new VlanId(0)).
            setPortId(portId.toString()).
            build();
        when(service.setPortMap(input)).thenReturn(HTTP_OK);

        handler.setPortMap(port, node, portId, null);
        verify(service).setPortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMap(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>Physical port name is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMap3() throws Exception {
        Uuid tenantId = new Uuid("9697026a-47c0-4028-acd8-c873eacca310");
        Uuid networkId = new Uuid("bf700718-4616-43c2-8062-b68659b795ea");
        Uuid uuid = new Uuid("7c5ea917-019d-4c3c-a5db-16d57640f9ec");
        Port port = new PortBuilder().
            setTenantId(tenantId).
            setNetworkId(networkId).
            setUuid(uuid).
            build();
        OfNode node = new OfNode(333L);
        String portName = "port-34";

        String tname = "9697026a47c0028acd8c873eacca310";
        String bname = "bf70071846163c28062b68659b795ea";
        String iname = "7c5ea917019dc3ca5db16d57640f9ec";
        SetPortMapInput input = new SetPortMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            setNode(node.getNodeId()).
            setVlanId(new VlanId(0)).
            setPortName(portName).
            build();
        when(service.setPortMap(input)).thenReturn(HTTP_OK);

        handler.setPortMap(port, node, null, portName);
        verify(service).setPortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMap(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>Broken neutron port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMap4() throws Exception {
        Port port = new PortBuilder().build();
        OfNode node = new OfNode(99887766L);
        Long portId = 33L;

        SetPortMapInput input = new SetPortMapInputBuilder().
            setNode(node.getNodeId()).
            setVlanId(new VlanId(0)).
            setPortId(portId.toString()).
            build();
        when(service.setPortMap(input)).thenReturn(HTTP_BAD_REQUEST);

        handler.setPortMap(port, node, portId, null);
        verify(service).setPortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#deletePortMap(Port)}.
     *
     * <ul>
     *   <li>Valid neutron port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeletePortMap1() throws Exception {
        Uuid tenantId = new Uuid("fffe27e1-7f84-4972-b478-3489a8d92a27");
        Uuid networkId = new Uuid("5e05ea14-b668-4e1c-b89b-63bfc8ae3575");
        Uuid uuid = new Uuid("8b35fcb8-2710-4282-aaa3-7c0daf8564d4");
        Port port = new PortBuilder().
            setTenantId(tenantId).
            setNetworkId(networkId).
            setUuid(uuid).
            build();

        String tname = "fffe27e17f84972b4783489a8d92a27";
        String bname = "5e05ea14b668e1cb89b63bfc8ae3575";
        String iname = "8b35fcb82710282aaa37c0daf8564d4";
        RemovePortMapInput input = new RemovePortMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setInterfaceName(iname).
            build();
        when(service.removePortMap(input)).thenReturn(HTTP_OK);

        handler.deletePortMap(port);
        verify(service).removePortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#deletePortMap(Port)}.
     *
     * <ul>
     *   <li>Broken neutron port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeletePortMap2() throws Exception {
        Port port = new PortBuilder().build();
        RemovePortMapInput input = new RemovePortMapInputBuilder().build();
        when(service.removePortMap(input)).thenReturn(HTTP_BAD_REQUEST);

        handler.deletePortMap(port);
        verify(service).removePortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#deletePortMap(Port)}.
     *
     * <ul>
     *   <li>The specified neutron port is not present.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeletePortMap3() throws Exception {
        Port port = new PortBuilder().build();
        RemovePortMapInput input = new RemovePortMapInputBuilder().build();
        when(service.removePortMap(input)).thenReturn(HTTP_NOT_FOUND);

        handler.deletePortMap(port);
        verify(service).removePortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createInstanceIdentifier(NodeKey,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateInstanceIdentifier1() throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        String[] bridges = {"bridge-1", "bridge-2"};
        for (String nid: nodeIds) {
            NodeId nodeId = new NodeId(nid);
            NodeKey nodeKey = new NodeKey(nodeId);
            for (String bname: bridges) {
                String nodeStr = nodeId.getValue() + "/bridge/" + bname;
                InstanceIdentifier<Node> path = newNodePath(nodeStr);
                assertEquals(path, Whitebox.invokeMethod(
                                 handler, "createInstanceIdentifier", nodeKey,
                                 bname));
                verifyZeroInteractions(utils, service);
            }
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createInstanceIdentifier(NodeId)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateInstanceIdentifier2() throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        for (String nid: nodeIds) {
            NodeId nodeId = new NodeId(nid);
            InstanceIdentifier<Node> path = newNodePath(nodeId);
            assertEquals(path, Whitebox.invokeMethod(
                             handler, "createInstanceIdentifier", nodeId));
            verifyZeroInteractions(utils, service);
        }
    }

    /**
     * Test case for {@link OVSDBEventHandler#getManagedNodeId(NodeId,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetManagedNodeId() throws Exception {
        String[] nodeIds = {"ovsdb:node-1", "ovsdb:node-2"};
        String[] bridges = {DEFAULT_BRIDGE_NAME, "bridge-1", "bridge-2"};
        for (String nid: nodeIds) {
            NodeId nodeId = new NodeId(nid);
            for (String bname: bridges) {
                String idStr = nid + "/bridge/" + bname;
                NodeId expected = new NodeId(idStr);
                assertEquals(expected, Whitebox.invokeMethod(
                                 handler, "getManagedNodeId", nodeId, bname));
                verifyZeroInteractions(utils, service);
            }
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#isBridgeOnOvsdbNode(OvsdbNodeAugmentation,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testisBridgeOnOvsdbNode() throws Exception {
        // In case where managed node entry list is null.
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        assertEquals(false, Whitebox.invokeMethod(
                         handler, "isBridgeOnOvsdbNode", ovnode,
                         DEFAULT_BRIDGE_NAME));
        verifyZeroInteractions(utils, service);

        // In case where managed node entry is not found.
        List<ManagedNodeEntry> list = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            InstanceIdentifier<Node> path = newNodePath("ovsdb:node-" + i);
            ManagedNodeEntry mnent = new ManagedNodeEntryBuilder().
                setBridgeRef(new OvsdbBridgeRef(path)).
                build();
            list.add(mnent);
        }
        ovnode = new OvsdbNodeAugmentationBuilder().
            setManagedNodeEntry(list).
            build();
        assertEquals(false, Whitebox.invokeMethod(
                         handler, "isBridgeOnOvsdbNode", ovnode,
                         DEFAULT_BRIDGE_NAME));
        verifyZeroInteractions(utils, service);

        // In case where bridge node path is present in the managed node entry
        // list.
        String bridgeStr = "ovsdb:node1/bridge/" + DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bridgeRef = newNodePath(bridgeStr);
        ManagedNodeEntry mnent = new ManagedNodeEntryBuilder().
            setBridgeRef(new OvsdbBridgeRef(bridgeRef)).
            build();
        list.add(mnent);
        ovnode = new OvsdbNodeAugmentationBuilder().
            setManagedNodeEntry(list).
            build();
        assertEquals(true, Whitebox.invokeMethod(
                         handler, "isBridgeOnOvsdbNode", ovnode,
                         DEFAULT_BRIDGE_NAME));
        verifyZeroInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addBridge(NodeKey,OvsdbNodeAugmentation,String)}.
     *
     * <ul>
     *   <li>Successful completion.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddBridge1() throws Exception {
        NodeId nodeId = new NodeId("ovsdb:node-1");
        NodeKey nodeKey = new NodeKey(nodeId);

        String ipaddr = "192.168.100.234";
        IpAddress ctlrIp = new IpAddress(new Ipv4Address(ipaddr));
        ConnectionInfo cinfo = new ConnectionInfoBuilder().
            setLocalIp(ctlrIp).
            build();
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            setConnectionInfo(cinfo).
            build();
        ControllerEntry cent = new ControllerEntryBuilder().
            setTarget(new Uri("tcp:" + ipaddr + ":6653")).
            build();
        ProtocolEntry pent = new ProtocolEntryBuilder().
            setProtocol(OvsdbBridgeProtocolOpenflow13.class).
            build();
        String mngNodeIdStr = nodeId.getValue() + "/bridge/" +
            DEFAULT_BRIDGE_NAME;
        InstanceIdentifier<Node> bnodePath = newNodePath(mngNodeIdStr);
        OvsdbBridgeAugmentation ovbr = new OvsdbBridgeAugmentationBuilder().
            setBridgeName(new OvsdbBridgeName(DEFAULT_BRIDGE_NAME)).
            setControllerEntry(Collections.singletonList(cent)).
            setProtocolEntry(Collections.singletonList(pent)).
            setFailMode(OvsdbFailModeSecure.class).
            setManagedBy(new OvsdbNodeRef(newNodePath(nodeId))).
            build();
        Node bnode = new NodeBuilder().
            setNodeId(new NodeId(mngNodeIdStr)).
            addAugmentation(OvsdbBridgeAugmentation.class, ovbr).
            build();
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        when(utils.put(store, bnodePath, bnode)).thenReturn(true);

        assertEquals(true, Whitebox.invokeMethod(
                         handler, "addBridge", nodeKey, ovnode,
                         DEFAULT_BRIDGE_NAME));
        verify(utils).put(store, bnodePath, bnode);
        verifyNoMoreInteractions(utils, service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addBridge(NodeKey,OvsdbNodeAugmentation,String)}.
     *
     * <ul>
     *   <li>No connection info.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddBridge2() throws Exception {
        NodeId nodeId = new NodeId("ovsdb:node-1");
        NodeKey nodeKey = new NodeKey(nodeId);
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();

        assertEquals(false, Whitebox.invokeMethod(
                         handler, "addBridge", nodeKey, ovnode,
                         DEFAULT_BRIDGE_NAME));
        verifyZeroInteractions(utils, service);
    }

    /**
     * Test case for {@link OVSDBEventHandler#getOvsdbBridgeName()}.
     */
    @Test
    public void testGetOvsdbBridgeName() {
        assertEquals(DEFAULT_BRIDGE_NAME, handler.getOvsdbBridgeName());

        String[] bridges = {"bridge-1", "bridge-2"};
        for (String bname: bridges) {
            NeutronConfig cfg = new NeutronConfig(bname, null, null, null);
            OVSDBEventHandler ovh = new OVSDBEventHandler(cfg, utils, service);
            assertEquals(bname, ovh.getOvsdbBridgeName());
            verifyZeroInteractions(utils, service);
        }
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#getControllerTarget(OvsdbNodeAugmentation)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetControllerTarget() throws Exception {
        // In case of successful completion.
        String[] ipaddrs = {"10.20.30.40", "192.168.0.1", "255.255.255.255"};
        for (String ipaddr: ipaddrs) {
            IpAddress ctlrIp = new IpAddress(new Ipv4Address(ipaddr));
            ConnectionInfo cinfo = new ConnectionInfoBuilder().
                setLocalIp(ctlrIp).
                build();
            OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
                setConnectionInfo(cinfo).
                build();
            String expected = "tcp:" + ipaddr + ":6653";
            assertEquals(expected, Whitebox.invokeMethod(
                             handler, "getControllerTarget", ovnode));
            verifyZeroInteractions(utils, service);
        }

        // In case where connection-info is null.
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        assertNull(Whitebox.invokeMethod(
                       handler, "getControllerTarget", ovnode));
        verifyZeroInteractions(utils, service);

        // In case where where IP address is not set in connection-info.
        ConnectionInfo cinfo = new ConnectionInfoBuilder().
            build();
        ovnode = new OvsdbNodeAugmentationBuilder().build();
        assertNull(Whitebox.invokeMethod(
                       handler, "getControllerTarget", ovnode));
        verifyZeroInteractions(utils, service);
    }

    /**
     * Create path to the node in the network topology.
     *
     * @param id  The node identifier.
     * @return  Instance identifier.
     */
    private InstanceIdentifier<Node> newNodePath(String id) {
        return newNodePath(new NodeId(id));
    }

    /**
     * Create path to the node in the network topology.
     *
     * @param nodeId  The node identifier.
     * @return  Instance identifier.
     */
    private InstanceIdentifier<Node> newNodePath(NodeId nodeId) {
        TopologyId topoId = new TopologyId(new Uri("ovsdb:1"));
        return InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, new TopologyKey(topoId)).
            child(Node.class, new NodeKey(nodeId)).
            build();
    }
}
