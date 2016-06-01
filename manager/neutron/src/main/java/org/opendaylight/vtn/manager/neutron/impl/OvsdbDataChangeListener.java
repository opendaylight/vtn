/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import java.util.Collection;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.concepts.ListenerRegistration;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Node;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;

/**
 * Data tree change listener that listens network topology for ovsdb.
 */
public final class OvsdbDataChangeListener
    implements AutoCloseable, DataTreeChangeListener<Node> {
    /**
     * Logger instance.
     */
    static final Logger  LOG =
        LoggerFactory.getLogger(OvsdbDataChangeListener.class);

    /**
     * The data broker service.
     */
    @Nonnull
    private final DataBroker  dataBroker;

    /**
     * OVSDBEventHandler instance.
     */
    @Nonnull
    private final OVSDBEventHandler  eventHandler;

    /**
     * Registration of the data change listener.
     */
    private final ListenerRegistration<OvsdbDataChangeListener>  registration;

    /**
     * Class constructor setting the data broker.
     *
     * @param db   A {@link DataBroker} instance.
     * @param ovh  An {@link OVSDBEventHandler} instance.
     */
    public OvsdbDataChangeListener(@Nonnull DataBroker db,
                                   @Nonnull OVSDBEventHandler ovh) {
        dataBroker = db;
        eventHandler = ovh;

        InstanceIdentifier<Node> path = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class,
                  new TopologyKey(new TopologyId(new Uri("ovsdb:1")))).
            child(Node.class).
            build();
        DataTreeIdentifier<Node> ident = new DataTreeIdentifier<>(
            LogicalDatastoreType.OPERATIONAL, path);
        registration = db.registerDataTreeChangeListener(ident, this);
        LOG.debug("OVSDB topology listener has been registered.");
    }

    // AutoCloseable

    /**
     * Close the OVSDB data change listener.
     */
    @Override
    public void close() {
        registration.close();
        LOG.debug("OVSDB topology listener has been closed.");
    }

    // DataTreeChangeListener

    /**
     * Invoked when the specified data tree has been modified.
     *
     * @param changes  A collection of data tree modifications.
     */
    @Override
    public void onDataTreeChanged(
        @Nonnull Collection<DataTreeModification<Node>> changes) {
        try (ReadTransactionHolder txh =
             new ReadTransactionHolder(dataBroker)) {
            for (DataTreeModification<Node> change: changes) {
                OvsdbNodeChange ovchg = null;
                DataObjectModification<Node> mod = change.getRootNode();
                ModificationType modType = mod.getModificationType();
                Node before = mod.getDataBefore();
                if (modType == ModificationType.DELETE) {
                    if (before == null) {
                        LOG.warn("Ignore null deleted node.");
                    } else {
                        LOG.trace("OVSDB node has been deleted: {}", before);
                        ovchg = OvsdbNodeChange.
                            nodeRemoved(eventHandler, txh, before);
                    }
                } else {
                    Node after = mod.getDataAfter();
                    if (after == null) {
                        LOG.warn("Ignore null updated node.");
                    } else if (before == null) {
                        LOG.trace("OVSDB node has been created: {}", after);
                        ovchg = OvsdbNodeChange.
                            nodeCreated(eventHandler, txh, after);
                    } else {
                        LOG.trace("OVSDB node has been updated: {} -> {}",
                                  before, after);
                        ovchg = OvsdbNodeChange.
                            nodeUpdated(eventHandler, txh, mod, before, after);
                    }
                }

                if (ovchg != null) {
                    ovchg.apply();
                }
            }
        } catch (RuntimeException e) {
            LOG.error("Uncaught exception while handling OVSDB data changes.",
                      e);
        }
    }
}
