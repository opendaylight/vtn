/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.sal.binding.api.BindingAwareBroker.ProviderContext;
import org.opendaylight.controller.sal.binding.api.BindingAwareProvider;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.Topology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.TopologyKey;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopology;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.NetworkTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TopologyId;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

/**
 * Implementation of vtn-neutron module that associates Neutron network with
 * VTN.
 */
public class NeutronProvider implements BindingAwareProvider, AutoCloseable {
    /**
     * The logger instance.
     */
    private static final Logger LOG = LoggerFactory.getLogger(NeutronProvider.class);

    /**
     * The data broker service.
     */
    private DataBroker  dataBroker;

    /**
     * The data change listener that listens OVSDB topology changes.
     */
    private OvsdbDataChangeListener ovsdbDataChangeListener;

    /**
     * The data change listener that listens neutron network changes.
     */
    private NeutronNetworkChangeListener neutronNetworkChangeListener;

    /**
     * The data change listener that listens neutron port changes.
     */
    private PortDataChangeListener portDataChangeListener;

    /**
     * The manager.neutron configuration.
     */
    private final NeutronConfig  bundleConfig;

    /**
     * Construct a new instance.
     *
     * @param cfg  The configuration for the manager.neutron bundle.
     */
    public NeutronProvider(NeutronConfig cfg) {
        bundleConfig = cfg;
        LOG.info("Bundle configuration: {}", cfg);
    }

    /**
     * Method invoked when the open flow switch is Added.
     * @param session the session object
     */
    @Override
    public void onSessionInitiated(ProviderContext session) {
        LOG.trace("Neutron provider Session Initiated");
        DataBroker db = session.getSALService(DataBroker.class);
        dataBroker = db;

        MdsalUtils md = new MdsalUtils(db);
        VTNManagerService vtn = new VTNManagerService(md, session);

        OVSDBEventHandler ovh = new OVSDBEventHandler(bundleConfig, md, vtn);
        ovsdbDataChangeListener = new OvsdbDataChangeListener(db, ovh);
        neutronNetworkChangeListener =
            new NeutronNetworkChangeListener(db, new NetworkHandler(vtn));
        initializeOvsdbTopology(LogicalDatastoreType.OPERATIONAL);
        initializeOvsdbTopology(LogicalDatastoreType.CONFIGURATION);
        portDataChangeListener = new PortDataChangeListener(db, vtn);
    }

    /**
     * Method invoked when the open flow switch is Added.
     */
    @Override
    public void close() {
        OvsdbDataChangeListener ovsdbListener = ovsdbDataChangeListener;
        if (ovsdbListener != null) {
            ovsdbDataChangeListener = null;
            ovsdbListener.close();
        }

        NeutronNetworkChangeListener neutronListener =
            neutronNetworkChangeListener;
        if (neutronListener != null) {
            neutronNetworkChangeListener = null;
            neutronListener.close();
        }

        PortDataChangeListener portListener = portDataChangeListener;
        if (portListener != null) {
            portDataChangeListener = null;
            portListener.close();
        }

        LOG.info("Neutron provider has been closed.");
    }

    /**
     * Method invoked when the open flow switch is Added.
     *
     * @param type  The type of the logical datastore.
     */
    private void initializeOvsdbTopology(LogicalDatastoreType type) {
        TopologyId topoId = new TopologyId(new Uri("ovsdb:1"));
        InstanceIdentifier<Topology> path = InstanceIdentifier.
            builder(NetworkTopology.class).
            child(Topology.class, new TopologyKey(topoId)).
            build();
        ReadWriteTransaction transaction = dataBroker.newReadWriteTransaction();
        initializeTopology(transaction, type);
        CheckedFuture<Optional<Topology>, ReadFailedException> ovsdbTp = transaction.read(type, path);
        try {
            if (!ovsdbTp.get().isPresent()) {
                TopologyBuilder tpb = new TopologyBuilder().
                    setTopologyId(topoId);
                transaction.put(type, path, tpb.build());
                transaction.submit();
            } else {
                transaction.cancel();
            }
        } catch (Exception e) {
            LOG.error("Error initializing ovsdb topology", e);
        }
    }

    /**
     * Method invoked when the open flow switch is Added.
     *
     * @param transaction  Read-write transaction for MD-SAL datastore.
     * @param type         The type of the logical datastore.
     */
    private void initializeTopology(ReadWriteTransaction transaction, LogicalDatastoreType type) {
        InstanceIdentifier<NetworkTopology> path = InstanceIdentifier.create(NetworkTopology.class);
        CheckedFuture<Optional<NetworkTopology>, ReadFailedException> topology = transaction.read(type, path);
        try {
            if (!topology.get().isPresent()) {
                NetworkTopologyBuilder ntb = new NetworkTopologyBuilder();
                transaction.put(type, path, ntb.build());
            }
        } catch (Exception e) {
            LOG.error("Error initializing ovsdb topology {}", e);
        }
    }
}
