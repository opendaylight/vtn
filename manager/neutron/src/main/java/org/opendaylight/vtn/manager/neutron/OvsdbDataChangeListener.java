/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import com.google.common.base.Preconditions;
import com.google.common.base.Predicates;
import com.google.common.collect.Maps;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbBridgeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbNodeAugmentation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.params.xml.ns.yang.ovsdb.rev150105.OvsdbTerminationPointAugmentation;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class OvsdbDataChangeListener implements AutoCloseable, DataChangeListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(OvsdbDataChangeListener.class);

    /**
     * DataBroker Object to perform MDSAL operation.
     */
    private DataBroker dataBroker = null;
    /**
     * OVSDBEventHandler instance.
     */
    private OVSDBEventHandler ovsdbeventHandler = null;

    /**
     * Class constructor setting the data broker.
     *
     * @param dataBroker the {@link org.opendaylight.controller.md.sal.binding.api.DataBroker}
     */
    public OvsdbDataChangeListener(DataBroker dataBroker) {
        LOG.trace("OvsdbDataChangeListener initiated()");
        this.dataBroker = dataBroker;
        InstanceIdentifier<Node> path = InstanceIdentifier
            .create(NetworkTopology.class)
            .child(Topology.class, new TopologyKey(new TopologyId(new Uri("ovsdb:1"))))
            .child(Node.class);
        dataBroker.registerDataChangeListener(LogicalDatastoreType.OPERATIONAL, path, this, DataChangeScope.SUBTREE);
        ovsdbeventHandler = new OVSDBEventHandler(dataBroker);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void close() throws Exception {
    }

    /**
     * This method is called on Node or Port addition in the OVSDB Topology
     * @param changes  
     */
    @Override
    public void onDataChanged(
         AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        LOG.trace("onDataChanged: {}", changes);
        processOvsdbConnections(changes);
        processOvsdbConnectionAttributeUpdates(changes);
    }

    /**
     * Method invoked when the open flow switch is Added.
     * @param changes
     */
    private void processOvsdbConnections(AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (Map.Entry<InstanceIdentifier<?>, DataObject> created : changes.getCreatedData().entrySet()) {
            if (created.getValue() instanceof OvsdbNodeAugmentation) {
                Node ovsdbNode = getNode(changes.getCreatedData(), created);
                LOG.info("process Ovsdb Node: <{}>, ovsdbNode: <{}>", created, ovsdbNode);
            }
        }
    }

    /**
     * Method invoked when the open flow switch is Updated.
     * @param changes
     */
    private void processOvsdbConnectionAttributeUpdates(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {

        for (Map.Entry<InstanceIdentifier<?>, DataObject> updatedOvsdbNode : changes.getUpdatedData().entrySet()) {
            if (updatedOvsdbNode.getKey().getTargetType().equals(OvsdbNodeAugmentation.class)){
                LOG.trace("processOvsdbConnectionAttributeUpdates: {}", updatedOvsdbNode);
                Node parentNode  = getNode(changes.getUpdatedData(), updatedOvsdbNode);
                if (parentNode == null) {
                    // Logging this warning, to catch any change in southbound plugin's behavior.
                    LOG.warn("Parent Node for OvsdbNodeAugmentation is not found. On OvsdbNodeAugmentation update "
                            + "data store must provide the parent node update. This condition should not occur "
                            + "with the existing models defined in southbound plugin." );
                    continue;
                }
                LOG.info("processOvsdbConnectionAttributeUpdates <{}> related update on Node: <{}>",
                        updatedOvsdbNode.getValue(), parentNode);

                ovsdbeventHandler.nodeAdded(parentNode, updatedOvsdbNode.getValue());
            }
        }
    }

    /**
     * Method invoked to get the node details from the DataObject.
     * @param changes
     * @param change
     */
    private Node getNode(Map<InstanceIdentifier<?>, DataObject> changes,
                         Map.Entry<InstanceIdentifier<?>, DataObject> change) {
        InstanceIdentifier<Node> nodeInstanceIdentifier = change.getKey().firstIdentifierOf(Node.class);
        return (Node)changes.get(nodeInstanceIdentifier);
    }

    /**
     *  Method invoked to get the node details from the path.
     * @param changes
     * @param path
     */
    private Node getNode(Map<InstanceIdentifier<?>, DataObject> changes,InstanceIdentifier<?> path) {
        InstanceIdentifier<Node> nodeInstanceIdentifier = path.firstIdentifierOf(Node.class);
        return (Node)changes.get(nodeInstanceIdentifier);
    }

}
