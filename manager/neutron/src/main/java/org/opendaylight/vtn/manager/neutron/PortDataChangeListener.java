/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;
import java.util.Map.Entry;
import java.util.Set;
import java.util.Map;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.Ports;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.ports.rev150712.ports.attributes.ports.Port;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class PortDataChangeListener extends VTNNeutronUtils implements AutoCloseable, DataChangeListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PortDataChangeListener.class);

    /**
     * DataBroker Object to perform MDSAL operation.
     */
    private DataBroker dataBroker = null;

    /**
     * VTN identifiers in neutron port object.
     */
    private static final int VTN_IDENTIFIERS_IN_PORT = 3;

    /**
     * ListenerRegistration Object to perform registration.
     */
    private ListenerRegistration<DataChangeListener> registration;



    public PortDataChangeListener(DataBroker dataBroker) {
        this.dataBroker = dataBroker;
        InstanceIdentifier<Port> path = InstanceIdentifier.create(Neutron.class)
                .child(Ports.class)
                .child(Port.class);
        registration = this.dataBroker.registerDataChangeListener(LogicalDatastoreType.CONFIGURATION, path, this, DataChangeScope.SUBTREE);
    }

    @Override
    public void close() throws Exception {
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
                int result = portCreation(port);
            }
        }
    }

    private int portCreation(Port port) {
        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        result = getVTNIdentifiers(port, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canCreatePort getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];
        String portID = vtnIDs[2];
        String portDesc = port.getName();
        Boolean portAdminState = port.isAdminStateUp();

        result = isTenantExist(tenantID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Tenant does not exist for tenant-id - {}, result - {}",
                      tenantID, result);
            return result;
        }

        result = isBridgeExist(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Bridge does not exist for tenant-id - {}, " +
                      "bridge-id - {}, result - {}",
                   tenantID, bridgeID, result);
            return result;
        }

        /**
         * check if the object already exists in VTN manager
         */
        result = isBridgeInterfaceExist(tenantID, bridgeID, portID);
        if (result == HttpURLConnection.HTTP_OK) {
            /**
             * A bridge inteface with given port id exists return error
             */
            result = HttpURLConnection.HTTP_CONFLICT;
        } else if (result == HttpURLConnection.HTTP_NOT_FOUND) {
            /**
             * A bridge interface with given port id not found
             * return success
             */
            result = HttpURLConnection.HTTP_OK;
        }

        if (result == HttpURLConnection.HTTP_OK) {
            result = HttpURLConnection.HTTP_CREATED;
        }
        if (result != HttpURLConnection.HTTP_CREATED) {
            LOG.error("Port create validation failed, result - {}", result);
            return HttpURLConnection.HTTP_NOT_FOUND;
        }
        result = createBridgeInterface(tenantID,
                                       bridgeID,
                                       portID,
                                       portDesc,
                                       portAdminState);
        int bif = isBridgeInterfaceExist(tenantID, bridgeID, portID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createBridgeInterface failed for tenant-id - {}, ",
                      "bridge-id - {}, port-id - {}, result - {}",
                      tenantID, bridgeID, portID, result);
            return HttpURLConnection.HTTP_NOT_FOUND;
        }
        return result;
    }

    private int getVTNIdentifiers(Port port, String[] vtnIDs) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        /**
         * To basic validation of the request
         */
        if (port == null) {
            LOG.error("port object not specified");
            return result;
        }

        String tenantUUID = port.getTenantId().getValue();
        String bridgeUUID = port.getNetworkId().getValue();
        String portUUID = port.getUuid().getValue();


        if ((tenantUUID == null) || (bridgeUUID == null) || portUUID == null) {
            LOG.error("neutron identifiers not specified");
            return result;
        }

        String tenantID = convertUUIDToKey(tenantUUID);
        if (tenantID == null) {
            LOG.error("Invalid tenant identifier");
            return result;
        }

        String bridgeID = convertUUIDToKey(bridgeUUID);
        if (bridgeID == null) {
            LOG.error("Invalid bridge identifier");
            return result;
        }

        String portID = convertUUIDToKey(portUUID);
        if (portID == null) {
            LOG.error("Invalid port identifier");
            return result;
        }

        vtnIDs[0] = tenantID;
        vtnIDs[1] = bridgeID;
        vtnIDs[2] = portID;
        return HttpURLConnection.HTTP_OK;
    }

    /**
     * Create a new virtual interface in the bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param portID port identifier provided by neutron.
     * @param portDesc port description provided by neutron.
     * @param adminState port admin status provided by neutron.
     * @return interface creation status in HTTP response status code.
     */
    private int createBridgeInterface(String tenantID,
                                      String bridgeID,
                                      String portID,
                                      String portDesc,
                                      Boolean adminState) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VInterfaceConfig conf = new VInterfaceConfig(portDesc, adminState);
        VBridgeIfPath path = new VBridgeIfPath(tenantID, bridgeID, portID);
        Status status = getVTNManager().addInterface(path, conf);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
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
     * Invoked when a port deletion is requested
     * to indicate if the specified port can be deleted.
     *
     * @param port     An instance of the Neutron Port object to be deleted.
     * @return A HTTP status code to the deletion request.
     */
    public int canDeletePort(Port port) {
        int result = HttpURLConnection.HTTP_OK;

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        result = getVTNIdentifiers(port, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canDeletePort getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];
        String portID = vtnIDs[2];

        /**
         * check if the object already exists in VTN manager
         */
        result = isBridgeInterfaceExist(tenantID, bridgeID, portID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Interface does not exist for tenant-id - {}, " +
                      "bridge-id - {}, port-id - {}, result - {}",
                      tenantID, bridgeID, portID, result);
        }
        return result;
    }

    /**
     * Invoked to take action after a port has been deleted.
     *
     * @param port  An instance of deleted Neutron Port object.
     */
    public void neutronPortDeleted(Port port) {

        int result = canDeletePort(port);
        if  (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deletePort validation failed, result - {}", result);
            return;
        }
        String tenantID = convertNeutronIDToVTNKey(port.getTenantId().getValue());
        String bridgeID = convertNeutronIDToVTNKey(port.getNetworkId().getValue());
        String portID = convertNeutronIDToVTNKey(port.getUuid().getValue());

        result = deleteBridgeInterface(tenantID, bridgeID, portID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deleteBridgeInterface failed for tenant-id - {}, " +
                      "bridge-id - {}, port-id - {}, result - {}",
                      tenantID, bridgeID, portID, result);
        }
    }

    /**
     * Delete an existing virtual interface.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param portID port identifier provided by neutron.
     * @return interface deletion status in HTTP response status code.
     */
    private int deleteBridgeInterface(String tenantID,
                                      String bridgeID,
                                      String portID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;

        VBridgeIfPath path = new VBridgeIfPath(tenantID, bridgeID, portID);
        Status status = getVTNManager().removeInterface(path);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }
}
