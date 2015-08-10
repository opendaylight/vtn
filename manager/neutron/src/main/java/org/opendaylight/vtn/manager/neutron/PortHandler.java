/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterface;

import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.neutron.spi.INeutronPortAware;
import org.opendaylight.neutron.spi.NeutronPort;

/**
 * Handle requests for Neutron Port.
 */
public class PortHandler extends VTNNeutronUtils
                         implements INeutronPortAware {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(PortHandler.class);

    /**
     * VTN identifiers in neutron port object.
     */
    private static final int VTN_IDENTIFIERS_IN_PORT = 3;

    /**
     * Invoked when a port creation is requested
     * to indicate if the specified port can be created.
     *
     * @param port     An instance of proposed new Port object.
     * @return A HTTP status code to the creation request.
     */
    @Override
    public int canCreatePort(NeutronPort port) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        result = getVTNIdentifiers(port, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canCreatePort getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];
        String portID = vtnIDs[2];

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
        return result;
    }

    /**
     * Invoked to take action after a port has been created.
     *
     * @param port An instance of new Neutron Port object.
     */
    @Override
    public void neutronPortCreated(NeutronPort port) {

        int result = canCreatePort(port);
        if (result != HttpURLConnection.HTTP_CREATED) {
            LOG.error("Port create validation failed, result - {}", result);
            return;
        }

        String tenantID = convertNeutronIDToVTNKey(port.getTenantID());
        String bridgeID = convertNeutronIDToVTNKey(port.getNetworkUUID());
        String portID = convertNeutronIDToVTNKey(port.getID());
        String portDesc = port.getName();
        Boolean portAdminState = port.getAdminStateUp();

        // create vBridge interface.
        result = createBridgeInterface(tenantID,
                                       bridgeID,
                                       portID,
                                       portDesc,
                                       portAdminState);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createBridgeInterface failed for tenant-id - {}, " +
                      "bridge-id - {}, port-id - {}, result - {}",
                      tenantID, bridgeID, portID, result);
            return;
        }

    }

    /**
     * Invoked when a port update is requested
     * to indicate if the specified port can be changed
     * using the specified delta.
     *
     * @param delta    Updates to the port object using patch semantics.
     * @param original An instance of the Neutron Port object
     *                  to be updated.
     * @return A HTTP status code to the update request.
     */
    @Override
    public int canUpdatePort(NeutronPort delta,
                             NeutronPort original) {
        int result = HttpURLConnection.HTTP_OK;
        /**
         * To basic validation of the request
         */

        if ((original == null) || (delta == null)) {
            LOG.error("port object not specified");
            return HttpURLConnection.HTTP_BAD_REQUEST;
        }

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        result = getVTNIdentifiers(original, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canUpdatePort getVTNIdentifiers failed, result - {}",
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
     * Invoked to take action after a port has been updated.
     *
     * @param port An instance of modified Neutron Port object.
     */
    @Override
    public void neutronPortUpdated(NeutronPort port) {
        int result = HttpURLConnection.HTTP_OK;
        /**
         * To basic validation of the request
         */

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_PORT];
        result = getVTNIdentifiers(port, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("neutronPortUpdated getVTNIdentifiers failed, " +
                      "result - {}", result);
            return;
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
            return;
        }

        VInterface vIf = getBridgeInterface(tenantID, bridgeID, portID);
        boolean modify = canModifyInterface(port, vIf);
        String portDesc = port.getName();
        Boolean portAdminState = port.getAdminStateUp();

        if (modify) {
            result = modifyBridgeInterface(tenantID,
                                           bridgeID,
                                           portID,
                                           portDesc,
                                           portAdminState);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("Modifying bridge interface failed for " +
                          "tenant-id - {}, bridge-id - {}, port-id - {}, " +
                          "result - {}", tenantID, bridgeID, portID, result);
                return;
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
    @Override
    public int canDeletePort(NeutronPort port) {
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
    @Override
    public void neutronPortDeleted(NeutronPort port) {

        int result = canDeletePort(port);
        if  (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deletePort validation failed, result - {}", result);
            return;
        }

        String tenantID = convertNeutronIDToVTNKey(port.getTenantID());
        String bridgeID = convertNeutronIDToVTNKey(port.getNetworkUUID());
        String portID = convertNeutronIDToVTNKey(port.getID());

        result = deleteBridgeInterface(tenantID, bridgeID, portID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deleteBridgeInterface failed for tenant-id - {}, " +
                      "bridge-id - {}, port-id - {}, result - {}",
                      tenantID, bridgeID, portID, result);
        }
    }

    /**
     * Validate and return VTN identifiers from the given neutron object.
     *
     * @param port   An instance of Port object.
     * @param vtnIDs VTN identifiers.
     * @return A HTTP status code to the creation request.
     */
    private int getVTNIdentifiers(NeutronPort port, String[] vtnIDs) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        /**
         * To basic validation of the request
         */
        if (port == null) {
            LOG.error("port object not specified");
            return result;
        }

        String tenantUUID = port.getTenantID();
        String bridgeUUID = port.getNetworkUUID();
        String portUUID = port.getID();

        if ((tenantUUID == null) || (bridgeUUID == null) || portUUID == null) {
            LOG.error("neutron identifiers not specified");
            return result;
        }

        String tenantID = convertNeutronIDToVTNKey(tenantUUID);
        if (tenantID == null) {
            LOG.error("Invalid tenant identifier");
            return result;
        }

        String bridgeID = convertNeutronIDToVTNKey(bridgeUUID);
        if (bridgeID == null) {
            LOG.error("Invalid bridge identifier");
            return result;
        }

        String portID = convertNeutronIDToVTNKey(portUUID);
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
     * Verify if the virtual interface can be modified.
     *
     * @param port neutron port object.
     * @param vIf  virtual interface in VTN
     * @return {@code true} virtual interface can be modified.
     *         {@code false} virtual interface need not be modified.
     */
    private boolean canModifyInterface(NeutronPort port, VInterface vIf) {
        /**
         * if vbridge description has changed update the description
         */
        String portDesc = port.getName();
        String ifDesc = vIf.getDescription();
        boolean modify = false;

        if ((portDesc != null) && (ifDesc != null)) {
            if (!portDesc.equals(ifDesc)) {
                /**
                 * both new and old description is not null
                 * and not equal,modify bridge
                 */
                modify = true;
            }
        } else if ((portDesc == null) && (ifDesc == null)) {
            /**
             * both new and old description are null no change
             * dont modify bridge
             */
            modify = false;
        } else {
            /**
             * either new or old description is null,
             * description has changed modify bridge
             */
            modify = true;
        }

        /**
         * if admin state is changed set modify to true
         */
        Boolean portAdminState = port.getAdminStateUp();
        if (portAdminState != null &&
            !portAdminState.equals(vIf.getEnabled())) {
            modify = true;
        }
        return modify;
    }

    /**
     * Returns the virtual L2 bridge interface information specified by
     * the interface identifier.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param portID port identifier provided by neutron.
     * @return interface information associated with the interface.
     */
    private VInterface getBridgeInterface(String tenantID,
                                          String bridgeID,
                                          String portID) {
        VBridgeIfPath path = new VBridgeIfPath(tenantID, bridgeID, portID);
        try {
            return getVTNManager().getInterface(path);
        } catch (VTNException e) {
            LOG.error("getBridgeInterface error, path - {}, e - {}", path,
                      e.toString());
            return null;
        }
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
     * Modify configuration of an existing virtual interface.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param portID port identifier provided by neutron.
     * @param portDesc port description provided by neutron.
     * @param adminState port admin status provided by neutron.
     * @return interface modification status in HTTP response status code.
     */
    private int modifyBridgeInterface(String tenantID,
                                      String bridgeID,
                                      String portID,
                                      String portDesc,
                                      Boolean adminState) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        boolean all = false;

        VInterfaceConfig conf = new VInterfaceConfig(portDesc, adminState);
        VBridgeIfPath path = new VBridgeIfPath(tenantID, bridgeID, portID);
        Status status = getVTNManager().modifyInterface(path, conf, all);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
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
