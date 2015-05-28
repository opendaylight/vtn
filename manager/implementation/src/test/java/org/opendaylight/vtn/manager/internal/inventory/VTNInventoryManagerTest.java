/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.inventory;

import org.slf4j.Logger;

import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;

/**
 * JUnit test for {@link VTNInventoryManager}.
 */
public class VTNInventoryManagerTest {
    /**
     * Static instance of VTNInventoryManager to perform unit testing.
     */
    private static VTNInventoryManager vtnInventoryManager;

    /**
     * Static instance of VTNManagerProvider to perform unit testing.
     */
    private static VTNManagerProvider vtnManagerProvider;

    /**
     * Static instance of VTNInventoryListener to perform unit testing.
     */
    private static VTNInventoryListener vtnInventoryListener;

    /**
     * This method creates the requird objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        vtnManagerProvider = Mockito.mock(VTNManagerProvider.class);
        Mockito.when(vtnManagerProvider.getDataBroker()).thenReturn(Mockito.mock(DataBroker.class));
        vtnInventoryManager = new VTNInventoryManager(vtnManagerProvider);
        vtnInventoryListener = new VTNManagerImpl();
        try {
            VTNManagerProvider vtnManagerProvider2 = Mockito.mock(VTNManagerProvider.class);
            Mockito.when(vtnManagerProvider.getDataBroker()).thenThrow(new RuntimeException());
            VTNInventoryManager vtnInventoryManager2 = new VTNInventoryManager(vtnManagerProvider2);
            VTNInventoryListener vtnInventoryListener2 = new VTNManagerImpl();
        } catch (Exception exception) {
            Assert.assertNotNull(exception instanceof IllegalStateException);
        }
    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {
        vtnInventoryListener = null;
        vtnInventoryManager = null;
        vtnManagerProvider = null;
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#addListener(VTNInventoryListener)}.
     */
    @Test
    public void testAddListener() {
        try {
            vtnInventoryManager.addListener(vtnInventoryListener);
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#isAlive()}.
     */
    @Test
    public void testIsAlive() {
        Boolean result = vtnInventoryManager.isAlive();
        Assert.assertTrue(result instanceof Boolean);
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#shutdown()}.
     */
    @Test
    public void testShutdown() {
        try {
            vtnInventoryManager.shutdown();
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#close()}.
     */
    @Test
    public void testClose() {
        try {
            vtnInventoryManager.close();
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#getComparator()}.
     */
    @Test
    public void testGetComparator() {
        IdentifierTargetComparator identifierTargetComparator = vtnInventoryManager.getComparator();
        Assert.assertTrue(identifierTargetComparator instanceof IdentifierTargetComparator);
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#getOrder(VtnUpdateType)}.
     */
    @Test
    public void testGetOrder() {
        Assert.assertTrue(vtnInventoryManager.getOrder(VtnUpdateType.CREATED));
        Assert.assertTrue(vtnInventoryManager.getOrder(VtnUpdateType.CHANGED));
        Assert.assertFalse(vtnInventoryManager.getOrder(VtnUpdateType.REMOVED));
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#enterEvent(AsyncDataChangeEvent)}.
     */
    @Test
    public void testEnterEvent() {
        Assert.assertNull(vtnInventoryManager.enterEvent(Mockito.mock(AsyncDataChangeEvent.class)));
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#exitEvent(Void)}.
     */
    @Test
    public void testExitEvent() {
        try {
            vtnInventoryManager.exitEvent(null);
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#onCreated(Void,IdentifiedData)}.
     */
    @Test
    public void testOnCreated() {
        try {
            vtnInventoryManager.onCreated(null, Mockito.mock(IdentifiedData.class));
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#onUpdated(Void,ChangedData)}.
     */
    @Test
    public void testOnUpdated() {
        try {
            InstanceIdentifier<FlowCapableNodeConnector> instanceIdentifier = InstanceIdentifier.builder(Nodes.class).child(Node.class).child(NodeConnector.class)
                .augmentation(FlowCapableNodeConnector.class).build();
            ChangedData changedData = new ChangedData(instanceIdentifier, Mockito.mock(FlowCapableNodeConnector.class), Mockito.mock(FlowCapableNodeConnector.class));
            vtnInventoryManager.onUpdated(null, changedData);
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed................... ");
        }
        try {
            InstanceIdentifier<VtnNode> instanceIdentifier = InstanceIdentifier.builder(VtnNodes.class).child(VtnNode.class).build();
            ChangedData changedData = new ChangedData(instanceIdentifier, Mockito.mock(VtnNode.class), Mockito.mock(VtnNode.class));
            vtnInventoryManager.onUpdated(null, changedData);
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed................... ");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#onRemoved(Void,IdentifiedData)}.
     */
    @Test
    public void testOnRemoved() {
        try {
            vtnInventoryManager.onRemoved(null, Mockito.mock(IdentifiedData.class));
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        try {
            Assert.assertTrue(vtnInventoryManager.getWildcardPath() instanceof InstanceIdentifier);
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNInventoryManager#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        try {
            Assert.assertTrue(vtnInventoryManager.getLogger() instanceof Logger);
        } catch (Exception exception) {
            Assert.fail("VTNInventoryManager test case failed...................");
        }
    }
}
