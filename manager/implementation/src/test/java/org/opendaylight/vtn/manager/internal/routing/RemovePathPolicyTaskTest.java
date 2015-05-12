/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.VTNException;

/**
 * JUnit test for {@link RemovePathPolicyTask}
 */
public class RemovePathPolicyTaskTest {

    /**
      * Static instance of RemovePolicyTask to perform unit testing.
      */
    private static RemovePathPolicyTask removePathPolicyTask;
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
      * Static instance of RemovePathPolicyInput to perform unit testing.
      */
    private static RemovePathPolicyInput removePathPolicyInput;
    /**
      * Static instance of RemovePathPolicyInputBuilder to perform unit testing.
      */
    private static RemovePathPolicyInputBuilder removePathPolicyInputBuilder;
    /**
      * Static instance of VtnPathPolicy to perform unit testing.
      */
    private static VtnPathPolicy vtnPathPolicy;
    /**
      * Static instance of VtnPathPolicyBuilder to perform unit testing.
      */
    private static VtnPathPolicyBuilder vtnPathPolicyBuilder;
    /**
      * Static instance of VtnPortDesc to perform unit testing.
      */
    private static VtnPortDesc vtnPortDesc;
    /**
      * Static instance of VtnUpdateType to perform unit testing.
      */
    private static VtnUpdateType vtnUpdateType;
    /**
      * Static instance of TxContext to perform unit testing.
      */
    private static TxContext txContext;
    /**
      * Static instance of VtnPathCost to perform unit testing.
      */
    private static VtnPathCost vtnPathCost;
    /**
      * Static instance of VtnPath to perform unit testing.
      */
    private static VtnPathCostBuilder vtnPathCostBuilder;

    /**
     * This method creates the requird objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        vtnManagerImpl = new VTNManagerImpl();
        vtnManagerProvider = new VTNManagerProvStub();
        topologyGraph = new TopologyGraph(vtnManagerProvider);
        removePathPolicyInputBuilder = new RemovePathPolicyInputBuilder();
        removePathPolicyInputBuilder.setId(new Integer(10));
        removePathPolicyInput = removePathPolicyInputBuilder.build();
        vtnPathPolicyBuilder = new VtnPathPolicyBuilder();
        vtnPathPolicy = vtnPathPolicyBuilder.build();
        vtnPortDesc = new VtnPortDesc("openflow:1,,");
        vtnUpdateType = VtnUpdateType.REMOVED;
    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {
        vtnUpdateType = null;
        vtnPortDesc = null;
        vtnPathPolicy = null;
        vtnPathPolicyBuilder = null;
        removePathPolicyInputBuilder = null;
        removePathPolicyInput = null;
        removePathPolicyInputBuilder = null;
        topologyGraph = null;
        vtnManagerProvider = null;
        vtnManagerImpl = null;
    }

    /**
     * Test method for
     * {@link RemovePathPolicyTask#create(TopologyGraph , RemovePathPolicyInput)}.
     */
    @Test
    public void testCreate() {
        try {
            Assert.assertTrue(RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput) instanceof RemovePathPolicyTask);
        } catch (Exception ex) {
            Assert.assertFalse(ex instanceof RpcException);
        }
        try {
            RemovePathPolicyTask.create(topologyGraph, null);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);
        }
        try {
            removePathPolicyInputBuilder.setId(null);
            removePathPolicyInput = removePathPolicyInputBuilder.build();
            RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof RpcException);
        }
    }

    /**
      * Test method for
      * {@link RemovePathPolicyTask#onStarted(TxContext, VtnPathPolicy)}.
      */
    @Test
    public void testOnStarted() {
        try {
            vtnManagerProvider = new VTNManagerProvStub();
            topologyGraph = new TopologyGraph(vtnManagerProvider);
            removePathPolicyTask = RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
            vtnPathPolicy = null;
            removePathPolicyTask.onStarted(txContext, null);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof VTNException);
        }
        try {
            vtnManagerProvider = new VTNManagerProvStub();
            topologyGraph = new TopologyGraph(vtnManagerProvider);
            removePathPolicyTask = RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
            vtnPathPolicy = null;
            removePathPolicyTask.onStarted(null, null);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof VTNException);
        }

        try {
            txContext = vtnManagerProvider.newTxContext();
            vtnPathPolicyBuilder = new VtnPathPolicyBuilder();
            vtnPathPolicyBuilder.setDefaultCost(new Long(1000L));
            vtnPathPolicyBuilder.setId(new Integer(10));
            vtnPathPolicyBuilder.setKey(new VtnPathPolicyKey(new Integer(10)));
            vtnPathCostBuilder = new VtnPathCostBuilder();
            vtnPathCostBuilder.setCost(new Long(1000L));
            vtnPathCostBuilder.setKey(new VtnPathCostKey(new VtnPortDesc("openflow:1,,")));
            vtnPathCostBuilder.setPortDesc(new VtnPortDesc("openflow:1,,"));
            List<VtnPathCost> list = new ArrayList<VtnPathCost>();
            list.add(vtnPathCostBuilder.build());
            vtnPathPolicyBuilder.setVtnPathCost(list);
            removePathPolicyTask = RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
            removePathPolicyTask.onStarted(txContext, vtnPathPolicyBuilder.build());
        } catch (Exception ex) {
            Assert.assertFalse(ex instanceof VTNException);
        }
    }

    /**
     * Test method for
     * {@link RemovePathPolicyTask#onSuccess().
     */
    @Test
    public void testOnSuccess() {
        try {
            removePathPolicyTask = RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
            removePathPolicyTask.onSuccess(vtnManagerProvider, vtnUpdateType);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
    }

    /**
     * Test method for
     * {@link RemovePathPolicyTask#getOutputType().
     */
    @Test
    public void testGetOutputType() {
        try {
            removePathPolicyTask = RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
            Assert.assertEquals(Void.class, removePathPolicyTask.getOutputType());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
    }

    /**
     * Test method for
     * {@link RemovePathPolicyTask#createOutputType().
     */
    public void testCreateOutput() {
        try {
            removePathPolicyTask = RemovePathPolicyTask.create(topologyGraph, removePathPolicyInput);
            Assert.assertTrue(removePathPolicyTask.createOutput(vtnUpdateType) instanceof Void);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
    }
}
