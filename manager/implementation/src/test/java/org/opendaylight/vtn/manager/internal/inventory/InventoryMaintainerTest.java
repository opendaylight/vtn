/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.Mock;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * JUnit test for {@link InventoryMaintainer}
 */
public class InventoryMaintainerTest extends TestBase {
    /**
     * Mock-up of {@link TxQueue}.
     */
    @Mock
    private TxQueue  txQueue;

    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Mock-up of {@link ListenerRegistration}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  registration;

    /**
     * Mock-up of {@link Logger}.
     */
    @Mock
    private Logger  logger;

    /**
     * A {@link NodeConnectorListener} instance for test.
     */
    private NodeConnectorListener  ncListener;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);

        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 any(NodeConnectorListener.class), any(DataChangeScope.class))).
            thenReturn(registration);
        ncListener = new NodeConnectorListener(txQueue, dataBroker);
    }

    /**
     * Test case for {@link InventoryMaintainer#submit(TxTask)}.
     */
    @Test
    public void testSubmit() {
        TxTask<?> task = new PortUpdateTask(logger);
        ncListener.submit(task);
        verify(txQueue).post(task);
        verifyNoMoreInteractions(txQueue, logger);
    }

    /**
     * Test case for {@link InventoryMaintainer#submitInitial(TxTask)}.
     */
    @Test
    public void testSubmitInitial() {
        TxTask<?> task = new PortUpdateTask(logger);
        ncListener.submitInitial(task);
        verify(txQueue).postFirst(task);
        verifyNoMoreInteractions(txQueue, logger);
    }
}
