/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.recordLog;

import java.util.Collection;
import java.util.Objects;

import javax.annotation.Nonnull;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.Networks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Data tree change listener that listens neutron network changes.
 */
public final class NeutronNetworkChangeListener
    implements AutoCloseable, DataTreeChangeListener<Network> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(NeutronNetworkChangeListener.class);

    /**
     * ListenerRegistration Object to perform registration.
     */
    private ListenerRegistration<NeutronNetworkChangeListener> registration;

    /**
     * NetworkHandler instance.
     */
    private NetworkHandler networkHandler;

    /**
     * Determine whether the specified neutron network is changed or not.
     *
     * <p>
     *   This method compares network fields only used by
     *   {@link NetworkHandler#neutronNetworkUpdated(Network)}.
     * </p>
     *
     * @param before  Neutron network instance before modification.
     * @param after   Neutron network instance after modification.
     * @return  {@code true} if the specified neutron network is updated.
     *          {@code false} otherwise.
     */
    static boolean isChanged(@Nonnull Network before, @Nonnull Network after) {
        // Compare UUIDs.
        boolean same = (Objects.equals(before.getTenantId(),
                                       after.getTenantId()) &&
                        Objects.equals(before.getUuid(), after.getUuid()));
        if (same) {
            // Compare name and shared.
            same = (Objects.equals(before.getName(), after.getName()) &&
                    Objects.equals(before.isShared(), after.isShared()));
        }

        return !same;
    }

    /**
     * Class constructor setting the data broker.
     *
     * @param db  A {@link DataBroker} instance.
     * @param nh  A {@link NetworkHandler} instance.
     */
    public NeutronNetworkChangeListener(DataBroker db, NetworkHandler nh) {
        networkHandler = nh;

        InstanceIdentifier<Network> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Networks.class).
            child(Network.class).
            build();
        DataTreeIdentifier<Network> ident = new DataTreeIdentifier<>(
            LogicalDatastoreType.CONFIGURATION, path);
        registration = db.registerDataTreeChangeListener(ident, this);
        LOG.debug("Neutron network listener has been registered.");
    }

    /**
     * Delete the virtual bridge associated with the given neutron network.
     *
     * @param nw  A neutron network instance.
     */
    private void deleteNetwork(Network nw) {
        if (nw == null) {
            LOG.warn("Null network has been deleted.");
        } else {
            recordLog(LOG, "Neutron network has been deleted: {}", nw);
            networkHandler.neutronNetworkDeleted(nw);
        }
    }

    // AutoCloseable

    /**
     * Close the neutron network change listener.
     */
    @Override
    public void close() {
        registration.close();
        LOG.debug("Neutron network listener has been closed.");
    }

    // DataTreeChangeListener

    /**
     * Invoked when the specified data tree has been modified.
     *
     * @param changes  A collection of data tree modifications.
     */
    @Override
    public void onDataTreeChanged(
        @Nonnull Collection<DataTreeModification<Network>> changes) {
        for (DataTreeModification<Network> change: changes) {
            DataObjectModification<Network> mod = change.getRootNode();
            ModificationType modType = mod.getModificationType();
            Network before = mod.getDataBefore();
            if (modType == ModificationType.DELETE) {
                deleteNetwork(before);
            } else {
                Network after = mod.getDataAfter();
                if (after == null) {
                    // This should never happen.
                    LOG.warn("Null network has been updated.");
                } else if (before == null) {
                    recordLog(LOG, "Neutron network has been created", after);
                    networkHandler.neutronNetworkCreated(after);
                } else if (isChanged(before, after)) {
                    recordLog(LOG, "Neutron network has been changed",
                              before, after);
                    networkHandler.neutronNetworkUpdated(after);
                }
            }
        }
    }
}
