/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isA;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import static org.powermock.api.mockito.PowerMockito.mockStatic;
import static org.powermock.api.mockito.PowerMockito.verifyStatic;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;

import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentationBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;

/**
 * JUnit test for {@link OvsdbDataChangeListener}.
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({OVSDBEventHandler.class, OvsdbNodeChange.class})
public class OvsdbDataChangeListenerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * OVSDB event handler.
     */
    private OVSDBEventHandler  ovsdbHandler;

    /**
     * Registration to be associated with {@link OvsdbDataChangeListener}.
     */
    @Mock
    private ListenerRegistration<DataTreeChangeListener<Node>>  listenerReg;

    /**
     * An {@link OvsdbDataChangeListener} instance for test.
     */
    private OvsdbDataChangeListener  ovsdbListener;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);
        ovsdbHandler = PowerMockito.mock(OVSDBEventHandler.class);
        Class<DataTreeIdentifier> idtype = DataTreeIdentifier.class;
        Class<DataTreeChangeListener> ltype = DataTreeChangeListener.class;
        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<Node>)any(idtype),
                 (DataTreeChangeListener<Node>)isA(ltype))).
            thenReturn(listenerReg);
        ovsdbListener = new OvsdbDataChangeListener(dataBroker, ovsdbHandler);
    }

    /**
     * Test case for
     * {@link OvsdbDataChangeListener#OvsdbDataChangeListener(DataBroker,OVSDBEventHandler)}.
     */
    @Test
    public void testConstructor() {
        // Ensure that OvsdbDataChangeListener has been registered as
        // data tree change listener.
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<Node> ident =
            new DataTreeIdentifier<>(store, getPath());
        verify(dataBroker).
            registerDataTreeChangeListener(ident, ovsdbListener);
        verifyNoMoreInteractions(dataBroker, listenerReg, ovsdbHandler);
    }

    /**
     * Test case for {@link OvsdbDataChangeListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);

        // Close the listener.
        ovsdbListener.close();
        verify(listenerReg).close();
        verifyNoMoreInteractions(listenerReg);
    }

    /**
     * Test case for
     * {@link OvsdbDataChangeListener#onDataTreeChanged(java.util.Collection)}.
     */
    @Test
    public void testOnDataTreeChanged() {
        mockStatic(OvsdbNodeChange.class);

        List<DataTreeModification<Node>> changes = new ArrayList<>();
        Map<Node, OvsdbNodeChange> createdNodes = new HashMap<>();
        Map<DataObjectModification<Node>, OvsdbNodeChange> updatedNodes =
            new HashMap<>();
        Map<Node, OvsdbNodeChange> deletedNodes = new HashMap<>();

        // 1 node has been created.
        Node node = newNode("ovsdb:node1");
        changes.add(newModification(null, node));
        OvsdbNodeChange ovchg = PowerMockito.mock(OvsdbNodeChange.class);
        when(OvsdbNodeChange.nodeCreated(
                 eq(ovsdbHandler), isA(ReadTransactionHolder.class), eq(node))).
            thenReturn(ovchg);
        createdNodes.put(node, ovchg);

        // 1 node has been updated.
        String nid = "ovsdb:node2";
        Node before = newNode(nid);
        Node after = newOvsdbNode(nid);
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, null);
        changes.add(newModification(mod));
        ovchg = PowerMockito.mock(OvsdbNodeChange.class);
        when(OvsdbNodeChange.nodeUpdated(
                 eq(ovsdbHandler), isA(ReadTransactionHolder.class), eq(mod),
                 eq(before), eq(after))).
            thenReturn(ovchg);
        updatedNodes.put(mod, ovchg);

        // 1 node has been deleted.
        node = newNode("ovsdb:node3");
        changes.add(newModification(node, null));
        ovchg = PowerMockito.mock(OvsdbNodeChange.class);
        when(OvsdbNodeChange.nodeRemoved(
                 eq(ovsdbHandler), isA(ReadTransactionHolder.class), eq(node))).
            thenReturn(ovchg);
        deletedNodes.put(node, ovchg);

        // Null node should be ignored.
        nid = "ovsdb:node-null";
        NodeKey nodeKey = new NodeKey(new NodeId(nid));
        TopologyKey topoKey =
            new TopologyKey(new TopologyId(new Uri("ovsdb:1")));
        InstanceIdentifier<Node> path = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Node.class, nodeKey).
            build();
        mod = newKeyedModification(
            Node.class, ModificationType.WRITE, nodeKey, null, null, null);
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        changes.add(newTreeModification(path, store, mod));

        mod = newKeyedModification(
            Node.class, ModificationType.SUBTREE_MODIFIED, nodeKey, before,
            null, null);
        changes.add(newTreeModification(path, store, mod));

        mod = newKeyedModification(
            Node.class, ModificationType.DELETE, nodeKey, null, null, null);
        changes.add(newTreeModification(path, store, mod));

        mod = newKeyedModification(
            Node.class, ModificationType.DELETE, nodeKey, null, after, null);
        changes.add(newTreeModification(path, store, mod));

        // 3 more nodes have been deleted.
        for (int i = 1; i <= 3; i++) {
            nid = "ovsdb:node-del:" + i;
            node = newNode(nid);
            changes.add(newModification(node, null));
            ovchg = PowerMockito.mock(OvsdbNodeChange.class);
            when(OvsdbNodeChange.nodeRemoved(
                     eq(ovsdbHandler), isA(ReadTransactionHolder.class),
                     eq(node))).
                thenReturn(ovchg);
            deletedNodes.put(node, ovchg);
        }

        // 5 more nodes have been created.
        for (int i = 1; i <= 5; i++) {
            nid = "ovsdb:node-add:" + i;
            node = newNode(nid);
            changes.add(newModification(null, node));
            ovchg = PowerMockito.mock(OvsdbNodeChange.class);
            when(OvsdbNodeChange.nodeCreated(
                     eq(ovsdbHandler), isA(ReadTransactionHolder.class),
                     eq(node))).
                thenReturn(ovchg);
            createdNodes.put(node, ovchg);
        }

        // 4 more nodes have been updated.
        for (int i = 1; i <= 4; i++) {
            nid = "ovsdb:node-change:" + i;
            before = newNode(nid);
            after = newOvsdbNode(nid);
            mod = newKeyedModification(before, after, null);
            changes.add(newModification(mod));
            ovchg = PowerMockito.mock(OvsdbNodeChange.class);
            when(OvsdbNodeChange.nodeUpdated(
                     eq(ovsdbHandler), isA(ReadTransactionHolder.class),
                     eq(mod), eq(before), eq(after))).
                thenReturn(ovchg);
            updatedNodes.put(mod, ovchg);
        }

        // Notify changes.
        ovsdbListener.onDataTreeChanged(changes);

        ReadTransactionHolder holder = null;
        for (Entry<Node, OvsdbNodeChange> entry: createdNodes.entrySet()) {
            node = entry.getKey();
            ovchg = entry.getValue();
            verifyStatic();
            ArgumentCaptor<ReadTransactionHolder> captor =
                ArgumentCaptor.forClass(ReadTransactionHolder.class);
            OvsdbNodeChange.nodeCreated(
                eq(ovsdbHandler), captor.capture(), eq(node));
            List<ReadTransactionHolder> args = captor.getAllValues();
            assertEquals(1, args.size());
            ReadTransactionHolder txh = args.get(0);
            assertNotNull(txh);
            if (holder == null) {
                holder = txh;
            } else {
                assertSame(holder, txh);
            }

            if (ovchg != null) {
                verify(ovchg).apply();
                verifyNoMoreInteractions(ovchg);
            }
        }

        for (Entry<DataObjectModification<Node>, OvsdbNodeChange> entry:
                 updatedNodes.entrySet()) {
            mod = entry.getKey();
            ovchg = entry.getValue();
            before = mod.getDataBefore();
            after = mod.getDataAfter();
            verifyStatic();
            OvsdbNodeChange.
                nodeUpdated(ovsdbHandler, holder, mod, before, after);

            if (ovchg != null) {
                verify(ovchg).apply();
                verifyNoMoreInteractions(ovchg);
            }
        }

        for (Entry<Node, OvsdbNodeChange> entry: deletedNodes.entrySet()) {
            node = entry.getKey();
            ovchg = entry.getValue();
            verifyStatic();
            OvsdbNodeChange.nodeRemoved(ovsdbHandler, holder, node);

            if (ovchg != null) {
                verify(ovchg).apply();
                verifyNoMoreInteractions(ovchg);
            }
        }

        verifyZeroInteractions(ovsdbHandler);
        PowerMockito.verifyNoMoreInteractions(OvsdbNodeChange.class);

        // Empty collection should be ignored.
        changes.clear();
        ovsdbListener.onDataTreeChanged(changes);
        verifyZeroInteractions(ovsdbHandler);
        PowerMockito.verifyNoMoreInteractions(OvsdbNodeChange.class);
    }

    /**
     * Test case for
     * {@link OvsdbDataChangeListener#onDataTreeChanged(java.util.Collection)}.
     *
     * <p>
     *   This method verifies that unexpected exception is caught.
     * </p>
     */
    @Test
    public void testOnDataTreeChangedError() {
        mockStatic(OvsdbNodeChange.class);

        Node node = newNode("ovsdb:node1");
        DataTreeModification<Node> change = newModification(null, node);
        IllegalStateException iae = new IllegalStateException();
        when(OvsdbNodeChange.nodeCreated(
                 eq(ovsdbHandler), isA(ReadTransactionHolder.class), eq(node))).
            thenThrow(iae);
        List<DataTreeModification<Node>> changes =
            Collections.singletonList(change);

        // Notify changes.
        ovsdbListener.onDataTreeChanged(changes);

        verifyStatic();
        OvsdbNodeChange.nodeCreated(
            eq(ovsdbHandler), isA(ReadTransactionHolder.class), eq(node));
        verifyZeroInteractions(ovsdbHandler);
        PowerMockito.verifyNoMoreInteractions(OvsdbNodeChange.class);
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<Node> getPath() {
        TopologyKey topoKey =
            new TopologyKey(new TopologyId(new Uri("ovsdb:1")));
        return InstanceIdentifier.builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Node.class).
            build();
    }

    /**
     * Create a new node instance.
     *
     * @param nid  The node identifier.
     * @return  A {@link Node} instance.
     */
    private Node newNode(String nid) {
        return new NodeBuilder().
            setNodeId(new NodeId(nid)).
            build();
    }

    /**
     * Create a new node instance with OVSDB augmentation.
     *
     * @param nid  The node identifier.
     * @return  A {@link Node} instance.
     */
    private Node newOvsdbNode(String nid) {
        OvsdbNodeAugmentation ovnode = new OvsdbNodeAugmentationBuilder().
            build();
        return new NodeBuilder().
            setNodeId(new NodeId(nid)).
            addAugmentation(OvsdbNodeAugmentation.class, ovnode).
            build();
    }

    /**
     * Create a new data tree modification that notifies changed node in the
     * OVSDB topology.
     *
     * @param mod  Data object modification for a node.
     * @return  A {@link DataTreeModification} instance.
     */
    private DataTreeModification<Node> newModification(
        DataObjectModification<Node> mod) {
        Node before = mod.getDataBefore();
        Node after = mod.getDataAfter();
        Node node = (before == null) ? after : before;
        TopologyKey topoKey =
            new TopologyKey(new TopologyId(new Uri("ovsdb:1")));
        InstanceIdentifier<Node> path = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Node.class, node.getKey()).
            build();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        return newTreeModification(path, store, mod);
    }

    /**
     * Create a new data tree modification that notifies changed node in the
     * OVSDB topology.
     *
     * @param before  Node instance before modification.
     * @param after   Node instance after modification.
     * @return  A {@link DataTreeModification} instance.
     */
    private DataTreeModification<Node> newModification(
        Node before, Node after) {
        Node node = (before == null) ? after : before;
        TopologyKey topoKey =
            new TopologyKey(new TopologyId(new Uri("ovsdb:1")));
        InstanceIdentifier<Node> path = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, topoKey).
            child(Node.class, node.getKey()).
            build();
        DataObjectModification<Node> mod =
            newKeyedModification(before, after, null);
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        return newTreeModification(path, store, mod);
    }
}
