/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.eq;
import static org.mockito.Matchers.isA;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.times;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.HashMap;
import java.util.Map;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcConsumerRegistry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.Networks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * JUnit test for {@link NeutronNetworkChangeListener}.
 */
@RunWith(MockitoJUnitRunner.class)
public class NeutronNetworkChangeListenerTest extends TestBase {
    /**
     * DataBroker instance for test.
     */
    @Mock
    private DataBroker  dataBroker;
    /**
     * Registration to be associated with {@link NeutronNetworkChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  listenerReg;
    /**
     * An {@link OperationalListener} instance for test.
     */
    private NeutronNetworkChangeListener  neutronNetworkListener;
    /**
     * VTNManagerService instance for test.
     */
    private VTNManagerService  vtnManager;
    /**
     * MdsalUtils instance for test.
     */
    private MdsalUtils mdSal;
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
     * AsyncDataChangeEvent object reference for unit testing.
     */
    private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> asyncDataChangeEvent;
    /**
     * Collection of InstanceIdentifier.
     */
    private Map<InstanceIdentifier<?>, DataObject> networkMap;
    /**
     * Network object reference for unit testing.
     */
    private Network network;
    /**
     * NetworkKey object reference for unit testing.
     */
    private NetworkKey networkKey;
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
        neutronNetworkListener =
                   new NeutronNetworkChangeListener(dataBroker, vtnManager);

        networkMap = new HashMap<InstanceIdentifier<?>,  DataObject>();
        when(asyncDataChangeEvent.getCreatedData())
                .thenReturn(networkMap);
        when(asyncDataChangeEvent.getUpdatedData())
                .thenReturn(networkMap);
        networkKey = mock(NetworkKey.class);
        network = mock(Network.class);
        when(network.getKey()).thenReturn(networkKey);
        instanceIdentifier = mock(InstanceIdentifier.class);
        when(dataBroker.registerDataChangeListener(
                  any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                  isA(NeutronNetworkChangeListener.class), any(DataChangeScope.class))).
             thenReturn(listenerReg);
        neutronNetworkListener = new NeutronNetworkChangeListener(dataBroker , vtnManager);
    }
    /**
     * Test case for
     * {@link NeutronNetworkChangeListener#NeutronNetworkChangeListener(DataBroker , VTNManagerService ) }.
     */
    @Test
    public void testConstructor() {
        LogicalDatastoreType oper = LogicalDatastoreType.CONFIGURATION;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        // Ensure that NeutronNetworkChangeListener has been registered as data change
        // listener.
        verify(dataBroker, times(0)).
                    registerDataChangeListener(eq(oper),
                                               eq(getPath()),
                                               eq(neutronNetworkListener),
                                               eq(scope));
        verifyZeroInteractions(listenerReg);
    }
    /**
     * Return a path to the MD-SAL data model to listen.
     * @return  A path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Network> getPath() {
        return InstanceIdentifier.create(Neutron.class).
                child(Networks.class).
                child(Network.class);

    }

    /**
     * Test case for {@link neutronNetworkListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);
        // Close the listener.
        neutronNetworkListener.close();
        verify(listenerReg).close();
    }

    /**
     * Test case for
     * {@link neutronNetworkListener#createNetwork(AsyncDataChangeEvent)}
     *
     */
    @Test
    public void testonDataChanged() {
        neutronNetworkListener.onDataChanged(asyncDataChangeEvent);
        /**
         * Verifying asyncDataChangeEventMockObj object invoking both
         * getCreatedData and getUpdatedData methods.
         */
        verify(asyncDataChangeEvent, times(1)).getCreatedData();
        verify(asyncDataChangeEvent, times(1)).getUpdatedData();
    }
}
