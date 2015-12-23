/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isA;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import org.slf4j.Logger;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.BindingAwareBroker.RpcRegistration;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.RoutingUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.RoutingUpdatedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.AddedLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.AddedLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.RemovedLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.RemovedLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.remove.path.cost.output.RemovePathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.remove.path.cost.output.RemovePathCostResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * JUnit test for {@link VTNRoutingManager}.
 */
public class VTNRoutingManagerTest extends TestBase {
    /**
     * Mock-up of {@link VTNManagerProvider}.
     */
    @Mock
    private VTNManagerProvider  vtnProvider;

    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Mock-up of {@link ReadOnlyTransaction} to be associated with
     * {@link #dataBroker}.
     */
    @Mock
    private ReadOnlyTransaction  roTransaction;

    /**
     * Registration to be associated with {@link PathPolicyListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  ppListenerReg;

    /**
     * Registration to be associated with {@link VTNRoutingManager}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  routingListenerReg;

    /**
     * A {@link VtnTopology} instance which contains the initial network
     * topology.
     */
    private VtnTopology  initialTopology;

    /**
     * A {@link VTNRoutingManager} instance for test.
     */
    private VTNRoutingManager  routingManager;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);

        when(vtnProvider.getDataBroker()).thenReturn(dataBroker);
        when(dataBroker.newReadOnlyTransaction()).thenReturn(roTransaction);

        InstanceIdentifier<VtnTopology> topoPath =
            InstanceIdentifier.create(VtnTopology.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(roTransaction.read(oper, topoPath)).
            thenReturn(getReadResult(initialTopology));

        InstanceIdentifier<VtnPathPolicy> policyPath = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class).
            build();
        when(dataBroker.registerDataChangeListener(
                 eq(LogicalDatastoreType.OPERATIONAL), eq(policyPath),
                 isA(ClusteredDataChangeListener.class),
                 any(DataChangeScope.class))).
            thenReturn(ppListenerReg);

        InstanceIdentifier<VtnLink> linkPath = InstanceIdentifier.
            builder(VtnTopology.class).
            child(VtnLink.class).
            build();
        when(dataBroker.registerDataChangeListener(
                 eq(LogicalDatastoreType.OPERATIONAL), eq(linkPath),
                 isA(ClusteredDataChangeListener.class),
                 any(DataChangeScope.class))).
            thenReturn(routingListenerReg);

        routingManager = new VTNRoutingManager(vtnProvider);
    }

    /**
     * Tear down test environment.
     */
    @After
    public void tearDown() {
        initialTopology = null;
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#VTNRoutingManager(VTNManagerProvider)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        verifyRoutingManager();

        // Ensure that the topology graph is empty.
        TopologyGraph topo = getTopologyGraph(routingManager);
        Set<SalNode> nodes = new HashSet<>();
        assertTrue(topo.getVertices().isEmpty());

        // Ensure that the topology graph is initialized if the network
        // topology is not empty.
        SalNode snode1 = new SalNode(1L);
        SalNode snode2 = new SalNode(2L);
        SalNode snode3 = new SalNode(3L);
        Collections.addAll(nodes, snode1, snode2, snode3);

        // openflow:1:1 <-> openflow:2:1
        List<VtnLink> vlinks = new ArrayList<>();
        createVtnLink(vlinks, snode1, 1L, snode2, 1L);

        // openflow:1:2 <-> openflow:3:1
        createVtnLink(vlinks, snode1, 2L, snode3, 1L);
        initialTopology = new VtnTopologyBuilder().setVtnLink(vlinks).build();
        setUp();
        verifyRoutingManager();

        topo = getTopologyGraph(routingManager);
        Collection<SalNode> verts = topo.getVertices();
        assertEquals(nodes.size(), verts.size());
        for (SalNode snode: verts) {
            assertEquals(true, nodes.contains(snode));
        }

        SalPort sport11 = new SalPort(1L, 1L);
        SalPort sport12 = new SalPort(1L, 2L);
        SalPort sport21 = new SalPort(2L, 1L);
        SalPort sport31 = new SalPort(3L, 1L);

        // openflow:1:1 -> openflow:2:1
        checkRoute(snode1, snode2, sport11, sport21);

        // openflow:2:1 -> openflow:1:1
        checkRoute(snode2, snode1, sport21, sport11);

        // openflow:1:2 -> openflow:3:1
        checkRoute(snode1, snode3, sport12, sport31);

        // openflow:3:1 -> openflow:1:2
        checkRoute(snode3, snode1, sport31, sport12);

        // openflow:2:1 -> openflow:1:1, openflow:1:2 -> openflow:3:1
        checkRoute(snode2, snode3, sport21, sport11, sport12, sport31);

        // openflow:3:1 -> openflow:1:2, openflow:1:1 -> openflow:2:1
        checkRoute(snode3, snode2, sport31, sport12, sport11, sport21);

        // In case of unknown node.
        RouteResolver rr = routingManager.getRouteResolver(0);
        for (long dpid = 10L; dpid <= 20L; dpid++) {
            SalNode snode = new SalNode(dpid);
            assertEquals(null, rr.getRoute(null, snode, snode1));
            assertEquals(null, rr.getRoute(null, snode, snode2));
            assertEquals(null, rr.getRoute(null, snode, snode3));
            assertEquals(null, rr.getRoute(null, snode1, snode));
            assertEquals(null, rr.getRoute(null, snode2, snode));
            assertEquals(null, rr.getRoute(null, snode3, snode));
        }
    }

    /**
     * Test case for {@link VTNRoutingManager#addListener(VTNRoutingListener)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddListener() throws Exception {
        List<VTNRoutingListener> expected = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            VTNRoutingListener l = mock(VTNRoutingListener.class);
            expected.add(l);
            routingManager.addListener(l);

            List listeners =
                getFieldValue(routingManager, List.class, "vtnListeners");
            assertEquals(expected, listeners);
        }

        // Below calls should do nothing because listeners are already added.
        for (VTNRoutingListener l: expected) {
            routingManager.addListener(l);
            List listeners =
                getFieldValue(routingManager, List.class, "vtnListeners");
            assertEquals(expected, listeners);
        }
    }

    /**
     * Test case for {@link VTNRoutingManager#getRouteResolver(Integer)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetRouteResolver() throws Exception {
        RouteResolver res0 = routingManager.getRouteResolver(0);
        assertNotNull(res0);
        assertEquals(0, res0.getPathPolicyId());

        TopologyGraph topo = getTopologyGraph(routingManager);
        for (int i = 1; i <= 3; i++) {
            assertEquals(null, routingManager.getRouteResolver(i));
            topo.updateResolver(i);
            RouteResolver rr = routingManager.getRouteResolver(i);
            assertNotNull(rr);
            assertEquals(i, rr.getPathPolicyId());
        }

        for (int i = 1; i <= 3; i++) {
            RouteResolver rr = routingManager.getRouteResolver(i);
            assertNotNull(rr);
            assertEquals(i, rr.getPathPolicyId());
            topo.removeResolver(i);
            assertEquals(null, routingManager.getRouteResolver(i));
        }

        assertSame(res0, routingManager.getRouteResolver(0));
        assertEquals(null, routingManager.getRouteResolver(null));
    }

    /**
     * Test case for {@link VTNRoutingManager#close()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClose() throws Exception {
        List<VTNRoutingListener> expected = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            VTNRoutingListener l = mock(VTNRoutingListener.class);
            expected.add(l);
            routingManager.addListener(l);
        }

        List listeners =
            getFieldValue(routingManager, List.class, "vtnListeners");
        assertEquals(expected, listeners);

        verifyZeroInteractions(ppListenerReg);
        verifyZeroInteractions(routingListenerReg);

        // Listener registrations should be closed only once.
        expected = Collections.<VTNRoutingListener>emptyList();
        for (int i = 0; i < 10; i++) {
            routingManager.close();
            listeners =
                getFieldValue(routingManager, List.class, "vtnListeners");
            assertEquals(expected, listeners);
            verify(ppListenerReg).close();
            verify(routingListenerReg).close();
        }
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#enterEvent(AsyncDataChangeEvent)}.
     */
    @Test
    public void testEnterEvent() {
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev = null;
        TopologyEventContext ectx = routingManager.enterEvent(ev);
        assertNotNull(ectx);
        assertEquals(0, ectx.getCreated().size());
        assertEquals(0, ectx.getRemoved().size());
    }

    /**
     * Test case for {@link VTNRoutingManager#exitEvent(TopologyEventContext)}.
     *
     * <ul>
     *   <li>The process is the owner of the VTN inventory.
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExitEvent() throws Exception {
        testExitEvent(true);
    }

    /**
     * Test case for {@link VTNRoutingManager#exitEvent(TopologyEventContext)}.
     *
     * <ul>
     *   <li>The process is not the owner of the VTN inventory.
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExitEventNotOwner() throws Exception {
        testExitEvent(false);
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#onCreated(TopologyEventContext,IdentifiedData)}.
     */
    @Test
    public void testOnCreated() {
        TopologyEventContext ectx = new TopologyEventContext();
        List<VtnLink> expected = new ArrayList<>();
        List<VtnLink> empty = Collections.<VtnLink>emptyList();
        assertEquals(expected, ectx.getCreated());
        assertEquals(empty, ectx.getRemoved());

        for (long l = 0; l <= 10; l++) {
            SalPort src = new SalPort(1L + l, 2L + l);
            SalPort dst = new SalPort(10L + l, 20L + l);
            VtnLink vlink = new VtnLinkBuilder().
                setLinkId(new LinkId(src.toString())).
                setSource(src.getNodeConnectorId()).
                setDestination(dst.getNodeConnectorId()).
                build();
            expected.add(vlink);
            InstanceIdentifier<VtnLink> path =
                InventoryUtils.toVtnLinkIdentifier(vlink.getLinkId());
            IdentifiedData<VtnLink> data = new IdentifiedData<>(path, vlink);
            routingManager.onCreated(ectx, data);

            assertEquals(expected, ectx.getCreated());
            assertEquals(empty, ectx.getRemoved());
        }
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#onUpdated(TopologyEventContext,ChangedData)}.
     */
    @Test
    public void testOnUpdated() {
        TopologyEventContext ectx = new TopologyEventContext();
        List<VtnLink> created = new ArrayList<>();
        List<VtnLink> removed = new ArrayList<>();
        assertEquals(created, ectx.getCreated());
        assertEquals(removed, ectx.getRemoved());

        for (long l = 0; l <= 10; l++) {
            SalPort src = new SalPort(1L + l, 2L + l);
            SalPort dst1 = new SalPort(10L + l, 20L + l);
            SalPort dst2 = new SalPort(10L + l, 21L + l);
            VtnLink vlinkOld = new VtnLinkBuilder().
                setLinkId(new LinkId(src.toString())).
                setSource(src.getNodeConnectorId()).
                setDestination(dst1.getNodeConnectorId()).
                build();
            VtnLink vlink = new VtnLinkBuilder().
                setLinkId(vlinkOld.getLinkId()).
                setSource(src.getNodeConnectorId()).
                setDestination(dst2.getNodeConnectorId()).
                build();
            created.add(vlink);
            removed.add(vlinkOld);
            InstanceIdentifier<VtnLink> path =
                InventoryUtils.toVtnLinkIdentifier(vlink.getLinkId());
            ChangedData<VtnLink> data =
                new ChangedData<>(path, vlink, vlinkOld);
            routingManager.onUpdated(ectx, data);

            assertEquals(created, ectx.getCreated());
            assertEquals(removed, ectx.getRemoved());
        }
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#onRemoved(TopologyEventContext,IdentifiedData)}.
     */
    @Test
    public void testOnRemoved() {
        TopologyEventContext ectx = new TopologyEventContext();
        List<VtnLink> expected = new ArrayList<>();
        List<VtnLink> empty = Collections.<VtnLink>emptyList();
        assertEquals(empty, ectx.getCreated());
        assertEquals(expected, ectx.getRemoved());

        for (long l = 0; l <= 10; l++) {
            SalPort src = new SalPort(1L + l, 2L + l);
            SalPort dst = new SalPort(10L + l, 20L + l);
            VtnLink vlink = new VtnLinkBuilder().
                setLinkId(new LinkId(src.toString())).
                setSource(src.getNodeConnectorId()).
                setDestination(dst.getNodeConnectorId()).
                build();
            expected.add(vlink);
            InstanceIdentifier<VtnLink> path =
                InventoryUtils.toVtnLinkIdentifier(vlink.getLinkId());
            IdentifiedData<VtnLink> data = new IdentifiedData<>(path, vlink);
            routingManager.onRemoved(ectx, data);

            assertEquals(empty, ectx.getCreated());
            assertEquals(expected, ectx.getRemoved());
        }
    }

    /**
     * Ensure that a data change event is processed correctly.
     *
     * <p>
     *   The process is the owner of the VTN inventory.
     * </p>
     *
     * <ul>
     *   <li>
     *     {@link VTNRoutingManager#onDataChanged(AsyncDataChangeEvent)}
     *   </li>
     *   <li>
     *     {@link VTNRoutingManager#onCreated(TopologyEventContext,IdentifiedData)}
     *   </li>
     *   <li>
     *     {@link VTNRoutingManager#onRemoved(TopologyEventContext,IdentifiedData)}
     *   </li>
     *   <li>{@link VTNRoutingManager#addListener(VTNRoutingListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEvent() throws Exception {
        testEvent(true);
    }

    /**
     * Ensure that a data change event is processed correctly.
     *
     * <p>
     *   The process is not the owner of the VTN inventory.
     * </p>
     *
     * <ul>
     *   <li>
     *     {@link VTNRoutingManager#onDataChanged(AsyncDataChangeEvent)}
     *   </li>
     *   <li>
     *     {@link VTNRoutingManager#onCreated(TopologyEventContext,IdentifiedData)}
     *   </li>
     *   <li>
     *     {@link VTNRoutingManager#onRemoved(TopologyEventContext,IdentifiedData)}
     *   </li>
     *   <li>{@link VTNRoutingManager#addListener(VTNRoutingListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEventNotOwner() throws Exception {
        testEvent(false);
    }

    /**
     * Test case for {@link VTNRoutingManager#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        assertEquals(getPath(), routingManager.getWildcardPath());
    }

    /**
     * Test case for {@link VTNRoutingManager#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Logger logger = routingManager.getLogger();
        assertEquals(VTNRoutingManager.class.getName(), logger.getName());
    }

    /**
     * Test case for {@link VTNRoutingManager#initConfig(boolean)}.
     */
    @Test
    public void testInitConfig() {
        Map<Boolean, String> cases = new HashMap<>();
        String baseName = PathPolicyListener.class.getName();
        cases.put(true, baseName + ".PathPolicyLoadTask");
        cases.put(false, baseName + ".PathPolicySaveTask");
        for (Map.Entry<Boolean, String> entry: cases.entrySet()) {
            reset(vtnProvider);
            boolean master = entry.getKey().booleanValue();
            @SuppressWarnings("unchecked")
            VTNFuture<?> future = (VTNFuture<?>)mock(VTNFuture.class);
            when(vtnProvider.post(any(TxTask.class))).thenReturn(future);
            assertEquals(future, routingManager.initConfig(master));
            ArgumentCaptor<TxTask> captor =
                ArgumentCaptor.forClass(TxTask.class);
            verify(vtnProvider).post(captor.capture());
            List<TxTask> tasks = captor.getAllValues();
            assertEquals(1, tasks.size());
            TxTask task = tasks.get(0);
            assertEquals(entry.getValue(), task.getClass().getCanonicalName());
        }
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#initRpcServices(RpcProviderRegistry,CompositeAutoCloseable)}.
     */
    @Test
    public void testInitRpcServices() {
        Logger logger = mock(Logger.class);
        CompositeAutoCloseable closeables = new CompositeAutoCloseable(logger);
        RpcProviderRegistry rpcReg = mock(RpcProviderRegistry.class);
        @SuppressWarnings("unchecked")
        RpcRegistration<VtnPathPolicyService> reg =
            (RpcRegistration<VtnPathPolicyService>)mock(RpcRegistration.class);
        Class<VtnPathPolicyService> type = VtnPathPolicyService.class;
        when(rpcReg.addRpcImplementation(type, routingManager)).
            thenReturn(reg);

        routingManager.initRpcServices(rpcReg, closeables);
        verify(rpcReg).addRpcImplementation(type, routingManager);
        verifyZeroInteractions(reg);

        closeables.close();
        verify(reg).close();
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#setPathPolicy(SetPathPolicyInput)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPathPolicy() throws Exception {
        int id = 1;
        TopologyGraph topo = getTopologyGraph(routingManager);
        assertEquals(null, routingManager.getRouteResolver(id));
        Long rev = topo.getRevision(id);
        assertEquals(0L, rev.longValue());

        // In case of successful completion.
        reset(vtnProvider);
        SetPathPolicyInput input = new SetPathPolicyInputBuilder().
            setId(id).
            setDefaultCost(100L).
            setOperation(VtnUpdateOperationType.SET).
            build();
        SettableVTNFuture<VtnUpdateType> taskFuture = new SettableVTNFuture<>();
        when(vtnProvider.postSync(isA(SetPathPolicyTask.class))).
            thenReturn(taskFuture);
        SettableVTNFuture<Void> flowFuture = new SettableVTNFuture<>();
        when(vtnProvider.removeFlows(isA(AllFlowRemover.class))).
            thenReturn(flowFuture);
        Future<RpcResult<SetPathPolicyOutput>> future =
            routingManager.setPathPolicy(input);
        VtnUpdateType utype = VtnUpdateType.CREATED;
        ArgumentCaptor<SetPathPolicyTask> tcaptor =
            ArgumentCaptor.forClass(SetPathPolicyTask.class);
        verify(vtnProvider).postSync(tcaptor.capture());
        List<SetPathPolicyTask> tasks = tcaptor.getAllValues();
        assertEquals(1, tasks.size());
        SetPathPolicyTask task = tasks.get(0);
        task.onSuccess(vtnProvider, utype);
        topo.updateResolver(id);
        taskFuture.set(utype);

        TimeUnit unit = TimeUnit.SECONDS;
        RpcResult<SetPathPolicyOutput> result = future.get(1L, unit);
        assertEquals(true, result.isSuccessful());
        SetPathPolicyOutput output = result.getResult();
        assertEquals(utype, output.getStatus());
        verify(vtnProvider).removeFlows(isA(AllFlowRemover.class));
        RouteResolver rr = routingManager.getRouteResolver(id);
        assertNotNull(rr);
        assertEquals(id, rr.getPathPolicyId());
        assertNotEquals(rev, topo.getRevision(id));

        // In case of failure.
        reset(vtnProvider);
        input = null;
        verifyRpcInputNull(routingManager.setPathPolicy(input));
        verifyZeroInteractions(vtnProvider);
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#removePathPolicy(RemovePathPolicyInput)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemovePathPolicy() throws Exception {
        // Prepare route resolver for path policy to be removed.
        int id = 1;
        TopologyGraph topo = getTopologyGraph(routingManager);
        assertTrue(topo.updateResolver(id));
        assertNotNull(routingManager.getRouteResolver(id));
        Long rev = topo.getRevision(id);

        // In case of successful completion.
        reset(vtnProvider);
        RemovePathPolicyInput input = new RemovePathPolicyInputBuilder().
            setId(id).build();
        SettableVTNFuture<VtnUpdateType> taskFuture = new SettableVTNFuture<>();
        when(vtnProvider.postSync(isA(RemovePathPolicyTask.class))).
            thenReturn(taskFuture);
        VTNFuture<Void> flowFuture = new SettableVTNFuture<>();
        when(vtnProvider.removeFlows(any(FlowRemover.class))).
            thenReturn(flowFuture);
        Future<RpcResult<Void>> future =
            routingManager.removePathPolicy(input);
        ArgumentCaptor<RemovePathPolicyTask> tcaptor =
            ArgumentCaptor.forClass(RemovePathPolicyTask.class);
        verify(vtnProvider).postSync(tcaptor.capture());
        List<RemovePathPolicyTask> tasks = tcaptor.getAllValues();
        assertEquals(1, tasks.size());
        RemovePathPolicyTask task = tasks.get(0);
        VtnUpdateType utype = VtnUpdateType.REMOVED;
        task.onSuccess(vtnProvider, utype);
        topo.removeResolver(id);
        taskFuture.set(utype);

        TimeUnit unit = TimeUnit.SECONDS;
        RpcResult<Void> result = future.get(1L, unit);
        assertEquals(true, result.isSuccessful());
        ArgumentCaptor<PathPolicyFlowRemover> fcaptor =
            ArgumentCaptor.forClass(PathPolicyFlowRemover.class);
        verify(vtnProvider).removeFlows(fcaptor.capture());
        List<PathPolicyFlowRemover> removers = fcaptor.getAllValues();
        assertEquals(1, removers.size());
        Set<Integer> idSet = removers.get(0).getPathPolicyIds();
        assertEquals(1, idSet.size());
        assertEquals(true, idSet.contains(id));
        assertEquals(null, routingManager.getRouteResolver(id));
        assertNotEquals(rev, topo.getRevision(id));

        // In case of failure.
        reset(vtnProvider);
        input = null;
        verifyRpcInputNull(routingManager.removePathPolicy(input));
        verifyZeroInteractions(vtnProvider);
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#setPathCost(SetPathCostInput)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPathCost() throws Exception {
        // Prepare route resolver for path policy to be changed.
        int id = 2;
        TopologyGraph topo = getTopologyGraph(routingManager);
        assertTrue(topo.updateResolver(id));
        assertNotNull(routingManager.getRouteResolver(id));
        Long rev = topo.getRevision(id);

        // In case of successful completion.
        reset(vtnProvider);
        List<PathCostList> costs = new ArrayList<>();
        List<SetPathCostResult> expected = new ArrayList<>();
        List<VtnUpdateType> taskResults = new ArrayList<>();
        VtnPortDesc unchanged = new VtnPortDesc("openflow:3,10,,");
        PathCostList pcl = new PathCostListBuilder().
            setPortDesc(unchanged).setCost(12345L).build();
        costs.add(pcl);
        taskResults.add(null);
        SetPathCostResult r = new SetPathCostResultBuilder().
            setPortDesc(unchanged).build();
        expected.add(r);

        VtnPortDesc created = new VtnPortDesc("openflow:1,3,eth3");
        pcl = new PathCostListBuilder().
            setPortDesc(created).setCost(999L).build();
        costs.add(pcl);
        VtnUpdateType utype = VtnUpdateType.CREATED;
        taskResults.add(utype);
        r = new SetPathCostResultBuilder().
            setPortDesc(created).setStatus(utype).build();
        expected.add(r);

        VtnPortDesc changed = new VtnPortDesc("openflow:2,,eth10");
        pcl = new PathCostListBuilder().
            setPortDesc(changed).setCost(100000L).build();
        costs.add(pcl);
        utype = VtnUpdateType.CHANGED;
        taskResults.add(utype);
        r = new SetPathCostResultBuilder().
            setPortDesc(changed).setStatus(utype).build();
        expected.add(r);

        SetPathCostInput input = new SetPathCostInputBuilder().
            setId(id).setPathCostList(costs).build();
        SettableVTNFuture<List<VtnUpdateType>> taskFuture =
            new SettableVTNFuture<>();
        when(vtnProvider.postSync(isA(SetPathCostTask.class))).
            thenReturn(taskFuture);
        VTNFuture<Void> flowFuture = new SettableVTNFuture<>();
        when(vtnProvider.removeFlows(any(FlowRemover.class))).
            thenReturn(flowFuture);
        Future<RpcResult<SetPathCostOutput>> future =
            routingManager.setPathCost(input);
        ArgumentCaptor<SetPathCostTask> tcaptor =
            ArgumentCaptor.forClass(SetPathCostTask.class);
        verify(vtnProvider).postSync(tcaptor.capture());
        List<SetPathCostTask> tasks = tcaptor.getAllValues();
        assertEquals(1, tasks.size());
        SetPathCostTask task = tasks.get(0);
        task.onSuccess(vtnProvider, taskResults);
        topo.updateResolver(id);
        taskFuture.set(taskResults);

        TimeUnit unit = TimeUnit.SECONDS;
        RpcResult<SetPathCostOutput> result = future.get(1L, unit);
        assertEquals(true, result.isSuccessful());
        SetPathCostOutput output = result.getResult();
        assertEquals(expected, output.getSetPathCostResult());
        ArgumentCaptor<PathPolicyFlowRemover> fcaptor =
            ArgumentCaptor.forClass(PathPolicyFlowRemover.class);
        verify(vtnProvider).removeFlows(fcaptor.capture());
        List<PathPolicyFlowRemover> removers = fcaptor.getAllValues();
        assertEquals(1, removers.size());
        Set<Integer> idSet = removers.get(0).getPathPolicyIds();
        assertEquals(1, idSet.size());
        assertEquals(true, idSet.contains(id));
        assertNotEquals(rev, topo.getRevision(id));

        // In case of failure.
        reset(vtnProvider);
        input = null;
        verifyRpcInputNull(routingManager.setPathCost(input));
        verifyZeroInteractions(vtnProvider);
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#removePathCost(RemovePathCostInput)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRemovePathCost() throws Exception {
        // Prepare route resolver for path policy to be changed.
        int id = 3;
        TopologyGraph topo = getTopologyGraph(routingManager);
        assertTrue(topo.updateResolver(id));
        assertNotNull(routingManager.getRouteResolver(id));
        Long rev = topo.getRevision(id);

        // In case of successful completion.
        reset(vtnProvider);
        Map<VtnPortDesc, VtnUpdateType> map = new HashMap<>();
        List<VtnPortDesc> descs = new ArrayList<>();
        VtnPortDesc notfound1 = new VtnPortDesc("openflow:3,10,");
        map.put(notfound1, null);
        descs.add(notfound1);

        VtnPortDesc notfound2 = new VtnPortDesc("openflow:1,2,");
        map.put(notfound2, null);
        descs.add(notfound2);

        VtnPortDesc removed = new VtnPortDesc("openflow:10,5,eth5");
        map.put(removed, VtnUpdateType.REMOVED);
        descs.add(removed);

        RemovePathCostInput input = new RemovePathCostInputBuilder().
            setId(id).setPortDesc(descs).build();
        SettableVTNFuture<List<VtnUpdateType>> taskFuture =
            new SettableVTNFuture<>();
        when(vtnProvider.postSync(isA(RemovePathCostTask.class))).
            thenReturn(taskFuture);
        VTNFuture<Void> flowFuture = new SettableVTNFuture<>();
        when(vtnProvider.removeFlows(any(FlowRemover.class))).
            thenReturn(flowFuture);
        Future<RpcResult<RemovePathCostOutput>> future =
            routingManager.removePathCost(input);

        ArgumentCaptor<RemovePathCostTask> tcaptor =
            ArgumentCaptor.forClass(RemovePathCostTask.class);
        verify(vtnProvider).postSync(tcaptor.capture());
        List<RemovePathCostTask> tasks = tcaptor.getAllValues();
        assertEquals(1, tasks.size());
        RemovePathCostTask task = tasks.get(0);
        List<VtnUpdateType> taskResults = new ArrayList<>();
        List<RemovePathCostResult> expected = new ArrayList<>();
        for (RemoveCostTask rtask: task.getSubTasks()) {
            VtnPortDesc vdesc = rtask.getPortDesc();
            assertTrue(map.containsKey(vdesc));
            VtnUpdateType status = map.remove(vdesc);
            RemovePathCostResult r = new RemovePathCostResultBuilder().
                setPortDesc(vdesc).setStatus(status).build();
            expected.add(r);
            taskResults.add(status);
        }
        assertEquals(0, map.size());

        task.onSuccess(vtnProvider, taskResults);
        topo.updateResolver(id);
        taskFuture.set(taskResults);

        TimeUnit unit = TimeUnit.SECONDS;
        RpcResult<RemovePathCostOutput> result = future.get(1L, unit);
        assertEquals(true, result.isSuccessful());
        RemovePathCostOutput output = result.getResult();
        assertEquals(expected, output.getRemovePathCostResult());
        ArgumentCaptor<PathPolicyFlowRemover> fcaptor =
            ArgumentCaptor.forClass(PathPolicyFlowRemover.class);
        verify(vtnProvider).removeFlows(fcaptor.capture());
        List<PathPolicyFlowRemover> removers = fcaptor.getAllValues();
        assertEquals(1, removers.size());
        Set<Integer> idSet = removers.get(0).getPathPolicyIds();
        assertEquals(1, idSet.size());
        assertEquals(true, idSet.contains(id));
        assertNotEquals(rev, topo.getRevision(id));

        // In case of failure.
        reset(vtnProvider);
        input = null;
        verifyRpcInputNull(routingManager.removePathCost(input));
        verifyZeroInteractions(vtnProvider);
    }

    /**
     * Test case for
     * {@link VTNRoutingManager#clearPathPolicy()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClearPathPolicy() throws Exception {
        // Prepare path policies to be removed.
        TopologyGraph topo = getTopologyGraph(routingManager);
        List<VtnPathPolicy> policies = new ArrayList<>();
        Set<Integer> policyIds = new HashSet<>();
        for (int id = 1; id <= 3; id++) {
            policyIds.add(id);
            VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(id).build();
            policies.add(vpp);
            topo.updateResolver(id);
            assertNotNull(routingManager.getRouteResolver(id));
        }
        VtnPathPolicies root = new VtnPathPoliciesBuilder().
            setVtnPathPolicy(policies).build();

        reset(vtnProvider);
        SettableVTNFuture<VtnUpdateType> taskFuture = new SettableVTNFuture<>();
        when(vtnProvider.postSync(isA(ClearPathPolicyTask.class))).
            thenReturn(taskFuture);
        VTNFuture<Void> flowFuture = new SettableVTNFuture<>();
        when(vtnProvider.removeFlows(any(FlowRemover.class))).
            thenReturn(flowFuture);
        Future<RpcResult<ClearPathPolicyOutput>> future =
            routingManager.clearPathPolicy();
        ArgumentCaptor<ClearPathPolicyTask> tcaptor =
            ArgumentCaptor.forClass(ClearPathPolicyTask.class);
        verify(vtnProvider).postSync(tcaptor.capture());
        List<ClearPathPolicyTask> tasks = tcaptor.getAllValues();
        assertEquals(1, tasks.size());
        ClearPathPolicyTask task = tasks.get(0);
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InstanceIdentifier<VtnPathPolicies> path =
            InstanceIdentifier.create(VtnPathPolicies.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(tx.read(oper, path)).thenReturn(getReadResult(root));
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VtnUpdateType utype = task.execute(ctx);
        assertEquals(VtnUpdateType.REMOVED, utype);
        verify(ctx).getReadWriteTransaction();
        verify(tx).read(oper, path);
        VtnPathPolicies empty = new VtnPathPoliciesBuilder().build();
        verify(tx).put(oper, path, empty, true);

        task.onSuccess(vtnProvider, utype);
        for (int id = 1; id <= 3; id++) {
            topo.removeResolver(id);
        }
        taskFuture.set(utype);

        TimeUnit unit = TimeUnit.SECONDS;
        RpcResult<ClearPathPolicyOutput> result = future.get(1L, unit);
        assertEquals(true, result.isSuccessful());
        ClearPathPolicyOutput output = result.getResult();
        assertEquals(utype, output.getStatus());
        ArgumentCaptor<PathPolicyFlowRemover> fcaptor =
            ArgumentCaptor.forClass(PathPolicyFlowRemover.class);
        verify(vtnProvider).removeFlows(fcaptor.capture());
        List<PathPolicyFlowRemover> removers = fcaptor.getAllValues();
        assertEquals(1, removers.size());
        assertEquals(policyIds, removers.get(0).getPathPolicyIds());
        for (int id = 1; id <= 3; id++) {
            assertEquals(null, routingManager.getRouteResolver(id));
        }
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<VtnLink> getPath() {
        return InstanceIdentifier.builder(VtnTopology.class).
            child(VtnLink.class).build();
    }

    /**
     * Return a {@link TopologyGraph} instance configured in the given
     * routing manager.
     *
     * @param rtm  A {@link VTNRoutingManager} instance.
     * @return  A {@link TopologyGraph} instance.
     * @throws Exception  An error occurred.
     */
    private TopologyGraph getTopologyGraph(VTNRoutingManager rtm)
        throws Exception {
        return getFieldValue(rtm, TopologyGraph.class, "topology");
    }

    /**
     * Ensure that the routing manager was initialized correctly.
     *
     * @throws Exception  An error occurred.
     */
    private void verifyRoutingManager() throws Exception {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;

        // Ensure that PathPolicyListener is registered as clustered data
        // change listener.
        InstanceIdentifier<VtnPathPolicy> ppath = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class).
            build();
        ArgumentCaptor<ClusteredDataChangeListener> captor =
            ArgumentCaptor.forClass(ClusteredDataChangeListener.class);
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(ppath), captor.capture(), eq(scope));
        List<ClusteredDataChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        ClusteredDataChangeListener cdcl = wrappers.get(0);
        DataChangeListener dcl =
            getFieldValue(cdcl, DataChangeListener.class, "theListener");
        assertEquals(PathPolicyListener.class, dcl.getClass());

        // Ensure that VTNRoutingManager is registered as clustered data
        // change listener.
        captor = ArgumentCaptor.forClass(ClusteredDataChangeListener.class);
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(getPath()), captor.capture(), eq(scope));
        wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        cdcl = wrappers.get(0);
        assertEquals(routingManager,
                     getFieldValue(cdcl, DataChangeListener.class,
                                   "theListener"));

        verifyZeroInteractions(ppListenerReg);
        verifyZeroInteractions(routingListenerReg);

        // Default route resolver must be always present.
        RouteResolver rr = routingManager.getRouteResolver(0);
        assertNotNull(rr);
        assertEquals(0, rr.getPathPolicyId());

        for (int i = 1; i <= 10; i++) {
            assertEquals(null, routingManager.getRouteResolver(i));
        }
    }

    /**
     * Create a network topology link.
     *
     * @param snode   The source node.
     * @param sport   The source port number.
     * @param dnode   The destination node.
     * @param dport   The destination port number.
     * @return  A {@link VtnLink} instance.
     */
    private VtnLink createVtnLink(SalNode snode, long sport, SalNode dnode,
                                  long dport) {
        SalPort src = new SalPort(snode.getNodeNumber(), sport);
        SalPort dst = new SalPort(dnode.getNodeNumber(), dport);
        return new VtnLinkBuilder().
            setLinkId(new LinkId(src.toString())).
            setSource(src.getNodeConnectorId()).
            setDestination(dst.getNodeConnectorId()).
            build();
    }

    /**
     * Create network topology links which conntects the given 2 switch ports.
     *
     * @param vlinks  A list to store created links.
     * @param node1   The first node to be connected.
     * @param port1   The port number of {@code node1} to be connected.
     * @param node2   The second node to be connected.
     * @param port2   The port number of {@code node2} to be connected.
     */
    private void createVtnLink(List<VtnLink> vlinks, SalNode node1, long port1,
                               SalNode node2, long port2) {
        SalPort sport1 = new SalPort(node1.getNodeNumber(), port1);
        SalPort sport2 = new SalPort(node2.getNodeNumber(), port2);
        VtnLink vlink = new VtnLinkBuilder().
            setLinkId(new LinkId(sport1.toString())).
            setSource(sport1.getNodeConnectorId()).
            setDestination(sport2.getNodeConnectorId()).
            build();
        vlinks.add(vlink);

        vlink = new VtnLinkBuilder().
            setLinkId(new LinkId(sport2.toString())).
            setSource(sport2.getNodeConnectorId()).
            setDestination(sport1.getNodeConnectorId()).
            build();
        vlinks.add(vlink);
    }

    /**
     * Check the packet routing table.
     *
     * @param src    The source node of the packet.
     * @param dst    The destination node of the packet.
     * @param ports  Switch ports which represents the expected packet route.
     */
    private void checkRoute(SalNode src, SalNode dst, SalPort ... ports) {
        RouteResolver rr = routingManager.getRouteResolver(0);
        List<LinkEdge> route = rr.getRoute(null, src, dst);
        if (ports.length == 0) {
            assertEquals(null, route);
            return;
        }

        assertEquals(0, ports.length & 1);
        int idx = 0;
        for (LinkEdge le: route) {
            assertEquals(ports[idx], le.getSourcePort());
            assertEquals(ports[idx + 1], le.getDestinationPort());
            idx += 2;
        }
        assertEquals(ports.length, idx);
    }

    /**
     * Common test for {@link #testExitEvent()} and
     * {@link #testExitEventNotOwner()}.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testExitEvent(boolean owner) throws Exception {
        final int nlisteners = 3;
        VTNRoutingListener[] listeners = new VTNRoutingListener[nlisteners];
        for (int i = 0; i < nlisteners; i++) {
            VTNRoutingListener l = mock(VTNRoutingListener.class);
            listeners[i] = l;
            routingManager.addListener(l);
        }

        // In case where topology was not changed.
        reset(vtnProvider);
        TopologyEventContext ectx = new TopologyEventContext();
        routingManager.exitEvent(ectx);
        verifyZeroInteractions(vtnProvider);
        for (VTNRoutingListener l: listeners) {
            verifyZeroInteractions(l);
        }

        // In case where 2 links were added.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        TopologyGraph topo = getTopologyGraph(routingManager);
        assertTrue(topo.getVertices().isEmpty());
        SalNode snode1 = new SalNode(1L);
        SalNode snode2 = new SalNode(2L);
        Set<SalNode> nodes = new HashSet<>();
        Collections.addAll(nodes, snode1, snode2);

        List<VtnLink> vlinks = new ArrayList<>();
        createVtnLink(vlinks, snode1, 1L, snode2, 1L);

        List<AddedLink> addedLinks = new ArrayList<>();
        for (VtnLink vlink: vlinks) {
            ectx.addCreated(vlink);
            addedLinks.add(new AddedLinkBuilder(vlink).build());
        }
        routingManager.exitEvent(ectx);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);

        if (owner) {
            RoutingUpdated updated = new RoutingUpdatedBuilder().
                setAddedLink(addedLinks).
                setRemovedLink(Collections.<RemovedLink>emptyList()).
                build();
            verify(vtnProvider).publish(updated);
            verify(vtnProvider, times(nlisteners)).post(any(TxTask.class));
            ArgumentCaptor<RoutingEvent> captor =
                ArgumentCaptor.forClass(RoutingEvent.class);
            verify(vtnProvider, times(nlisteners)).post(captor.capture());
            List<RoutingEvent> delivered = captor.getAllValues();
            assertEquals(nlisteners, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                RoutingEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNRoutingListener.class,
                                           "listener"));
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);

        Collection<SalNode> verts = topo.getVertices();
        assertEquals(nodes.size(), verts.size());
        for (SalNode snode: verts) {
            assertEquals(true, nodes.contains(snode));
        }

        // openflow:1:1 -> openflow:2:1
        SalPort sport11 = new SalPort(1L, 1L);
        SalPort sport21 = new SalPort(2L, 1L);
        checkRoute(snode1, snode2, sport11, sport21);

        // openflow:2:1 -> openflow:1:1
        checkRoute(snode2, snode1, sport21, sport11);

        // In case where the link from openflow:1:1 to openflow:2:1 was
        // removed.
        reset(vtnProvider);
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        VtnLink vlink = new VtnLinkBuilder().
            setLinkId(new LinkId(sport11.toString())).
            setSource(sport11.getNodeConnectorId()).
            setDestination(sport21.getNodeConnectorId()).
            build();
        List<RemovedLink> removedLinks =
            Collections.singletonList(new RemovedLinkBuilder(vlink).build());
        ectx = new TopologyEventContext();
        ectx.addRemoved(vlink);
        routingManager.exitEvent(ectx);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);

        if (owner) {
            RoutingUpdated updated = new RoutingUpdatedBuilder().
                setAddedLink(Collections.<AddedLink>emptyList()).
                setRemovedLink(removedLinks).
                build();
            verify(vtnProvider).publish(updated);
            verify(vtnProvider, times(nlisteners)).post(any(TxTask.class));
            ArgumentCaptor<RoutingEvent> captor =
                ArgumentCaptor.forClass(RoutingEvent.class);
            verify(vtnProvider, times(nlisteners)).post(captor.capture());
            List<RoutingEvent> delivered = captor.getAllValues();
            assertEquals(nlisteners, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                RoutingEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNRoutingListener.class,
                                           "listener"));
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);

        verts = topo.getVertices();
        assertEquals(nodes.size(), verts.size());
        for (SalNode snode: verts) {
            assertEquals(true, nodes.contains(snode));
        }

        // openflow:1:1 -> openflow:2:1
        checkRoute(snode1, snode2);

        // openflow:2:1 -> openflow:1:1
        checkRoute(snode2, snode1, sport21, sport11);
    }

    /**
     * Common test for {@link #testEvent()} and {@link #testEventNotOwner()}.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testEvent(boolean owner) throws Exception {
        reset(vtnProvider);
        final int nlisteners = 3;
        VTNRoutingListener[] listeners = new VTNRoutingListener[nlisteners];
        for (int i = 0; i < nlisteners; i++) {
            VTNRoutingListener l = mock(VTNRoutingListener.class);
            listeners[i] = l;
            routingManager.addListener(l);
        }

        // Set up initial topology.
        SalNode snode1 = new SalNode(1L);
        SalNode snode2 = new SalNode(2L);
        SalNode snode3 = new SalNode(3L);
        SalNode snode4 = new SalNode(4L);
        Set<SalNode> nodes = new HashSet<>();
        Collections.addAll(nodes, snode1, snode2, snode3, snode4);

        // openflow:1:1 <-> openflow:2:1
        List<VtnLink> vlinks = new ArrayList<>();
        createVtnLink(vlinks, snode1, 1L, snode2, 1L);

        // openflow:1:2 <-> openflow:3:1
        List<VtnLink> deleted = new ArrayList<>();
        createVtnLink(deleted, snode1, 2L, snode3, 1L);
        vlinks.addAll(deleted);

        // openflow:3:2 <-> openflow:4:1
        List<VtnLink> changed = new ArrayList<>();
        createVtnLink(changed, snode3, 2L, snode4, 1L);
        vlinks.addAll(changed);

        TopologyGraph topo = getTopologyGraph(routingManager);
        assertEquals(true,
                     topo.update(vlinks, Collections.<VtnLink>emptyList()));
        Collection<SalNode> verts = topo.getVertices();
        assertEquals(nodes.size(), verts.size());
        for (SalNode snode: verts) {
            assertEquals(true, nodes.contains(snode));
        }

        // Verify initial topology.
        SalPort sport11 = new SalPort(1L, 1L);
        SalPort sport12 = new SalPort(1L, 2L);
        SalPort sport21 = new SalPort(2L, 1L);
        SalPort sport31 = new SalPort(3L, 1L);
        SalPort sport32 = new SalPort(3L, 2L);
        SalPort sport41 = new SalPort(4L, 1L);
        checkRoute(snode1, snode2, sport11, sport21);
        checkRoute(snode2, snode1, sport21, sport11);
        checkRoute(snode1, snode3, sport12, sport31);
        checkRoute(snode3, snode1, sport31, sport12);
        checkRoute(snode2, snode3, sport21, sport11, sport12, sport31);
        checkRoute(snode3, snode2, sport31, sport12, sport11, sport21);
        checkRoute(snode1, snode4, sport12, sport31, sport32, sport41);
        checkRoute(snode4, snode1, sport41, sport32, sport31, sport12);

        // Create links between openflow:5 and openflow:2.
        Map<InstanceIdentifier<?>, DataObject> created = new HashMap<>();
        List<VtnLink> added = new ArrayList<>();
        Set<AddedLink> addedSet = new HashSet<>();
        SalNode snode5 = new SalNode(5L);
        createVtnLink(added, snode2, 2L, snode5, 1L);
        for (VtnLink vlink: added) {
            InstanceIdentifier<VtnLink> path =
                InventoryUtils.toVtnLinkIdentifier(vlink.getLinkId());
            assertEquals(null, created.put(path, vlink));
            AddedLink al = new AddedLinkBuilder(vlink).build();
            assertEquals(true, addedSet.add(al));
        }

        // Remove links between openflow:1 and openflow:3.
        Map<InstanceIdentifier<?>, DataObject> original = new HashMap<>();
        Set<InstanceIdentifier<?>> removed = new HashSet<>();
        Set<RemovedLink> removedSet = new HashSet<>();
        for (VtnLink vlink: deleted) {
            InstanceIdentifier<VtnLink> path =
                InventoryUtils.toVtnLinkIdentifier(vlink.getLinkId());
            assertEquals(true, removed.add(path));
            assertEquals(null, original.put(path, vlink));
            RemovedLink rl = new RemovedLinkBuilder(vlink).build();
            assertEquals(true, removedSet.add(rl));
        }

        // Remove link from openflow:3:2 to openflow:4:1.
        VtnLink vlink1 = changed.get(0);
        assertEquals("openflow:3:2", vlink1.getLinkId().getValue());
        InstanceIdentifier<VtnLink> path1 =
            InventoryUtils.toVtnLinkIdentifier(vlink1.getLinkId());
        assertEquals(true, removed.add(path1));
        assertEquals(null, original.put(path1, vlink1));
        assertTrue(removedSet.add(new RemovedLinkBuilder(vlink1).build()));

        // Create a link from openflow:2:3 to openflow:4:1.
        vlink1 = createVtnLink(snode2, 3L, snode4, 1L);
        path1 = InventoryUtils.toVtnLinkIdentifier(vlink1.getLinkId());
        assertEquals(null, created.put(path1, vlink1));
        assertTrue(addedSet.add(new AddedLinkBuilder(vlink1).build()));

        // Change link openflow:4:1 to openflow:3:2 as from openflow:4:1
        // to openflow:2:3.
        Map<InstanceIdentifier<?>, DataObject> updated = new HashMap<>();
        vlink1 = changed.get(1);
        assertEquals("openflow:4:1", vlink1.getLinkId().getValue());
        VtnLink newLink = createVtnLink(snode4, 1L, snode2, 3L);
        path1 = InventoryUtils.toVtnLinkIdentifier(vlink1.getLinkId());
        assertEquals(null, original.put(path1, vlink1));
        assertEquals(null, updated.put(path1, newLink));
        assertTrue(removedSet.add(new RemovedLinkBuilder(vlink1).build()));
        assertTrue(addedSet.add(new AddedLinkBuilder(newLink).build()));

        // Construct an AsyncDataChangeEvent.
        @SuppressWarnings("unchecked")
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> event =
            (AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject>)mock(
                AsyncDataChangeEvent.class);
        when(event.getCreatedData()).
            thenReturn(Collections.unmodifiableMap(created));
        when(event.getUpdatedData()).
            thenReturn(Collections.unmodifiableMap(updated));
        when(event.getRemovedPaths()).
            thenReturn(Collections.unmodifiableSet(removed));
        when(event.getOriginalData()).
            thenReturn(Collections.unmodifiableMap(original));

        // Notify data change event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        routingManager.onDataChanged(event);

        if (owner) {
            // Verify MD-SAL notification.
            ArgumentCaptor<RoutingUpdated> ncaptor =
                ArgumentCaptor.forClass(RoutingUpdated.class);
            verify(vtnProvider).publish(ncaptor.capture());
            List<RoutingUpdated> notifications = ncaptor.getAllValues();
            assertEquals(1, notifications.size());
            RoutingUpdated ru = notifications.get(0);
            for (AddedLink al: ru.getAddedLink()) {
                assertEquals(true, addedSet.remove(al));
            }
            for (RemovedLink rl: ru.getRemovedLink()) {
                assertEquals(true, removedSet.remove(rl));
            }
            assertEquals(true, addedSet.isEmpty());
            assertEquals(true, removedSet.isEmpty());

            // Ensure that routing events are delivered to listeners.
            verify(vtnProvider, times(nlisteners)).post(any(TxTask.class));
            ArgumentCaptor<RoutingEvent> ecaptor =
                ArgumentCaptor.forClass(RoutingEvent.class);
            verify(vtnProvider, times(nlisteners)).post(ecaptor.capture());
            List<RoutingEvent> delivered = ecaptor.getAllValues();
            assertEquals(nlisteners, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                RoutingEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNRoutingListener.class,
                                           "listener"));
                verifyZeroInteractions(listeners[i]);
            }
        }
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);

        // openflow:3 should be removed from the topology, and openflow:5
        // should be added to the topology.
        assertEquals(true, nodes.remove(snode3));
        assertEquals(true, nodes.add(snode5));
        verts = topo.getVertices();
        assertEquals(nodes.size(), verts.size());
        for (SalNode snode: verts) {
            assertEquals(true, nodes.contains(snode));

            // openflow:3 should be unreachable.
            checkRoute(snode, snode3);
            checkRoute(snode3, snode);
        }

        // Links between openflow:1 and openflow2 should be still active.
        checkRoute(snode1, snode2, sport11, sport21);
        checkRoute(snode2, snode1, sport21, sport11);

        // openflow:5 should be reachable.
        SalPort sport22 = new SalPort(2L, 2L);
        SalPort sport51 = new SalPort(5L, 1L);
        checkRoute(snode2, snode5, sport22, sport51);
        checkRoute(snode5, snode2, sport51, sport22);
        checkRoute(snode1, snode5, sport11, sport21, sport22, sport51);
        checkRoute(snode5, snode1, sport51, sport22, sport21, sport11);

        // openflow:4:1 is now connected to openflow:2:3.
        SalPort sport23 = new SalPort(2L, 3L);
        checkRoute(snode1, snode4, sport11, sport21, sport23, sport41);
        checkRoute(snode4, snode1, sport41, sport23, sport21, sport11);
    }
}
