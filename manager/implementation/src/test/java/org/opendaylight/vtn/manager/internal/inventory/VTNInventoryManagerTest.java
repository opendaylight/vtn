/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.any;
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
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
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
    private ListenerRegistration<DataChangeListener>  nodeListenerReg;

    /**
     * Registration to be associated with {@link NodeConnectorListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  ncListenerReg;

    /**
     * Registration to be associated with {@link TopologyListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  topoListenerReg;

    /**
     * Registration to be associated with {@link VTNInventoryManager}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  inventoryListenerReg;

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

        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 isA(NodeListener.class), any(DataChangeScope.class))).
            thenReturn(nodeListenerReg);

        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 isA(NodeConnectorListener.class), any(DataChangeScope.class))).
            thenReturn(ncListenerReg);

        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 isA(TopologyListener.class), any(DataChangeScope.class))).
            thenReturn(topoListenerReg);

        InstanceIdentifier<VtnNode> vnodePath = getPath();
        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), eq(vnodePath),
                 isA(ClusteredDataChangeListener.class),
                 any(DataChangeScope.class))).
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
        DataChangeScope scope = DataChangeScope.SUBTREE;

        // Ensure that NodeListner is registered as data change listener.
        InstanceIdentifier<FlowCapableNode> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class).
            augmentation(FlowCapableNode.class).
            build();
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(path), isA(NodeListener.class), eq(scope));

        // Ensure that NodeConnectorListner is registered as data change
        // listener.
        InstanceIdentifier<FlowCapableNodeConnector> cpath = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class).
            child(NodeConnector.class).
            augmentation(FlowCapableNodeConnector.class).
            build();
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(cpath), isA(NodeConnectorListener.class), eq(scope));

        // Ensure that TopologyListner is registered as data change listener.
        TopologyKey topoKey = new TopologyKey(new TopologyId("flow:1"));
        InstanceIdentifier<Link> tpath = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Link.class).
            build();
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(tpath), isA(TopologyListener.class), eq(scope));

        // Ensure that VTNInventoryManager is registered as clustered data
        // change listener.
        InstanceIdentifier<VtnNode> vpath = getPath();
        ArgumentCaptor<ClusteredDataChangeListener> captor =
            ArgumentCaptor.forClass(ClusteredDataChangeListener.class);
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(vpath), captor.capture(), eq(scope));
        List<ClusteredDataChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        ClusteredDataChangeListener cdcl = wrappers.get(0);
        assertEquals(inventoryManager,
                     getFieldValue(cdcl, DataChangeListener.class,
                                   "theListener"));

        verifyZeroInteractions(nodeListenerReg);
        verifyZeroInteractions(ncListenerReg);
        verifyZeroInteractions(topoListenerReg);
        verifyZeroInteractions(inventoryListenerReg);
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

        // Below calls should do nothing because listeners are already added.
        for (VTNInventoryListener l: expected) {
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
     * Test case for {@link VTNInventoryManager#getComparator()}.
     */
    @Test
    public void testGetComparator() {
        IdentifierTargetComparator comp = inventoryManager.getComparator();
        assertTrue(comp.getOrder(VtnNode.class).intValue() <
                   comp.getOrder(VtnPort.class).intValue());

        // Sort instance identifiers.
        InstanceIdentifier<VtnNode> nodePath1 =
            new SalNode(1L).getVtnNodeIdentifier();
        InstanceIdentifier<VtnNode> nodePath2 =
            new SalNode(2L).getVtnNodeIdentifier();
        InstanceIdentifier<VtnPort> portPath1 =
            new SalPort(1L, 1L).getVtnPortIdentifier();
        InstanceIdentifier<VtnPort> portPath2 =
            new SalPort(1L, 2L).getVtnPortIdentifier();
        List<InstanceIdentifier<? extends DataObject>> list = new ArrayList<>();
        Collections.addAll(list, nodePath1, portPath1, nodePath2, portPath2);

        Collections.sort(list, comp);
        assertEquals(VtnNode.class, list.get(0).getTargetType());
        assertEquals(VtnNode.class, list.get(1).getTargetType());
        assertEquals(VtnPort.class, list.get(2).getTargetType());
        assertEquals(VtnPort.class, list.get(3).getTargetType());
    }

    /**
     * Test case for {@link VTNInventoryManager#getOrder(VtnUpdateType)}.
     */
    @Test
    public void testGetOrder() {
        assertEquals(true, inventoryManager.getOrder(VtnUpdateType.CREATED));
        assertEquals(true, inventoryManager.getOrder(VtnUpdateType.CHANGED));
        assertEquals(false, inventoryManager.getOrder(VtnUpdateType.REMOVED));
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#enterEvent(AsyncDataChangeEvent)}.
     */
    @Test
    public void testEnterEvent() {
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev = null;
        assertEquals(null, inventoryManager.enterEvent(ev));
    }

    /**
     * Test case for
     * {@link VTNInventoryManager#exitEvent(Void)}.
     */
    @Test
    public void testExitEvent() {
        // exitEvent() should do nothing.
        inventoryManager.exitEvent(null);
    }

    /**
     * Ensure that creation events are delivered to
     * {@link VTNInventoryListener} if the process is the owner of the VTN
     * inventory.
     *
     * <ul>
     *   <li>{@link VTNInventoryManager#onCreated(Void,IdentifiedData)}</li>
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
     *   <li>{@link VTNInventoryManager#onCreated(Void,IdentifiedData)}</li>
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
     *   <li>{@link VTNInventoryManager#onUpdated(Void,ChangedData)}</li>
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
     *   <li>{@link VTNInventoryManager#onUpdated(Void,ChangedData)}</li>
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
     *   <li>{@link VTNInventoryManager#onRemoved(Void,IdentifiedData)}</li>
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
     *   <li>{@link VTNInventoryManager#onRemoved(Void,IdentifiedData)}</li>
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
     *     {@link VTNInventoryManager#onDataChanged(AsyncDataChangeEvent)}
     *   </li>
     *   <li>{@link VTNInventoryManager#onCreated(Void,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#onUpdated(Void,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#onRemoved(Void,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEvent() throws Exception {
        testEvent(true);
    }

    /**
     * Ensure that a data change event listener never delivers inventory event
     * if the process is not the owner of the VTN inventory.
     *
     * <ul>
     *   <li>
     *     {@link VTNInventoryManager#onDataChanged(AsyncDataChangeEvent)}
     *   </li>
     *   <li>{@link VTNInventoryManager#onCreated(Void,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#onUpdated(Void,ChangedData)}</li>
     *   <li>{@link VTNInventoryManager#onRemoved(Void,IdentifiedData)}</li>
     *   <li>{@link VTNInventoryManager#addListener(VTNInventoryListener)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEventNotOwner() throws Exception {
        testEvent(false);
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

        @SuppressWarnings("unchecked")
        ConcurrentMap<String, Boolean> nodeIds = (ConcurrentMap<String, Boolean>)
            getFieldValue(inventoryManager, ConcurrentMap.class, "nodeIds");
        @SuppressWarnings("unchecked")
        ConcurrentMap<String, Boolean> portIds = (ConcurrentMap<String, Boolean>)
            getFieldValue(inventoryManager, ConcurrentMap.class, "portIds");

        ConcurrentMap<String, Boolean> expectedNodes =
            new ConcurrentHashMap<>();
        ConcurrentMap<String, Boolean> expectedPorts =
            new ConcurrentHashMap<>();

        SalNode snode = new SalNode(1L);
        SalPort esport = new SalPort(123L, 456L);
        SalPort isport = new SalPort(-1L, 0xffffff00L);
        SalPort dsport = new SalPort(4444L, 123L);
        boolean created = (utype == VtnUpdateType.CREATED);
        if (!created) {
            nodeIds.put(snode.toString(), Boolean.TRUE);
            expectedNodes.put(snode.toString(), Boolean.TRUE);

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
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        VtnNode vnode = new VtnNodeBuilder().
            setId(snode.getNodeId()).
            setOpenflowVersion(VtnOpenflowVersion.OF13).
            build();
        IdentifiedData<VtnNode> ndata =
            new IdentifiedData<>(snode.getVtnNodeIdentifier(), vnode);
        if (created) {
            inventoryManager.onCreated(null, ndata);
            expectedNodes.put(snode.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(null, ndata);
            expectedNodes.remove(snode.toString());
        }

        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, ndata);
        } else {
            inventoryManager.onRemoved(null, ndata);
        }
        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, epdata);
            expectedPorts.put(esport.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(null, epdata);
            expectedPorts.remove(esport.toString());
        }

        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, epdata);
        } else {
            inventoryManager.onRemoved(null, epdata);
        }
        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, ipdata);
            expectedPorts.put(isport.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(null, ipdata);
            expectedPorts.remove(isport.toString());
        }

        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, ipdata);
        } else {
            inventoryManager.onRemoved(null, ipdata);
        }
        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, dpdata);
            expectedPorts.put(dsport.toString(), Boolean.TRUE);
        } else {
            inventoryManager.onRemoved(null, dpdata);
            expectedPorts.remove(dsport.toString());
        }

        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, dpdata);
        } else {
            inventoryManager.onRemoved(null, dpdata);
        }
        assertEquals(expectedNodes, nodeIds);
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
        if (created) {
            inventoryManager.onCreated(null, badData);
        } else {
            inventoryManager.onRemoved(null, badData);
        }

        assertEquals(expectedNodes, nodeIds);
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
        inventoryManager.onCreated(null, ndata);
        inventoryManager.onCreated(null, epdata);
        inventoryManager.onCreated(null, ipdata);
        inventoryManager.onCreated(null, badData);
        inventoryManager.onRemoved(null, ndata);
        inventoryManager.onRemoved(null, epdata);
        inventoryManager.onRemoved(null, ipdata);
        inventoryManager.onRemoved(null, badData);

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
        inventoryManager.onUpdated(null, ndata);

        // Node change event should never be delivered to listeners.
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // The change of OpenFlow version should be logged.
            verify(vnodeOld, never()).getId();
            verify(vnodeOld).getOpenflowVersion();
            verify(vnodeNew).getId();
            verify(vnodeNew).getOpenflowVersion();
        }
        verifyNoMoreInteractions(vtnProvider, vnodeOld, vnodeNew);
        reset(vtnProvider);

        // In case where the node is not changed.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        vnodeOld = mock(VtnNode.class);
        when(vnodeOld.getId()).thenReturn(snode.getNodeId());
        when(vnodeOld.getOpenflowVersion()).
            thenReturn((VtnOpenflowVersion)null);
        vnodeNew = mock(VtnNode.class);
        when(vnodeNew.getId()).thenReturn(snode.getNodeId());
        when(vnodeNew.getOpenflowVersion()).
            thenReturn((VtnOpenflowVersion)null);
        ndata = new ChangedData<>(snode.getVtnNodeIdentifier(), vnodeNew,
                                  vnodeOld);
        inventoryManager.onUpdated(null, ndata);

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
        inventoryManager.onUpdated(null, pdata1);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Vefiry delivered events.
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
        inventoryManager.onUpdated(null, pdata2);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Vefiry delivered events.
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
        inventoryManager.onUpdated(null, pdata3);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            // Vefiry delivered events.
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
        inventoryManager.onUpdated(null, badData);

        verify(vtnProvider).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider, badNode1, badNode2);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
        reset(vtnProvider);

        // No events should be delivered after shutdown.
        when(vtnProvider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        inventoryManager.shutdown();
        inventoryManager.onUpdated(null, ndata);
        inventoryManager.onUpdated(null, pdata1);
        inventoryManager.onUpdated(null, pdata2);
        inventoryManager.onUpdated(null, pdata3);
        inventoryManager.onUpdated(null, badData);

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
     * @throws Exception  An error occurred.
     */
    private void testEvent(boolean owner) throws Exception {
        reset(vtnProvider);
        final int nlisteners = 3;
        VTNInventoryListener[] listeners = new VTNInventoryListener[nlisteners];
        for (int i = 0; i < nlisteners; i++) {
            VTNInventoryListener l = mock(VTNInventoryListener.class);
            listeners[i] = l;
            inventoryManager.addListener(l);
        }

        // 3 nodes have been created.
        Map<InstanceIdentifier<?>, DataObject> created = new HashMap<>();
        Map<SalNode, VtnNodeEvent> createdNodes = new HashMap<>();
        VtnOpenflowVersion[] vers = {
            null,
            VtnOpenflowVersion.OF10,
            VtnOpenflowVersion.OF13,
        };
        long dpid = 1L;
        for (VtnOpenflowVersion ver: vers) {
            SalNode snode = new SalNode(dpid);
            VtnNode vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).
                setOpenflowVersion(ver).
                build();
            VtnNodeEvent nev =
                new VtnNodeEvent(null, vnode, VtnUpdateType.CREATED);
            assertNull(createdNodes.put(snode, nev));
            assertNull(created.put(snode.getVtnNodeIdentifier(), vnode));
            dpid++;
        }

        // 2 nodes have been changed.
        Map<InstanceIdentifier<?>, DataObject> original = new HashMap<>();
        Map<InstanceIdentifier<?>, DataObject> updated = new HashMap<>();
        SalNode snode = new SalNode(10L);
        VtnNode vnodeOld = new VtnNodeBuilder().
            setId(snode.getNodeId()).
            build();
        VtnNode vnode = new VtnNodeBuilder().
            setId(snode.getNodeId()).
            setOpenflowVersion(VtnOpenflowVersion.OF10).
            build();
        InstanceIdentifier<VtnNode> vnPath = snode.getVtnNodeIdentifier();
        assertNull(original.put(vnPath, vnodeOld));
        assertNull(updated.put(vnPath, vnode));

        snode = new SalNode(11L);
        vnodeOld = new VtnNodeBuilder().
            setId(snode.getNodeId()).
            build();
        vnode = new VtnNodeBuilder().
            setId(snode.getNodeId()).
            setOpenflowVersion(VtnOpenflowVersion.OF13).
            build();
        vnPath = snode.getVtnNodeIdentifier();
        assertNull(original.put(vnPath, vnodeOld));
        assertNull(updated.put(vnPath, vnode));

        @SuppressWarnings("unchecked")
        ConcurrentMap<String, Boolean> nodeIds = (ConcurrentMap<String, Boolean>)
            getFieldValue(inventoryManager, ConcurrentMap.class, "nodeIds");

        // 3 nodes have been removed.
        Set<InstanceIdentifier<?>> removed = new HashSet<>();
        Map<SalNode, VtnNodeEvent> removedNodes = new HashMap<>();
        dpid = 100L;
        for (VtnOpenflowVersion ver: vers) {
            snode = new SalNode(dpid);
            vnode = new VtnNodeBuilder().
                setId(snode.getNodeId()).
                setOpenflowVersion(ver).
                build();
            VtnNodeEvent nev =
                new VtnNodeEvent(null, vnode, VtnUpdateType.REMOVED);
            vnPath = snode.getVtnNodeIdentifier();
            assertNull(removedNodes.put(snode, nev));
            assertTrue(removed.add(vnPath));
            assertNull(original.put(vnPath, vnode));
            assertNull(nodeIds.put(snode.toString(), Boolean.TRUE));
            dpid++;
        }

        Map<SalPort, VtnPortEvent> createdPorts = new HashMap<>();
        PortLink plink = new PortLinkBuilder().
            setLinkId(new LinkId("link:1")).
            setPeer(new NodeConnectorId("openflow:3:4")).
            build();
        List<PortLink> plinks = Collections.singletonList(plink);
        for (long dp = 1L; dp <= 2L; dp++) {
            // 2 edge ports per node are created.
            for (long pnum = 1L; pnum <= 2L; pnum++) {
                SalPort sport = new SalPort(dp, pnum);
                VtnPort vport = new VtnPortBuilder().
                    setId(sport.getNodeConnectorId()).
                    setName("port-" + pnum).
                    setEnabled(true).
                    setCost(1000L).
                    build();
                VtnPortEvent pev = new VtnPortEvent(
                    null, vport, Boolean.TRUE, Boolean.FALSE,
                    VtnUpdateType.CREATED);
                assertNull(createdPorts.put(sport, pev));
                assertNull(created.put(sport.getVtnPortIdentifier(), vport));
            }

            // 2 inter-switch link per node are created.
            for (long pnum = 10L; pnum <= 11L; pnum++) {
                SalPort sport = new SalPort(dp, pnum);
                VtnPort vport = new VtnPortBuilder().
                    setId(sport.getNodeConnectorId()).
                    setName("port-" + pnum).
                    setEnabled(true).
                    setPortLink(plinks).
                    setCost(1000L).
                    build();
                VtnPortEvent pev = new VtnPortEvent(
                    null, vport, Boolean.TRUE, Boolean.TRUE,
                    VtnUpdateType.CREATED);
                assertNull(createdPorts.put(sport, pev));
                assertNull(created.put(sport.getVtnPortIdentifier(), vport));
            }
        }

        // 1 port has been enabled.
        Map<SalPort, VtnPortEvent> changedPorts = new HashMap<>();
        SalPort sport = new SalPort(123L, 1L);
        VtnPort vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-1").
            setEnabled(false).
            setCost(1000L).
            build();
        VtnPort vport = new VtnPortBuilder(vportOld).
            setEnabled(true).
            build();
        VtnPortEvent pev =
            new VtnPortEvent(null, vport, Boolean.TRUE, null,
                             VtnUpdateType.CHANGED);
        InstanceIdentifier<VtnPort> vpPath = sport.getVtnPortIdentifier();
        assertNull(changedPorts.put(sport, pev));
        assertNull(original.put(vpPath, vportOld));
        assertNull(updated.put(vpPath, vport));

        // 1 port has been disabled.
        sport = new SalPort(-1L, 3333L);
        vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-3333").
            setEnabled(true).
            setCost(1000L).
            build();
        vport = new VtnPortBuilder(vportOld).
            setEnabled(false).
            build();
        pev = new VtnPortEvent(null, vport, Boolean.FALSE, null,
                               VtnUpdateType.CHANGED);
        vpPath = sport.getVtnPortIdentifier();
        assertNull(changedPorts.put(sport, pev));
        assertNull(original.put(vpPath, vportOld));
        assertNull(updated.put(vpPath, vport));

        // 1 port has become inter-link port.
        sport = new SalPort(12345L, 6L);
        vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-6").
            setEnabled(true).
            setCost(1000L).
            build();
        vport = new VtnPortBuilder(vportOld).
            setPortLink(plinks).
            build();
        pev = new VtnPortEvent(null, vport, null, Boolean.TRUE,
                               VtnUpdateType.CHANGED);
        vpPath = sport.getVtnPortIdentifier();
        assertNull(changedPorts.put(sport, pev));
        assertNull(original.put(vpPath, vportOld));
        assertNull(updated.put(vpPath, vport));

        // 1 port has become edge port.
        sport = new SalPort(9999L, 88L);
        vportOld = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-88").
            setEnabled(true).
            setPortLink(plinks).
            setCost(1000L).
            build();
        vport = new VtnPortBuilder(vportOld).
            setPortLink(null).
            build();
        pev = new VtnPortEvent(null, vport, null, Boolean.FALSE,
                               VtnUpdateType.CHANGED);
        vpPath = sport.getVtnPortIdentifier();
        assertNull(changedPorts.put(sport, pev));
        assertNull(original.put(vpPath, vportOld));
        assertNull(updated.put(vpPath, vport));

        @SuppressWarnings("unchecked")
        ConcurrentMap<String, Boolean> portIds = (ConcurrentMap<String, Boolean>)
            getFieldValue(inventoryManager, ConcurrentMap.class, "portIds");

        // 3 ports have been removed.
        Map<SalPort, VtnPortEvent> removedPorts = new HashMap<>();
        sport = new SalPort(1111L, 222L);
        vport = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-222").
            setEnabled(true).
            setPortLink(plinks).
            setCost(1000L).
            build();
        pev = new VtnPortEvent(null, vport, null, null, VtnUpdateType.REMOVED);
        vpPath = sport.getVtnPortIdentifier();
        assertNull(removedPorts.put(sport, pev));
        assertTrue(removed.add(sport.getVtnPortIdentifier()));
        assertNull(original.put(vpPath, vport));
        assertNull(portIds.put(sport.toString(), Boolean.TRUE));

        sport = new SalPort(45L, 678L);
        vport = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-678").
            setEnabled(true).
            setCost(1000L).
            build();
        pev = new VtnPortEvent(null, vport, null, null, VtnUpdateType.REMOVED);
        vpPath = sport.getVtnPortIdentifier();
        assertNull(removedPorts.put(sport, pev));
        assertTrue(removed.add(sport.getVtnPortIdentifier()));
        assertNull(original.put(vpPath, vport));
        assertNull(portIds.put(sport.toString(), Boolean.TRUE));

        sport = new SalPort(123456789L, 3L);
        vport = new VtnPortBuilder().
            setId(sport.getNodeConnectorId()).
            setName("port-3").
            setEnabled(false).
            setCost(10000L).
            build();
        pev = new VtnPortEvent(null, vport, null, null, VtnUpdateType.REMOVED);
        vpPath = sport.getVtnPortIdentifier();
        assertNull(removedPorts.put(sport, pev));
        assertTrue(removed.add(sport.getVtnPortIdentifier()));
        assertNull(original.put(vpPath, vport));
        assertNull(portIds.put(sport.toString(), Boolean.TRUE));

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
        inventoryManager.onDataChanged(event);

        if (owner) {
            // Verify delivered events and order.
            // Node creation events must come first.
            ArgumentCaptor<TxEvent> captor =
                ArgumentCaptor.forClass(TxEvent.class);
            int numNodeEvents =
                (createdNodes.size() + removedNodes.size()) * nlisteners;
            int numPortEvents =
                (createdPorts.size() + changedPorts.size() +
                 removedPorts.size()) *
                nlisteners;
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
                snode = nev.getSalNode();
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

            // The next must be port creation events.
            Map<SalPort, VtnPortEvent> portEvents = new HashMap<>(createdPorts);
            for (int i = 0; i < createdPorts.size(); i++) {
                TxEvent tev = it.next();
                assertTrue(tev instanceof VtnPortEvent);
                pev = (VtnPortEvent)tev;
                sport = pev.getSalPort();
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
                pev = (VtnPortEvent)tev;
                sport = pev.getSalPort();
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
                snode = nev.getSalNode();
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
                pev = (VtnPortEvent)tev;
                sport = pev.getSalPort();
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

        int count = created.size() + original.size();
        verify(vtnProvider, times(count)).isOwner(VTNEntityType.INVENTORY);
        verifyNoMoreInteractions(vtnProvider);
        for (VTNInventoryListener l: listeners) {
            verifyZeroInteractions(l);
        }
    }
}
