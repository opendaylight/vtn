/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import java.util.Map;
import java.util.Map.Entry;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.concepts.ListenerRegistration;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;

public final class OvsdbDataChangeListener
    implements AutoCloseable, DataChangeListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(OvsdbDataChangeListener.class);

    /**
     * OVSDBEventHandler instance.
     */
    private OVSDBEventHandler ovsdbeventHandler;

    /**
     * Registration of the data change listener.
     */
    private ListenerRegistration<DataChangeListener>  listenerRegistration;

    /**
     * Class constructor setting the data broker.
     *
     * @param db   A {@link DataBroker} instance.
     * @param md   A {@link MdsalUtils} instance.
     * @param vtn  A {@link VTNManagerService} instance.
     */
    public OvsdbDataChangeListener(DataBroker db, MdsalUtils md,
                                   VTNManagerService vtn) {
        InstanceIdentifier<Node> path = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class,
                  new TopologyKey(new TopologyId(new Uri("ovsdb:1")))).
            child(Node.class).
            build();
        listenerRegistration = db.registerDataChangeListener(
            LogicalDatastoreType.OPERATIONAL, path, this,
            DataChangeScope.SUBTREE);
        ovsdbeventHandler = new OVSDBEventHandler(md, vtn);
    }

    /**
     * Close the OVSDB data change listener.
     */
    @Override
    public void close() {
        listenerRegistration.close();
    }

    /**
     * This method is called on Node or Port addition in the OVSDB Topology
     */
    @Override
    public void onDataChanged(
         AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        LOG.trace("onDataChanged: {}", changes);
        processOvsdbConnections(changes);
        processPortCreation(changes);
        processPortDeletion(changes);
        processOvsdbDisConnected(changes);
    }

    /**
     * Method invoked when the ovs switch gets connected to ODL
     * @param changes
     */
    private void processOvsdbConnections(AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (Entry<InstanceIdentifier<?>, DataObject> created : changes.getCreatedData().entrySet()) {
            if (created.getValue() instanceof OvsdbNodeAugmentation) {
                Node parentNode = getNode(changes.getCreatedData(), created);
                if (parentNode == null) {
                    continue;
                } else {
                    LOG.trace("process Ovsdb Node: <{}>, ovsdbNode: <{}>", created, parentNode);
                    ovsdbeventHandler.nodeAdded(parentNode, created.getValue());
                }
            }
        }
    }

    /**
     * Method invoked when the ovs switch gets disconnected from ODL
     * @param changes
     */
    private void processOvsdbDisConnected(AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (InstanceIdentifier<?> removed : changes.getRemovedPaths()) {
            if (removed.getTargetType().equals(OvsdbNodeAugmentation.class)) {
                Node parentNode = getNode(changes.getOriginalData(), removed);
                if (parentNode  == null) {
                    continue;
                } else {
                    ovsdbeventHandler.nodeRemoved(parentNode);
                }
            }
        }
    }

    /**
     * Method invoked to get the node details from the DataObject.
     * @param changes
     * @param change
     */
    private Node getNode(Map<InstanceIdentifier<?>, DataObject> changes,
                         Entry<InstanceIdentifier<?>, DataObject> change) {
        InstanceIdentifier<Node> nodeInstanceIdentifier = change.getKey().firstIdentifierOf(Node.class);
        return (Node)changes.get(nodeInstanceIdentifier);
    }

    /**
     *  Method invoked to get the node details from the path.
     * @param changes
     * @param path
     */
    private Node getNode(Map<InstanceIdentifier<?>, DataObject> changes, InstanceIdentifier<?> path) {
        InstanceIdentifier<Node> nodeInstanceIdentifier = path.firstIdentifierOf(Node.class);
        return (Node)changes.get(nodeInstanceIdentifier);
    }

    /**
     * Method invoked when the port is created.
     * @param changes
     */
    private void processPortCreation(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (Entry<InstanceIdentifier<?>, DataObject> newPort : changes.getCreatedData().entrySet()) {
            if (newPort.getKey().getTargetType().equals(OvsdbTerminationPointAugmentation.class)) {
                try {
                    LOG.trace("processPortCreation: port details {}", newPort);
                    //If user created termination point only, Node will get updated
                    Node tpParentNode  = getNode(changes.getCreatedData(), newPort);
                    if (tpParentNode == null) {
                        tpParentNode  = getNode(changes.getUpdatedData(), newPort);
                    }
                    if (tpParentNode != null) {
                        ovsdbeventHandler.readOVSDBPorts(tpParentNode, "added");
                    }
                } catch (Exception ex) {
                    LOG.error("exception obtained {}", ex);
                }
            }
        }
    }

    /**
     * Method invoked when the port is deleted.
     * @param changes
     */
    private void processPortDeletion(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {

        for (InstanceIdentifier<?> removedPort : changes.getRemovedPaths()) {
            if (removedPort.getTargetType().equals(OvsdbTerminationPointAugmentation.class)) {
                Node tpParentNode = getNode(changes.getOriginalData(), removedPort);
                if (tpParentNode == null) {
                    //Throwing this warning in case behavior of southbound plugin changes.
                    LOG.warn("Port's {} parent node details are not present in original data, "
                            + "it should not happen", removedPort);
                    continue;
                } else {
                    ovsdbeventHandler.readOVSDBPorts(tpParentNode , "delete");
                }
            }
        }
    }


}
