/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.networkconfig.neutron.INeutronNetworkAware;
import org.opendaylight.controller.networkconfig.neutron.NeutronNetwork;

import org.opendaylight.vtn.manager.IVTNManager;

/**
 * Handle requests for Neutron Network.
 */
public class NetworkHandler implements INeutronNetworkAware {
    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(NetworkHandler.class);

    /**
     * VTN manager service instance.
     */
    private IVTNManager vtnManager;

    /**
     * Services provide this interface method to indicate if the specified network can be created
     *
     * @param network
     *            instance of proposed new Neutron Network object
     * @return integer
     */
    public int canCreateNetwork(NeutronNetwork network) {
        return 200;
    }

    /**
     * Services provide this interface method for taking action after a network has been created
     *
     * @param network
     *            instance of new Neutron Network object
     * @return void
     */
    public void neutronNetworkCreated(NeutronNetwork network) {
    }

    /**
     * Services provide this interface method to indicate if the specified network can be changed using the specified
     * delta
     *
     * @param delta
     *            updates to the network object using patch semantics
     * @param network
     *            instance of the Neutron Network object to be updated
     * @return integer
     */
    public int canUpdateNetwork(NeutronNetwork delta, NeutronNetwork original) {
        return 200;
    }

    /**
     * Services provide this interface method for taking action after a network has been updated
     *
     * @param network
     *            instance of modified Neutron Network object
     * @return void
     */
    public void neutronNetworkUpdated(NeutronNetwork network) {
    }

    /**
     * Services provide this interface method to indicate if the specified network can be deleted
     *
     * @param network
     *            instance of the Neutron Network object to be deleted
     * @return integer
     */
    public int canDeleteNetwork(NeutronNetwork network) {
        return 200;
    }

    /**
     * Services provide this interface method for taking action after a network has been deleted
     *
     * @param network
     *            instance of deleted Neutron Network object
     * @return void
     */
    public void neutronNetworkDeleted(NeutronNetwork network) {
    }

    /**
     * Invoked when a vtn manager service is registered.
     *
     * @param service  VTN manager service.
     */
    void setVTNManager(IVTNManager service) {
        LOG.debug("Set vtn mange: {}", service);
        vtnManager = service;
    }

    /**
     * Invoked when a vtn manager service is unregistered.
     *
     * @param service  VTN manager service.
     */
    void unsetSwitchManager(IVTNManager service) {
        if (vtnManager == service) {
            LOG.debug("Unset vtn manager: {}", service);
            vtnManager = null;
        }
    }
}
