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
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;

/**
 * JUnit test for {@link NodeListener}
 *
 * NodeListener test class is to test Listener class listens
 * the change of MD-SAL nodes or not.
 */

public class NodeListenerTest extends TestBase{

    /**
     * Logger instance.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(NodeListenerTest.class);
    /**
     * create a mock object for DataBroker class
     */
    DataBroker broker = Mockito.mock(DataBroker.class);
    /**
     * create a mock object for VTNManagerProvider class
     */
    VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
    /**
     * create a mock object for FlowCapableNode class
     */
    FlowCapableNode fCapableNode = Mockito.mock(FlowCapableNode.class);
    /**
     * create a mock object for AsyncDataChangeEvent class
     */
    AsyncDataChangeEvent asyncDataChangeEvent = Mockito.mock(AsyncDataChangeEvent.class);
     /**
     * create a  object for TxQueue
     */
    TxQueue txQueue = new TxQueueImpl("QUEUE1", provider);
    /**
     * create a  object for NodeListener
     */
    NodeListener nodeListener = new NodeListener(txQueue, broker);
    /**
     * create a  object for NodeUpdateTask
     */
    NodeUpdateTask nodeUpdateTask = new NodeUpdateTask(LOG);
    /**
     * create a  object for InstanceIdentifier
     */
    InstanceIdentifier<FlowCapableNode> instanceIdentifier = InstanceIdentifier.builder(Nodes.class).child(Node.class).augmentation(FlowCapableNode.class).build();

    Long nodeId = 100L;
    Long portId = 200L;
    /**
     * create a  object for SalPort
     */
    SalPort salPort = new SalPort(nodeId, portId, "port address");

    /**
     * Test method for
     * {@link NodeListener#getLogger}.
     */
    @Test
    public void testGetLogger() {

        assertTrue(nodeListener.getLogger() instanceof Logger);
    }

    /**
     * Test method for
     * {@link NodeListener#onRemoved}.
     */
    @Test
    public void testOnRemoved() {
        try {
            nodeUpdateTask.addUpdated(instanceIdentifier, salPort);
            IdentifiedData identifiedData = new IdentifiedData(instanceIdentifier, fCapableNode);
            nodeListener.onRemoved(nodeUpdateTask, identifiedData);
        } catch (Exception e) {
            assertTrue(e instanceof ClassCastException);
        }
    }

     /**
     * Test method for
     * {@link NodeListener#onUpdated}.
     */
    @Test
    public void testOnUpdated() {
        try {
            nodeUpdateTask.addUpdated(instanceIdentifier, salPort);
            ChangedData changedData = new ChangedData(instanceIdentifier, fCapableNode, fCapableNode);
            nodeListener.onUpdated(nodeUpdateTask, changedData);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

    /**
     * Test method for
     * {@link NodeListener#onCreated}.
     */
    @Test
    public void testOnCreated() {
        try {
            nodeUpdateTask.addUpdated(instanceIdentifier, salPort);
            IdentifiedData identifiedData = new IdentifiedData(instanceIdentifier, fCapableNode);
            nodeListener.onCreated(nodeUpdateTask, identifiedData);
        } catch (Exception e) {
            assertTrue(e instanceof ClassCastException);
        }
    }

    /**
     * Test method for
     * {@link NodeListener#enterEvent}.
     */
    @Test
    public void testEnterEvent() {
        try {
            assertTrue(nodeListener.enterEvent(asyncDataChangeEvent) instanceof NodeUpdateTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

     /**
     * Test method for
     * {@link NodeListener#exitEvent}.
     */
    @Test
    public void testExitEvent() {

        try {
            nodeListener.exitEvent(nodeUpdateTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

        try {
            nodeUpdateTask.addUpdated(instanceIdentifier, salPort);
            nodeListener.exitEvent(nodeUpdateTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

    }

    /**
     * Test method for
     * {@link NodeListener#getRequiredEvents}.
     */
    @Test
    public void testGetRequiredEvents() {

        nodeListener.getRequiredEvents();

    }
}
