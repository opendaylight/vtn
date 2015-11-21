/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.opendaylight.vtn.manager.internal.inventory.StaticTopologyManager.IDENT_TOPOLOGY;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticEdgePorts;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLinks;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContext;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * A MD-SAL datastore transaction task that updates VTN static network topology
 * information.
 *
 * <p>
 *   This task returns a {@link LinkUpdateContext} instance.
 * </p>
 */
final class StaticLinkUpdateTask extends AbstractTxTask<LinkUpdateContext> {
    /**
     * A set of switch ports to be updated its static network topology.
     */
    private final Set<SalPort>  updatedPorts = new HashSet<>();

    /**
     * A logger instance.
     */
    private final Logger  logger;

    /**
     * Static network topology configuration.
     */
    private VtnStaticTopology  staticTopology;

    /**
     * Determine whether the static inter-switch link configuration should be
     * saved or not.
     */
    private boolean  saveLinks;

    /**
     * Determine whether the static edge port configuration should be saved
     * or not.
     */
    private boolean  saveEdges;

    /**
     * Determine whether the static configuration is successfully loaded
     * from the config DS or not.
     */
    private boolean  configLoaded;

    /**
     * Construct a new instance.
     *
     * @param log  A {@link Logger} instance.
     */
    StaticLinkUpdateTask(Logger log) {
        logger = log;
    }

    /**
     * Add a static inter-switch link configuration that was created or
     * removed.
     *
     * @param swlink  A {@link StaticSwitchLink} instance that was created or
     *                removed.
     */
    void addUpdated(StaticSwitchLink swlink) {
        addUpdatedPort(swlink.getSource());
        saveLinks = true;
    }

    /**
     * Add a static inter-switch link configuration that was updated.
     *
     * @param old     A {@link StaticSwitchLink} instance which represents
     *                the given data before update.
     * @param swlink  A {@link StaticSwitchLink} instance that was updated.
     */
    void addUpdated(StaticSwitchLink old, StaticSwitchLink swlink) {
        addUpdatedPort(swlink.getSource());
        addUpdatedPort(old.getDestination());
        saveLinks = true;
    }

    /**
     * Add a static edge port configuration that was created or removed.
     *
     * @param edge  A {@link StaticEdgePort} instance that was created or
     *              removed.
     */
    void addUpdated(StaticEdgePort edge) {
        addUpdatedPort(edge.getPort());
        saveEdges = true;
    }

    /**
     * Return a set of switch ports to be updated.
     *
     * <p>
     *   This method is only for unit test.
     * </p>
     *
     * @return  A set of {@link SalPort} instances.
     */
    Set<SalPort> getUpdatedPorts() {
        return Collections.unmodifiableSet(updatedPorts);
    }

    /**
     * Add a switch port to be updated its static network topology.
     *
     * @param ncId  A {@link NodeConnectorId} which specifies the switch port.
     */
    private void addUpdatedPort(NodeConnectorId ncId) {
        SalPort sport = SalPort.create(ncId);
        if (sport != null) {
            updatedPorts.add(sport);
        }
    }

    /**
     * Save the static network topology configuration to the file.
     */
    private void saveConfig() {
        VtnStaticTopology vstopo = staticTopology;

        if (vstopo == null) {
            // Delete the static network topology configuration file.
            StaticTopologyManager.saveConfig(null);
        } else {
            if (saveLinks) {
                // Update the configuration file for static inter-switch links.
                new XmlStaticSwitchLinks().save(vstopo.getStaticSwitchLinks());
            }
            if (saveEdges) {
                // Update the configuration file for static edge ports.
                new XmlStaticEdgePorts().save(vstopo.getStaticEdgePorts());
            }
        }
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public LinkUpdateContext execute(TxContext ctx) throws VTNException {
        configLoaded = false;
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        InventoryReader reader = ctx.getReadSpecific(InventoryReader.class);
        LinkUpdateContext luctx = new LinkUpdateContext(tx, reader);

        // Read whole static network topology configuration.
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        staticTopology =
            DataStoreUtils.read(tx, cstore, IDENT_TOPOLOGY).orNull();
        configLoaded = true;

        // Prefetch configuration into the inventory reader.
        reader.prefetch(staticTopology);

        for (SalPort sport: updatedPorts) {
            // Read the notified switch port.
            // If the port was removed, static links on that port will be
            // removed by PortUpdateTask.
            VtnPort vport = reader.get(sport);
            if (vport != null) {
                luctx.updateStaticTopology(sport, vport);
            }
        }

        // Resolve ignored inter-switch links.
        luctx.resolveIgnoredLinks();

        return luctx;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider,
                          LinkUpdateContext luctx) {
        luctx.recordLogs(logger);
        saveConfig();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onFailure(VTNManagerProvider provider, Throwable t) {
        logger.error("Failed to update VTN static network topology.", t);

        // Configuration should be saved as long as the configuration
        // is available in the config DS.
        if (configLoaded) {
            saveConfig();
        }
    }
}
