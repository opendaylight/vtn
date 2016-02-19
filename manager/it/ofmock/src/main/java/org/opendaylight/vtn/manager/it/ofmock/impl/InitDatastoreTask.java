/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodesBuilder;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopologyBuilder;

/**
 * {@code InitDatastoreTask} describes a task that initializes the MD-SAL
 * datastore managed by openflowplugin.
 */
public final class InitDatastoreTask extends TxTask<Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(InitDatastoreTask.class);

    /**
     * Construct a new instance.
     *
     * @param broker  The data broker service.
     */
    public InitDatastoreTask(DataBroker broker) {
        super(broker);
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    protected void execute(ReadWriteTransaction tx) {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<Nodes> npath =
            InstanceIdentifier.create(Nodes.class);
        Nodes nodes = new NodesBuilder().build();
        tx.merge(oper, npath, nodes, true);

        InstanceIdentifier<NetworkTopology> tpath =
            InstanceIdentifier.create(NetworkTopology.class);
        NetworkTopology topo = new NetworkTopologyBuilder().build();
        tx.merge(oper, tpath, topo, true);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
