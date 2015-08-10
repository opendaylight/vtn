/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.neutron.spi.INeutronSubnetAware;
import org.opendaylight.neutron.spi.NeutronSubnet;

/**
 * Handle requests for Neutron Subnet.
 */
public class SubnetHandler extends VTNNeutronUtils
                           implements INeutronSubnetAware {

    /**
     * Logger instance.
     */
    static final Logger LOG = LoggerFactory.getLogger(SubnetHandler.class);

    /**
     * Invoked when a subnet creation is requested
     * to indicate if the specified subnet can be created.
     *
     * @param subnet An instance of the Neutron Subnet object to be created.
     * @return A HTTP status code to the creation request.
     */
    @Override
    public int canCreateSubnet(NeutronSubnet subnet) {
        return HttpURLConnection.HTTP_CREATED;
    }

    /**
     * Invoked when a subnet creation is requested
     * to indicate if the specified subnet can be created.
     *
     * @param subnet  An instance of the Neutron Subnet object to be created.
     */
    @Override
    public void neutronSubnetCreated(NeutronSubnet subnet) {
        // Currently implementation is unavailable for this method.
    }

    /**
     * Invoked when a subnet updation is requested
     * to indicate if the specified subnet can be updated.
     *
     * @param delta     Updates to the Neutron subnet object using patch semantics.
     * @param original  An instance of the Neutron subnet object to be updated.
     * @return A HTTP status code to the creation request.
     */
    @Override
    public int canUpdateSubnet(NeutronSubnet delta, NeutronSubnet original) {
        return HttpURLConnection.HTTP_CREATED;
    }

    /**
     * Invoked when a subnet updation is requested
     * to indicate if the specified subnet can be updated.
     *
     * @param subnet  An instance of the Neutron Subnet object to be updated.
     */
    @Override
    public void neutronSubnetUpdated(NeutronSubnet subnet) {
        // Currently implementation is unavailable for this method.
    }

    /**
     * Invoked when a subnet deletion is requested
     * to indicate if the specified subnet can be deleted.
     *
     * @param subnet  An instance of the Neutron Subnet object to be deleted.
     * @return A HTTP status code to the deletion request.
     */
    @Override
    public int canDeleteSubnet(NeutronSubnet subnet) {
        return HttpURLConnection.HTTP_CREATED;
    }

    /**
     * Invoked when a subnet deletion is requested
     * to indicate if the specified subnet can be deleted.
     *
     * @param subnet  An instance of the Neutron Subnet object to be deleted.
     */
    @Override
    public void neutronSubnetDeleted(NeutronSubnet subnet) {
        // Currently implementation is unavailable for this method.
    }
}
