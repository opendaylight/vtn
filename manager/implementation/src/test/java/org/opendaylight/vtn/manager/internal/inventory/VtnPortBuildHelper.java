/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.opendaylight.vtn.manager.internal.TestBase.newKeyedModification;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.BuildHelper;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * Helper class for building vtn-port instance.
 */
public final class VtnPortBuildHelper
    extends BuildHelper<VtnPort, VtnPortBuilder> {
    /**
     * A set of port links.
     */
    private final Map<LinkId, PortLinkBuildHelper>  portLinks =
        new HashMap<>();

    /**
     * Construct a new instance.
     *
     * @param sport  A {@link SalPort} instance that specifies the switch port.
     */
    public VtnPortBuildHelper(SalPort sport) {
        this(sport.getNodeConnectorId());
    }

    /**
     * Construct a new instance.
     *
     * @param nid  A {@link NodeConnectorId} instance that specifies the
     *             switch port.
     */
    public VtnPortBuildHelper(NodeConnectorId nid) {
        super(new VtnPortBuilder().setId(nid));
    }

    /**
     * Add the specified port link to this switch port.
     *
     * @param peer  A {@link SalPort} instance that specifies the peer port.
     * @return  This instance.
     */
    public VtnPortBuildHelper addPortLink(SalPort peer) {
        NodeConnectorId ncId = getBuilder().getId();
        LinkId lid = new LinkId(ncId.getValue());
        PortLinkBuildHelper plhelper = portLinks.get(lid);
        if (plhelper == null) {
            plhelper = new PortLinkBuildHelper(lid, peer);
            portLinks.put(lid, plhelper);
        } else {
            plhelper.getBuilder().setPeer(peer.getNodeConnectorId());
        }
        return this;
    }

    /**
     * Return the port link associated with the given link ID in this switch
     * port.
     *
     * @param lid  The link ID.
     * @return  A {@link PortLinkBuildHelper} instance associated with the
     *          given ID if found. {@code null} if not found.
     */
    public PortLinkBuildHelper getPortLink(LinkId lid) {
        return portLinks.get(lid);
    }

    /**
     * Return a set of port link identifiers.
     *
     * @return  A set of port link identifiers.
     */
    public Set<LinkId> getPortLinkKeys() {
        return new HashSet<LinkId>(portLinks.keySet());
    }

    /**
     * Create a new data object modification that represents creation of
     * this vtn-port.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnPort> newCreatedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        for (PortLinkBuildHelper child: portLinks.values()) {
            children.add(child.newCreatedModification());
        }

        return newKeyedModification(null, build(), children);
    }

    /**
     * Create a new data object modification that represents deletion of
     * this vtn-port.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnPort> newDeletedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        for (PortLinkBuildHelper child: portLinks.values()) {
            children.add(child.newDeletedModification());
        }

        return newKeyedModification(build(), null, children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * vtn-port made by merge operation.
     *
     * @param before  A {@link VtnPortBuildHelper} instance that indicates the
     *                vtn-port value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnPort> newMergeModification(
        VtnPortBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();

        Set<LinkId> oldLinks = before.getPortLinkKeys();
        for (Entry<LinkId, PortLinkBuildHelper> entry: portLinks.entrySet()) {
            LinkId lid = entry.getKey();
            PortLinkBuildHelper child = entry.getValue();
            if (oldLinks.remove(lid)) {
                PortLinkBuildHelper old = before.getPortLink(lid);
                if (child.isChanged(old)) {
                    children.add(child.newMergeModification(old));
                }
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (LinkId lid: oldLinks) {
            PortLinkBuildHelper old = before.getPortLink(lid);
            children.add(old.newDeletedModification());
        }

        return newKeyedModification(before.build(), build(), children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * vtn-port made by put operation.
     *
     * @param before  A {@link VtnPortBuildHelper} instance that indicates the
     *                vtn-port value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnPort> newPutModification(
        VtnPortBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();

        Set<LinkId> oldLinks = before.getPortLinkKeys();
        for (Entry<LinkId, PortLinkBuildHelper> entry: portLinks.entrySet()) {
            LinkId lid = entry.getKey();
            PortLinkBuildHelper child = entry.getValue();
            if (oldLinks.remove(lid)) {
                PortLinkBuildHelper old = before.getPortLink(lid);
                children.add(child.newPutModification(old));
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (LinkId lid: oldLinks) {
            PortLinkBuildHelper old = before.getPortLink(lid);
            children.add(old.newDeletedModification());
        }

        VtnPort vport = build();
        return newKeyedModification(VtnPort.class, ModificationType.WRITE,
                                    vport.getKey(), before.build(), vport,
                                    children);
    }

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        for (PortLinkBuildHelper child: portLinks.values()) {
            child.freeze();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(VtnPortBuilder builder) {
        List<PortLink> plinks = new ArrayList<>(portLinks.size());
        for (PortLinkBuildHelper child: portLinks.values()) {
            plinks.add(child.build());
        }
        if (!plinks.isEmpty()) {
            builder.setPortLink(plinks);
        }
    }
}
