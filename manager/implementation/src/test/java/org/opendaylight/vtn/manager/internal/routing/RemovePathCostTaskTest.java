/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Assert;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import java.util.List;
import java.util.ArrayList;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.VTNException;

import java.util.NoSuchElementException;

/**
 * JUnit test for {@link RemovePathCostTask}
 */
public class RemovePathCostTaskTest {
    /**
      * Static instance of RemovePathCostTask to perform unit testing.
      */
    private static RemovePathCostTask removePathCostTask;
    /**
      * Static instance of TopologyGraph to perform unit testing.
      */
    private static TopologyGraph topologyGraph;
    /**
      * Static instance of VTNManagerProvider to perform unit testing.
      */
    private static VTNManagerProvider vtnManagerProvider;
    /**
      * Static instance of VTNManagerImpl to perform unit testing.
      */
    private static VTNManagerImpl vtnManagerImpl;
    /**
      * Static instance of RemovePathCostInput to perform unit testing.
      */
    private static RemovePathCostInput removePathCostInput;
    /**
      * Static instance of RemovePathCostInputBuilder to perform unit testing.
      */
    private static RemovePathCostInputBuilder removePathCostInputBuilder;
    /**
      * Static instance of VtnPortDesc to perform unit testing.
      */
    private static VtnPortDesc vtnPortDesc, vtnPortDesc2;
    /**
      * Static instance of VtnUpdateType to perform unit testing.
      */
    private static VtnUpdateType vtnUpdateType, vtnUpdateType2, vtnUpdateType3;
    /**
      * Static instance of TxContext to perform unit testing.
      */
    private static TxContext txContext;

    /**
     * This method creates the required objects to perform unit testing.
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        vtnManagerImpl = new VTNManagerImpl();
        vtnManagerProvider = vtnManagerImpl.getVTNProvider();
        topologyGraph = new TopologyGraph(vtnManagerProvider);
        vtnPortDesc = new VtnPortDesc("openflow:1,,");
        removePathCostInputBuilder = new RemovePathCostInputBuilder();
        removePathCostInputBuilder.setId(new Integer(10));
        List<VtnPortDesc> list = new ArrayList<VtnPortDesc>();
        list.add(vtnPortDesc);
        removePathCostInputBuilder.setPortDesc(list);
        removePathCostInput = removePathCostInputBuilder.build();
        vtnUpdateType = VtnUpdateType.REMOVED;
        vtnUpdateType2 = VtnUpdateType.CREATED;
        vtnUpdateType3 = VtnUpdateType.CHANGED;
    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {
        removePathCostInput = null;
        removePathCostInputBuilder = null;
        vtnPortDesc = null;
        topologyGraph = null;
        vtnManagerProvider = null;
        vtnManagerImpl = null;
        vtnUpdateType = null;
        vtnUpdateType2 = null;
        vtnUpdateType3 = null;
    }

    /**
     * Test method for
     * {@link RemovePathCostTask#create(TopologyGraph, RemovePathCostInput)}.
     */
    @Test
    public void testCreate() {
        try {
            Assert.assertTrue(RemovePathCostTask.create(topologyGraph, removePathCostInput) instanceof RemovePathCostTask);
        } catch (Exception ex) {
            Assert.assertFalse(ex instanceof RpcException);
        }

        try {
            RemovePathCostTask.create(topologyGraph, null);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);

        }

        try {
            removePathCostInputBuilder.setId(null);
            RemovePathCostTask.create(topologyGraph, removePathCostInputBuilder.build());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);
        }

        try {
            removePathCostInputBuilder.setId(new Integer(10));
            removePathCostInputBuilder.setPortDesc(null);
            RemovePathCostTask.create(topologyGraph, removePathCostInputBuilder.build());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);
        }

        try {
            removePathCostInputBuilder.setId(new Integer(10));
            List<VtnPortDesc> list = new ArrayList<VtnPortDesc>();
            list.add(null);
            removePathCostInputBuilder.setPortDesc(list);
            RemovePathCostTask.create(topologyGraph, removePathCostInputBuilder.build());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);
        }
        try {
            removePathCostInputBuilder.setId(new Integer(10));
            removePathCostInputBuilder.setPortDesc(new ArrayList());
            RemovePathCostTask.create(topologyGraph, removePathCostInputBuilder.build());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);
        }
    }

    /**
     * Test method for
     * {@link RemovePathCostTask#onStarted(TxContext)}.
     */
    @Test
    public void testOnStarted() {
        try {
            removePathCostTask.onStarted(txContext);
        } catch (Exception ex) {
            Assert.assertFalse(ex instanceof VTNException);
        }
    }

    /**
     * Test method for
     * {@link RemovePathCostTask#onSuccess(VTNManagerProvider, List)}.
     */
    @Test
    public void testOnSuccess() {
        List<VtnUpdateType> list = new ArrayList<VtnUpdateType>();
        list.add(vtnUpdateType);
        list.add(vtnUpdateType2);
        list.add(vtnUpdateType3);

        try {
            removePathCostTask = RemovePathCostTask.create(topologyGraph, removePathCostInput);
            removePathCostTask.onSuccess(vtnManagerProvider, list);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
        try {
            removePathCostTask = RemovePathCostTask.create(topologyGraph, removePathCostInput);
            List<VtnUpdateType> list2 = new ArrayList<VtnUpdateType>();
            list2.add(null);
            removePathCostTask.onSuccess(vtnManagerProvider, list2);
        } catch (Exception ex) {
            Assert.assertEquals(ex, null);
        }
    }

    /**
     * Test method for
     * {@link RemovePathCostTask#getOutputType()}.
     */
    @Test
    public void testGetOutputType() {
        try {
            removePathCostTask = RemovePathCostTask.create(topologyGraph, removePathCostInput);
            Assert.assertEquals(RemovePathCostOutput.class, removePathCostTask.getOutputType());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
    }

    /**
     * Test method for
     * {@link RemovePathCostTask#createOutput(List)}.
     */
    @Test
    public void testCreateOutput() {
        try {
            removePathCostTask = RemovePathCostTask.create(topologyGraph, removePathCostInput);
            List<VtnUpdateType> list = new ArrayList<VtnUpdateType>();
            list.add(vtnUpdateType);
            list.add(vtnUpdateType2);
            list.add(vtnUpdateType3);
            Assert.assertTrue(removePathCostTask.createOutput(list) instanceof RemovePathCostOutput);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NoSuchElementException);
        }
    }
}

