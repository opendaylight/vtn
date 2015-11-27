/*
 * Copyright (c) 2015 NEC Corporation and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static java.net.HttpURLConnection.HTTP_OK;

import static org.opendaylight.vtn.manager.neutron.impl.VTNNeutronUtils.convertUUIDToKey;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;

public final class NetworkHandler {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory
            .getLogger(NetworkHandler.class);

    /**
     * VTN Manager service.
     */
    private final VTNManagerService  vtnManager;

    /**
     * Construct a new instance.
     *
     * @param vtn  A {@link VTNManagerService} instance.
     */
    public NetworkHandler(VTNManagerService vtn) {
        vtnManager = vtn;
    }

    /**
     * Invoked to take action after a network has been created.
     *
     * @param network  An instance of new Network object.
     */
    public void neutronNetworkCreated(Network network) {
        boolean tenantCreated = false;
        String tenantIdentity = network.getTenantId().getValue();
        String tenantID = convertUUIDToKey(tenantIdentity);
        int result = vtnManager.updateTenant(tenantID, VnodeUpdateMode.UPDATE);
        if (result != HTTP_OK) {
            LOG.error("failed to create network, result - {}", result);
        } else {
            tenantCreated = true;
            String networkDesc = network.getName();
            String bridgeID = convertUUIDToKey(network.getUuid().getValue());
            result = vtnManager.updateBridge(tenantID, bridgeID, networkDesc,
                                             VnodeUpdateMode.CREATE);
            if (result != HTTP_OK) {
                LOG.error("Unable to create vBridge: tenant-id={}, " +
                          "bridge-id={}, result={}", tenantIdentity,
                          network.getUuid().getValue(), result);
            }
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
        VbridgeConfig bconf = vtnManager.getBridgeConfig(tenantID, bridgeID);
        if (bconf == null) {
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
        boolean modify = canModifyBridge(network, bconf);
        String networkDesc = network.getName();
        if (modify) {
            int result = vtnManager.updateBridge(
                tenantID, bridgeID, networkDesc, VnodeUpdateMode.MODIFY);
            if (result != HTTP_OK) {
                LOG.error("Modifying bridge description failed for "
                          + "bridge id - {} , tenant id - {}, result - {}",
                          bridgeID, tenantID, result);
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

        int result = vtnManager.removeBridge(tenantID, bridgeID);
        if (result != HTTP_OK) {
            LOG.error("removeBridge failed for tenant-id - {}, "
                      + "Bridge-id - {}, result - {}", tenantID, bridgeID,
                      result);
        }
        if (!vtnManager.hasBridge(tenantID)) {
            result = vtnManager.removeTenant(tenantID);
            if (result != HTTP_OK) {
                LOG.error("removeTenant failed for tenant-id - {}, "
                          + "result - {}", tenantID, result);
            }
        }
    }

    /**
     * Verify if the virtual bridge can be modified.
     *
     * @param network A {@link Network} instance.
     * @param bridge  The current configuration of the vBridge.
     * @return {@code true} virtual bridge can be modified.
     *         {@code false} virtual bridge need not be modified.
     */
    private boolean canModifyBridge(Network network, VbridgeConfig bridge) {
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
}
