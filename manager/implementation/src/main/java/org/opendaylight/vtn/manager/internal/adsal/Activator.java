/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.adsal;

import java.util.Dictionary;
import java.util.Hashtable;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;

import org.opendaylight.controller.sal.core.ComponentActivatorAbstractBase;

/**
 * OSGi bundle activator for the VTN implementation.
 */
public class Activator extends ComponentActivatorAbstractBase {
    /**
     * The name of the default container.
     */
    private static final String  DEFAULT_CONTAINER = "default";

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
            c.setInterface(new String[]{IVTNGlobal.class.getName()}, null);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(VTNManagerProvider.class).
                  setCallbacks("setVTNProvider", "unsetVTNProvider").
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
            Dictionary<String, Object> props = new Hashtable<>();

            // Export the services.
            c.setInterface(new String[]{IVTNManager.class.getName()}, props);

            if (containerName.equals(DEFAULT_CONTAINER)) {
                // Register dependency to MD-SAL VTN Manager provider.
                c.add(createServiceDependency().
                      setService(VTNManagerProvider.class).
                      setCallbacks("setVTNProvider", "unsetVTNProvider").
                      setRequired(true));
            }
        }
    }
}
