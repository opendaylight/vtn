/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import java.util.Set;
import java.util.Map;
import java.util.Map.Entry;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.Networks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public final class NeutronNetworkChangeListener
    implements AutoCloseable, DataChangeListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(NeutronNetworkChangeListener.class);

    /**
     * ListenerRegistration Object to perform registration.
     */
    private ListenerRegistration<DataChangeListener> registration;

    /**
     * NetworkHandler instance.
     */
    private NetworkHandler networkHandler;

    /**
     * Class constructor setting the data broker.
     *
     * @param db   A {@link DataBroker} instance.
     * @param vtn  A {@link VTNManagerService} instance.
     */
    public NeutronNetworkChangeListener(DataBroker db, VTNManagerService vtn) {
        LOG.info("Network DataChange Listener Registration()");
        InstanceIdentifier<Network> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Networks.class).
            child(Network.class).
            build();
        networkHandler = new NetworkHandler(vtn);
        registration = db.registerDataChangeListener(
            LogicalDatastoreType.CONFIGURATION, path, this,
            DataChangeScope.SUBTREE);
    }

    /**
     * Close the neutron network change listener.
     */
    @Override
    public void close() {
        registration.close();
    }

    /**
     * This method is called to create, update and delete Network
     */
    @Override
    public void onDataChanged(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        createNetwork(changes);
        updateNetwork(changes);
        deleteNetwork(changes);
    }

    /**
     * Method invoked when Network is created.
     * @param changes
     */
    private void createNetwork(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (Entry<InstanceIdentifier<?>, DataObject> newNetwork : changes.getCreatedData().entrySet()) {
            if (newNetwork.getValue() instanceof Network) {
                Network network = (Network)newNetwork.getValue();
                if (!(null == newNetwork)) {
                    networkHandler.neutronNetworkCreated(network);
                }
            }
        }
    }

    /**
     * Method invoked when Network is modified.
     * @param changes
     */
    private void updateNetwork(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (Entry<InstanceIdentifier<?>, DataObject> modifyNetwork : changes.getUpdatedData().entrySet()) {
            if (modifyNetwork.getValue() instanceof Network) {
                Network network = (Network)modifyNetwork.getValue();
                if (!(null == modifyNetwork)) {
                    networkHandler.neutronNetworkUpdated(network);
                }
            }
        }
    }

    /**
     * Method invoked when Network is deleted.
     * @param changes
     */
    private void deleteNetwork(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        Map<InstanceIdentifier<?>, DataObject> originalDataObject = changes.getOriginalData();
        Set<InstanceIdentifier<?>> iiD = changes.getRemovedPaths();
        for (InstanceIdentifier instanceIdentifier : iiD) {
            try {
                if (originalDataObject.get(instanceIdentifier) instanceof Network) {
                    Network network = (Network)originalDataObject.get(instanceIdentifier);
                    networkHandler.neutronNetworkDeleted(network);
                }
            } catch (Exception e) {
                LOG.error("Could not delete VTN Renderer :{} ", e);
            }
        }
    }
}
