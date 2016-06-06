/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isA;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
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
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcInvocation;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;

/**
 * JUnit test for {@link VTNInventoryManager}.
 */
public class VTNInventoryManagerTest extends TestBase {
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
     * Registration to be associated with {@link NodeListener}.
     */
    @Mock
    private ListenerRegistration<NodeListener>  nodeListenerReg;

    /**
     * Registration to be associated with {@link NodeConnectorListener}.
     */
    @Mock
    private ListenerRegistration<NodeConnectorListener>  ncListenerReg;

    /**
     * Registration to be associated with {@link TopologyListener}.
     */
    @Mock
    private ListenerRegistration<TopologyListener>  topoListenerReg;

    /**
     * Registration to be associated with {@link VTNInventoryManager}.
     */
    @Mock
    private ListenerRegistration<DataTreeChangeListener<VtnNode>>  inventoryListenerReg;

    /**
     * A {@link VTNInventoryManager} instance for test.
     */
    private VTNInventoryManager  inventoryManager;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);
        when(vtnProvider.getDataBroker()).thenReturn(dataBroker);

        Class<DataTreeIdentifier> idtype = DataTreeIdentifier.class;
        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<FlowCapableNode>)isA(idtype),
                 isA(NodeListener.class))).
            thenReturn(nodeListenerReg);

        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<FlowCapableNodeConnector>)isA(idtype),
                 isA(NodeConnectorListener.class))).
            thenReturn(ncListenerReg);

        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<Link>)isA(idtype),
                 isA(TopologyListener.class))).
            thenReturn(topoListenerReg);

        Class<ClusteredDataTreeChangeListener> cltype =
            ClusteredDataTreeChangeListener.class;
        InstanceIdentifier<VtnNode> vpath = getPath();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<VtnNode> vident =
            new DataTreeIdentifier<>(oper, vpath);
        when(dataBroker.registerDataTreeChangeListener(
                 eq(vident), (DataTreeChangeListener<VtnNode>)isA(cltype))).
            thenReturn(inventoryListenerReg);

        inventoryManager = new VTNInventoryManager(vtnProvider);
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#VTNInventoryManager(VTNManagerProvider)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Ensure that NodeListner is registered as data change listener.
        InstanceIdentifier<FlowCapableNode> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class).
            augmentation(FlowCapableNode.class).
            build();
        DataTreeIdentifier<FlowCapableNode> ident =
            new DataTreeIdentifier<>(oper, path);
        verify(dataBroker).registerDataTreeChangeListener(
            eq(ident), isA(NodeListener.class));

        // Ensure that NodeConnectorListner is registered as data change
        // listener.
        InstanceIdentifier<FlowCapableNodeConnector> cpath = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class).
            child(NodeConnector.class).
            augmentation(FlowCapableNodeConnector.class).
            build();
        DataTreeIdentifier<FlowCapableNodeConnector> cident =
            new DataTreeIdentifier<>(oper, cpath);
        verify(dataBroker).registerDataTreeChangeListener(
            eq(cident), isA(NodeConnectorListener.class));

        // Ensure that TopologyListner is registered as data change listener.
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        InstanceIdentifier<Link> tpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class).
            build();
        DataTreeIdentifier<Link> tident = new DataTreeIdentifier<>(oper, tpath);
        verify(dataBroker).registerDataTreeChangeListener(
            eq(tident), isA(TopologyListener.class));

        // Ensure that VTNInventoryManager is registered as clustered data
        // change listener.
        InstanceIdentifier<VtnNode> vpath = getPath();
        DataTreeIdentifier<VtnNode> vident =
            new DataTreeIdentifier<>(oper, vpath);
        ArgumentCaptor<DataTreeChangeListener> captor =
            ArgumentCaptor.forClass(DataTreeChangeListener.class);
        verify(dataBroker).registerDataTreeChangeListener(
            eq(vident), (DataTreeChangeListener<VtnNode>)captor.capture());
        List<DataTreeChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        DataTreeChangeListener<?> dcl = wrappers.get(0);
        assertTrue(dcl instanceof ClusteredDataTreeChangeListener);
        assertEquals(inventoryManager,
                     getFieldValue(dcl, DataTreeChangeListener.class,
                                   "theListener"));

        verifyZeroInteractions(nodeListenerReg, ncListenerReg,
                               topoListenerReg, inventoryListenerReg);
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#addListener(VTNInventoryListener)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddListener() throws Exception {
        List<VTNInventoryListener> expected = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            expected.add(l);
            inventoryManager.addListener(l);

            List listeners =
                getFieldValue(inventoryManager, List.class, "vtnListeners");
            assertEquals(expected, listeners);
        }
    }

    /**
     * Test case for {@link VTNInventoryManager#shutdown()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testShutdown() throws Exception {
        ListenerRegistration[] regs = {
            nodeListenerReg,
            ncListenerReg,
            topoListenerReg,
            inventoryListenerReg,
        };

        for (ListenerRegistration reg: regs) {
            verifyZeroInteractions(reg);
        }

        List<VTNInventoryListener> expected = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            expected.add(l);
            inventoryManager.addListener(l);
        }

        List listeners =
            getFieldValue(inventoryManager, List.class, "vtnListeners");
        assertEquals(expected, listeners);

        inventoryManager.shutdown();

        listeners =
            getFieldValue(inventoryManager, List.class, "vtnListeners");
        expected = Collections.<VTNInventoryListener>emptyList();
        assertEquals(expected, listeners);

        // Data change listeners should be still active.
        for (ListenerRegistration reg: regs) {
            verifyZeroInteractions(reg);
        }
    }

    /**
     * Test case for {@link VTNInventoryManager#close()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClose() throws Exception {
        List<VTNInventoryListener> expected = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            expected.add(l);
            inventoryManager.addListener(l);
        }

        List listeners =
            getFieldValue(inventoryManager, List.class, "vtnListeners");
        assertEquals(expected, listeners);

        ListenerRegistration[] regs = {
            nodeListenerReg,
            ncListenerReg,
            topoListenerReg,
            inventoryListenerReg,
        };

        for (ListenerRegistration reg: regs) {
            verifyZeroInteractions(reg);
        }

        inventoryManager.close();
        for (ListenerRegistration reg: regs) {
            verify(reg).close();
        }

        listeners =
            getFieldValue(inventoryManager, List.class, "vtnListeners");
        expected = Collections.<VTNInventoryListener>emptyList();
        assertEquals(expected, listeners);

        // Listener registrations should never be closed twice.
        inventoryManager.close();
        for (ListenerRegistration reg: regs) {
            verify(reg).close();
        }
    }

    /**
     * Test case for {@link VTNInventoryManager#isDepth(VtnUpdateType)}.
     */
    @Test
    public void testGetOrder() {
        assertEquals(true, inventoryManager.isDepth(VtnUpdateType.CREATED));
        assertEquals(true, inventoryManager.isDepth(VtnUpdateType.CHANGED));
        assertEquals(false, inventoryManager.isDepth(VtnUpdateType.REMOVED));
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#enterEvent()}.
     */
    @Test
    public void testEnterEvent() {
        InventoryEvents ectx = inventoryManager.enterEvent();
        assertNotNull(ectx);
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#exitEvent(InventoryEvents)}.
     */
    @Test
    public void testExitEvent() {
        VTNInventoryListener[] listeners = {
            mock(VTNInventoryListener.class),
            mock(VTNInventoryListener.class),
            mock(VTNInventoryListener.class),
        };
        for (VTNInventoryListener l: listeners) {
            inventoryManager.addListener(l);
        }

        // No event should be delivered if InventoryEvents is empty.
        inventoryManager.exitEvent(new InventoryEvents());
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
    }

    /**
     * Ensure that creation events are delivered to
     * {@link VTNInventoryListener} if the process is the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onCreated(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnCreated() throws Exception {
        testOnCreatedOrRemoved(VtnUpdateType.CREATED, true);
    }

    /**
     * Ensure that creation event is never delivered to
     * {@link VTNInventoryListener} if the process is not the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onCreated(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnCreatedNotOwner() throws Exception {
        testOnCreatedOrRemoved(VtnUpdateType.CREATED, false);
    }

    /**
     * Ensure that update events are delivered to
     * {@link VTNInventoryListener} if the process is the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onUpdated(InventoryEvents,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnUpdated() throws Exception {
        testOnUpdated(true);
    }

    /**
     * Ensure that update event is never delivered to
     * {@link VTNInventoryListener} if the process is not the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onUpdated(InventoryEvents,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnUpdatedNotOwner() throws Exception {
        testOnUpdated(false);
    }

    /**
     * Ensure that removal events are delivered to
     * {@link VTNInventoryListener} if the process is the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onRemoved(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnRemoved() throws Exception {
        testOnCreatedOrRemoved(VtnUpdateType.REMOVED, true);
    }

    /**
     * Ensure that removal event is never delivered to
     * {@link VTNInventoryListener} if the process is not the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onRemoved(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnRemovedNotOwner() throws Exception {
        testOnCreatedOrRemoved(VtnUpdateType.REMOVED, false);
    }

    /**
     * Ensure that a data change event is processed correctly if the process
     * is the owner of the VTN inventory.
     *
     * <ul>
     *   <li>
     *     {@link VTNInventoryManager#onDataTreeChanged(Collection)}
     *   </li>
     *   <li>{@link VTNInventoryManager#onCreated(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#onUpdated(InventoryEvents,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#onRemoved(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * <p>
     *   This test updates vtn-inventory using put operation.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPutEvent() throws Exception {
        testEvent(true, false);
    }

    /**
     * Ensure that a data change event is processed correctly if the process
     * is the owner of the VTN inventory.
     *
     * <ul>
     *   <li>
     *     {@link VTNInventoryManager#onDataTreeChanged(Collection)}
     *   </li>
     *   <li>{@link VTNInventoryManager#onCreated(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#onUpdated(InventoryEvents,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#onRemoved(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * <p>
     *   This test updates vtn-inventory using merge operation.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMergeEvent() throws Exception {
        testEvent(true, true);
    }

    /**
     * Ensure that a data change event listener never delivers inventory event
     * if the process is not the owner of the VTN inventory.
     *
     * <ul>
     *   <li>
     *     {@link VTNInventoryManager#onDataTreeChanged(Collection)}
     *   </li>
     *   <li>{@link VTNInventoryManager#onCreated(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#onUpdated(InventoryEvents,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#onRemoved(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * <p>
     *   This test updates vtn-inventory using put operation.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPutEventNotOwner() throws Exception {
        testEvent(false, false);
    }

    /**
     * Ensure that a data change event listener never delivers inventory event
     * if the process is not the owner of the VTN inventory.
     *
     * <ul>
     *   <li>
     *     {@link VTNInventoryManager#onDataTreeChanged(Collection)}
     *   </li>
     *   <li>{@link VTNInventoryManager#onCreated(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#onUpdated(InventoryEvents,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#onRemoved(InventoryEvents,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * <p>
     *   This test updates vtn-inventory using merge operation.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMergeEventNotOwner() throws Exception {
        testEvent(false, true);
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        assertEquals(getPath(), inventoryManager.getWildcardPath());
    }

    /**
     * Test case for {@link VTNInventoryManager#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Logger logger = inventoryManager.getLogger();
        assertEquals(VTNInventoryManager.class.getName(), logger.getName());
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<VtnNode> getPath() {
        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class).build();
    }

    /**
     * Common test for creation and removal event delivery.
     *
     * @param utype  A {@link VtnUpdateType} instance.
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testOnCreatedOrRemoved(VtnUpdateType utype, boolean owner)
        throws Exception {
        reset(vtnProvider);

        Map<String, Set<NodeRpcInvocation<?, ?>>> expectedNodes =
            new HashMap<>();
        Map<String, Boolean> expectedPorts = new HashMap<>();

        VtnNodeManager nodeMgr = inventoryManager.getVtnNodeManager();
        Set<NodeRpcInvocation<?, ?>> emptyRpcs =
            Collections.<NodeRpcInvocation<?, ?>>emptySet();

        @SuppressWarnings("unchecked")
        ConcurrentMap<String, Boolean> portIds = (ConcurrentMap<String, Boolean>)
            getFieldValue(inventoryManager, ConcurrentMap.class, "portIds");

        SalNode snode = new SalNode(1L);
        SalPort esport = new SalPort(123L, 456L);
        SalPort isport = new SalPort(-1L, 0xffffff00L);
        SalPort dsport = new SalPort(4444L, 123L);
        boolean created = (utype == VtnUpdateType.CREATED);
        if (!created) {
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).
                build();
            assertEquals(snode.toString(), nodeMgr.add(vnode));
            expectedNodes.put(snode.toString(), emptyRpcs);

            portIds.put(esport.toString(), Boolean.TRUE);
            portIds.put(isport.toString(), Boolean.TRUE);
            portIds.put(dsport.toString(), Boolean.TRUE);
            expectedPorts.put(esport.toString(), Boolean.TRUE);
            expectedPorts.put(isport.toString(), Boolean.TRUE);
            expectedPorts.put(dsport.toString(), Boolean.TRUE);
        }

        final int nlisteners = 3;
        VTNInventoryListener[] listeners = new VTNInventoryListener[nlisteners];
        for (int i = 0; i < nlisteners; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            listeners[i] = l;
            inventoryManager.addListener(l);
        }

        // In case of node event.
        InventoryEvents ectx = new InventoryEvents();
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        VtnNode vnode = new VtnNodeBuilder().
            setId(snode.getNodeId()).
            setOpenflowVersion(VtnOpenflowVersion.OF13).
            build();
        IdentifiedData<VtnNode> ndata =
            new IdentifiedData<>(snode.getVtnNodeIdentifier(), vnode);
        if (created) {
            inventoryManager.onCreated(ectx, ndata);
            expectedNodes.put(snode.toString(), emptyRpcs);
        } else {
            inventoryManager.onRemoved(ectx, ndata);
            expectedNodes.remove(snode.toString());
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnNodeEvent> ncaptor =
                ArgumentCaptor.forClass(VtnNodeEvent.class);
            verify(vtnProvider, times(nlisteners)).post(ncaptor.capture());
            List<VtnNodeEvent> ndelivered = ncaptor.getAllValues();
            assertEquals(listeners.length, ndelivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnNodeEvent ev = ndelivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(snode, ev.getSalNode());
                assertSame(vnode, ev.getVtnNode());
                assertEquals(utype, ev.getUpdateType());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);

        // Same event should be ignored.
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, ndata);
        } else {
            inventoryManager.onRemoved(ectx, ndata);
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider, times(2)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case of edge port event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        VtnPort evport = new VtnPortBuilder().
            setId(esport.getNodeConnectorId()).
            setName("port-456").
            setEnabled(true).
            setCost(1000L).
            build();
        IdentifiedData<VtnPort> epdata =
            new IdentifiedData<>(esport.getVtnPortIdentifier(), evport);
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, epdata);
            expectedPorts.put(esport.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(ectx, epdata);
            expectedPorts.remove(esport.toString());
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnPortEvent> epcaptor =
                ArgumentCaptor.forClass(VtnPortEvent.class);
            verify(vtnProvider, times(nlisteners)).post(epcaptor.capture());
            List<VtnPortEvent> pdelivered = epcaptor.getAllValues();
            assertEquals(listeners.length, pdelivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnPortEvent ev = pdelivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(esport, ev.getSalPort());
                assertSame(evport, ev.getVtnPort());
                Boolean exIsl;
                Boolean exState;
                if (created) {
                    exIsl = Boolean.FALSE;
                    exState = Boolean.TRUE;
                } else {
                    exIsl = null;
                    exState = null;
                }
                assertEquals(exIsl, ev.getInterSwitchLinkChange());
                assertEquals(exState, ev.getStateChange());
                assertEquals(utype, ev.getUpdateType());
                assertEquals(!created, ev.isDisabled());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);

        // Same event should be ignored.
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, epdata);
        } else {
            inventoryManager.onRemoved(ectx, epdata);
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider, times(2)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case of inter-switch link port event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        PortLink plink = new PortLinkBuilder().
            setLinkId(new LinkId("link:1")).
            setPeer(new NodeConnectorId("openflow:3:4")).
            build();
        VtnPort ivport = new VtnPortBuilder().
            setId(isport.getNodeConnectorId()).
            setName("port-MAX").
            setEnabled(true).
            setCost(2000L).
            setPortLink(Collections.singletonList(plink)).
            build();
        IdentifiedData<VtnPort> ipdata =
            new IdentifiedData<>(isport.getVtnPortIdentifier(), ivport);
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, ipdata);
            expectedPorts.put(isport.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(ectx, ipdata);
            expectedPorts.remove(isport.toString());
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnPortEvent> ipcaptor =
                ArgumentCaptor.forClass(VtnPortEvent.class);
            verify(vtnProvider, times(nlisteners)).post(ipcaptor.capture());
            List<VtnPortEvent> pdelivered = ipcaptor.getAllValues();
            assertEquals(listeners.length, pdelivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnPortEvent ev = pdelivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(isport, ev.getSalPort());
                assertSame(ivport, ev.getVtnPort());
                Boolean exIsl;
                Boolean exState;
                if (created) {
                    exIsl = Boolean.TRUE;
                    exState = Boolean.TRUE;
                } else {
                    exIsl = null;
                    exState = null;
                }
                assertEquals(exIsl, ev.getInterSwitchLinkChange());
                assertEquals(exState, ev.getStateChange());
                assertEquals(utype, ev.getUpdateType());
                assertEquals(!created, ev.isDisabled());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);

        // Same event should be ignored.
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, ipdata);
        } else {
            inventoryManager.onRemoved(ectx, ipdata);
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider, times(2)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case of inactive port event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        VtnPort dvport = createVtnPortBuilder(dsport).
            setEnabled(false).
            setName("port-DOWN").
            build();
        IdentifiedData<VtnPort> dpdata =
            new IdentifiedData<>(dsport.getVtnPortIdentifier(), dvport);
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, dpdata);
            expectedPorts.put(dsport.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(ectx, dpdata);
            expectedPorts.remove(dsport.toString());
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnPortEvent> dpcaptor =
                ArgumentCaptor.forClass(VtnPortEvent.class);
            verify(vtnProvider, times(nlisteners)).post(dpcaptor.capture());
            List<VtnPortEvent> pdelivered = dpcaptor.getAllValues();
            assertEquals(listeners.length, pdelivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnPortEvent ev = pdelivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(dsport, ev.getSalPort());
                assertSame(dvport, ev.getVtnPort());
                Boolean exIsl;
                Boolean exState;
                if (created) {
                    exIsl = Boolean.FALSE;
                    exState = Boolean.FALSE;
                } else {
                    exIsl = null;
                    exState = null;
                }
                assertEquals(exIsl, ev.getInterSwitchLinkChange());
                assertEquals(exState, ev.getStateChange());
                assertEquals(utype, ev.getUpdateType());
                assertEquals(true, ev.isDisabled());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);

        // Same event should be ignored.
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, dpdata);
        } else {
            inventoryManager.onRemoved(ectx, dpdata);
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider, times(2)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case of unexpected event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        InstanceIdentifier<Node> badPath = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(new NodeId("unknown:1"))).
            build();
        Node badNode = mock(Node.class);
        IdentifiedData<Node> badData = new IdentifiedData<>(badPath, badNode);
        ectx = new InventoryEvents();
        if (created) {
            inventoryManager.onCreated(ectx, badData);
        } else {
            inventoryManager.onRemoved(ectx, badData);
        }
        inventoryManager.exitEvent(ectx);

        assertEquals(expectedNodes, nodeMgr.getNodeMap());
        assertEquals(expectedPorts, portIds);
        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider, badNode);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
        reset(vtnProvider);

        // No events should be delivered after shutdown.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        inventoryManager.shutdown();
        inventoryManager.exitEvent(ectx);
        inventoryManager.onCreated(ectx, ndata);
        inventoryManager.onCreated(ectx, epdata);
        inventoryManager.onCreated(ectx, ipdata);
        inventoryManager.onCreated(ectx, badData);
        inventoryManager.onRemoved(ectx, ndata);
        inventoryManager.onRemoved(ectx, epdata);
        inventoryManager.onRemoved(ectx, ipdata);
        inventoryManager.onRemoved(ectx, badData);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider, times(8)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
    }

    /**
     * Common test for update event delivery.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testOnUpdated(boolean owner) throws Exception {
        reset(vtnProvider);
        final int nlisteners = 3;
        VTNInventoryListener[] listeners = new VTNInventoryListener[nlisteners];
        for (int i = 0; i < nlisteners; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            listeners[i] = l;
            inventoryManager.addListener(l);
        }

        // In case of node event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        SalNode snode = new SalNode(1L);
        VtnNode vnodeOld = mock(VtnNode.class);
        when(vnodeOld.getId()).thenReturn(snode.getNodeId());
        when(vnodeOld.getOpenflowVersion()).
            thenReturn((VtnOpenflowVersion)null);
        VtnNode vnodeNew = mock(VtnNode.class);
        when(vnodeNew.getId()).thenReturn(snode.getNodeId());
        when(vnodeNew.getOpenflowVersion()).
            thenReturn(VtnOpenflowVersion.OF10);
        ChangedData<VtnNode> ndata = new ChangedData<>(
            snode.getVtnNodeIdentifier(), vnodeNew, vnodeOld);
        InventoryEvents ectx = new InventoryEvents();
        inventoryManager.onUpdated(ectx, ndata);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnNodeEvent> captor =
                ArgumentCaptor.forClass(VtnNodeEvent.class);
            verify(vtnProvider, never()).post(isA(VtnPortEvent.class));
            verify(vtnProvider, times(nlisteners)).post(captor.capture());
            List<VtnNodeEvent> delivered = captor.getAllValues();
            assertEquals(listeners.length, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnNodeEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(snode, ev.getSalNode());
                assertEquals(vnodeNew, ev.getVtnNode());
                assertEquals(VtnUpdateType.CHANGED, ev.getUpdateType());
                verifyZeroInteractions(listeners[i]);
            }

            // The change of OpenFlow version should be logged.
            verify(vnodeOld, never()).getId();
            verify(vnodeOld).getOpenflowVersion();
            verify(vnodeNew, times(2)).getId();
            verify(vnodeNew).getOpenflowVersion();
        }
        verifyNoMoreInteractions(vtnProvider, vnodeOld, vnodeNew);
        reset(vtnProvider);

        // Node change event should be ignored in the following cases.
        //   - The node is no changed.
        //   - The old node already contains OF version.
        //   - The new node does not contain OF version.
        VtnOpenflowVersion[][] verCases = {
            {null, null},
            {VtnOpenflowVersion.OF10, null},
            {VtnOpenflowVersion.OF13, null},
            {VtnOpenflowVersion.OF10, VtnOpenflowVersion.OF10},
            {VtnOpenflowVersion.OF13, VtnOpenflowVersion.OF13},
        };
        for (VtnOpenflowVersion[] vers: verCases) {
            when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).
                thenReturn(owner);
            vnodeOld = mock(VtnNode.class);
            when(vnodeOld.getId()).thenReturn(snode.getNodeId());
            when(vnodeOld.getOpenflowVersion()).
                thenReturn(vers[0]);
            vnodeNew = mock(VtnNode.class);
            when(vnodeNew.getId()).thenReturn(snode.getNodeId());
            when(vnodeNew.getOpenflowVersion()).
                thenReturn(vers[1]);
            ndata = new ChangedData<>(snode.getVtnNodeIdentifier(), vnodeNew,
                                      vnodeOld);
            ectx = new InventoryEvents();
            inventoryManager.onUpdated(ectx, ndata);
            inventoryManager.exitEvent(ectx);

            for (VTNInventoryListener l: listeners) {
                verifyZeroInteractions(l);
            }
            verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
            if (owner) {
                verify(vnodeOld).getOpenflowVersion();
                verify(vnodeNew).getOpenflowVersion();
            }
            verifyNoMoreInteractions(vtnProvider, vnodeOld, vnodeNew);
            reset(vtnProvider);
        }

        // In case where the port name is enabled.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        SalPort sport = new SalPort(123L, 456L);
        VtnPort vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setEnabled(false).
            setCost(1000L).
            build();
        VtnPort vportNew = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-456").
            setEnabled(true).
            setCost(1000L).
            build();
        ChangedData<VtnPort> pdata1 = new ChangedData<>(
            sport.getVtnPortIdentifier(), vportNew, vportOld);
        ectx = new InventoryEvents();
        inventoryManager.onUpdated(ectx, pdata1);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnPortEvent> captor =
                ArgumentCaptor.forClass(VtnPortEvent.class);
            verify(vtnProvider, never()).post(isA(VtnNodeEvent.class));
            verify(vtnProvider, times(nlisteners)).post(captor.capture());
            List<VtnPortEvent> delivered = captor.getAllValues();
            assertEquals(listeners.length, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnPortEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(sport, ev.getSalPort());
                assertEquals(vportNew, ev.getVtnPort());
                assertEquals(null, ev.getInterSwitchLinkChange());
                assertEquals(Boolean.TRUE, ev.getStateChange());
                assertEquals(VtnUpdateType.CHANGED, ev.getUpdateType());
                assertEquals(false, ev.isDisabled());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case where the port link is changed.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        PortLink plink = new PortLinkBuilder().
            setLinkId(new LinkId("link:1")).
            setPeer(new NodeConnectorId("openflow:3:4")).
            build();
        vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-456").
            setPortLink(Collections.<PortLink>emptyList()).
            setEnabled(true).
            setCost(1000L).
            build();
        vportNew = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-456").
            setPortLink(Collections.singletonList(plink)).
            setEnabled(true).
            setCost(1000L).
            build();
        ChangedData<VtnPort> pdata2 = new ChangedData<>(
            sport.getVtnPortIdentifier(), vportNew, vportOld);
        ectx = new InventoryEvents();
        inventoryManager.onUpdated(ectx, pdata2);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnPortEvent> captor =
                ArgumentCaptor.forClass(VtnPortEvent.class);
            verify(vtnProvider, never()).post(isA(VtnNodeEvent.class));
            verify(vtnProvider, times(nlisteners)).post(captor.capture());
            List<VtnPortEvent> delivered = captor.getAllValues();
            assertEquals(listeners.length, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnPortEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(sport, ev.getSalPort());
                assertEquals(vportNew, ev.getVtnPort());
                assertEquals(Boolean.TRUE, ev.getInterSwitchLinkChange());
                assertEquals(null, ev.getStateChange());
                assertEquals(VtnUpdateType.CHANGED, ev.getUpdateType());
                assertEquals(false, ev.isDisabled());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case where the port link is deleted, and the port is disabled.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-456").
            setPortLink(Collections.singletonList(plink)).
            setEnabled(true).
            setCost(1000L).
            build();
        vportNew = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-456").
            setEnabled(false).
            setCost(1000L).
            build();
        ChangedData<VtnPort> pdata3 = new ChangedData<>(
            sport.getVtnPortIdentifier(), vportNew, vportOld);
        ectx = new InventoryEvents();
        inventoryManager.onUpdated(ectx, pdata3);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Verify delivered events.
            ArgumentCaptor<VtnPortEvent> captor =
                ArgumentCaptor.forClass(VtnPortEvent.class);
            verify(vtnProvider, never()).post(isA(VtnNodeEvent.class));
            verify(vtnProvider, times(nlisteners)).post(captor.capture());
            List<VtnPortEvent> delivered = captor.getAllValues();
            assertEquals(listeners.length, delivered.size());
            for (int i = 0; i < nlisteners; i++) {
                VtnPortEvent ev = delivered.get(i);
                assertEquals(listeners[i],
                             getFieldValue(ev, VTNInventoryListener.class,
                                           "listener"));
                assertEquals(sport, ev.getSalPort());
                assertEquals(vportNew, ev.getVtnPort());
                assertEquals(Boolean.FALSE, ev.getInterSwitchLinkChange());
                assertEquals(Boolean.FALSE, ev.getStateChange());
                assertEquals(VtnUpdateType.CHANGED, ev.getUpdateType());
                assertEquals(true, ev.isDisabled());
                verifyZeroInteractions(listeners[i]);
            }
        }
        verifyNoMoreInteractions(vtnProvider);
        reset(vtnProvider);

        // In case of unexpected event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        InstanceIdentifier<Node> badPath = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(new NodeId("unknown:1"))).
            build();
        Node badNode1 = mock(Node.class);
        Node badNode2 = mock(Node.class);
        ChangedData<Node> badData =
            new ChangedData<>(badPath, badNode1, badNode2);
        ectx = new InventoryEvents();
        inventoryManager.onUpdated(ectx, badData);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider, badNode1, badNode2);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
        reset(vtnProvider);

        // No events should be delivered after shutdown.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        inventoryManager.shutdown();
        ectx = new InventoryEvents();
        inventoryManager.onUpdated(ectx, ndata);
        inventoryManager.onUpdated(ectx, pdata1);
        inventoryManager.onUpdated(ectx, pdata2);
        inventoryManager.onUpdated(ectx, pdata3);
        inventoryManager.onUpdated(ectx, badData);
        inventoryManager.exitEvent(ectx);

        verify(vtnProvider, times(5)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
    }

    /**
     * Common test for data change event listener.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @param merge  Use merge operation if {@code true}.
     *               Use put operation if {@code false}.
     * @throws Exception  An error occurred.
     */
    private void testEvent(boolean owner, boolean merge) throws Exception {
        reset(vtnProvider);
        final int nlisteners = 3;
        VTNInventoryListener[] listeners = new VTNInventoryListener[nlisteners];
        for (int i = 0; i < nlisteners; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            listeners[i] = l;
            inventoryManager.addListener(l);
        }

        VtnInventoryBuilder inventory = new VtnInventoryBuilder();
        VtnInventoryBuilder oldInventory = new VtnInventoryBuilder();
        Map<Long, VtnNodeBuilder> nodeBuilders = new HashMap<>();

        // 3 nodes have been created.
        Set<SalNode> createdNodeIds = new HashSet<>();
        VtnOpenflowVersion[] vers = {
            null,
            VtnOpenflowVersion.OF10,
            VtnOpenflowVersion.OF13,
        };
        long dpid = 1L;
        for (VtnOpenflowVersion ver: vers) {
            SalNode snode = new SalNode(dpid++);
            inventory.createNode(snode).getBuilder().setOpenflowVersion(ver);
            assertEquals(true, createdNodeIds.add(snode));
        }

        // 2 nodes have been changed.
        Set<SalNode> changedNodeIds = new HashSet<>();
        dpid = 10L;
        for (VtnOpenflowVersion ver: vers) {
            if (ver != null) {
                SalNode snode = new SalNode(dpid++);
                oldInventory.createNode(snode);
                inventory.createNode(snode).getBuilder().
                    setOpenflowVersion(ver);
                assertEquals(true, changedNodeIds.add(snode));
            }
        }

        // 3 nodes have been removed.
        VtnNodeManager nodeMgr = inventoryManager.getVtnNodeManager();
        Set<SalNode> removedNodeIds = new HashSet<>();
        dpid = 100L;
        for (VtnOpenflowVersion ver: vers) {
            SalNode snode = new SalNode(dpid++);
            VtnNode vnode = oldInventory.createNode(snode).getBuilder().
                setOpenflowVersion(ver).
                build();
            assertEquals(true, removedNodeIds.add(snode));
            assertEquals(snode.toString(), nodeMgr.add(vnode));
        }

        Set<SalPort> createdEdgePortIds = new HashSet<>();
        Set<SalPort> createdInterPortIds = new HashSet<>();
        long peerDpid = 999L;
        long peerPort = 314L;
        for (long dp = 1L; dp <= 2L; dp++) {
            // 2 edge ports per node are created.
            for (long pnum = 1L; pnum <= 2L; pnum++) {
                SalPort sport = new SalPort(dp, pnum);
                inventory.createPort(sport).getBuilder().
                    setName("port-" + pnum).
                    setEnabled(true).
                    setCost(1000L);
                assertEquals(true, createdEdgePortIds.add(sport));
            }

            // 2 inter-switch link per node are created.
            for (long pnum = 10L; pnum <= 11L; pnum++) {
                SalPort sport = new SalPort(dp, pnum);
                SalPort peer = new SalPort(peerDpid, peerPort++);
                VtnPortBuildHelper phelper = inventory.createPort(sport);
                phelper.getBuilder().
                    setName("port-" + pnum).
                    setEnabled(true).
                    setCost(1000L);
                phelper.addPortLink(peer);
                assertEquals(true, createdInterPortIds.add(sport));
            }
        }

        // 2 ports has been enabled.
        SalPort[] enabledPortIds = {
            new SalPort(123L, 1L),
            new SalPort(11L, 123456789L),
        };
        for (SalPort sport: enabledPortIds) {
            String name = "port-" + sport.getPortNumber();
            oldInventory.createPort(sport).getBuilder().
                setName(name).
                setEnabled(false).
                setCost(1000L);
            inventory.createPort(sport).getBuilder().
                setName(name).
                setEnabled(true).
                setCost(1000L);
        }

        // 3 ports has been disabled.
        SalPort[] disabledPortIds = {
            new SalPort(-1L, 3333L),
            new SalPort(9988L, 776655L),
            new SalPort(10L, 34567890L),
        };
        for (SalPort sport: disabledPortIds) {
            String name = "port-" + sport.getPortNumber();
            oldInventory.createPort(sport).getBuilder().
                setName(name).
                setEnabled(true).
                setCost(1000L);
            inventory.createPort(sport).getBuilder().
                setName(name).
                setEnabled(false).
                setCost(1000L);
        }

        // 2 ports has become inter-link port.
        SalPort[] changedInterPortIds = {
            new SalPort(12345L, 6L),
            new SalPort(11L, 19283746L),
        };
        for (SalPort sport: changedInterPortIds) {
            String name = "port-" + sport.getPortNumber();
            SalPort peer = new SalPort(peerDpid, peerPort++);
            oldInventory.createPort(sport).getBuilder().
                setName(name).
                setEnabled(true).
                setCost(1000L);
            VtnPortBuildHelper phelper = inventory.createPort(sport);
            phelper.getBuilder().
                setName(name).
                setEnabled(true).
                setCost(1000L);
            phelper.addPortLink(peer);
        }

        // 3 port has become edge port.
        SalPort[] changedEdgePortIds = {
            new SalPort(9999L, 88L),
            new SalPort(999999L, 8888L),
            new SalPort(10L, 66666666L),
        };
        for (SalPort sport: changedEdgePortIds) {
            String name = "port-" + sport.getPortNumber();
            SalPort peer = new SalPort(peerDpid, peerPort++);
            VtnPortBuildHelper phelper = oldInventory.createPort(sport);
            phelper.getBuilder().
                setName(name).
                setEnabled(true).
                setCost(1000L);
            phelper.addPortLink(peer);
            inventory.createPort(sport).getBuilder().
                setName(name).
                setEnabled(true).
                setCost(1000L);
        }

        @SuppressWarnings("unchecked")
        ConcurrentMap<String, Boolean> portIds = (ConcurrentMap<String, Boolean>)
            getFieldValue(inventoryManager, ConcurrentMap.class, "portIds");

        // 4 ports have been removed.
        SalPort[] removedPortIds = {
            new SalPort(1111L, 222L),
            new SalPort(45L, 678L),
            new SalPort(11L, 77777777L),
            new SalPort(123456789L, 3L),
        };
        for (int i = 0; i < removedPortIds.length; i++) {
            SalPort sport = removedPortIds[i];
            String name = "port-" + sport.getPortNumber();
            boolean enabled = (i != 3);
            VtnPortBuildHelper phelper = oldInventory.createPort(sport);
            phelper.getBuilder().
                setName(name).
                setEnabled(enabled).
                setCost(1000L);
            if ((i & 1) == 0) {
                SalPort peer = new SalPort(peerDpid, peerPort++);
                phelper.addPortLink(peer);
            }

            inventory.createNode(sport.getSalNode());
            assertEquals(null, portIds.put(sport.toString(), Boolean.TRUE));
        }

        // Create 2 nodes and 3 ports per node that are not changed.
        dpid = 1000L;
        for (VtnOpenflowVersion ver: vers) {
            if (ver == null) {
                continue;
            }

            SalNode snode = new SalNode(dpid++);
            oldInventory.createNode(snode);
            inventory.createNode(snode).getBuilder().
                setOpenflowVersion(ver);
            oldInventory.createNode(snode).getBuilder().
                setOpenflowVersion(ver);
            for (long pnum = 1L; pnum <= 3L; pnum++) {
                SalPort sport = new SalPort(dpid, pnum);
                String name = "port-" + pnum;
                SalPort peer = new SalPort(peerDpid, peerPort++);
                VtnPortBuildHelper phelper = inventory.createPort(sport);
                phelper.getBuilder().
                    setName(name).
                    setEnabled(true).
                    setCost(10000L);
                phelper.addPortLink(peer);

                phelper = oldInventory.createPort(sport);
                phelper.getBuilder().
                    setName(name).
                    setEnabled(true).
                    setCost(10000L);
                phelper.addPortLink(peer);
            }
        }

        inventory.freeze();
        oldInventory.freeze();

        // Fix up MD-SAL notifications.
        Map<SalNode, VtnNodeEvent> createdNodes = new HashMap<>();
        for (SalNode snode: createdNodeIds) {
            VtnNode vnode = inventory.getNode(snode).build();
            VtnNodeEvent nev = new VtnNodeEvent(vnode, VtnUpdateType.CREATED);
            assertEquals(null, createdNodes.put(snode, nev));
        }

        Map<SalNode, VtnNodeEvent> changedNodes = new HashMap<>();
        for (SalNode snode: changedNodeIds) {
            VtnNode vnode = inventory.getNode(snode).build();
            VtnNodeEvent nev = new VtnNodeEvent(vnode, VtnUpdateType.CHANGED);
            assertEquals(null, changedNodes.put(snode, nev));
        }

        Map<SalNode, VtnNodeEvent> removedNodes = new HashMap<>();
        for (SalNode snode: removedNodeIds) {
            VtnNode vnode = oldInventory.getNode(snode).build();
            VtnNodeEvent nev = new VtnNodeEvent(vnode, VtnUpdateType.REMOVED);
            assertEquals(null, removedNodes.put(snode, nev));
        }

        Map<SalPort, VtnPortEvent> createdPorts = new HashMap<>();
        for (SalPort sport: createdEdgePortIds) {
            VtnPort vport = inventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, Boolean.TRUE, Boolean.FALSE, VtnUpdateType.CREATED);
            assertEquals(null, createdPorts.put(sport, pev));
        }
        for (SalPort sport: createdInterPortIds) {
            VtnPort vport = inventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, Boolean.TRUE, Boolean.TRUE, VtnUpdateType.CREATED);
            assertEquals(null, createdPorts.put(sport, pev));
        }

        Map<SalPort, VtnPortEvent> changedPorts = new HashMap<>();
        for (SalPort sport: enabledPortIds) {
            VtnPort vport = inventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, Boolean.TRUE, null, VtnUpdateType.CHANGED);
            assertEquals(null, changedPorts.put(sport, pev));
        }
        for (SalPort sport: disabledPortIds) {
            VtnPort vport = inventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, Boolean.FALSE, null, VtnUpdateType.CHANGED);
            assertEquals(null, changedPorts.put(sport, pev));
        }
        for (SalPort sport: changedInterPortIds) {
            VtnPort vport = inventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, null, Boolean.TRUE, VtnUpdateType.CHANGED);
            assertEquals(null, changedPorts.put(sport, pev));
        }
        for (SalPort sport: changedEdgePortIds) {
            VtnPort vport = inventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, null, Boolean.FALSE, VtnUpdateType.CHANGED);
            assertEquals(null, changedPorts.put(sport, pev));
        }

        Map<SalPort, VtnPortEvent> removedPorts = new HashMap<>();
        for (SalPort sport: removedPortIds) {
            VtnPort vport = oldInventory.getPort(sport).build();
            VtnPortEvent pev = new VtnPortEvent(
                vport, null, null, VtnUpdateType.REMOVED);
            assertEquals(null, removedPorts.put(sport, pev));
        }

        // Construct data tree modifications.
        Collection<DataTreeModification<VtnNode>> changes =
            inventory.createEvent(oldInventory, merge);

        // Notify data change event.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        inventoryManager.onDataTreeChanged(changes);

        if (owner) {
            // Verify delivered events and order.
            // Node creation events must come first.
            ArgumentCaptor<TxEvent> captor =
                ArgumentCaptor.forClass(TxEvent.class);
            int numNodeEvents =
                (createdNodes.size() + changedNodes.size() +
                 removedNodes.size()) * nlisteners;
            int numPortEvents =
                (createdPorts.size() + changedPorts.size() +
                 removedPorts.size()) * nlisteners;
            int numEvents = numNodeEvents + numPortEvents;
            verify(vtnProvider, times(numEvents)).post(captor.capture());
            List<TxEvent> delivered = captor.getAllValues();
            assertEquals(numEvents, delivered.size());
            Iterator<TxEvent> it = delivered.iterator();
            Map<SalNode, VtnNodeEvent> nodeEvents =
                new HashMap<>(createdNodes);
            for (int i = 0; i < createdNodes.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnNodeEvent);
                VtnNodeEvent nev = (VtnNodeEvent)tev;
                SalNode snode = nev.getSalNode();
                VtnNodeEvent expected = nodeEvents.remove(snode);
                assertNotNull(expected);
                for (int j = 0; j < nlisteners; j++) {
                    if (j != 0) {
                        tev = it.next();
                        assertTrue(tev instanceof VtnNodeEvent);
                        nev = (VtnNodeEvent)tev;
                    }
                    assertEquals(listeners[j],
                                 getFieldValue(nev, VTNInventoryListener.class,
                                               "listener"));
                    assertEquals(snode, nev.getSalNode());
                    assertSame(expected.getVtnNode(), nev.getVtnNode());
                    assertEquals(VtnUpdateType.CREATED, nev.getUpdateType());
                }
            }
            assertTrue(nodeEvents.isEmpty());

            // The next must be node update events.
            nodeEvents = new HashMap<>(changedNodes);
            for (int i = 0; i < changedNodes.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnNodeEvent);
                VtnNodeEvent nev = (VtnNodeEvent)tev;
                SalNode snode = nev.getSalNode();
                VtnNodeEvent expected = nodeEvents.remove(snode);
                assertNotNull(expected);
                for (int j = 0; j < nlisteners; j++) {
                    if (j != 0) {
                        tev = it.next();
                        assertTrue(tev instanceof VtnNodeEvent);
                        nev = (VtnNodeEvent)tev;
                    }
                    assertEquals(listeners[j],
                                 getFieldValue(nev, VTNInventoryListener.class,
                                               "listener"));
                    assertEquals(snode, nev.getSalNode());
                    assertSame(expected.getVtnNode(), nev.getVtnNode());
                    assertEquals(VtnUpdateType.CHANGED, nev.getUpdateType());
                }
            }
            assertTrue(nodeEvents.isEmpty());

            // The next must be port creation events.
            Map<SalPort, VtnPortEvent> portEvents = new HashMap<>(createdPorts);
            for (int i = 0; i < createdPorts.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnPortEvent);
                VtnPortEvent pev = (VtnPortEvent)tev;
                SalPort sport = pev.getSalPort();
                VtnPortEvent expected = portEvents.remove(sport);
                assertNotNull(expected);
                for (int j = 0; j < nlisteners; j++) {
                    if (j != 0) {
                        tev = it.next();
                        assertTrue(tev instanceof VtnPortEvent);
                        pev = (VtnPortEvent)tev;
                    }
                    assertEquals(listeners[j],
                                 getFieldValue(pev, VTNInventoryListener.class,
                                               "listener"));
                    assertEquals(sport, pev.getSalPort());
                    assertSame(expected.getVtnPort(), pev.getVtnPort());
                    assertEquals(expected.getInterSwitchLinkChange(),
                                 pev.getInterSwitchLinkChange());
                    assertEquals(expected.getStateChange(),
                                 pev.getStateChange());
                    assertEquals(VtnUpdateType.CREATED, pev.getUpdateType());
                    assertEquals(expected.isDisabled(), pev.isDisabled());
                }
            }
            assertTrue(portEvents.isEmpty());

            // The next must be port removal events.
            portEvents = new HashMap<>(removedPorts);
            for (int i = 0; i < removedPorts.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnPortEvent);
                VtnPortEvent pev = (VtnPortEvent)tev;
                SalPort sport = pev.getSalPort();
                VtnPortEvent expected = portEvents.remove(sport);
                assertNotNull(expected);
                for (int j = 0; j < nlisteners; j++) {
                    if (j != 0) {
                        tev = it.next();
                        assertTrue(tev instanceof VtnPortEvent);
                        pev = (VtnPortEvent)tev;
                    }
                    assertEquals(listeners[j],
                                 getFieldValue(pev, VTNInventoryListener.class,
                                               "listener"));
                    assertEquals(sport, pev.getSalPort());
                    assertSame(expected.getVtnPort(), pev.getVtnPort());
                    assertEquals(expected.getInterSwitchLinkChange(),
                                 pev.getInterSwitchLinkChange());
                    assertEquals(expected.getStateChange(),
                                 pev.getStateChange());
                    assertEquals(VtnUpdateType.REMOVED, pev.getUpdateType());
                    assertEquals(true, pev.isDisabled());
                }
            }
            assertTrue(portEvents.isEmpty());

            // The next must be node removal events.
            nodeEvents = new HashMap<>(removedNodes);
            for (int i = 0; i < removedNodes.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnNodeEvent);
                VtnNodeEvent nev = (VtnNodeEvent)tev;
                SalNode snode = nev.getSalNode();
                VtnNodeEvent expected = nodeEvents.remove(snode);
                assertNotNull(expected);
                for (int j = 0; j < nlisteners; j++) {
                    if (j != 0) {
                        tev = it.next();
                        assertTrue(tev instanceof VtnNodeEvent);
                        nev = (VtnNodeEvent)tev;
                    }
                    assertEquals(listeners[j],
                                 getFieldValue(nev, VTNInventoryListener.class,
                                               "listener"));
                    assertEquals(snode, nev.getSalNode());
                    assertSame(expected.getVtnNode(), nev.getVtnNode());
                    assertEquals(VtnUpdateType.REMOVED, nev.getUpdateType());
                }
            }
            assertTrue(nodeEvents.isEmpty());

            // Port update events must come last.
            portEvents = new HashMap<>(changedPorts);
            for (int i = 0; i < changedPorts.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnPortEvent);
                VtnPortEvent pev = (VtnPortEvent)tev;
                SalPort sport = pev.getSalPort();
                VtnPortEvent expected = portEvents.remove(sport);
                assertNotNull(expected);
                for (int j = 0; j < nlisteners; j++) {
                    if (j != 0) {
                        tev = it.next();
                        assertTrue(tev instanceof VtnPortEvent);
                        pev = (VtnPortEvent)tev;
                    }
                    assertEquals(listeners[j],
                                 getFieldValue(pev, VTNInventoryListener.class,
                                               "listener"));
                    assertEquals(sport, pev.getSalPort());
                    assertSame(expected.getVtnPort(), pev.getVtnPort());
                    assertEquals(expected.getInterSwitchLinkChange(),
                                 pev.getInterSwitchLinkChange());
                    assertEquals(expected.getStateChange(),
                                 pev.getStateChange());
                    assertEquals(VtnUpdateType.CHANGED, pev.getUpdateType());
                    assertEquals(expected.isDisabled(), pev.isDisabled());
                }
            }
            assertTrue(portEvents.isEmpty());
            assertFalse(it.hasNext());
        }

        int count = createdNodes.size() + changedNodes.size() +
            removedNodes.size() + createdPorts.size() + changedPorts.size() +
            removedPorts.size();
        verify(vtnProvider, times(count)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
    }
}
