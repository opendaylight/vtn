/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.Dictionary;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.IVTNFlowDebugger;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapAclType;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.UpdateOperation;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilterId;

import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.ContainerPathMapEvent;
import org.opendaylight.vtn.manager.internal.cluster.ContainerPathMapImpl;
import org.opendaylight.vtn.manager.internal.cluster.FlowCondImpl;
import org.opendaylight.vtn.manager.internal.cluster.FlowConditionEvent;
import org.opendaylight.vtn.manager.internal.cluster.FlowFilterMap;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.FlowMatchImpl;
import org.opendaylight.vtn.manager.internal.cluster.FlowModResult;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PathPolicyEvent;
import org.opendaylight.vtn.manager.internal.cluster.PathPolicyImpl;
import org.opendaylight.vtn.manager.internal.cluster.PortProperty;
import org.opendaylight.vtn.manager.internal.cluster.RawPacketEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

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
import org.opendaylight.controller.sal.core.ContainerFlow;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.IContainerListener;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.flowprogrammer.IFlowProgrammerListener;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IDataPacketService;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IListenDataPacket;
import org.opendaylight.controller.sal.packet.LLDP;
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
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.statisticsmanager.IStatisticsManager;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.controller.topologymanager.ITopologyManagerAware;

/**
 * Implementation of VTN Manager service.
 */
public class VTNManagerImpl
    implements IVTNManager, IVTNFlowDebugger, RouteResolver,
               ICacheUpdateAware<ClusterEventId, Object>,
               IConfigurationContainerAware, IInventoryListener,
               ITopologyManagerAware, IContainerListener, IListenDataPacket,
               IListenRoutingUpdates, IHostFinder, IFlowProgrammerListener {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VTNManagerImpl.class);

    /**
     * Maximum lifetime, in milliseconds, of a cluster event.
     */
    private static final long CLUSTER_EVENT_LIFETIME = 1000L;

    /**
     * The number of bytes in an IPv4 address.
     */
    static final int  IPV4_ADDRLEN = 4;

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
     * The name of the cluster cache which keeps flow conditions.
     */
    static final String CACHE_FLOWCOND = "vtn.flowcond";

    /**
     * The name of the cluster cache which keeps path policies.
     */
    static final String CACHE_PATHPOLICY = "vtn.pathpolicy";

    /**
     * The name of the cluster cache which keeps container path maps.
     */
    static final String CACHE_PATHMAP = "vtn.pathmap";

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
     * Polling interval, in milliseconds, to wait for completion of cluster
     * cache initialization.
     */
    private static final long  CACHE_INIT_POLLTIME = 100L;

    /**
     * Default link cost used when the specified path policy does not exist.
     */
    private static final long  DEFAULT_LINK_COST = 1L;

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
     *   This map keeps pairs of {@link NodeConnector} instances corresponding
     *   to switch ports connected each other.
     * </p>
     */
    private ConcurrentMap<NodeConnector, NodeConnector>  islDB;

    /**
     * Keeps flow conditions configured in this container.
     */
    private ConcurrentMap<String, FlowCondImpl>  flowCondDB;

    /**
     * Keeps path policies configured in this container.
     */
    private ConcurrentMap<Integer, PathPolicyImpl>  pathPolicyDB;

    /**
     * Keeps container path maps configured in this container.
     */
    private ConcurrentMap<Integer, ContainerPathMapImpl>  pathMapDB;

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
     * Interface between path policy and SAL routing.
     */
    private final Map<Integer, PathPolicyMap>  pathPolicyMap =
        new HashMap<Integer, PathPolicyMap>();

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
     * Statistics manager service.
     */
    private IStatisticsManager  statisticsManager;

    /**
     * Host tracker service instance.
     */
    private IfIptoHost  hostTracker;

    /**
     * Connection manager service instance.
     */
    private IConnectionManager  connectionManager;

    /**
     * Container manager service instance.
     *
     * <p>
     *   Note that {@code null} is set unless this instance is associated with
     *   the default container.
     * </p>
     */
    private IContainerManager  containerManager;

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
            LOG.trace("Start");
            for (Runnable r = getTask(); r != null; r = getTask()) {
                try {
                    r.run();
                } catch (Exception e) {
                    LOG.error("Exception occurred on a task thread.", e);
                }
            }
            LOG.trace("Exit");
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

        LOG.trace("{}: init() called", cname);
        containerName = cname;

        // Initialize configuration directory for the container.
        ContainerConfig cfg = new ContainerConfig(cname);
        cfg.init();

        // Load static configuration.
        String root = GlobalConstants.STARTUPHOME.toString();
        vtnConfig = new VTNConfig(root, cname);

        if (containerManager != null) {
            assert containerName.equals(GlobalConstants.DEFAULT.toString());
            inContainerMode = containerManager.inContainerMode();
        } else {
            inContainerMode = false;
        }

        createCaches();

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

        // Current path policy implementation uses the global routing table
        // for max throughput. It should be initialized here because it is
        // global and it can be initialized only one time.
        PathPolicyMap ppmap =
            new PathPolicyMap(this, PathPolicyImpl.POLICY_ID);
        ppmap.register();
        pathPolicyMap.put(Integer.valueOf(PathPolicyImpl.POLICY_ID), ppmap);

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
        LOG.info("{}: VTN Manager has been started", containerName);
    }

    /**
     * Function called just before the dependency manager stops the service.
     */
    void stopping() {
        LOG.trace("{}: stopping() called", containerName);

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
        LOG.trace("{}: stop() called", containerName);
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
        LOG.trace("{}: destroy() called", containerName);
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

        LOG.info("{}: VTN Manager has been destroyed", containerName);
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
            islDB = new ConcurrentHashMap<NodeConnector, NodeConnector>();
            clusterEvent =
                new ConcurrentHashMap<ClusterEventId, ClusterEvent>();
            flowDB = new ConcurrentHashMap<FlowGroupId, VTNFlow>();
            flowCondDB = new ConcurrentHashMap<String, FlowCondImpl>();
            pathPolicyDB = new ConcurrentHashMap<Integer, PathPolicyImpl>();
            pathMapDB = new ConcurrentHashMap<Integer, ContainerPathMapImpl>();
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
        createCache(cluster, CACHE_FLOWCOND, cmode);
        createCache(cluster, CACHE_PATHPOLICY, cmode);
        createCache(cluster, CACHE_PATHMAP, cmode);

        tenantDB = (ConcurrentMap<String, VTenantImpl>)
            getCache(cluster, CACHE_TENANT);
        stateDB = (ConcurrentMap<VTenantPath, Object>)
            getCache(cluster, CACHE_STATE);
        nodeDB = (ConcurrentMap<Node, VNodeState>)
            getCache(cluster, CACHE_NODES);
        portDB = (ConcurrentMap<NodeConnector, PortProperty>)
            getCache(cluster, CACHE_PORTS);
        islDB = (ConcurrentMap<NodeConnector, NodeConnector>)
            getCache(cluster, CACHE_ISL);
        flowCondDB = (ConcurrentMap<String, FlowCondImpl>)
            getCache(cluster, CACHE_FLOWCOND);
        pathPolicyDB = (ConcurrentMap<Integer, PathPolicyImpl>)
            getCache(cluster, CACHE_PATHPOLICY);
        pathMapDB = (ConcurrentMap<Integer, ContainerPathMapImpl>)
            getCache(cluster, CACHE_PATHMAP);

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
            LOG.debug("{}: {}: Cache already exists", containerName,
                      cacheName);
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
            cluster.destroyCache(CACHE_FLOWCOND);
            cluster.destroyCache(CACHE_PATHPOLICY);
            cluster.destroyCache(CACHE_PATHMAP);
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
            if (addISL(edge, null) >= 0) {
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
                NodeUtils.checkNode(node);
            } catch (VTNException e) {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}: addNode: Ignore invalid node {}: {}",
                              containerName, node, e);
                }
                return false;
            }
        }

        if (nodeDB.putIfAbsent(node, VNodeState.UP) == null) {
            LOG.info("{}: addNode: New node {}", containerName, node);
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
            TimerTask task = disabledNodes.remove(node);
            if (task != null) {
                task.cancel();
            }

            // Clean up node connectors in the given node.
            Set<NodeConnector> ports =
                new HashSet<NodeConnector>(portDB.keySet());
            for (NodeConnector nc: ports) {
                if (node.equals(nc.getNode())) {
                    LOG.info("{}: removeNode({}): Remove port {}",
                             containerName, node, nc);
                    if (portDB.remove(nc) != null) {
                        removeISLPort(nc, null);
                    }
                }
            }

            LOG.info("{}: removeNode: Removed {}", containerName, node);

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
            NodeUtils.checkNodeConnector(nc);
        } catch (VTNException e) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addPort: Ignore invalid port {}: {}",
                          containerName, nc, e);
            }
            return null;
        }

        if (switchManager.isSpecial(nc)) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addPort: Ignore special port {}, {}",
                          containerName, nc, propMap);
            }
            return null;
        }

        PortProperty pp = new PortProperty(propMap);
        PortProperty old = portDB.putIfAbsent(nc, pp);
        if (old == null) {
            LOG.info("{}: addPort: New port: port={}, prop={}",
                     containerName, nc, pp);
            return UpdateType.ADDED;
        }

        if (!old.equals(pp)) {
            portDB.put(nc, pp);
            LOG.info("{}: addPort: Property has been changed: " +
                     "port={}, prop={} -> {}", containerName, nc, old, pp);
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
            LOG.info("{}: removePort: Removed {}", containerName, nc);
            removeISLPort(nc, null);
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
     * @param islMap    If a non-{@code null} value is specified, switch ports
     *                  that were actually changed its state are set to the
     *                  specified map. {@link Boolean#TRUE} is set if the
     *                  port associated with the key was changed to ISL port.
     *                  {@link Boolean#FALSE} is set if the port was changed
     *                  to edge port.
     * @return  {@code true} is returned if inter switch links was actually
     *          updated. {@code false} is returned if link map was not changed.
     */
    private boolean updateISL(List<TopoEdgeUpdate> topoList,
                              Map<NodeConnector, Boolean> islMap) {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            boolean changed = false;
            for (TopoEdgeUpdate topo: topoList) {
                UpdateType type = topo.getUpdateType();
                Edge edge = topo.getEdge();
                if ((type == UpdateType.ADDED && addISL(edge, islMap) > 0) ||
                    (type == UpdateType.REMOVED && removeISL(edge, islMap))) {
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
     * @param edge    A edge;
     * @param islMap  If a non-{@code null} value is specified, switch ports
     *                that were actually changed its state is set to the
     *                specified map. {@link Boolean#TRUE} is set if the
     *                port associated with the key was changed to ISL port.
     *                {@link Boolean#FALSE} is set if the port was changed
     *                to edge port.
     * @return  {@code 1} is returned if the given edge is actually added.
     *          {@code 0} is returned if the given edge already exists in
     *          the inter switch link map.
     *          {@code -1} is returned if the given edge is invalid.
     */
    private int addISL(Edge edge, Map<NodeConnector, Boolean> islMap) {
        // Ensure both ports are valid.
        NodeConnector head = edge.getHeadNodeConnector();
        NodeConnector tail = edge.getTailNodeConnector();

        try {
            NodeUtils.checkNodeConnector(head);
            NodeUtils.checkNodeConnector(tail);
        } catch (VTNException e) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: addISL: Ignore invalid edge {}: {}",
                          containerName, edge, e);
            }

            return -1;
        }

        boolean h = addISLPort(head, tail, islMap);
        boolean t = addISLPort(tail, head, islMap);
        return (h || t) ? 1 : 0;
    }

    /**
     * Remove inter switch link specified by the given edge.
     *
     * <p>
     *   This method must be called with holding writer lock of
     *   {@link #rwLock}.
     * </p>
     *
     * @param edge    A edge.
     * @param islMap  If a non-{@code null} value is specified, switch ports
     *                that were actually changed its state is set to the
     *                specified map. {@link Boolean#TRUE} is set if the
     *                port associated with the key was changed to ISL port.
     *                {@link Boolean#FALSE} is set if the port was changed
     *                to edge port.
     * @return  {@code true} is returned if the given edge is actually removed.
     *          Otherwise {@code false} is returned.
     */
    private boolean removeISL(Edge edge, Map<NodeConnector, Boolean> islMap) {
        NodeConnector head = edge.getHeadNodeConnector();
        NodeConnector tail = edge.getTailNodeConnector();
        boolean h = removeISLPort(head, islMap);
        boolean t = removeISLPort(tail, islMap);
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
     * @param nc      A {@link NodeConnector} instance corresponding to the
     *                target switch port.
     * @param peer    A {@link NodeConnector} instance corresponding to the
     *                switch port connected to {@link nc}.
     * @param islMap  If a non-{@code null} value is specified, the specified
     *                port is put to the given map only if the port was
     *                actually added to the ISL map.
     * @return  {@code true} is returned if the given node connector is
     *          actually added. {@code false} is returned if the given node
     *          connector exists in the inter switch link map.
     */
    private boolean addISLPort(NodeConnector nc, NodeConnector peer,
                               Map<NodeConnector, Boolean> islMap) {
        if (islDB.putIfAbsent(nc, peer) == null) {
            if (islMap != null) {
                islMap.put(nc, Boolean.TRUE);
            }
            LOG.info("{}: addISLPort: New ISL port {} <-> {}",
                     containerName, nc, peer);
            return true;
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: addISLPort: Ignore existing port {}",
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
     * @param nc      A node connector.
     * @param islMap  If a non-{@code null} value is specified, the specified
     *                port is put to the given map only if the port was
     *                actually removed from the ISL map.
     * @return  {@code true} is returned if the given node connector is
     *          actually removed. {@code false} is returned if the given node
     *          connector does not exist in the inter switch link map.
     */
    private boolean removeISLPort(NodeConnector nc,
                                  Map<NodeConnector, Boolean> islMap) {
        NodeConnector port = nc;
        boolean removed = false;
        while (true) {
            NodeConnector peer = islDB.remove(port);
            if (peer == null) {
                if (LOG.isDebugEnabled()) {
                    LOG.debug("{}: removeISLPort: Ignore unknown port {}",
                              containerName, port);
                }
                break;
            }

            removed = true;
            if (islMap != null) {
                islMap.put(port, Boolean.FALSE);
            }
            LOG.info("{}: removeISLPort: Removed {} <-> {}",
                     containerName, port, peer);
            port = peer;
        }

        return removed;
    }

    /**
     * Determine whether the given node connector is associated with the
     * physical switch port at the edge of the SDN network.
     *
     * <p>
     *   Note that this method does not check whether the given switch port
     *   is managed by the controller or not.
     * </p>
     *
     * @param nc  A node connector.
     * @return  {@code true} is returned only if the given node connector
     *          is associated with the physical switch port at the edge of
     *          the SDN network.
     */
    private boolean isEdgePortImpl(NodeConnector nc) {
        NodeConnector peer = islDB.get(nc);
        return (peer == null || !exists(peer));
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
        LOG.trace("{}: Set cluster service: {}", containerName, service);
        clusterService = service;
    }

    /**
     * Invoked when a cluster container service is unregistered.
     *
     * @param service  Cluster container service.
     */
    void unsetClusterContainerService(IClusterContainerServices service) {
        if (service != null && service.equals(clusterService)) {
            LOG.trace("{}: Unset cluster service: {}", containerName, service);
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
        LOG.trace("{}: Set switch manager: {}", containerName, service);
        switchManager = service;
    }

    /**
     * Invoked when a switch manager service is unregistered.
     *
     * @param service  Switch manager service.
     */
    void unsetSwitchManager(ISwitchManager service) {
        if (service != null && service.equals(switchManager)) {
            LOG.trace("{}: Unset switch manager: {}", containerName, service);
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
        LOG.trace("{}: Set topology manager: {}", containerName, service);
        topologyManager = service;
    }

    /**
     * Invoked when a topology manager service is unregistered.
     *
     * @param service  Topology manager service.
     */
    void unsetTopologyManager(ITopologyManager service) {
        if (service != null && service.equals(topologyManager)) {
            LOG.trace("{}: Unset topology manager: {}", containerName,
                      service);
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
        LOG.trace("{}: Set forwarding rule manager: {}", containerName,
                  service);
        fwRuleManager = service;
    }

    /**
     * Invoked when a forwarding rule manager service is unregistered.
     *
     * @param service  Forwarding rule manager service.
     */
    void unsetForwardingRuleManager(IForwardingRulesManager service) {
        if (service != null && service.equals(fwRuleManager)) {
            LOG.trace("{}: Unset forwarding rule manager: {}", containerName,
                      service);
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
        LOG.trace("{}: Set routing service: {}", containerName, service);
        routing = service;
    }

    /**
     * Invoked when a routing service is unregistered.
     *
     * @param service  Routing service.
     */
    void unsetRouting(IRouting service) {
        if (service != null && service.equals(routing)) {
            LOG.trace("{}: Unset routing service: {}", containerName, service);
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
        LOG.trace("{}: Set data packet service: {}", containerName, service);
        dataPacketService = service;
    }

    /**
     * Invoked when a data packet service is unregistered.
     *
     * @param service  Data packet service.
     */
    void unsetDataPacketService(IDataPacketService service) {
        if (service != null && service.equals(dataPacketService)) {
            LOG.trace("{}: Unset data packet service: {}", containerName,
                      service);
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
     * Invoked when a statistics manager service is registered.
     *
     * @param service  Statistics manager service.
     */
    void setStatisticsManager(IStatisticsManager service) {
        LOG.trace("{}: Set statistics manager service: {}", containerName,
                  service);
        statisticsManager = service;
    }

    /**
     * Invoked when a statistics manager service is unregistered.
     *
     * @param service  Statistics manager service.
     */
    void unsetStatisticsManager(IStatisticsManager service) {
        if (service != null && service.equals(statisticsManager)) {
            LOG.trace("{}: Unset statistics manager service: {}",
                      containerName, service);
            statisticsManager = null;
        }
    }

    /**
     * Return statistics manager service instance.
     *
     * @return  Statistics manager service.
     */
    public IStatisticsManager getStatisticsManager() {
        return statisticsManager;
    }

    /**
     * Invoked when a host tracker service is registered.
     *
     * @param service  Host tracker service.
     */
    void setHostTracker(IfIptoHost service) {
        LOG.trace("{}: Set host tracker service: {}", containerName, service);
        hostTracker = service;
    }

    /**
     * Invoked when a host tracker service is unregistered.
     *
     * @param service  Host tracker service.
     */
    void unsetHostTracker(IfIptoHost service) {
        if (service != null && service.equals(hostTracker)) {
            LOG.trace("{}: Unset host tracker service: {}", containerName,
                      service);
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
        LOG.trace("{}: Set connection manager service: {}", containerName,
                  service);
        connectionManager = service;
    }

    /**
     * Invoked when a connection manager service is unregistered.
     *
     * @param service  Connection manager service.
     */
    void unsetConnectionManager(IConnectionManager service) {
        if (service != null && service.equals(connectionManager)) {
            LOG.trace("{}: Unset connection manager service: {}",
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
     * Invoked when a container manager service is registered.
     *
     * @param service  Container manager service.
     */
    void setContainerManager(IContainerManager service) {
        LOG.trace("{}: Set container manager service: {}", containerName,
                  service);
        containerManager = service;
    }

    /**
     * Invoked when a container manager service is unregistered.
     *
     * @param service  Container manager service.
     */
    void unsetContainerManager(IContainerManager service) {
        if (service != null && service.equals(containerManager)) {
            LOG.trace("{}: Unset container manager service: {}",
                      containerName, service);
            containerManager = null;
        }
    }

    /**
     * Return container manager service instance.
     *
     * @return  Container manager service.
     *          Note that {@code null} is always returned unless this instance
     *          is associated with the default container.
     */
    public IContainerManager getContainerManager() {
        return containerManager;
    }

    /**
     * Invoked when a host listener is registered.
     *
     * @param service  Host listener service.
     */
    void addHostListener(IfHostListener service) {
        if (hostListeners.addIfAbsent(service)) {
            LOG.trace("{}: Add host listener: {}", containerName, service);
        }
    }

    /**
     * Invoked when a host listener is unregistered.
     *
     * @param service  Host listener service.
     */
    void removeHostListener(IfHostListener service) {
        if (hostListeners.remove(service)) {
            LOG.trace("{}: Remove host listener: {}", containerName, service);
        }
    }

    /**
     * Invoked when a VTN resource manager service is registered.
     *
     * @param service  VTN resource manager service.
     */
    void setResourceManager(IVTNResourceManager service) {
        LOG.trace("{}: Set VTN resource manager: {}", containerName, service);
        resourceManager = service;
    }

    /**
     * Invoked when a VTN resource manager service is unregistered.
     *
     * @param service  VTN resource manager service.
     */
    void unsetResourceManager(IVTNResourceManager service) {
        if (service != null && service.equals(resourceManager)) {
            LOG.trace("{}: Unset VTN resource manager: {}", containerName,
                      service);
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
            LOG.trace("{}: Add VTN manager listener: {}", containerName,
                      service);
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
            LOG.trace("{}: Remove VTN manager listener: {}", containerName,
                      service);
        }
    }

    /**
     * Invoked when a VTN mode listener service is registered.
     *
     * @param listener  VTN mode listener service.
     */
    void addVTNModeListener(IVTNModeListener listener) {
        if (vtnModeListeners.addIfAbsent(listener)) {
            LOG.trace("{}: Add VTN mode listener: {}", containerName,
                      listener);
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
            LOG.trace("{}: Remove VTN mode listener: {}", containerName,
                      service);
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
     * Return the flow condition DB.
     *
     * @return  The flow condition DB.
     */
    public ConcurrentMap<String, FlowCondImpl> getFlowCondDB() {
        return flowCondDB;
    }

    /**
     * Let the specified VTN visible to other controllers in the cluster.
     *
     * @param vtn  A virtual tenant.
     */
    public void export(VTenantImpl vtn) {
        tenantDB.put(vtn.getName(), vtn);
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
     * Return a collection of existing {@link MacAddressTable} instances.
     *
     * @return  A collection of {@link MacAddressTable} instances.
     */
    public Collection<MacAddressTable> getMacAddressTables() {
        return macTableMap.values();
    }

    /**
     * Return a MAC address table associated with the given virtual L2 bridge.
     *
     * @param path   Path to the virtual L2 bridge.
     * @return  A MAC address table associated with the given virtual L2
     *          bridge. {@code null} is returned if not found.
     */
    public MacAddressTable getMacAddressTable(VBridgePath path) {
        VBridgePath bpath = (path.getClass().equals(VBridgePath.class))
            ? path
            : new VBridgePath(path.getTenantName(), path.getBridgeName());
        return macTableMap.get(bpath);
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
     * Return a VTN flow database for the VTN specified by the path.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  A VTN flow database.
     * @throws VTNException  An error occurred.
     */
    private VTNFlowDatabase getTenantFlowDB(VTenantPath path)
        throws VTNException {
        String tenantName = getTenantName(path);
        VTNFlowDatabase fdb = getTenantFlowDB(tenantName);
        if (fdb == null) {
            throw new VTNException(tenantNotFound(tenantName));
        }

        return fdb;
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
     * Collect inactive flows and remove them in background.
     */
    public void cleanUpRemovedFlows() {
        if (clusterService.amICoordinator()) {
            RemovedFlowMatch fmatch = new RemovedFlowMatch(fwRuleManager);
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                fdb.removeFlows(this, fmatch);
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
        return (exists(nc) && isEdgePortImpl(nc));
    }

    /**
     * Return a set of existing nodes.
     *
     * @return  A set of existing nodes.
     */
    public Set<Node> getNodes() {
        return new HashSet<Node>(nodeDB.keySet());
    }

    /**
     * Determine whether the given node exists or not.
     *
     * @param node  Node associated with SDN switch.
     * @return  {@code true} is returned if the given node exists.
     *          Otherwise {@code false} is returned.
     */
    public boolean exists(Node node) {
        return nodeDB.containsKey(node);
    }

    /**
     * Determine whether the given switch port exists or not.
     *
     * @param port  A {@link NodeConnector} instance corresponding to a
     *              physical swtich.
     * @return  {@code true} is returned if the given port exists.
     *          Otherwise {@code false} is returned.
     */
    public boolean exists(NodeConnector port) {
        return (nodeDB.containsKey(port.getNode()) &&
                portDB.containsKey(port));
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
            if (nodeDB.containsKey(port.getNode())) {
                PortProperty pp = entry.getValue();
                if (pp.isEnabled() && isEdgePortImpl(port)) {
                    portSet.add(port);
                }
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
     * <p>
     *   Switch ports to be collected can be selected by specifying
     *   {@link PortFilter} instance to {@code filter}.
     *   If a {@link PortFilter} instance is specified, this method collects
     *   switch ports accepted by
     *   {@link PortFilter#accept(NodeConnector, PortProperty)}.
     * </p>
     *
     * @param portSet  A set of node connectors to store results.
     * @param filter   A {@link PortFilter} instance which filters switch port.
     *                 All switch ports are stored to {@code portSet} if
     *                 {@code null} is specified.
     */
    public void collectUpEdgePorts(Set<NodeConnector> portSet,
                                   PortFilter filter) {
        if (filter == null) {
            collectUpEdgePorts(portSet);
            return;
        }

        for (Map.Entry<NodeConnector, PortProperty> entry: portDB.entrySet()) {
            NodeConnector port = entry.getKey();
            if (nodeDB.containsKey(port.getNode())) {
                PortProperty pp = entry.getValue();
                if (pp.isEnabled() && isEdgePortImpl(port) &&
                    filter.accept(port, pp)) {
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
            if (nodeDB.containsKey(node) && port.getNode().equals(node)) {
                PortProperty pp = entry.getValue();
                if (pp.isEnabled() && isEdgePortImpl(port)) {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     * Determine whether the given physical switch port is enabled or not.
     *
     * @param port  Node connector associated with physical switch port.
     * @return  {@code true} is returned if the given physical switch port
     *          is enabled. Otherwise {@code false} is returned.
     */
    public boolean isEnabled(NodeConnector port) {
        if (nodeDB.containsKey(port.getNode())) {
            PortProperty pp = portDB.get(port);
            return (pp != null && pp.isEnabled());
        }

        return false;
    }

    /**
     * Return property of the specified physical switch port.
     *
     * @param nc  Node connector associated with physical switch port.
     * @return  {@code PortProperty} object which represents switch port
     *          property is returned. {@code null} is returned if not found.
     */
    public PortProperty getPortProperty(NodeConnector nc) {
        return (nodeDB.containsKey(nc.getNode())) ? portDB.get(nc) : null;
    }

    /**
     * Return the name of the specified switch port.
     *
     * @param nc  A {@link NodeConnector} instance corresponding to the
     *            physical switch port.
     * @return  The name of the switch port.
     *          {@code null} is returned if the port name could not be
     *          determined.
     */
    public String getPortName(NodeConnector nc) {
        if (nodeDB.containsKey(nc.getNode())) {
            PortProperty prop = portDB.get(nc);
            return (prop == null) ? null : prop.getName();
        }

        return null;
    }

    /**
     * Transmit an ethernet frame to the specified node connector.
     *
     * @param nc     A node connector.
     * @param ether  An ethernet frame.
     * @return  {@code true} if the given packet was sent.
     *          Otherwise {@code false}.
     */
    public boolean transmit(NodeConnector nc, Ethernet ether) {
        Node node = nc.getNode();
        IDataPacketService pktSrv = dataPacketService;
        RawPacket pkt = pktSrv.encodeDataPacket(ether);
        ConnectionLocality cl = connectionManager.getLocalityStatus(node);
        boolean ret = true;
        if (cl == ConnectionLocality.LOCAL) {
            if (!disabledNodes.containsKey(node)) {
                pkt.setOutgoingNodeConnector(nc);
                pktSrv.transmitDataPacket(pkt);
            } else if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Don't send packet to disabled node: {}",
                          containerName, node);
                ret = false;
            }
        } else if (cl == ConnectionLocality.NOT_LOCAL) {
            // Toss the packet to remote cluster nodes.
            RawPacketEvent ev = new RawPacketEvent(pkt, nc);
            postEvent(ev);
        } else {
            LOG.warn("{}: Drop packet because target port is " +
                     "uncontrollable: {}", containerName, nc);
            ret = false;
        }

        return ret;
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
     * Save virtual tenant configuration, and apply current configuration to
     * the VTN Manager.
     *
     * @param tenantName  The name of the virtual tenant.
     * @return  A {@link VTenantImpl} instance corresponding to the specified
     *          tenant name. {@code null} is returned if not fonud.
     */
    public VTenantImpl saveTenantConfig(String tenantName) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return saveTenantConfigLocked(tenantName);
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
     * @return  A {@link VTenantImpl} instance corresponding to the specified
     *          tenant name. {@code null} is returned if not fonud.
     */
    private VTenantImpl saveTenantConfigLocked(String tenantName) {
        VTenantImpl vtn = tenantDB.get(tenantName);
        if (vtn != null) {
            vtn.saveConfig(this);
        }

        return vtn;
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
            if (LOG.isTraceEnabled() && !remote.equals(provider)) {
                LOG.trace("{}: Wait for {} to initialize cluster caches",
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

        LOG.trace("{}: Became configuration provider: {}",
                  containerName, myaddr);
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
        ContainerConfig cfg = new ContainerConfig(containerName);
        if (waitForCache(path, myaddr)) {
            // Load VTN configurations.
            for (String name: cfg.getKeys(ContainerConfig.Type.TENANT)) {
                loadTenantConfig(cfg, name);
            }

            // Load flow conditions.
            for (String name: cfg.getKeys(ContainerConfig.Type.FLOWCOND)) {
                loadFlowCondition(cfg, name);
            }

            // Load path policies.
            for (String name: cfg.getKeys(ContainerConfig.Type.PATHPOLICY)) {
                loadPathPolicy(cfg, name);
            }

            // Load container path maps.
            for (String name: cfg.getKeys(ContainerConfig.Type.PATHMAP)) {
                loadContainerPathMap(cfg, name);
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
            cfg.deleteAll(ContainerConfig.Type.TENANT, names);

            // Update configuration files for flow conditions.
            names.clear();
            for (FlowCondImpl fc: flowCondDB.values()) {
                fc.saveConfig(this);
                names.add(fc.getName());
            }

            // Remove configuration files for obsolete flow conditions.
            cfg.deleteAll(ContainerConfig.Type.FLOWCOND, names);

            // Update configuration files for path policies.
            names.clear();
            for (PathPolicyImpl pp: pathPolicyDB.values()) {
                pp.saveConfig(this);
                names.add(Integer.toString(pp.getPolicyId()));
            }

            // Remove configuration files for obsolete path policies.
            cfg.deleteAll(ContainerConfig.Type.PATHPOLICY, names);

            // Update configuration files for container path maps.
            names.clear();
            for (ContainerPathMapImpl cpm: pathMapDB.values()) {
                cpm.saveConfig(this);
                names.add(Integer.toString(cpm.getIndex()));
            }

            // Remove configuration files for obsolete container path maps.
            cfg.deleteAll(ContainerConfig.Type.PATHMAP, names);
        }
    }

    /**
     * Load configuration for the specified virtual tenant.
     *
     * @param cfg         A {@link ContainerConfig} instance.
     * @param tenantName  The name of the tenant.
     */
    private void loadTenantConfig(ContainerConfig cfg, String tenantName) {
        // Read tenant configuration.
        VTenantImpl newvtn =
            (VTenantImpl)cfg.load(ContainerConfig.Type.TENANT, tenantName);
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
     * Load the specified flow condition from the file.
     *
     * @param cfg   A {@link ContainerConfig} instance.
     * @param name  The name of the flow condition.
     */
    private void loadFlowCondition(ContainerConfig cfg, String name) {
        FlowCondImpl newfc = (FlowCondImpl)
            cfg.load(ContainerConfig.Type.FLOWCOND, name);
        if (newfc != null) {
            FlowCondImpl fc = flowCondDB.putIfAbsent(name, newfc);
            if (fc == null) {
                String fmt = "{}: Flow condition was loaded: {}";
                if (LOG.isTraceEnabled()) {
                    LOG.trace(fmt, containerName, fc);
                } else {
                    LOG.info(fmt, containerName, name);
                }
            }
        }
    }

    /**
     * Load the specified path policy from the file.
     *
     * @param cfg   A {@link ContainerConfig} instance.
     * @param name  A string representation of the path policy ID.
     */
    private void loadPathPolicy(ContainerConfig cfg, String name) {
        Integer id;
        try {
            id = Integer.valueOf(name);
        } catch (Exception e) {
            return;
        }

        PathPolicyImpl newpp = (PathPolicyImpl)
            cfg.load(ContainerConfig.Type.PATHPOLICY, name);
        if (newpp != null) {
            PathPolicyImpl pp = pathPolicyDB.putIfAbsent(id, newpp);
            if (pp == null) {
                String msg = "Path policy was loaded";
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}.{}: {}: {}", containerName, name, msg,
                              newpp);
                } else {
                    LOG.info("{}.{}: {}.", containerName, name, msg);
                }
            }
        }
    }

    /**
     * Load the specified container path map from the file.
     *
     * @param cfg   A {@link ContainerConfig} instance.
     * @param name  A string representation of the path map index.
     */
    private void loadContainerPathMap(ContainerConfig cfg, String name) {
        Integer id;
        try {
            id = Integer.valueOf(name);
        } catch (Exception e) {
            return;
        }

        ContainerPathMapImpl newcpm = (ContainerPathMapImpl)
            cfg.load(ContainerConfig.Type.PATHMAP, name);
        if (newcpm != null) {
            ContainerPathMapImpl cpm = pathMapDB.putIfAbsent(id, newcpm);
            if (cpm == null) {
                String msg = "Container path map was loaded";
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}.{}: {}: {}", containerName, name, msg,
                              newcpm);
                } else {
                    LOG.info("{}.{}: {}.", containerName, name, msg);
                }
            }
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
            throw new VTNException(MiscUtils.
                                   argumentIsNull("Tenant configuration"));
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
     * Return a failure status that indicates the specified flow condition
     * does not exist.
     *
     * @param name  The name of the flow condition.
     * @return  A failure status.
     */
    private Status flowConditionNotFound(String name) {
        String msg = name + ": Flow condition does not exist";
        return new Status(StatusCode.NOTFOUND, msg);
    }

    /**
     * Return a failure status that indicates the specified path policy
     * does not exist.
     *
     * @param id  The identifier of the path policy.
     * @return  A failure status.
     */
    private Status pathPolicyNotFound(Integer id) {
        String msg = id + ": Path policy does not exist";
        return new Status(StatusCode.NOTFOUND, msg);
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
            throw new VTNException(MiscUtils.argumentIsNull("Path"));
        }

        String tenantName = path.getTenantName();
        if (tenantName == null) {
            throw new VTNException(MiscUtils.argumentIsNull("Tenant name"));
        }

        return tenantName;
    }

    /**
     * Return the virtual tenant instance associated with the given
     * flow filter ID.
     *
     * <p>
     *   This method must be called with the VTN Manager lock.
     * </p>
     *
     * @param fid   A {@link FlowFilterId} instance.
     * @return  Virtual tenant instance is returned.
     * @throws VTNException  An error occurred.
     */
    private VTenantImpl getTenantImpl(FlowFilterId fid) throws VTNException {
        if (fid == null) {
            throw new VTNException(MiscUtils.argumentIsNull("Flow filter ID"));
        }

        return getTenantImpl(fid.getPath());
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
     * Ensure that the specified flow condition name is not null.
     *
     * @param name  The name of the flow condition.
     * @throws VTNException  An error occurred.
     */
    private void checkFlowConditionName(String name) throws VTNException {
        if (name == null) {
            throw new VTNException(MiscUtils.
                                   argumentIsNull("Flow condition name"));
        }
    }

    /**
     * Return the flow condition instance associated with the given name.
     *
     * <p>
     *   This method must be called with the VTN Manager lock.
     * </p>
     *
     * @param name  The name of the flow condition.
     * @return  A flow condition associated with the specified name.
     * @throws VTNException  An error occurred.
     */
    private FlowCondImpl getFlowCondImpl(String name) throws VTNException {
        FlowCondImpl fc = flowCondDB.get(name);
        if (fc == null) {
            throw new VTNException(flowConditionNotFound(name));
        }

        return fc;
    }

    /**
     * Return the flow condition instance associated with the given name.
     *
     * <ul>
     *   <li>
     *     This method ensures that the specified name is not {@code null}.
     *   </li>
     *   <li>This method must be called with the VTN Manager lock.</li>
     * </ul>
     *
     * @param name  The name of the flow condition.
     * @return  A flow condition associated with the specified name.
     * @throws VTNException  An error occurred.
     */
    private FlowCondImpl getFlowCondImplCheck(String name)
        throws VTNException {
        checkFlowConditionName(name);
        return getFlowCondImpl(name);
    }

    /**
     * Return the path policy instance associated with the given ID.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param id  The identifier of the path policy.
     * @return  A path policy associated with the specified ID.
     * @throws VTNException  An error occurred.
     */
    private PathPolicyImpl getPathPolicyImpl(Integer id) throws VTNException {
        PathPolicyImpl pp = pathPolicyDB.get(id);
        if (pp == null) {
            throw new VTNException(pathPolicyNotFound(id));
        }

        return pp;
    }

    /**
     * Update internal state of the routing table corresponding to the
     * specified path policy.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param id  The identifier of the path policy.
     */
    private void updatePathPolicyMap(Integer id) {
        PathPolicyMap ppmap = pathPolicyMap.get(id);
        if (ppmap != null) {
            ppmap.reset();
            return;
        }

        // This should never happen.
        LOG.warn("{}: {}: Path policy map not found.", containerName, id);
    }

    /**
     * Record an error log message which indicates an unexpected exception
     * was caught.
     *
     * @param log   A {@link Logger} instance.
     * @param path  A {@link VTenantPath} which specifies the virtual node.
     * @param e     An exception to be logged.
     * @param args  Additional objects to be added to a log message.
     */
    public void logException(Logger log, VTenantPath path, Exception e,
                             Object ... args) {
        StringBuilder builder = new StringBuilder(containerName);
        if (path != null) {
            builder.append(':').append(path);
        }
        builder.append(": Unexpected exception");

        if (args != null) {
            String sep = ": ";
            for (Object o: args) {
                builder.append(sep).append(o);
                sep = ", ";
            }
        }
        log.error(builder.toString(), e);
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
     * Create a reference to the port mapping configured in this container.
     *
     * @param path  A path to the virtual interface which contains port mapping
     *              configuration.
     * @return      A reference to the port mapping.
     */
    public MapReference getMapReference(VInterfacePath path) {
        return new MapReference(MapType.PORT, containerName, (VNodePath)path);
    }

    /**
     * Create a reference to the MAC mapping configured in this container.
     *
     * @param path  A path to the MAC mapping.
     * @return      A reference to the MAC mapping.
     */
    public MapReference getMapReference(MacMapPath path) {
        return new MapReference(MapType.MAC, containerName, path);
    }

    /**
     * Create a reference to the VLAN mapping configured in this container.
     *
     * @param path  A path to the VLAN mapping.
     * @return      A reference to the VLAN mapping.
     */
    public MapReference getMapReference(VlanMapPath path) {
        return new MapReference(MapType.VLAN, containerName, path);
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
                putEvent(evid, cev);
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
     * Call vTerminal listeners on the calling thread.
     *
     * @param path   Path to the vTerminal.
     * @param vterm  Information about the vTerminal.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     *               {@code CHANGED} if changed.
     */
    public void notifyListeners(VTerminalPath path, VTerminal vterm,
                                UpdateType type) {
        LOG.info("{}:{}: vTerminal {}: {}", containerName, path,
                 type.getName(), vterm);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.vTerminalChanged(path, vterm, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(": Unhandled exception in vTerminal listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
    }

    /**
     * Notify the vTerminal changes.
     *
     * @param path   Path to the vTerminal.
     * @param vterm  Information about the vTerminal.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     *               {@code CHANGED} if changed.
     */
    public void notifyChange(final VTerminalPath path, final VTerminal vterm,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                notifyListeners(path, vterm, type);
            }
        };
        postTask(r);
    }

    /**
     * Notify the specified listener of the vTerminal.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the vTerminal.
     * @param vterm     Information about the vTerminal.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     *                  {@code CHANGED} if changed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VTerminalPath path, final VTerminal vterm,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.vTerminalChanged(path, vterm, type);
            }
        };
        postTask(r);
    }

    /**
     * Call virtual interface listeners on the calling thread.
     *
     * @param path    Path to the virtual interface.
     * @param viface  Information about the virtual interface.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyListeners(VInterfacePath path, VInterface viface,
                                UpdateType type) {
        LOG.info("{}:{}: Virtual interface {}: {}", containerName, path,
                 type.getName(), viface);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                path.vInterfaceChanged(listener, viface, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(": Unhandled exception in ").
                    append(path.getNodeType()).
                    append(" interface listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
    }

    /**
     * Notify the virtual interface changes.
     *
     * @param path    Path to the virtual interface.
     * @param viface  Information about the virtual interface.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyChange(final VInterfacePath path,
                             final VInterface viface, final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                notifyListeners(path, viface, type);
            }
        };
        postTask(r);
    }

    /**
     * Notify the specified listener of the virtual interface changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the virtualinterface.
     * @param viface    Information about the virtual interface.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     *                  {@code CHANGED} if changed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VInterfacePath path,
                             final VInterface viface, final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                path.vInterfaceChanged(listener, viface, type);
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
                builder.append(
                    ": Unhandled exception in VLAN mapping listener: ").
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
     * Call MAC mapping listeners on the calling thread.
     *
     * @param path    Path to the bridge associated with the MAC mapping.
     * @param mcconf  Configuration information about the MAC mapping.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyListeners(VBridgePath path, MacMapConfig mcconf,
                                UpdateType type) {
        String msg = "MAC mapping";
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}: {} {}: {}", containerName, path, msg,
                      type.getName(), mcconf);
        } else {
            LOG.info("{}:{}: {} {}", containerName, path, msg,
                     type.getName());
        }

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                listener.macMapChanged(path, mcconf, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(
                    ": Unhandled exception in MAC mapping listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
    }

    /**
     * Notify MAC mapping changes.
     *
     * @param path    Path to the bridge associated with the MAC mapping.
     * @param mcconf  Configuration information about the MAC mapping.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void notifyChange(final VBridgePath path, final MacMapConfig mcconf,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                notifyListeners(path, mcconf, type);
            }
        };
        postTask(r);
    }

    /**
     * Notify MAC mapping changes.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the bridge associated with the MAC mapping.
     * @param mcconf    Configuration information about the MAC mapping.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     *                  {@code CHANGED} if changed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VBridgePath path, final MacMapConfig mcconf,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                listener.macMapChanged(path, mcconf, type);
            }
        };
        postTask(r);
    }

    /**
     * Call port mapping listeners on the calling thread.
     *
     * @param path  Path to the virtual interface.
     * @param pmap  Information about the port mapping.
     * @param type  {@code ADDED} if added.
     *              {@code REMOVED} if removed.
     */
    public void notifyListeners(VInterfacePath path, PortMap pmap,
                                UpdateType type) {
        LOG.info("{}:{}: Port mapping {}: {}",
                 containerName, path, type.getName(), pmap);

        for (IVTNManagerAware listener: vtnManagerAware) {
            try {
                path.portMapChanged(listener, pmap, type);
            } catch (Exception e) {
                StringBuilder builder = new StringBuilder(containerName);
                builder.append(
                    ": Unhandled exception in port mapping listener: ").
                    append(listener).append(": ").append(e.toString());
                LOG.error(builder.toString(), e);
            }
        }
    }

    /**
     * Notify changes of port mapping configured in virtual interface.
     *
     * @param path  Path to the virtual interface.
     * @param pmap  Information about the port mapping.
     * @param type  {@code ADDED} if added.
     *              {@code REMOVED} if removed.
     */
    public void notifyChange(final VInterfacePath path, final PortMap pmap,
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
     * Notify changes of port mapping configured in virtual interface.
     *
     * @param listener  VTN manager listener service.
     * @param path      Path to the virtual interface.
     * @param pmap      Information about the port mapping.
     * @param type      {@code ADDED} if added.
     *                  {@code REMOVED} if removed.
     */
    public void notifyChange(final IVTNManagerAware listener,
                             final VInterfacePath path, final PortMap pmap,
                             final UpdateType type) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                path.portMapChanged(listener, pmap, type);
            }
        };
        postTask(r);
    }

    /**
     * Notify the host listener of a new host.
     *
     * @param host  A new host.
     */
    public void notifyHost(final HostNodeConnector host) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                for (IfHostListener listener: hostListeners) {
                    try {
                        listener.hostListener(host);
                    } catch (Exception e) {
                        StringBuilder builder =
                            new StringBuilder(containerName);
                        builder.append(
                            ": Unhandled exception in host listener: ").
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

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (type == UpdateType.ADDED) {
                // Create flow database for a new tenant.
                createTenantFlowDB(tenantName);

                // Save configurations, and update VTN mode.
                saveTenantConfigLocked(tenantName);
                updateVTNMode(true);
            } else {
                // Remove flow database.
                // Flow entries in this tenant is purged by event originator.
                removeTenantFlowDB(tenantName);

                // Delete the virtual tenant configuration file.
                ContainerConfig cfg = new ContainerConfig(containerName);
                cfg.delete(ContainerConfig.Type.TENANT, tenantName);

                // Save tenant names, and update VTN mode.
                updateVTNMode(false);

                // Purge canceled timer tasks.
                resourceManager.getTimer().purge();
            }
        } finally {
            unlock(wrlock);
        }
    }

    /**
     * Called when a flow condition was added, removed, or changed by
     * remote cluster node.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index which specifies the flow match condition
     *               in the flow condition. A negative value is specified if
     *               the target is flow condition itself.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     *               {@code CHANGED} if changed.
     */
    public void updateFlowCondition(String name, int index, UpdateType type) {
        ContainerConfig cfg = new ContainerConfig(containerName);
        final ContainerConfig.Type cfgType = ContainerConfig.Type.FLOWCOND;

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (type == UpdateType.REMOVED && index < 0) {
                // Delete the configuration file for the flow condition.
                cfg.delete(cfgType, name);

                if (index < 0) {
                    LOG.info("{}:{}: Flow condition was removed.",
                             containerName, name);
                } else {
                    LOG.info("{}:{}.{}: Flow match condition was removed.",
                             containerName, name, index);
                }

                return;
            }

            // Save the configuration for the flow condition.
            FlowCondImpl fc = flowCondDB.get(name);
            if (fc == null) {
                LOG.debug("{}:{}: Ignore phantom event of the " +
                          "flow condition: index={}", containerName,
                          name, index);
                return;
            }

            fc.saveConfig(this);
            if (!LOG.isTraceEnabled()) {
                // Record a simple informational log.
                if (index < 0) {
                    LOG.info("{}:{}: Flow condition was {}.",
                             containerName, name, type.getName());
                } else {
                    LOG.info("{}:{}.{}: Flow match condition was {}.",
                             containerName, name, index, type.getName());
                }
                return;
            }

            if (index < 0) {
                LOG.trace("{}:{}: Flow condition was {}: {}",
                          containerName, name, type.getName(), fc);
            } else {
                LOG.trace("{}:{}.{}: Flow match condition was {}: {}",
                          containerName, name, index, type.getName(), fc);
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Called when a path policy was added, removed, or changed by remote
     * cluster node.
     *
     * @param id    The identifier of the path policy.
     * @param type  {@code ADDED} if added.
     *              {@code REMOVED} if removed.
     *              {@code CHANGED} if changed.
     */
    public void updatePathPolicy(int id, UpdateType type) {
        ContainerConfig cfg = new ContainerConfig(containerName);
        final ContainerConfig.Type cfgType = ContainerConfig.Type.PATHPOLICY;

        Integer pid = Integer.valueOf(id);
        String name = Integer.toString(id);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (type == UpdateType.REMOVED) {
                // Delete the configuration file for the path policy.
                cfg.delete(cfgType, name);
                LOG.info("{}:{}: Path policy was removed.",
                         containerName, name);

                return;
            }

            // Reset internal state of the routing table.
            updatePathPolicyMap(pid);

            // Save the configuration for the path policy.
            PathPolicyImpl pp = pathPolicyDB.get(pid);
            if (pp == null) {
                LOG.debug("{}:{}: Ignore phantom event of the path policy.",
                          containerName, name);
                return;
            }

            pp.saveConfig(this);
            LOG.info("{}:{}: Path policy was {}.",
                     containerName, name, type.getName());
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Called when a container path map was added, removed, or changed by
     * remote cluster node.
     *
     * @param index  The index of the container path map.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     *               {@code CHANGED} if changed.
     */
    public void updateContainerPathMap(int index, UpdateType type) {
        ContainerConfig cfg = new ContainerConfig(containerName);
        final ContainerConfig.Type cfgType = ContainerConfig.Type.PATHMAP;
        Integer key = Integer.valueOf(index);
        String name = key.toString();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (type == UpdateType.REMOVED) {
                // Delete the configuration file for the container path map.
                cfg.delete(cfgType, name);
                LOG.info("{}.{}: Container path map was removed.",
                         containerName, name);

                return;
            }

            // Save the configuration for the container path map.
            ContainerPathMapImpl cpm = pathMapDB.get(key);
            if (cpm == null) {
                LOG.debug("{}.{}: Ignore phantom event of the " +
                          "container path map.", containerName, name);
                return;
            }

            cpm.saveConfig(this);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Container path map was {}: {}",
                          containerName, name, type.getName(), cpm);
            } else {
                LOG.info("{}.{}: Container path map was {}.",
                         containerName, name, type.getName());
            }
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Called when a VTN path map was added, removed, or changed by remote
     * cluster node.
     *
     * @param tname  The name of the virtual tenant.
     * @param index  The index of the container path map.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     *               {@code CHANGED} if changed.
     */
    public void updateVTenantPathMap(String tname, int index,
                                     UpdateType type) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Save the VTN configuration.
            VTenantImpl vtn = saveTenantConfigLocked(tname);
            if (vtn == null) {
                LOG.debug("{}:{}.{}: Ignore phantom event of the VTN path map.",
                          containerName, tname, index);
                return;
            }

            LOG.info("{}:{}.{}: VTN path map was {}.",
                      containerName, tname, index, type.getName());
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return the link cost determined by the path policy.
     *
     * @param id    The identifier of the path policy.
     * @param port  A {@link NodeConnector} instance corresponding to the
     *              switch port.
     * @return    A {@link Long} instance which represents the link cost.
     */
    public Long getLinkCost(Integer id, NodeConnector port) {
        // VTN Manager lock should never be held here because this method will
        // be called with holding the SAL routing service lock.
        PathPolicyImpl pp = pathPolicyDB.get(id);
        if (pp == null) {
            LOG.debug("{}: Path policy not found: id={}, port={}",
                      containerName, id, port);
            return Long.valueOf(DEFAULT_LINK_COST);
        }

        return pp.getCost(this, port);
    }

    /**
     * Determine whether the specified path policy exists or not.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param id  The identifier of the path policy.
     * @return    {@code true} if exists. Otherwise {@code false}.
     */
    public boolean pathPolicyExists(Integer id) {
        return pathPolicyDB.containsKey(id);
    }

    /**
     * Return a packet route resolver associated with the specified
     * path policy ID.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param id  The identifier of the path policy.
     * @return  A {@link RouteResolver} instance if found.
     *          {@code null} if not found.
     */
    public RouteResolver getRouteResolver(int id) {
        if (id == PathPolicyImpl.POLICY_DEFAULT) {
            return this;
        }

        Integer key = Integer.valueOf(id);
        return pathPolicyMap.get(key);
    }

    /**
     * Evaluate the container path map list against the specified packet.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param pctx   The context of the ARP packet to send.
     * @param tconf  The configuration for the virtual tenant.
     * @return  A {@link RouteResolver} instance is returned if a path map in
     *          the container path map list matched the packet.
     *          The default route resolver is returned if no container path
     *          map patched the packet.
     */
    public RouteResolver evalPathMap(PacketContext pctx, VTenantConfig tconf) {
        if (!pathMapDB.isEmpty()) {
            // Sort container path maps in ascending order of indices.
            ArrayList<ContainerPathMapImpl> list =
                new ArrayList<ContainerPathMapImpl>(pathMapDB.values());
            Collections.sort(list, new PathMapImplComparator());
            for (ContainerPathMapImpl cpm: list) {
                RouteResolver rr = cpm.evaluate(this, pctx);
                if (rr != null) {
                    return rr;
                }
            }
        }

        return this;
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
                if (disabledNodes.remove(node) != null) {
                    LOG.info("{}: {}: Start packet service",
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
     * Determine whether the specified node is contained in
     * {@link #disabledNodes} or not.
     *
     * <p>
     *   This method is only for testing.
     * </p>
     *
     * @param node  A {@link Node} instance to be tested.
     * @return  {@code true} only if the specified node is contained in
     *          {@link #disabledNodes}.
     */
    boolean isDisabled(Node node) {
        return disabledNodes.containsKey(node);
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
     * Create a {@link StatsReader} instance to read flow statistics.
     *
     * @param mode   A {@link org.opendaylight.vtn.manager.flow.DataFlow.Mode}
     *               instance.
     * @param cache  Specify {@code true} if you want to cache statistics in
     *               a returned {@link StatsReader} instance.
     * @return  A {@link StatsReader} instance.
     *          {@code null} is returned if
     *          {@link org.opendaylight.vtn.manager.flow.DataFlow.Mode#SUMMARY}
     *          is passed to {@code mode}.
     * @throws VTNException   {@code mode} is {@code null}.
     */
    private StatsReader createStatsReader(DataFlow.Mode mode, boolean cache)
        throws VTNException {
        if (mode == DataFlow.Mode.SUMMARY) {
            return null;
        } else if (mode == null) {
            throw new VTNException(MiscUtils.argumentIsNull("Mode"));
        }

        boolean update = (mode == DataFlow.Mode.UPDATE_STATS);
        return new StatsReader(statisticsManager, update, cache);
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

    /**
     * Purge network caches specified by {@link MapCleaner} instance.
     *
     * <p>
     *   Note that network cache purging will be done asynchronously.
     * </p>
     *
     * @param cleaner     A {@link MapCleaner} instance.
     * @param tenantName  The name of the target virtual tenant.
     */
    void purge(final MapCleaner cleaner, final String tenantName) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                Lock rdlock = rwLock.readLock();
                rdlock.lock();
                try {
                    cleaner.purge(VTNManagerImpl.this, tenantName);
                } finally {
                    unlock(rdlock);
                }
            }
        };
        postTask(r);
    }

    /**
     * Update internal state of the specified vBridge in this container.
     *
     * <p>
     *   Note that vBridge state will be updated asynchronously.
     * </p>
     *
     * @param path  A path to the vBridge.
     */
    void updateBridgeState(final VBridgePath path) {
        Runnable r = new Runnable() {
            @Override
            public void run() {
                Lock rdlock = rwLock.readLock();
                rdlock.lock();
                try {
                    VTenantImpl vtn = getTenantImpl(path);
                    vtn.updateBridgeState(VTNManagerImpl.this, path);
                } catch (VTNException e) {
                    // Ignore.
                } finally {
                    unlock(rdlock);
                }
            }
        };
        postTask(r);
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
        ArrayList<VTenant> list;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            list = new ArrayList<VTenant>(tenantDB.size());
            for (VTenantImpl vtn: tenantDB.values()) {
                list.add(vtn.getVTenant());
            }
        } finally {
            rdlock.unlock();
        }

        // Sort tenants by their name.
        Collections.sort(list, new VTenantComparator());
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
            MiscUtils.checkName("Tenant", tenantName);

            VTenantImpl vtn = new VTenantImpl(containerName, tenantName,
                                              tconf);
            if (tenantDB.putIfAbsent(tenantName, vtn) != null) {
                String msg = tenantName + ": Tenant name already exists";
                return new Status(StatusCode.CONFLICT, msg);
            }

            // Create a VTN flow database.
            createTenantFlowDB(tenantName);

            Status status = vtn.saveConfig(null);
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
            return vtn.setVTenantConfig(this, path, tconf, all);
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

            data.setModeChanged();
            enqueueEvent(path, vtenant, UpdateType.REMOVED);

            return new Status(StatusCode.SUCCESS, null);
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
            return vtn.addBridge(this, path, bconf);
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
            return vtn.modifyBridge(this, path, bconf, all);
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
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeBridge(this, path);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }


    /**
     * Return a list of vTerminals in the specified tenant.
     *
     * @param path  Path to the virtual tenant.
     * @return  A list of vTerminals.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VTerminal> getTerminals(VTenantPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getTerminals(this);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return information about the specified vTerminal.
     *
     * @param path  Path to the vTerminal.
     * @return  Information about the specified vTerminal.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VTerminal getTerminal(VTerminalPath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getTerminal(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Add a new vTerminal.
     *
     * @param path    Path to the vTerminal to be added.
     * @param vtconf  vTerminal configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addTerminal(VTerminalPath path, VTerminalConfig vtconf) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.addTerminal(this, path, vtconf);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Modify configuration of existing vTerminal.
     *
     * @param path    Path to the vTerminal.
     * @param vtconf  vTerminal configuration.
     * @param all     If {@code true} is specified, all attributes of the
     *                vTerminal are modified. In this case, {@code null} in
     *                {@code vtconf} is interpreted as default value.
     *                If {@code false} is specified, an attribute is not
     *                modified if its value in {@code vtconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyTerminal(VTerminalPath path, VTerminalConfig vtconf,
                               boolean all) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.modifyTerminal(this, path, vtconf, all);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the vTerminal specified by the given name.
     *
     * @param path  Path to the vTerminal.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeTerminal(VTerminalPath path) {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeTerminal(this, path);
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
     * @param path  Path to the vBridge.
     * @return  A list of virtual interfaces.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VInterface> getInterfaces(VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getInterfaces(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return information about the specified virtual interface attached to
     * the virtual L2 bridge.
     *
     * @param path  Path to the vBridge interface.
     * @return  Information about the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VInterface getInterface(VBridgeIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getInterface(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Add a new virtual interface to the virtual L2 bridge.
     *
     * @param path   Path to the vBridge interface to be added.
     * @param iconf  Interface configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addInterface(VBridgeIfPath path, VInterfaceConfig iconf) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.addInterface(this, path, iconf);
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
     * @param path   Path to the vBridge interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyInterface(VBridgeIfPath path, VInterfaceConfig iconf,
                                  boolean all) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.modifyInterface(this, path, iconf, all);
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
    public Status removeInterface(VBridgeIfPath path) {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeInterface(this, path);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return a list of virtual interfaces attached to the specified vTerminal.
     *
     * @param path  Path to the vTerminal.
     * @return  A list of virtual interfaces.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VInterface> getInterfaces(VTerminalPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getInterfaces(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return information about the specified virtual interface attached to
     * the vTerminal.
     *
     * @param path  Path to the vTerminal interface.
     * @return  Information about the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public VInterface getInterface(VTerminalIfPath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getInterface(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Add a new virtual interface to the vTerminal.
     *
     * @param path   Path to the vTerminal interface to be added.
     * @param iconf  Interface configuration.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status addInterface(VTerminalIfPath path, VInterfaceConfig iconf) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.addInterface(this, path, iconf);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Modify configuration of existing virtual interface attached to the
     * vTerminal.
     *
     * @param path   Path to the vTerminal interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status modifyInterface(VTerminalIfPath path, VInterfaceConfig iconf,
                                  boolean all) {
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.modifyInterface(this, path, iconf, all);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the virtual interface from the vTerminal.
     *
     * @param path  Path to the interface.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status removeInterface(VTerminalIfPath path) {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeInterface(this, path);
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
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.addVlanMap(this, path, vlconf);
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
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removeVlanMap(this, path, mapId);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return the port mapping configuration applied to the specified vBridge
     * interface.
     *
     * @param path  Path to the vBridge interface.
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
     * Return the port mapping configuration applied to the specified
     * vTerminal interface.
     *
     * @param path  Path to the vTerminal interface.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on the specified interface.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PortMap getPortMap(VTerminalIfPath path) throws VTNException {
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
     * vBridge interface.
     *
     * @param path    Path to the vBridge interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf) {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.setPortMap(this, path, pmconf);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Create or destroy mapping between the physical switch port and the
     * vTerminal interface.
     *
     * @param path    Path to the vTerminal.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  "Success" or failure reason.
     */
    @Override
    public Status setPortMap(VTerminalIfPath path, PortMapConfig pmconf) {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.setPortMap(this, path, pmconf);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return information about the MAC mapping configured in the specified
     * vBridge.
     *
     * @param path   Path to the bridge.
     * @return  A {@link MacMap} object which represents information about
     *          the MAC mapping specified by {@code path}.
     *          {@code null} is returned if the MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public MacMap getMacMap(VBridgePath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacMap(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return configuration information about MAC mapping in the specified
     * vBridge.
     *
     * @param path     Path to the vBridge.
     * @param aclType  The type of access control list.
     * @return  A set of {@link DataLinkHost} instances which contains host
     *          information in the specified access control list is returned.
     *          {@code null} is returned if MAC mapping is not configured in
     *          the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public Set<DataLinkHost> getMacMapConfig(VBridgePath path,
                                             MacMapAclType aclType)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacMapConfig(path, aclType);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return a list of {@link MacAddressEntry} instances corresponding to
     * all the MAC address information actually mapped by MAC mapping
     * configured in the specified vBridge.
     *
     * @param path  Path to the vBridge.
     * @return  A list of {@link MacAddressEntry} instances corresponding to
     *          all the MAC address information actually mapped to the vBridge
     *          specified by {@code path}.
     *          {@code null} is returned if MAC mapping is not configured
     *          in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<MacAddressEntry> getMacMappedHosts(VBridgePath path)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacMappedHosts(this, path);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Determine whether the host specified by the MAC address is actually
     * mapped by MAC mapping configured in the specified vBridge.
     *
     * @param path  Path to the vBridge.
     * @param addr  A {@link DataLinkAddress} instance which represents the
     *              MAC address.
     * @return  A {@link MacAddressEntry} instancw which represents information
     *          about the host corresponding to {@code addr} is returned
     *          if it is actually mapped to the specified vBridge by MAC
     *          mapping.
     *          {@code null} is returned if the MAC address specified by
     *          {@code addr} is not mapped by MAC mapping, or MAC mapping is
     *          not configured in the specified vBridge.
     * @throws VTNException  An error occurred.
     */
    @Override
    public MacAddressEntry getMacMappedHost(VBridgePath path,
                                            DataLinkAddress addr)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getMacMappedHost(this, path, addr);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Change MAC mapping configuration as specified by {@link MacMapConfig}
     * instance.
     *
     * @param path    A {@link VBridgePath} object that specifies the position
     *                of the vBridge.
     * @param op      A {@link UpdateOperation} instance which indicates
     *                how to change the MAC mapping configuration.
     * @param mcconf  A {@link MacMapConfig} instance which contains the MAC
     *                mapping configuration information.
     * @return        A {@link UpdateType} object which represents the result
     *                of the operation is returned.
     *                {@code null} is returned if the configuration was not
     *                changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setMacMap(VBridgePath path, UpdateOperation op,
                                MacMapConfig mcconf) throws VTNException {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.setMacMap(this, path, op, mcconf);
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Change the access controll list for the specified MAC mapping.
     *
     * @param path      A {@link VBridgePath} object that specifies the
     *                  position of the vBridge.
     * @param op        A {@link UpdateOperation} instance which indicates
     *                  how to change the MAC mapping configuration.
     * @param aclType   The type of access control list.
     * @param dlhosts   A set of {@link DataLinkHost} instances.
     * @return          A {@link UpdateType} object which represents the result
     *                  of the operation is returned.
     *                  {@code null} is returned if the configuration was not
     *                  changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setMacMap(VBridgePath path, UpdateOperation op,
                                MacMapAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.setMacMap(this, path, op, aclType, dlhosts);
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
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: findHost() called: addr={}, pathSet={}",
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
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: probeHost() called: host={}", containerName, host);
        }

        if (host == null) {
            return false;
        }

        NodeConnector nc = host.getnodeConnector();
        try {
            checkService();
            NodeUtils.checkNodeConnector(nc);
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.debug("{}: probeHost: Ignore request for {}: {}",
                      containerName, host, status.getDescription());
            return false;
        }

        if (!exists(nc)) {
            LOG.debug("{}: probeHost: Ignore request for {}: Unknown port {}",
                      containerName, host, nc);
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

        short vlan = host.getVlan();
        Ethernet ether = createArpRequest(dst, target, vlan);
        if (ether == null) {
            LOG.debug("{}: probeHost: Ignore request for {}: " +
                      "Invalid IP address", containerName, host);
            return false;
        }

        PacketContext pctx = new PacketContext(ether, nc);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (!isEdgePortImpl(nc)) {
                LOG.debug("{}: probeHost: Ignore request for {}: " +
                          "Internal port", containerName, host);
                return false;
            }

            // Determine the virtual node that maps the given host.
            MapReference ref = resourceManager.getMapReference(dst, nc, vlan);
            if (ref != null && containerName.equals(ref.getContainerName())) {
                VNodePath path = ref.getPath();
                VTenantImpl vtn = getTenantImpl(path);
                return vtn.probeHost(this, ref, pctx);
            }
        } catch (VTNException e) {
            if (LOG.isDebugEnabled()) {
                Status status = e.getStatus();
                LOG.debug("{}: Ignore probe request: {}", containerName,
                          status.getDescription());
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

    /**
     * Return information about all data flows present in the specified VTN.
     *
     * @param path    A {@link VTenantPath} object that specifies the position
     *                of the VTN.
     * @param mode    A {@link org.opendaylight.vtn.manager.flow.DataFlow.Mode}
     *                instance which specifies behavior of this method.
     * @param filter  If a {@link DataFlowFilter} instance is specified,
     *                only data flows that meet the condition specified by
     *                {@link DataFlowFilter} instance is returned.
     *                All data flows in the VTN is returned if {@code null}
     *                is specified.
     * @return  A list of {@link DataFlow} instances which represents
     *          information about data flows.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<DataFlow> getDataFlows(VTenantPath path, DataFlow.Mode mode,
                                       DataFlowFilter filter)
        throws VTNException {
        if (inContainerMode) {
            // No flow entry is active in container mode.
            // It's harmless to access inContainerMode flag without holding
            // rmLock.
            return new ArrayList<DataFlow>(0);
        }

        // We should not acquire lock here because succeeding method call may
        // make requests to get flow statistics. Synchronization will be done
        // by VTNFlowDatabase appropriately.
        VTNFlowDatabase fdb = getTenantFlowDB(path);
        StatsReader reader = createStatsReader(mode, true);
        DataFlowFilterImpl flt = new DataFlowFilterImpl(this, filter);
        return fdb.getFlows(this, reader, flt);
    }

    /**
     * Return information about the specified data flow in the VTN.
     *
     * @param path    A {@link VTenantPath} object that specifies the position
     *                of the VTN.
     * @param flowId  An identifier of the data flow.
     * @param mode    A {@link org.opendaylight.vtn.manager.flow.DataFlow.Mode}
     *                instance which specifies behavior of this method.
     * @return  A {@link DataFlow} instance which represents information
     *          about the specified data flow.
     *          {@code null} is returned if the specified data flow was not
     *          found.
     * @throws VTNException  An error occurred.
     */
    @Override
    public DataFlow getDataFlow(VTenantPath path, long flowId,
                                DataFlow.Mode mode)
        throws VTNException {
        if (inContainerMode) {
            // No flow entry is active in container mode.
            // It's harmless to access inContainerMode flag without holding
            // rmLock.
            return null;
        }

        // We should not acquire lock here because succeeding method call may
        // make requests to get flow statistics. Synchronization will be done
        // by VTNFlowDatabase appropriately.
        VTNFlowDatabase fdb = getTenantFlowDB(path);
        StatsReader reader = createStatsReader(mode, false);
        return fdb.getFlow(this, flowId, reader);
    }

    /**
     * Return the number of data flows present in the specified VTN.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  The number of data flows present in the specified VTN.
     * @throws VTNException  An error occurred.
     */
    @Override
    public int getDataFlowCount(VTenantPath path) throws VTNException {
        if (inContainerMode) {
            // No flow entry is active in container mode.
            // It's harmless to access inContainerMode flag without holding
            // rmLock.
            return 0;
        }

        VTNFlowDatabase fdb = getTenantFlowDB(path);
        return fdb.getFlowCount();
    }

    /**
     * Return a list of flow conditions configured in the container.
     *
     * @return  A list of {@link FlowCondition} instances corresponding to
     *          all flow conditions configured in the container.
     *          An empty list is returned if no flow condition is configured.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<FlowCondition> getFlowConditions() throws VTNException {
        ArrayList<FlowCondition> list;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            list = new ArrayList<FlowCondition>(flowCondDB.size());
            for (FlowCondImpl fc: flowCondDB.values()) {
                list.add(fc.getFlowCondition());
            }
        } finally {
            rdlock.unlock();
        }

        // Sort flow conditions by their name.
        Collections.sort(list, new FlowConditionComparator());
        return list;
    }

    /**
     * Return information about the flow condition specified by the name.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link FlowCondition} instance which represents information
     *          about the flow condition specified by {@code name}.
     * @throws VTNException  An error occurred.
     */
    @Override
    public FlowCondition getFlowCondition(String name) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            FlowCondImpl fc = getFlowCondImplCheck(name);
            return fc.getFlowCondition();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or modify the flow condition.
     *
     * @param name   The name of the flow condition.
     * @param fcond  A {@link FlowCondition} instance which specifies the
     *               configuration of the flow condition.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setFlowCondition(String name, FlowCondition fcond)
        throws VTNException {
        MiscUtils.checkName("Flow condition", name);

        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            UpdateType result;
            FlowCondImpl fc;
            while (true) {
                FlowCondImpl oldfc = flowCondDB.get(name);
                if (oldfc == null) {
                    // Create a new flow condition.
                    fc = new FlowCondImpl(name, fcond);
                    if (flowCondDB.putIfAbsent(name, fc) == null) {
                        result = UpdateType.ADDED;
                        break;
                    }
                } else {
                    // Update existing flow condition.
                    fc = oldfc.clone();
                    List<FlowMatch> matches = (fcond == null)
                        ? null : fcond.getMatches();
                    if (!fc.setMatches(matches)) {
                        // No change was made to flow condition.
                        return null;
                    }

                    if (flowCondDB.replace(name, oldfc, fc)) {
                        result = UpdateType.CHANGED;
                        break;
                    }
                }
            }

            // REVISIT: Select flow entries affected by the change.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }

            Status status = fc.saveConfig(this);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Flow condition was {}: {}",
                          containerName, name, result.getName(), fcond);
            } else {
                LOG.info("{}:{}: Flow condition was {}.",
                         containerName, name, result.getName());
            }
            FlowConditionEvent.raise(this, name, result);

            return result;
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the flow condition specified by the name.
     *
     * @param name  The name of the flow condition to be removed.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removeFlowCondition(String name) {
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkFlowConditionName(name);
            checkUpdate();

            FlowCondImpl fc = flowCondDB.remove(name);
            if (fc == null) {
                return flowConditionNotFound(name);
            }

            // REVISIT: Select flow entries affected by the change.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }

            fc.destroy(this);

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Flow condition was removed: {}",
                          containerName, name, fc.getFlowCondition());
            } else {
                LOG.info("{}:{}: Flow condition was removed.",
                         containerName, name);
            }
            FlowConditionEvent.raise(this, name, UpdateType.REMOVED);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a {@link FlowMatch} instance configured in the flow condition
     * specified by the flow condition name and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition
     *               in the flow condition.
     * @return  A {@link FlowMatch} instance which represents a flow match
     *          condition.
     *          {@code null} is returned if no flow match condition is
     *          configured at the specified match index.
     * @throws VTNException  An error occurred.
     */
    @Override
    public FlowMatch getFlowConditionMatch(String name, int index)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            FlowCondImpl fc = getFlowCondImplCheck(name);
            return fc.getMatch(index);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Configure a flow match condition into the flow condition specified
     * by the flow condition name and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition in
     *               the flow condition.
     * @param match  A {@link FlowMatch} instance which represents a flow
     *               match condition to be configured.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setFlowConditionMatch(String name, int index,
                                            FlowMatch match)
        throws VTNException {
        checkFlowConditionName(name);

        // Complete FlowMatch instance.
        FlowMatch mt;
        if (match == null) {
            // Create an empty flow match.
            mt = new FlowMatch(index, null, null, null);
        } else {
            // Ensure that the match index is assigned.
            mt = match.assignIndex(index);
        }

        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            UpdateType result;
            FlowCondImpl fc, oldfc;
            do {
                oldfc = getFlowCondImpl(name);
                fc = oldfc.clone();
                result = fc.setMatch(mt);
                if (result == null) {
                    // No change was made to flow condition.
                    return null;
                }
            } while (!flowCondDB.replace(name, oldfc, fc));

            // REVISIT: Select flow entries affected by the change.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }

            Status status = fc.saveConfig(this);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}.{}: Flow match condition was {}: {}",
                          containerName, name, index, result.getName(), mt);
            } else {
                LOG.info("{}:{}.{}: Flow match condition was {}.",
                         containerName, name, index, result.getName());
            }
            FlowConditionEvent.raise(this, name, index, result);

            return result;
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the flow match condition specified by the flow condition name
     * and match index.
     *
     * @param name   The name of the flow condition.
     * @param index  The match index that specifies flow match condition
     *               in the flow condition.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removeFlowConditionMatch(String name, int index) {
        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkFlowConditionName(name);
            checkUpdate();

            FlowCondImpl fc, oldfc;
            FlowMatchImpl fm;
            do {
                oldfc = getFlowCondImpl(name);
                fc = oldfc.clone();
                fm = fc.removeMatch(index);
                if (fm == null) {
                    // No change was made to flow condition.
                    return null;
                }
            } while (!flowCondDB.replace(name, oldfc, fc));

            // REVISIT: Select flow entries affected by the change.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }

            Status status = fc.saveConfig(this);
            if (status.isSuccess()) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}.{}: " +
                              "Flow match condition was removed: {}",
                              containerName, name, index, fm);
                } else {
                    LOG.info("{}:{}.{}: Flow match condition was removed.",
                             containerName, name, index);
                }

                FlowConditionEvent.raise(this, name, index,
                                         UpdateType.REMOVED);
            }

            return status;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return a list of path policy identifiers present in the container.
     *
     * @return  A list of {@link Integer} instances corresponding to all the
     *          path policies present in the container.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<Integer> getPathPolicyIds() throws VTNException {
        ArrayList<Integer> list;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            list = new ArrayList<Integer>(pathPolicyDB.keySet());
        } finally {
            rdlock.unlock();
        }

        // Sort path policy IDs.
        Collections.sort(list);
        return list;
    }

    /**
     * Return the configuration of the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return  A {@link PathPolicy} instance which contains the configuration
     *          of the specified path policy.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PathPolicy getPathPolicy(int id) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            PathPolicyImpl pp = getPathPolicyImpl(Integer.valueOf(id));
            return pp.getPathPolicy();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Create or modify the path policy.
     *
     * @param id      The identifier of the path policy.
     * @param policy  A {@link PathPolicy} instance which specifies the
     *                configuration of the path policy.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathPolicy(int id, PathPolicy policy)
        throws VTNException {
        Integer pid = Integer.valueOf(id);
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            UpdateType result;
            PathPolicyImpl pp;
            while (true) {
                PathPolicyImpl oldpp = pathPolicyDB.get(pid);
                if (oldpp == null) {
                    // Create a new path policy.
                    pp = new PathPolicyImpl(id, policy);
                    if (pathPolicyDB.putIfAbsent(pid, pp) == null) {
                        result = UpdateType.ADDED;
                        break;
                    }
                } else {
                    // Update existing path policy.
                    pp = oldpp.clone();
                    if (!pp.setPathPolicy(policy)) {
                        // No change was made to path policy.
                        return null;
                    }

                    if (pathPolicyDB.replace(pid, oldpp, pp)) {
                        result = UpdateType.CHANGED;
                        break;
                    }
                }
            }

            if (result == UpdateType.ADDED) {
                // Remove all flows because new path policy may affect
                // all present flows.
                for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                    VTNThreadData.removeFlows(this, fdb);
                }
            } else {
                // Remove all flows that was routed by the specified path
                // policy.
                PathPolicyFlowMatch fmatch = new PathPolicyFlowMatch(id);
                for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                    VTNThreadData.removeFlows(this, fdb, fmatch);
                }
            }

            updatePathPolicyMap(pid);

            Status status = pp.saveConfig(this);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Path policy was {}: {}",
                          containerName, pid, result.getName(), policy);
            } else {
                LOG.info("{}.{}: Path policy was {}.",
                         containerName, pid, result.getName());
            }
            PathPolicyEvent.raise(this, id, result);

            return result;
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the path policy specified by the identifier.
     *
     * @param id  The identifier of the path policy to be removed.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathPolicy(int id) {
        Integer pid = Integer.valueOf(id);
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            PathPolicyImpl pp = pathPolicyDB.remove(pid);
            if (pp == null) {
                return pathPolicyNotFound(pid);
            }

            // Remove all flows that was routed by the specified path policy.
            PathPolicyFlowMatch fmatch = new PathPolicyFlowMatch(id);
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb, fmatch);
            }

            pp.destroy(this);

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Path policy was removed: {}",
                          containerName, pid, pp.getPathPolicy());
            } else {
                LOG.info("{}.{}: Path policy was removed.",
                         containerName, pid);
            }
            PathPolicyEvent.raise(this, id, UpdateType.REMOVED);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }

        return new Status(StatusCode.SUCCESS, null);
    }


    /**
     * Return the default link cost configured in the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return    The default link cost configured in the specified path policy
     *            is returned. {@link PathPolicy#COST_UNDEF} means that the
     *            default cost should be determined by link speed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public long getPathPolicyDefaultCost(int id) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            PathPolicyImpl pp = getPathPolicyImpl(Integer.valueOf(id));
            return pp.getDefaultCost();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change the default link cost for the specified path policy.
     *
     * @param id    The identifier of the path policy.
     * @param cost  The default cost value to be set.
     * @return  {@code true} if the default cost was changed.
     *          {@code false} if not changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public boolean setPathPolicyDefaultCost(int id, long cost)
        throws VTNException {
        Integer pid = Integer.valueOf(id);

        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            Long old;
            PathPolicyImpl pp, oldpp;
            do {
                oldpp = getPathPolicyImpl(pid);
                pp = oldpp.clone();
                old = pp.setDefaultCost(cost);
                if (old == null) {
                    // No change was made to path policy.
                    return false;
                }
            } while (!pathPolicyDB.replace(pid, oldpp, pp));

            // Remove all flows that was routed by the specified path policy.
            PathPolicyFlowMatch fmatch = new PathPolicyFlowMatch(id);
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb, fmatch);
            }

            updatePathPolicyMap(pid);

            Status status = pp.saveConfig(this);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }

            LOG.info("{}.{}: Default path cost was changed: {} -> {}",
                     containerName, pid, old, cost);
            PathPolicyEvent.raise(this, id, UpdateType.CHANGED);

            return true;
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return the cost of transmitting a packet from the specified switch port
     * configured in the specified path policy.
     *
     * @param id    The identifier of the path policy.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @return  The cost of transmitting a packet from the specified physical
     *          switch port. Zero is returned if {@code ploc} is not configured
     *          in the specified path policy.
     * @throws VTNException  An error occurred.
     */
    @Override
    public long getPathPolicyCost(int id, PortLocation ploc)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            PathPolicyImpl pp = getPathPolicyImpl(Integer.valueOf(id));
            return pp.getPathCost(ploc);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Associate the cost of transmitting a packet with the specified switch
     * port in the specified path policy.
     *
     * <p>
     *   The specified cost value is used when a packet is transmitted from the
     *   switch port specified by a {@link PortLocation} instance.
     * </p>
     *
     * @param id    The identifier of the path policy to be removed.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @param cost  The cost of transmitting a packet.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathPolicyCost(int id, PortLocation ploc, long cost)
        throws VTNException {
        Integer pid = Integer.valueOf(id);

        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            UpdateType result;
            PathPolicyImpl pp, oldpp;
            do {
                oldpp = getPathPolicyImpl(pid);
                pp = oldpp.clone();
                result = pp.setPathCost(ploc, cost);
                if (result == null) {
                    // No change was made to path policy.
                    return null;
                }
            } while (!pathPolicyDB.replace(pid, oldpp, pp));

            // Remove all flows that was routed by the specified path policy.
            PathPolicyFlowMatch fmatch = new PathPolicyFlowMatch(id);
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb, fmatch);
            }

            updatePathPolicyMap(pid);

            Status status = pp.saveConfig(this);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }

            LOG.info("{}.{}: Path policy cost was {}: {} -> {}",
                     containerName, pid, result.getName(), ploc, cost);
            PathPolicyEvent.raise(this, id, UpdateType.CHANGED);

            return result;
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the cost associated with the specified switch port in the
     * specified path policy.
     *
     * @param id    The identifier of the path policy to be removed.
     * @param ploc  A {@link PortLocation} instance which specifies the
     *              location of the physical switch port.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathPolicyCost(int id, PortLocation ploc) {
        Integer pid = Integer.valueOf(id);

        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            PathPolicyImpl pp, oldpp;
            Long old;
            do {
                oldpp = getPathPolicyImpl(pid);
                pp = oldpp.clone();
                old = pp.removePathCost(ploc);
                if (old == null) {
                    // No change was made to flow condition.
                    return null;
                }
            } while (!pathPolicyDB.replace(pid, oldpp, pp));

            // Remove all flows that was routed by the specified path policy.
            PathPolicyFlowMatch fmatch = new PathPolicyFlowMatch(id);
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb, fmatch);
            }

            updatePathPolicyMap(pid);

            Status status = pp.saveConfig(this);
            if (status.isSuccess()) {
                LOG.info("{}.{}: Path policy cost was removed: {} -> {}",
                         containerName, pid, ploc, old);
                PathPolicyEvent.raise(this, id, UpdateType.CHANGED);
            }

            return status;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return a list of container path maps configured in the container.
     *
     * @return  A list of {@link PathMap} instances corresponding to all
     *          container path maps configured in the container.
     *          An empty list is returned if no container path map is
     *          configured.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<PathMap> getPathMaps() throws VTNException {
        ArrayList<PathMap> list;
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            list = new ArrayList<PathMap>(pathMapDB.size());
            for (ContainerPathMapImpl cpm: pathMapDB.values()) {
                list.add(cpm.getPathMap());
            }
        } finally {
            unlock(rdlock);
        }

        // Sort path maps by their indices.
        Collections.sort(list, new PathMapComparator());
        return list;
    }

    /**
     * Return information about the container path map specified by the index
     * number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @return  A {@link PathMap} instance corresponding to the specified
     *          container path map is returned.
     *          {@code null} is returned if the specified path map does not
     *          exist in the container.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PathMap getPathMap(int index) throws VTNException {
        Integer key = Integer.valueOf(index);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            ContainerPathMapImpl cpm = pathMapDB.get(key);
            return (cpm == null) ? null : cpm.getPathMap();
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Create or modify the container path map specified by the index number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @param pmap   A {@link PathMap} instance which specifies the
     *               configuration of the path map.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathMap(int index, PathMap pmap) throws VTNException {
        ContainerPathMapImpl cpm = new ContainerPathMapImpl(index, pmap);
        Integer key = Integer.valueOf(index);
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            UpdateType result;
            ContainerPathMapImpl oldcpm = pathMapDB.put(key, cpm);
            if (oldcpm == null) {
                result = UpdateType.ADDED;
            } else if (oldcpm.equals(cpm)) {
                // No change was made to path map.
                return null;
            } else {
                result = UpdateType.CHANGED;
            }

            // REVISIT: Select flow entries affected by the change.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }

            Status status = cpm.saveConfig(this);
            if (!status.isSuccess()) {
                throw new VTNException(status);
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Container path map was {}: {}",
                          containerName, key, result.getName(), pmap);
            } else {
                LOG.info("{}.{}: Container path map was {}.",
                         containerName, key, result.getName());
            }
            ContainerPathMapEvent.raise(this, index, result);

            return result;
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the container path map specified by the index number.
     *
     * @param index  The index value which specifies the path map in the
     *               container.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathMap(int index) {
        Integer key = Integer.valueOf(index);
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            ContainerPathMapImpl cpm = pathMapDB.remove(key);
            if (cpm == null) {
                return null;
            }

            // REVISIT: Select flow entries affected by the change.
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                VTNThreadData.removeFlows(this, fdb);
            }

            cpm.destroy(this);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Container path map was removed: {}",
                          containerName, key, cpm.getPathMap());
            } else {
                LOG.info("{}.{}: Container path map was removed.",
                         containerName, key);
            }
            ContainerPathMapEvent.raise(this, index, UpdateType.REMOVED);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Return a list of VTN path maps configured in the VTN.
     *
     * @param path  A {@link VTenantPath} object that specifies the position
     *              of the VTN.
     * @return  A list of {@link PathMap} instances corresponding to all
     *          VTN path maps configured in the VTN.
     *          An empty list is returned if no VTN path map is configured.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<PathMap> getPathMaps(VTenantPath path) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getPathMaps();
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Return information about the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @return  A {@link PathMap} instance corresponding to the specified
     *          VTN path map is returned.
     *          {@code null} is returned if the specified path map does not
     *          exist in the VTN.
     * @throws VTNException  An error occurred.
     */
    @Override
    public PathMap getPathMap(VTenantPath path, int index)
        throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            VTenantImpl vtn = getTenantImpl(path);
            return vtn.getPathMap(index);
        } finally {
            unlock(rdlock);
        }
    }

    /**
     * Create or modify the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @param pmap   A {@link PathMap} instance which specifies the
     *               configuration of the path map.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setPathMap(VTenantPath path, int index, PathMap pmap)
        throws VTNException {
        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.setPathMap(this, index, pmap);
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Remove the VTN path map specified by the index number.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @param index  The index value which specifies the path map in the VTN.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removePathMap(VTenantPath path, int index) {
        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(path);
            return vtn.removePathMap(this, index);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }
    }

    /**
     * Return a list of flow filters configured in the specified flow filter
     * list.
     *
     * @param fid  A {@link FlowFilterId} instance which specifies the
     *             flow filter list in the virtual node.
     * @return  A list of {@link FlowFilter} instances corresponding to all
     *          flow filters configured in the list specified by {@code fid}.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<FlowFilter> getFlowFilters(FlowFilterId fid)
        throws VTNException {
        LockStack lstack = new LockStack();
        lstack.push(rwLock.readLock());
        try {
            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            return ffmap.getAll();
        } finally {
            lstack.clear();
        }
    }

    /**
     * Return information about the flow filter specified by the index number.
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index  The index value which specifies the flow filter in the
     *               flow filter list specified by {@code fid}.
     * @return  A {@link FlowFilter} instance corresponding to the flow filter
     *          specified by {@code fid} and {@code index}.
     *          {@code null} is returned if the specified flow filter does not
     *          exist.
     * @throws VTNException  An error occurred.
     */
    @Override
    public FlowFilter getFlowFilter(FlowFilterId fid, int index)
        throws VTNException {
        LockStack lstack = new LockStack();
        lstack.push(rwLock.readLock());
        try {
            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            return ffmap.get(index);
        } finally {
            lstack.clear();
        }
    }

    /**
     * Create or modify the flow filter specified by the index number.
     *
     * @param fid     A {@link FlowFilterId} instance which specifies the
     *                flow filter list in the virtual node.
     * @param index   The index value which specifies the flow filter in the
     *                flow filter list.
     * @param filter  A {@link FlowFilter} instance which specifies the
     *                configuration of the flow filter.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setFlowFilter(FlowFilterId fid, int index,
                                    FlowFilter filter) throws VTNException {
        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        LockStack lstack = new LockStack();
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            UpdateType result = ffmap.set(this, index, filter);
            if (result != null) {
                export(vtn);
                Status status = vtn.saveConfigImpl(null);
                if (!status.isSuccess()) {
                    throw new VTNException(status);
                }
            }
            return result;
        } finally {
            lstack.clear();
            data.cleanUp(this);
        }
    }

    /**
     * Remove the flow filter specified by the index number.
     *
     * @param fid    A {@link FlowFilterId} instance which specifies the
     *               flow filter list in the virtual node.
     * @param index  The index value which specifies the flow filter in the
     *               flow filter list specified by {@code fid}.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status removeFlowFilter(FlowFilterId fid, int index) {
        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        LockStack lstack = new LockStack();
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            Status result = ffmap.remove(this, index);
            if (result != null) {
                export(vtn);
                Status svres = vtn.saveConfigImpl(null);
                if (!svres.isSuccess()) {
                    return svres;
                }
            }
            return result;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            lstack.clear();
            data.cleanUp(this);
        }
    }

    /**
     * Remove all the flow filters present in the specified flow filter list.
     *
     * @param fid  A {@link FlowFilterId} instance which specifies the
     *             flow filter list in the virtual node.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearFlowFilter(FlowFilterId fid) {
        // Acquire writer lock in order to block receiveDataPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        LockStack lstack = new LockStack();
        try {
            checkUpdate();

            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            Status result = ffmap.clear(this);
            if (result != null) {
                export(vtn);
                Status svres = vtn.saveConfigImpl(null);
                if (!svres.isSuccess()) {
                    return svres;
                }
            }
            return result;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            lstack.clear();
            data.cleanUp(this);
        }
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

    // RouteResolver

    /**
     * {@inheritDoc}
     */
    @Override
    public Path getRoute(Node src, Node dst) {
        return routing.getRoute(src, dst);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getPathPolicyId() {
        return RouteResolver.ID_DEFAULT;
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
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            Status status = new Status(StatusCode.SUCCESS, null);

            // Save VTN configurations.
            for (VTenantImpl vtn: tenantDB.values()) {
                Status st = vtn.saveConfig(null);
                if (!st.isSuccess()) {
                    status = st;
                }
            }

            // Save flow condition configurations.
            for (FlowCondImpl fc: flowCondDB.values()) {
                Status st = fc.saveConfig(this);
                if (!st.isSuccess()) {
                    status = st;
                }
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
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        Lock wrlock = rwLock.writeLock();
        UpdateType utype = type;
        wrlock.lock();
        try {
            // Maintain the node DB.
            if (type == UpdateType.REMOVED) {
                if (!removeNode(node)) {
                    return;
                }

                // Uninstall VTN flows related to the removed node.
                for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                    fdb.removeFlows(this, node);
                }

                // Flush MAC address table entries detected on the removed
                // node.
                for (MacAddressTable table: macTableMap.values()) {
                    table.flush(node);
                }
            } else {
                if (!addNode(node)) {
                    return;
                }

                if (connectionManager.getLocalityStatus(node) ==
                    ConnectionLocality.LOCAL) {
                    // Remove all VTN flows related to this node because
                    // all flow entries in this node should be removed by
                    // protocol plugin.
                    for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                        fdb.removeFlows(this, node);
                    }
                }

                utype = UpdateType.ADDED;
            }

            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNode(this, node, utype);
            }
        } finally {
            unlock(wrlock);
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
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        UpdateType utype = type;
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Maintain the port DB.
            VNodeState pstate;
            if (type == UpdateType.REMOVED) {
                if (!removePort(nc)) {
                    return;
                }

                pstate = VNodeState.UNKNOWN;
            } else {
                utype = addPort(nc, propMap);
                if (utype == null) {
                    return;
                }

                // Determine whether the port is up or not.
                pstate = (isEnabled(nc)) ? VNodeState.UP : VNodeState.DOWN;
            }

            if (pstate != VNodeState.UP) {
                // Uninstall VTN flows related to the switch port.
                for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                    fdb.removeFlows(this, nc);
                }

                // Flush MAC address table entries detected on the switch port.
                for (MacAddressTable table: macTableMap.values()) {
                    table.flush(nc);
                }
            }

            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNodeConnector(this, nc, pstate, utype);
            }
        } finally {
            unlock(wrlock);
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
        HashMap<NodeConnector, Boolean> islMap =
            new HashMap<NodeConnector, Boolean>();

        // Maintain the inter switch link DB.
        if (!updateISL(topoList, islMap)) {
            return;
        }

        EdgeUpdateState estate = new EdgeUpdateState(islMap);

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            // Uninstall VTN flows related to the switch port that was
            // changed its link state.
            for (NodeConnector port: islMap.keySet()) {
                for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                    fdb.removeFlows(this, port);
                }
            }

            // Flush MAC address table entries detected on the switch port
            // that was changed to ISL port.
            for (MacAddressTable table: macTableMap.values()) {
                table.flush(estate);
            }

            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.edgeUpdate(this, estate);
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

        Lock wrlock = rwLock.writeLock();
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
            NodeUtils.checkNodeConnector(nc, false);
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
                LOG.trace("{}: Ignore non-ethernet packet: {}", containerName,
                          decoded);
            }
            return PacketResult.IGNORED;
        }

        // Create a packet context.
        PacketContext pctx = new PacketContext(inPkt, (Ethernet)decoded);
        Packet payload = pctx.getPayload();
        if (payload instanceof LLDP) {
            return PacketResult.IGNORED;
        }

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
            NodeUtils.checkNodeType(node.getType());
            NodeUtils.checkNodeConnectorType(nc.getType());

            if (!isEdgePortImpl(nc)) {
                LOG.debug("{}: Ignore packet from internal node connector: {}",
                          containerName, nc);
                if (connectionManager.getLocalityStatus(node) ==
                    ConnectionLocality.LOCAL) {
                    // This PACKET_IN may be caused by an obsolete flow entry.
                    // So all flow entries related to this port should be
                    // removed.
                    for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                        fdb.removeFlows(this, nc);
                    }
                }
                return PacketResult.IGNORED;
            }

            // Determine virtual network mapping that maps the packet.
            short vlan = pctx.getVlan();
            MapReference ref = resourceManager.getMapReference(src, nc, vlan);
            if (ref != null && containerName.equals(ref.getContainerName())) {
                pctx.setMapReference(ref);
                VNodePath path = ref.getPath();
                VTenantImpl vtn = getTenantImpl(path);
                return vtn.receive(this, ref, pctx);
            }
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.error("{}: Ignore packet: {}", containerName,
                      status.getDescription());
        } catch (Exception e) {
            logException(LOG, null, e, pctx.getDescription());
        } finally {
            unlock(rdlock);
        }

        return PacketResult.IGNORED;
    }

    // IListenRoutingUpdates

    /**
     * Invoked when the recalculation of the all shortest path tree is done.
     */
    @Override
    public void recalculateDone() {
        LOG.trace("{}: Shortest path recalculated", containerName);

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
        if (containerManager != null && containerManager.inContainerMode()) {
            // The given flow was removed by forwarding rule manager, and it
            // will be restored when the controller exits the container mode.
            // Note that we can not use inContainerMode variable here because
            // containerModeUpdated() handler for FRM may be called before
            // the VTN Manager. Although this code may miss FLOW_REMOVED
            // notifications actually sent by OF switch, they will be fixed
            // when the controller quits the container mode.
            assert containerName.equals(GlobalConstants.DEFAULT.toString());
            LOG.trace("{}: Ignore FLOW_REMOVED during container mode: " +
                      "node={}, flow={}", containerName, node, flow);
            return;
        }

        LOG.trace("{}: flowRemoved() called: node={}, flow={}",
                  containerName, node, flow);

        String empty = "";
        FlowEntry entry = new FlowEntry(empty, empty, flow, node);
        for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
            if (fdb.flowRemoved(this, entry, false)) {
                return;
            }
        }

        // Below are workaround for a bug of old version of Open vSwitch.
        Flow fixedFlow = VTNFlowDatabase.fixBrokenOvsFlow(flow);
        if (fixedFlow != null) {
            entry = new FlowEntry(empty, empty, fixedFlow, node);
            for (VTNFlowDatabase fdb: vtnFlowMap.values()) {
                // In this case we need to uninstall ingress flow too because
                // it may be still kept by the forwarding rule manager.
                if (fdb.flowRemoved(this, entry, true)) {
                    return;
                }
            }
        }

        if (flow.getIdleTimeout() != 0) {
            LOG.trace("{}: Expired flow not found: node={}, flow={}",
                      containerName, node, flow);
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
