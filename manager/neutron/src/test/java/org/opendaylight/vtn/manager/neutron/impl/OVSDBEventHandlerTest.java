/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.MockitoAnnotations.initMocks;
import java.util.List;
import org.junit.After;
import org.junit.Before;
import org.junit.runner.RunWith;
import org.junit.Test;

import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.WriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;
import org.opendaylight.controller.sal.binding.api.RpcConsumerRegistry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.ovsdb.node.attributes.ManagedNodeEntry;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yangtools.yang.binding.DataObject;

import com.google.common.util.concurrent.CheckedFuture;

/**
 * JUnit test for {@link OVSDBEventHandler}.
 */
@RunWith(MockitoJUnitRunner.class)
public final class OVSDBEventHandlerTest {
    /**
     * DataBroker instance for test.
     */
    private DataBroker dataBroker;
    /**
     * MdsalUtils instance for test.
     */
    private MdsalUtils utils;
    /**
     * VTNManagerService instance for test.
     */
    private VTNManagerService service;
    /**
     * An {@link OVSDBEventHandler} instance for test.
     */
    private OVSDBEventHandler handler;
    /**
     * Instance of WriteTransaction to perform unit testing.
     */
    private WriteTransaction transaction;
    /**
     * Instance of CheckedFuture to perform unit testing.
     */
    private CheckedFuture<Void, TransactionCommitFailedException> value = null;
    /**
     * RPC consumer registry.
     */
    @Mock
    private RpcConsumerRegistry  rpcRegistry;
    /**
     * RPC service for VTN management.
     */
    @Mock
    private VtnService  vtnService;
    /**
     * RPC service for vBridge management.
     */
    @Mock
    private VtnVbridgeService  vbridgeService;
    /**
     * RPC service for virtual interface management.
     */
    @Mock
    private VtnVinterfaceService  vinterfaceService;
    /**
     * RPC service for port mapping management.
     */
    @Mock
    private VtnPortMapService  portMapService;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() throws Exception {
        initMocks(this);
        dataBroker = Mockito.mock(DataBroker.class);
        transaction = Mockito.mock(WriteTransaction.class);
        value = Mockito.mock(CheckedFuture.class);
        utils = new MdsalUtils(dataBroker);

        Mockito.when(rpcRegistry.getRpcService(VtnService.class)).
            thenReturn(vtnService);
        Mockito.when(rpcRegistry.getRpcService(VtnVbridgeService.class)).
            thenReturn(vbridgeService);
        Mockito.when(rpcRegistry.getRpcService(VtnVinterfaceService.class)).
            thenReturn(vinterfaceService);
        Mockito.when(rpcRegistry.getRpcService(VtnPortMapService.class)).
            thenReturn(portMapService);

        service = new VTNManagerService(utils, rpcRegistry);
        handler = new OVSDBEventHandler(utils, service);

        handler.getOvsdbFailMode();
        handler.getOvsdbProtocol();
        handler.getOvsdbBridgeName();
        handler.getOvsdbPortName();

    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#nodeAdded(Node,DataObject)}.
     */
    @Test
    public void testNodeAdded() throws Exception {
        Node mockNode = Mockito.mock(Node.class);
        DataObject mockDataObject = Mockito.mock(DataObject.class);
        OvsdbNodeAugmentation mockOvsdbNodeAugmentation = Mockito.mock(OvsdbNodeAugmentation.class);
        final List<ManagedNodeEntry> listManagedNodeEntry = Mockito.mock(List.class);
        ManagedNodeEntry mockManagedNodeEntry = Mockito.mock(ManagedNodeEntry.class);

        handler.setOvsdbPortName("eth0");
        handler.setOvsdbBridgeName("br-int");
        handler.setOvsdbProtocol("OpenFlow13");
        handler.setOvsdbFailMode("secure");

        Mockito.when(mockNode.getAugmentation(OvsdbNodeAugmentation.class)).
                thenReturn(mockOvsdbNodeAugmentation);
        listManagedNodeEntry.add(mockManagedNodeEntry);
        handler.nodeAdded(mockNode, mockDataObject);

    }
    /**
     * Test case for negative scenario
     * {@link OVSDBEventHandler#nodeAdded(Node,DataObject)}.
     */
    @Test
    public void testNodeAddedFail() throws Exception {
        Node mockNode = Mockito.mock(Node.class);
        DataObject mockDataObject = Mockito.mock(DataObject.class);

        handler.setOvsdbPortName(null);
        handler.setOvsdbBridgeName(null);
        handler.setOvsdbProtocol(null);
        handler.setOvsdbFailMode(null);
        handler.nodeAdded(mockNode, mockDataObject);
    }
    /**
     * Test case for
     * {@link OVSDBEventHandler#NodeRemove(Node,DataObject)}.
     */
    @Test
    public void testNodeRemoved() throws Exception {
        Node mockNode = Mockito.mock(Node.class);
        NodeKey key = Mockito.mock(NodeKey.class);
        NodeId nodeId = Mockito.mock(NodeId.class);
        handler.setOvsdbBridgeName("br-int");
        Mockito.when(mockNode.getKey()).thenReturn(key);
        Mockito.when(key.getNodeId()).thenReturn(nodeId);
        Mockito.when(transaction.submit()).thenReturn(value);
        Mockito.when(dataBroker.newWriteOnlyTransaction()).thenReturn(transaction);
        handler.nodeRemoved(mockNode);

    }
    /**
     * This will make unwanted object eligible for garbage collection.
     */
    @After
    public void tearDown() {
        handler = null;
    }
}
