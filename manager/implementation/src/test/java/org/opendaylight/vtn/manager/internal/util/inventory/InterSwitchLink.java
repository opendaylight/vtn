/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * {@code InterSwitchLink} describes a link between switch ports.
 */
public final class InterSwitchLink {
    /**
     * The source port of the link.
     */
    private final SalPort  source;

    /**
     * The destination port of the link.
     */
    private final SalPort  destination;

    /**
     * Determine whether this link is a static link or not.
     */
    private final boolean  staticLink;

    /**
     * A VTN link which represents this link.
     */
    private final VtnLink  vtnLink;

    /**
     * A port link for the source port.
     */
    private final PortLink  sourceLink;

    /**
     * A port link for the destination port.
     */
    private final PortLink  destinationLink;

    /**
     * Construct a new instance which represents an inter-switch link
     * detected by topology-manager.
     *
     * @param src  The source port of the link.
     * @param dst  The destination port of the link.
     */
    public InterSwitchLink(SalPort src, SalPort dst) {
        this(src, dst, false);
    }

    /**
     * Construct a new instance.
     *
     * @param src  The source port of the link.
     * @param dst  The destination port of the link.
     * @param st   {@code true} means that the link is a static link.
     */
    public InterSwitchLink(SalPort src, SalPort dst, boolean st) {
        source = src;
        destination = dst;
        staticLink = st;

        LinkId lid = new LinkId(src.toString());
        Boolean stLink = (st) ? Boolean.TRUE : null;
        NodeConnectorId srcNc = src.getNodeConnectorId();
        NodeConnectorId dstNc = dst.getNodeConnectorId();
        vtnLink = new VtnLinkBuilder().
            setLinkId(lid).setSource(srcNc).setDestination(dstNc).
            setStaticLink(stLink).build();
        sourceLink = new PortLinkBuilder().
            setLinkId(lid).setPeer(dstNc).build();
        destinationLink = new PortLinkBuilder().
            setLinkId(lid).setPeer(srcNc).build();
    }

    /**
     * Return the source port of the link.
     *
     * @return  A {@link SalPort} instance which specifies the source port
     *          of the link.
     */
    public SalPort getSource() {
        return source;
    }

    /**
     * Return the destination port of the link.
     *
     * @return  A {@link SalPort} instance which specifies the destination
     *          port of the link.
     */
    public SalPort getDestination() {
        return destination;
    }

    /**
     * Return a {@link VtnLink} instance which represents this link.
     *
     * @return  A {@link VtnLink} instance.
     */
    public VtnLink getVtnLink() {
        return vtnLink;
    }

    /**
     * Return an {@link IgnoredLink} instance which represents this link.
     *
     * @return  An {@link IgnoredLink} instance.
     */
    public IgnoredLink getIgnoredLink() {
        return new IgnoredLinkBuilder(vtnLink).build();
    }

    /**
     * Return a {@link PortLink} instance which represents the source
     * termination point of the link.
     *
     * @return  A {@link PortLink} instance.
     */
    public PortLink getSourceLink() {
        return sourceLink;
    }

    /**
     * Return a {@link PortLink} instance which represents the destination
     * termination point of the link.
     *
     * @return  A {@link PortLink} instance.
     */
    public PortLink getDestinationLink() {
        return destinationLink;
    }

    /**
     * Return an instance identifier which specifies the VTN link.
     *
     * @return  An instance identifier which specifies the VTN link.
     */
    public InstanceIdentifier<VtnLink> getVtnLinkPath() {
        return InstanceIdentifier.builder(VtnTopology.class).
            child(VtnLink.class, vtnLink.getKey()).
            build();
    }

    /**
     * Return an instance identifier which specifies this link in the
     * ignored-links container.
     *
     * @return  An instance identifier which specifies this link in the
     *          ignored-links container.
     */
    public InstanceIdentifier<IgnoredLink> getIgnoredLinkPath() {
        IgnoredLinkKey key = new IgnoredLinkKey(vtnLink.getLinkId());
        return InstanceIdentifier.builder(IgnoredLinks.class).
            child(IgnoredLink.class, key).
            build();
    }

    /**
     * Return an insance identifier which specifies the source termination
     * point of the link.
     *
     * @return  An instance identifier which specifies the {@link PortLink}
     *          instance for the source port.
     */
    public InstanceIdentifier<PortLink> getSourceLinkPath() {
        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class, source.getVtnNodeKey()).
            child(VtnPort.class, source.getVtnPortKey()).
            child(PortLink.class, sourceLink.getKey()).
            build();
    }

    /**
     * Return an insance identifier which specifies the destination
     * termination point of the link.
     *
     * @return  An instance identifier which specifies the {@link PortLink}
     *          instance for the destination port.
     */
    public InstanceIdentifier<PortLink> getDestinationLinkPath() {
        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class, destination.getVtnNodeKey()).
            child(VtnPort.class, destination.getVtnPortKey()).
            child(PortLink.class, destinationLink.getKey()).
            build();
    }

    /**
     * Return a static link configuration for this link.
     *
     * @return  A {@link StaticSwitchLink} instance which represents this link.
     */
    public StaticSwitchLink getStaticSwitchLink() {
        return new StaticSwitchLinkBuilder().
            setSource(source.getNodeConnectorId()).
            setDestination(destination.getNodeConnectorId()).
            build();
    }
}
