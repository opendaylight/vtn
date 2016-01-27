/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.topology.inventory.rev131030.InventoryNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.topology.inventory.rev131030.InventoryNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.topology.inventory.rev131030.InventoryNodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.topology.inventory.rev131030.InventoryNodeConnectorBuilder;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NodeId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.DestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.SourceBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.LinkKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.NodeKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPoint;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.node.TerminationPointKey;

/**
 * {@code TopologyUtils} class is a collection of utility class methods for
 * network topology management.
 */
public final class TopologyUtils {
    /**
     * A MD-SAL topology key which specifies the network topology for
     * OpenFlow network.
     */
    public static final TopologyKey  OF_TOPOLOGY_KEY =
        new TopologyKey(new TopologyId("flow:1"));

    /**
     * Private constructor that protects this class from instantiating.
     */
    private TopologyUtils() {}

    /**
     * Return an instance identifier builder for OpenFlow topology.
     *
     * @return  An instance identifier builder for OpenFlow topology.
     */
    public static InstanceIdentifierBuilder<Topology> getTopologyPathBuilder() {
        return InstanceIdentifier.builder(NetworkTopology.class).
            child(Topology.class, OF_TOPOLOGY_KEY);
    }

    /**
     * Create an instance identifier forthe specified node in the network
     * topology.
     *
     * @param nid  The MD-SAL node identifier.
     * @return  An instance identifier for the specified node in the network
     *          topology.
     */
    public static InstanceIdentifier<Node> getTopologyNodePath(String nid) {
        return getTopologyPathBuilder().
            child(Node.class, new NodeKey(new NodeId(nid))).
            build();
    }

    /**
     * Create a new node in the network topology.
     *
     * @param node  An {@link OfNode} instance.
     * @return  A node in the network topology.
     */
    public static Node newTopologyNode(OfNode node) {
        InventoryNode invNode = new InventoryNodeBuilder().
            setInventoryNodeRef(node.getNodeRef()).
            build();

        return new NodeBuilder().
            setNodeId(new NodeId(node.getNodeIdentifier())).
            addAugmentation(InventoryNode.class, invNode).
            build();
    }

    /**
     * Add a topology node into the MD-SAL datastore for the network topology.
     *
     * @param provider  The ofmock provider service.
     * @param node      An {@link OfNode} instance.
     */
    public static void addTopologyNode(OfMockProvider provider, OfNode node) {
        UpdateDataTask<Node> task = new UpdateDataTask<>(
            provider.getDataBroker(),
            getTopologyNodePath(node.getNodeIdentifier()),
            newTopologyNode(node));
        provider.getTopologyExecutor().execute(task);
    }

    /**
     * Create an instance identifier for the specified termination point in
     * the network topology.
     *
     * @param nid  The MD-SAL node identifier.
     * @param pid  The MD-SAL port identifier.
     * @return  An instance identifier for the specified termination point
     *          in the network topology.
     */
    public static InstanceIdentifier<TerminationPoint> getTerminationPointPath(
        String nid, String pid) {
        TerminationPointKey key = new TerminationPointKey(new TpId(pid));
        return getTopologyPathBuilder().
            child(Node.class, new NodeKey(new NodeId(nid))).
            child(TerminationPoint.class, key).
            build();
    }

    /**
     * Create a new termination point in the network topology.
     *
     * @param port  An {@link OfPort} instance.
     * @return  A termination point in the network topology.
     */
    public static TerminationPoint newTerminationPoint(OfPort port) {
        InventoryNodeConnector invPort = new InventoryNodeConnectorBuilder().
            setInventoryNodeConnectorRef(port.getPortRef()).
            build();

        return new TerminationPointBuilder().
            setTpId(new TpId(port.getPortIdentifier())).
            addAugmentation(InventoryNodeConnector.class, invPort).
            build();
    }

    /**
     * Add a termination point into the MD-SAL datastore for the network
     * topology.
     *
     * @param provider  The ofmock provider service.
     * @param port      An {@link OfPort} instance.
     */
    public static void addTerminationPoint(OfMockProvider provider,
                                           OfPort port) {
        String nid = port.getNodeIdentifier();
        String pid = port.getPortIdentifier();
        UpdateDataTask<TerminationPoint> task = new UpdateDataTask<>(
            provider.getDataBroker(),
            getTerminationPointPath(nid, pid),
            newTerminationPoint(port));
        provider.getTopologyExecutor().execute(task);
    }

    /**
     * Create an instance identifier for the inter-switch link in the network
     * topology.
     *
     * @param pid  The MD-SAL port identifier that specifies the source port
     *             of the link.
     * @return  An instance identifier for the specified link.
     */
    public static InstanceIdentifier<Link> getLinkPath(String pid) {
        return getTopologyPathBuilder().
            child(Link.class, new LinkKey(new LinkId(pid))).
            build();
    }

    /**
     * Create a new inter-switch link in the network topology.
     *
     * @param port      An {@link OfPort} instance that specifies the source
     *                  port of the link.
     * @param peer      The MD-SAL port identifier that specifies the
     *                  destination port of the link.
     * @return  A link in the network topology.
     */
    public static Link newLink(OfPort port, String peer) {
        String pid = port.getPortIdentifier();
        String srcNode = port.getNodeIdentifier();
        String dstNode = OfMockUtils.getNodeIdentifier(peer);
        SourceBuilder sb = new SourceBuilder().
            setSourceNode(new NodeId(srcNode)).
            setSourceTp(new TpId(pid));
        DestinationBuilder db = new DestinationBuilder().
            setDestNode(new NodeId(dstNode)).
            setDestTp(new TpId(peer));

        return new LinkBuilder().
            setLinkId(new LinkId(pid)).
            setSource(sb.build()).
            setDestination(db.build()).
            build();
    }

    /**
     * Add an inter-switch link into the MD-SAL datastore for the network
     * topology.
     *
     * @param provider  The ofmock provider service.
     * @param port      An {@link OfPort} instance that specifies the source
     *                  port of the link.
     * @param peer      The MD-SAL port identifier that specifies the
     *                  destination port of the link.
     */
    public static void addLink(OfMockProvider provider, OfPort port,
                               String peer) {
        String pid = port.getPortIdentifier();
        Link link = newLink(port, peer);
        UpdateDataTask<Link> task = new UpdateDataTask<>(
            provider.getDataBroker(), getLinkPath(pid), link);
        provider.getTopologyExecutor().execute(task);
    }

    /**
     * Remove the specified inter-switch link from the MD-SAL datastore for the
     * network topology.
     *
     * @param provider  The ofmock provider service.
     * @param pid       The MD-SAL port identifier that specifies the source
     *                  port of the link.
     */
    public static void removeLink(OfMockProvider provider, String pid) {
        DeleteDataTask<Link> task = new DeleteDataTask<>(
            provider.getDataBroker(), getLinkPath(pid));
        provider.getTopologyExecutor().execute(task);
    }
}
