/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.isA;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.mock;

import java.util.HashMap;
import java.util.Map;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.runners.MockitoJUnitRunner;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcConsumerRegistry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * JUnit test for {@link PortDataChangeListener}.
 */
@RunWith(MockitoJUnitRunner.class)
public class PortDataChangeListenerTest extends TestBase {
    /**
     * DataBroker instance for test.
     */
    @Mock
    private DataBroker  dataBroker;
    /**
     * Registration to be associated with {@link PortDataChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  listenerReg;
    /**
     * An {@link PortDataChangeListener} instance for test.
     */
    private PortDataChangeListener  portDataChangeListener;
    /**
     * VTNManagerService instance for test.
     */
    private VTNManagerService  vtnManagerService;
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
    private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> asyncDataChangeEvent;;
    /**
     * Collection of InstanceIdentifier.
     */
    private Map<InstanceIdentifier<?>, DataObject> portMap;
    /**
     * Port object reference for unit testing.
     */
    private Port port;
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
        MdsalUtils mdSal = new MdsalUtils(dataBroker);

        Mockito.when(rpcRegistry.getRpcService(VtnService.class)).
            thenReturn(vtnService);
        Mockito.when(rpcRegistry.getRpcService(VtnVbridgeService.class)).
            thenReturn(vbridgeService);
        Mockito.when(rpcRegistry.getRpcService(VtnVinterfaceService.class)).
            thenReturn(vinterfaceService);
        Mockito.when(rpcRegistry.getRpcService(VtnPortMapService.class)).
            thenReturn(portMapService);

        vtnManagerService = new VTNManagerService(mdSal, rpcRegistry);
        asyncDataChangeEvent = Mockito.mock(AsyncDataChangeEvent.class);
        portMap = new HashMap<InstanceIdentifier<?>,  DataObject>();
        when(asyncDataChangeEvent.getCreatedData())
                .thenReturn(portMap);
        when(asyncDataChangeEvent.getUpdatedData())
                .thenReturn(portMap);
        port = mock(Port.class);
        instanceIdentifier = mock(InstanceIdentifier.class);
        when(dataBroker.registerDataChangeListener(
                  any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                  isA(PortDataChangeListener.class), any(DataChangeScope.class))).
             thenReturn(listenerReg);

        portDataChangeListener = new PortDataChangeListener(dataBroker , vtnManagerService);
    }

    /**
     * Test case for
     * {@link portDataChangeListener#PortDataChangeListener(DataBroker,vtnManagerService)}.
     */
    @Test
    public void testConstructor() {
        LogicalDatastoreType oper = LogicalDatastoreType.CONFIGURATION;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        portDataChangeListener = new
                       PortDataChangeListener(dataBroker, vtnManagerService);
        Mockito.when(dataBroker.
                        registerDataChangeListener(eq(oper),
                                                   eq(getPath()),
                                                   eq(portDataChangeListener),
                                                   eq(scope))).
                thenReturn(listenerReg);
        // Ensure that PortDataChangeListener has been registered as data change
        // listener.
        verifyZeroInteractions(listenerReg);
    }
    /**
     * Return a path to the MD-SAL data model to listen.
     * @return  A path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Port> getPath() {
        return InstanceIdentifier.create(Neutron.class).
                child(Ports.class).
                child(Port.class);
    }

    /**
     * Test case for {@link PortDataChangeListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);

        // Close the listener.
        portDataChangeListener.close();
        verify(listenerReg).close();

        // Listener registrations should never be closed twice.
        portDataChangeListener.close();
        verify(listenerReg , times(2)).close();
    }

    /**
     * Test case for
     * {@link neutronNetworkListener#createNetwork(AsyncDataChangeEvent)}
     *
     */
    @Test
    public void testonDataChanged() {
        portDataChangeListener.onDataChanged(asyncDataChangeEvent);
        /**
         * Verifying asyncDataChangeEventMockObj object invoking both
         * getCreatedData and getUpdatedData methods.
         */
        verify(asyncDataChangeEvent , times(1)).getCreatedData();
        verify(asyncDataChangeEvent , times(1)).getRemovedPaths();
    }
}
