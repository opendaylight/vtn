/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.junit.Test;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.mockito.Mockito;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;


/**
 * JUnit test for {@link NodeConnectorListener}
 *
 * NodeConnectorListener test class is to test Listener class listens
 * the change of MD-SAL node connectors or not.
 */

public class NodeConnectorListenerTest extends TestBase{
    /**
     * Logger instance.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(NodeConnectorListenerTest.class);
    /**
     * create a mock object for DataBroker class
     */
    DataBroker broker = Mockito.mock(DataBroker.class);
    /**
     * create a mock object for VTNManagerProvider class
     */
    VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
    /**
     * create a mock object for DataObject class
     */
    DataObject dataObject = Mockito.mock(DataObject.class);
    /**
     * create a mock object for FlowCapableNodeConnector class
     */
    FlowCapableNodeConnector fCapableNC = Mockito.mock(FlowCapableNodeConnector.class);
    /**
     * create a mock object for AsyncDataChangeEvent class
     */
    AsyncDataChangeEvent asyncDataChangeEvent = Mockito.mock(AsyncDataChangeEvent.class);
    /**
     * create a object for TxQueue
     */
    TxQueue txQueue = new TxQueueImpl("QUEUE1", provider);
    /**
     * create a object for NodeConnectorListener
     */
    NodeConnectorListener nodeConnectorListener = new NodeConnectorListener(txQueue, broker);
    /**
     * create a object for PortUpdateTask
     */
    PortUpdateTask portUpdateTask = new PortUpdateTask(LOG);
    /**
     * create a object for InstanceIdentifier
     */
    InstanceIdentifier<FlowCapableNodeConnector> data = InstanceIdentifier.builder(Nodes.class).child(Node.class).child(NodeConnector.class).augmentation(FlowCapableNodeConnector.class).build();

    Long nodeId = 100L;
    Long portId = 200L;
    /**
     * create a object for SalPort
     */
    SalPort salPort = new SalPort(nodeId, portId, "port address");

    /**
     * Test method for
     * {@link NodeConnectorListener#getLogger}.
     */
    @Test
    public void testGetLogger() {

        assertTrue(nodeConnectorListener.getLogger() instanceof Logger);
    }

    /**
     * Test method for
     * {@link NodeConnectorListener#onRemoved}.
     */
    @Test
    public void testOnRemoved() {
        try {
            portUpdateTask.addUpdated(data, salPort);
            IdentifiedData identifiedData = new IdentifiedData(data, fCapableNC);
            nodeConnectorListener.onRemoved(portUpdateTask, identifiedData);
        } catch (Exception e) {
            assertTrue(e instanceof ClassCastException);
        }
    }

    /**
     * Test method for
     * {@link NodeConnectorListener#enterEvent}.
     */
    @Test
    public void testEnterEvent() {
        try {
            assertTrue(nodeConnectorListener.enterEvent(asyncDataChangeEvent) instanceof PortUpdateTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

    /**
     * Test method for
     * {@link NodeConnectorListener#onUpdated}.
     */
    @Test
    public void testOnUpdated() {
        try {
            portUpdateTask.addUpdated(data, salPort);
            ChangedData changedData = new ChangedData(data, fCapableNC, fCapableNC);
            nodeConnectorListener.onUpdated(portUpdateTask, changedData);
        } catch (Exception e) {
            assertTrue(e instanceof ClassCastException);
        }
    }

    /**
     * Test method for
     * {@link NodeConnectorListener#onCreate}.
     */
    @Test
    public void testOnCretaed() {

        try {
            portUpdateTask.addUpdated(data, salPort);
            IdentifiedData identifiedData = new IdentifiedData(data, fCapableNC);
            nodeConnectorListener.onCreated(portUpdateTask, identifiedData);
        } catch (Exception e) {
            assertTrue(e instanceof ClassCastException);
        }
    }

    /**
     * Test method for
     * {@link NodeConnectorListener#exitEvent}.
     */
    @Test
    public void testExitEvent() {

        try {
            nodeConnectorListener.exitEvent(portUpdateTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

        try {
            portUpdateTask.addUpdated(data, salPort);
            nodeConnectorListener.exitEvent(portUpdateTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

    }

    /**
     * Test method for
     * {@link NodeConnectorListener#getWildcardPath}.
     */
    @Test
    public void testGetWildcardPath() {

        nodeConnectorListener.getWildcardPath();

    }
}
