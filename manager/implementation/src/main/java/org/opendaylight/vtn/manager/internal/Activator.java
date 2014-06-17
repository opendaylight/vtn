/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Dictionary;
import java.util.Hashtable;
import java.util.Set;
import java.util.HashSet;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.IVTNFlowDebugger;
import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;

import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.clustering.services.
    IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterGlobalServices;
import org.opendaylight.controller.clustering.services.ICoordinatorChangeAware;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.forwardingrulesmanager.
    IForwardingRulesManager;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.core.ComponentActivatorAbstractBase;
import org.opendaylight.controller.sal.core.IContainerListener;
import org.opendaylight.controller.sal.flowprogrammer.IFlowProgrammerListener;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.IListenDataPacket;
import org.opendaylight.controller.sal.routing.IListenRoutingUpdates;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.controller.topologymanager.ITopologyManagerAware;

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
            propSet.add(VTNManagerImpl.CACHE_FLOWS);
            props.put("cachenames", propSet);
            props.put("salListenerName", "vtnmanager");

            // Export the services.
            ArrayList<String> list = new ArrayList<String>();
            list.add(IVTNManager.class.getName());
            list.add(ICacheUpdateAware.class.getName());
            list.add(IConfigurationContainerAware.class.getName());
            list.add(IInventoryListener.class.getName());
            list.add(ITopologyManagerAware.class.getName());
            list.add(IListenDataPacket.class.getName());
            list.add(IListenRoutingUpdates.class.getName());
            list.add(IHostFinder.class.getName());
            list.add(IFlowProgrammerListener.class.getName());
            if (containerName.equals(GlobalConstants.DEFAULT.toString())) {
                list.add(IContainerListener.class.getName());
            }

            // Export IVTNFlowDebugger only if "vtn.debug" system property
            // is defined as "true".
            String debug = System.getProperty("vtn.debug");
            if (debug != null && Boolean.parseBoolean(debug)) {
                list.add(IVTNFlowDebugger.class.getName());
            }

            c.setInterface(list.toArray(new String[list.size()]), props);

            // Create service dependencies.
            c.add(createServiceDependency().
                  setService(IVTNResourceManager.class).
                  setCallbacks("setResourceManager", "unsetResourceManager").
                  setRequired(true));

            c.add(createServiceDependency().
                  setService(IConnectionManager.class).
                  setCallbacks("setConnectionManager",
                               "unsetConnectionManager").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(IVTNManagerAware.class).
                  setCallbacks("addVTNManagerAware", "removeVTNManagerAware").
                  setRequired(false));

            c.add(createContainerServiceDependency(containerName).
                  setService(IVTNModeListener.class).
                  setCallbacks("addVTNModeListener", "removeVTNModeListener").
                  setRequired(false));

            c.add(createContainerServiceDependency(containerName).
                  setService(IClusterContainerServices.class).
                  setCallbacks("setClusterContainerService",
                               "unsetClusterContainerService").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(ISwitchManager.class).
                  setCallbacks("setSwitchManager", "unsetSwitchManager").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(ITopologyManager.class).
                  setCallbacks("setTopologyManager", "unsetTopologyManager").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(IForwardingRulesManager.class).
                  setCallbacks("setForwardingRuleManager",
                               "unsetForwardingRuleManager").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(IRouting.class).
                  setCallbacks("setRouting", "unsetRouting").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(IDataPacketService.class).
                  setCallbacks("setDataPacketService",
                               "unsetDataPacketService").
                  setRequired(true));

            c.add(createContainerServiceDependency(containerName).
                  setService(IStatisticsManager.class).
                  setCallbacks("setStatisticsManager",
                               "unsetStatisticsManager").
                  setRequired(true));

            // VTN manager can run without any host listener.
            c.add(createContainerServiceDependency(containerName).
                  setService(IfHostListener.class).
                  setCallbacks("addHostListener", "removeHostListener").
                  setRequired(false));

            // Although VTN manager does not use IfIptoHost service,
            // ArpHandler emulator does.
            c.add(createContainerServiceDependency(containerName).
                  setService(IfIptoHost.class).
                  setCallbacks("setHostTracker", "unsetHostTracker").
                  setRequired(true));
        }
    }

    /**
     * Invoked when a container is being destroyed.
     *
     * @param containerName  The name of the container.
     */
    @Override
    public void containerDestroy(String containerName) {
        VTNManagerImpl mgr = (VTNManagerImpl)ServiceHelper.
            getInstance(IVTNManager.class, containerName, this);
        if (mgr != null) {
            // Let the VTN manager know that this container is being destroyed.
            mgr.containerDestroy();
        }

        super.containerDestroy(containerName);

        // Remove configuration files.
        ContainerConfig cfg = new ContainerConfig(containerName);
        cfg.cleanUp();

        // Clean up resources registered to the resource manager service.
        IVTNResourceManager resMgr = (IVTNResourceManager)ServiceHelper.
            getGlobalInstance(IVTNResourceManager.class, this);
        if (resMgr != null) {
            resMgr.cleanUp(containerName);
        }
    }
}
