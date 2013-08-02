/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.net.InetAddress;
import java.net.Inet4Address;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Dictionary;
import java.util.EnumSet;
import java.util.List;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Map;
import java.util.TreeMap;
import java.util.Hashtable;
import java.util.Set;
import java.util.HashSet;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.clustering.services.IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.containermanager.IContainerManager;
import org.opendaylight.controller.forwardingrulesmanager.IForwardingRulesManager;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.core.ContainerFlow;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.IContainerListener;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IListenDataPacket;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.routing.IListenRoutingUpdates;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.topology.TopoEdgeUpdate;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.IObjectReader;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.ObjectReader;
import org.opendaylight.controller.sal.utils.ObjectWriter;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.controller.topologymanager.ITopologyManagerAware;

/**
 * Implementation of VTN Manager service.
 */
public class VTNManagerImpl implements IVTNManager, IObjectReader,
    ICacheUpdateAware<String, Long>, IConfigurationContainerAware,
    IInventoryListener, ITopologyManagerAware, IContainerListener,
    IListenDataPacket, IListenRoutingUpdates, IHostFinder {
    /**
     * Logger instance.
     */
    private final static Logger  LOG =
        LoggerFactory.getLogger(VTNManagerImpl.class);

    /**
     * Maximum length of the resource name.
     */
    private final static int RESOURCE_NAME_MAXLEN = 31;

    /**
     * Regular expression that matches valid resource name.
     */
    private final static Pattern RESOURCE_NAME_REGEX =
        Pattern.compile("^\\p{Alnum}[\\p{Alnum}_]*$");

    /**
     * Maximum value of VLAN ID.
     */
    final static short VLAN_ID_MAX = 4095;

    /**
     * VLAN ID which represents untagged frame.
     */
    final static short VLAN_ID_UNTAGGED = 0;

    /**
     * Cluster cache name associated with {@link #tenantDB}.
     */
    final static String  CACHE_TENANT = "vtn.tenant";

    /**
     * Cluster cache name associated with {@link #stateDB}.
     */
    final static String  CACHE_STATE = "vtn.state";

    /**
     * Cluster cache name associated with {@link #configSaveEvent}.
     */
    final static String  CACHE_SAVE_EVENT = "vtn.configSaveEvent";

    /**
     * {@link #configSaveEvent} key that indicates save request for all tenant
     * configuration.
     */
    private final static String  CHSAVE_ALL = "<all>";

    /**
     * Keeps virtual tenant configurations in a container.
     */
    private ConcurrentMap<String, VTenantImpl>  tenantDB;

    /**
     * Keeps runtime state of virtual nodes.
     */
    private ConcurrentMap<VTenantPath, Object>  stateDB;

    /**
     * A cluster cache used to raise an config save event to the cluster nodes.
     */
    private ConcurrentMap<String, Long>  configSaveEvent;

    /**
     * Cluster container service instance.
     */
    private IClusterContainerServices  clusterService;

    /**
     * Switch manager service instance.
     */
    private ISwitchManager  switchManager;

    /**
     * Topology manager service instance.
     */
    private ITopologyManager  topologyManager;

    /**
     * Forwarding rule manager service instance.
     */
    private IForwardingRulesManager  fwRuleManager;

    /**
     * Routing service instance.
     */
    private IRouting  routing;

    /**
     * Data packet service instance.
     */
    private IDataPacketService  dataPacketService;

    /**
     * Host tracker service instance.
     */
    private IfIptoHost  hostTracker;

    /**
     * Host listeners.
     */
    private final CopyOnWriteArrayList<IfHostListener>  hostListeners =
        new CopyOnWriteArrayList<IfHostListener>();

    /**
     * Global resource manager service.
     */
    private IVTNResourceManager  resourceManager;

    /**
     * VTN manager listeners.
     */
    private final CopyOnWriteArrayList<IVTNManagerAware>  vtnManagerAware =
        new CopyOnWriteArrayList<IVTNManagerAware>();

    /**
     * VTN mode listeners.
     */
    private final CopyOnWriteArrayList<IVTNModeListener>  vtnModeListeners =
        new CopyOnWriteArrayList<IVTNModeListener>();

    /**
     * Container name associated with this service.
     */
    private String  containerName;

    /**
     * Read write lock to synchronize per-container resources.
     */
    private final ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

    /**
     * Long value to be set to {@link #configSaveEvent}.
     */
    private final AtomicLong  saveEventValue = new AtomicLong();

    /**
     * Single-threaded job queue runner.
     */
    private JobQueueThread  jobQueueThread;

    /**
     * The name of the file to keep virtual tenant names.
     */
    private String  tenantListFileName;

    /**
     * ARP handler emulator.
     */
    private ArpHandler  arpHandler;

    /**
     * True if non-default container exists.
     * This variable always keeps {@code false} if this service is associated
     * with a non-default container.
     */
    private boolean  inContainerMode;

    /**
     * True if the container is being destroyed.
     */
    private boolean  destroying;

    /**
     * Static configuration.
     */
    private VTNConfig  vtnConfig;

    /**
     * Keep nodes which are not in service yet.
     *
     * <p>
     *   If a node is contained in this map, any packet from the node is
     *   ignored, and no packet is sent to the node.
     * </p>
     */
    private ConcurrentMap<Node, TimerTask>  disabledNodes =
        new ConcurrentHashMap<Node, TimerTask>();

    /**
     * MAC address tables associated with vBridges.
     */
    private final Hashtable<VBridgePath, MacAddressTable> macTableMap =
        new Hashtable<VBridgePath, MacAddressTable>();

    /**
     * A thread which executes queued jobs.
     */
    private class JobQueueThread extends Thread {
        /**
         * Job queue.
         */
        private final LinkedList<Runnable> jobQueue =
            new LinkedList<Runnable>();

        /**
         * Determine whether the job queue is active or not.
         */
        private boolean  active = true;

        /**
         * Construct a job queue thread.
         *
         * @param name  The name of the thread.
         */
        private JobQueueThread(String name) {
            super(name);
        }

        /**
         * Dequeue a job.
         *
         * @return  A dequeued job. {@link null} is returned if the queue
         *          is down.
         */
        private synchronized Runnable getJob() {
            while (jobQueue.size() == 0) {
                if (!active) {
                    return null;
                }
                try {
                    wait();
                } catch (InterruptedException e) {
                }
            }

            return jobQueue.remove();
        }

        /**
         * Post a job to the job queue.
         *
         * @param r  A runnable to be run on a job queue runner thread.
         */
        private synchronized void post(Runnable r) {
            if (active) {
                jobQueue.add(r);
                notify();
            }
        }

        /**
         * Shut down the job queue.
         */
        private synchronized void shutdown() {
            jobQueue.clear();
            active = false;
            notify();
        }

        /**
         * Main routine of a job queue thread.
         */
        @Override
        public void run() {
            LOG.info("{}: Start job queue runner", containerName);
            for (Runnable r = getJob(); r != null; r = getJob()) {
                try {
                    r.run();
                } catch (Exception e) {
                    LOG.error(containerName +
                              ": Exception occurred on a job thread.", e);
                }
            }
            LOG.info("{}: Shutdown job queue runner", containerName);
        }
    }

    /**
     * Function called by the dependency manager when all the required
     * dependencies are satisfied.
     *
     * @param c  Dependency manager component.
     */
    void init(Component c) {
        // Determine container name.
        Dictionary<?, ?> props = c.getServiceProperties();
        String cname = null;
        if (props != null) {
            cname = (String)props.get("containerName");
        }
        if (cname == null) {
            LOG.error("Container name is not specified.");
            return;
        }

        containerName = cname;

        // Load static configuration.
        String root = GlobalConstants.STARTUPHOME.toString();
        vtnConfig = new VTNConfig(root, cname);

        if (cname.equals(GlobalConstants.DEFAULT.toString())) {
            IContainerManager ctMgr = (IContainerManager)ServiceHelper.
                getGlobalInstance(IContainerManager.class, this);
            inContainerMode =
                (ctMgr != null && ctMgr.hasNonDefaultContainer());
        } else {
            inContainerMode = false;
        }

        createCaches();
        saveEventValue.set(System.currentTimeMillis());
        tenantListFileName = root + "vtn-names.conf";

        // Start job queue thread.
        jobQueueThread = new JobQueueThread("VTN Job Thread: " + cname);
        jobQueueThread.start();

        // Load saved configurations.
        loadConfig();

        if (inContainerMode || tenantDB == null || tenantDB.isEmpty()) {
            // Start ARP handler emulator.
            arpHandler = new ArpHandler(this);
        }
    }

    /**
     * Function called by the dependency manager before the services exported
     * by the component are unregistered, this will be followed by a
     * "destroy()" calls.
     */
    void stop() {
        synchronized (this) {
            // Stop timeout timer in the ARP handler emulator.
            if (arpHandler != null) {
                arpHandler.destroy();
            }
        }
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     */
    void destroy() {
        vtnManagerAware.clear();
        if (jobQueueThread != null) {
            jobQueueThread.shutdown();
            try {
                jobQueueThread.join();
            } catch (InterruptedException e) {
            }
            jobQueueThread = null;
        }

        // Remove all MAC address tables.
        IVTNResourceManager resMgr = resourceManager;
        ArrayList<MacAddressTable> tables;
        synchronized (macTableMap) {
            tables = new ArrayList<MacAddressTable>(macTableMap.size());
            for (Iterator<MacAddressTable> it =
                     macTableMap.values().iterator(); it.hasNext();) {
                MacAddressTable table = it.next();
                tables.add(table);
                it.remove();
            }
        }

        for (MacAddressTable table: tables) {
            table.destroy(null);
        }

        for (Iterator<TimerTask> it = disabledNodes.values().iterator();
             it.hasNext();) {
            TimerTask task = it.next();
            task.cancel();
            it.remove();
        }

        Timer timer = resMgr.getTimer();
        timer.purge();

        if (destroying) {
            // Clear all caches.
            LOG.debug("{}: Clear VTN caches.", containerName);
            if (tenantDB != null) {
                tenantDB.clear();
            }
            if (stateDB != null) {
                stateDB.clear();
            }
            destroyCaches();
        }
    }

    /**
     * Create cluster caches for VTN.
     */
    private void createCaches() {
        IClusterContainerServices cluster = clusterService;
        if (cluster == null) {
            LOG.error("{}: Cluster service is not yet registered.",
                      containerName);
            return;
        }

        Set<IClusterServices.cacheMode> cmode =
            EnumSet.of(IClusterServices.cacheMode.NON_TRANSACTIONAL);
        createCache(cluster, CACHE_TENANT, cmode);
        createCache(cluster, CACHE_STATE, cmode);
        createCache(cluster, CACHE_SAVE_EVENT, cmode);

        tenantDB = (ConcurrentMap<String, VTenantImpl>)
            getCache(cluster, CACHE_TENANT);
        stateDB = (ConcurrentMap<VTenantPath, Object>)
            getCache(cluster, CACHE_STATE);
        configSaveEvent = (ConcurrentMap<String, Long>)
            getCache(cluster, CACHE_SAVE_EVENT);

        LOG.debug("{}: Created VTN caches.", containerName);
    }

    /**
     * Create a named cluster cache.
     *
     * @param cs         Cluster container service.
     * @param cacheName  The name of cluster cache.
     * @param cmode      Set of cache modes.
     */
    private void createCache(IClusterContainerServices cs, String cacheName,
                             Set<IClusterServices.cacheMode> cmode) {
        try {
            cs.createCache(cacheName, cmode);
        } catch (CacheExistException e) {
            LOG.warn("{}: {}: Cache already exists", containerName, cacheName);
        } catch (CacheConfigException e) {
            LOG.error("{}: {}: Invalid cache configuration: {}",
                      containerName, cacheName, cmode);
        } catch (Exception e) {
            if (LOG.isErrorEnabled()) {
                String msg = containerName + ": " + cacheName +
                    ": Failed to create cache";
                LOG.error(msg, e);
            }
        }
    }

    /**
     * Retrieve cluster cache associated with the given name.
     *
     * @param cs         Cluster container service.
     * @param cacheName  The name of cluster cache.
     * @return           Cluster cache, or {@code null} on failure.
     */
    private ConcurrentMap<?, ?> getCache(IClusterContainerServices cs,
                                         String cacheName) {
        ConcurrentMap<?, ?> cache = cs.getCache(cacheName);
        if (cache == null) {
            LOG.error("{}: {}: Cache not found", containerName, cacheName);
        }

        return cache;
    }

    /**
     * Destroy cluster caches.
     */
    private void destroyCaches() {
        IClusterContainerServices cluster = clusterService;
        if (cluster != null) {
            cluster.destroyCache(CACHE_TENANT);
            cluster.destroyCache(CACHE_STATE);
            cluster.destroyCache(CACHE_SAVE_EVENT);

            LOG.debug("{}: Destroyed VTN caches.", containerName);
        }
    }

    /**
     * Called when the container is being destroyed.
     */
    void containerDestroy() {
        destroying = true;
    }

    /**
     * Return the name of the container associated with this service.
     *
     * @return The name of the container.
     */
    public String getContainerName() {
        return containerName;
    }

    /**
     * Invoked when a cluster container service is registered.
     *
     * @param service  Cluster container service.
     */
    void setClusterContainerService(IClusterContainerServices service) {
        LOG.debug("{}: Set cluster service: {}", containerName, service);
        clusterService = service;
    }

    /**
     * Invoked when a cluster container service is unregistered.
     *
     * @param service  Cluster container service.
     */
    void unsetClusterContainerService(IClusterContainerServices service) {
        if (clusterService == service) {
            LOG.debug("{}: Unset cluster service: {}", containerName, service);
            clusterService = null;
        }
    }

    /**
     * Invoked when a switch manager service is registered.
     *
     * @param service  Switch manager service.
     */
    void setSwitchManager(ISwitchManager service) {
        LOG.debug("{}: Set switch manager: {}", containerName, service);
        switchManager = service;
    }

    /**
     * Invoked when a switch manager service is unregistered.
     *
     * @param service  Switch manager service.
     */
    void unsetSwitchManager(ISwitchManager service) {
        if (switchManager == service) {
            LOG.debug("{}: Unset switch manager: {}", containerName, service);
            switchManager = null;
        }
    }

    /**
     * Return switch manager service instance.
     *
     * @return  Switch manager service.
     */
    public ISwitchManager getSwitchManager() {
        return switchManager;
    }

    /**
     * Invoked when a topology manager service is registered.
     *
     * @param service  Topology manager service.
     */
    void setTopologyManager(ITopologyManager service) {
        LOG.debug("{}: Set topology manager: {}", containerName, service);
        topologyManager = service;
    }

    /**
     * Invoked when a topology manager service is unregistered.
     *
     * @param service  Topology manager service.
     */
    void unsetTopologyManager(ITopologyManager service) {
        if (topologyManager == service) {
            LOG.debug("{}: Unset topology manager: {}",
                      containerName, service);
            topologyManager = null;
        }
    }

    /**
     * Return topology manager service instance.
     *
     * @return  Topology manager service.
     */
    public ITopologyManager getTopologyManager() {
        return topologyManager;
    }

    /**
     * Invoked when a forwarding rule manager service is registered.
     *
     * @param service  Forwarding rule manager service.
     */
    void setForwardingRuleManager(IForwardingRulesManager service) {
        LOG.debug("{}: Set forwarding rule manager: {}",
                  containerName, service);
        fwRuleManager = service;
    }

    /**
     * Invoked when a forwarding rule manager service is unregistered.
     *
     * @param service  Forwarding rule manager service.
     */
    void unsetForwardingRuleManager(IForwardingRulesManager service) {
        if (fwRuleManager == service) {
            LOG.debug("{}: Unset forwarding rule manager: {}",
                      containerName, service);
            fwRuleManager = null;
        }
    }

    /**
     * Return forwarding rule manager service instance.
     *
     * @return  Forwarding rule manager service.
     */
    public IForwardingRulesManager getForwardingRuleManager() {
        return fwRuleManager;
    }

    /**
     * Invoked when a routing service is registered.
     *
     * @param service  Routing service.
     */
    void setRouting(IRouting service) {
        LOG.debug("{}: Set routing service: {}", containerName, service);
        routing = service;
    }

    /**
     * Invoked when a routing service is unregistered.
     *
     * @param service  Routing service.
     */
    void unsetRouting(IRouting service) {
        if (routing == service) {
            LOG.debug("{}: Unset routing service: {}", containerName, service);
            routing = null;
        }
    }

    /**
     * Return routing service instance.
     *
     * @return  Routing service.
     */
    public IRouting getRouting() {
        return routing;
    }

    /**
     * Invoked when a data packet service is registered.
     *
     * @param service  Data packet service.
     */
    void setDataPacketService(IDataPacketService service) {
        LOG.debug("{}: Set data packet service: {}", containerName, service);
        dataPacketService = service;
    }

    /**
     * Invoked when a data packet service is unregistered.
     *
     * @param service  Data packet service.
     */
    void unsetDataPacketService(IDataPacketService service) {
        if (dataPacketService == service) {
            LOG.debug("{}: Unset data packet service: {}",
                      containerName, service);
            dataPacketService = null;
        }
    }

    /**
     * Return data packet service instance.
     *
     * @return  Data packet service.
     */
    public IDataPacketService getDataPacketService() {
        return dataPacketService;
    }

    /**
     * Invoked when a host tracker service is registered.
     *
     * @param service  Host tracker service.
     */
    void setHostTracker(IfIptoHost service) {
        LOG.debug("{}: Set host tracker service: {}", containerName, service);
        hostTracker = service;
    }

    /**
     * Invoked when a host tracker service is unregistered.
     *
     * @param service  Host tracker service.
     */
    void unsetHostTracker(IfIptoHost service) {
        if (hostTracker == service) {
            LOG.debug("{}: Unset host tracker service: {}",
                      containerName, service);
            hostTracker = null;
        }
    }

    /**
     * Return host tracker service instance.
     *
     * @return  Host tracker service.
     */
    public IfIptoHost getHostTracker() {
        return hostTracker;
    }

    /**
     * Invoked when a host listener is registered.
     *
     * @param service  Host listener service.
     */
    void addHostListener(IfHostListener service) {
        if (hostListeners.addIfAbsent(service)) {
            LOG.debug("Add host listener: {}", service);
        }
    }

    /**
     * Invoked when a host listener is unregistered.
     *
     * @param service  Host listener service.
     */
    void removeHostListener(IfHostListener service) {
        if (hostListeners.remove(service)) {
            LOG.debug("Remove host listener: {}", service);
        }
    }

    /**
     * Invoked when a VTN resource manager service is registered.
     *
     * @param service  VTN resource manager service.
     */
    void setResourceManager(IVTNResourceManager service) {
        LOG.debug("Set VTN resource manager: {}", service);
        resourceManager = service;
    }

    /**
     * Invoked when a VTN resource manager service is unregistered.
     *
     * @param service  VTN resource manager service.
     */
    void unsetResourceManager(IVTNResourceManager service) {
        if (resourceManager == service) {
            LOG.debug("Unset VTN resource manager: {}", service);
            resourceManager = service;
        }
    }

    /**
     * Return VTN global resource manager instance.
     *
     * @return  VTN global resource manager service.
     */
    public IVTNResourceManager getResourceManager() {
        return resourceManager;
    }

    /**
     * Invoked when a VTN manager listener service is registered.
     *
     * @param service  VTN manager listener service.
     */
    void addVTNManagerAware(IVTNManagerAware service) {
        if (vtnManagerAware.addIfAbsent(service)) {
            LOG.debug("Add VTN manager listener: {}", service);
            notifyConfiguration(service);
        }
    }

    /**
     * Invoked when a VTN manager listener service is unregistered.
     *
     * @param service  VTN manager listener service.
     */
    void removeVTNManagerAware(IVTNManagerAware service) {
        if (vtnManagerAware.remove(service)) {
            LOG.debug("Remove VTN manager listener: {}", service);
        }
    }

    /**
     * Invoked when a VTN mode listener service is registered.
     *
     * @param listener  VTN mode listener service.
     */
    void addVTNModeListener(IVTNModeListener listener) {
        if (vtnModeListeners.addIfAbsent(listener)) {
            LOG.debug("Add VTN mode listener: {}", listener);

            synchronized (this) {
                notifyChange(listener, arpHandler == null);
            }
        }
    }

    /**
     * Invoked when a VTN mode listener service is unregistered.
     *
     * @param service  VTN mode listener service.
     */
    void removeVTNModeListener(IVTNModeListener service) {
        if (vtnModeListeners.remove(service)) {
            LOG.debug("Remove VTN mode listener: {}", service);
        }
    }

    /**
     * Return the virtual node state DB.
     *
     * @return  Virtual node state DB.
     */
    public ConcurrentMap<VTenantPath, Object> getStateDB() {
        return stateDB;
    }

    /**
     * Add a MAC address table for a virtual L2 bridge.
     *
     * @param path  Path to the virtual L2 bridge.
     * @param age   Interval in milliseconds between aging tasks.
     */
    public void addMacAddressTable(VBridgePath path, int age) {
        StringBuilder builder = new StringBuilder(containerName);
        builder.append(':').append(path.toString());
        String name = builder.toString();

        MacAddressTable table = new MacAddressTable(this, name, age);
        macTableMap.put(path, table);
    }

    /**
     * Remove a MAC address table associated with the given virtual L2 bridge.
     *
     * @param path   Path to the virtual L2 bridge.
     * @param purge  If {@code true} is passed, purge all canceled aging
     *               tasks in the global timer task queue.
     */
    public void removeMacAddressTable(VBridgePath path, boolean purge) {
        MacAddressTable table = macTableMap.remove(path);
        if (table != null) {
            table.destroy((purge) ? this : null);
        }
    }

    /**
     * Return a MAC address table associated with the given virtual L2 bridge.
     *
     * @param path   Path to the virtual L2 bridge.
     * @return  A MAC address table associated with the given virtual L2
     *          bridge.
     */
    public MacAddressTable getMacAddressTable(VBridgePath path) {
        return macTableMap.get(path);
    }

    /**
     * Determine whether the given node connector is associated with the
     * physical switch port at the edge of the SDN network.
     *
     * @param nc  A node connector.
     * @return  {@code true} is returned only if the given node connector
     *          is associated with the physical switch port at the edge of
     *          the SDN network.
     */
    public boolean isEdgePort(NodeConnector nc) {
        return (!switchManager.isSpecial(nc) &&
                !topologyManager.isInternal(nc));
    }

    /**
     * Transmit an ethernet frame to the specified node connector.
     *
     * @param nc     A node connector.
     * @param ether  An ethernet frame.
     */
    public void transmit(NodeConnector nc, Ethernet ether) {
        if (disabledNodes.containsKey(nc.getNode())) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Don't send packet to disabled node: {}",
                          containerName, nc.getNode());
            }
            return;
        }

        IDataPacketService pktSrv = dataPacketService;

        RawPacket pkt = pktSrv.encodeDataPacket(ether);
        pkt.setOutgoingNodeConnector(nc);
        pktSrv.transmitDataPacket(pkt);
    }

    /**
     * Create an ethernet frame which contains an ARP request message.
     *
     * <p>
     *   Controller's MAC address is used as source MAC adddress, and zero
     *   is used as sender protocol address.
     * </p>
     *
     * @param dst   Destination MAC address.
     * @param addr  Target IP address.
     * @return  An ethernet frame. {@code null} is returned if {@code addr}
     *          is invalid.
     */
    public Ethernet createArpRequest(byte[] dst, InetAddress addr) {
        return createArpRequest(dst, addr, (short)0);
    }

    /**
     * Create an ethernet frame which contains an ARP request message.
     *
     * <p>
     *   Controller's MAC address is used as source MAC adddress, and zero
     *   is used as sender protocol address.
     * </p>
     *
     * @param dst   Destination MAC address.
     * @param addr  Target IP address.
     * @param vlan  VLAN ID. Zero means VLAN tag should not be added.
     * @return  An ethernet frame. {@code null} is returned if {@code addr}
     *          is invalid.
     */
    public Ethernet createArpRequest(byte[] dst, InetAddress addr,
                                     short vlan) {
        // IP address must be an IPv4 address.
        if (!(addr instanceof Inet4Address)) {
            return null;
        }

        // Set controller's MAC address to source MAC address.
        byte[] src = switchManager.getControllerMAC();
        byte[] target = addr.getAddress();

        // Create an ARP request message.
        // Set zero to sender protocol address.
        byte[] sender = new byte[]{0, 0, 0, 0};
        return createArp(ARP.REQUEST, src, dst, sender, target, vlan);
    }

    /**
     * Create an ethernet frame which contains an ARP message.
     *
     * @param op      Operation code defined by ARP.
     * @param src     Source MAC address.
     * @param dst     Destination MAC address.
     * @param sender  Sender IP address.
     * @param target  Target IP address.
     * @param vlan    VLAN ID. Zero means VLAN tag should not be added.
     * @return  An ethernet frame.
     */
    public Ethernet createArp(short op, byte[] src, byte[] dst, byte[] sender,
                              byte[] target, short vlan) {
        assert src.length == EthernetAddress.SIZE &&
            dst.length == EthernetAddress.SIZE &&
            sender.length == 4 && target.length == 4;

        byte[] tha = (NetUtils.isBroadcastMACAddr(dst))
            ? new byte[]{0, 0, 0, 0, 0, 0} : dst;

        // Create an ARP request message.
        ARP arp = new ARP();
        arp.setHardwareType(ARP.HW_TYPE_ETHERNET).
            setProtocolType(EtherTypes.IPv4.shortValue()).
            setHardwareAddressLength((byte)EthernetAddress.SIZE).
            setProtocolAddressLength((byte)target.length).
            setOpCode(op).
            setSenderHardwareAddress(src).setSenderProtocolAddress(sender).
            setTargetHardwareAddress(tha).setTargetProtocolAddress(target);

        short ethType = EtherTypes.ARP.shortValue();
        Packet payload = arp;
        if (vlan != 0) {
            // Add VLAN tag.
            IEEE8021Q tag = new IEEE8021Q();
            tag.setCfi((byte)0).setPcp((byte)0).setVid(vlan).
                setEtherType(ethType).setPayload(arp);
            ethType = EtherTypes.VLANTAGGED.shortValue();
            payload = tag;
        }

        // Create an ethernet frame.
        Ethernet ether = new Ethernet();
        ether.setSourceMACAddress(src).setDestinationMACAddress(dst).
            setEtherType(ethType).setPayload(payload);

        return ether;
    }

    /**
     * Raise a configuration save event to the cluster nodes.
     *
     * @param tenantName  The name of the tenant.
     */
    private void raiseConfigSaveEvent(String tenantName) {
        ConcurrentMap<String, Long> event = configSaveEvent;
        if (event != null) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Send config save event",
                          containerName, tenantName);
            }
            configSaveEvent.put(tenantName, saveEventValue.getAndIncrement());
        }
    }

    /**
     * Save a set of virtual tenant names.
     *
     * @return  "Success" or failure reason.
     */
    private Status saveTenantNames() {
        if (tenantDB == null) {
            return cacheNotInitialized();
        }

        HashSet<String> nameSet;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            nameSet = new HashSet<String>(tenantDB.keySet());
        } finally {
            rdlock.unlock();
        }

        return saveTenantNames(nameSet);
    }

    /**
     * Save a set of virtual tenant names.
     *
     * @param nameSet  A set of virtual tenant names.
     * @return  "Success" or failure reason.
     */
    private synchronized Status saveTenantNames(Set<String> nameSet) {
        ObjectWriter wtr = new ObjectWriter();
        Status status = wtr.write(nameSet, tenantListFileName);
        if (status.isSuccess()) {
            return status;
        }

        String msg = "Failed to save tenant names";
        LOG.error("{}: {}", msg, status);
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Save virtual tenant configuration.
     *
     * @param vtn  The tenant instance.
     * @return  "Success" or failure reason.
     */
    private Status saveTenantConfig(VTenantImpl vtn) {
        raiseConfigSaveEvent(vtn.getName());
        return vtn.saveConfig(null);
    }

    /**
     * Load all virtual tenant configurations.
     */
    private void loadConfig() {
        if (tenantDB == null) {
            return;
        }

        // Read tenant names.
        ObjectReader rdr = new ObjectReader();
        HashSet<String> nameSet = (HashSet<String>)
            rdr.read(this, tenantListFileName);
        if (nameSet != null) {
            for (String name: nameSet) {
                loadTenantConfig(name);
            }
        }
    }

    /**
     * Load configuration for the specified virtual tenant.
     *
     * @param tenantName  The name of the tenant.
     */
    private void loadTenantConfig(String tenantName) {
        String path = VTenantImpl.getConfigFilePath(containerName, tenantName);

        // Read tenant configuration.
        ObjectReader rdr = new ObjectReader();
        VTenantImpl vtn = (VTenantImpl)rdr.read(this, path);
        if (vtn != null) {
            vtn.resume(this);
            tenantDB.put(tenantName, vtn);
            LOG.info("{}: Load tenant: {}", containerName, vtn.getVTenant());
        }
    }

    /**
     * Return a failure status if the controller is in container mode.
     *
     * @return  A failure status if in container mode.
     *          Otherwise {@code null}.
     */
    private synchronized Status checkContainerMode() {
        if (inContainerMode) {
            return new Status(StatusCode.NOTACCEPTABLE,
                              "VTN is disabled by container mode");
        }

        return null;
    }

    /**
     * Remove all configuration files associated with the container.
     *
     * @param containerName  The name of the container.
     */
    static void cleanUpConfigFile(String containerName) {
        StringBuilder builder = new StringBuilder("vtn-");
        builder.append(containerName).append('-');

        final String prefix = builder.toString();
        final String suffix = ".conf";
        final int minlen = prefix.length() + suffix.length();
        FilenameFilter filter = new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                return (name.startsWith(prefix) && name.endsWith(suffix) &&
                        name.length() > minlen);
            }
        };

        File root = new File(GlobalConstants.STARTUPHOME.toString());
        File[] files = root.listFiles(filter);
        if (files != null) {
            for (File f: files) {
                LOG.debug("Delete configuration file: {}", f);
                f.delete();
            }
        }
    }

    /**
     * Check the specified resource name.
     *
     * @param desc  Brief description of the resource.
     * @param name  The name of the resource.
     * @throws VTNException  The specified name is invalid.
     */
    public static void checkName(String desc, String name)
        throws VTNException {
        if (name == null) {
            Status status = argumentIsNull(desc + " name");
            throw new VTNException(status);
        }

        if (name.isEmpty()) {
            Status status = new Status(StatusCode.BADREQUEST,
                                       desc + " name cannot be empty");
            throw new VTNException(status);
        }

        int len = name.length();
        if (len > RESOURCE_NAME_MAXLEN) {
            Status status = new Status(StatusCode.BADREQUEST,
                                       desc + " name is too long");
            throw new VTNException(status);
        }

        Matcher m = RESOURCE_NAME_REGEX.matcher(name);
        if (!m.matches()) {
            Status status = new Status(StatusCode.BADREQUEST, desc +
                                       " name contains invalid character");
            throw new VTNException(status);
        }
    }

    /**
     * Check the specified VLAN ID.
     *
     * @param vlan  VLAN ID.
     * @throws VTNException  The specified VLAN ID is invalid.
     */
    public static void checkVlan(short vlan) throws VTNException {
        if (vlan < 0 || vlan > VLAN_ID_MAX) {
            String msg = "Invalid VLAN ID: " + vlan;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Check the specified node.
     *
     * @param node  Node to be tested.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNode(Node node) throws VTNException {
        if (node == null) {
            Status status = argumentIsNull("Node");
            throw new VTNException(status);
        }

        String type = node.getType();
        Object id = node.getID();
        if (type == null || id == null) {
            String msg = "Broken node is specified";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        // Currently only OpenFlow node is supported.
        if (!type.equals(Node.NodeIDType.OPENFLOW)) {
            String msg = "Unsupported node type";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Check the specified node connector.
     *
     * @param nc  Node connector to be tested.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNodeConnector(NodeConnector nc)
        throws VTNException {
        if (nc == null) {
            Status status = argumentIsNull("Node connector");
            throw new VTNException(status);
        }

        String type = nc.getType();
        Object id = nc.getID();
        if (type == null || id == null) {
            String msg = "Broken node connector is specified";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        // Currently only OpenFlow node is supported.
        if (!type.equals(NodeConnector.NodeConnectorIDType.OPENFLOW)) {
            String msg = "Unsupported node connector type";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        checkNode(nc.getNode());
    }

    /**
     * Return a failure status which represents the VTN cluster cache is not
     * yet initialized.
     *
     * @return  A failure reason.
     */
    private Status cacheNotInitialized() {
        String msg = "VTN cache is not yet initialized";
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Return a failure status that indicates the specified tenant does not
     * exist.
     *
     * @param tenantName  The name of the tenant.
     * @return  A failure status.
     */
    private Status tenantNotFound(String tenantName) {
        String msg = tenantName + ": Tenant does not exist";
        return new Status(StatusCode.NOTFOUND, msg);
    }

    /**
     * Return a failure status which represents a {@code null} is specified
     * unexpectedly.
     *
     * @param desc  Brief description of the argument.
     * @return  A failure reason.
     */
    public static Status argumentIsNull(String desc) {
        String msg = desc + " cannot be null";
        return new Status(StatusCode.BADREQUEST, msg);
    }

    /**
     * Determine whether the given MAC address is a unicast address or not.
     *
     * @param addr  A MAC address.
     * @return  {@code true} is returned only if the given MAC address is a
     *          unicast address.
     */
    public static boolean isUnicast(byte[] addr) {
        return ((addr[0] & 0x1) == 0);
    }

    /**
     * Return the virtual tenant instance associated with the given name.
     *
     * <p>
     *   This method must be called with the VTN Manager lock.
     * </p>
     *
     * @param path   Path to the virtual tenant.
     * @return  Virtual tenant instance is returned.
     * @throws VTNException  An error occurred.
     */
    private VTenantImpl getTenantImpl(VTenantPath path)
        throws VTNException {
        if (tenantDB == null) {
            throw new VTNException(cacheNotInitialized());
        }
        if (path == null) {
            throw new VTNException(argumentIsNull("Path"));
        }

        String tenantName = path.getTenantName();
        if (tenantName == null) {
            throw new VTNException(argumentIsNull("Tenant name"));
        }

        VTenantImpl vtn = tenantDB.get(tenantName);
        if (vtn == null) {
            Status status = tenantNotFound(tenantName);
            throw new VTNException(status);
        }

        return vtn;
    }

    /**
     * Run the specified job on a job queue.
     *
     * @param r  A runnable to be run on a job queue runner thread.
     */
    void postJob(Runnable r) {
        jobQueueThread.post(r);
    }

    /**
     * Notify the VTN mode change.
     *
     * @param active  {@code true} if the VTN has been activated.
     *                {@code false} if the VTN has been inactivated.
     */
    public void notifyChange(final boolean active) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                if (LOG.isInfoEnabled()) {
                    String m = (active) ? "Enter" : "Exit";
                    LOG.info("{} VTN mode", m);
                }

                for (IVTNModeListener listener: vtnModeListeners) {
                    listener.vtnModeChanged(active);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify the specified listner of the VTN mode change.
     *
     * @param listener VTN mode listener service.
     * @param active   {@code true} if the VTN has been activated.
     *                 {@code false} if the VTN has been inactivated.
     */
    public void notifyChange(final IVTNModeListener listener,
                             final boolean active) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.vtnModeChanged(active);
            }
        };
        postJob(r);
    }

    /**
     * Notify the virtual tenant changes.
     *
     * @param path    Path to the tenant.
     * @param vtenant Information about the virtual tenant.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyChange(final VTenantPath path, final VTenant vtenant,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                LOG.info("{}: Tenant {}: {}", path, type.getName(), vtenant);
                for (IVTNManagerAware listener: vtnManagerAware) {
                    listener.vtnChanged(path, vtenant, type);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify the specified listner of the virtual tenant changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the tenant.
     * @param vtenant   Information about the virtual tenant.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     *                  {@code CHANGED} if changed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VTenantPath path, final VTenant vtenant,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.vtnChanged(path, vtenant, type);
            }
        };
        postJob(r);
    }

    /**
     * Notify the virtual L2 bridge changes.
     *
     * @param path     Path to the bridge.
     * @param vbridge  Information about the virtual L2 bridge.
     * @param type     {@code ADDED} if added.
     *                 {@code REMOVED} if removed.
     *                 {@code CHANGED} if changed.
     */
    public void notifyChange(final VBridgePath path, final VBridge vbridge,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                LOG.info("{}: Bridge {}: {}", path, type.getName(), vbridge);

                for (IVTNManagerAware listener: vtnManagerAware) {
                    listener.vBridgeChanged(path, vbridge, type);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify the specified listener of the virtual L2 bridge changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the bridge.
     * @param vbridge   Information about the virtual L2 bridge.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     *                  {@code CHANGED} if changed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VBridgePath path, final VBridge vbridge,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.vBridgeChanged(path, vbridge, type);
            }
        };
        postJob(r);
    }

    /**
     * Notify the L2 bridge interface changes.
     *
     * @param path    Path to the interface.
     * @param viface  Information about the virtual interface.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyChange(final VBridgeIfPath path, final VInterface viface,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                LOG.info("{}: Bridge interface {}: {}", path, type.getName(),
                         viface);

                for (IVTNManagerAware listener: vtnManagerAware) {
                    listener.vBridgeInterfaceChanged(path, viface, type);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify the specified listener of the L2 bridge interface changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the interface.
     * @param viface    Information about the virtual interface.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     *                  {@code CHANGED} if changed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VBridgeIfPath path, final VInterface viface,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.vBridgeInterfaceChanged(path, viface, type);
            }
        };
        postJob(r);
    }

    /**
     * Notify VLAN mapping changes.
     *
     * @param path   Path to the bridge associated with the VLAN mapping.
     * @param vlmap  Information about the VLAN mapping.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     */
    public void notifyChange(final VBridgePath path, final VlanMap vlmap,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                LOG.info("{}: VLAN mapping {}: {}", path, type.getName(),
                         vlmap);

                for (IVTNManagerAware listener: vtnManagerAware) {
                    listener.vlanMapChanged(path, vlmap, type);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify VLAN mapping changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the bridge associated with the VLAN mapping.
     * @param vlmap     Information about the VLAN mapping.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VBridgePath path, final VlanMap vlmap,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.vlanMapChanged(path, vlmap, type);
            }
        };
        postJob(r);
    }

    /**
     * Notify port mapping changes.
     *
     * @param path  Path to the bridge interface.
     * @param pmap  Information about the port mapping.
     * @param type  {@code ADDED} if added.
     *              {@code REMOVED} if removed.
     */
    public void notifyChange(final VBridgeIfPath path, final PortMap pmap,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                LOG.info("{}: Port mapping {}: {}", path, type.getName(),
                         pmap);

                for (IVTNManagerAware listener: vtnManagerAware) {
                    listener.portMapChanged(path, pmap, type);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify port mapping changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the bridge interface.
     * @param pmap      Information about the port mapping.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VBridgeIfPath path, final PortMap pmap,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.portMapChanged(path, pmap, type);
            }
        };
        postJob(r);
    }

    /**
     * Notify the host listener of a new host.
     *
     * @param host  A new host.
     */
    void notifyHost(final HostNodeConnector host) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                for (IfHostListener listener: hostListeners) {
                    listener.hostListener(host);
                }
            }
        };
        postJob(r);
    }

    /**
     * Notify the listener of current VTN configuration.
     *
     * @param listener  VTN manager listener service.
     */
    private void notifyConfiguration(IVTNManagerAware listener) {
        UpdateType type = UpdateType.ADDED;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                VTenant vtenant = vtn.getVTenant();
                VTenantPath path = new VTenantPath(vtn.getName());
                notifyChange(listener, path, vtenant, type);
                vtn.notifyConfiguration(this, listener);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change VTN mode if needed.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     */
    private synchronized void updateVTNMode() {
        if (!inContainerMode && !tenantDB.isEmpty()) {
            // Stop ARP handler emulator, and activate VTN.
            if (arpHandler != null) {
                arpHandler.destroy();
                arpHandler = null;
                notifyChange(true);
            }
        } else if (arpHandler == null) {
            // Inactivate VTN, and start ARP handler emulator.
            arpHandler = new ArpHandler(this);
            notifyChange(false);
        }
    }

    /**
     * Save all configurations to the local filesystem.
     *
     * @param apply  {@code true} is passed if the current configuration needs
     *               to be applied to the VTN Manager.
     * @return  "Success" or failure reason.
     */
    private Status saveLocalConfig(boolean apply) {
        if (tenantDB == null) {
            return cacheNotInitialized();
        }

        VTNManagerImpl mgr = (apply) ? this : null;
        HashSet<String> nameSet = new HashSet<String>();
        Status failure = null;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                nameSet.add(vtn.getName());

                Status status = vtn.saveConfig(mgr);
                if (!status.isSuccess()) {
                    failure = status;
                }
            }
        } finally {
            rdlock.unlock();
        }

        Status status = saveTenantNames(nameSet);
        if (failure != null) {
            return failure;
        }

        return status;
    }

    /**
     * Return ARP handler emulator.
     *
     * @return  An ARP handler emulator object is returned if no VTN exists.
     *          {@code null} is returned if at least one VTN exists.
     */
    private synchronized ArpHandler getArpHandler() {
        return arpHandler;
    }

    /**
     * Add the given node to {@link #disabledNodes}.
     *
     * <p>
     *   Neighbor node discovery may not be completed on a newly detected node.
     *   If a PACKET_OUT is sent to a port in a newly detected node, it may
     *   cause broadcast packet storm because the controller can not
     *   determine internal port in a node yet.
     * </p>
     * <p>
     *   That is why we should disable any packet service on a newly detected
     *   node for a while.
     * </p>
     *
     * @param node  A newly detected node.
     */
    private void addDisabledNode(final Node node) {
        int edgeWait = vtnConfig.getNodeEdgeWait();
        if (edgeWait <= 0) {
            return;
        }

        // Create a timer task to remove the node from disabledNodes.
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                if (disabledNodes.remove(node) != null) {
                    if (LOG.isDebugEnabled()) {
                        LOG.debug("{}: {}: Start packet service",
                                  containerName, node);
                    }
                }
            }
        };

        if (disabledNodes.putIfAbsent(node, task) == null) {
            Timer timer = resourceManager.getTimer();
            timer.schedule(task, edgeWait);
        }
    }

    // IVTNManager

    /**
     * Determine whether the Virtual Tenant Network is active in the container.
     *
     * @return  {@code true} is returned if the VTN is active in the container.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public synchronized boolean isActive() {
        return (arpHandler == null);
    }

    /**
     * Return a list of virtual tenant configurations.
     *
     * @return  A list which contains tenant configurations.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VTenant> getTenants() throws VTNException {
        if (tenantDB == null) {
            throw new VTNException(cacheNotInitialized());
        }

        // Sort tenants by their name.
        TreeMap<String, VTenantImpl> tree;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            tree = new TreeMap(tenantDB);
        } finally {
            rdlock.unlock();
        }

        List<VTenant> list = new ArrayList<VTenant>();
        for (VTenantImpl vtn: tree.values()) {
            list.add(vtn.getVTenant());
        }

        return list;
    }

    /**
     * Return the tenant information specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  Information about the specified tenant.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VTenant getTenant(VTenantPath path) throws VTNException {
        VTenant vtenant;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtenant = vtn.getVTenant();
        } finally {
            rdlock.unlock();
        }

        return vtenant;
    }

    /**
     * Add a new virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addTenant(VTenantPath path, VTenantConfig tconf) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        if (tenantDB == null) {
            return cacheNotInitialized();
        }
        if (path == null) {
            return argumentIsNull("Path");
        }
        if (tconf == null) {
            return argumentIsNull("Tenant configuration");
        }

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Ensure the given tenant name is valid.
            String tenantName = path.getTenantName();
            checkName("Tenant", tenantName);

            VTenantImpl vtn = new VTenantImpl(containerName, tenantName,
                                              tconf);
            if (tenantDB.putIfAbsent(tenantName, vtn) != null) {
                String msg = tenantName + ": Tenant name already exists";
                return new Status(StatusCode.CONFLICT, msg);
            }

            Status stName = saveTenantNames();
            Status stConf = saveTenantConfig(vtn);
            if (!stName.isSuccess()) {
                status = stName;
            } else if (!stConf.isSuccess()) {
                status = stConf;
            } else {
                status = new Status(StatusCode.SUCCESS, null);
            }

            VTenant vtenant = vtn.getVTenant();
            updateVTNMode();
            notifyChange(path, vtenant, UpdateType.ADDED);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            wrlock.unlock();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @param all    If {@code true} is specified, all attributes of the
     *               tenant are modified. In this case, {@code null} in
     *               {@code tconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code tconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyTenant(VTenantPath path, VTenantConfig tconf,
                               boolean all) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }
        if (tconf == null) {
            return argumentIsNull("Tenant configuration");
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            if (!vtn.setVTenantConfig(this, path, tconf, all)) {
                return new Status(StatusCode.SUCCESS, "Not modified");
            }

            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Remove a tenant specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeTenant(VTenantPath path) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        if (tenantDB == null) {
            return cacheNotInitialized();
        }
        if (path == null) {
            return argumentIsNull("Path");
        }

        String tenantName = path.getTenantName();
        if (tenantName == null) {
            return argumentIsNull("Tenant name");
        }

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Make the specified tenant invisible.
            VTenantImpl vtn = tenantDB.remove(tenantName);
            if (vtn == null) {
                return tenantNotFound(tenantName);
            }

            status = saveTenantNames();
            if (configSaveEvent != null) {
                configSaveEvent.remove(tenantName);
            }

            VTenant vtenant = vtn.getVTenant();
            vtn.destroy(this);
            notifyChange(path, vtenant, UpdateType.REMOVED);
            updateVTNMode();
        } finally {
            wrlock.unlock();
        }

        return status;
    }

    /**
     * Return a list of virtual L2 bridges in the specified tenant.
     *
     * @param path  Path to the virtual tenant.
     * @return  A list of virtual L2 bridges.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VBridge> getBridges(VTenantPath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getBridges(this);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the specified virtual L2 bridge.
     *
     * @param path  Path to the virtual bridge.
     * @return  Information about the specified L2 bridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VBridge getBridge(VBridgePath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getBridge(this, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add a new virtual L2 bridge.
     *
     * @param path   Path to the virtual bridge to be added.
     * @param bconf  Bridge configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addBridge(VBridgePath path, VBridgeConfig bconf) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.addBridge(this, path, bconf);
            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual L2 bridge.
     *
     * @param path   Path to the virtual bridge.
     * @param bconf  Bridge configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               bridge are modified. In this case, {@code null} in
     *               {@code bconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code bconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyBridge(VBridgePath path, VBridgeConfig bconf,
                               boolean all) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            if (!vtn.modifyBridge(this, path, bconf, all)) {
                return new Status(StatusCode.SUCCESS, "Not modified");
            }

            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Remove the virtual L2 bridge specified by the given name.
     *
     * @param path  Path to the virtual bridge.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeBridge(VBridgePath path) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeBridge(this, path);
            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Return a list of virtual interfaces attached to the specified virtual
     * L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of virtual interfaces.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VInterface> getBridgeInterfaces(VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getBridgeInterfaces(this, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the specified virtual interface attached to
     * the virtual L2 bridge.
     *
     * @param path  Path to the interface.
     * @return  Information about the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VInterface getBridgeInterface(VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getBridgeInterface(this, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add a new virtual interface to the virtual L2 bridge.
     *
     * @param path   Path to the interface to be added.
     * @param iconf  Interface configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addBridgeInterface(VBridgeIfPath path,
                                     VInterfaceConfig iconf) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.addBridgeInterface(this, path, iconf);
            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Modify configuration of existing virtual interface attached to the
     * virtual L2 bridge.
     *
     * @param path   Path to the interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyBridgeInterface(VBridgeIfPath path,
                                        VInterfaceConfig iconf, boolean all) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            if (!vtn.modifyBridgeInterface(this, path, iconf, all)) {
                return new Status(StatusCode.SUCCESS, "Not modified");
            }

            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Remove the virtual interface from the virtual L2 bridge.
     *
     * @param path  Path to the interface.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeBridgeInterface(VBridgeIfPath path) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeBridgeInterface(this, path);
            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Return a list of VLAN mappings in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of VLAN mappings.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VlanMap> getVlanMaps(VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getVlanMaps(path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return information about the specified VLAN mapping in the virtual
     * L2 bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  Information about the specified VLAN mapping.
     * @throws VTNException  An error occurred.
     */
    @Override
     public VlanMap getVlanMap(VBridgePath path, String mapId)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getVlanMap(path, mapId);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Add a new VLAN mapping to the virtual L2 bridge.
     *
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about added VLAN mapping, which includes
     *          VLAN map identifier.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        Status status = checkContainerMode();
        if (status != null) {
            throw new VTNException(status);
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            VlanMap vlmap = vtn.addVlanMap(this, path, vlconf);
            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }
            return vlmap;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove the VLAN mapping from the virtual L2 bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeVlanMap(VBridgePath path, String mapId) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeVlanMap(this, path, mapId);
            tenantDB.put(vtn.getName(), vtn);
            status = saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return status;
    }

    /**
     * Return the port mapping configuration applied to the specified virtual
     * bridge interface.
     *
     * @param path  Path to the bridge interface.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PortMap getPortMap(VBridgeIfPath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getPortMap(this, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * virtual bridge interface.
     *
     * @param path    Path to the bridge interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf) {
        Status status = checkContainerMode();
        if (status != null) {
            return status;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.setPortMap(this, path, pmconf);
            tenantDB.put(vtn.getName(), vtn);
            return saveTenantConfig(vtn);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Initiate the discovery of a host base on its IP address.
     *
     * <p>
     *   If the given IP address is an IPv4 address, this method sends
     *   a broadcast ARP request to the specified virtual L2 bridges.
     *   If a host is found, it is reported to {@code HostTracker} via
     *   {@code IfHostListener}.
     * </p>
     *
     * @param addr     IP address.
     * @param pathSet  A set of destination paths of virtual L2 bridges.
     *                 If {@code null} is specified, a ARP request is sent
     *                 to all existing bridges.
     */
    @Override
    public void findHost(InetAddress addr, Set<VBridgePath> pathSet) {
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: findHost() called: addr={}, pathSet={}",
                      containerName, addr, pathSet);
        }

        // Create an ARP request with specifying broadcast address.
        byte[] bcast = {-1, -1, -1, -1, -1, -1};
        Ethernet ether = createArpRequest(bcast, addr);
        if (ether == null) {
            return;
        }

        PacketContext pctx = new PacketContext(ether, null);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (pathSet != null) {
                for (VBridgePath bpath: pathSet) {
                    try {
                        VTenantImpl vtn = getTenantImpl(bpath);
                        vtn.findHost(this, pctx, bpath);
                    } catch (VTNException e) {
                        LOG.error("{}: findHost({}): {}",
                                  containerName, bpath, e.toString());
                    }
                }
            } else {
                for (VTenantImpl vtn: tenantDB.values()) {
                    vtn.findHost(this, pctx);
                }
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Send a unicast ARP request to the specified host.
     *
     * <p>
     *   If the specified host sends a ARP reply, it is reported to
     *   {@code HostTracker} via {@code IfHostListener}.
     * </p>
     *
     * @param host  A host to be probed.
     * @return  {@code true} is returned if an ARP request was sent.
     *          Otherwise {@code false} is returned.
     */
    @Override
    public boolean probeHost(HostNodeConnector host) {
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: probeHost() called: host={}", containerName, host);
        }

        if (host == null) {
            return false;
        }

        NodeConnector nc = host.getnodeConnector();
        try {
            checkNodeConnector(nc);
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.debug("{}: probeHost: Ignore request for {}: {}",
                      containerName, host, status.getDescription());
            return false;
        }

        if (topologyManager.isInternal(nc)) {
            LOG.debug("{}: probeHost: Ignore request for {}: Internal port",
                      containerName, host);
            return false;
        }

        // Create an unicast ARP request.
        InetAddress target = host.getNetworkAddress();
        byte[] dst = host.getDataLayerAddressBytes();
        if (dst == null) {
            LOG.debug("{}: probeHost: Ignore request for {}: Invalid dstaddr",
                      containerName, host);
            return false;
        }
        Ethernet ether = createArpRequest(dst, target, host.getVlan());
        if (ether == null) {
            LOG.debug("{}: probeHost: Ignore request for {}: " +
                      "Invalid IP address", containerName, host);
            return false;
        }

        PacketContext pctx = new PacketContext(ether, nc);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Port mappings should precede VLAN mappings.
            for (MapType type: MapType.values()) {
                if (type == MapType.ALL) {
                    break;
                }

                for (VTenantImpl vtn: tenantDB.values()) {
                    Boolean res = vtn.probeHost(this, type, pctx);
                    if (res != null) {
                        return res.booleanValue();
                    }
                }
            }
        } finally {
            rdlock.unlock();
        }

        return false;
    }

    /**
     * Return a list of MAC address entries learned by the specified virtual
     * L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of MAC address entries.
     * @throws VTNException  An error occurred.
     */
    public List<MacAddressEntry> getMacEntries(VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacEntries(this, path);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Search for a MAC address entry from the MAC address table in the
     * specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @param addr  MAC address.
     * @return  A MAC address entry associated with the specified MAC address.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacEntry(this, path, addr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Remove a MAC address entry from the MAC address table in the virtual L2
     * bridge.
     *
     * @param path  Path to the bridge.
     * @param addr  Ethernet address.
     * @return  A MAC address entry actually removed is returned.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry removeMacEntry(VBridgePath path,
                                          DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeMacEntry(this, path, addr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Flush all MAC address table entries in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  "Success" or failure reason.
     */
    public Status flushMacEntries(VBridgePath path) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            vtn.flushMacEntries(this, path);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    // IObjectReader

    /**
     * Read an object from the given input stream.
     *
     * @param ois  Input stream.
     * @return     An object.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @Override
    public Object readObject(ObjectInputStream ois)
        throws IOException, ClassNotFoundException {
        return ois.readObject();
    }

    // ICacheUpdateAware

    /**
     * Invoked when a new entry is available in the cache, the key is
     * only provided, the value will come as an entryUpdated invocation.
     *
     * @param key Key for the entry just created
     * @param cacheName name of the cache for which update has been
     * received
     * @param originLocal true if the event is generated from this
     * node
     */
    @Override
    public void entryCreated(String key, String cacheName,
                             boolean originLocal) {
        if (!originLocal) {
            // Update a list of the tenant names.
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Received new tenant name",
                          containerName, key);
            }
            saveTenantNames();
        }
    }

    /**
     * Called anytime a given entry is updated
     *
     * @param key         Key for the entry modified.
     * @param newValue    The new value the key will have.
     * @param cacheName   Name of the cache for which update has been received.
     * @param originLocal {@code true} if the event is generated from this
     *                    node.
     */
    @Override
    public void entryUpdated(String key, Long newValue, String cacheName,
                             boolean originLocal) {
        if (originLocal || tenantDB == null) {
            return;
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}.{}: Received config save event",
                      containerName, key);
        }
        if (key.equals(CHSAVE_ALL)) {
            saveLocalConfig(true);
            return;
        }

        // Save the tenant configuration specified by the map key.
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = tenantDB.get(key);
            if (vtn != null) {
                vtn.saveConfig(this);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Called anytime a given key is removed from the
     * ConcurrentHashMap we are listening to.
     *
     * @param key         Key of the entry removed
     * @param cacheName   Name of the cache for which update has been received.
     * @param originLocal {@code true} if the event is generated from this
     *                    node.
     */
    @Override
    public void entryDeleted(String key, String cacheName,
                             boolean originLocal) {
        if (!originLocal) {
            // Update a list of the tenant names.
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Received removed tenant name",
                          containerName, key);
            }
            saveTenantNames();

            if (!key.equals(CHSAVE_ALL)) {
                // Delete the virtual tenant configuration file.
                VTenantImpl.deleteConfigFile(containerName, key);
            }
        }
    }

    // IConfigurationContainerAware

    /**
     * Trigger from configuration component to persist the configuration state.
     *
     * @return  "Success" or failure reason.
     */
    @Override
    public Status saveConfiguration() {
        raiseConfigSaveEvent(CHSAVE_ALL);
        return saveLocalConfig(false);
    }

    // IInventoryListener

    /**
     * This method is called when some properties of a node are
     * added/deleted/changed.
     *
     * @param node     {@link Node} being updated
     * @param type     {@link UpdateType}
     * @param propMap  Map of {@link Property}
     */
    @Override
    public void notifyNode(Node node, UpdateType type,
                           Map<String, Property> propMap) {
        if (type == UpdateType.CHANGED) {
            // Nothing to do.
            return;
        }

        if (type == UpdateType.ADDED) {
            addDisabledNode(node);
        } else {
            assert type == UpdateType.REMOVED;
            TimerTask task = disabledNodes.remove(node);
            if (task != null) {
                task.cancel();
            }
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNode(this, node, type);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * This method is called when some properties of a node connector are
     * added/deleted/changed.
     *
     * @param nc       Node connector being updated
     * @param type     {@link UpdateType}
     * @param propMap  Map of {@link Property}
     */
    @Override
    public void notifyNodeConnector(NodeConnector nc, UpdateType type,
                                    Map<String, Property> propMap) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNodeConnector(this, nc, type);
            }
        } finally {
            rdlock.unlock();
        }
    }

    // ITopologyManagerAware

    /**
     * Called to update on Edge in the topology graph.
     *
     * @param topoList  List of topoedgeupdates Each topoedgeupdate includes
     *                  edge, its Properties (BandWidth and/or Latency etc)
     *                  and update type.
     */
    @Override
    public void edgeUpdate(List<TopoEdgeUpdate> topoList) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.edgeUpdate(this, topoList);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Called when an Edge utilization is above the safety threshold configured
     * on the controller
     *
     * @param edge  The edge which bandwidth usage is above the safety level
     */
    @Override
    public void edgeOverUtilized(Edge edge) {
    }

    /**
     * Called when the Edge utilization is back to normal, below the safety
     * threshold level configured on the controller
     *
     * @param edge  The edge which bandwidth usage is back to the normal level.
     */
    @Override
    public void edgeUtilBackToNormal(Edge edge) {
    }

    // IContainerListener

    /**
     * Called to notify a change in the tag assigned to a switch.
     *
     * @param containerName container for which the update has been raised
     * @param n Node of the tag under notification
     * @param oldTag previous version of the tag, this differ from the
     * newTag only if the UpdateType is a modify
     * @param newTag new value for the tag, different from oldTag only
     * in case of modify operation
     * @param t type of update
     */
    @Override
    public void tagUpdated(String containerName, Node n, short oldTag,
                           short newTag, UpdateType t) {
    }

    /**
     * Notification raised when the container flow layout changes.
     *
     * @param containerName container for which the update has been raised
     * @param previousFlow previous value of the container flow under
     * update, differs from the currentFlow only and only if it's an
     * update operation
     * @param currentFlow current version of the container flow differs from
     * the previousFlow only in case of update
     * @param t type of update
     */
    @Override
    public void containerFlowUpdated(String containerName,
                                     ContainerFlow previousFlow,
                                     ContainerFlow currentFlow,
                                     UpdateType t) {
    }

    /**
     * Notification raised when a NodeConnector is added or removed in
     * the container.
     *
     * @param containerName container for which the update has been raised
     * @param p NodeConnector being updated
     * @param t type of modification, but among the types the modify
     * operation is not expected to be raised because the
     * nodeConnectors are anyway immutable so this is only used to
     * add/delete
     */
    @Override
    public void nodeConnectorUpdated(String containerName, NodeConnector p,
                                     UpdateType t) {
    }

    /**
     * Notification raised when the container mode has changed.
     * This notification is needed for some bundle in the default container
     * to cleanup some HW state when switching from non-slicing to
     * slicing case and vice-versa.
     *
     * @param t  ADDED when first container is created, REMOVED when last
     *           container is removed
     */
    @Override
    public void containerModeUpdated(UpdateType t) {
        assert containerName.equals(GlobalConstants.DEFAULT.toString());

        boolean mode;
        switch (t) {
        case ADDED:
            mode = true;
            break;

        case REMOVED:
            mode = false;
            break;

        default:
            return;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            synchronized (this) {
                inContainerMode = mode;
                updateVTNMode();
            }
        } finally {
            rdlock.unlock();
        }
    }

    // IListenDataPacket

    /**
     * Handler for receiving the packet.
     *
     * @param inPkt  Packet received
     * @return  An indication if the packet should still be processed or
     *          we should stop it.
     */
    @Override
    public PacketResult receiveDataPacket(RawPacket inPkt) {
        if (inPkt == null) {
            LOG.warn("{}: Ignore null packet", containerName);
            return PacketResult.IGNORED;
        }

        // Verify incoming node connector in the raw packet.
        NodeConnector nc = inPkt.getIncomingNodeConnector();
        try {
            checkNodeConnector(nc);
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.error("{}: Ignore packet: {}", containerName,
                      status.getDescription());
            return PacketResult.IGNORED;
        }

        if (disabledNodes.containsKey(nc.getNode())) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Ignore packet from disabled node: {}",
                          containerName, nc.getNode());
            }
            return PacketResult.IGNORED;
        }

        // Decode received packet.
        Packet decoded = dataPacketService.decodeDataPacket(inPkt);
        if (!(decoded instanceof Ethernet)) {
            if (decoded == null) {
                LOG.error("{}: Ignore broken packet", containerName);
            } else {
                LOG.debug("{}: Ignore non-ethernet packet: {}", containerName,
                          decoded);
            }
            return PacketResult.IGNORED;
        }

        // Create a packet context.
        PacketResult res = PacketResult.IGNORED;
        PacketContext pctx = new PacketContext(inPkt, (Ethernet)decoded);

        byte[] src = pctx.getSourceAddress();
        byte[] ctlrMac = switchManager.getControllerMAC();
        if (Arrays.equals(src, ctlrMac)) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Ignore self-originated packet: {}",
                          containerName, pctx.getDescription(nc));
            }
            return PacketResult.IGNORED;
        }

        ArpHandler arph = getArpHandler();
        if (arph != null) {
            // No VTN. Pass packet to the ARP handler emulator.
            return arph.receive(pctx);
        }

        if (topologyManager.isInternal(nc)) {
            LOG.debug("{}: Ignore packet from internal node connector: {}",
                      containerName, nc);
            return PacketResult.IGNORED;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Port mappings should precede VLAN mappings.
            for (MapType type: MapType.values()) {
                if (type == MapType.ALL) {
                    break;
                }

                for (VTenantImpl vtn: tenantDB.values()) {
                    res = vtn.receive(this, type, pctx);
                    if (res != PacketResult.IGNORED) {
                        break;
                    }
                }
            }
        } finally {
            rdlock.unlock();
        }

        return res;
    }

    // IListenRoutingUpdates

    /**
     * Invoked when the recalculation of the all shortest path tree is done.
     */
    @Override
    public void recalculateDone() {
        LOG.debug("{}: Shortest path recalculated", containerName);

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.recalculateDone(this);
            }
        } finally {
            rdlock.unlock();
        }
    }

    // IHostFinder

    /**
     * This method initiates the discovery of a host based on its IP address.
     * This is triggered by query of an application to the HostTracker. The
     * requested IP address doesn't exist in the local database at this point.
     *
     * @param networkAddress  IP Address encapsulated in InetAddress class
     */
    @Override
    public void find(InetAddress networkAddress) {
        ArpHandler arph = getArpHandler();
        if (arph == null) {
            findHost(networkAddress, null);
        } else {
            arph.find(networkAddress);
        }
    }

    /**
     * This method is called by HostTracker to see if a learned Host is still
     * in the network. Used mostly for ARP Aging.
     *
     * @param host  The Host that needs to be probed
     */
    @Override
    public void probe(HostNodeConnector host) {
        ArpHandler arph = getArpHandler();
        if (arph == null) {
            probeHost(host);
        } else {
            arph.probe(host);
        }
    }
}
