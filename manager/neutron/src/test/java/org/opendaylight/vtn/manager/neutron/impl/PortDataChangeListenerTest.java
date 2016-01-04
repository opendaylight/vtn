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
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;
import static org.mockito.Mockito.times;
import static org.powermock.api.mockito.PowerMockito.mock;

import java.util.HashMap;
import java.util.Map;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.api.mockito.PowerMockito;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * JUnit test for {@link PortDataChangeListener}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({VTNManagerService.class, MdsalUtils.class, InstanceIdentifier.class, OfNode.class, PortDataChangeListener.class})
public class PortDataChangeListenerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
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
     * VTNManagerService instance.
     */
    private VTNManagerService  vtnManagerService;
    /**
     * Set up test environment.
     */
    /**
     * AsyncDataChangeEvent object reference for unit testing.
     */
    private AsyncDataChangeEvent asyncDataChangeEventMockObj;
    /**
     * Collection of InstanceIdentifier and Intent.
     */
    private Map<InstanceIdentifier<?>, DataObject> portMap;
    /**
     * Intent object reference for unit testing.
     */
    private Port port;
    /**
     * InstanceIdentifier object reference for unit testing.
     */
    private InstanceIdentifier<?> instanceIdentifier;
    @Before
    public void setUp() throws Exception {
        initMocks(this);
        vtnManagerService = PowerMockito.mock(VTNManagerService.class);
        portDataChangeListener = PowerMockito.spy(new PortDataChangeListener(dataBroker , vtnManagerService));
        asyncDataChangeEventMockObj = mock(AsyncDataChangeEvent.class);
        portMap = new HashMap<InstanceIdentifier<?>,  DataObject>();
        when(asyncDataChangeEventMockObj.getCreatedData())
                .thenReturn(portMap);
        when(asyncDataChangeEventMockObj.getUpdatedData())
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

        // Ensure that PortDataChangeListener has been registered as data change
        // listener.
        verifyZeroInteractions(listenerReg);
    }
    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Port> getPath() {
        return InstanceIdentifier.create(Neutron.class).
                child(Ports.class).
                child(Port.class);
    }

    /**
     * Test case for {@link neutronNetworkListener#close()}.
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
        portDataChangeListener.onDataChanged(asyncDataChangeEventMockObj);
        /**
         * Verifying asyncDataChangeEventMockObj object invoking both
         * getCreatedData and getUpdatedData methods.
         */
        verify(asyncDataChangeEventMockObj , times(1)).getCreatedData();
    }
}
