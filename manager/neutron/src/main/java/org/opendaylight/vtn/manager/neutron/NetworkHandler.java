/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;
import java.util.List;
import java.util.Iterator;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.VlanMap;

import org.opendaylight.controller.sal.utils.Status;

import org.opendaylight.neutron.spi.INeutronNetworkAware;
import org.opendaylight.neutron.spi.NeutronNetwork;

/**
 * Handle requests for Neutron Network.
 */
public class NetworkHandler extends VTNNeutronUtils
                            implements INeutronNetworkAware {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(NetworkHandler.class);

   /**
     * VTN identifiers in neutron network object.
     */
    private static final int VTN_IDENTIFIERS_IN_NETWORK = 2;

    /**
     * Provider segmentation type supported.
     */
    private static final String VTN_SUPPORTED_NETWORK_TYPE = "vlan";

    /**
     * Invoked when a network creation is requested
     * to indicate if the specified network can be created.
     *
     * @param network  An instance of proposed new Neutron Network object.
     * @return A HTTP status code to the creation request.
     */
    @Override
    public int canCreateNetwork(NeutronNetwork network) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_NETWORK];
        result = getVTNIdentifiers(network, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canCreateNetwork getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];

        LOG.trace("tenantID - {}, bridgeID - {}", tenantID, bridgeID);

        result = canCreateBridge(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canCreateBridge failed for tenant-id - {}, " +
                      "bridge-id - {}, result - {}",
                      tenantID, bridgeID, result);
            return result;
        }

        Boolean shared = network.getShared();
        if ((shared != null) && (shared)) {
            LOG.error("Network shared attribute not supported");
            return HttpURLConnection.HTTP_NOT_ACCEPTABLE;
        }

        result = canCreateVlanMap(network);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canCreateVlanMap failed for tenant-id - {}, " +
                      "bridge-id - {}, result - {}",
                      tenantID, bridgeID, result);
            return result;
        }

        return HttpURLConnection.HTTP_CREATED;
    }

    /**
     * Invoked to take action after a network has been created.
     *
     * @param network  An instance of new Neutron Network object.
     */
    @Override
    public void neutronNetworkCreated(NeutronNetwork network) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;

        result = canCreateNetwork(network);
        if (result != HttpURLConnection.HTTP_CREATED) {
            LOG.error("Bridge create validation failed for result - {}",
                      result);
            return;
        }

        String tenantID = convertNeutronIDToVTNKey(network.getTenantID());
        String bridgeID = convertNeutronIDToVTNKey(network.getID());

        boolean tenantCreated = false;
        result = isTenantExist(tenantID);

        if (result == HttpURLConnection.HTTP_NOT_FOUND) {
            // create vtn
            result = createTenant(tenantID);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("createTenant failed for tenant - {}, result - {}",
                          tenantID, result);
                return;
            }
            tenantCreated = true;
        } else if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("failed to create network, result - {}", result);
            return;
        }

        boolean bridgeCreated = false;
        // create bridge.
        String networkDesc = network.getNetworkName();

        result = createBridge(tenantID, bridgeID, networkDesc);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createBridge failed for tenant-id {}, bridge-id - {}," +
                      " result - {}", tenantID, bridgeID, result);

            // delete Tenant if created
            if (tenantCreated) {
                result = deleteTenant(tenantID);
                if (result != HttpURLConnection.HTTP_OK) {
                    LOG.error("deleteTenant failed for tenant-id - {}, " +
                              "result - {}", tenantID, result);
                }
            }
            return;
        }
        bridgeCreated = true;

        // create vlanmap
        result = createVlanMap(network, tenantCreated, bridgeCreated);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createVlanMap failed for vlan id - {}, result - {}",
                      network.getProviderSegmentationID(), result);

        }
        return;
    }

    /**
     * Invoked when a network update is requested
     * to indicate if the specified network can be changed
     * using the specified delta.
     *
     * @param delta     Updates to the network object using patch semantics.
     * @param original  An instance of the Neutron Network object
     *                  to be updated.
     * @return A HTTP status code to the update request.
     */
    @Override
    public int canUpdateNetwork(NeutronNetwork delta,
                                NeutronNetwork original) {
        int result = HttpURLConnection.HTTP_OK;
        /**
         * To basic validation of the request
         */
        if ((original == null) || (delta == null)) {
            LOG.error("network object not specified");
            return HttpURLConnection.HTTP_BAD_REQUEST;
        }

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_NETWORK];
        result = getVTNIdentifiers(original, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canUpdateNetwork getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];

        /**
         * check if the object already exists in VTN manager
         */
        result = isBridgeExist(tenantID, bridgeID);
        if (result == HttpURLConnection.HTTP_OK) {
            Boolean shared = delta.getShared();
            if ((shared != null) && (shared)) {
                return HttpURLConnection.HTTP_NOT_ACCEPTABLE;
            }

            result = canModifyVlanMap(delta, original);
        }

        return result;
    }

    /**
     * Invoked to take action after a network has been updated.
     *
     * @param network An instance of modified Neutron Network object.
     */
    @Override
    public void neutronNetworkUpdated(NeutronNetwork network) {

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_NETWORK];
        int result = getVTNIdentifiers(network, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("neutronNetworkUpdated getVTNIdentifiers failed, " +
                      "result - {}", result);
            return;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];

        /**
         * check if the object already exists in VTN manager
         */
        result = isBridgeExist(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("Tenant id - {} or  bridge id - {} does not exist",
                      tenantID, bridgeID);
            return;
        }

        Boolean shared = network.getShared();
        if ((shared != null) && (shared)) {
            LOG.error("Shared option - {} not supported in vtn", shared);
            return;
        }

        /**
         * if vbridge description has changed update the description
         */
        VBridge bridge = getBridge(tenantID, bridgeID);
        boolean modify = canModifyBridge(network, bridge);
        String networkDesc = network.getNetworkName();

        if (modify) {
            result = modifyBridge(tenantID, bridgeID, networkDesc);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("Modifying bridge description failed for " +
                          "bridge id - {} , tenant id - {}, result - {}",
                          bridgeID, tenantID, result);
                return;
            }
        }

        result = modifyVlanMap(network);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("modifyVlanMap failed for vlan id - {}, result - {}",
                      network.getProviderSegmentationID(), result);
        }
        return;
    }

    /**
     * Invoked when a network deletion is requested
     * to indicate if the specified network can be deleted.
     *
     * @param network  An instance of the Neutron Network object to be deleted.
     * @return A HTTP status code to the deletion request.
     */
    @Override
    public int canDeleteNetwork(NeutronNetwork network) {
        int result = HttpURLConnection.HTTP_OK;

        String[] vtnIDs = new String[VTN_IDENTIFIERS_IN_NETWORK];
        result = getVTNIdentifiers(network, vtnIDs);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("canDeleteNetwork getVTNIdentifiers failed, result - {}",
                      result);
            return result;
        }
        String tenantID = vtnIDs[0];
        String bridgeID = vtnIDs[1];

        /**
         * check if the object already exists in VTN manager
         */
        result = isBridgeExist(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            /**
             * A  with given network id does not exists return error
             */
            result = HttpURLConnection.HTTP_NOT_FOUND;
        }
        return result;
    }

    /**
     * Invoked to take action after a network has been deleted.
     *
     * @param network  An instance of deleted Neutron Network object.
     */
    @Override
    public void neutronNetworkDeleted(NeutronNetwork network) {

        int result = canDeleteNetwork(network);
        if  (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deleteNetwork validation failed for result - {}",
                      result);
            return;
        }

        String tenantID = convertNeutronIDToVTNKey(network.getTenantID());
        String bridgeID = convertNeutronIDToVTNKey(network.getID());
        result = deleteBridge(tenantID, bridgeID);
        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("deleteBridge failed for tenant-id - {}, " +
                      "Bridge-id - {}, result - {}",
                      tenantID, bridgeID, result);
        }

        if (!isVbridgesInTenant(tenantID)) {
            result = deleteTenant(tenantID);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("deleteTenant failed for tenant-id - {}, " +
                          "result - {}", tenantID, result);
            }
        }
    }

    /**
     * Validate and return VTN identifiers from the given neutron object.
     *
     * @param network   An instance of network object.
     * @param vtnIDs VTN identifiers.
     * @return Get VTN identifiers status in HTTP response status code.
     */
    private int getVTNIdentifiers(NeutronNetwork network, String[] vtnIDs) {
        int result = HttpURLConnection.HTTP_BAD_REQUEST;
        /**
         * To basic validation of the request
         */
        if (network == null) {
            LOG.error("network object not specified");
            return result;
        }

        String tenantUUID = network.getTenantID();
        String bridgeUUID = network.getID();

        if ((tenantUUID == null) || (bridgeUUID == null)) {
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

        vtnIDs[0] = tenantID;
        vtnIDs[1] = bridgeID;

        return HttpURLConnection.HTTP_OK;
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
     * Check if virtual L2 bridges exist in a tenant.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @return  {@code true} if bridges exist in tenant.
     *          {@code false} if no bridges exist in tenant.
     */
    private boolean isVbridgesInTenant(String tenantID) {
        List<VBridge> list = getBridges(tenantID);
        if ((list != null) && (!list.isEmpty()))  {
            return true;
        } else {
            LOG.error("vbridge list is null or empty for tenant-id - {}",
                      tenantID);
            return false;
        }
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
            LOG.error("getBridges error, path - {}, e - {}", path,
                      e.toString());
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
            LOG.error("getBridge error, path - {}, e - {}", path,
                      e.toString());
            return null;
        }
    }

    /**
     * verify if the virtual L2 bridge can be created.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @return Create bridge validation status in HTTP response status code.
     */
    private int canCreateBridge(String tenantID, String bridgeID) {
        /**
         * check if the object already exists in VTN manager
         */
        int result = isBridgeExist(tenantID, bridgeID);
        if (result == HttpURLConnection.HTTP_OK) {
            /**
             * A  with given network id exists return error
             */
            result = HttpURLConnection.HTTP_CONFLICT;
        } else if (result == HttpURLConnection.HTTP_NOT_FOUND) {
            /**
             * A  with given network id not found
             * return success
             */
            result = HttpURLConnection.HTTP_OK;
        }
        return result;
    }

    /**
     * Create a new virtual L2 bridge in a tenant.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param bridgeDesc bridge description provided by neutron.
     * @return bridge creation status in HTTP response status code.
     */
    private int createBridge(String tenantID,
                             String bridgeID,
                             String bridgeDesc) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VBridgeConfig conf = new VBridgeConfig(bridgeDesc);
        VBridgePath path = new VBridgePath(tenantID, bridgeID);
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
     * @param network neutron network object.
     * @param bridge  virtual bridge in VTN
     * @return {@code true} virtual bridge can be modified.
     *         {@code false} virtual bridge need not be modified.
     */
    private boolean canModifyBridge(NeutronNetwork network, VBridge bridge) {
        String networkDesc = network.getNetworkName();
        String bridgeDesc = bridge.getDescription();
        boolean modify = false;

        if ((networkDesc != null) && (bridgeDesc != null)) {
            if (!networkDesc.equals(bridgeDesc)) {
                /**
                 * both new and old description is not null
                 * and not equal,modify bridge
                 */
                modify = true;
            }
        } else if ((networkDesc == null) && (bridgeDesc == null)) {
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
        return modify;
    }

    /**
     * Modify configuration of an existing virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param bridgeDesc bridge description provided by neutron.
     * @return bridge modification status in HTTP response status code.
     */
    private int modifyBridge(String tenantID,
                             String bridgeID,
                             String bridgeDesc) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        boolean all = false;

        VBridgeConfig conf = new VBridgeConfig(bridgeDesc);
        VBridgePath path = new VBridgePath(tenantID, bridgeID);
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

        VBridgePath path = new VBridgePath(tenantID, bridgeID);
        Status status = getVTNManager().removeBridge(path);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
        } else {
            result = getException(status);
        }
        return result;
    }

    /**
     * Check if VLAN map configuration exist in vtn manager.
     *
     * @param providerID VLAN identifier provided by neutron.
     * @return  VLAN map exist status in HTTP response status code.
     */
    private int isVlanMapExist(String providerID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        short vlanID = 0;
        try {
            vlanID = Short.parseShort(providerID);
            List<VTenant> listTenant = getTenants();
            if (listTenant == null) {
                LOG.trace("isVlanMapExist listTenant is null");
                return result;
            }
            Iterator itrTenant = listTenant.iterator();
            while (itrTenant.hasNext()) {
                VTenant tmpTenant = (VTenant)itrTenant.next();
                String tenantID = tmpTenant.getName();
                LOG.trace("tenant-id - {}", tenantID);
                List<VBridge> listBridge = getBridges(tenantID);
                if (listBridge == null) {
                    LOG.trace("isVlanMapExist listBridge is null");
                    continue;
                }
                Iterator itrBridge = listBridge.iterator();
                while (itrBridge.hasNext()) {
                    VBridge tmpBridge = (VBridge)itrBridge.next();
                    String bridgeID = tmpBridge.getName();
                    LOG.trace("bridge-id - {}", bridgeID);

                    VlanMapConfig conf = new VlanMapConfig(null, vlanID);
                    VBridgePath path = new VBridgePath(tenantID, bridgeID);
                    if (getVTNManager().getVlanMap(path, conf) != null) {
                        result = HttpURLConnection.HTTP_OK;
                        return result;
                    }
                }
            }
        } catch (NumberFormatException nfe) {
            LOG.error("Execption NFE, vlan-id - {}", providerID);
            return HttpURLConnection.HTTP_BAD_REQUEST;
        } catch (VTNException e) {
            LOG.error("Caught VTNException, vlan-id -  " + providerID, e);
            result = getException(e.getStatus());
        }
        return result;
    }

    /**
     * Returns a list of all VLAN mappings in a virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @return A list of VLAN mappings in the bridge.
     */
    private List<VlanMap> getVlanMaps(String tenantID, String bridgeID) {
        VBridgePath path = new VBridgePath(tenantID, bridgeID);
        try {
            return getVTNManager().getVlanMaps(path);
        } catch (VTNException e) {
            LOG.error("getVlanMaps error, path - {}, e - {}", path,
                      e.toString());
            return null;
        }
    }

    /**
     * verify if the vlan map can be created.
     *
     * @param network  An instance of proposed new Neutron Network object.
     * @return Create Vlanmap validation status in HTTP response status code.
     */
    private int canCreateVlanMap(NeutronNetwork network) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;

        if (network.getProviderNetworkType() == null) {
            return HttpURLConnection.HTTP_OK;
        }

        String networkType = network.getProviderNetworkType();
        if (!networkType.equals(VTN_SUPPORTED_NETWORK_TYPE)) {
            return HttpURLConnection.HTTP_OK;
        }

        // validate vlanmap id
        if (network.getProviderSegmentationID() == null) {
            LOG.error("Segmentation ID not provided");
            return HttpURLConnection.HTTP_BAD_REQUEST;
        }

        String vlanID = network.getProviderSegmentationID();
        result = isVlanMapExist(vlanID);
        if (result == HttpURLConnection.HTTP_NOT_FOUND) {
            result = HttpURLConnection.HTTP_OK;
        } else if (result == HttpURLConnection.HTTP_OK) {
            LOG.error("Vlan-id - {} alreay in use", vlanID);
            result = HttpURLConnection.HTTP_CONFLICT;
        }
        return result;
    }

    /**
     * Create a new VLAN mapping to the virtual L2 bridge.
     *
     * @param network  An instance of new Neutron Network object.
     * @param tenantCreated new tenant created flag
     * @param bridgeCreated new bridge created flag
     * @return VLAN mapping creation status in HTTP response status code.
     */
    private int createVlanMap(NeutronNetwork network,
                              boolean tenantCreated,
                              boolean bridgeCreated) {

        if (network == null) {
            LOG.error("Invalid request for CreateVlanMap");
            return HttpURLConnection.HTTP_BAD_REQUEST;
        }

        String providerType = network.getProviderNetworkType();
        String providerID = network.getProviderSegmentationID();
        if ((providerType == null) || (providerID == null)) {
            return HttpURLConnection.HTTP_OK;
        }

        String tenantID = convertNeutronIDToVTNKey(network.getTenantID());
        String bridgeID = convertNeutronIDToVTNKey(network.getID());

        int result = createVlanMap(tenantID,
                                   bridgeID,
                                   providerType,
                                   providerID);

        if (result != HttpURLConnection.HTTP_OK) {
            LOG.error("createVlanMap failed for vlan id - {}, result - {}",
                      providerID, result);

            // delete bridge, tenant vtn if created
            if (bridgeCreated) {
                result = deleteBridge(tenantID, bridgeID);
                if (result != HttpURLConnection.HTTP_OK) {
                    LOG.error("deleteBridge failed for tenant-id - {}, " +
                              "Bridge-id - {}, result - {}",
                              tenantID, bridgeID, result);
                }
            }
            if (tenantCreated) {
                result = deleteTenant(tenantID);
                if (result != HttpURLConnection.HTTP_OK) {
                    LOG.error("deleteTenant failed for tenant-id - {}, " +
                              "result - {}", tenantID, result);
                }
            }
        }
        return result;
    }

    /**
     * Create a new VLAN mapping to the virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param vlanID vlan identifier provided by neutron.
     * @return VLAN mapping creation status in HTTP response status code.
     */
    private int createVlanMap(String tenantID, String bridgeID, short vlanID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;
        VlanMapConfig conf = new VlanMapConfig(null, vlanID);
        VBridgePath path = new VBridgePath(tenantID, bridgeID);
        try {
            getVTNManager().addVlanMap(path, conf);
            result = HttpURLConnection.HTTP_OK;
        } catch (VTNException e) {
            LOG.error("createVlanMap error, path - {}, conf - {}, e - {}",
                      path, conf, e.toString());
            result = getException(e.getStatus());
        }
        return result;
    }

    /**
     * Create a new VLAN mapping to the virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param providerType provider segmentation type provided by neutron.
     * @param providerID provider segementation identifier provided by neutron.
     * @return VLAN mapping creation status in HTTP response status code.
     */
    private int createVlanMap(String tenantID,
                              String bridgeID,
                              String providerType,
                              String providerID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;

        if (!providerType.equals(VTN_SUPPORTED_NETWORK_TYPE)) {
            return HttpURLConnection.HTTP_OK;
        }

        try {
            short vlanID = 0;
            if ((providerID != null) && (!providerID.isEmpty())) {
                vlanID = Short.parseShort(providerID);
            }
            result = createVlanMap(tenantID, bridgeID, vlanID);
        } catch (NumberFormatException nfe) {
            LOG.error("Exception NFE vlan ID - {}", providerID);
            result = HttpURLConnection.HTTP_BAD_REQUEST;
        }
        return result;
    }

    /**
     * verify if the vlan map can be modified.
     *
     * @param delta     Updates to the network object using patch semantics.
     * @param original  An instance of the Neutron Network object
     *                  to be updated.
     * @return Modify vlanmap validation status in HTTP response status code.
     */
    private int canModifyVlanMap(NeutronNetwork delta,
                                 NeutronNetwork original) {
        int result = HttpURLConnection.HTTP_OK;
        boolean validateVlanMap = false;
        String deltaProviderID = delta.getProviderSegmentationID();
        String deltaProviderType = delta.getProviderNetworkType();
        String orgProviderType = original.getProviderNetworkType();

        if (deltaProviderID != null) {
            if (deltaProviderType != null) {
                if (deltaProviderType.equals(VTN_SUPPORTED_NETWORK_TYPE)) {
                    validateVlanMap = true;
                }
            } else if (orgProviderType != null) {
                if (orgProviderType.equals(VTN_SUPPORTED_NETWORK_TYPE)) {
                    validateVlanMap = true;
                }
            } else {
                LOG.error("Invalid network type");
                return HttpURLConnection.HTTP_BAD_REQUEST;
            }

            if (validateVlanMap) {
                // validate vlanmap id
                result = isVlanMapExist(deltaProviderID);
                if (result == HttpURLConnection.HTTP_NOT_FOUND) {
                    result = HttpURLConnection.HTTP_OK;
                } else if (result == HttpURLConnection.HTTP_OK) {
                    LOG.error("Vlan-id - {} alreay in use", deltaProviderID);
                    result = HttpURLConnection.HTTP_CONFLICT;
                }
            }
        }
        return result;
    }

    /**
     * Modify existing VLAN mapping configuration in the virtual L2 bridge.
     *
     * @param network An instance of modified Neutron Network object.
     * @return VLAN mapping modification status in HTTP response status code.
     */
    private int modifyVlanMap(NeutronNetwork network) {
        int result = HttpURLConnection.HTTP_OK;

        LOG.trace("modify vlanmap");
        if (network == null) {
            LOG.error("Invalid request for modifyVlanMap");
            return HttpURLConnection.HTTP_BAD_REQUEST;
        }

        String providerType = network.getProviderNetworkType();
        String providerID = network.getProviderSegmentationID();

        if ((providerType == null) || (providerID == null)) {
            LOG.trace("Provider settings not configured return OK");
            return HttpURLConnection.HTTP_OK;
        }

        String tenantID = convertNeutronIDToVTNKey(network.getTenantID());
        String bridgeID = convertNeutronIDToVTNKey(network.getID());

        if (providerType.equals(VTN_SUPPORTED_NETWORK_TYPE)) {
            result = deleteVlanMaps(tenantID, bridgeID);
            LOG.trace("modify vlanmap deleteVlanMaps, result - {}", result);
            if ((result == HttpURLConnection.HTTP_OK) ||
                   (result == HttpURLConnection.HTTP_NOT_FOUND)) {
                result = createVlanMap(tenantID,
                                       bridgeID,
                                       providerType,
                                       providerID);
            }
        } else {
            /**
             *  provider segmentation type modified to non vlan
             *  delete existing vlan map
             */
            if (isVlanMapExist(providerID) == HttpURLConnection.HTTP_OK) {
                result = deleteVlanMaps(tenantID, bridgeID);
            }
        }
        return result;
    }

    /**
     * Delete existing VLAN mapping configuration in the virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @param mapID provider segementation identifier provided by neutron.
     * @return VLAN mapping deletion status in HTTP response status code.
     */
    private int deleteVlanMap(String tenantID,
                              String bridgeID,
                              String mapID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;

        LOG.trace("tenant-id - {}, bridge-id - {}, map-id - {}",
                  tenantID, bridgeID,  mapID);
        VBridgePath path = new VBridgePath(tenantID, bridgeID);
        Status status = getVTNManager().removeVlanMap(path, mapID);
        if (status.isSuccess()) {
            result = HttpURLConnection.HTTP_OK;
            LOG.trace("deleteVlanMap result - {}", result);
        } else {
            result = getException(status);
            LOG.error("deleteVlanMap error. path - {}, map-id - {}, " +
                      "result - {}", path, mapID, result);
        }
        return result;
    }

    /**
     * Delete all VLAN mapping configuration in the virtual L2 bridge.
     *
     * @param tenantID tenant identifier provided by neutron.
     * @param bridgeID bridge identifier provided by neutron.
     * @return VLAN mapping deletion status in HTTP response status code.
     */
    private int deleteVlanMaps(String tenantID, String bridgeID) {
        int result = HttpURLConnection.HTTP_NOT_FOUND;

        List<VlanMap> list = getVlanMaps(tenantID, bridgeID);
        if (list == null) {
            return result;
        }
        Iterator itr = list.iterator();
        while (itr.hasNext()) {
            VlanMap tmpVlanMap = (VlanMap)itr.next();
            String mapID = tmpVlanMap.getId();
            result = deleteVlanMap(tenantID, bridgeID, mapID);
            if (result != HttpURLConnection.HTTP_OK) {
                LOG.error("deleteVlanMap failed for tenant-id - {}, " +
                          "bridge-id - {}, map-id - {}, result - {}",
                          tenantID, bridgeID, mapID, result);
                break;
            }
        }
        return result;
    }
}
