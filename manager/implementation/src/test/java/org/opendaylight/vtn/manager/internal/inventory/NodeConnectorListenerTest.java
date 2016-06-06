/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.isA;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.Mock;

import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnectorBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortNumberUni;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.State;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.StateBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.queues.Queue;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link NodeConnectorListener}.
 */
public class NodeConnectorListenerTest extends TestBase{
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
    private ListenerRegistration<DataTreeChangeListener<FlowCapableNodeConnector>>  registration;

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

        Class<DataTreeIdentifier> idtype = DataTreeIdentifier.class;
        Class<DataTreeChangeListener> ltype = DataTreeChangeListener.class;
        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<FlowCapableNodeConnector>)any(idtype),
                 (DataTreeChangeListener<FlowCapableNodeConnector>)isA(ltype))).
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
        DataTreeIdentifier<FlowCapableNodeConnector> ident =
            new DataTreeIdentifier<>(oper, getPath());
        verify(dataBroker).registerDataTreeChangeListener(ident, ncListener);
        verifyZeroInteractions(registration);
        verifyZeroInteractions(txQueue);
        assertEquals(FlowCapableNodeConnector.class,
                     ncListener.getTargetType());
    }

    /**
     * Test case for
     * {@link NodeConnectorListener#enterEvent()}.
     */
    @Test
    public void testEnterEvent() {
        PortUpdateTask task = ncListener.enterEvent();
        assertTrue(task instanceof PortUpdateTask);
        PortUpdateTask task1 = ncListener.enterEvent();
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
        verify(txQueue).post(task);
        verifyNoMoreInteractions(txQueue, logger);
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
     * Test case for
     * {@link NodeConnectorListener#isUpdated(FlowCapableNodeConnector,FlowCapableNodeConnector)}.
     */
    @Test
    public void testIsUpdated() {
        FlowCapableNodeConnectorBuilder builder =
            new FlowCapableNodeConnectorBuilder();
        FlowCapableNodeConnector old = builder.build();
        FlowCapableNodeConnector fcnc = builder.build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));

        // Change name.
        String[] names = {"port-1", "port-2", "eth3"};
        for (String name: names) {
            fcnc = builder.setName(name).build();
            assertEquals(true, ncListener.isUpdated(old, fcnc));
            old = builder.build();
            assertEquals(false, ncListener.isUpdated(old, fcnc));
        }

        // Change configuation.
        PortConfig[] portConfigs = {
            new PortConfig(false, false, false, false),
            new PortConfig(true, false, false, false),
            new PortConfig(true, true, false, false),
            new PortConfig(false, true, false, true),
        };
        for (PortConfig pc: portConfigs) {
            fcnc = builder.setConfiguration(pc).build();
            assertEquals(true, ncListener.isUpdated(old, fcnc));
            old = builder.build();
            assertEquals(false, ncListener.isUpdated(old, fcnc));
        }

        // Change state.
        StateBuilder stb = new StateBuilder();
        State[] portStates = {
            stb.build(),
            stb.setLinkDown(true).build(),
            stb.setBlocked(true).build(),
            stb.setLinkDown(false).setLive(true).build(),
        };
        for (State st: portStates) {
            fcnc = builder.setState(st).build();
            assertEquals(true, ncListener.isUpdated(old, fcnc));
            old = builder.build();
            assertEquals(false, ncListener.isUpdated(old, fcnc));
        }

        // Change current-feature.
        PortFeatures[] features = {
            new PortFeatures(true, false, true, false, true, false, false,
                             false, false, false, false, false, false, false,
                             false, false),
            new PortFeatures(false, false, true, false, true, false, false,
                             false, false, false, false, false, false, false,
                             false, true),
            new PortFeatures(true, false, true, false, true, false, false,
                             false, true, false, false, false, false, false,
                             false, true),
        };
        for (PortFeatures feature: features) {
            fcnc = builder.setCurrentFeature(feature).build();
            assertEquals(true, ncListener.isUpdated(old, fcnc));
            old = builder.build();
            assertEquals(false, ncListener.isUpdated(old, fcnc));
        }

        // Change current-speed.
        Long[] speeds = {1000L, 10000L, 100000L};
        for (Long speed: speeds) {
            fcnc = builder.setCurrentSpeed(speed).build();
            assertEquals(true, ncListener.isUpdated(old, fcnc));
            old = builder.build();
            assertEquals(false, ncListener.isUpdated(old, fcnc));
        }

        // Other fields should be ignored.
        fcnc = builder.setAdvertisedFeatures(features[0]).build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
        fcnc = builder.setHardwareAddress(new MacAddress("00:11:22:33:44:55")).
            build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
        fcnc = builder.setMaximumSpeed(10000000L).build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
        fcnc = builder.setPeerFeatures(features[1]).build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
        fcnc = builder.setPortNumber(new PortNumberUni(123L)).build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
        fcnc = builder.setQueue(Collections.<Queue>emptyList()).build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
        fcnc = builder.setSupported(features[2]).build();
        assertEquals(false, ncListener.isUpdated(old, fcnc));
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
