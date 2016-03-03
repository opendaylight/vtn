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

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ConnectionInfo;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.junit.Assert;
import org.junit.Before;
import org.junit.runner.RunWith;
import org.junit.Test;
import org.mockito.Matchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ControllerEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.bridge.attributes.ProtocolEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.port._interface.attributes.InterfaceExternalIds;
import java.security.InvalidParameterException;
import com.google.common.base.Optional;

/**
 * JUnit test for {@link OVSDBEventHandler}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ OVSDBEventHandler.class, VTNManagerService.class,
                  MdsalUtils.class, InstanceIdentifier.class,
                  VTNNeutronUtils.class, Port.class, OfNode.class})
public final class OVSDBEventHandlerTest {
    /**
     * Mock-up of {@link DataBroker}.
     */
    private DataBroker dataBroker;
    /**
     * Mock-up of {@link DataBroker}.
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
    /**
     * An {@link OVSDBEventHandler} instance for test.
     */
    private LogicalDatastoreType oper;
    @Before
    public void setUp() throws Exception {
        dataBroker = Mockito.mock(DataBroker.class);
        utils = Mockito.spy(new MdsalUtils(dataBroker));
        service = PowerMockito.mock(VTNManagerService.class);
        handler = PowerMockito.spy(new OVSDBEventHandler(utils, service));
        oper = LogicalDatastoreType.CONFIGURATION;
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeAdded(Node,DataObject)}.
     */
    @Test
    public void testNodeAdded() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        DataObject mockDataObject = PowerMockito.mock(DataObject.class);
        handler.nodeAdded(mockNode, mockDataObject);
        PowerMockito.verifyPrivate(handler, Mockito.times(1)).
                                   invoke("createInternalNetworkForNeutron",
                                    mockNode);
        PowerMockito.doThrow(new RuntimeException("Exception for testing...")).
                        when(handler, "createInternalNetworkForNeutron",
                             mockNode);
        handler.nodeAdded(mockNode, mockDataObject);
        PowerMockito.verifyPrivate(handler, Mockito.times(2)).
                                   invoke("createInternalNetworkForNeutron",
                                   mockNode);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeRemoved(Node)}.
     */
    @Test
    public void testNodeRemoved() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        PowerMockito.doReturn(true).
                        when(handler, "deleteBridge",
                             Matchers.eq(mockNode), Matchers.eq(null));
        handler.nodeRemoved(mockNode);
        PowerMockito.verifyPrivate(handler, Mockito.times(1)).
                                   invoke("deleteBridge",
                                   mockNode, null);

    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#DeleteBridge()}.
     */
    @Test
    public void testDeleteBridge() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        InstanceIdentifier<Node> mockInstanceIdentifier  =
                                    PowerMockito.mock(InstanceIdentifier.class);
        utils = Mockito.mock(MdsalUtils.class);
        Mockito.when(utils.delete(Matchers.eq(LogicalDatastoreType.CONFIGURATION), Matchers.eq(mockInstanceIdentifier))).thenReturn(true);
        handler = PowerMockito.spy(new OVSDBEventHandler(utils, service));
        PowerMockito.doReturn(mockInstanceIdentifier).
            when(handler , "createInstanceIdentifier" , null, "br-int");
        boolean output = Whitebox.invokeMethod(handler,
                                  "deleteBridge" , mockNode , "br-int");
        Assert.assertTrue(output);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler
     *        #extractTerminationPointAugmentation(Node,portName)}.
     */
    @Test
    public void testExtractTerminationPointAugmentation() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Object output = Whitebox.invokeMethod(handler,
                                              "extractTerminationPointAugmentation",
                                               mockNode, "node1");
        Assert.assertEquals(output, null);
        Mockito.when(mockNode.getAugmentation(OvsdbBridgeAugmentation.class)).
                thenReturn(PowerMockito.mock(OvsdbBridgeAugmentation.class));
        output = Whitebox.invokeMethod(handler,
                                       "extractTerminationPointAugmentation",
                                       mockNode, "node1");
        Assert.assertEquals(output, null);
        List<OvsdbTerminationPointAugmentation> mockList =
                           new ArrayList<OvsdbTerminationPointAugmentation>();

        OvsdbTerminationPointAugmentation ovsdbTPAOne =
                    PowerMockito.mock(OvsdbTerminationPointAugmentation.class);
        OvsdbTerminationPointAugmentation ovsdbTPATwo =
                    PowerMockito.mock(OvsdbTerminationPointAugmentation.class);
        Mockito.when(ovsdbTPAOne.getName()).thenReturn("node1");
        Mockito.when(ovsdbTPATwo.getName()).thenReturn("node2");
        mockList.add(ovsdbTPATwo);
        mockList.add(ovsdbTPAOne);
        PowerMockito.doReturn(mockList).
                     when(handler, "extractTerminationPointAugmentations",
                          mockNode);
        output = Whitebox.invokeMethod(handler,
                                       "extractTerminationPointAugmentation",
                                       mockNode, "node1");
        Assert.assertTrue(mockList.size() > 0);

    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#extractTerminationPointAugmentations(Node)}.
     */
    @Test
    public void testExtractTerminationPointAugmentations() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        List<TerminationPoint> mockList = new ArrayList<TerminationPoint>();
        TerminationPoint mockTPOne = Mockito.mock(TerminationPoint.class);
        TerminationPoint mockTPTwo = Mockito.mock(TerminationPoint.class);
        Mockito.when(mockTPTwo.
                     getAugmentation(OvsdbTerminationPointAugmentation.class)).
                     thenReturn(Mockito.mock(
                                OvsdbTerminationPointAugmentation .class));
        mockList.add(mockTPOne);
        mockList.add(mockTPTwo);
        Mockito.when(mockNode.getTerminationPoint()).thenReturn(mockList , null);
        Object output = Whitebox.
                        invokeMethod(handler,
                                     "extractTerminationPointAugmentations",
                                     mockNode);
        Assert.assertTrue(output instanceof List);
        output = Whitebox.invokeMethod(handler,
                                       "extractTerminationPointAugmentations",
                                       mockNode);
        Assert.assertTrue(output instanceof List);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#addPortToBridge(Node,bridgeName,portName)}.
     */
    @Test
    public void testAddPortToBridge() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        InstanceIdentifier<Node> mockInstanceIdentifier  =
                                   PowerMockito.mock(InstanceIdentifier.class);
        Mockito.when(mockNode.getNodeId()).
                thenReturn(Mockito.mock(NodeId.class));
        PowerMockito.doReturn(mockNode).
                       when(handler , "getBridgeConfigNode" ,
                            mockNode , "bridge1");
        PowerMockito.
                  doReturn(Mockito.
                           mock(OvsdbTerminationPointAugmentation.class)).
                          when(handler, "extractTerminationPointAugmentation",
                               mockNode, "port1");
        boolean output = Whitebox.invokeMethod(handler,
                                               "addPortToBridge", mockNode,
                                               "bridge1", "port1");
        Assert.assertTrue(output);
        PowerMockito.doReturn(null).
                     when(handler, "extractTerminationPointAugmentation",
                          mockNode, "port1");
        PowerMockito.doReturn(true).
                     when(handler, "addTerminationPoint",
                          mockNode, "bridge1", "port1");
        output = Whitebox.
                    invokeMethod(handler , "addPortToBridge" , mockNode,
                                 "bridge1" , "port1");
        Assert.assertTrue(output);
        PowerMockito.doReturn(false).
                       when(handler, "addTerminationPoint",
                            mockNode, "bridge1", "port1");
        output = Whitebox.
                     invokeMethod(handler, "addPortToBridge",
                                  mockNode, "bridge1", "port1");
        Assert.assertFalse(output);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler
     *        #setManagedByForBridge(OvsdbBridgeAugmentationBuilder,NodeKey)}.
     */
    @Test
    public void testSetManagedByForBridge() throws Exception {
        OvsdbBridgeAugmentationBuilder mockBuilder =
                           Mockito.mock(OvsdbBridgeAugmentationBuilder.class);
        Whitebox.invokeMethod(handler,
                              "setManagedByForBridge", mockBuilder,
                              Mockito.mock(NodeKey.class));
        Mockito.verify(mockBuilder, Mockito.times(1)).
                       setManagedBy(Matchers.any(OvsdbNodeRef.class));
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler
     *  #createTerminationPointInstanceIdentifier(Node, portName)}.
     */
    @Test
    public void testCreateTerminationPointInstanceIdentifier() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Mockito.when(mockNode.getKey()).
                thenReturn(Mockito.mock(NodeKey.class));
        Object output =
                Whitebox.invokeMethod(handler,
                                      "createTerminationPointInstanceIdentifier",
                                      mockNode, "port1");
        Assert.assertTrue(output instanceof InstanceIdentifier);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler
     *  #addTerminationPoint(Node, bridgeName, portName)}.
     */
    @Test
    public void testAddTerminationPoint() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        NodeKey key = PowerMockito.mock(NodeKey.class);
        Mockito.when(mockNode.getKey()).thenReturn(key);
        Mockito.when(key.getNodeId()).thenReturn(Mockito.mock(NodeId.class));
        MdsalUtils utils2 = PowerMockito.mock(MdsalUtils.class);
        handler = new OVSDBEventHandler(utils2, service);
        boolean output =
                Whitebox.invokeMethod(handler,
                                     "addTerminationPoint", mockNode,
                                     "bridge1", "port1");
        Assert.assertFalse(output);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#createControllerEntries(targetString)}.
     */
    @Test
    public void testCreateControllerEntries() throws Exception {
        Object output =
                Whitebox.invokeMethod(handler,
                                      "createControllerEntries", "test uri1");
        Assert.assertTrue(output instanceof ArrayList);
        Assert.assertEquals(((List)output).size(), 1);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#CreateMdsalProtocols}.
     */
    @Test
    public void testCreateMdsalProtocols() throws Exception {
        Object output = Whitebox.invokeMethod(handler, "createMdsalProtocols");
        Assert.assertTrue(output instanceof ArrayList);
        Assert.assertEquals(((List)output).size(), 1);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler
     *  #setPortMapForInterface(Port, OfNode, ofPort, portName)}.
     */
    @Test
    public void testSetPortMapForInterface() throws Exception {
        PowerMockito.doReturn(HTTP_OK).
                     when(handler, "getVTNIdentifiers",
                          Matchers.any(Port.class),
                          Matchers.eq(new String[3]));
        Object output =
                Whitebox.invokeMethod(handler, "setPortMapForInterface",
                                      PowerMockito.mock(Port.class),
                                      PowerMockito.mock(OfNode.class),
                                      new Long(1000), "port1");
        Assert.assertNotNull(output);
        PowerMockito.doReturn(HTTP_BAD_REQUEST).
                     when(handler, "getVTNIdentifiers",
                          Matchers.any(Port.class),
                          Matchers.eq(new String[3]));
        output = Whitebox.invokeMethod(handler, "setPortMapForInterface",
                                       PowerMockito.mock(Port.class),
                                       PowerMockito.mock(OfNode.class),
                                       new Long(1000), "port1");
        Assert.assertNotNull(output);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#deletePortMapForInterface(Port) }.
     */
    @Test
    public void testDeletePortMapForInterface() throws Exception {
        PowerMockito.doReturn(HTTP_OK).when(handler, "getVTNIdentifiers",
                                            Matchers.any(Port.class),
                                            Matchers.eq(new String[3]));
        int output = Whitebox.invokeMethod(handler,
                                           "deletePortMapForInterface",
                                           PowerMockito.mock(Port.class));
        Assert.assertNotNull(output);
        PowerMockito.doReturn(HTTP_BAD_REQUEST).
                     when(handler , "getVTNIdentifiers" ,
                          Matchers.any(Port.class) ,
                          Matchers.eq(new String[3]));
        output = Whitebox.invokeMethod(handler,
                                       "deletePortMapForInterface",
                                       PowerMockito.mock(Port.class));
        Assert.assertNotNull(output);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#getVTNIdentifiers(Port , String[] vtnIDs) }.
     */
    @Test
    public void testGetVTNIdentifiers() throws Exception {
        Port port = null;
        Object output = Whitebox.invokeMethod(handler,
                                             "getVTNIdentifiers",
                                             port, new String[3]);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
        port = Mockito.mock(Port.class);
        Uuid mockUuid = Mockito.mock(Uuid.class);
        Mockito.when(port.getTenantId()).thenReturn(mockUuid);
        Mockito.when(port.getNetworkId()).thenReturn(mockUuid);
        Mockito.when(port.getUuid()).thenReturn(mockUuid);
        output = Whitebox.invokeMethod(handler,
                                       "getVTNIdentifiers", port,
                                       new String[3]);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
        Mockito.when(mockUuid.getValue()).
               thenReturn("123e4567-e89b-12d3-a456-426655440000");
        output = Whitebox.invokeMethod(handler,
                                       "getVTNIdentifiers",
                                       port, new String[3]);
        Assert.assertEquals(output, HTTP_OK);
        PowerMockito.mockStatic(VTNNeutronUtils.class);
        PowerMockito.when(VTNNeutronUtils.
                          convertUUIDToKey(Matchers.any(String.class))).
                     thenReturn(null, "s1", null, "s1", "s2",
                                null, "s1", "s2", "s3", null);
        output = Whitebox.invokeMethod(handler,
                                       "getVTNIdentifiers",
                                       port, new String[3]);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
        output = Whitebox.invokeMethod(handler,
                                       "getVTNIdentifiers", port,
                                       new String[3]);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
        output = Whitebox.invokeMethod(handler,
                                       "getVTNIdentifiers",
                                       port, new String[3]);
        Assert.assertEquals(output, HTTP_BAD_REQUEST);
        output = Whitebox.invokeMethod(handler,
                                       "getVTNIdentifiers", port,
                                       new String[3]);
        Assert.assertEquals(output, HTTP_OK);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#readNeutronPort(String uuid) }.
     */
    @Test
    public void testReadNeutronPort() throws Exception {
        Object output =
                Whitebox.invokeMethod(handler,
                                      "readNeutronPort",
                                      "123e4567-e89b-12d3-a456-426655440000");
        Assert.assertNull(output);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#extractBridgeAugmentation(Node) }.
     */
    @Test
    public void testExtractBridgeAugmentation() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Object output = Whitebox.invokeMethod(handler,
                                              "extractBridgeAugmentation",
                                              null);
        Assert.assertNull(output);
        OvsdbBridgeAugmentation  mockOvsdbBridgeAugmentation  =
                             PowerMockito.mock(OvsdbBridgeAugmentation .class);
        Mockito.when(mockNode.getAugmentation(OvsdbBridgeAugmentation.class)).
                thenReturn(mockOvsdbBridgeAugmentation);
        output = Whitebox.invokeMethod(handler,
                                       "extractBridgeAugmentation", mockNode);
        Assert.assertEquals(output, mockOvsdbBridgeAugmentation);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#getBridgeNode(Node, bridgeName) }.
     */
    @Test
    public void testGetBridgeNode() throws Exception {
        Node mockNode = PowerMockito.mock(Node.class);
        Object output =
                Whitebox.invokeMethod(handler , "getBridgeNode",
                                      mockNode , "bridge1");
        Assert.assertNull(output);
        OvsdbBridgeAugmentation  mockOvsdbBridgeAugmentation  =
                                 Mockito.mock(OvsdbBridgeAugmentation .class);
        OvsdbBridgeName mockBridgeName = Mockito.mock(OvsdbBridgeName.class);
        Mockito.when(mockBridgeName.getValue()).thenReturn("bridge1");
        Mockito.when(mockOvsdbBridgeAugmentation.getBridgeName()).
                thenReturn(mockBridgeName);
        PowerMockito.doReturn(mockOvsdbBridgeAugmentation).
                     when(handler, "extractBridgeAugmentation",
                          Matchers.any(Node.class));
        output = Whitebox.invokeMethod(handler,
                                      "getBridgeNode", mockNode, "bridge1");
        Assert.assertEquals(output, mockOvsdbBridgeAugmentation);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#createInstanceIdentifier(NodeId) }.
     */
    @Test
    public void testCreateInstanceIdentifier() throws Exception {
        NodeId mockId = Mockito.mock(NodeId.class);
        NodeKey mockKey = Mockito.mock(NodeKey.class);
        Mockito.when(mockKey.getNodeId()).thenReturn(mockId);
        Object output =
                Whitebox.invokeMethod(handler , "createInstanceIdentifier",
                                      mockKey , "bridge1");
        Assert.assertTrue(output instanceof InstanceIdentifier);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#isBridgeOnOvsdbNode(Node , bridgeName) }.
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
        Assert.assertFalse(output);
        OvsdbNodeAugmentation  mockOvsdbNodeAugmentation  =
                                      Mockito.mock(OvsdbNodeAugmentation .class);
        PowerMockito.doReturn(mockOvsdbNodeAugmentation).
                       when(handler , "extractOvsdbNode" ,
                               Matchers.any(Node.class));
        //empty list shld ret null
        List<ManagedNodeEntry> mockmgedList =
                                  new ArrayList<ManagedNodeEntry>();
        Mockito.when(mockOvsdbNodeAugmentation.getManagedNodeEntry()).
                  thenReturn(mockmgedList);
        output = Whitebox.invokeMethod(handler,
                                       "isBridgeOnOvsdbNode",
                                                mockovsNode, "bridge1");
        Assert.assertFalse(output);
        ManagedNodeEntry mockTPOne = Mockito.mock(ManagedNodeEntry.class);
        ManagedNodeEntry mockTPTwo = Mockito.mock(ManagedNodeEntry.class);
        mockmgedList.add(mockTPOne);
        mockmgedList.add(mockTPTwo);
        Mockito.when(mockOvsdbNodeAugmentation.getManagedNodeEntry()).
                  thenReturn(mockmgedList);
    }

    /**
     * Test case for
     * {@link OVSDBEventHandler#addBridge(Node , bridgeName) }.
     */
    @Test
    public void testaddBridge() throws Exception {
        //Test case to return false
        Node mockovsNode = PowerMockito.mock(Node.class);
        PowerMockito.doReturn("10.10.10.10").when(handler, "getControllerTarget",
                                                  mockovsNode);
        ConnectionInfo  mockConnectionInfo  =
                                     Mockito.mock(ConnectionInfo.class);
        PowerMockito.doReturn(null).
                       when(handler , "getConnectionInfo", mockovsNode);

        try {
            boolean output = Whitebox.invokeMethod(handler,
                                  "addBridge" , mockovsNode , "bridge1");
        }  catch (Exception e) {
            Assert.assertTrue(e instanceof InvalidParameterException);
        }
        InstanceIdentifier<Node>   mockInstanceIdentifier  =
                                    PowerMockito.mock(InstanceIdentifier.class);
        PowerMockito.doReturn(mockInstanceIdentifier).
                       when(handler , "createInstanceIdentifier" ,
                            null , "bridge1");
        NodeId mockNodeId  = Mockito.mock(NodeId.class);
        PowerMockito.doReturn(mockNodeId).
                       when(handler, "createManagedNodeId",
                               Matchers.eq(mockInstanceIdentifier));

        ControllerEntry mockControllerEntry  =
                                 Mockito.mock(ControllerEntry.class);
        PowerMockito.doReturn(new ArrayList()).
                       when(handler, "createControllerEntries", "bridge1");

        ProtocolEntry mockProtocolEntry  =
                                 Mockito.mock(ProtocolEntry.class);
        PowerMockito.doReturn(new ArrayList()).
                       when(handler, "createMdsalProtocols");

        utils =  Mockito.mock(MdsalUtils.class);
        Mockito.when(utils.put(oper , mockInstanceIdentifier , null)).
                  thenReturn(true);
        Mockito.when(utils.put(oper , mockInstanceIdentifier , null)).
                  thenReturn(false);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#readOVSDBPorts(Node , action) }.
     */
    @Test
    public void testreadOVSDBPorts() throws Exception {
        Node mockovsNode = PowerMockito.mock(Node.class);
        Optional mockOptional = Mockito.mock(Optional.class);
        utils =  Mockito.mock(MdsalUtils.class);
        InstanceIdentifier<Node> mockInstanceIdentifier  =
                                   PowerMockito.mock(InstanceIdentifier.class);
        Mockito.when(utils.read(Matchers.eq(LogicalDatastoreType.OPERATIONAL),
                                  Matchers.any(InstanceIdentifier.class))).
                   thenReturn(mockOptional);
        handler = PowerMockito.spy(new OVSDBEventHandler(utils, service));
        NodeId mockNodeId = Mockito.mock(NodeId.class);
        mockNodeId = mockovsNode.getNodeId();
        PowerMockito.doReturn(mockInstanceIdentifier).
                       when(handler,
                               "createInstanceIdentifier", mockNodeId);
        Mockito.when(mockOptional.orNull()).thenReturn(mockovsNode);
        OvsdbBridgeAugmentation mockOvsdbBridgeAugmentation =
                                PowerMockito.mock(OvsdbBridgeAugmentation.class);
        PowerMockito.doReturn(mockOvsdbBridgeAugmentation).
                       when(handler ,
                               "getBridgeNode" , mockovsNode , null);
        List<InterfaceExternalIds> mockListForExtenralIds =
                                     new ArrayList<InterfaceExternalIds>();
        InterfaceExternalIds mockInterfaceExternalIds =
                                     Mockito.mock(InterfaceExternalIds.class);
        Mockito.when(mockInterfaceExternalIds.getExternalIdKey()).
                   thenReturn("iface-id");
        mockListForExtenralIds.add(mockInterfaceExternalIds);

        List<OvsdbTerminationPointAugmentation> mockList =
                              new ArrayList<OvsdbTerminationPointAugmentation>();
        OvsdbTerminationPointAugmentation mockTPOne =
                           Mockito.mock(OvsdbTerminationPointAugmentation.class);
        Mockito.when(mockTPOne.getInterfaceExternalIds()).
                   thenReturn(mockListForExtenralIds);
        mockList.add(mockTPOne);
        PowerMockito.doReturn(mockList).
                       when(handler ,
                               "extractTerminationPointAugmentations" ,
                                mockovsNode);
        Whitebox.invokeMethod(handler, "readOVSDBPorts",
                              mockovsNode, "update");
    }
}
