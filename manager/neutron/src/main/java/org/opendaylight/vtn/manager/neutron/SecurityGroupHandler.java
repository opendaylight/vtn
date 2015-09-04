/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;

import org.opendaylight.neutron.spi.INeutronSecurityGroupAware;
import org.opendaylight.neutron.spi.INeutronSecurityRuleAware;
import org.opendaylight.neutron.spi.NeutronSecurityGroup;
import org.opendaylight.neutron.spi.NeutronSecurityRule;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Handle requests for OpenStack Neutron v2.0 Port Security API calls.
 */

public class SecurityGroupHandler extends VTNNeutronUtils
                                implements INeutronSecurityGroupAware, INeutronSecurityRuleAware {
    private static final Logger LOG = LoggerFactory.getLogger(SecurityGroupHandler.class);

    @Override
    public int canCreateNeutronSecurityGroup(NeutronSecurityGroup neutronSecurityGroup) {
        return HttpURLConnection.HTTP_CREATED;
    }

    @Override
    public void neutronSecurityGroupCreated(NeutronSecurityGroup neutronSecurityGroup) {
        int result = canCreateNeutronSecurityGroup(neutronSecurityGroup);
        LOG.trace("Neutron Security Group creation created {} ", result);
        if (result != HttpURLConnection.HTTP_CREATED) {
            LOG.debug("Neutron Security Group creation failed {} ", result);
        }
    }

    @Override
    public int canUpdateNeutronSecurityGroup(NeutronSecurityGroup delta, NeutronSecurityGroup original) {
        return HttpURLConnection.HTTP_OK;
    }

    @Override
    public void neutronSecurityGroupUpdated(NeutronSecurityGroup neutronSecurityGroup) {
        //TODO
    }

    @Override
    public int canDeleteNeutronSecurityGroup(NeutronSecurityGroup neutronSecurityGroup) {
        return HttpURLConnection.HTTP_OK;
    }

    @Override
    public void neutronSecurityGroupDeleted(NeutronSecurityGroup neutronSecurityGroup) {
        //TODO: Trigger flowmod removals
        int result = canDeleteNeutronSecurityGroup(neutronSecurityGroup);
        LOG.trace(" deleted Neutron Security Rule {} ", result);
        if  (result != HttpURLConnection.HTTP_OK) {
            LOG.error(" delete Neutron Security Rule validation failed for result - {} ", result);
        }
    }

    /**
     * Invoked when a Security Rules creation is requested
     * to indicate if the specified Rule can be created.
     *
     * @param neutronSecurityRule  An instance of proposed new Neutron Security Rule object.
     * @return A HTTP status code to the creation request.
     */

    @Override
    public int canCreateNeutronSecurityRule(NeutronSecurityRule neutronSecurityRule) {
        return HttpURLConnection.HTTP_CREATED;
    }

    @Override
    public void neutronSecurityRuleCreated(NeutronSecurityRule neutronSecurityRule) {
        int result = canCreateNeutronSecurityRule(neutronSecurityRule);
        LOG.trace("Neutron Security Group creation {} ", result);
        if (result != HttpURLConnection.HTTP_CREATED) {
            LOG.debug("Neutron Security Group creation failed {} ", result);
        }
    }

    @Override
    public int canUpdateNeutronSecurityRule(NeutronSecurityRule delta, NeutronSecurityRule original) {
        return HttpURLConnection.HTTP_OK;
    }

    @Override
    public void neutronSecurityRuleUpdated(NeutronSecurityRule neutronSecurityRule) {
        //TODO
    }

    @Override
    public int canDeleteNeutronSecurityRule(NeutronSecurityRule neutronSecurityRule) {
        return HttpURLConnection.HTTP_OK;
    }

    @Override
    public void neutronSecurityRuleDeleted(NeutronSecurityRule neutronSecurityRule) {
        int result = canDeleteNeutronSecurityRule(neutronSecurityRule);
        LOG.trace(" delete Neutron Security Rule validation passed for result - {} ", result);
        if  (result != HttpURLConnection.HTTP_OK) {
            LOG.error(" delete Neutron Security Rule validation failed for result - {} ", result);
        }
    }
}
