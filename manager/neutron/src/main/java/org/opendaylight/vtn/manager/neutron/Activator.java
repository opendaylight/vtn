/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.ArrayList;
import java.util.Dictionary;
import java.util.Hashtable;

import org.apache.felix.dm.Component;

import org.opendaylight.controller.networkconfig.neutron.INeutronNetworkAware;
import org.opendaylight.controller.sal.core.ComponentActivatorAbstractBase;

import org.opendaylight.vtn.manager.neutron.NetworkHandler;
import org.opendaylight.vtn.manager.IVTNManager;

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
        return new Object[]{NetworkHandler.class};
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
            Dictionary<String, Object> props =
                new Hashtable<String, Object>();
            props.put("salListenerName", "vtnmanager");

            // Export the services.
            ArrayList<String> list = new ArrayList<String>();
            list.add(INeutronNetworkAware.class.getName());
            c.setInterface(list.toArray(new String[list.size()]), props);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNManager.class).
                  setCallbacks("setVTNManager", "unsetVTNManager").
                  setRequired(true));
        }
    }
}
