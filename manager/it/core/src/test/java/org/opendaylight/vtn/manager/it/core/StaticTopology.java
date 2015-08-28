/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePortsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code StaticTopology} describes the configuration for the static network
 * topology.
 */
public final class StaticTopology {
    /**
     * Path to the static network topology configuration in the MD-SAL DS.
     */
    private static final InstanceIdentifier<VtnStaticTopology> TOPO_PATH =
        InstanceIdentifier.create(VtnStaticTopology.class);

    /**
     * ofmock service.
     */
    private final OfMockService  ofMock;

    /**
     * Configuration for the static inter-switch link.
     *
     * <p>
     *   Each map entry describes an inter-switch link. This map takes
     *   source port of inter-switch link as a key. Destination port
     *   of inter-switch link is associated with a key in this map.
     * </p>
     */
    private final Map<String, String>  switchLinks;

    /**
     * A set of static edge ports.
     */
    private final Set<String>  edgePorts;

    /**
     * Construct a new instance.
     *
     * @param mock  ofmock service.
     */
    public StaticTopology(OfMockService mock) {
        ofMock = mock;
        switchLinks = new HashMap<>();
        edgePorts = new HashSet<>();
    }

    /**
     * Construct a copy of the given instance.
     *
     * @param stopo  A {@link StaticTopology} instance to be copied.
     */
    public StaticTopology(StaticTopology stopo) {
        ofMock = stopo.ofMock;
        switchLinks = new HashMap<>(stopo.switchLinks);
        edgePorts = new HashSet<>(stopo.edgePorts);
    }

    /**
     * Add a static inter-switch link configuration.
     *
     * @param src  The port identifier which specifies the source port of
     *             the link.
     * @param dst  The port identifier which specifies the destination port of
     *             the link.
     */
    public void addSwitchLink(String src, String dst) {
        switchLinks.put(src, dst);
    }

    /**
     * Remove a static inter-switch link configuration.
     *
     * @param src  The port identifier which specifies the soutce port of
     *             the link to be removed.
     */
    public void removeSwitchLink(String src) {
        switchLinks.remove(src);
    }

    /**
     * Add a static edge port configuration.
     *
     * @param port  The port identifier to be treated as an edge port.
     */
    public void addEdgePort(String port) {
        edgePorts.add(port);
    }

    /**
     * Remove a static edge port configuration.
     *
     * @param port  The port identifier to be removed from the static edge
     *              port configuration.
     */
    public void removeEdgePort(String port) {
        edgePorts.remove(port);
    }

    /**
     * Remove all configuration.
     */
    public void clear() {
        switchLinks.clear();
        edgePorts.clear();
    }

    /**
     * Apply the static network topology configuration.
     *
     * @throws IllegalStateException
     *    Failed to apply configuration.
     */
    public void apply() {
        ReadWriteTransaction tx = ofMock.newReadWriteTransaction();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        VtnStaticTopology vstopo = getVtnStaticTopology();
        if (vstopo == null) {
            // Delete the configuration.
            DataStoreUtils.delete(tx, config, TOPO_PATH);
        } else {
            // Apply the configuration.
            tx.put(config, TOPO_PATH, vstopo, true);
        }

        DataStoreUtils.submit(tx);
    }

    /**
     * Construct a MD-SAL container which contains static network topology.
     *
     * @return  A MD-SAL container which contains static network topology
     *          configuration. {@code null} if no static network topology is
     *          configured.
     */
    private VtnStaticTopology getVtnStaticTopology() {
        StaticSwitchLinks swlinks = getStaticSwitchLinks();
        StaticEdgePorts edges = getStaticEdgePorts();
        if (swlinks == null && edges == null) {
            return null;
        }

        return new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlinks).
            setStaticEdgePorts(edges).
            build();
    }

    /**
     * Construct a MD-SAL container which contains inter-switch link
     * configuration.
     *
     * @return  A MD-SAL container which contains inter-switch link
     *          configuration. {@code null} if no inter-switch link is
     *          configured.
     */
    private StaticSwitchLinks getStaticSwitchLinks() {
        if (switchLinks.isEmpty()) {
            return null;
        }

        List<StaticSwitchLink> swlinks = new ArrayList<>();
        for (Map.Entry<String, String> entry: switchLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            StaticSwitchLink swl = new StaticSwitchLinkBuilder().
                setSource(new NodeConnectorId(src)).
                setDestination(new NodeConnectorId(dst)).
                build();
            swlinks.add(swl);
        }

        return new StaticSwitchLinksBuilder().
            setStaticSwitchLink(swlinks).
            build();
    }

    /**
     * Construct a MD-SAL container which contains static edge port
     * configuration.
     *
     * @return  A MD-SAL container which contains static edge port
     *          configuration. {@code null} if no static edge port is
     *          configured.
     */
    private StaticEdgePorts getStaticEdgePorts() {
        if (edgePorts.isEmpty()) {
            return null;
        }

        List<StaticEdgePort> edges = new ArrayList<>();
        for (String port: edgePorts) {
            StaticEdgePort ep = new StaticEdgePortBuilder().
                setPort(new NodeConnectorId(port)).
                build();
            edges.add(ep);
        }

        return new StaticEdgePortsBuilder().
            setStaticEdgePort(edges).
            build();
    }
}
