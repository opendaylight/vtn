/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Test;
import org.slf4j.Logger;
import org.junit.Assert;
import org.mockito.Mockito;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import java.util.Set;
import java.util.concurrent.Future;
import org.slf4j.LoggerFactory;
import java.util.concurrent.TimeUnit;
import com.google.common.base.Optional;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.vtn.manager.internal.util.AbstractDataChangeListener;
import com.google.common.util.concurrent.CheckedFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;

/**
 * JUnit test for {@link VTNRoutingManager}
 */
public class VTNRoutingManagerTest {
    /**
      * Static instance of VTNRoutingManager to perform unit testing.
      */
    private static VTNRoutingManager vtnRoutingManager;
    /**
      * Static instance of VTNManagerProvider to perform unit testing.
      */
    private static VTNManagerProvider vtnManagerProvider;
    /**
      * Static instance of VTNRoutingListener to perform unit testing.
      */
    private static VTNRoutingListener vtnRoutingListener;
    /**
      * Static instance of DataBroker to perform unit testing.
      */
    private static DataBroker dataBroker;

    /**
     * This method creates the requird objects to perform unit testing
     */
    @BeforeClass
    public static void setUpBeforeClass() {
        try {
            vtnManagerProvider = Mockito.mock(VTNManagerProvider.class);
            dataBroker = Mockito.mock(DataBroker.class);
            ReadOnlyTransaction readOnlyTransaction = Mockito.mock(ReadOnlyTransaction.class);
            CheckedFuture checkedFuture = Mockito.mock(CheckedFuture.class);
            Optional optional = Mockito.mock(Optional.class);
            VtnTopology vtnTopology = Mockito.mock(VtnTopology.class);
            Mockito.when(optional.orNull()).thenReturn(vtnTopology);
            Mockito.when(checkedFuture.checkedGet(Mockito.anyLong(), Mockito.eq(TimeUnit.SECONDS))).thenReturn(optional);
            Mockito.when(readOnlyTransaction.read(Mockito.isA(LogicalDatastoreType.class), Mockito.isA(InstanceIdentifier.class))).thenReturn(checkedFuture);
            Mockito.when(dataBroker.newReadOnlyTransaction()).thenReturn(readOnlyTransaction);
            Mockito.when(dataBroker.registerDataChangeListener(Mockito.isA(LogicalDatastoreType.class), Mockito.isA(InstanceIdentifier.class)
                , Mockito.isA(AbstractDataChangeListener.class), Mockito.isA(DataChangeScope.class))).thenReturn(Mockito.mock(ListenerRegistration.class));
            Mockito.when(vtnManagerProvider.getDataBroker()).thenReturn(dataBroker);
            vtnRoutingManager = new VTNRoutingManager(vtnManagerProvider);
            vtnRoutingListener = new VTNManagerImpl();
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
        //To test negative scenario
        try {
            VTNManagerProvider vtnManagerProvider2 = Mockito.mock(VTNManagerProvider.class);
            DataBroker dataBroker2 = Mockito.mock(DataBroker.class);
            ReadOnlyTransaction readOnlyTransaction2 = Mockito.mock(ReadOnlyTransaction.class);
            CheckedFuture checkedFuture2 = Mockito.mock(CheckedFuture.class);
            Optional optional2 = Mockito.mock(Optional.class);
            Mockito.when(optional2.orNull()).thenThrow(new NullPointerException());
            Mockito.when(checkedFuture2.checkedGet(Mockito.anyLong(), Mockito.eq(TimeUnit.SECONDS))).thenReturn(optional2);
            Mockito.when(readOnlyTransaction2.read(Mockito.isA(LogicalDatastoreType.class), Mockito.isA(InstanceIdentifier.class))).thenReturn(checkedFuture2);
            Mockito.when(dataBroker2.newReadOnlyTransaction()).thenReturn(readOnlyTransaction2);
            Mockito.when(dataBroker2.registerDataChangeListener(Mockito.isA(LogicalDatastoreType.class), Mockito.isA(InstanceIdentifier.class)
                , Mockito.isA(AbstractDataChangeListener.class), Mockito.isA(DataChangeScope.class))).thenReturn(Mockito.mock(ListenerRegistration.class));
            Mockito.when(vtnManagerProvider2.getDataBroker()).thenReturn(dataBroker2);
            VTNRoutingManager vtnRoutingManager2 = new VTNRoutingManager(vtnManagerProvider2);
        } catch (Exception exception) {
            Assert.assertTrue(exception instanceof IllegalStateException);
        }
        //To test negative scenario
        try {
            VTNManagerProvider vtnManagerProvider3 = Mockito.mock(VTNManagerProvider.class);
            DataBroker dataBroker3 = Mockito.mock(DataBroker.class);
            ReadOnlyTransaction readOnlyTransaction3 = Mockito.mock(ReadOnlyTransaction.class);
            CheckedFuture checkedFuture3 = Mockito.mock(CheckedFuture.class);
            Optional optional3 = Mockito.mock(Optional.class);
            Mockito.when(optional3.orNull()).thenReturn(null);
            Mockito.when(checkedFuture3.checkedGet(Mockito.anyLong(), Mockito.eq(TimeUnit.SECONDS))).thenReturn(optional3);
            Mockito.when(readOnlyTransaction3.read(Mockito.isA(LogicalDatastoreType.class), Mockito.isA(InstanceIdentifier.class))).thenReturn(checkedFuture3);
            Mockito.when(dataBroker3.newReadOnlyTransaction()).thenReturn(readOnlyTransaction3);
            Mockito.when(dataBroker3.registerDataChangeListener(Mockito.isA(LogicalDatastoreType.class), Mockito.isA(InstanceIdentifier.class)
                , Mockito.isA(AbstractDataChangeListener.class), Mockito.isA(DataChangeScope.class))).thenThrow(new RuntimeException());
            Mockito.when(vtnManagerProvider3.getDataBroker()).thenReturn(dataBroker3);
            VTNRoutingManager vtnRoutingManager3 = new VTNRoutingManager(vtnManagerProvider3);
        } catch (Exception exception) {
            Assert.assertTrue(exception instanceof IllegalStateException);
        }
    }

    /**
     * This method makes unnecessary objects eligible for garbage collection
     */
    @AfterClass
    public static void tearDownAfterClass() {
        vtnRoutingManager = null;
        vtnManagerProvider = null;
        vtnRoutingListener = null;
        dataBroker = null;
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#addListener()}.
     */
    @Test
    public void testAddListener() {
        try {
            vtnRoutingManager.addListener(vtnRoutingListener);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#getRouteResolver()}.
     */
    @Test
    public void testGetRouteResolver() {
        Assert.assertTrue(vtnRoutingManager.getRouteResolver(0) instanceof RouteResolver);
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#close()}.
     */
    @Test
    public void testClose() {
        try {
            vtnRoutingManager.close();
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#enterEvent()}.
     */
    @Test
    public void testEnterEvent() {
        Assert.assertTrue(vtnRoutingManager.enterEvent(Mockito.mock(AsyncDataChangeEvent.class)) instanceof TopologyEventContext);
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#exitEvent()}.
     */
    @Test
    public void testExitEvent() {
        try {
            TopologyEventContext topologyEventContext = new TopologyEventContext();
            vtnRoutingManager.exitEvent(topologyEventContext);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#onCreated()}.
     */
    @Test
    public void testOnCreated() {
        try {
            TopologyEventContext topologyEventContext = new TopologyEventContext();
            IdentifiedData identifiedData = Mockito.mock(IdentifiedData.class);
            vtnRoutingManager.onCreated(topologyEventContext, identifiedData);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#onUpdated()}.
     */
    @Test
    public void testOnUpdated() {
        try {
            vtnRoutingManager.onUpdated(new TopologyEventContext(), null);
        } catch (Exception exception) {
            Assert.assertTrue(exception instanceof IllegalStateException);
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#onRemoved()}.
     */
    @Test
    public void testOnRemoved() {
        try {
            TopologyEventContext topologyEventContext = new TopologyEventContext();
            IdentifiedData identifiedData = Mockito.mock(IdentifiedData.class);
            vtnRoutingManager.onRemoved(topologyEventContext, identifiedData);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        Assert.assertTrue(vtnRoutingManager.getWildcardPath() instanceof InstanceIdentifier);
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#getRequiredEvents()}.
     */
    @Test
    public void testGetRequiredEvents() {
        try {
            Assert.assertTrue(vtnRoutingManager.getRequiredEvents() instanceof Set);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Assert.assertTrue(vtnRoutingManager.getLogger() instanceof Logger);
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#initConfig()}.
     */
    @Test
    public void testInitConfig() {
        try {
            VTNFuture vtnFuture = vtnRoutingManager.initConfig(true);
            vtnFuture = vtnRoutingManager.initConfig(true);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#initRpcService()}.
     */
    @Test
    public void testInitRpcServices() {
        try {
            vtnRoutingManager.initRpcServices(Mockito.mock(RpcProviderRegistry.class), new CompositeAutoCloseable(LoggerFactory.getLogger(VTNRoutingManager.class)));
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#setPathPolicy()}.
     */
    @Test
    public void testSetPathPolicy() {
        try {
            SetPathPolicyInputBuilder setPathPolicyInputBuilder = new SetPathPolicyInputBuilder().setId(1).setOperation(VtnUpdateOperationType.SET).setPresent(true);
            SetPathPolicyInput setPathPolicyInput = setPathPolicyInputBuilder.build();
            vtnRoutingManager.setPathPolicy(setPathPolicyInput);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#removePathPolicy()}.
     */
    @Test
    public void testRemovePathPolicy() {
        try {
            RemovePathPolicyInputBuilder removePathPolicyInputBuilder = new RemovePathPolicyInputBuilder().setId(1);
            RemovePathPolicyInput removePathPolicyInput = removePathPolicyInputBuilder.build();
            vtnRoutingManager.removePathPolicy(removePathPolicyInput);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#setPathCost()}.
     */
    @Test
    public void testSetPathCost() {
        try {
            SetPathCostInputBuilder setPathCostInputBuilder = new SetPathCostInputBuilder().setId(1);
            SetPathCostInput setPathCostInput = setPathCostInputBuilder.build();
            vtnRoutingManager.setPathCost(setPathCostInput);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#removePathCost()}.
     */
    @Test
    public void testRemovePathCost() {
        try {
            RemovePathCostInputBuilder removePathCostInputBuilder = new RemovePathCostInputBuilder().setId(1);
            RemovePathCostInput removePathCostInput = removePathCostInputBuilder.build();
            vtnRoutingManager.removePathCost(removePathCostInput);
        } catch (Exception exception) {
            Assert.fail("VTNRoutingManager test case failed...................");
        }
    }

    /**
     * Test method for
     * {@link VTNRoutingManager#clearPathCost()}.
     */
    @Test
    public void testClearPathPolicy() {
        try {
            Future<RpcResult<ClearPathPolicyOutput>> future = vtnRoutingManager.clearPathPolicy();
        } catch (Exception exception) {
            Assert.assertFalse(exception instanceof VTNException);
        }
    }
}

