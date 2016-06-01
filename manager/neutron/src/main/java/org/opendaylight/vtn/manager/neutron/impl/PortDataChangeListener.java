/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_OK;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getBridgeId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getInterfaceId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.getTenantId;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.recordLog;

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

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

/**
 * Data tree change listener that listens neutron port changes.
 */
public final class PortDataChangeListener
    implements AutoCloseable, DataTreeChangeListener<Port> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PortDataChangeListener.class);

    /**
     * VTN Manager service.
     */
    private final VTNManagerService  vtnManager;

    /**
     * ListenerRegistration Object to perform registration.
     */
    private ListenerRegistration<PortDataChangeListener> registration;

    /**
     * Construct a new instance.
     *
     * @param db   A {@link DataBroker} instance.
     * @param vtn  A {@link VTNManagerService} instance.
     */
    public PortDataChangeListener(DataBroker db, VTNManagerService vtn) {
        vtnManager = vtn;

        InstanceIdentifier<Port> path = InstanceIdentifier.
            builder(Neutron.class).
            child(Ports.class).
            child(Port.class).
            build();
        DataTreeIdentifier<Port> ident = new DataTreeIdentifier<>(
            LogicalDatastoreType.CONFIGURATION, path);
        registration = db.registerDataTreeChangeListener(ident, this);
        LOG.debug("Neutron port listener has been registered.");
    }

    /**
     * Create a new vBridge interface associated with the given port.
     *
     * @param port  A {@link Port} instance.
     */
    private void createPort(Port port) {
        if (port == null) {
            LOG.warn("Null neutron port has been created.");
        } else {
            recordLog(LOG, "Neutron port has been created", port);
            UpdateVinterfaceInput input = new UpdateVinterfaceInputBuilder().
                setTenantName(getTenantId(port)).
                setBridgeName(getBridgeId(port)).
                setInterfaceName(getInterfaceId(port)).
                setDescription(port.getName()).
                setEnabled(port.isAdminStateUp()).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            int result = vtnManager.updateInterface(input);
            if (result != HTTP_OK) {
                LOG.error("Failed to create vBridge interface: port={}", port);
            }
        }
    }

    /**
     * Delete the vBridge interface associated with the given port.
     *
     * @param port  An instance of deleted Neutron Port object.
     */
    private void deletePort(Port port) {
        if (port == null) {
            LOG.warn("Null neutron port has been deleted.");
        } else {
            recordLog(LOG, "Neutron port has been deleted", port);
            RemoveVinterfaceInput input = new RemoveVinterfaceInputBuilder().
                setTenantName(getTenantId(port)).
                setBridgeName(getBridgeId(port)).
                setInterfaceName(getInterfaceId(port)).
                build();
            int result = vtnManager.removeInterface(input);
            if (result != HTTP_OK) {
                LOG.error("Failed to delete vBridge interface: port={}", port);
            }
        }
    }

    // AutoCloseable

    /**
     * Close the neutron port change listener.
     */
    @Override
    public void close() {
        registration.close();
        LOG.debug("Neutron port listener has been closed.");
    }

    // DataTreeChangeListener

    /**
     * Invoked when the specified data tree has been modified.
     *
     * @param changes  A collection of data tree modifications.
     */
    @Override
    public void onDataTreeChanged(
        @Nonnull Collection<DataTreeModification<Port>> changes) {
        for (DataTreeModification<Port> change: changes) {
            DataObjectModification<Port> mod = change.getRootNode();
            ModificationType modType = mod.getModificationType();
            Port before = mod.getDataBefore();
            if (modType == ModificationType.DELETE) {
                deletePort(before);
            } else if (before == null) {
                createPort(mod.getDataAfter());
            }
        }
    }
}
