/**
 * Copyright (c) 2015 NEC Corporation and others.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;
import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;

public class NetworkHandler extends VTNNeutronUtils {

    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory
            .getLogger(NetworkHandler.class);

    /**
     * Invoked to take action after a network has been created.
     *
     * @param network  An instance of new Network object.
     */
    public void neutronNetworkCreated(Network network) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        boolean tenantCreated = false;
        String tenantIdentity = network.getTenantId().getValue();
        String tenantID = convertUUIDToKey(tenantIdentity);
        result = isTenantExist(tenantID);
        if (result == HttpURLConnection.HTTP_NOT_FOUND) {
            result = createTenant(tenantID);
        } else if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("failed to create network, result - {}", result);
            return;
        }
        LOG.info("Result" + result);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createTenant failed for tenant - {}, result - {}",
                    network.getTenantId().getValue(), result);
            return;
        }
        tenantCreated = true;
        String networkDesc = network.getName();
        String bridgeID = convertUUIDToKey(network.getUuid().getValue());
        result = createBridge(tenantID, bridgeID, networkDesc);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createBridge failed for tenant-id {}, bridge-id - {},"
                    + " result - {}", network.getTenantId().getValue(), network
                    .getUuid().getValue(), result);
            return;
        }
    }

    /**
     * Invoked to take action after a network has been updated.
     *
     * @param network An instance of modified Network object.
     */
    public void neutronNetworkUpdated(Network network) {
        /**
         * check if the object already exists in VTN manager
         */
        String tenantID = convertUUIDToKey(network.getTenantId().getValue());
        String bridgeID = convertUUIDToKey(network.getUuid().getValue());
        int result = isBridgeExist(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Tenant id - {} or  bridge id - {} does not exist",
                    tenantID, bridgeID);
            return;
        }
        Boolean shared = network.isShared();
        if ((shared != null) && (shared)) {
            LOG.error("Shared option - {} not supported in vtn", shared);
            return;
        }
        /**
         * if vbridge description has changed update the description
         */
        VBridge bridge = getBridge(tenantID, bridgeID);
        boolean modify = canModifyBridge(network, bridge);
        String networkDesc = network.getName();
        if (modify) {
            result = modifyBridge(tenantID, bridgeID, networkDesc);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("Modifying bridge description failed for "
                        + "bridge id - {} , tenant id - {}, result - {}",
                        bridgeID, tenantID, result);
                return;
            }
        }
    }

    /**
     * Invoked to take action after a network has been deleted.
     *
     * @param network  An instance of deleted Network object.
     */
    public void neutronNetworkDeleted(Network network) {
        String tenantID = convertUUIDToKey(network.getTenantId().getValue());
        String bridgeID = convertUUIDToKey(network.getUuid().getValue());
        int result = canDeleteNetwork(network);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deleteNetwork validation failed for result - {}", result);
            return;
        }
        result = deleteBridge(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deleteBridge failed for tenant-id - {}, "
                    + "Bridge-id - {}, result - {}", tenantID, bridgeID, result);
        }
        if (!isVbridgesInTenant(tenantID)) {
            result = deleteTenant(tenantID);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("deleteTenant failed for tenant-id - {}, "
                        + "result - {}", tenantID, result);
            }
        }
    }

    /**
     * Invoked when a network deletion is requested
     * to indicate if the specified network can be deleted.
     *
     * @param network  An instance of the Network object to be deleted.
     * @return A HTTP status code to the deletion request.
     */
    public int canDeleteNetwork(Network network) {
        int result = HttpURLConnection.HTTP_OK;
        /**
         * check if the object already exists in VTN manager
         */
        String tenantID = convertUUIDToKey(network.getTenantId().getValue());
        String bridgeID = convertUUIDToKey(network.getUuid().getValue());
        result = isBridgeExist(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            /**
             * A with given network id does not exists return error
             */
            result = HttpURLConnection.HTTP_NOT_FOUND;
        }
        return result;
    }

    /**
     * Create a new virtual tenant.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @return tenant creation status in HTTP response status code.
     */
    private int createTenant(String tenantID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VTenantConfig conf = new VTenantConfig(null);

        VTenantPath path = new VTenantPath(tenantID);
        Status status = getVTNManager().addTenant(path, conf);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }

    /**
     * Delete an existing tenant.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @return tenant deletion status in HTTP response status code.
     */
    private int deleteTenant(String tenantID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VTenantPath path = new VTenantPath(tenantID);
        Status status = getVTNManager().removeTenant(path);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }

    /**
     * Returns a list of all virtual tenants.
     *
     * @return A list of virtual tenant.
     */
    private List<VTenant> getTenants() {
        try {
            return getVTNManager().getTenants();
        } catch (VTNException e) {
            LOG.error("getTenants error, {}", e.toString());
            return null;
        }
    }

    /**
     * verify if the virtual L2 bridge can be created.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param bridgeDesc bridge description provided by neutron.
     * @return Create bridge validation status in HTTP response status code.
     */
    private int createBridge(String tenantID, String bridgeID, String bridgeDesc) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VBridgeConfig conf = new VBridgeConfig(bridgeDesc);
        VBridgePath path = new VBridgePath(tenantID,
                bridgeID);
        Status status = getVTNManager().addBridge(path, conf);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }

    /**
     * Verify if the virtual bridge can be modified.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param bridgeDesc bridge description provided by neutron.
     * @return Modify bridge validation status in HTTP response status code.
     */
    private int modifyBridge(String tenantID, String bridgeID, String bridgeDesc) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        boolean all = false;
        VBridgeConfig conf = new VBridgeConfig(bridgeDesc);
        VBridgePath path = new VBridgePath(tenantID,
                bridgeID);
        Status status = getVTNManager().modifyBridge(path, conf, all);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }

    /**
     * Delete an existing virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @return bridge deletion status in HTTP response status code.
     */
    private int deleteBridge(String tenantID, String bridgeID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VBridgePath path = new VBridgePath(tenantID,
                bridgeID);
        Status status = getVTNManager().removeBridge(path);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }

    /**
     * Returns a list of all L2 virtual bridges in the virtual tenant.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @return A list of virtual L2 bridges in the tenant.
     */
    private List<VBridge> getBridges(String tenantID) {
        VTenantPath path = new VTenantPath(tenantID);
        try {
            return getVTNManager().getBridges(path);
        } catch (VTNException e) {
            LOG.error("getBridges error, path - {}, e - {}", path, e.toString());
            return null;
        }
    }

    /**
     * Returns the virtual L2 bridge information specified by the
     * bridge identifier.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @return L2 bridge information associated with the bridge.
     */
    private VBridge getBridge(String tenantID, String bridgeID) {
        VBridgePath path = new VBridgePath(tenantID, bridgeID);
        try {
            return getVTNManager().getBridge(path);
        } catch (VTNException e) {
            LOG.error("getBridge error, path - {}, e - {}", path, e.toString());
            return null;
        }
    }

    /**
     * Verify if the virtual bridge can be modified.
     *
     * @param network object.
     * @param bridge  virtual bridge in VTN
     * @return {@code true} virtual bridge can be modified.
     *         {@code false} virtual bridge need not be modified.
     */
    private boolean canModifyBridge(Network network, VBridge bridge) {
        String networkDesc = network.getName();
        String bridgeDesc = bridge.getDescription();
        boolean modify = false;
        if ((networkDesc != null) && (bridgeDesc != null)) {
            if (!networkDesc.equals(bridgeDesc)) {
                /**
                 * both new and old description is not null and not equal,modify
                 * bridge
                 */
                modify = true;
            }
        } else if ((networkDesc == null) && (bridgeDesc == null)) {
            /**
             * both new and old description are null no change dont modify
             * bridge
             */
            modify = false;
        } else {
            /**
             * either new or old description is null, description has changed
             * modify bridge
             */
            modify = true;
        }
        return modify;
    }

    /**
     * Check if virtual L2 bridges exist in a tenant.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @return  {@code true} if bridges exist in tenant.
     *          {@code false} if no bridges exist in tenant.
     */
    private boolean isVbridgesInTenant(String tenantID) {
        List<VBridge> list = getBridges(tenantID);
        if ((list != null) && (!list.isEmpty())) {
            return true;
        } else {
            LOG.error("vbridge list is null or empty for tenant-id - {}",
                    tenantID);
            return false;
        }
    }
}
