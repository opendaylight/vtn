/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_BAD_REQUEST;
import static java.net.HttpURLConnection.HTTP_OK;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.convertNeutronIDToVTNKey;
import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.convertUUIDToKey;

import java.util.Map.Entry;
import java.util.Set;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

public final class PortDataChangeListener
    implements AutoCloseable, DataChangeListener {
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
    private ListenerRegistration<DataChangeListener> registration;

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
        registration = db.registerDataChangeListener(
            LogicalDatastoreType.CONFIGURATION, path, this,
            DataChangeScope.SUBTREE);
    }

    /**
     * Close the neutron port change listener.
     */
    public void close() {
        registration.close();
    }

    @Override
    public void onDataChanged(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        createPort(changes);
        deletePort(changes);
    }

    private void createPort(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        for (Entry<InstanceIdentifier<?>, DataObject> newNetwork : changes.getCreatedData().entrySet()) {
            if (newNetwork.getValue() instanceof Port) {
                Port port = (Port)newNetwork.getValue();
                portCreation(port);
            }
        }
    }

    /**
     * Create a new vBridge interface associated with the given port.
     *
     * @param port  A {@link Port} instance.
     * @return  A HTTP status code that indicates the result.
     *          {@link java.net.HttpURLConnection#HTTP_OK} indicates
     *          successful completion.
     */
    private int portCreation(Port port) {
        UpdateVinterfaceInputBuilder builder =
            toUpdateVinterfaceInputBuilder(port);
        int result;
        if (builder == null) {
            result = HTTP_BAD_REQUEST;
        } else {
            UpdateVinterfaceInput input = builder.
                setDescription(port.getName()).
                setEnabled(port.isAdminStateUp()).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            result = vtnManager.updateInterface(input);
        }

        return result;
    }

    /**
     * Create a update-vinterface input builder that specifies the virtual
     * interface associated with the given port.
     *
     * @param port  A {@link Port} instance.
     * @return  An {@link UpdateVinterfaceInputBuilder} instance on success.
     *          {@code null} on failure.
     */
    private UpdateVinterfaceInputBuilder toUpdateVinterfaceInputBuilder(
        Port port) {
        UpdateVinterfaceInputBuilder builder = null;
        if (port == null) {
            LOG.error("port object not specified");
        } else {
            String tenantUUID = port.getTenantId().getValue();
            String bridgeUUID = port.getNetworkId().getValue();
            String portUUID = port.getUuid().getValue();

            String tenantID = convertUUIDToKey(tenantUUID);
            String bridgeID = convertUUIDToKey(bridgeUUID);
            String portID = convertUUIDToKey(portUUID);
            if (tenantID == null) {
                LOG.error("Invalid tenant identifier: {}", tenantUUID);
            } else if (bridgeID == null) {
                LOG.error("Invalid bridge identifier: {}", bridgeUUID);
            } else if (portID == null) {
                LOG.error("Invalid port identifier: {}", portUUID);
            } else {
                builder = new UpdateVinterfaceInputBuilder().
                    setTenantName(tenantID).
                    setBridgeName(bridgeID).
                    setInterfaceName(portID);
            }
        }

        return builder;
    }

    /**
     * Method invoked when Port is deleted.
     * @param changes
     */
    private void deletePort(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> changes) {
        Map<InstanceIdentifier<?>, DataObject> originalDataObject = changes.getOriginalData();
        Set<InstanceIdentifier<?>> iiD = changes.getRemovedPaths();
        for (InstanceIdentifier instanceIdentifier : iiD) {
            try {
                if (originalDataObject.get(instanceIdentifier) instanceof Port) {
                    Port port = (Port)originalDataObject.get(instanceIdentifier);
                    neutronPortDeleted(port);
                }
            } catch (Exception e) {
                LOG.error("Could not delete VTN Renderer :{} ", e);
            }
        }
    }

    /**
     * Invoked to take action after a port has been deleted.
     *
     * @param port  An instance of deleted Neutron Port object.
     */
    public void neutronPortDeleted(Port port) {
        String tenantID =
            convertNeutronIDToVTNKey(port.getTenantId().getValue());
        String bridgeID =
            convertNeutronIDToVTNKey(port.getNetworkId().getValue());
        String portID = convertNeutronIDToVTNKey(port.getUuid().getValue());

        RemoveVinterfaceInput input = new RemoveVinterfaceInputBuilder().
            setTenantName(tenantID).
            setBridgeName(bridgeID).
            setInterfaceName(portID).
            build();
        int result = vtnManager.removeInterface(input);
        if (result != HTTP_OK) {
            LOG.error("removeInterface failed for tenant-id - {}, " +
                      "bridge-id - {}, port-id - {}, result - {}",
                      tenantID, bridgeID, portID, result);
        }
    }
}
