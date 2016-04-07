/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.MockitoAnnotations.initMocks;
import static org.mockito.Mockito.times;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import java.util.Map;
import org.junit.After;
import org.junit.Before;
import org.junit.runner.RunWith;
import org.junit.Test;

import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcConsumerRegistry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * JUnit test for {@link OvsdbDataChangeListener}.
 */
@RunWith(MockitoJUnitRunner.class)
public class OvsdbDataChangeListenerTest extends TestBase{
    /**
     * DataBroker instance for test.
     */
    @Mock
    private DataBroker  dataBroker;
    /**
     * VTNManagerService instance for test.
     */
    private VTNManagerService  vtnManager;
    /**
     * MdsalUtils instance for test.
     */
    private MdsalUtils mdSal;
    /**
     * Registration to be associated with {@link OvsdbDataChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  listenerReg;
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
     * An {@link OvsdbDataChangeListener} instance for test.
     */
    private OvsdbDataChangeListener  ovsdbDataChangeListener;
    /**
     * An {@link OVSDBEventHandler} instance for test.
     */
    private OVSDBEventHandler handler;
   /**
     * AsyncDataChangeEvent object reference for unit testing.
     */
    private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> asyncDataChangeEvent;
    /**
     * Collection of InstanceIdentifier.
     */
    private Map<InstanceIdentifier<?>, DataObject> nodeMap;
    /**
     * NetworkKey object reference for unit testing.
     */
    private NodeKey nodeKey;
    /**
     * Node object reference for unit testing.
     */
    private Node node;
    /**
     * InstanceIdentifier object reference for unit testing.
     */
    private InstanceIdentifier<?> instanceIdentifier;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() throws Exception {
        initMocks(this);
        mdSal = new MdsalUtils(dataBroker);

        Mockito.when(rpcRegistry.getRpcService(VtnService.class)).
            thenReturn(vtnService);
        Mockito.when(rpcRegistry.getRpcService(VtnVbridgeService.class)).
            thenReturn(vbridgeService);
        Mockito.when(rpcRegistry.getRpcService(VtnVinterfaceService.class)).
            thenReturn(vinterfaceService);
        Mockito.when(rpcRegistry.getRpcService(VtnPortMapService.class)).
            thenReturn(portMapService);

        vtnManager = new VTNManagerService(mdSal, rpcRegistry);

        asyncDataChangeEvent = Mockito.mock(AsyncDataChangeEvent.class);
        ovsdbDataChangeListener =
                   new OvsdbDataChangeListener(dataBroker,
                                               mdSal, vtnManager);

    }
    /**
     * Test case for
     * {@link OperationalListener#
     * OperationalListener(DataBroker,AtomicReference)}.
     */
    @Test
    public void testConstructor() {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        ovsdbDataChangeListener = new
                       OvsdbDataChangeListener(dataBroker,
                                               mdSal,
                                               vtnManager);
        // Ensure that NeutronNetworkChangeListener has been
        // registered as data change listener.
        Mockito.when(dataBroker.
                        registerDataChangeListener(eq(oper),
                                                   eq(getPath()),
                                                   eq(ovsdbDataChangeListener),
                                                   eq(scope))).
                thenReturn(listenerReg);
        verifyZeroInteractions(listenerReg);
    }
    /**
     * Test case for
     * {@link neutronNetworkListener#createNetwork(AsyncDataChangeEvent)}
     *
     */
    @Test
    public void testonDataChanged() {
        ovsdbDataChangeListener.onDataChanged(asyncDataChangeEvent);
        /**
         * Verifying asyncDataChangeEventMockObj object invoking both
         * getCreatedData and getUpdatedData methods.
         */
        verify(asyncDataChangeEvent , times(2)).getCreatedData();
        verify(asyncDataChangeEvent , times(2)).getRemovedPaths();

    }
    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Node> getPath() {
        return InstanceIdentifier.create(NetworkTopology.class).
                child(Topology.class).
                child(Node.class);
    }
    /**
     * This will make unwanted object eligible for garbage collection.
     */
    @After
    public void tearDown() {
        ovsdbDataChangeListener = null;
    }
}
