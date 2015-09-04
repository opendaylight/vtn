/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.IVTNManager;

import org.opendaylight.controller.sal.core.ComponentActivatorAbstractBase;

import org.opendaylight.neutron.spi.INeutronNetworkAware;
import org.opendaylight.neutron.spi.INeutronPortAware;
import org.opendaylight.neutron.spi.INeutronPortCRUD;
import org.opendaylight.neutron.spi.INeutronSubnetAware;
import org.opendaylight.neutron.spi.INeutronSecurityGroupAware;

import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbConfigurationService;
import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbConnectionService;
import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbInventoryListener;

/**
 * OSGi bundle activator for the VTN Neutron Interface.
 */
public class Activator extends ComponentActivatorAbstractBase {
    /**
     * Function called when the activator starts just after some
     * initializations are done by the
     * ComponentActivatorAbstractBase.
     */
    @Override
    public void init() {
    }

    /**
     * Function called when the activator stops just before the
     * cleanup done by ComponentActivatorAbstractBase.
     *
     */
    @Override
    public void destroy() {
    }

    /**
     * Function that is used to communicate to dependency manager the
     * list of known implementations for services inside a container.
     *
     * @return An array containing all the CLASS objects that will be
     * instantiated in order to get an fully working implementation
     * Object
     */
    @Override
    public Object[] getImplementations() {
        Object[] res = {NetworkHandler.class,
                        PortHandler.class,
                        SubnetHandler.class,
                        SecurityGroupHandler.class,
                        OVSDBPluginEventHandler.class};
        return res;
    }

    /**
     * Function that is called when configuration of the dependencies
     * is required.
     *
     * @param c dependency manager Component object, used for
     * configuring the dependencies exported and imported
     * @param imp Implementation class that is being configured,
     * needed as long as the same routine can configure multiple
     * implementations
     * @param containerName The containerName being configured, this allow
     * also optional per-container different behavior if needed, usually
     * should not be the case though.
     */
    @Override
    public void configureInstance(Component c, Object imp,
                                  String containerName) {
        if (imp.equals(NetworkHandler.class)) {
            // Export the services.
            c.setInterface(INeutronNetworkAware.class.getName(), null);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNManager.class).
                  setCallbacks("setVTNManager", "unsetVTNManager").
                  setRequired(true));
        }
        if (imp.equals(PortHandler.class)) {
            // Export the services.
            c.setInterface(INeutronPortAware.class.getName(), null);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNManager.class).
                  setCallbacks("setVTNManager", "unsetVTNManager").
                  setRequired(true));
        }
        if (imp.equals(SubnetHandler.class)) {
            // Export the services.
            c.setInterface(INeutronSubnetAware.class.getName(), null);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNManager.class).
                  setCallbacks("setVTNManager", "unsetVTNManager").
                  setRequired(true));
        }
        if (imp.equals(SecurityGroupHandler.class)) {
            // Export the services.
            c.setInterface(INeutronSecurityGroupAware.class.getName(), null);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNManager.class).
                  setCallbacks("setVTNManager", "unsetVTNManager").
                  setRequired(true));
        }
        if (imp.equals(OVSDBPluginEventHandler.class)) {
            c.setInterface(OvsdbInventoryListener.class.getName(), null);

            c.add(createServiceDependency().
                  setService(IVTNManager.class).
                  setCallbacks("setVTNManager", "unsetVTNManager").
                  setRequired(true));
            c.add(createServiceDependency().
                  setService(OvsdbConfigurationService.class).
                  setCallbacks("setOVSDBConfigService", "unsetOVSDBConfigService").
                  setRequired(true));

            c.add(createServiceDependency().
                  setService(INeutronPortCRUD.class).
                  setCallbacks("setNeutronPortCRUD", "unsetNeutronPortCRUD").
                  setRequired(true));

            c.add(createServiceDependency().
                  setService(OvsdbConnectionService.class).
                  setCallbacks("setConnectionService", "unsetConnectionService").
                  setRequired(true));
        }

    }
}
