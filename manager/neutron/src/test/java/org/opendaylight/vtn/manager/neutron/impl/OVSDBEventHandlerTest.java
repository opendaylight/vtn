/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_OK;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.neutron.impl.NeutronConfigTest.DEFAULT_BRIDGE_NAME;

import java.util.ArrayList;
import java.util.List;
import java.security.InvalidParameterException;

import com.google.common.base.Optional;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.Matchers;
import org.mockito.Mockito;

import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ConnectionInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIds;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;

/**
 * JUnit test for {@link OVSDBEventHandler}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ OVSDBEventHandler.class, VTNManagerService.class,
                  MdsalUtils.class, InstanceIdentifier.class,
                  VTNNeutronUtils.class, Port.class, OfNode.class})
public final class OVSDBEventHandlerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
     */
    private DataBroker dataBroker;

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
        dataBroker = mock(DataBroker.class);
        utils = Mockito.spy(new MdsalUtils(dataBroker));
        service = PowerMockito.mock(VTNManagerService.class);
        NeutronConfig cfg = new NeutronConfig();
        handler = PowerMockito.spy(new OVSDBEventHandler(cfg, utils, service));
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeAdded(Node,DataObject)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNodeAdded() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        DataObject mockDataObject = PowerMockito.mock(DataObject.class);
        handler.nodeAdded(mockNode, mockDataObject);
        PowerMockito.verifyPrivate(handler, Mockito.times(1)).
            invoke("readSystemProperties", mockNode);
        PowerMockito.doThrow(new RuntimeException("Exception for testing...")).
            when(handler, "readSystemProperties", mockNode);
        handler.nodeAdded(mockNode, mockDataObject);
        PowerMockito.verifyPrivate(handler, Mockito.times(2)).
            invoke("readSystemProperties", mockNode);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeRemoved(Node)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNodeRemoved() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        PowerMockito.doReturn(true).
            when(handler, "deleteBridge",
                 Matchers.eq(mockNode), Matchers.eq(DEFAULT_BRIDGE_NAME));
        handler.nodeRemoved(mockNode);
        PowerMockito.verifyPrivate(handler, Mockito.times(1)).
            invoke("deleteBridge", mockNode, DEFAULT_BRIDGE_NAME);
    }

    /**
     * Test case for {@link OVSDBEventHandler#deleteBridge(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeleteBridge() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        InstanceIdentifier<Node> mockInstanceIdentifier =
            PowerMockito.mock(InstanceIdentifier.class);
        utils = mock(MdsalUtils.class);
        when(utils.delete(Matchers.eq(LogicalDatastoreType.CONFIGURATION),
                          Matchers.eq(mockInstanceIdentifier))).
            thenReturn(true);
        NeutronConfig cfg = new NeutronConfig();
        handler = PowerMockito.spy(new OVSDBEventHandler(cfg, utils, service));
        PowerMockito.doReturn(mockInstanceIdentifier).
            when(handler, "createInstanceIdentifier", null, "br-int");
        boolean output = Whitebox.invokeMethod(
            handler, "deleteBridge", mockNode, "br-int");
        assertTrue(output);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#extractTerminationPointAugmentation(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExtractTerminationPointAugmentation() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Object output = Whitebox.invokeMethod(handler,
                                              "extractTerminationPointAugmentation",
                                               mockNode, "node1");
        assertEquals(output, null);
        when(mockNode.getAugmentation(OvsdbBridgeAugmentation.class)).
            thenReturn(PowerMockito.mock(OvsdbBridgeAugmentation.class));
        output = Whitebox.invokeMethod(
            handler, "extractTerminationPointAugmentation", mockNode, "node1");
        assertEquals(output, null);
        List<OvsdbTerminationPointAugmentation> mockList =
                           new ArrayList<OvsdbTerminationPointAugmentation>();

        OvsdbTerminationPointAugmentation ovsdbTPAOne =
                    PowerMockito.mock(OvsdbTerminationPointAugmentation.class);
        OvsdbTerminationPointAugmentation ovsdbTPATwo =
                    PowerMockito.mock(OvsdbTerminationPointAugmentation.class);
        when(ovsdbTPAOne.getName()).thenReturn("node1");
        when(ovsdbTPATwo.getName()).thenReturn("node2");
        mockList.add(ovsdbTPATwo);
        mockList.add(ovsdbTPAOne);
        PowerMockito.doReturn(mockList).
            when(handler, "extractTerminationPointAugmentations", mockNode);
        output = Whitebox.invokeMethod(
            handler, "extractTerminationPointAugmentation", mockNode, "node1");
        assertTrue(mockList.size() > 0);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#extractTerminationPointAugmentations(Node)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExtractTerminationPointAugmentations() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        List<TerminationPoint> mockList = new ArrayList<TerminationPoint>();
        TerminationPoint mockTPOne = Mockito.mock(TerminationPoint.class);
        TerminationPoint mockTPTwo = Mockito.mock(TerminationPoint.class);
        when(mockTPTwo.getAugmentation(
                 OvsdbTerminationPointAugmentation.class)).
            thenReturn(mock(OvsdbTerminationPointAugmentation .class));
        mockList.add(mockTPOne);
        mockList.add(mockTPTwo);
        when(mockNode.getTerminationPoint()).thenReturn(mockList, null);
        Object output = Whitebox.invokeMethod(
            handler, "extractTerminationPointAugmentations", mockNode);
        assertTrue(output instanceof List);
        output = Whitebox.invokeMethod(
            handler, "extractTerminationPointAugmentations", mockNode);
        assertTrue(output instanceof List);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addPortToBridge(Node,String,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddPortToBridge() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        InstanceIdentifier<Node> mockInstanceIdentifier  =
                                   PowerMockito.mock(InstanceIdentifier.class);
        when(mockNode.getNodeId()).thenReturn(mock(NodeId.class));
        PowerMockito.doReturn(mockNode).
            when(handler, "getBridgeConfigNode", mockNode, "bridge1");
        PowerMockito.doReturn(mock(OvsdbTerminationPointAugmentation.class)).
            when(handler, "extractTerminationPointAugmentation",
                 mockNode, "port1");
        boolean output = Whitebox.invokeMethod(
            handler, "addPortToBridge", mockNode, "bridge1", "port1");
        assertTrue(output);
        PowerMockito.doReturn(null).
            when(handler, "extractTerminationPointAugmentation", mockNode,
                 "port1");
        PowerMockito.doReturn(true).
            when(handler, "addTerminationPoint", mockNode, "bridge1", "port1");
        output = Whitebox.invokeMethod(
            handler, "addPortToBridge", mockNode, "bridge1", "port1");
        assertTrue(output);
        PowerMockito.doReturn(false).
            when(handler, "addTerminationPoint", mockNode, "bridge1", "port1");
        output = Whitebox.invokeMethod(
            handler, "addPortToBridge", mockNode, "bridge1", "port1");
        assertFalse(output);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setManagedByForBridge(OvsdbBridgeAugmentationBuilder,NodeKey)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetManagedByForBridge() throws Exception {
        OvsdbBridgeAugmentationBuilder mockBuilder =
            mock(OvsdbBridgeAugmentationBuilder.class);
        Whitebox.invokeMethod(handler,
                              "setManagedByForBridge", mockBuilder,
                              mock(NodeKey.class));
        verify(mockBuilder, Mockito.times(1)).
            setManagedBy(Matchers.any(OvsdbNodeRef.class));
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createTerminationPointInstanceIdentifier(Node, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateTerminationPointInstanceIdentifier() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        when(mockNode.getKey()).thenReturn(mock(NodeKey.class));
        Object output = Whitebox.invokeMethod(
            handler, "createTerminationPointInstanceIdentifier", mockNode,
            "port1");
        assertTrue(output instanceof InstanceIdentifier);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addTerminationPoint(Node, String, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddTerminationPoint() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        NodeKey key = PowerMockito.mock(NodeKey.class);
        when(mockNode.getKey()).thenReturn(key);
        when(key.getNodeId()).thenReturn(mock(NodeId.class));
        MdsalUtils utils2 = PowerMockito.mock(MdsalUtils.class);
        NeutronConfig cfg = new NeutronConfig();
        handler = new OVSDBEventHandler(cfg, utils2, service);
        boolean output =
                Whitebox.invokeMethod(handler,
                                     "addTerminationPoint", mockNode,
                                     "bridge1", "port1");
        assertFalse(output);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createControllerEntries(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateControllerEntries() throws Exception {
        Object output =
                Whitebox.invokeMethod(handler,
                                      "createControllerEntries", "test uri1");
        assertTrue(output instanceof ArrayList);
        assertEquals(((List)output).size(), 1);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createMdsalProtocols()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateMdsalProtocols() throws Exception {
        Object output = Whitebox.invokeMethod(handler, "createMdsalProtocols");
        assertTrue(output instanceof ArrayList);
        assertEquals(((List)output).size(), 1);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMapForInterface(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>No physical port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMapForInterface1() throws Exception {
        Uuid tenantId = new Uuid("03e46195-f9de-4802-a573-a037635fa4aa");
        Uuid networkId = new Uuid("0b998c41-6856-4720-94b2-ae727b6cb01b");
        Uuid uuid = new Uuid("59079c03-1ddf-41ab-8b3b-a62bbf179404");
        Port port = new PortBuilder().
            setTenantId(tenantId).
            setNetworkId(networkId).
            setUuid(uuid).
            build();
        OfNode node = new OfNode(12345L);

        Whitebox.invokeMethod(handler, "setPortMapForInterface",
                              port, node, null, null);
        verifyZeroInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMapForInterface(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>Physical port ID is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMapForInterface2() throws Exception {
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

        Whitebox.invokeMethod(handler, "setPortMapForInterface",
                              port, node, portId, null);
        verify(service).setPortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMapForInterface(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>Physical port name is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMapForInterface3() throws Exception {
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

        Whitebox.invokeMethod(handler, "setPortMapForInterface",
                              port, node, null, portName);
        verify(service).setPortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#setPortMapForInterface(Port,OfNode,Long,String)}.
     *
     * <ul>
     *   <li>Broken neutron port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortMapForInterface4() throws Exception {
        Port port = new PortBuilder().build();
        OfNode node = new OfNode(99887766L);
        Long portId = 33L;

        SetPortMapInput input = new SetPortMapInputBuilder().
            setNode(node.getNodeId()).
            setVlanId(new VlanId(0)).
            setPortId(portId.toString()).
            build();
        when(service.setPortMap(input)).thenReturn(HTTP_BAD_REQUEST);

        Whitebox.invokeMethod(handler, "setPortMapForInterface",
                              port, node, portId, null);
        verify(service).setPortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#deletePortMapForInterface(Port)}.
     *
     * <ul>
     *   <li>Valid neutron port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeletePortMapForInterface1() throws Exception {
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

        Whitebox.invokeMethod(handler, "deletePortMapForInterface", port);
        verify(service).removePortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#deletePortMapForInterface(Port)}.
     *
     * <ul>
     *   <li>Broken neutron port is specified.</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDeletePortMapForInterface2() throws Exception {
        Port port = new PortBuilder().build();
        RemovePortMapInput input = new RemovePortMapInputBuilder().build();
        when(service.removePortMap(input)).thenReturn(HTTP_BAD_REQUEST);

        Whitebox.invokeMethod(handler, "deletePortMapForInterface", port);
        verify(service).removePortMap(input);
        verifyNoMoreInteractions(service);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#readNeutronPort(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadNeutronPort() throws Exception {
        Object output =
                Whitebox.invokeMethod(handler,
                                      "readNeutronPort",
                                      "123e4567-e89b-12d3-a456-426655440000");
        assertNull(output);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#extractBridgeAugmentation(Node)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExtractBridgeAugmentation() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Object output = Whitebox.invokeMethod(handler,
                                              "extractBridgeAugmentation",
                                              null);
        assertNull(output);
        OvsdbBridgeAugmentation  mockOvsdbBridgeAugmentation  =
            PowerMockito.mock(OvsdbBridgeAugmentation .class);
        when(mockNode.getAugmentation(OvsdbBridgeAugmentation.class)).
            thenReturn(mockOvsdbBridgeAugmentation);
        output = Whitebox.invokeMethod(handler,
                                       "extractBridgeAugmentation", mockNode);
        assertEquals(output, mockOvsdbBridgeAugmentation);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#getBridgeNode(Node, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetBridgeNode() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Object output = Whitebox.invokeMethod(
            handler, "getBridgeNode", mockNode, "bridge1");
        assertNull(output);
        OvsdbBridgeAugmentation  mockOvsdbBridgeAugmentation  =
            mock(OvsdbBridgeAugmentation .class);
        OvsdbBridgeName mockBridgeName = mock(OvsdbBridgeName.class);
        when(mockBridgeName.getValue()).thenReturn("bridge1");
        when(mockOvsdbBridgeAugmentation.getBridgeName()).
            thenReturn(mockBridgeName);
        PowerMockito.doReturn(mockOvsdbBridgeAugmentation).
                     when(handler, "extractBridgeAugmentation",
                          Matchers.any(Node.class));
        output = Whitebox.invokeMethod(handler,
                                      "getBridgeNode", mockNode, "bridge1");
        assertEquals(output, mockOvsdbBridgeAugmentation);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#createInstanceIdentifier(NodeId)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateInstanceIdentifier() throws Exception {
        NodeId mockId = mock(NodeId.class);
        NodeKey mockKey = mock(NodeKey.class);
        when(mockKey.getNodeId()).thenReturn(mockId);
        Object output = Whitebox.invokeMethod(
            handler, "createInstanceIdentifier", mockKey, "bridge1");
        assertTrue(output instanceof InstanceIdentifier);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#isBridgeOnOvsdbNode(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testisBridgeOnOvsdbNode() throws Exception {
        //Test case to return false
        Node mockovsNode = PowerMockito.mock(Node.class);
        PowerMockito.doReturn(null).when(handler,
                                           "extractOvsdbNode",
                                                   Matchers.eq(mockovsNode));
        boolean output = Whitebox.
                                invokeMethod(handler,
                                                  "isBridgeOnOvsdbNode",
                                                     mockovsNode, "bridge1");
        assertFalse(output);
        OvsdbNodeAugmentation  mockOvsdbNodeAugmentation  =
            mock(OvsdbNodeAugmentation .class);
        PowerMockito.doReturn(mockOvsdbNodeAugmentation).
            when(handler, "extractOvsdbNode", Matchers.any(Node.class));
        //empty list shld ret null
        List<ManagedNodeEntry> mockmgedList =
                                  new ArrayList<ManagedNodeEntry>();
        when(mockOvsdbNodeAugmentation.getManagedNodeEntry()).
            thenReturn(mockmgedList);
        output = Whitebox.invokeMethod(handler,
                                       "isBridgeOnOvsdbNode",
                                                mockovsNode, "bridge1");
        assertFalse(output);
        ManagedNodeEntry mockTPOne = mock(ManagedNodeEntry.class);
        ManagedNodeEntry mockTPTwo = mock(ManagedNodeEntry.class);
        mockmgedList.add(mockTPOne);
        mockmgedList.add(mockTPTwo);
        when(mockOvsdbNodeAugmentation.getManagedNodeEntry()).
            thenReturn(mockmgedList);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addBridge(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testaddBridge() throws Exception {
        //Test case to return false
        Node mockovsNode = PowerMockito.mock(Node.class);
        PowerMockito.doReturn("10.10.10.10").
            when(handler, "getControllerTarget", mockovsNode);
        ConnectionInfo  mockConnectionInfo  = mock(ConnectionInfo.class);
        PowerMockito.doReturn(null).
            when(handler, "getConnectionInfo", mockovsNode);

        try {
            boolean output = Whitebox.invokeMethod(
                handler, "addBridge", mockovsNode, "bridge1");
        }  catch (Exception e) {
            assertTrue(e instanceof InvalidParameterException);
        }
        InstanceIdentifier<Node>   mockInstanceIdentifier  =
            PowerMockito.mock(InstanceIdentifier.class);
        PowerMockito.doReturn(mockInstanceIdentifier).
            when(handler, "createInstanceIdentifier", null, "bridge1");
        NodeId mockNodeId  = mock(NodeId.class);
        PowerMockito.doReturn(mockNodeId).
                       when(handler, "createManagedNodeId",
                               Matchers.eq(mockInstanceIdentifier));

        ControllerEntry mockControllerEntry = mock(ControllerEntry.class);
        PowerMockito.doReturn(new ArrayList()).
                       when(handler, "createControllerEntries", "bridge1");

        ProtocolEntry mockProtocolEntry = mock(ProtocolEntry.class);
        PowerMockito.doReturn(new ArrayList()).
                       when(handler, "createMdsalProtocols");

        utils =  mock(MdsalUtils.class);

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(utils.put(oper, mockInstanceIdentifier, null)).thenReturn(true);
        when(utils.put(oper, mockInstanceIdentifier, null)).thenReturn(false);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#readOVSDBPorts(Node,String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testreadOVSDBPorts() throws Exception {
        Node mockovsNode = PowerMockito.mock(Node.class);
        Optional mockOptional = mock(Optional.class);
        utils =  Mockito.mock(MdsalUtils.class);
        InstanceIdentifier<Node> mockInstanceIdentifier  =
            PowerMockito.mock(InstanceIdentifier.class);
        when(utils.read(Matchers.eq(LogicalDatastoreType.OPERATIONAL),
                        Matchers.any(InstanceIdentifier.class))).
            thenReturn(mockOptional);
        NeutronConfig cfg = new NeutronConfig();
        handler = PowerMockito.spy(new OVSDBEventHandler(cfg, utils, service));
        NodeId mockNodeId = mock(NodeId.class);
        mockNodeId = mockovsNode.getNodeId();
        PowerMockito.doReturn(mockInstanceIdentifier).
            when(handler, "createInstanceIdentifier", mockNodeId);
        when(mockOptional.orNull()).thenReturn(mockovsNode);
        OvsdbBridgeAugmentation ovbridge =
            new OvsdbBridgeAugmentationBuilder().
            setBridgeName(new OvsdbBridgeName(DEFAULT_BRIDGE_NAME)).
            build();
        PowerMockito.doReturn(ovbridge).
            when(handler, "getBridgeNode", mockovsNode, DEFAULT_BRIDGE_NAME);
        List<InterfaceExternalIds> mockListForExtenralIds =
                                     new ArrayList<InterfaceExternalIds>();
        InterfaceExternalIds mockInterfaceExternalIds =
            mock(InterfaceExternalIds.class);
        when(mockInterfaceExternalIds.getExternalIdKey()).
            thenReturn("iface-id");
        mockListForExtenralIds.add(mockInterfaceExternalIds);

        List<OvsdbTerminationPointAugmentation> mockList =
                              new ArrayList<OvsdbTerminationPointAugmentation>();
        OvsdbTerminationPointAugmentation mockTPOne =
            mock(OvsdbTerminationPointAugmentation.class);
        when(mockTPOne.getInterfaceExternalIds()).
            thenReturn(mockListForExtenralIds);
        mockList.add(mockTPOne);
        PowerMockito.doReturn(mockList).
            when(handler, "extractTerminationPointAugmentations", mockovsNode);
        Whitebox.invokeMethod(handler, "readOVSDBPorts", mockovsNode, "update");
    }
}
