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
import java.util.Deque;
import java.util.List;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Map;
import java.util.TreeMap;
import java.util.Set;
import java.util.HashSet;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.IVTNFlowDebugger;
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
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortProperty;
import org.opendaylight.vtn.manager.internal.cluster.RawPacketEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.clustering.services.
    IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.containermanager.IContainerManager;
import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.
    IForwardingRulesManager;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.ContainerFlow;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.IContainerListener;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.flowprogrammer.IFlowProgrammerListener;
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
import org.opendaylight.controller.sal.utils.HexEncode;
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
public class VTNManagerImpl
    implements IVTNManager, IVTNFlowDebugger, IObjectReader,
               ICacheUpdateAware<ClusterEventId, Object>,
               IConfigurationContainerAware, IInventoryListener,
               ITopologyManagerAware, IContainerListener, IListenDataPacket,
               IListenRoutingUpdates, IHostFinder, IFlowProgrammerListener {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VTNManagerImpl.class);

    /**
     * Maximum length of the resource name.
     */
    private static final int RESOURCE_NAME_MAXLEN = 31;

    /**
     * Maximum lifetime, in milliseconds, of a cluster event.
     */
    private static final long CLUSTER_EVENT_LIFETIME = 1000L;

    /**
     * Regular expression that matches valid resource name.
     */
    private static final Pattern RESOURCE_NAME_REGEX =
        Pattern.compile("^\\p{Alnum}[\\p{Alnum}_]*$");

    /**
     * Maximum value of VLAN ID.
     */
    static final short VLAN_ID_MAX = 4095;

    /**
     * The number of bytes in an IPv4 address.
     */
    static final int  IPV4_ADDRLEN = 4;

    /**
     * The number of bits in an integer value.
     */
    static final int  NBITS_INT = 32;

    /**
     * Cluster cache name associated with {@link #tenantDB}.
     */
    static final String  CACHE_TENANT = "vtn.tenant";

    /**
     * Cluster cache name associated with {@link #stateDB}.
     */
    static final String  CACHE_STATE = "vtn.state";

    /**
     * The name of the cluster cache which keeps pairs of existing nodes and
     * sets of node connectors.
     */
    static final String CACHE_NODES = "vtn.nodes";

    /**
     * The name of the cluster cache which keeps pairs of existing node
     * connectors and properties.
     */
    static final String CACHE_PORTS = "vtn.ports";

    /**
     * The name of the cluster cache which keeps internal ports.
     */
    static final String CACHE_ISL = "vtn.isl";

    /**
     * The name of the cluster cache which keeps MAC address table entries.
     */
    static final String CACHE_MAC = "vtn.mac";

    /**
     * Cluster cache name associated with {@link #clusterEvent}.
     */
    static final String  CACHE_EVENT = "vtn.clusterEvent";

    /**
     * The name of the cluster cache which keeps flow entries in the container.
     */
    static final String CACHE_FLOWS = "vtn.flows";

    /**
     * Pseudo tenant name used for the configuration file name which keeps
     * a list of tenant names.
     */
    private static final String  FNAME_TENANT_NAMES = "tenant-names";

    /**
     * Polling interval, in milliseconds, to wait for completion of cluster
     * cache initialization.
     */
    private static final long  CACHE_INIT_POLLTIME = 100L;

    /**
     * Keeps virtual tenant configurations in a container.
     */
    private ConcurrentMap<String, VTenantImpl>  tenantDB;

    /**
     * Keeps runtime state of virtual nodes.
     */
    private ConcurrentMap<VTenantPath, Object>  stateDB;

    /**
     * A cluster cache used to deliver events to nodes in the cluster.
     */
    private ConcurrentMap<ClusterEventId, ClusterEvent>  clusterEvent;

    /**
     * VTN flow database.
     */
    private ConcurrentMap<FlowGroupId, VTNFlow>  flowDB;

    /**
     * Keeps existing nodes as map key.
     *
     * <p>
     *   This map is used as {@code Set<Node>}. So we use {@code VNodeState}
     *   enum as value in order to reduce memory footprint and traffic between
     *   cluster nodes.
     * </p>
     */
    private ConcurrentMap<Node, VNodeState>  nodeDB;

    /**
     * Keeps pairs of existing node connectors and properties.
     */
    private ConcurrentMap<NodeConnector, PortProperty>  portDB;

    /**
     * Keeps internal ports as map key.
     *
     * <p>
     *   This map is used as {@code Set<NodeConnector>}. So we use
     *   {@code VNodeState} enum as value in order to reduce memory footprint
     *   and traffic between cluster nodes.
     * </p>
     */
    private ConcurrentMap<NodeConnector, VNodeState>  islDB;

    /**
     * Keeps all MAC address table entries in this container.
     *
     * <p>
     *   This map should not use MAC address as the map key because
     *   different MAC address table entries with the same MAC address may
     *   exist in different virtual bridges.
     * </p>
     * <p>
     *   Note that {@code null} is set if controller cluster is not configured.
     * </p>
     */
    private ConcurrentMap<MacTableEntryId, MacTableEntry> macAddressDB;

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
     * Connection manager service instance.
     */
    private IConnectionManager  connectionManager;

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
     * Single-threaded task queue runner.
     */
    private TaskQueueThread  taskQueueThread;

    /**
     * Single-threaded task queue runner which executes {@link FlowModTask}.
     */
    private TaskQueueThread  flowTaskThread;

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
     * True if the VTN Manager service is available.
     */
    private volatile boolean  serviceAvailable;

    /**
     * True if the container is being destroyed.
     */
    private boolean  destroying;

    /**
     * Static configuration.
     */
    private VTNConfig  vtnConfig;

    /**
     * List of cluster events.
     */
    private final List<ClusterEvent>  clusterEventQueue =
        new ArrayList<ClusterEvent>();

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
    private final ConcurrentMap<VBridgePath, MacAddressTable> macTableMap =
        new ConcurrentHashMap<VBridgePath, MacAddressTable>();

    /**
     * Flow database associated with virtual tenants.
     */
    private final ConcurrentMap<String, VTNFlowDatabase> vtnFlowMap =
        new ConcurrentHashMap<String, VTNFlowDatabase>();

    /**
     * Set of remote flow modification requests.
     */
    private final Set<RemoteFlowRequest>  remoteFlowRequests =
        new HashSet<RemoteFlowRequest>();

    /**
     * A thread which executes queued tasks.
     */
    private final class TaskQueueThread extends Thread {
        /**
         * Task queue.
         */
        private final Deque<Runnable> taskQueue = new LinkedList<Runnable>();

        /**
         * Determine whether the task queue is active or not.
         */
        private boolean  active = true;

        /**
         * Construct a task thread.
         *
         * @param name  The name of the thread.
         */
        private TaskQueueThread(String name) {
            super(name);
        }

        /**
         * Dequeue a task.
         *
         * @return  A dequeued task. {@code null} is returned if the queue
         *          is down.
         */
        private synchronized Runnable getTask() {
            while (taskQueue.size() == 0) {
                if (!active) {
                    return null;
                }
                try {
                    wait();
                } catch (InterruptedException e) {
                }
            }

            return taskQueue.remove();
        }

        /**
         * Post a task to the task queue.
         *
         * @param r  A runnable to be run on a task queue runner thread.
         * @return  {@code true} is returned if the given task was successfully
         *          posted. {@code false} is returned if the task thread is
         *          no longer available.
         */
        private synchronized boolean post(Runnable r) {
            boolean ret = active;
            if (ret) {
                taskQueue.addLast(r);
                notify();
            }
            return ret;
        }

        /**
         * Shut down the task queue.
         *
         * <p>
         *   All queued tasks are dropped.
         * </p>
         */
        private synchronized void shutdown() {
            shutdown(true);
        }

        /**
         * Shut down the task queue.
         *
         * @param clear  Clear task queue if {@code true}.
         */
        private synchronized void shutdown(boolean clear) {
            if (clear) {
                taskQueue.clear();
            }
            active = false;
            notify();
        }

        /**
         * Main routine of a task thread.
         */
        @Override
        public void run() {
            LOG.debug("Start");
            for (Runnable r = getTask(); r != null; r = getTask()) {
                try {
                    r.run();
                } catch (Exception e) {
                    LOG.error("Exception occurred on a task thread.", e);
                }
            }
            LOG.debug("Exit");
        }
    }

    /**
     * A timer task which interrupts the specified thread.
     */
    private final class AlarmTask extends TimerTask {
        /**
         * The target thread.
         */
        private final Thread  targetThread;

        /**
         * Determine whether this alarm is active or not.
         */
        private boolean  active = true;

        /**
         * Construct a new alarm task to interrupt the calling thread.
         */
        private AlarmTask() {
            this(Thread.currentThread());
        }

        /**
         * Construct a new alarm task to interrupt the given thread.
         *
         * @param thread  A target thread.
         */
        private AlarmTask(Thread thread) {
            targetThread = thread;
        }

        /**
         * Inactivate this alarm.
         */
        private synchronized void inactivate() {
            active = false;
            notifyAll();
        }

        /**
         * Wait for completion of this alarm.
         */
        private synchronized void complete() {
            while (active) {
                try {
                    wait();
                } catch (InterruptedException e) {
                }
            }
        }

        /**
         * Interrupt the target thread.
         */
        @Override
        public void run() {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Alarm expired: thread={}", containerName,
                          targetThread);
            }
            targetThread.interrupt();
            inactivate();
        }

        /**
         * Cancel this alarm task.
         *
         * @return  {@code true} is returned only if this alarm has been
         *          canceled before expiration.
         */
        @Override
        public boolean cancel() {
            boolean ret = super.cancel();
            if (ret) {
                inactivate();
            }

            return ret;
        }
    }

    /**
     * An implementation of {@link FilenameFilter} which selects VTN
     * configuration files.
     */
    private static final class ConfigFileNameFilter implements FilenameFilter {
        /**
         * A set of valid tenant names.
         */
        private final Set<String>  validTenants;

        /**
         * Prefix of VTN configuration file name.
         */
        private final String  prefix;

        /**
         * The length of {@link #prefix}.
         */
        private final int  prefixLength;

        /**
         * The minimum length of VTN configuration file name.
         */
        private final int  minimumLength;

        /**
         * Construct a new filename filter which selects configuration files
         * for obsolete tenants.
         *
         * @param cname    The name of the container.
         * @param nameSet  A set of valid virtual tenant names.
         *                 If a non-{@code null} value is specified,
         *                 this object selects tenant configuration files
         *                 only if its name is not contained in the given set.
         *                 In that case this filter never selects the tenant
         *                 name list file even if the given set is empty.
         *                 If {@code null} is specified, all configuration
         *                 files, including the tenant name list file, are
         *                 selected.
         */
        private ConfigFileNameFilter(String cname, Set<String> nameSet) {
            validTenants = nameSet;
            StringBuilder builder =
                new StringBuilder(VTenantImpl.CONFIG_FILE_PREFIX);
            builder.append(cname).append('-');
            prefix = builder.toString();
            prefixLength = prefix.length();
            minimumLength = prefixLength +
                VTenantImpl.CONFIG_FILE_SUFFIX.length();
        }

        /**
         * Determine whether the specified file should be selected or not.
         *
         * @param dir   The directory in which the file was found.
         * @param name  The name of the file.
         * @return      {@code true} is returned if the specified file name
         *              should be selected.
         *              Otherwise {@code false} is returned.
         */
        @Override
        public boolean accept(File dir, String name) {
            if (!name.startsWith(prefix) ||
                !name.endsWith(VTenantImpl.CONFIG_FILE_SUFFIX)) {
                return false;
            }

            int len = name.length() - minimumLength;
            if (len <= 0) {
                return false;
            }

            if (validTenants == null) {
                return true;
            }

            // Parse tenant name part in the filename.
            String tname = name.substring(prefixLength, prefixLength + len);
            return !(FNAME_TENANT_NAMES.equals(tname) ||
                     validTenants.contains(tname));
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

        LOG.debug("{}: init() called", cname);
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
        tenantListFileName =
            VTenantImpl.getConfigFilePath(cname, FNAME_TENANT_NAMES);

        // Start VTN task thread.
        taskQueueThread = new TaskQueueThread("VTN Task Thread: " + cname);
        taskQueueThread.start();

        // Start VTN flow task thread.
        flowTaskThread = new TaskQueueThread("VTN Flow Thread: " + cname);
        flowTaskThread.start();

        // Initialize inventory caches.
        initInventory();
        initISL();

        // Load saved configurations.
        loadConfig();

        // Initialize MAC address tables.
        initMacAddressTable();

        // Initialize VTN flow databases.
        initFlowDatabase();

        if (inContainerMode || tenantDB.isEmpty()) {
            // Start ARP handler emulator.
            arpHandler = new ArpHandler(this);
        }

        resourceManager.addManager(this);
        serviceAvailable = true;
    }

    /**
     * Function called by the dependency manager after the VTN Manager service
     * has been registered to the OSGi service repository.
     */
    void started() {
        LOG.debug("{}: started() called", containerName);
    }

    /**
     * Function called just before the dependency manager stops the service.
     */
    void stopping() {
        LOG.debug("{}: stopping() called", containerName);

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            serviceAvailable = false;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Function called by the dependency manager before the services exported
     * by the component are unregistered, this will be followed by a
     * "destroy()" calls.
     */
    void stop() {
        LOG.debug("{}: stop() called", containerName);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Stop timeout timer in the ARP handler emulator.
            if (arpHandler != null) {
                arpHandler.destroy();
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     */
    void destroy() {
        LOG.debug("{}: destroy() called", containerName);
        resourceManager.removeManager(this);
        vtnManagerAware.clear();

        // Remove all MAC address tables.
        for (Iterator<MacAddressTable> it = macTableMap.values().iterator();
             it.hasNext();) {
            MacAddressTable table = it.next();
            table.destroy(false);
            it.remove();
        }

        // Terminate internal threads.
        if (flowTaskThread != null) {
            flowTaskThread.shutdown(false);
            try {
                flowTaskThread.join();
            } catch (InterruptedException e) {
                LOG.warn("{}: Interrupted while joining thread: {}",
                         containerName, flowTaskThread.getName());
            }
            flowTaskThread = null;
        }

        if (taskQueueThread != null) {
            taskQueueThread.shutdown();
            try {
                taskQueueThread.join();
            } catch (InterruptedException e) {
                LOG.warn("{}: Interrupted while joining thread: {}",
                         containerName, taskQueueThread.getName());
            }
            taskQueueThread = null;
        }

        for (Iterator<TimerTask> it = disabledNodes.values().iterator();
             it.hasNext();) {
            TimerTask task = it.next();
            task.cancel();
            it.remove();
        }

        Timer timer = resourceManager.getTimer();
        timer.purge();

        if (destroying) {
            destroyCaches();
        }
    }

    /**
     * Create cluster caches for VTN.
     */
    private void createCaches() {
        IClusterContainerServices cluster = clusterService;
        if (cluster == null) {
            // Create dummy caches.
            tenantDB = new ConcurrentHashMap<String, VTenantImpl>();
            stateDB = new ConcurrentHashMap<VTenantPath, Object>();
            nodeDB = new ConcurrentHashMap<Node, VNodeState>();
            portDB = new ConcurrentHashMap<NodeConnector, PortProperty>();
            islDB = new ConcurrentHashMap<NodeConnector, VNodeState>();
            clusterEvent =
                new ConcurrentHashMap<ClusterEventId, ClusterEvent>();
            flowDB = new ConcurrentHashMap<FlowGroupId, VTNFlow>();
            macAddressDB = null;
            return;
        }

        // Create transactional cluster caches.
        Set<IClusterServices.cacheMode> cmode =
            EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL,
                       IClusterServices.cacheMode.SYNC);
        createCache(cluster, CACHE_TENANT, cmode);
        createCache(cluster, CACHE_STATE, cmode);
        createCache(cluster, CACHE_NODES, cmode);
        createCache(cluster, CACHE_PORTS, cmode);
        createCache(cluster, CACHE_ISL, cmode);

        tenantDB = (ConcurrentMap<String, VTenantImpl>)
            getCache(cluster, CACHE_TENANT);
        stateDB = (ConcurrentMap<VTenantPath, Object>)
            getCache(cluster, CACHE_STATE);
        nodeDB = (ConcurrentMap<Node, VNodeState>)
            getCache(cluster, CACHE_NODES);
        portDB = (ConcurrentMap<NodeConnector, PortProperty>)
            getCache(cluster, CACHE_PORTS);
        islDB = (ConcurrentMap<NodeConnector, VNodeState>)
            getCache(cluster, CACHE_ISL);

        InetAddress ipaddr = resourceManager.getControllerAddress();
        if (ipaddr.isLoopbackAddress()) {
            // Controller cluster is not configured.
            macAddressDB = null;
        } else {
            // Create cluster cache for MAC address table entries.
            createCache(cluster, CACHE_MAC, cmode);
            macAddressDB = (ConcurrentMap<MacTableEntryId, MacTableEntry>)
                getCache(cluster, CACHE_MAC);
        }

        // Create non-transactional cluster caches.
        // Keys in non-transactional caches should never conflict between
        // cluster nodes.
        cmode = EnumSet.of(IClusterServices.cacheMode.NON_TRANSACTIONAL,
                           IClusterServices.cacheMode.SYNC);
        createCache(cluster, CACHE_EVENT, cmode);
        createCache(cluster, CACHE_FLOWS, cmode);

        clusterEvent = (ConcurrentMap<ClusterEventId, ClusterEvent>)
            getCache(cluster, CACHE_EVENT);
        flowDB = (ConcurrentMap<FlowGroupId, VTNFlow>)
            getCache(cluster, CACHE_FLOWS);

        // Remove obsolete events in clusterEvent.
        Set<ClusterEventId> removed = new HashSet<ClusterEventId>();
        for (ClusterEventId evid: clusterEvent.keySet()) {
            if (evid.isLocal()) {
                removed.add(evid);
            }
        }
        for (ClusterEventId evid: removed) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Remove obsolete cluster event: {}",
                          containerName, evid);
            }
            removeEvent(evid);
        }

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
            LOG.info("{}: {}: Cache already exists", containerName, cacheName);
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
     * @return           Cluster cache.
     * @throws IllegalStateException
     *    The specified cache was not found.
     */
    private ConcurrentMap<?, ?> getCache(IClusterContainerServices cs,
                                         String cacheName) {
        ConcurrentMap<?, ?> cache = cs.getCache(cacheName);
        if (cache == null) {
            String msg = containerName + ": " + cacheName +
                ": Cache not found";
            LOG.error(msg);
            throw new IllegalStateException(msg);
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
            cluster.destroyCache(CACHE_NODES);
            cluster.destroyCache(CACHE_PORTS);
            cluster.destroyCache(CACHE_ISL);
            cluster.destroyCache(CACHE_EVENT);
            cluster.destroyCache(CACHE_FLOWS);

            if (macAddressDB != null) {
                cluster.destroyCache(CACHE_MAC);
            }

            LOG.debug("{}: Destroyed VTN caches.", containerName);
        }
    }

    /**
     * Initialize inventory caches.
     */
    void initInventory() {
        if (switchManager == null) {
            return;
        }

        Set<Node> curNode = new HashSet<Node>(nodeDB.keySet());
        Set<NodeConnector> curPort =
            new HashSet<NodeConnector>(portDB.keySet());
        for (Node node: switchManager.getNodes()) {
            addNode(node);
            curNode.remove(node);

            Set<NodeConnector> ncSet =
                switchManager.getPhysicalNodeConnectors(node);
            for (NodeConnector nc: ncSet) {
                Map<String, Property> prop =
                    switchManager.getNodeConnectorProps(nc);
                addPort(nc, prop);
                curPort.remove(nc);
            }
        }

        if (!curNode.isEmpty()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Remove obsolete nodes from nodeDB: {}",
                          containerName, curNode);
            }
            for (Node node: curNode) {
                nodeDB.remove(node);
            }
        }

        if (!curPort.isEmpty()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Remove obsolete ports from portDB: {}",
                          containerName, curPort);
            }
            for (NodeConnector nc: curPort) {
                portDB.remove(nc);
                islDB.remove(nc);
            }
        }
    }

    /**
     * Initialize inter-switch link cache.
     */
    void initISL() {
        if (topologyManager == null) {
            return;
        }

        Map<Edge, Set<Property>> edgeMap = topologyManager.getEdges();
        if (edgeMap == null) {
            return;
        }

        Set<NodeConnector> current =
            new HashSet<NodeConnector>(islDB.keySet());
        for (Edge edge: edgeMap.keySet()) {
            if (addISL(edge) >= 0) {
                NodeConnector head = edge.getHeadNodeConnector();
                NodeConnector tail = edge.getTailNodeConnector();
                current.remove(head);
                current.remove(tail);
            }
        }

        if (!current.isEmpty()) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Remove obsolete ports from islDB: {}",
                          containerName, current);
            }
            for (NodeConnector nc: current) {
                islDB.remove(nc);
            }
        }
    }

    /**
     * Add the given node to the node DB.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param node  Node associated with the SDN switch.
     * @return  {@code true} is returned if the given node was actually added.
     *          Otherwise {@code false} is returned.
     */
    private boolean addNode(Node node) {
        return addNode(node, true);
    }

    /**
     * Add the given node to the node DB.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param node    Node associated with the SDN switch.
     * @param verify  Verify the given node if {@code true}.
     * @return  {@code true} is returned if the given node was actually added.
     *          Otherwise {@code false} is returned.
     */
    private boolean addNode(Node node, boolean verify) {
        if (verify) {
            try {
                checkNode(node);
            } catch (VTNException e) {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}: addNode: Ignore invalid node {}: {}",
                              containerName, node, e);
                }
                return false;
            }
        }

        if (nodeDB.putIfAbsent(node, VNodeState.UP) == null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addNode: New node {}", containerName, node);
            }
            addDisabledNode(node);
            return true;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: addNode: Ignore existing node {}",
                      containerName, node);
        }
        return false;
    }

    /**
     * Remove the given node from the node DB.
     *
     * <p>
     *   This method also removes node connectors associated with the given
     *   node from the port DB.
     * </p>
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param node  Node associated with the SDN switch.
     * @return  {@code true} is returned if the given node was actually
     *          removed. Otherwise {@code false} is returned.
     */
    private boolean removeNode(Node node) {
        if (nodeDB.remove(node) != null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: removeNode: Removed {}", containerName, node);
            }

            TimerTask task = disabledNodes.remove(node);
            if (task != null) {
                task.cancel();
            }

            // Clean up node connectors in the given node.
            Set<NodeConnector> ports =
                new HashSet<NodeConnector>(portDB.keySet());
            for (NodeConnector nc: ports) {
                if (node.equals(nc.getNode())) {
                    if (LOG.isDebugEnabled()) {
                        LOG.debug("{}: removeNode({}): Remove port {}",
                                  containerName, node, nc);
                    }
                    if (portDB.remove(nc) != null) {
                        removeISLPort(nc);
                    }
                }
            }

            return true;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: removeNode: Ignore unknown node {}",
                      containerName, node);
        }
        return false;
    }

    /**
     * Add the given node connector associated with the physical switch port
     * to the port DB.
     *
     * <p>
     *   If a node in the given node connector does not exist in the node DB,
     *   it will also be added.
     * </p>
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param nc       Node connector associated with the SDN switch port.
     * @param propMap  Property map associated with the SDN switch port.
     * @return  {@code UpdateType.ADDED} is returned if the given port was
     *          actually added.
     *          {@code UpdateType.CHANGED} is returned if property of the
     *          given port was changed.
     *          Otherwise {@code null} is returned.
     */
    private UpdateType addPort(NodeConnector nc,
                               Map<String, Property> propMap) {
        try {
            checkNodeConnector(nc);
        } catch (VTNException e) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addPort: Ignore invalid port {}: {}",
                          containerName, nc, e);
            }
            return null;
        }

        if (switchManager.isSpecial(nc)) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addPort: Ignore special port {}",
                          containerName, nc);
            }
            return null;
        }

        String name;
        boolean enabled;
        if (propMap == null) {
            name = null;
            enabled = false;
        } else {
            Name nm = (Name)propMap.get(Name.NamePropName);
            name = (nm == null) ? null : nm.getValue();

            Config cf = (Config)propMap.get(Config.ConfigPropName);
            State st = (State)propMap.get(State.StatePropName);
            enabled =
                (cf != null && cf.getValue() == Config.ADMIN_UP &&
                 st != null && st.getValue() == State.EDGE_UP);
        }
        PortProperty pp = new PortProperty(name, enabled);
        PortProperty old = portDB.putIfAbsent(nc, pp);
        if (old == null) {
            addNode(nc.getNode(), false);
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addPort: New port: port={}, prop={}",
                          containerName, nc, pp);
            }
            return UpdateType.ADDED;
        }

        if (!old.equals(pp)) {
            portDB.put(nc, pp);
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addPort: Property has been changed: " +
                          "port={}, prop={} -> {}", containerName, nc,
                          old, pp);
            }
            return UpdateType.CHANGED;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: addPort: Ignore existing port {}",
                      containerName, nc);
        }
        return null;
    }

    /**
     * Remove the given node connector from the port DB.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param nc       Node connector associated with the SDN switch port.
     * @return  {@code true} is returned if the given node connector was
     *          actually removed. Otherwise {@code false} is returned.
     */
    private boolean removePort(NodeConnector nc) {
        if (portDB.remove(nc) != null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: removePort: Removed {}", containerName, nc);
            }
            removeISLPort(nc);
            return true;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: removePort: Ignore unknown port {}",
                      containerName, nc);
        }
        return false;
    }

    /**
     * Update inter switch links.
     *
     * @param topoList  A list of {@code TopoEdgeUpdate} passed by topology
     *                  manager.
     * @return  {@code true} is returned if inter switch links was actually
     *          updated. {@code false} is returned if link map was not changed.
     */
    private boolean updateISL(List<TopoEdgeUpdate> topoList) {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            boolean changed = false;
            for (TopoEdgeUpdate topo: topoList) {
                UpdateType type = topo.getUpdateType();
                Edge edge = topo.getEdge();
                if ((type == UpdateType.ADDED && addISL(edge) > 0) ||
                    (type == UpdateType.REMOVED && removeISL(edge))) {
                    changed = true;
                }
            }

            return changed;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Add inter switch link specified by the given edge.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param edge  A edge;
     * @return  {@code 1} is returned if the given edge is actually added.
     *          {@code 0} is returned if the given edge already exists in
     *          the inter switch link map.
     *          {@code -1} is returned if the given edge is invalid.
     */
    private int addISL(Edge edge) {
        NodeConnector head = edge.getHeadNodeConnector();
        NodeConnector tail = edge.getTailNodeConnector();
        if (portDB.containsKey(head) && portDB.containsKey(tail)) {
            boolean h = addISLPort(head);
            boolean t = addISLPort(tail);
            return (h || t) ? 1 : 0;
        } else if (LOG.isDebugEnabled()) {
            LOG.debug("{}: addISL: Ignore invalid edge: {}",
                      containerName, edge);
        }

        return -1;
    }

    /**
     * Remove inter switch link specified by the given edge.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param edge  A edge.
     * @return  {@code true} is returned if the given edge is actually removed.
     *          Otherwise {@code false} is returned.
     */
    private boolean removeISL(Edge edge) {
        NodeConnector head = edge.getHeadNodeConnector();
        NodeConnector tail = edge.getTailNodeConnector();
        boolean h = removeISLPort(head);
        boolean t = removeISLPort(tail);
        return (h || t);
    }

    /**
     * Add the given node connector to the inter switch link map.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param nc  A node connector.
     * @return  {@code true} is returned if the given node connector is
     *          actually added. {@code false} is returned if the given node
     *          connector exists in the inter switch link map.
     */
    private boolean addISLPort(NodeConnector nc) {
        if (islDB.putIfAbsent(nc, VNodeState.UP) == null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addISLPort: New ISL port {}",
                          containerName, nc);
            }
            return true;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: addISLPort: Ignore existing port {}",
                      containerName, nc);
        }
        return false;
    }

    /**
     * Remove the given node connector from the inter switch link map.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param nc  A node connector.
     * @return  {@code true} is returned if the given node connector is
     *          actually removed. {@code false} is returned if the given node
     *          connector does not exist in the inter switch link map.
     */
    private boolean removeISLPort(NodeConnector nc) {
        if (islDB.remove(nc) != null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: removeISLPort: Removed {}", containerName, nc);
            }
            return true;
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: removeISLPort: Ignore unknown port {}",
                      containerName, nc);
        }
        return false;
    }

    /**
     * Initialize the VTN flow database.
     */
    private void initFlowDatabase() {
        List<FlowGroupId> orphans = new ArrayList<FlowGroupId>();
        for (VTNFlow vflow: flowDB.values()) {
            FlowGroupId gid = vflow.getGroupId();
            VTNFlowDatabase fdb = getTenantFlowDB(gid);
            if (fdb != null) {
                // Initialize indices for the VTN flow.
                fdb.createIndex(this, vflow);
            } else {
                // This should never happen.
                LOG.error("{}: Orphan VTN flow was found: {}",
                          containerName, gid);
                orphans.add(gid);
            }
        }

        for (FlowGroupId gid: orphans) {
            flowDB.remove(gid);
        }
    }

    /**
     * Initialize MAC address tables.
     */
    private void initMacAddressTable() {
        if (macAddressDB == null) {
            // Controller cluster is not configured.
            return;
        }

        Set<MacTableEntryId> removed = new HashSet<MacTableEntryId>();
        for (MacTableEntry tent: macAddressDB.values()) {
            MacTableEntryId id = tent.getEntryId();
            if (id.isLocal()) {
                // This MAC address kept by another cluster node is obsolete.
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: Remove obsolete MAC address: {}",
                              containerName, tent);
                }
                removed.add(id);
                continue;
            }

            MacAddressTable table = getMacAddressTable(id);
            if (table != null) {
                table.add(tent);
            } else {
                // This should never happen.
                LOG.error("{}: Orphan MAC address was found: {}",
                          containerName, tent);
                removed.add(id);
            }
        }

        for (MacTableEntryId id: removed) {
            removeMacAddress(id);
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
     * Return cluster service instance.
     *
     * @return  Cluster service instance.
     */
    public IClusterContainerServices getClusterContainerService() {
        return clusterService;
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
     * Invoked when a connection manager service is registered.
     *
     * @param service  Connection manager service.
     */
    void setConnectionManager(IConnectionManager service) {
        LOG.debug("{}: Set connection manager service: {}",
                  containerName, service);
        connectionManager = service;
    }

    /**
     * Invoked when a connection manager service is unregistered.
     *
     * @param service  Connection manager service.
     */
    void unsetConnectionManager(IConnectionManager service) {
        if (connectionManager == service) {
            LOG.debug("{}: Unset connection manager service: {}",
                      containerName, service);
            connectionManager = null;
        }
    }

    /**
     * Return connection manager service instance.
     *
     * @return  Connection manager service.
     */
    public IConnectionManager getConnectionManager() {
        return connectionManager;
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
            resourceManager = null;
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
            notifyChange(listener, isActive());
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
     * Return the switch node DB.
     *
     * @return  Switch node DB.
     */
    public ConcurrentMap<Node, VNodeState> getNodeDB() {
        return nodeDB;
    }

    /**
     * Return the switch port DB.
     *
     * @return  Switch port DB.
     */
    public ConcurrentMap<NodeConnector, PortProperty> getPortDB() {
        return portDB;
    }

    /**
     * Return the MAC address entry DB.
     *
     * @return  MAC address entry DB.
     *          {@code null} is returned if controller cluster is not
     *          configured.
     */
    public ConcurrentMap<MacTableEntryId, MacTableEntry> getMacAddressDB() {
        return macAddressDB;
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
     * Return VTN flow database.
     *
     * @return VTN flow database.
     */
    public ConcurrentMap<FlowGroupId, VTNFlow> getFlowDB() {
        return flowDB;
    }

    /**
     * Return a {@link VTNConfig} object which contains static configuration.
     *
     * @return  A {@link VTNConfig} object.
     */
    public VTNConfig getVTNConfig() {
        return vtnConfig;
    }

    /**
     * Determine whether the VTN Manager service is available or not.
     *
     * @return  {@code true} is returned if the VTN Manager service is
     *          available. Otherwise {@code false} is returned.
     */
    public boolean isAvailable() {
        // It's harmless to access serviceAvailable flag without holding
        // rwLock because this flag will be turned off only once by stopping().
        return serviceAvailable;
    }

    /**
     * Interrupt the calling thread after the specified milliseconds passes.
     *
     * <p>
     *   Note that this method clears interrupt state of the calling thread.
     * </p>
     *
     * @param delay  Delay in milliseconds to be inserted before interrupt.
     * @return  A timer task which implements alarm timer.
     */
    public TimerTask setAlarm(long delay) {
        Timer timer = resourceManager.getTimer();
        AlarmTask task = new AlarmTask();
        Thread.interrupted();
        timer.schedule(task, delay);

        return task;
    }

    /**
     * Cancel the given alarm task.
     *
     * @param task  An alarm task returned by {@link #setAlarm(long)}.
     */
    public void cancelAlarm(TimerTask task) {
        task.cancel();
        if (task instanceof AlarmTask) {
            AlarmTask alarm = (AlarmTask)task;
            alarm.complete();
            Thread.interrupted();
        }
    }

    /**
     * Set alarm timer for flow modification.
     *
     * <p>
     *   Note that this method clears interrupt state of the calling thread.
     * </p>
     *
     * @return  A timer task which implements alarm timer.
     */
    public TimerTask setFlowModAlarm() {
        return setAlarm((long)vtnConfig.getFlowModTimeout());
    }

    /**
     * Add a MAC address table for a virtual L2 bridge.
     *
     * @param path  Path to the virtual L2 bridge.
     * @param age   Interval in milliseconds between aging tasks.
     */
    public void addMacAddressTable(VBridgePath path, int age) {
        MacAddressTable table = new MacAddressTable(this, path, age);
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
            table.destroy(purge);
        }
    }

    /**
     * Return a MAC address table associated with the given virtual L2 bridge.
     *
     * @param path   Path to the virtual L2 bridge.
     * @return  A MAC address table associated with the given virtual L2
     *          bridge. {@code null} is returned if not found.
     */
    public MacAddressTable getMacAddressTable(VBridgePath path) {
        return macTableMap.get(path);
    }

    /**
     * Return a MAC address table that contains the specified MAC address
     * table entry.
     *
     * @param id  An identifier of a MAC address table entry.
     * @return  A MAC address table is returned if found.
     *          {@code null} is returned if not found.
     */
    public MacAddressTable getMacAddressTable(MacTableEntryId id) {
        VBridgePath path = id.getBridgePath();
        return macTableMap.get(path);
    }

    /**
     * Create a new VTN flow database object if not exists.
     *
     * @param name  The name of the virtual tenant.
     */
    public void createTenantFlowDB(String name) {
        VTNFlowDatabase fdb = new VTNFlowDatabase(name);
        vtnFlowMap.put(name, fdb);
    }

    /**
     * Remove VTN flow database object.
     *
     * @param name  The name of the virtual tenant.
     * @return  A removed VTN flow database object.
     *          {@code null} is returned if database does not exist.
     */
    public VTNFlowDatabase removeTenantFlowDB(String name) {
        return vtnFlowMap.remove(name);
    }

    /**
     * Return a VTN flow database object associated with the specified virtual
     * tenant.
     *
     * @param name  The name of the virtual tenant.
     * @return  A VTN flow database associated with the specified virtual
     *          tenant. {@code null} is returned if not fonud.
     */
    public VTNFlowDatabase getTenantFlowDB(String name) {
        return vtnFlowMap.get(name);
    }

    /**
     * Return a VTN flow database which contains flow entry specified by the
     * given name.
     *
     * @param gid  Identifier of the flow group.
     * @return  A VTN flow database is returned if found.
     *          {@code null} is returned if not fonud.
     */
    public VTNFlowDatabase getTenantFlowDB(FlowGroupId gid) {
        String tname = gid.getTenantName();
        return getTenantFlowDB(tname);
    }

    /**
     * Set remote flow modification request which wait for completion of flow
     * modification on remote cluster node.
     *
     * @param req  A remote flow modification request.
     */
    public void addRemoteFlowRequest(RemoteFlowRequest req) {
        synchronized (remoteFlowRequests) {
            remoteFlowRequests.add(req);
        }
    }

    /**
     * Remove remote flow modification request.
     *
     * @param req  A remote flow modification request.
     */
    public void removeRemoteFlowRequest(RemoteFlowRequest req) {
        synchronized (remoteFlowRequests) {
            remoteFlowRequests.remove(req);
        }
    }

    /**
     * Called when a remote cluster node reports a result of flow modification.
     *
     * @param name    The name of flow entry.
     * @param result  Result of flow modification.
     */
    public void setRemoteFlowResult(String name, FlowModResult result) {
        int rsize = resourceManager.getRemoteClusterSize();
        synchronized (remoteFlowRequests) {
            RemoteFlowRequest done = null;
            for (RemoteFlowRequest req: remoteFlowRequests) {
                if (req.setResult(name, result, rsize)) {
                    done = req;
                    break;
                }
            }

            if (done != null) {
                remoteFlowRequests.remove(done);
            }
        }
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
        return (portDB.containsKey(nc) && !islDB.containsKey(nc));
    }

    /**
     * Return a set of existing nodes.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @return  A set of existing nodes.
     */
    public Set<Node> getNodes() {
        return new HashSet<Node>(nodeDB.keySet());
    }

    /**
     * Determine whether the given node exists or not.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param node  Node associated with SDN switch.
     * @return  {@code true} is returned if the given node exists.
     *          Otherwise {@code false} is returned.
     */
    public boolean exists(Node node) {
        return nodeDB.containsKey(node);
    }

    /**
     * Collect node connectors associated with edge switch ports in up state.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param portSet  A set of node connectors to store results.
     */
    public void collectUpEdgePorts(Set<NodeConnector> portSet) {
        for (Map.Entry<NodeConnector, PortProperty> entry: portDB.entrySet()) {
            NodeConnector port = entry.getKey();
            PortProperty pp = entry.getValue();
            if (pp.isEnabled() && !islDB.containsKey(port)) {
                portSet.add(port);
            }
        }
    }

    /**
     * Collect node connectors associated with edge switch ports in up state
     * for the given node.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param portSet  A set of node connectors to store results.
     * @param node     Node associated with SDN switch.
     *                 If {@code null} is specified, this method collects all
     *                 enabled edge ports.
     */
    public void collectUpEdgePorts(Set<NodeConnector> portSet, Node node) {
        if (node == null) {
            collectUpEdgePorts(portSet);
            return;
        }

        for (Map.Entry<NodeConnector, PortProperty> entry: portDB.entrySet()) {
            NodeConnector port = entry.getKey();
            if (port.getNode().equals(node)) {
                PortProperty pp = entry.getValue();
                if (pp.isEnabled() && !islDB.containsKey(port)) {
                    portSet.add(port);
                }
            }
        }
    }

    /**
     * Determine whether the given node has at least one edge port in up state
     * or not.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param node     Node associated with SDN switch.
     * @return  {@code true} is returned if the given node has at least one
     *          edge port in up state. Otherwise {@code false} is returned.
     */
    public boolean hasEdgePort(Node node) {
        for (Map.Entry<NodeConnector, PortProperty> entry: portDB.entrySet()) {
            NodeConnector port = entry.getKey();
            if (port.getNode().equals(node)) {
                PortProperty pp = entry.getValue();
                if (pp.isEnabled() && !islDB.containsKey(port)) {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * Determine whether the given physical switch port is enabled or not.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param port  Node connector associated with physical switch port.
     * @return  {@code true} is returned if the given physical switch port
     *          is enabled. Otherwise {@code false} is returned.
     */
    public boolean isEnabled(NodeConnector port) {
        PortProperty pp = portDB.get(port);
        return (pp != null && pp.isEnabled());
    }

    /**
     * Return property of the specified physical switch port.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param nc  Node connector associated with physical switch port.
     * @return  {@code PortProperty} object which represents switch port
     *          property is returned. {@code null} is returned if not found.
     */
    public PortProperty getPortProperty(NodeConnector nc) {
        return portDB.get(nc);
    }

    /**
     * Transmit an ethernet frame to the specified node connector.
     *
     * @param nc     A node connector.
     * @param ether  An ethernet frame.
     */
    public void transmit(NodeConnector nc, Ethernet ether) {
        Node node = nc.getNode();
        IDataPacketService pktSrv = dataPacketService;
        RawPacket pkt = pktSrv.encodeDataPacket(ether);
        ConnectionLocality cl = connectionManager.getLocalityStatus(node);
        if (cl == ConnectionLocality.LOCAL) {
            if (!disabledNodes.containsKey(node)) {
                pkt.setOutgoingNodeConnector(nc);
                pktSrv.transmitDataPacket(pkt);
            } else if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Don't send packet to disabled node: {}",
                          containerName, node);
            }
        } else if (cl == ConnectionLocality.NOT_LOCAL) {
            // Toss the packet to remote cluster nodes.
            RawPacketEvent ev = new RawPacketEvent(pkt, nc);
            postEvent(ev);
        } else {
            LOG.warn("{}: Drop packet because target port is " +
                     "uncontrollable: {}", containerName, nc);
        }
    }

    /**
     * Send a raw packet data to the specified node connector.
     *
     * <p>
     *   This method is provided only for {@link RawPacketEvent}.
     * </p>
     *
     * @param data  A raw packet data.
     * @param nc    A node connector associated with a switch port where the
     *              packet should be sent.
     */
    public void transmit(byte[] data, NodeConnector nc) {
        Node node = nc.getNode();
        ConnectionLocality cl = connectionManager.getLocalityStatus(node);
        if (cl == ConnectionLocality.LOCAL) {
            if (disabledNodes.containsKey(node)) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: Don't send packet to disabled node: {}",
                              containerName, node);
                }
                return;
            }

            RawPacket pkt;
            try {
                pkt = new RawPacket(data);
            } catch (Exception e) {
                LOG.error("{}: Failed to instantiate raw packet: port={}",
                          containerName, nc);
                return;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Accept raw packet event: port={}",
                          containerName, nc);
            }

            pkt.setOutgoingNodeConnector(nc);
            dataPacketService.transmitDataPacket(pkt);
        } else if (LOG.isTraceEnabled()) {
            LOG.trace("{}: Ignore raw packet event: {}", containerName, nc);
        }
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
        if (addr instanceof Inet4Address) {
            return createArpRequest(dst, addr.getAddress(), vlan);
        }
        return null;
    }

    /**
     * Create an ethernet frame which contains an ARP request message.
     *
     * <p>
     *   Controller's MAC address is used as source MAC adddress, and zero
     *   is used as sender protocol address.
     * </p>
     *
     * @param dst     Destination MAC address.
     * @param target  Target IP address.
     * @param vlan    VLAN ID. Zero means VLAN tag should not be added.
     * @return  An ethernet frame.
     */
    public Ethernet createArpRequest(byte[] dst, byte[] target, short vlan) {
        // Set controller's MAC address to source MAC address.
        byte[] src = switchManager.getControllerMAC();

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
            sender.length == IPV4_ADDRLEN && target.length == IPV4_ADDRLEN;

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
            // Add a VLAN tag.
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
     * Save a set of virtual tenant names.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @return  "Success" or failure reason.
     */
    private Status saveTenantNamesLocked() {
        HashSet<String> nameSet = new HashSet<String>(tenantDB.keySet());
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
            LOG.debug("{}: Save tenant names: {}", containerName,
                      tenantListFileName);
            return status;
        }

        String msg = "Failed to save tenant names";
        LOG.error("{}: {}", msg, status);
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Save virtual tenant configuration, and apply current configuration to
     * the VTN Manager.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param tenantName  The name of the virtual tenant.
     */
    public void saveTenantConfig(String tenantName) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            saveTenantConfigLocked(tenantName);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Save virtual tenant configuration, and apply current configuration to
     * the VTN Manager.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @param tenantName  The name of the virtual tenant.
     */
    private void saveTenantConfigLocked(String tenantName) {
        VTenantImpl vtn = tenantDB.get(tenantName);
        if (vtn != null) {
            vtn.saveConfig(this);
        }
    }

    /**
     * Wait for cluster caches to be initialized by another controller in
     * the cluster.
     *
     * @param path    A pseudo tenant path used for synchronization of
     *                cluster cache initialization.
     * @param myaddr  IP address of this controller in the cluster.
     * @return  {@code true} is returned if this controller should load the
     *          VTN configuration into cluster caches.
     *          {@code false} is returned if cluster caches was initialized
     *          by another controller in the cluster.
     */
    private boolean waitForCache(VTenantPath path, InetAddress myaddr) {
        ObjectPair<InetAddress, Boolean> self =
            new ObjectPair<InetAddress, Boolean>(myaddr, Boolean.FALSE);
        long current = System.currentTimeMillis();
        long limit = current + (long)vtnConfig.getCacheInitTimeout();
        InetAddress provider = null;

        // Try to associated a null tenant name with a pair of controller
        // address and Boolean objects in the state DB.
        ObjectPair<InetAddress, Boolean> another;
        while ((another = (ObjectPair<InetAddress, Boolean>)
                stateDB.putIfAbsent(path, self)) != null) {
            Boolean loaded = another.getRight();
            if (loaded.booleanValue()) {
                // The VTN configuration was loaded by another controller
                // in the cluster.
                if (LOG.isDebugEnabled()) {
                    InetAddress remote = another.getLeft();
                    LOG.debug("{}: Cluster cache was initialized by {}",
                              containerName, remote.getHostAddress());
                }
                return false;
            }

            InetAddress remote = another.getLeft();
            if (!resourceManager.isRemoteClusterAddress(remote)) {
                // Another controller in the cluster stopped while it was
                // loading the VTN configuration.
                LOG.warn("{}: Configuration provider seems dead: {}",
                         containerName, remote.getHostAddress());

                // Try to become configuration provider.
                if (stateDB.replace(path, another, self)) {
                    break;
                }
                continue;
            }

            if (current >= limit) {
                LOG.warn("{}: Cluster cache initialization did not complete" +
                         ": {}", containerName, remote.getHostAddress());
                return true;
            }

            // Wait for completion of initialization by polling.
            if (LOG.isDebugEnabled() && !remote.equals(provider)) {
                LOG.debug("{}: Wait for {} to initialize cluster caches",
                          containerName, remote.getHostAddress());
                provider = remote;
            }

            try {
                Thread.sleep(CACHE_INIT_POLLTIME);
                current = System.currentTimeMillis();
            } catch (InterruptedException e) {
                LOG.warn(containerName + ": Interrupted", e);

                // One more check should be done.
                limit = current;
            }
        }

        LOG.debug("{}: Became configuration provider", containerName);
        return true;
    }

    /**
     * Load all virtual tenant configurations.
     */
    private void loadConfig() {
        // Use a null tenant path for synchronization of cluster cache
        // initialization.
        VTenantPath path = new VTenantPath(null);
        InetAddress myaddr = resourceManager.getControllerAddress();
        if (waitForCache(path, myaddr)) {
            // Read tenant names.
            ObjectReader rdr = new ObjectReader();
            HashSet<String> nameSet = (HashSet<String>)
                rdr.read(this, tenantListFileName);
            if (nameSet != null) {
                for (String name: nameSet) {
                    loadTenantConfig(name);
                }
            }

            // Notify controllers in the cluster of completion of cluster
            // cache initialization.
            ObjectPair<InetAddress, Boolean> self =
                new ObjectPair<InetAddress, Boolean>(myaddr, Boolean.TRUE);
            stateDB.put(path, self);
        } else {
            Set<String> names = new HashSet<String>();
            for (VTenantImpl vtn: tenantDB.values()) {
                // Resume VTN configuration in the cluster cache.
                vtn.resume(this);
                names.add(vtn.getName());

                // Save configuration for this tenant.
                vtn.saveConfig(null);
            }

            // Remove configuration files for obsolete tenants.
            cleanUpConfigFile(containerName, names);

            // Save a list of tenant names.
            saveTenantNames(names);
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
        VTenantImpl newvtn = (VTenantImpl)rdr.read(this, path);
        if (newvtn != null) {
            VTenantImpl vtn = tenantDB.putIfAbsent(tenantName, newvtn);
            if (vtn == null) {
                LOG.info("{}: Tenant was loaded: {}", containerName,
                         newvtn.getVTenant());
                vtn = newvtn;
            }
            vtn.resume(this);
        }
    }

    /**
     * Check whether the VTN configuration can be updated or not.
     *
     * <p>
     *   This method must be called with holding {@link #rwLock}.
     * </p>
     *
     * @throws VTNException   VTN configuration can not be updated.
     */
    private void checkUpdate() throws VTNException {
        if (inContainerMode) {
            throw new VTNException(StatusCode.NOTACCEPTABLE,
                                   "VTN is disabled by container mode");
        }
        checkService();
    }

    /**
     * Ensure that the given tenant configuration is not null.
     *
     * @param tconf  Tenant configuration
     * @throws VTNException  {@code tconf} is {@code null}.
     */
    private void checkTenantConfig(VTenantConfig tconf) throws VTNException {
        if (tconf == null) {
            throw new VTNException(argumentIsNull("Tenant configuration"));
        }
    }

    /**
     * Check whether the VTN Manager service is available or not.
     *
     * @throws VTNException   VTN Manager service is not available.
     */
    private void checkService() throws VTNException {
        if (!serviceAvailable) {
            throw new VTNException(StatusCode.NOSERVICE,
                                   "VTN service is not available");
        }
    }

    /**
     * Remove all configuration files associated with the container, including
     * the tenant name list file.
     *
     * @param containerName  The name of the container.
     */
    static void cleanUpConfigFile(String containerName) {
        cleanUpConfigFile(containerName, null);
    }

    /**
     * Remove configuration files associated with the container.
     *
     * @param containerName  The name of the container.
     * @param validTenants   A set of valid virtual tenant names.
     *                       If a non-{@code null} value is specified, this
     *                       method removes tenant configuration files only if
     *                       its name is not contained in the given set.
     *                       In that case this method never removes the tenant
     *                       name list file even if the given set is empty.
     *                       If {@code null} is specified, all configuration
     *                       files, including the tenant name list file, are
     *                       removed.
     */
    private static void cleanUpConfigFile(String containerName,
                                          final Set<String> validTenants) {
        ConfigFileNameFilter filter =
            new ConfigFileNameFilter(containerName, validTenants);
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
     * Return a hash code of the given long integer value.
     *
     * @param value  A long integer value.
     * @return  A hash code of the given value.
     */
    public static int hashCode(long value) {
        return (int)(value ^ (value >>> NBITS_INT));
    }

    /**
     * Convert a long value which represents a MAC address into a string.
     *
     * @param mac  A long value which represents a MAC address.
     * @return  A string representation of MAC address.
     */
    public static String formatMacAddress(long mac) {
        byte[] addr = NetUtils.longToByteArray6(mac);
        return HexEncode.bytesToHexStringFormat(addr);
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
        // Currently only OpenFlow node is supported.
        checkNode(node, true);
    }

    /**
     * Check the specified node.
     *
     * @param node       Node to be tested.
     * @param checkType  {@code true} means that this method should check
     *                   whether the type of the given node is supported or
     *                   not.
     *                   {@code false} means that the caller does not care
     *                   about node type.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNode(Node node, boolean checkType)
        throws VTNException {
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

        if (checkType) {
            checkNodeType(type);
        }
    }

    /**
     * Check whether the given node type is supported by the VTN Manager.
     *
     * @param type  The node type to be tested.
     * @throws VTNException  The specified node type is not supported.
     */
    private static void checkNodeType(String type) throws VTNException {
        if (!Node.NodeIDType.OPENFLOW.equals(type)) {
            String msg = "Unsupported node type: " + type;
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
        // Currently only OpenFlow node is supported.
        checkNodeConnector(nc, true);
    }

    /**
     * Check the specified node connector.
     *
     * @param nc         Node connector to be tested.
     * @param checkType  {@code true} means that this method should check
     *                   whether the type of the given node connector is
     *                   supported or not.
     *                   {@code false} means that the caller does not care
     *                   about node connector type.
     * @throws VTNException  The specified node is invalid.
     */
    public static void checkNodeConnector(NodeConnector nc, boolean checkType)
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

        if (checkType) {
            checkNodeConnectorType(type);
        }

        checkNode(nc.getNode(), checkType);
    }

    /**
     * Check whether the given node connector type is supported by the
     * VTN Manager.
     *
     * @param type  The node connector type to be tested.
     * @throws VTNException  The specified node connector type is not
     *                       supported.
     */
    private static void checkNodeConnectorType(String type)
        throws VTNException {
        if (!NodeConnector.NodeConnectorIDType.OPENFLOW.equals(type)) {
            String msg = "Unsupported node connector type: " + type;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
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
     * Return the name of the virtual tenant in the given tenant path.
     *
     * @param path  Path to the virtual tenant.
     * @return  The name of the virtual tenant in the given path.
     * @throws VTNException  An error occurred.
     */
    private String getTenantName(VTenantPath path) throws VTNException {
        if (path == null) {
            throw new VTNException(argumentIsNull("Path"));
        }

        String tenantName = path.getTenantName();
        if (tenantName == null) {
            throw new VTNException(argumentIsNull("Tenant name"));
        }

        return tenantName;
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
    private VTenantImpl getTenantImpl(VTenantPath path) throws VTNException {
        String tenantName = getTenantName(path);
        VTenantImpl vtn = tenantDB.get(tenantName);
        if (vtn == null) {
            Status status = tenantNotFound(tenantName);
            throw new VTNException(status);
        }

        return vtn;
    }

    /**
     * Run the specified task on the VTN task thread.
     *
     * @param r  A runnable to be run on the task queue runner thread.
     */
    public void postTask(Runnable r) {
        taskQueueThread.post(r);
    }

    /**
     * Run the specified task on the VTN flow thread.
     *
     * @param task  A flow mod task to be run on the VTN flow thread.
     */
    void postFlowTask(FlowModTask task) {
        flowTaskThread.post(task);
    }

    /**
     * Run the specified task on the global async thread pool.
     *
     * @param r  A runnable to be run on the global async thread pool.
     */
    public void postAsync(Runnable r) {
        if (!resourceManager.executeAsync(r)) {
            LOG.error("{}: Async task was rejected: {}", containerName, r);
        }
    }

    /**
     * Run the specified flow mod task on the global async thread pool.
     *
     * @param task  A flow mod task to be run on the global async thread pool.
     */
    void postAsync(FlowModTask task) {
        if (!resourceManager.executeAsync(task)) {
            if (task instanceof FlowEntryTask) {
                FlowEntryTask ft = (FlowEntryTask)task;
                LOG.error("{}: FlowEntryTask was rejected: flow={}",
                          containerName, ft.getFlowEntry());
            } else {
                LOG.error("{}: {} was rejected",
                          containerName, task.getClass().getSimpleName());
            }

            if (task instanceof ClusterFlowModTask) {
                ClusterFlowModTask cft = (ClusterFlowModTask)task;
                cft.sendRemoteFlowModResult(FlowModResult.FAILED);
            }
            task.setResult(false);
        }
    }

    /**
     * Enqueue a cluster event to the event delivery queue.
     *
     * @param cev  A cluster event.
     */
    public void enqueueEvent(ClusterEvent cev) {
        synchronized (clusterEventQueue) {
            clusterEventQueue.add(cev);
        }
    }

    /**
     * Enqueue a virtual tenant event.
     *
     * @param path    Path to the tenant.
     * @param vtenant Information about the virtual tenant.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void enqueueEvent(VTenantPath path, VTenant vtenant,
                             UpdateType type) {
        VTenantEvent ev = new VTenantEvent(path, vtenant, type);
        enqueueEvent(ev);
    }

    /**
     * Post the given cluster event.
     *
     * @param cev  A cluster event.
     */
    public void postEvent(ClusterEvent cev) {
        // Call event handler for local node.
        cev.received(this, true);

        if (resourceManager.getRemoteClusterSize() > 0) {
            // Put the event to the cluser cache for event delivery.
            final ClusterEventId evid = new ClusterEventId();
            if (putEvent(evid, cev)) {
                // Create a timer task to remove cluster event object.
                TimerTask task = new TimerTask() {
                    @Override
                    public void run() {
                        removeEvent(evid);
                    }
                };

                Timer timer = resourceManager.getTimer();
                timer.schedule(task, CLUSTER_EVENT_LIFETIME);
            }
        }
    }

    /**
     * Deliver cluster events queued in the event delivery queue.
     */
    private void flushEventQueue() {
        ArrayList<ClusterEvent> list;
        synchronized (clusterEventQueue) {
            list = new ArrayList<ClusterEvent>(clusterEventQueue);
            clusterEventQueue.clear();
        }

        if (resourceManager.getRemoteClusterSize() <= 0) {
            // Only thing to do is to call event handlers for local node.
            for (ClusterEvent cev: list) {
                cev.received(this, true);
            }
        } else {
            // Call event handlers for local node.
            final ArrayList<ClusterEventId> ids =
                new ArrayList<ClusterEventId>();
            for (ClusterEvent cev: list) {
                cev.received(this, true);

                ClusterEventId evid = new ClusterEventId();
                ids.add(evid);
                clusterEvent.put(evid, cev);
            }

            // Create a timer task to remove cluster event objects.
            TimerTask task = new TimerTask() {
                @Override
                public void run() {
                    for (ClusterEventId evid: ids) {
                        removeEvent(evid);
                    }
                }
            };

            Timer timer = resourceManager.getTimer();
            timer.schedule(task, CLUSTER_EVENT_LIFETIME);
        }
    }

    /**
     * Call VTN mode listeners on the calling thread.
     *
     * @param active  {@code true} if the VTN has been activated.
     *                {@code false} if the VTN has been inactivated.
     */
    public void notifyListeners(boolean active) {
        if (LOG.isInfoEnabled()) {
            String m = (active) ? "Enter" : "Exit";
            LOG.info("{}: {} VTN mode", containerName, m);
        }

        for (IVTNModeListener listener: vtnModeListeners) {
            try {
                listener.vtnModeChanged(active);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(": Unhandled exception in mode listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
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
                notifyListeners(active);
            }
        };
        postTask(r);
    }

    /**
     * Notify the specified listener of the VTN mode change.
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
        postTask(r);
    }

    /**
     * Call virtual tenant listeners on the calling thread.
     *
     * @param path    Path to the tenant.
     * @param vtenant Information about the virtual tenant.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyListeners(VTenantPath path, VTenant vtenant,
                                UpdateType type) {
        LOG.info("{}:{}: Tenant {}: {}", containerName, path, type.getName(),
                 vtenant);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.vtnChanged(path, vtenant, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(": Unhandled exception in tenant listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
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
                notifyListeners(path, vtenant, type);
            }
        };
        postTask(r);
    }

    /**
     * Notify the specified listener of the virtual tenant changes.
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
        postTask(r);
    }

    /**
     * Call virtual L2 bridge listeners on the calling thread.
     *
     * @param path     Path to the bridge.
     * @param vbridge  Information about the virtual L2 bridge.
     * @param type     {@code ADDED} if added.
     *                 {@code REMOVED} if removed.
     *                 {@code CHANGED} if changed.
     */
    public void notifyListeners(VBridgePath path, VBridge vbridge,
                                UpdateType type) {
        LOG.info("{}:{}: Bridge {}: {}", containerName, path, type.getName(),
                 vbridge);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.vBridgeChanged(path, vbridge, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(": Unhandled exception in bridge listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
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
                notifyListeners(path, vbridge, type);
            }
        };
        postTask(r);
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
        postTask(r);
    }

    /**
     * Call virtual L2 bridge interface listeners on the calling thread.
     *
     * @param path    Path to the interface.
     * @param viface  Information about the virtual interface.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyListeners(VBridgeIfPath path, VInterface viface,
                                UpdateType type) {
        LOG.info("{}:{}: Bridge interface {}: {}", containerName, path,
                 type.getName(), viface);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.vBridgeInterfaceChanged(path, viface, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append
                    (": Unhandled exception in bridge interface listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
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
                notifyListeners(path, viface, type);
            }
        };
        postTask(r);
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
        postTask(r);
    }

    /**
     * Call VLAN mapping listeners on the calling thread.
     *
     * @param path   Path to the bridge associated with the VLAN mapping.
     * @param vlmap  Information about the VLAN mapping.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     */
    public void notifyListeners(VBridgePath path, VlanMap vlmap,
                                UpdateType type) {
        LOG.info("{}:{}: VLAN mapping {}: {}", containerName, path,
                 type.getName(), vlmap);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.vlanMapChanged(path, vlmap, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append
                    (": Unhandled exception in VLAN mapping listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
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
                notifyListeners(path, vlmap, type);
            }
        };
        postTask(r);
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
        postTask(r);
    }

    /**
     * Call port mapping listeners on the calling thread.
     *
     * @param path  Path to the bridge interface.
     * @param pmap  Information about the port mapping.
     * @param type  {@code ADDED} if added.
     *              {@code REMOVED} if removed.
     */
    public void notifyListeners(VBridgeIfPath path, PortMap pmap,
                                UpdateType type) {
        LOG.info("{}:{}: Port mapping {}: {}", containerName, path,
                 type.getName(), pmap);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.portMapChanged(path, pmap, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append
                    (": Unhandled exception in port mapping listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
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
                notifyListeners(path, pmap, type);
            }
        };
        postTask(r);
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
        postTask(r);
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
                    try {
                        listener.hostListener(host);
                    } catch (Exception e) {
                        StringBuilder builder =
                            new StringBuilder(containerName);
                        builder.append
                            (": Unhandled exception in host listener: ").
                            append(listener).append(": ").append(e.toString());
                        LOG.error(builder.toString(), e);
                    }
                }
            }
        };
        postTask(r);
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
            unlock(rdlock);
        }
    }

    /**
     * Called when a virtual tenant was added, removed, or changed by
     * remote cluster node.
     *
     * @param path  Path to the virtual tenant.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void updateTenant(final VTenantPath path, final UpdateType type) {
        String tenantName = path.getTenantName();

        if (type == UpdateType.CHANGED) {
            // Save the tenant configuration specified by the tenant name.
            saveTenantConfig(tenantName);
            return;
        }

        Lock wrlock = rwLock.readLock();
        wrlock.lock();
        try {
            if (type == UpdateType.ADDED) {
                // Create flow database for a new tenant.
                createTenantFlowDB(tenantName);

                // Save configurations, and update VTN mode.
                saveTenantNamesLocked();
                saveTenantConfigLocked(tenantName);
                updateVTNMode(true);
            } else {
                // Remove flow database.
                // Flow entries in this tenant is purged by event originator.
                removeTenantFlowDB(tenantName);

                // Delete the virtual tenant configuration file.
                VTenantImpl.deleteConfigFile(containerName, tenantName);

                // Save tenant names, and update VTN mode.
                saveTenantNamesLocked();
                updateVTNMode(false);

                // Purge canceled timer tasks.
                resourceManager.getTimer().purge();
            }
        } finally {
            unlock(wrlock);
        }
    }

    /**
     * Change VTN mode if needed.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param sync  If {@code true} is specified, mode listeners are invoked
     *              on the calling thread.
     *              If {@code false} is specified, mode listeners are invoked
     *              on the VTN task thread.
     */
    private void updateVTNMode(boolean sync) {
        if (!inContainerMode && !tenantDB.isEmpty()) {
            // Stop ARP handler emulator, and activate VTN.
            if (arpHandler != null) {
                arpHandler.destroy();
                arpHandler = null;
                if (sync) {
                    notifyListeners(true);
                } else {
                    notifyChange(true);
                }
            }
        } else if (arpHandler == null) {
            // Inactivate VTN, and start ARP handler emulator.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }
            arpHandler = new ArpHandler(this);
            if (sync) {
                notifyListeners(false);
            } else {
                notifyChange(false);
            }
        }
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
                if (disabledNodes.remove(node) != null &&
                    LOG.isDebugEnabled()) {
                    LOG.debug("{}: {}: Start packet service",
                              containerName, node);
                }
            }
        };

        if (disabledNodes.putIfAbsent(node, task) == null) {
            Timer timer = resourceManager.getTimer();
            timer.schedule(task, edgeWait);
        }
    }

    /**
     * Remove all nodes in {@link #disabledNodes}.
     *
     * <p>
     *   This method is only for testing.
     * </p>
     */
    void clearDisabledNode() {
        for (Iterator<TimerTask> it = disabledNodes.values().iterator();
             it.hasNext();) {
            TimerTask task = it.next();
            task.cancel();
            it.remove();
        }
    }

    /**
     * Flush pending cluster events, and then unlock the given lock object.
     *
     * @param lock  A lock object.
     */
    private void unlock(Lock lock) {
        unlock(lock, false);
    }

    /**
     * Flush pending cluster events, and then unlock the given lock object.
     *
     * @param lock    A lock object.
     * @param update  {@code true} means that the VTN mode should be updated.
     */
    void unlock(Lock lock, boolean update) {
        try {
            flushEventQueue();
            if (update) {
                updateVTNMode(false);
            }
        } finally {
            lock.unlock();
        }
    }

    /**
     * Put the given cluster event to the cluster event cache.
     *
     * @param evid  An identifier of the given event.
     * @param cev   An cluster event.
     * @return  {@code true} is returned on success.
     *          Otherwise {@code false} is returned.
     */
    private boolean putEvent(ClusterEventId evid, ClusterEvent cev) {
        try {
            clusterEvent.put(evid, cev);
            return true;
        } catch (Exception e) {
            LOG.error(containerName + ": Failed to put cluster event: id=" +
                      evid + " event=" + cev, e);
        }

        return false;
    }

    /**
     * Remove the given cluster event from the cluster event cache.
     *
     * @param evid  A cluster event ID.
     */
    private void removeEvent(ClusterEventId evid) {
        try {
            clusterEvent.remove(evid);
        } catch (Exception e) {
            LOG.error(containerName + ": Failed to remove cluster event: id=" +
                      evid, e);
        }
    }

    /**
     * Remove the given MAC address table entry from the cluster cache.
     *
     * @param id  An identifier of a MAC address table entry.
     */
    private void removeMacAddress(MacTableEntryId id) {
        try {
            macAddressDB.remove(id);
        } catch (Exception e) {
            LOG.error(containerName +
                      ": Failed to remove MAC address table entry: " + id, e);
        }
    }

    /**
     * Put a new MAC address table entry to the cluster cache.
     *
     * <p>
     *   Actual work of this method is executed on the VTN task thread.
     * </p>
     *
     * @param tent  A MAC address table entry.
     */
    void putMacTableEntry(final MacTableEntry tent) {
        if (macAddressDB != null) {
            Runnable r = new Runnable() {
                @Override
                public void run() {
                    MacTableEntryId id = tent.getEntryId();
                    while (macAddressDB.putIfAbsent(id, tent) != null) {
                        // Reassign entry ID.
                        id = tent.reassignEntryId();
                    }
                }
            };
            postTask(r);
        }
    }

    /**
     * Update the given MAC address table entry in the cluster cache.
     *
     * <p>
     *   Actual work of this method is executed on the VTN task thread.
     * </p>
     *
     * @param tent  A MAC address table entry.
     */
    void updateMacTableEntry(final MacTableEntry tent) {
        if (macAddressDB != null) {
            Runnable r = new Runnable() {
                @Override
                public void run() {
                    MacTableEntryId id = tent.getEntryId();
                    macAddressDB.put(id, tent);
                }
            };
            postTask(r);
        }
    }

    /**
     * Remove the specified MAC address table entry from the cluster cache.
     *
     * <p>
     *   Actual work of this method is executed on the VTN task thread.
     * </p>
     *
     * @param id  An identifier of MAC address table entry to be removed.
     */
    void removeMacTableEntry(final MacTableEntryId id) {
        if (macAddressDB != null) {
            Runnable r = new Runnable() {
                @Override
                public void run() {
                    removeMacAddress(id);
                }
            };
            postTask(r);
        }
    }

    /**
     * Remove MAC address table entries from the cluster cache.
     *
     * <p>
     *   Actual work of this method is executed on the VTN task thread.
     * </p>
     *
     * @param idSet  A set of MAC address table entry IDs to be removed.
     */
    void removeMacTableEntries(final Set<MacTableEntryId> idSet) {
        if (macAddressDB != null) {
            Runnable r = new Runnable() {
                @Override
                public void run() {
                    removeMacTableEntriesSync(idSet);
                }
            };
            postTask(r);
        }
    }

    /**
     * Remove MAC address table entries from the cluster cache.
     *
     * <p>
     *   This method removes entries from the cache on the calling thread.
     *   So this method shoule be called without holding any lock to avoid
     *   deadlock with InfiniSpan.
     * </p>
     *
     * @param idSet  A set of MAC address table entry IDs to be removed.
     */
    void removeMacTableEntriesSync(Set<MacTableEntryId> idSet) {
        if (macAddressDB != null) {
            for (MacTableEntryId id: idSet) {
                removeMacAddress(id);
            }
        }
    }

    /**
     * Invoked when at least one controller is added to or removed from the
     * cluster.
     *
     * @param added    A set of controller IP addresses added to the cluster.
     * @param removed  A set of controller IP addresses removed from the
     *                 cluster.
     */
    void clusterNodeChanged(final Set<InetAddress> added,
                            final Set<InetAddress> removed) {
        if (clusterService.amICoordinator()) {
            // Flush MAC address table entries created by removed cluster
            // nodes.
            Runnable r = new Runnable() {
                @Override
                public void run() {
                    for (MacAddressTable table: macTableMap.values()) {
                        table.flush(removed);
                    }
                }
            };
            postTask(r);
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
    public boolean isActive() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return (arpHandler == null);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return a list of virtual tenant configurations.
     *
     * @return  A list which contains tenant configurations.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VTenant> getTenants() throws VTNException {
        // Sort tenants by their name.
        TreeMap<String, VTenantImpl> tree;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            tree = new TreeMap(tenantDB);
        } finally {
            rdlock.unlock();
        }

        ArrayList<VTenant> list = new ArrayList<VTenant>();
        for (VTenantImpl vtn: tree.values()) {
            list.add(vtn.getVTenant());
        }
        list.trimToSize();

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
            unlock(rdlock);
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
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            String tenantName = getTenantName(path);
            checkTenantConfig(tconf);
            checkUpdate();

            // Ensure the given tenant name is valid.
            checkName("Tenant", tenantName);

            VTenantImpl vtn = new VTenantImpl(containerName, tenantName,
                                              tconf);
            if (tenantDB.putIfAbsent(tenantName, vtn) != null) {
                String msg = tenantName + ": Tenant name already exists";
                return new Status(StatusCode.CONFLICT, msg);
            }

            // Create a VTN flow database.
            createTenantFlowDB(tenantName);

            Status status = saveTenantNamesLocked();
            Status stConf = vtn.saveConfig(null);
            if (status.isSuccess()) {
                status = stConf;
            }

            VTenant vtenant = vtn.getVTenant();
            updateVTNMode(false);
            enqueueEvent(path, vtenant, UpdateType.ADDED);

            return status;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkTenantConfig(tconf);
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            if (!vtn.setVTenantConfig(this, path, tconf, all)) {
                return new Status(StatusCode.SUCCESS, "Not modified");
            }

            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove a tenant specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeTenant(VTenantPath path) {
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            String tenantName = getTenantName(path);
            checkUpdate();

            // Make the specified tenant invisible.
            VTenantImpl vtn = tenantDB.remove(tenantName);
            if (vtn == null) {
                return tenantNotFound(tenantName);
            }

            VTenant vtenant = vtn.getVTenant();
            vtn.destroy(this);

            // Purge all VTN flows in this tenant.
            VTNFlowDatabase fdb = removeTenantFlowDB(tenantName);
            if (fdb != null) {
                VTNThreadData.removeFlows(this, fdb);
            }

            Status status = saveTenantNamesLocked();
            data.setModeChanged();
            enqueueEvent(path, vtenant, UpdateType.REMOVED);

            return status;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
            unlock(rdlock);
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
            unlock(rdlock);
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.addBridge(this, path, bconf);
            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            if (!vtn.modifyBridge(this, path, bconf, all)) {
                return new Status(StatusCode.SUCCESS, "Not modified");
            }

            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the virtual L2 bridge specified by the given name.
     *
     * @param path  Path to the virtual bridge.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeBridge(VBridgePath path) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeBridge(this, path);
            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
            unlock(rdlock);
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
            unlock(rdlock);
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.addBridgeInterface(this, path, iconf);
            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            if (!vtn.modifyBridgeInterface(this, path, iconf, all)) {
                return new Status(StatusCode.SUCCESS, "Not modified");
            }

            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the virtual interface from the virtual L2 bridge.
     *
     * @param path  Path to the interface.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeBridgeInterface(VBridgeIfPath path) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeBridgeInterface(this, path);
            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
            unlock(rdlock);
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
            unlock(rdlock);
        }
    }

    /**
     * Return information about the VLAN mapping which matches the specified
     * VLAN mapping configuration in the specified virtual L2 bridge.
     *
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about the VLAN mapping which matches the specified
     *          VLAN mapping configuration.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VlanMap getVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getVlanMap(path, vlconf);
        } finally {
            unlock(rdlock);
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            VlanMap vlmap = vtn.addVlanMap(this, path, vlconf);
            tenantDB.put(vtn.getName(), vtn);
            Status status = vtn.saveConfig(null);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }
            return vlmap;
        } finally {
            data.cleanUp(this);
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeVlanMap(this, path, mapId);
            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
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
            unlock(rdlock);
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.setPortMap(this, path, pmconf);
            tenantDB.put(vtn.getName(), vtn);
            return vtn.saveConfig(null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
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
        byte[] bcast = NetUtils.getBroadcastMACAddr();
        Ethernet ether = createArpRequest(bcast, addr);
        if (ether == null) {
            return;
        }

        PacketContext pctx = new PacketContext(ether, null);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (!serviceAvailable) {
                return;
            }

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
            unlock(rdlock);
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
            checkService();
            checkNodeConnector(nc);
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.debug("{}: probeHost: Ignore request for {}: {}",
                      containerName, host, status.getDescription());
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
            if (islDB.containsKey(nc)) {
                LOG.debug("{}: probeHost: Ignore request for {}: " +
                          "Internal port", containerName, host);
                return false;
            }

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
            unlock(rdlock);
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
    @Override
    public List<MacAddressEntry> getMacEntries(VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacEntries(this, path);
        } finally {
            unlock(rdlock);
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
    @Override
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacEntry(this, path, addr);
        } finally {
            unlock(rdlock);
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
    @Override
    public MacAddressEntry removeMacEntry(VBridgePath path,
                                          DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeMacEntry(this, path, addr);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Flush all MAC address table entries in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status flushMacEntries(VBridgePath path) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.flushMacEntries(this, path);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            unlock(rdlock);
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    // IVTNFlowDebugger

    /**
     * Remove all flow entries in the specified virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeAllFlows(VTenantPath path) {
        FlowRemoveTask task;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            String tenantName = getTenantName(path);
            checkUpdate();

            VTNFlowDatabase fdb = getTenantFlowDB(tenantName);
            if (fdb == null) {
                return tenantNotFound(tenantName);
            }

            if (LOG.isDebugEnabled()) {
                LOG.debug("{}:{}: Clear flow entries", containerName, path);
            }
            task = fdb.clear(this);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            rdlock.unlock();
        }

        Status status;
        if (task != null) {
            // Wait for completion of flow remove task.
            FlowModResult result = task.getResult();
            status = result.toStatus();
        } else {
            status = new Status(StatusCode.SUCCESS, null);
        }

        return status;
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
    public void entryCreated(ClusterEventId key, String cacheName,
                             boolean originLocal) {
    }

    /**
     * Called anytime a given entry is updated.
     *
     * @param key         Key for the entry modified.
     * @param newValue    The new value the key will have.
     * @param cacheName   Name of the cache for which update has been received.
     * @param originLocal {@code true} if the event is generated from this
     *                    node.
     */
    @Override
    public void entryUpdated(ClusterEventId key, Object newValue,
                             String cacheName, boolean originLocal) {
        if (originLocal) {
            return;
        }

        if (key.isLocal()) {
            LOG.debug("{}: Local key is updated, but originLocal is false: " +
                      "key={}, value={}", containerName, key, newValue);
            return;
        }

        if (CACHE_EVENT.equals(cacheName)) {
            if (!(newValue instanceof ClusterEvent)) {
                LOG.error("{}: Unexpected value in cluster event cache: " +
                          "key={}, value={}", containerName, key, newValue);
                return;
            }

            ClusterEvent cev = (ClusterEvent)newValue;
            if (LOG.isTraceEnabled()) {
                cev.traceLog(this, LOG, key);
            }
            cev.received(this, false);
        } else if (CACHE_FLOWS.equals(cacheName)) {
            if (!(key instanceof FlowGroupId)) {
                LOG.error("{}: Unexpected key in flow DB: key={}, value={}",
                          containerName, key, newValue);
                return;
            }
            if (!(newValue instanceof VTNFlow)) {
                LOG.error("{}: Unexpected value in flow DB: key={}, value={}",
                          containerName, key, newValue);
                return;
            }

            FlowGroupId gid = (FlowGroupId)key;
            VTNFlowDatabase fdb = getTenantFlowDB(gid);
            if (fdb == null) {
                LOG.error("{}: update: Flow database was not found: group={}",
                          containerName, gid);
                return;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Received new VTN flow: group={}",
                          containerName, gid);
            }

            // Update indices for a new VTN flow.
            fdb.createIndex(this, (VTNFlow)newValue);
        } else if (CACHE_MAC.equals(cacheName)) {
            if (!(key instanceof MacTableEntryId)) {
                LOG.error("{}: Unexpected key in MAC address DB: key={}, " +
                          "value={}", containerName, key, newValue);
                return;
            }
            if (!(newValue instanceof MacTableEntry)) {
                LOG.error("{}: Unexpected value in MAC address DB: key={}, " +
                          "value={}", containerName, key, newValue);
                return;
            }

            MacTableEntryId id = (MacTableEntryId)key;
            MacAddressTable table = getMacAddressTable(id);
            if (table != null) {
                table.entryUpdated((MacTableEntry)newValue);
            } else {
                LOG.error("{}: MAC address table was not found: " +
                          "id={}, value={}", containerName, id, newValue);
            }
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
    public void entryDeleted(ClusterEventId key, String cacheName,
                             boolean originLocal) {
        if (originLocal) {
            return;
        }

        if (CACHE_FLOWS.equals(cacheName)) {
            if (!(key instanceof FlowGroupId)) {
                LOG.error("{}: Unexpected key in the flow DB: {}",
                          containerName, key);
                return;
            }

            FlowGroupId gid = (FlowGroupId)key;
            VTNFlowDatabase fdb = getTenantFlowDB(gid);
            if (fdb == null) {
                LOG.debug("{}: Flow database is already removed: group={}",
                          containerName, gid);
                return;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Received removed VTN flow group name: {}",
                          containerName, gid);
            }
            fdb.flowRemoved(this, gid);
        } else if (CACHE_MAC.equals(cacheName)) {
            if (!(key instanceof MacTableEntryId)) {
                LOG.error("{}: Unexpected key in MAC address DB: {}",
                          containerName, key);
                return;
            }

            MacTableEntryId id = (MacTableEntryId)key;
            MacAddressTable table = getMacAddressTable(id);
            if (table != null) {
                table.entryDeleted(id);
            } else if (LOG.isDebugEnabled()) {
                LOG.debug("{}: MAC address table is already removed: {}",
                          containerName, id);
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
        HashSet<String> nameSet = new HashSet<String>();
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            Status failure = null;
            for (VTenantImpl vtn: tenantDB.values()) {
                nameSet.add(vtn.getName());

                Status st = vtn.saveConfig(null);
                if (!st.isSuccess()) {
                    failure = st;
                }
            }

            Status status = saveTenantNames(nameSet);
            if (failure != null) {
                status = failure;
            }

            return status;
        } finally {
            unlock(rdlock);
        }
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

        // Maintain the node DB.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (type == UpdateType.ADDED) {
                if (connectionManager.getLocalityStatus(node) ==
                    ConnectionLocality.LOCAL) {
                    // Remove all VTN flows related to this node because
                    // all flow entries in this node should be removed by
                    // protocol plugin.
                    for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                        fdb.removeFlows(this, node, true);
                    }
                }

                if (!addNode(node)) {
                    return;
                }
            } else {
                assert type == UpdateType.REMOVED;
                if (!removeNode(node)) {
                    return;
                }
            }
        } finally {
            wrlock.unlock();
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNode(this, node, type);
            }
        } finally {
            unlock(rdlock);
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
        // Maintain the port DB.
        UpdateType utype = type;
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (type == UpdateType.REMOVED) {
                if (!removePort(nc)) {
                    return;
                }
            } else {
                utype = addPort(nc, propMap);
                if (utype == null) {
                    return;
                }
            }
        } finally {
            wrlock.unlock();
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNodeConnector(this, nc, utype);
            }
        } finally {
            unlock(rdlock);
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
        // Maintain the inter switch link DB.
        if (!updateISL(topoList)) {
            return;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.edgeUpdate(this, topoList);
            }
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Called when an Edge utilization is above the safety threshold configured
     * on the controller.
     *
     * @param edge  The edge which bandwidth usage is above the safety level
     */
    @Override
    public void edgeOverUtilized(Edge edge) {
    }

    /**
     * Called when the Edge utilization is back to normal, below the safety
     * threshold level configured on the controller.
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

        Lock wrlock = rwLock.readLock();
        wrlock.lock();
        try {
            inContainerMode = mode;
        } finally {
            unlock(wrlock, true);
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
            // Here we need to accept non-OpenFlow packet.
            // Node connector type check should be done only if the VTN is
            // active.
            checkNodeConnector(nc, false);
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.error("{}: Ignore packet: {}", containerName,
                      status.getDescription());
            return PacketResult.IGNORED;
        }

        Node node = nc.getNode();
        if (disabledNodes.containsKey(node)) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Ignore packet from disabled node: {}",
                          containerName, node);
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

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (arpHandler != null) {
                // No VTN. Pass packet to the ARP handler emulator.
                return arpHandler.receive(pctx);
            }

            // Ensure that the packet was sent by an OpenFlow node. */
            checkNodeType(node.getType());
            checkNodeConnectorType(nc.getType());

            if (islDB.containsKey(nc)) {
                LOG.debug("{}: Ignore packet from internal node connector: {}",
                          containerName, nc);
                if (connectionManager.getLocalityStatus(node) ==
                    ConnectionLocality.LOCAL) {
                    // This PACKET_IN may be caused by a obsolete flow entry.
                    // So all flow entries related to this port should be
                    // removed.
                    for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                        fdb.removeFlows(this, nc);
                    }
                }
                return PacketResult.IGNORED;
            }

            // Port mappings should precede VLAN mappings.
            LOOP:
            for (MapType type: MapType.values()) {
                if (type == MapType.ALL) {
                    break;
                }

                for (VTenantImpl vtn: tenantDB.values()) {
                    res = vtn.receive(this, type, pctx);
                    if (res != PacketResult.IGNORED) {
                        break LOOP;
                    }
                }
            }
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.error("{}: Ignore packet: {}", containerName,
                      status.getDescription());
            return PacketResult.IGNORED;
        } finally {
            unlock(rdlock);
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
            unlock(rdlock);
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
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (arpHandler == null) {
                findHost(networkAddress, null);
            } else {
                arpHandler.find(networkAddress);
            }
        } finally {
            unlock(rdlock);
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
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (arpHandler == null) {
                probeHost(host);
            } else {
                arpHandler.probe(host);
            }
        } finally {
            unlock(rdlock);
        }
    }

    // IFlowProgrammerListener

    /**
     * Invoked when a SAL flow has expired.
     *
     * @param node  The network node on which the flow got removed.
     * @param flow  The flow that got removed.
     *              Note: It may contain only the Match and flow parameters
     *              fields. Actions may not be present.
     */
    @Override
    public void flowRemoved(Node node, Flow flow) {
        VTNFlowDatabase found = null;
        String empty = "";
        FlowEntry entry = new FlowEntry(empty, empty, flow, node);
        for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
            if (fdb.containsIngressFlow(entry)) {
                found = fdb;
                break;
            }
        }
        if (found != null) {
            found.flowRemoved(this, entry);
        }
    }

    /**
     * Invoked when an error message has been received from a switch.
     *
     * @param node  The network node on which the error reported.
     * @param rid   The offending message request ID.
     * @param err   The error message.
     */
    @Override
    public void flowErrorReported(Node node, long rid, Object err) {
    }
}
