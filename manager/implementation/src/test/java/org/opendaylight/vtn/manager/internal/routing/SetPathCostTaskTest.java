/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.junit.Test;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.mockito.Mockito;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;

 /**
 * SetPathCostTask test class is to test  the MD-SAL transaction task that set
 * all the specified link cost configurations into the path policy
 * configuration or not.
 *
 * JUnit test for {@link SetPathCostTask}
 */
public class SetPathCostTaskTest extends TestBase {
    /**
     * define rpc exception object to test failure case.
     */
    private static RpcException rpcException1;
    /**
     * define rpc exception object to test failure case.
     */
    private static RpcException rpcException2;
    /**
     * define rpc exception object to test failure case.
     */
    private static RpcException rpcException3;
    /**
     * define VtnManagerImpl object.
     */
    private static VTNManagerImpl vtnManagerImpl;
    /**
     * define VtnUpdateType object in list.
     */
    private static List<VtnUpdateType> list;
    /**
     * define TopologyGraph object .
     */
    private static TopologyGraph topologyGraph;
    /**
     * define SetPathCostInputBuilder object .
     */
    private static SetPathCostInputBuilder setPathCostInputBuilder;
    /**
     * define PathCostList object in list .
     */
    private static List<PathCostList> pathCostList;
    /**
     * define VTNManagerProvider object .
     */
    private static VTNManagerProvider vtnManagerProvider;
    /**
     * define TxContext object .
     */
    private static TxContext txContext;
    /**
     * define VtnUpdateType object .
     */
    private static VtnUpdateType vtnUpdateType;

    /**
     * method to be executed before executing.
     * the Test Case class
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        rpcException1 = RpcUtils.getNullInputException();
        rpcException2 = PathPolicyUtils.getNullPolicyIdException();
        rpcException3 = PathPolicyUtils.getNoSwitchPortException();
        vtnManagerImpl = new VTNManagerImpl();
        vtnManagerProvider = vtnManagerImpl.getVTNProvider();
        topologyGraph = new TopologyGraph(vtnManagerProvider);
        setPathCostInputBuilder = new SetPathCostInputBuilder();
        pathCostList = new ArrayList();
        vtnUpdateType = VtnUpdateType.CREATED;
        list = new ArrayList<VtnUpdateType>(Arrays.asList(VtnUpdateType.values()));
    }

    /**
     * method to be executed after executing
     * the Test Case class
     */
    @AfterClass
    public static void tearDownAfterClass() {
        topologyGraph = null;
        vtnManagerProvider = null;
        vtnManagerImpl = null;
        vtnUpdateType = null;
    }

    /**
     * Test method for
     * {@link SetPathCostTask#create(TopologyGraph , SetPathCostInput)}.
     *
     * test the new task that set all the given link cost configurations
     * into the given path policy or not.
     *
     */
    @Test
    public void testCreate() {
        int i = 2;
        Long l1 = 100L, l2 = 200L;

        try {
            /**
             * Failure case - input is null
             */
            assertEquals(rpcException1, SetPathCostTask.create(topologyGraph, null));
        } catch (Exception expected) {
        }
        try {
            /**
             * Failure case - Input.Id is null
             */
            SetPathCostInput input = setPathCostInputBuilder.setId(null).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();
            assertEquals(rpcException2, SetPathCostTask.create(topologyGraph, input));
        } catch (Exception expected) {
        }

        try {
            /**
             * Failure case - PathCostList as null
             */
            pathCostList = null;

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            assertEquals(rpcException3, SetPathCostTask.create(topologyGraph, input));
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

        try {
            /**
             * Failure case - PathCostList contains only one object as null object
             */
            pathCostList = new ArrayList();

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            assertEquals(rpcException3, SetPathCostTask.create(topologyGraph, input));
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

        try {
            /**
             * Failure case - DuplicatePortException by adding same VTNPortDesc object twice
             */
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:10,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:11,22,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            assertTrue(SetPathCostTask.create(topologyGraph, input) instanceof SetPathCostTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

        try {
            /**
             * Success case - Creating SetPathCostTask object
             */
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:10,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:11,22,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            assertTrue(SetPathCostTask.create(topologyGraph, input) instanceof SetPathCostTask);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

    /**
     * Test method for
     * {@link SetPathCostTask#onStarted(TxContext)}.
     */
    @Test
    public void testOnStarted() {
        int i = 3;
        Long l1 = 120L, l2 = 220L;

        try {
            txContext = vtnManagerProvider.newTxContext();
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:1,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:1,2,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            SetPathCostTask setPathCostTask = SetPathCostTask.create(topologyGraph, input);

            setPathCostTask.onStarted(txContext);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

    /**
     * Test method for
     * {@link SetPathCostTask#onSuccess(VTNManagerProvider,List)}.
     */
    @Test
    public void testOnSuccess() {
        int i = 4;
        Long l1 = 130L, l2 = 230L;

        try {
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:3,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:4,5,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            SetPathCostTask setPathCostTask = SetPathCostTask.create(topologyGraph, input);

            /**
             * create a mock object for DataBroker class
             */
            VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
            setPathCostTask.onSuccess(provider, list);

            /**
             * Success case - Validation with VTNUpdateType list with only one null object
             */
            List<VtnUpdateType> nullObjectList = new ArrayList();
            nullObjectList.add(null);
            setPathCostTask.onSuccess(vtnManagerProvider, nullObjectList);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

    /**
     * Test method for
     * {@link SetPathCostTask#getOutputType()}.
     */
    @Test
    public void testGetOutputType() {
        int i = 5;
        Long l1 = 140L, l2 = 240L;

        try {
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:2,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:3,4,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            SetPathCostTask setPathCostTask = SetPathCostTask.create(topologyGraph, input);

            assertEquals(SetPathCostOutput.class, setPathCostTask.getOutputType());
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }

    /**
     * Test method for
     * {@link SetPathCostTask#createOutput(List)}.
     */
    @Test
    public void testCreateOutput() {
        int i = 6;
        Long l1 = 150L, l2 = 250L;

        try {
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:13,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:14,25,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(i).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            SetPathCostTask setPathCostTask = SetPathCostTask.create(topologyGraph, input);

            assertTrue(setPathCostTask.createOutput(list) instanceof SetPathCostOutput);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }

        try {
            PathCostList pcl1 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:18,,")).setCost(l1).build();
            PathCostList pcl2 = new PathCostListBuilder().setPortDesc(new VtnPortDesc("openflow:19,20,eth2")).setCost(l2).build();
            pathCostList.add(pcl1);
            pathCostList.add(pcl2);

            SetPathCostInput input = setPathCostInputBuilder.setId(8).build();
            input = setPathCostInputBuilder.setPathCostList(pathCostList).build();

            SetPathCostTask setPathCostTask = SetPathCostTask.create(topologyGraph, input);

            assertTrue(setPathCostTask.createOutput(list) instanceof SetPathCostOutput);

            /**
             * Success case - Validation with VTNUpdateType list with only one null object
             */
            List<VtnUpdateType> emptyList = new ArrayList();
            setPathCostTask.onSuccess(vtnManagerProvider, emptyList);
        } catch (Exception e) {
            assertTrue(e instanceof Exception);
        }
    }
}
