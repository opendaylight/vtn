/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.Collections;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;

/**
 * Listener class that listens the change of MD-SAL topology datastore.
 */
public final class TopologyListener
    extends InventoryMaintainer<Link, TopologyEventContext> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(TopologyListener.class);

    /**
     * Network topology identifier to listen.
     */
    private static final String TOPOLOGY_ID = "flow:1";

    /**
     * Required event types.
     */
    private static final Set<VtnUpdateType>  REQUIRED_EVENTS =
        Collections.unmodifiableSet(
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.REMOVED));

    /**
     * MD-SAL transaction task that initializes the VTN topology tree.
     */
    private class TopologyInitTask extends AbstractTxTask<Void> {
        /**
         * Initialize VTN network topology.
         *
         * @param tx        A {@link ReadWriteTransaction} instance.
         * @param topology  MD-SAL network topology.
         * @throws VTNException
         *    An error occurred.
         */
        private void initLinks(ReadWriteTransaction tx, Topology topology)
            throws VTNException {
            if (topology == null) {
                return;
            }

            List<Link> links = topology.getLink();
            if (links == null) {
                return;
            }

            for (Link link: links) {
                SalPort src = SalPort.create(link.getSource());
                SalPort dst = SalPort.create(link.getDestination());
                if (src == null || dst == null) {
                    LOG.debug("Ignore unsupported inter-switch link: {}",
                              link);
                    continue;
                }

                LinkId lid = link.getLinkId();
                if (!addVtnLink(tx, lid, src, dst)) {
                    LOG.warn("Ignore inter-switch link: {}: {} -> {}",
                             lid.getValue(), src, dst);
                }
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            // Read all inter-switch links in the MD-SAL datastore.
            TopologyKey topoKey = new TopologyKey(new TopologyId(TOPOLOGY_ID));
            InstanceIdentifier<Topology> topoPath = InstanceIdentifier.
                builder(NetworkTopology.class).
                child(Topology.class, topoKey).build();
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            Topology topology =
                DataStoreUtils.read(tx, oper, topoPath).orNull();

            // Initialize vtn-topology and ignored-links.
            InstanceIdentifier<VtnTopology> vtPath =
                InstanceIdentifier.create(VtnTopology.class);
            InstanceIdentifier<IgnoredLinks> igPath =
                InstanceIdentifier.create(IgnoredLinks.class);
            tx.delete(oper, vtPath);
            tx.delete(oper, igPath);
            tx.merge(oper, vtPath, new VtnTopologyBuilder().build(), true);
            tx.merge(oper, igPath, new IgnoredLinksBuilder().build(), true);
            initLinks(tx, topology);

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            LOG.warn("Failed to initialize VTN topology datastore.", t);
        }
    }

    /**
     * MD-SAL transaction task that updates the VTN link.
     */
    private class LinkUpdatedTask extends AbstractTxTask<Void>
        implements TopologyEventContext {
        /**
         * A map that keeps created links.
         */
        private final Map<LinkId, LinkEdge>  created = new HashMap<>();

        /**
         * A map that keeps removed links.
         */
        private final Map<LinkId, LinkEdge>  removed = new HashMap<>();

        /**
         * Remove the given link from the VTN network topology.
         *
         * @param tx   A {@link ReadWriteTransaction} instance.
         * @param lid  A {@link LinkId} instance.
         * @param le   A {@link LinkEdge} instance.
         * @throws VTNException  An error occurred.
         */
        private void remove(ReadWriteTransaction tx, LinkId lid, LinkEdge le)
            throws VTNException {
            // Remove the link from vtn-topology list.
            SalPort src = le.getSourcePort();
            SalPort dst = le.getDestinationPort();
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            InstanceIdentifier<VtnLink> lpath =
                InventoryUtils.toVtnLinkIdentifier(lid);
            if (DataStoreUtils.read(tx, oper, lpath).isPresent()) {
                tx.delete(oper, lpath);

                // Remove port links.
                removePortLink(tx, src, lid);
                removePortLink(tx, dst, lid);
            } else {
                // Remove the link from ignored-links list.
                InstanceIdentifier<IgnoredLink> ipath =
                    InventoryUtils.toIgnoredLinkIdentifier(lid);
                DataStoreUtils.delete(tx, oper, ipath);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            // Process link deletion events.
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            for (Map.Entry<LinkId, LinkEdge> entry: removed.entrySet()) {
                LinkId lid = entry.getKey();
                LinkEdge le = entry.getValue();
                remove(tx, lid, le);
            }

            // Process link creation events.
            for (Map.Entry<LinkId, LinkEdge> entry: created.entrySet()) {
                LinkId lid = entry.getKey();
                LinkEdge le = entry.getValue();
                SalPort src = le.getSourcePort();
                SalPort dst = le.getDestinationPort();
                if (!addVtnLink(tx, lid, src, dst)) {
                    LOG.warn("onCreated: Ignore inter-switch link: {}: " +
                             "{} -> {}", lid.getValue(), src, dst);
                }
            }

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            LOG.error("Failed to update VTN link.", t);
        }

        // TopologyEventContext

        /**
         * {@inheritDoc}
         */
        @Override
        public void addCreated(Link link) {
            LinkId lid = link.getLinkId();
            LinkEdge le = new LinkEdge(link);
            created.put(lid, le);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void addRemoved(Link link) {
            LinkId lid = link.getLinkId();
            LinkEdge le = new LinkEdge(link);
            removed.put(lid, le);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean hasLink() {
            return !(created.isEmpty() && removed.isEmpty());
        }
    }

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     */
    public TopologyListener(TxQueue queue, DataBroker broker) {
        super(queue, broker, Link.class, DataChangeScope.SUBTREE);
        submitInitial(new TopologyInitTask());
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected TopologyEventContext enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new LinkUpdatedTask();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(TopologyEventContext ectx) {
        if (ectx.hasLink()) {
            submit(ectx);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(TopologyEventContext ectx,
                             InstanceIdentifier<Link> key, Link value) {
        try {
            ectx.addCreated(value);
        } catch (IllegalArgumentException e) {
            LOG.debug("Ignore unsupported inter-switch link creation: " +
                      value, e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(TopologyEventContext ectx,
                             InstanceIdentifier<Link> key, Link oldValue,
                             Link newValue) {
        throw new IllegalStateException("Should never be called.");
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(TopologyEventContext ectx,
                             InstanceIdentifier<Link> key, Link value) {
        try {
            ectx.addRemoved(value);
        } catch (IllegalArgumentException e) {
            LOG.debug("Ignore unsupported inter-switch link deletion: " +
                      value, e);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<Link> getWildcardPath() {
        TopologyKey topoKey = new TopologyKey(new TopologyId(TOPOLOGY_ID));
        return InstanceIdentifier.builder(NetworkTopology.class).
            child(Topology.class, topoKey).child(Link.class).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Set<VtnUpdateType> getRequiredEvents() {
        return REQUIRED_EVENTS;
    }
}
