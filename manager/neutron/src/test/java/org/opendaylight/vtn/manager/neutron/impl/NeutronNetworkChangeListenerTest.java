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
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.Networks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.api.mockito.PowerMockito;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * JUnit test for {@link NeutronNetworkChangeListener}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({ NeutronNetworkChangeListener.class, VTNManagerService.class, MdsalUtils.class, InstanceIdentifier.class, OfNode.class})
public class NeutronNetworkChangeListenerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
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
     * VTNManagerService instance.
     */
    private VTNManagerService  vtnManagerService;
    /**
     * AsyncDataChangeEvent object reference for unit testing.
     */
    private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> asyncDataChangeEventMockObj;
    /**
     * Collection of InstanceIdentifier and Intent.
     */
    private Map<InstanceIdentifier<?>, DataObject> networkMap;
    /**
     * Intent object reference for unit testing.
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
        vtnManagerService = PowerMockito.mock(VTNManagerService.class);
        neutronNetworkListener = PowerMockito.spy(new NeutronNetworkChangeListener(dataBroker, vtnManagerService));
        asyncDataChangeEventMockObj = mock(AsyncDataChangeEvent.class);
        networkMap = new HashMap<InstanceIdentifier<?>,  DataObject>();
        when(asyncDataChangeEventMockObj.getCreatedData())
                .thenReturn(networkMap);
        when(asyncDataChangeEventMockObj.getUpdatedData())
                .thenReturn(networkMap);
        networkKey = mock(NetworkKey.class);
        network = mock(Network.class);
        when(network.getKey()).thenReturn(networkKey);
        instanceIdentifier = mock(InstanceIdentifier.class);
        //networkMap.put(instanceIdentifier, network);
        when(dataBroker.registerDataChangeListener(
                  any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                  isA(NeutronNetworkChangeListener.class), any(DataChangeScope.class))).
             thenReturn(listenerReg);
        neutronNetworkListener = new NeutronNetworkChangeListener(dataBroker , vtnManagerService);
    }
    /**
     * Test case for
     * {@link NeutronNetworkChangeListener#NeutronNetworkChangeListener(DataBroker , VTNManagerService ) }.
     */
    @Test
    public void testConstructor() {
        initMocks(this);
        LogicalDatastoreType oper = LogicalDatastoreType.CONFIGURATION;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        // Ensure that NeutronNetworkChangeListener has been registered as data change
        // listener.
        verify(dataBroker, times(0)).registerDataChangeListener(
            eq(oper), eq(getPath()), eq(neutronNetworkListener), eq(scope));
        verifyZeroInteractions(listenerReg);
    }
    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
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
        neutronNetworkListener.onDataChanged(asyncDataChangeEventMockObj);
        /**
         * Verifying asyncDataChangeEventMockObj object invoking both
         * getCreatedData and getUpdatedData methods.
         */
        verify(asyncDataChangeEventMockObj, times(1)).getCreatedData();
        verify(asyncDataChangeEventMockObj, times(1)).getUpdatedData();
    }
}
