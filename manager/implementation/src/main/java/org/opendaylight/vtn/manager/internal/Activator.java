/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Dictionary;
import java.util.Hashtable;
import java.util.Set;
import java.util.HashSet;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;

import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.clustering.services.
    IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.ICoordinatorChangeAware;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.core.ComponentActivatorAbstractBase;
import org.opendaylight.controller.sal.utils.GlobalConstants;

/**
 * OSGi bundle activator for the VTN implementation.
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
     * Function that is used to communicate to dependency manager the list of
     * known implementations for services that are container independent.
     *
     *
     * @return An array containing all the CLASS objects that will be
     *         instantiated in order to get an fully working implementation
     *         Object
     */
    @Override
    public Object[] getGlobalImplementations() {
        return new Object[]{GlobalResourceManager.class};
    }

    /**
     * Function that is called when configuration of the dependencies is
     * required.
     *
     * @param c
     *            dependency manager Component object, used for configuring the
     *            dependencies exported and imported
     * @param imp
     *            Implementation class that is being configured, needed as long
     *            as the same routine can configure multiple implementations
     */
    @Override
    public void configureGlobalInstance(Component c, Object imp) {
        if (imp.equals(GlobalResourceManager.class)) {
            // Export the services.
            String[] classes = {
                IVTNGlobal.class.getName(),
                IVTNResourceManager.class.getName(),
                ICoordinatorChangeAware.class.getName(),
            };
            c.setInterface(classes, null);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IClusterGlobalServices.class).
                  setCallbacks("setClusterGlobalService",
                               "unsetClusterGlobalService").
                  setRequired(true));
        }
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
        return new Object[]{VTNManagerImpl.class};
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
        if (imp.equals(VTNManagerImpl.class)) {
            Dictionary<String, Object> props =
                new Hashtable<String, Object>();
            Set<String> propSet = new HashSet<String>();
            propSet.add(VTNManagerImpl.CACHE_EVENT);
            propSet.add(VTNManagerImpl.CACHE_MAC);
            props.put("cachenames", propSet);
            props.put("salListenerName", "vtnmanager");

            // Export the services.
            ArrayList<String> list = new ArrayList<String>();
            list.add(IVTNManager.class.getName());
            list.add(ICacheUpdateAware.class.getName());
            list.add(IConfigurationContainerAware.class.getName());
            list.add(IHostFinder.class.getName());
            if (containerName.equals(GlobalConstants.DEFAULT.toString())) {
                // Register dependency to MD-SAL VTN Manager provider.
                c.add(createServiceDependency().
                      setService(VTNManagerProvider.class).
                      setCallbacks("setVTNProvider", "unsetVTNProvider").
                      setRequired(true));
            }

            c.setInterface(list.toArray(new String[list.size()]), props);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNResourceManager.class).
                  setCallbacks("setResourceManager", "unsetResourceManager").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(IVTNManagerAware.class).
                  setCallbacks("addVTNManagerAware", "removeVTNManagerAware").
                  setRequired(false));

            c.add(createContainerServiceDependency(containerName).
                  setService(IClusterContainerServices.class).
                  setCallbacks("setClusterContainerService",
                               "unsetClusterContainerService").
                  setRequired(true));

            // VTN manager can run without any host listener.
            c.add(createContainerServiceDependency(containerName).
                  setService(IfHostListener.class).
                  setCallbacks("addHostListener", "removeHostListener").
                  setRequired(false));
        }
    }
}
