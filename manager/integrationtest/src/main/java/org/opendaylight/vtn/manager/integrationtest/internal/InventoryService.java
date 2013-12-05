/*
 * Copyright (c) 2013 Cisco Systems, Inc. and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.integrationtest.internal;

import java.util.Collection;
import java.util.Map;
import java.util.Set;
import java.util.HashMap;
import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArraySet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.opendaylight.controller.sal.core.Actions;
import org.opendaylight.controller.sal.core.Bandwidth;
import org.opendaylight.controller.sal.core.Buffers;
import org.opendaylight.controller.sal.core.Capabilities;
import org.opendaylight.controller.sal.core.Capabilities.CapabilitiesType;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.Tables;
import org.opendaylight.controller.sal.core.TimeStamp;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.inventory.IPluginInInventoryService;
import org.opendaylight.controller.sal.inventory.IPluginOutInventoryService;

/**
 * Stub Implementation for IPluginInReadService used by SAL
 *
 *
 */
public class InventoryService implements IPluginInInventoryService {
    private static final Logger logger = LoggerFactory
            .getLogger(InventoryService.class);

    private ConcurrentMap<Node, Map<String, Property>> nodeProps; // properties
                                                                  // are
                                                                  // maintained
                                                                  // in global
                                                                  // container
                                                                  // only
    private ConcurrentMap<NodeConnector, Map<String, Property>> nodeConnectorProps; // properties
                                                                                    // are
                                                                                    // maintained
                                                                                    // in
                                                                                    // global
                                                                                    // container
                                                                                    // only
    private final Set<IPluginOutInventoryService> pluginOutInventoryServices =
            new CopyOnWriteArraySet<IPluginOutInventoryService>();

    private InventoryService global = null;

    public void setPluginOutInventoryServices(IPluginOutInventoryService service) {
        logger.debug("Got a service set request {}", service);
        if (this.pluginOutInventoryServices != null) {
            this.pluginOutInventoryServices.add(service);
        }
    }

    public void unsetPluginOutInventoryServices(IPluginOutInventoryService service) {
        logger.debug("Got a service UNset request");
        if (this.pluginOutInventoryServices != null) {
            this.pluginOutInventoryServices.remove(service);
        }
    }

    public void setInstanceForGlobal(IPluginInInventoryService ivs) {
        logger.debug("Got the global service set request {}", ivs);
        if (ivs == null) {
            return;
        }

        if (ivs != this && ivs instanceof InventoryService) {
            this.global = (InventoryService) ivs;
        }
    }

    public void unsetInstanceForGlobal(IPluginInInventoryService ivs) {
        logger.debug("Got the global service UNset request");
        if (ivs == null) {
            return;
        }

        this.global = null;
    }

    /**
     * Function called by the dependency manager when all the required
     * dependencies are satisfied
     *
     */
    void init() {
        logger.trace("openflow stub InventoryService init called");
        nodeProps = new ConcurrentHashMap<Node, Map<String, Property>>();
        nodeConnectorProps = new ConcurrentHashMap<NodeConnector, Map<String, Property>>();

        setupNodeConnectorProps();
    }

    private static Map<String, Property> getDefaultConnectorProps() {
        Map<String, Property> ncPropMap = new HashMap<String, Property>();

        Capabilities cap = new Capabilities(
                CapabilitiesType.FLOW_STATS_CAPABILITY.getValue());
        ncPropMap.put(Capabilities.CapabilitiesPropName, cap);

        Bandwidth bw = new Bandwidth(Bandwidth.BW1Gbps);
        ncPropMap.put(Bandwidth.BandwidthPropName, bw);

        State st = new State(State.EDGE_UP);
        ncPropMap.put(State.StatePropName, st);

        Config cf = new Config(Config.ADMIN_UP);
        ncPropMap.put(Config.ConfigPropName, cf);

        return ncPropMap;
    }

    private static Map<String, Property> getDefaultNodeProps() {
        Map<String, Property> propMap = new HashMap<String, Property>();

        Tables t = new Tables((byte) 1);
        propMap.put(Tables.TablesPropName, t);
        Capabilities c = new Capabilities((int) 3);
        propMap.put(Capabilities.CapabilitiesPropName, c);
        Actions a = new Actions((int) 2);
        propMap.put(Actions.ActionsPropName, a);
        Buffers b = new Buffers((int) 1);
        propMap.put(Buffers.BuffersPropName, b);
        Long connectedSinceTime = 100000L;
        TimeStamp timeStamp = new TimeStamp(connectedSinceTime,
                "connectedSince");
        propMap.put(TimeStamp.TimeStampPropName, timeStamp);

        return propMap;
    }

    private static Map<String, Property> getDefaultConnectorProps(String portName) {
        Map<String, Property> ncPropMap = getDefaultConnectorProps();
        ncPropMap.put(Name.NamePropName, new Name(portName));
        return ncPropMap;
    }

    private static Map<String, Property> getDefaultConnectorProps(Short portId) {
        return getDefaultConnectorProps("port-" + Integer.toHexString((int)(portId.shortValue())));
    }

    private void setupNodeConnectorProps() {
        Map<String, Property> propMap = getDefaultNodeProps();

        // setup property map for all node connectors
        NodeConnector nc;
        Node node;
        Short portID = new Short((short)0xCAFE);
        try {
            node = new Node(Node.NodeIDType.OPENFLOW, new Long(0xCAFE));
            nc = new NodeConnector(Node.NodeIDType.OPENFLOW, portID, node);
        } catch (ConstructionException e) {
            nc = null;
            node = null;
        }
        nodeProps.put(node, propMap);
        nodeConnectorProps.put(nc, getDefaultConnectorProps(portID));

        portID = new Short((short)12);
        try {
            node = new Node(Node.NodeIDType.OPENFLOW, new Long(0x3366));
            nc = new NodeConnector(Node.NodeIDType.OPENFLOW, portID, node);
        } catch (ConstructionException e) {
            nc = null;
            node = null;
        }
        nodeProps.put(node, propMap);
        nodeConnectorProps.put(nc, getDefaultConnectorProps(portID));

        portID = new Short((short)34);
        try {
            node = new Node(Node.NodeIDType.OPENFLOW, new Long(0x4477));
            nc = new NodeConnector(Node.NodeIDType.OPENFLOW, portID, node);
        } catch (ConstructionException e) {
            nc = null;
            node = null;
        }
        nodeProps.put(node, propMap);
        nodeConnectorProps.put(nc, getDefaultConnectorProps(portID));
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     *
     */
    void destroy() {
    }

    /**
     * Function called by dependency manager after "init ()" is called and after
     * the services provided by the class are registered in the service registry
     *
     */
    void start() {
    }

    /**
     * Method called when the plugin has exposed it's services, this will be
     * used to publish the updates so connection manager can think the
     * connection is local
     */
    void started() {
        logger.trace("openflow stub InventoryService started called");
        // update sal and discovery
        for (IPluginOutInventoryService service : pluginOutInventoryServices) {
            logger.debug("Adding Node and NodeConnectors to service {}", service);
            for (Node node : nodeProps.keySet()) {
                Set<Property> props = new HashSet<Property>(nodeProps.get(node)
                        .values());
                service.updateNode(node, UpdateType.ADDED, props);
                logger.trace("Adding Node {} with props {}", node, props);
            }
            for (NodeConnector nc : nodeConnectorProps.keySet()) {
                Set<Property> props = new HashSet<Property>(nodeConnectorProps.get(nc)
                        .values());
                service.updateNodeConnector(nc, UpdateType.ADDED, props);
                logger.trace("Adding NodeConnectors {} with props {}", nc, props);
            }
        }
    }

    /**
     * Function called by the dependency manager before the services exported by
     * the component are unregistered, this will be followed by a "destroy ()"
     * calls
     *
     */
    void stop() {
        pluginOutInventoryServices.clear();
    }

    /*
     * Add node
     */
    public void addNode(Node node, Collection<NodeConnector> connectors) {
        logger.trace("openflow stub InventoryService" + ((this.global == null) ? "(global)" : "")
                + " addNode(Node, Collection) called");

        // sanity check
        if (node == null || connectors == null || connectors.isEmpty()) {
            return;
        }

        for (NodeConnector nc : connectors) {
            if (nc.getNode() != node) {
                logger.debug("There is NO relationship between specified Node and NodeConnector(s).");
                return;
            }
        }

        Map<String, Property> propMap = getDefaultNodeProps();

        nodeProps.put(node, propMap);
        for (IPluginOutInventoryService service : pluginOutInventoryServices) {
            Set<Property> props = new HashSet<Property>(propMap.values());
            service.updateNode(node, UpdateType.ADDED, props);
            logger.debug("Adding Node {} with props {} to {}", node, props, service);
        }

        Map<String, Property> ncPropMap = getDefaultConnectorProps();

        for (NodeConnector nc : connectors) {
            nodeConnectorProps.put(nc, ncPropMap);
            for (IPluginOutInventoryService service : pluginOutInventoryServices) {
                Set<Property> ncProps = new HashSet<Property>(ncPropMap.values());
                service.updateNodeConnector(nc, UpdateType.ADDED, ncProps);
                logger.debug("Adding NodeConnectors {} with props {} to {}", nc, ncProps, service);
            }
        }

        // Notify to global
        if (this.global != null) {
            this.global.addNode(node, connectors);
        }
    }

    /*
     * Add node
     */
    public void addNode(Map<Node, Collection<NodeConnector>> mapConnectors) {
        logger.trace("openflow stub InventoryService" + ((this.global == null) ? "(global)" : "")
                + " addNode(Map) called");

        // sanity check
        if (mapConnectors == null || mapConnectors.isEmpty()) {
            return;
        }

        for (Map.Entry<Node, Collection<NodeConnector>> entry : mapConnectors.entrySet()) {
            if (entry.getValue() == null || entry.getValue().isEmpty()) {
                return;
            }
            for (NodeConnector nc : entry.getValue()) {
                if (!entry.getKey().equals(nc.getNode())) {
                    logger.debug("There is NO relationship between specified Node and NodeConnector(s).");
                    return;
                }
            }
        }

        for (Map.Entry<Node, Collection<NodeConnector>> entry : mapConnectors.entrySet()) {
            addNode(entry.getKey(), entry.getValue());
        }
    }


    /*
     * Remove node
     */
    public void removeNode(Node node) {
        logger.trace("openflow stub InventoryService" + ((this.global == null) ? "(global)" : "")
                + " removeNode(Node) called");

        // sanity check
        if (node == null) {
            return;
        }

        Set<NodeConnector> connectors = new HashSet<NodeConnector>();
        for (NodeConnector ncProp : nodeConnectorProps.keySet()) {
            if (ncProp.getNode().equals(node)) {
                connectors.add(ncProp);
            }
        }

        for (NodeConnector nc : connectors) {
            for (IPluginOutInventoryService service : pluginOutInventoryServices) {
                service.updateNodeConnector(nc, UpdateType.REMOVED, null);
                logger.debug("Removed NodeConnectors {} from {}", nc, service);
            }
            nodeConnectorProps.remove(nc);
        }

        for (IPluginOutInventoryService service : pluginOutInventoryServices) {
            service.updateNode(node, UpdateType.REMOVED, null);
            logger.debug("Adding Node {} from {}", node, service);
        }
        nodeProps.remove(node);

        // Notify to global
        if (this.global != null) {
            this.global.removeNode(node);
        }
    }

    public void removeNode(Collection<Node> nodes) {
        logger.trace("openflow stub InventoryService" + ((this.global == null) ? "(global)" : "")
                + " removeNode(Collection) called");
        if (nodes == null) {
            return;
        }
        for (Node node : nodes) {
            removeNode(node);
        }
    }

    /**
     * Retrieve nodes from openflow
     */
    @Override
    public ConcurrentMap<Node, Map<String, Property>> getNodeProps() {
        return nodeProps;
    }

    /**
     * Retrieve nodeConnectors from openflow
     */
    @Override
    public ConcurrentMap<NodeConnector, Map<String, Property>> getNodeConnectorProps(
            Boolean refresh) {
        return nodeConnectorProps;
    }

    @Override
    public Set<Node> getConfiguredNotConnectedNodes() {
        return null;
    }
}
