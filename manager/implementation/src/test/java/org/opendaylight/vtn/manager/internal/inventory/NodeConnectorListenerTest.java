/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

/**
 * JUnit test for {@link NodeConnectorListener}.
 */
public class NodeConnectorListenerTest extends TestBase{
    /**
     * Mock-up of {@link TxQueue}.
     */
    private TxQueue  txQueue;

    /**
     * Mock-up of {@link DataBroker}.
     */
    private DataBroker  dataBroker;

    /**
     * Mock-up of {@link ListenerRegistration}.
     */
    private ListenerRegistration  registration;

    /**
     * A {@link NodeConnectorListener} instance for test.
     */
    private NodeConnectorListener  ncListener;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        dataBroker = mock(DataBroker.class);
        txQueue = mock(TxQueue.class);
        registration = mock(ListenerRegistration.class);

        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 any(NodeConnectorListener.class), any(DataChangeScope.class))).
            thenReturn(registration);
        ncListener = new NodeConnectorListener(txQueue, dataBroker);
    }

    /**
     * Test case for
     * {@link NodeConnectorListener#NodeConnectorListener(TxQueue, DataBroker)}.
     */
    @Test
    public void testConstructor() {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        InstanceIdentifier<FlowCapableNodeConnector> path = getPath();
        verify(dataBroker).
            registerDataChangeListener(oper, path, ncListener, scope);
        verifyZeroInteractions(registration);
        verifyZeroInteractions(txQueue);
        assertEquals(FlowCapableNodeConnector.class,
                     ncListener.getTargetType());
    }

    /**
     * Test case for
     * {@link NodeConnectorListener#enterEvent(AsyncDataChangeEvent)}.
     */
    @Test
    public void testEnterEvent() {
        AsyncDataChangeEvent ev = null;
        PortUpdateTask task = ncListener.enterEvent(ev);
        assertTrue(task instanceof PortUpdateTask);
        PortUpdateTask task1 = ncListener.enterEvent(ev);
        assertTrue(task1 instanceof PortUpdateTask);
        assertNotSame(task, task1);
    }


    /**
     * Test case for {@link NodeConnectorListener#exitEvent(PortUpdateTask)}.
     */
    @Test
    public void testExitEvent() {
        reset(txQueue);
        Logger logger = mock(Logger.class);
        PortUpdateTask task = new PortUpdateTask(logger);
        ncListener.exitEvent(task);
        verifyZeroInteractions(txQueue);

        SalPort sport = new SalPort(123L, 45L);
        InstanceIdentifier<FlowCapableNodeConnector> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, sport.getNodeKey()).
            child(NodeConnector.class, sport.getNodeConnectorKey()).
            augmentation(FlowCapableNodeConnector.class).
            build();
        task.addUpdated(path, sport);
        ncListener.exitEvent(task);
        verify(txQueue, never()).postFirst(any(TxTask.class));
        verify(txQueue).post(task);
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for
     * {@link NodeConnectorListener#onCreated(PortUpdateTask,IdentifiedData)}.
     */
    @Test
    public void testOnCreated() {
        reset(txQueue);
        FlowCapableNodeConnector fcnc = mock(FlowCapableNodeConnector.class);

        Logger logger = mock(Logger.class);
        PortUpdateTask task = new PortUpdateTask(logger);
        assertEquals(false, task.hasUpdates());

        // In case of unsupported port.
        NodeId unsupportedNode = new NodeId("unknown:1");
        NodeConnectorId unsupportedPort = new NodeConnectorId("unknown:1:2");
        InstanceIdentifier<FlowCapableNodeConnector> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(unsupportedNode)).
            child(NodeConnector.class, new NodeConnectorKey(unsupportedPort)).
            augmentation(FlowCapableNodeConnector.class).
            build();
        IdentifiedData<FlowCapableNodeConnector> data =
            new IdentifiedData<>(path, fcnc);
        ncListener.onCreated(task, data);
        assertEquals(false, task.hasUpdates());

        // In case of OpenFlow logical port.
        NodeId nid = new NodeId("openflow:1");
        NodeConnectorId logical = new NodeConnectorId("openflow:1:CONTROLLER");
        path = InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(nid)).
            child(NodeConnector.class, new NodeConnectorKey(logical)).
            augmentation(FlowCapableNodeConnector.class).
            build();
        data = new IdentifiedData<>(path, fcnc);
        ncListener.onCreated(task, data);
        assertEquals(false, task.hasUpdates());

        // In case of OpenFlow physical port.
        SalPort[] ports = {
            new SalPort(1L, 1L),
            new SalPort(-1L, 0xffffff00L),
            new SalPort(123L, 456L),
        };
        Map<SalPort, InstanceIdentifier<FlowCapableNodeConnector>> updated =
            new HashMap<>();
        for (SalPort sport: ports) {
            path = InstanceIdentifier.builder(Nodes.class).
                child(Node.class, sport.getNodeKey()).
                child(NodeConnector.class, sport.getNodeConnectorKey()).
                augmentation(FlowCapableNodeConnector.class).
                build();
            assertEquals(null, updated.put(sport, path));
            data = new IdentifiedData<>(path, fcnc);
            ncListener.onCreated(task, data);
            assertEquals(true, task.hasUpdates());
        }

        verifyZeroInteractions(txQueue);
        assertEquals(ports.length, updated.size());
        assertEquals(updated, task.getUpdatedMap());
    }

    /**
     * Test case for
     * {@link NodeConnectorListener#onUpdated(PortUpdateTask,ChangedData)}.
     */
    @Test
    public void testOnUpdated() {
        reset(txQueue);
        FlowCapableNodeConnector fcnc1 = mock(FlowCapableNodeConnector.class);
        FlowCapableNodeConnector fcnc2 = mock(FlowCapableNodeConnector.class);

        Logger logger = mock(Logger.class);
        PortUpdateTask task = new PortUpdateTask(logger);
        assertEquals(false, task.hasUpdates());

        // In case of unsupported port.
        NodeId unsupportedNode = new NodeId("unknown:1");
        NodeConnectorId unsupportedPort = new NodeConnectorId("unknown:1:2");
        InstanceIdentifier<FlowCapableNodeConnector> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(unsupportedNode)).
            child(NodeConnector.class, new NodeConnectorKey(unsupportedPort)).
            augmentation(FlowCapableNodeConnector.class).
            build();
        ChangedData<FlowCapableNodeConnector> data =
            new ChangedData<>(path, fcnc1, fcnc2);
        ncListener.onUpdated(task, data);
        assertEquals(false, task.hasUpdates());

        // In case of OpenFlow logical port.
        NodeId nid = new NodeId("openflow:1");
        NodeConnectorId logical = new NodeConnectorId("openflow:1:CONTROLLER");
        path = InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(nid)).
            child(NodeConnector.class, new NodeConnectorKey(logical)).
            augmentation(FlowCapableNodeConnector.class).
            build();
        data = new ChangedData<>(path, fcnc1, fcnc2);
        ncListener.onUpdated(task, data);
        assertEquals(false, task.hasUpdates());

        // In case of OpenFlow physical port.
        SalPort[] ports = {
            new SalPort(1L, 1L),
            new SalPort(-1L, 0xffffff00L),
            new SalPort(123L, 456L),
        };
        Map<SalPort, InstanceIdentifier<FlowCapableNodeConnector>> updated =
            new HashMap<>();
        for (SalPort sport: ports) {
            path = InstanceIdentifier.builder(Nodes.class).
                child(Node.class, sport.getNodeKey()).
                child(NodeConnector.class, sport.getNodeConnectorKey()).
                augmentation(FlowCapableNodeConnector.class).
                build();
            assertEquals(null, updated.put(sport, path));
            data = new ChangedData<>(path, fcnc1, fcnc2);
            ncListener.onUpdated(task, data);
            assertEquals(true, task.hasUpdates());
        }

        verifyZeroInteractions(txQueue);
        assertEquals(ports.length, updated.size());
        assertEquals(updated, task.getUpdatedMap());
    }

    /**
     * Test case for
     * {@link NodeConnectorListener#onRemoved(PortUpdateTask,IdentifiedData)}.
     */
    @Test
    public void testOnRemoved() {
        reset(txQueue);
        FlowCapableNodeConnector fcnc = mock(FlowCapableNodeConnector.class);

        Logger logger = mock(Logger.class);
        PortUpdateTask task = new PortUpdateTask(logger);
        assertEquals(false, task.hasUpdates());

        // In case of unsupported port.
        NodeId unsupportedNode = new NodeId("unknown:1");
        NodeConnectorId unsupportedPort = new NodeConnectorId("unknown:1:2");
        InstanceIdentifier<FlowCapableNodeConnector> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(unsupportedNode)).
            child(NodeConnector.class, new NodeConnectorKey(unsupportedPort)).
            augmentation(FlowCapableNodeConnector.class).
            build();
        IdentifiedData<FlowCapableNodeConnector> data =
            new IdentifiedData<>(path, fcnc);
        ncListener.onRemoved(task, data);
        assertEquals(false, task.hasUpdates());

        // In case of OpenFlow logical port.
        NodeId nid = new NodeId("openflow:1");
        NodeConnectorId logical = new NodeConnectorId("openflow:1:CONTROLLER");
        path = InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(nid)).
            child(NodeConnector.class, new NodeConnectorKey(logical)).
            augmentation(FlowCapableNodeConnector.class).
            build();
        data = new IdentifiedData<>(path, fcnc);
        ncListener.onRemoved(task, data);
        assertEquals(false, task.hasUpdates());

        // In case of OpenFlow physical port.
        SalPort[] ports = {
            new SalPort(1L, 1L),
            new SalPort(-1L, 0xffffff00L),
            new SalPort(123L, 456L),
        };
        Map<SalPort, InstanceIdentifier<FlowCapableNodeConnector>> updated =
            new HashMap<>();
        for (SalPort sport: ports) {
            path = InstanceIdentifier.builder(Nodes.class).
                child(Node.class, sport.getNodeKey()).
                child(NodeConnector.class, sport.getNodeConnectorKey()).
                augmentation(FlowCapableNodeConnector.class).
                build();
            assertEquals(null, updated.put(sport, path));
            data = new IdentifiedData<>(path, fcnc);
            ncListener.onRemoved(task, data);
            assertEquals(true, task.hasUpdates());
        }

        verifyZeroInteractions(txQueue);
        assertEquals(ports.length, updated.size());
        assertEquals(updated, task.getUpdatedMap());
    }

    /**
     * Test case for {@link NodeConnectorListener#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        assertEquals(getPath(), ncListener.getWildcardPath());
    }

    /**
     * Test case for {@link NodeConnectorListener#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Logger logger = ncListener.getLogger();
        assertEquals(NodeConnectorListener.class.getName(), logger.getName());
    }

    /**
     * Test case for {@link NodeConnectorListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(registration);
        ncListener.close();
        verify(registration).close();
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<FlowCapableNodeConnector> getPath() {
        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class).
            child(NodeConnector.class).
            augmentation(FlowCapableNodeConnector.class).
            build();
    }
}
