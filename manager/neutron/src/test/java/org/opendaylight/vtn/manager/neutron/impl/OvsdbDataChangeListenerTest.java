/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.junit.Before;
import org.mockito.Mock;
import org.junit.Test;
import static org.mockito.Mockito.times;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;

import org.powermock.api.mockito.PowerMockito;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yangtools.concepts.ListenerRegistration;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;

import java.util.HashMap;
import java.util.Map;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isA;
import static org.mockito.MockitoAnnotations.initMocks;
import static org.mockito.Mockito.when;

import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * JUnit test for {@link OvsdbDataChangeListener}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ OvsdbDataChangeListener.class, VTNManagerService.class,
                  MdsalUtils.class, InstanceIdentifier.class, OfNode.class,
                  OVSDBEventHandler.class})
public class OvsdbDataChangeListenerTest {
    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;
    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private  MdsalUtils mdUtils;
    /**
     * VTNManagerService instance.
     */
    private VTNManagerService  vtnManagerService;
    /**
     * Registration to be associated with {@link OvsdbDataChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  listenerReg;
    /**
     * An {@link OvsdbDataChangeListener} instance for test.
     */
    private OvsdbDataChangeListener  ovsdbDataChangeListener;
    private OVSDBEventHandler handler;
    /**
     * AsyncDataChangeEvent object reference for unit testing.
     */
    private AsyncDataChangeEvent asyncDataChangeEventMockObj;
    /**
     * Collection of InstanceIdentifier and Intent.
     */
    private Map<InstanceIdentifier<?>, DataObject> nodeMap;
    /**
     * NetworkKey object reference for unit testing.
     */
    private NodeKey nodeKey;
    /**
     * Intent object reference for unit testing.
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
        vtnManagerService = PowerMockito.mock(VTNManagerService.class);
        ovsdbDataChangeListener = PowerMockito.
                               spy(new OvsdbDataChangeListener(dataBroker,
                                                                         mdUtils,
                                                                                     vtnManagerService));
        asyncDataChangeEventMockObj =
                                     PowerMockito.mock(AsyncDataChangeEvent.class);
        nodeMap = new HashMap<InstanceIdentifier<?>,  DataObject>();
        when(asyncDataChangeEventMockObj.getCreatedData())
                .thenReturn(nodeMap);
        when(asyncDataChangeEventMockObj.getUpdatedData())
                .thenReturn(nodeMap);
        nodeKey = PowerMockito.mock(NodeKey.class);
        node = PowerMockito.mock(Node.class);
        when(node.getKey()).thenReturn(nodeKey);
        instanceIdentifier = PowerMockito.mock(InstanceIdentifier.class);
        //nodeMap.put(instanceIdentifier, node);
        handler = PowerMockito.
                            spy(new OVSDBEventHandler(mdUtils, vtnManagerService));
        when(dataBroker.
                   registerDataChangeListener(any(LogicalDatastoreType.class),
                                                  any(InstanceIdentifier.class),
                                          isA(OvsdbDataChangeListener.class),
                                                              any(DataChangeScope.class))).
             thenReturn(listenerReg);
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
                       OvsdbDataChangeListener(dataBroker, mdUtils,
                                                         vtnManagerService);
        // Ensure that NeutronNetworkChangeListener has been registered as data change
        // listener.
        when(dataBroker.
                        registerDataChangeListener(eq(oper),
                                                         eq(getPath()),
                                                                     eq(ovsdbDataChangeListener),
                                                                     eq(scope))).
            thenReturn(listenerReg);
        //handler = new OVSDBEventHandler(mdUtils, vtnManagerService);
        verifyZeroInteractions(listenerReg);
    }
    /**
     * Test case for
     * {@link neutronNetworkListener#createNetwork(AsyncDataChangeEvent)}
     *
     */
    @Test
    public void testonDataChanged() {
        ovsdbDataChangeListener.onDataChanged(asyncDataChangeEventMockObj);
        ovsdbDataChangeListener.onDataChanged(null);
        /**
         * Verifying asyncDataChangeEventMockObj object invoking both
         * getCreatedData and getUpdatedData methods.
         */
        verify(asyncDataChangeEventMockObj , times(2)).getCreatedData();
        verify(asyncDataChangeEventMockObj , times(1)).getUpdatedData();


    }
    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Node> getPath() {
        return InstanceIdentifier.create(NetworkTopology.class).
                child(Topology.class).
                child(Node.class);


    }
}
