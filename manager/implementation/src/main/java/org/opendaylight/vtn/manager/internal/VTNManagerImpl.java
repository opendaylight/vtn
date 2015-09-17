/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.Dictionary;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.felix.dm.Component;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
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
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;
import org.opendaylight.vtn.manager.util.VTNIdentifiableComparator;

import org.opendaylight.vtn.manager.internal.config.VTNConfigImpl;
import org.opendaylight.vtn.manager.internal.flow.remove.PortFlowRemover;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryListener;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.packet.PacketInEvent;
import org.opendaylight.vtn.manager.internal.packet.VTNPacketListener;
import org.opendaylight.vtn.manager.internal.routing.RoutingEvent;
import org.opendaylight.vtn.manager.internal.routing.VTNRoutingListener;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.concurrent.AbstractVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowMatch;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.ArpPacketBuilder;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathCostConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.tx.DeleteDataTask;
import org.opendaylight.vtn.manager.internal.util.tx.PutDataTask;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;

import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.FlowFilterMap;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.clustering.services.CacheConfigException;
import org.opendaylight.controller.clustering.services.CacheExistException;
import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.clustering.services.
    IClusterContainerServices;
import org.opendaylight.controller.clustering.services.IClusterServices;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.hosttracker.IfHostListener;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.ClearFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.RemoveFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionMatchOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.remove.flow.condition.match.output.RemoveMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.input.FlowMatchList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.set.flow.condition.match.output.SetMatchResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.DataFlowMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowCountOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.GetDataFlowOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.output.DataFlowInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.ClearPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.RemovePathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.SetPathMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.remove.path.map.output.RemovePathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.output.SetPathMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.remove.path.cost.output.RemovePathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Implementation of VTN Manager service.
 */
public class VTNManagerImpl
    implements IVTNManager, ICacheUpdateAware<ClusterEventId, Object>,
               VTNInventoryListener, VTNRoutingListener, VTNPacketListener,
               IConfigurationContainerAware, IHostFinder {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VTNManagerImpl.class);

    /**
     * Maximum lifetime, in milliseconds, of a cluster event.
     */
    private static final long CLUSTER_EVENT_LIFETIME = 1000L;

    /**
     * The number of seconds to wait for completion of RPC.
     */
    private static final long  RPC_TIMEOUT = 60L;

    /**
     * Cluster cache name associated with {@link #tenantDB}.
     */
    static final String  CACHE_TENANT = "vtn.tenant";

    /**
     * Cluster cache name associated with {@link #stateDB}.
     */
    static final String  CACHE_STATE = "vtn.state";

    /**
     * The name of the cluster cache which keeps MAC address table entries.
     */
    static final String CACHE_MAC = "vtn.mac";

    /**
     * Cluster cache name associated with {@link #clusterEvent}.
     */
    static final String  CACHE_EVENT = "vtn.clusterEvent";

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
     * MD-SAL VTN Manager service provider.
     */
    private VTNManagerProvider  vtnProvider;

    /**
     * Cluster container service instance.
     */
    private IClusterContainerServices  clusterService;

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
     * Container name associated with this service.
     */
    private String  containerName;

    /**
     * Set true if the VTN Manager runs in a non-default container.
     */
    private boolean  nonDefault;

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
     * True if the VTN Manager service is available.
     */
    private volatile boolean  serviceAvailable;

    /**
     * List of cluster events.
     */
    private final List<ClusterEvent>  clusterEventQueue =
        new ArrayList<ClusterEvent>();

    /**
     * MAC address tables associated with vBridges.
     */
    private final ConcurrentMap<VBridgePath, MacAddressTable> macTableMap =
        new ConcurrentHashMap<VBridgePath, MacAddressTable>();

    /**
     * A thread which executes queued tasks.
     */
    private static final class TaskQueueThread extends Thread {
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
                    // Ignore interruption.
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
        nonDefault = !cname.equals(GlobalConstants.DEFAULT.toString());
        if (nonDefault) {
            LOG.trace("{}: Nothing to do in a non-default container.", cname);
            return;
        }

        // Initialize configuration directory for the container.
        ContainerConfig cfg = new ContainerConfig(cname);
        cfg.init();
        createCaches();

        // Start VTN task thread.
        taskQueueThread = new TaskQueueThread("VTN Task Thread: " + cname);
        taskQueueThread.start();

        if (vtnProvider != null) {
            TxTask<Void> initTask = new AbstractTxTask<Void>() {
                @Override
                protected Void execute(TxContext ctx) throws VTNException {
                    // Load saved configurations.
                    loadConfig(ctx);
                    return null;
                }
            };

            VTNFuture<Void> f = vtnProvider.postFirst(initTask);
            vtnProvider.setVTNManager(this);
            try {
                f.checkedGet();
            } catch (Exception e) {
                LOG.error(cname + ": Failed to load configuration.", e);
            }

            vtnProvider.configLoaded();
        }

        // Initialize MAC address tables.
        initMacAddressTable();

        resourceManager.addManager(this);
        if (vtnProvider != null) {
            serviceAvailable = true;
        }
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

        if (vtnProvider != null) {
            vtnProvider.shutdown();
        }
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
    }

    /**
     * Function called by the dependency manager when at least one dependency
     * become unsatisfied or when the component is shutting down because for
     * example bundle is being stopped.
     */
    void destroy() {
        LOG.trace("{}: destroy() called", containerName);
        if (!nonDefault) {
            return;
        }

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
            clusterEvent =
                new ConcurrentHashMap<ClusterEventId, ClusterEvent>();
            macAddressDB = null;
            return;
        }

        // Create transactional cluster caches.
        Set<IClusterServices.cacheMode> cmode =
            EnumSet.of(IClusterServices.cacheMode.TRANSACTIONAL,
                       IClusterServices.cacheMode.SYNC);
        createCache(cluster, CACHE_TENANT, cmode);
        createCache(cluster, CACHE_STATE, cmode);

        tenantDB = (ConcurrentMap<String, VTenantImpl>)
            getCache(cluster, CACHE_TENANT);
        stateDB = (ConcurrentMap<VTenantPath, Object>)
            getCache(cluster, CACHE_STATE);

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

        clusterEvent = (ConcurrentMap<ClusterEventId, ClusterEvent>)
            getCache(cluster, CACHE_EVENT);

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
            LOG.debug("Cache already exists: name=", cacheName, e);
        } catch (CacheConfigException e) {
            String msg = "Invalid cache configuration: name=" + cacheName +
                ", mode=" + cmode;
            LOG.error(msg, e);
        } catch (Exception e) {
            String msg = "Failed to create cache: name=" + cacheName;
            LOG.error(msg, e);
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
     * Return the name of the container associated with this service.
     *
     * @return The name of the container.
     */
    public String getContainerName() {
        return containerName;
    }

    /**
     * Invoked when a VTN Manager provider is registered.
     *
     * @param provider  VTN Manager provider service.
     */
    void setVTNProvider(VTNManagerProvider provider) {
        LOG.trace("{}: Set VTN Manager provider: {}", containerName, provider);
        vtnProvider = provider;
    }

    /**
     * Invoked when a VTN Manager provider is unregistered.
     *
     * @param provider  VTN Manager provider service.
     */
    void unsetVTNProvider(VTNManagerProvider provider) {
        if (provider != null && provider.equals(vtnProvider)) {
            LOG.trace("{}: Unset VTN Manager provider: {}",
                      containerName, provider);
            provider.close();
            vtnProvider = null;
        }
    }

    /**
     * Return VTN Manager provider.
     *
     * @return  VTN Manager provider instance.
     */
    public VTNManagerProvider getVTNProvider() {
        return vtnProvider;
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
     * Let the specified VTN visible to other controllers in the cluster.
     *
     * @param vtn  A virtual tenant.
     */
    public void export(VTenantImpl vtn) {
        tenantDB.put(vtn.getName(), vtn);
    }

    /**
     * Return a {@link VTNConfig} object which contains global configuration.
     *
     * @return  A {@link VTNConfig} object.
     */
    public VTNConfig getVTNConfig() {
        return (vtnProvider == null)
            ? new VTNConfigImpl() : vtnProvider.getVTNConfig();
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
     * Transmit an ethernet frame to the specified node connector.
     *
     * @param egress  A {@link SalPort} instance which specifies the egress
     *                switch port.
     * @param ether   An ethernet frame.
     */
    public void transmit(SalPort egress, Ethernet ether) {
        if (vtnProvider != null) {
            vtnProvider.transmit(egress, ether);
        }
    }

    /**
     * Transmit an ethernet frame to the specified node connector.
     *
     * @param nc     A node connector.
     * @param ether  An ethernet frame.
     */
    public void transmit(NodeConnector nc, Ethernet ether) {
        if (vtnProvider != null) {
            SalPort egress = SalPort.create(nc);
            if (egress == null) {
                // This should never happen.
                LOG.error("transmit: Unsupported switch port: {}", nc);
            } else {
                vtnProvider.transmit(egress, ether);
            }
        }
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
            try {
                vtn.saveConfig(this);
            } catch (VTNException e) {
                LOG.warn("Failed to save VTN configuration: " + tenantName, e);
            }
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
        long limit = current + (long)getVTNConfig().getInitTimeout();
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
     *
     * @param ctx  A {@link TxContext} instance.
     * @throws VTNException
     *   Failed to load configuration.
     */
    private void loadConfig(TxContext ctx) throws VTNException {
        // Use a null tenant path for synchronization of cluster cache
        // initialization.
        VTenantPath path = new VTenantPath(null);
        InetAddress myaddr = resourceManager.getControllerAddress();
        ContainerConfig cfg = new ContainerConfig(containerName);
        if (waitForCache(path, myaddr)) {
            // Load VTN configurations.
            for (String name: cfg.getKeys(ContainerConfig.Type.TENANT)) {
                loadTenantConfig(ctx, cfg, name);
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
                vtn.resume(this, ctx);
                names.add(vtn.getName());

                // Save configuration for this tenant.
                vtn.saveConfig(null);
            }

            // Remove configuration files for obsolete tenants.
            cfg.deleteAll(ContainerConfig.Type.TENANT, names);
        }
    }

    /**
     * Load configuration for the specified virtual tenant.
     *
     * @param ctx         A {@link TxContext} instance.
     * @param cfg         A {@link ContainerConfig} instance.
     * @param tenantName  The name of the tenant.
     * @throws VTNException
     *   Failed to load configuration.
     */
    private void loadTenantConfig(TxContext ctx, ContainerConfig cfg,
                                  String tenantName) throws VTNException {
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
            vtn.resume(this, ctx);
        }
    }

    /**
     * Ensure that the given tenant configuration is not null.
     *
     * @param tconf  Tenant configuration
     * @throws VTNException  {@code tconf} is {@code null}.
     */
    private void checkTenantConfig(VTenantConfig tconf) throws VTNException {
        if (tconf == null) {
            throw RpcException.getNullArgumentException(
                "Tenant configuration");
        }
    }

    /**
     * Ensure that the VTN Manager runs on the default container.
     *
     * @throws VTNException
     *    VTN Manager service runs in a non-default container.
     */
    private void checkDefault() throws VTNException {
        if (nonDefault) {
            throw new VTNException(VtnErrorTag.NOTACCEPTABLE,
                                   "Non-default container is not supported.");
        }
    }

    /**
     * Check whether the VTN Manager service is available or not.
     *
     * @return  VTN Manager provider service.
     * @throws VTNException   VTN Manager service is not available.
     */
    private VTNManagerProvider checkService() throws VTNException {
        checkDefault();

        VTNManagerProvider provider = vtnProvider;
        if (provider == null || !serviceAvailable) {
            throw new VTNException(VtnErrorTag.NOSERVICE,
                                   "VTN service is not available");
        }

        return provider;
    }

    /**
     * Return an exception that indicates the specified tenant does not exist.
     *
     * @param tenantName  The name of the tenant.
     * @return  A {@link RpcException} instance.
     */
    private RpcException getTenantNotFoundException(String tenantName) {
        String msg = tenantName + ": Tenant does not exist";
        return RpcException.getNotFoundException(msg);
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
            throw RpcException.getNullArgumentException("Flow filter ID");
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
        String tenantName = VTenantUtils.getName(path);
        VTenantImpl vtn = tenantDB.get(tenantName);
        if (vtn == null) {
            throw getTenantNotFoundException(tenantName);
        }

        return vtn;
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
     * Run the specified task on the global async thread pool.
     *
     * @param r  A runnable to be run on the global async thread pool.
     */
    public void postAsync(Runnable r) {
        if (vtnProvider != null && !vtnProvider.executeTask(r)) {
            LOG.error("{}: Async task was rejected: {}", containerName, r);
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

                if (vtnProvider != null) {
                    Timer timer = vtnProvider.getTimer();
                    timer.schedule(task, CLUSTER_EVENT_LIFETIME);
                }
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
        List<ClusterEvent> list;
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
            final List<ClusterEventId> ids =
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

            if (vtnProvider != null) {
                Timer timer = vtnProvider.getTimer();
                timer.schedule(task, CLUSTER_EVENT_LIFETIME);
            }
        }
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
                // Save configurations.
                saveTenantConfigLocked(tenantName);
            } else {
                // Delete the virtual tenant configuration file.
                ContainerConfig cfg = new ContainerConfig(containerName);
                cfg.delete(ContainerConfig.Type.TENANT, tenantName);

                if (vtnProvider != null) {
                    // Purge canceled timer tasks.
                    vtnProvider.getTimer().purge();
                }
            }
        } finally {
            unlock(wrlock);
        }
    }

    /**
     * Flush pending cluster events, and then unlock the given lock object.
     *
     * @param lock  A lock object.
     */
    void unlock(Lock lock) {
        try {
            flushEventQueue();
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
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f    A {@link Future} instance associated with the RPC task.
     * @param <T>  The type of the RPC output.
     * @return  The output of the RPC task.
     * @throws VTNException  An error occurred.
     */
    private <T> T getRpcOutput(Future<RpcResult<T>> f) throws VTNException {
        return getRpcOutput(f, false);
    }

    /**
     * Wait for completion of the RPC task associated with the given future.
     *
     * @param f         A {@link Future} instance associated with the RPC task.
     * @param nullable  Set {@code true} if the result can be null.
     * @param <T>  The type of the RPC output.
     * @return  The output of the RPC task.
     * @throws VTNException  An error occurred.
     */
    private <T> T getRpcOutput(Future<RpcResult<T>> f, boolean nullable)
        throws VTNException {
        RpcResult<T> result;
        try {
            result = f.get(RPC_TIMEOUT, TimeUnit.SECONDS);
        } catch (Exception e) {
            throw AbstractVTNFuture.getException(e);
        }

        if (result == null) {
            // This should never happen.
            throw new VTNException("RPC did not set result.");
        }

        if (result.isSuccessful()) {
            T res = result.getResult();
            if (!nullable && res == null) {
                // This should never happen.
                throw new VTNException("RPC did not set output.");
            }

            return res;
        }

        Collection<RpcError> errors = result.getErrors();
        if (errors == null || errors.isEmpty()) {
            // This should never happen.
            String msg = "RPC failed without error information: " + result;
            throw new VTNException(msg);
        }

        // VtnErrorTag should be encoded in application tag.
        RpcError rerr = errors.iterator().next();
        String msg = rerr.getMessage();
        String appTag = rerr.getApplicationTag();
        VtnErrorTag etag;
        try {
            etag = VtnErrorTag.valueOf(appTag);
        } catch (Exception e) {
            LOG.trace("Unknown application error tag in RpcError.", e);

            Throwable cause = rerr.getCause();
            String m = "RPC failed due to unexpected error: type=";
            StringBuilder builder = new StringBuilder(m).
                append(rerr.getErrorType()).
                append(", severity=").append(rerr.getSeverity()).
                append(", msg=").append(msg).
                append(", tag=").append(rerr.getTag()).
                append(", appTag=").append(appTag).
                append(", info=").append(rerr.getInfo());

            String emsg = builder.toString();
            LOG.error(emsg, cause);
            throw new VTNException(emsg, cause);
        }

        throw new VTNException(etag, msg);
    }

    /**
     * Return the element at the given index in the given list.
     *
     * <p>
     *   This method is used to get data fro the list generated by the RPC.
     * </p>
     *
     * @param list      A list generated by the RPC.
     * @param index     Index to the target element.
     * @param nullable  Set {@code true} if the result can be null.
     * @param <T>       The type of elements in the list.
     * @return  An element at the specified index in the list.
     * @throws VTNException  An error occurred.
     */
    private <T> T getRpcOutput(List<T> list, int index, boolean nullable)
        throws VTNException {
        if (list == null) {
            throw new VTNException("RPC did not set result.");
        }

        if (index >= list.size()) {
            throw new VTNException("Unexpected number of RPC results: " +
                                   list.size());
        }

        T ret = list.get(index);
        if (!nullable && ret == null) {
            String msg = String.format("RPC set null into result[%d]", index);
            throw new VTNException(msg);
        }

        return ret;
    }

    /**
     * Set the given data flow filter into the get-data-flow RPC input.
     *
     * @param builder  A input builder for get-data-flow RPC.
     * @param filter   A {@link DataFlowFilter} instance.
     * @return  {@code true} on success.
     *          {@code false} if the given data flow filter contains invalid
     *          value.
     */
    private boolean setInput(GetDataFlowInputBuilder builder,
                             DataFlowFilter filter) {
        // Set condition for physical switch.
        Node node = filter.getNode();
        if (node != null) {
            SalNode snode = SalNode.create(node);
            if (snode == null) {
                // Unsupported node is specified.
                return false;
            }

            builder.setNode(snode.getNodeId());

            SwitchPort swport = filter.getSwitchPort();
            if (swport != null) {
                String type = swport.getType();
                if (type != null &&
                    !NodeConnectorIDType.OPENFLOW.equals(type)) {
                    // Unsupported node-connector type.
                    return false;
                }

                try {
                    DataFlowPort dfp = new DataFlowPortBuilder().
                        setPortId(swport.getId()).
                        setPortName(swport.getName()).
                        build();
                    builder.setDataFlowPort(dfp);
                } catch (IllegalArgumentException e) {
                    LOG.trace("Invalid port name is specified: " + swport, e);
                    return false;
                }
            }
        }

        // Set condition for source L2 host.
        try {
            builder.setDataFlowSource(
                FlowUtils.toDataFlowSource(filter.getSourceHost()));
        } catch (RpcException e) {
            // The source host is specified by unsupported address type,
            // or an invalid VLAN ID is specified.
            LOG.trace("Unsupported source host: " + filter.getSourceHost(), e);
            return false;
        }

        return true;
    }

    /**
     * Invoke get-data-flow RPC.
     *
     * @param provider  VTN Manager provider service.
     * @param input     The input of get-data-flow RPC.
     * @return  A list of {@link DataFlowInfo} instances.
     * @throws VTNException  An error occurred.
     */
    private List<DataFlowInfo> getDataFlows(VTNManagerProvider provider,
                                            GetDataFlowInput input)
        throws VTNException {
        // Invoke RPC and await its completion.
        VtnFlowService rpc = provider.getVtnRpcService(VtnFlowService.class);
        GetDataFlowOutput output = getRpcOutput(rpc.getDataFlow(input));

        List<DataFlowInfo> result = output.getDataFlowInfo();
        if (result == null) {
            // This should never happen.
            throw new VTNException("Data flow list is unavailable.");
        }

        return result;
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
                    LOG.debug("Caught an exception while updating VBridge.",
                              e);
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
        return !tenantDB.isEmpty();
    }

    /**
     * Return a list of virtual tenant configurations.
     *
     * @return  A list which contains tenant configurations.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<VTenant> getTenants() throws VTNException {
        checkDefault();

        List<VTenant> list;
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
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);
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
        checkDefault();

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
     * {@code AddTenantTask} describes the MD-SAL datastore transaction task
     * that creates a VTN.
     *
     * <p>
     *   Note that this is tentative implementation.
     *   This class will be removed when the VTN Manager is fully migrated to
     *   MD-SAL.
     * </p>
     */
    private static final class AddTenantTask extends PutDataTask<Vtn> {
        /**
         * Construct a new instance.
         *
         * @param vname  A {@link VnodeName} instance that contains the name of
         *               the VTN.
         */
        private AddTenantTask(VnodeName vname) {
            super(LogicalDatastoreType.OPERATIONAL,
                  VTenantUtils.getIdentifier(vname),
                  new VtnBuilder().setName(vname).build(), true);
        }

        // PutDataTask

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean fixMissingParents() {
            return true;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onStarted(TxContext ctx, Vtn current)
            throws VTNException {
            if (current != null) {
                String name = VTenantUtils.getName(getTargetPath());
                throw VTenantUtils.getNameConflictException(name);
            }
        }
    }

    /**
     * {@code RemoveTenantTask} describes the MD-SAL datastore transaction task
     * that removes a VTN.
     *
     * <p>
     *   Note that this is tentative implementation.
     *   This class will be removed when the VTN Manager is fully migrated to
     *   MD-SAL.
     * </p>
     */
    private static final class RemoveTenantTask extends DeleteDataTask<Vtn> {
        /**
         * Construct a new instance.
         *
         * @param path  Path to the target VTN.
         */
        private RemoveTenantTask(InstanceIdentifier<Vtn> path) {
            super(LogicalDatastoreType.OPERATIONAL, path);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onFailure(VTNManagerProvider provider, Throwable t) {
            String name = VTenantUtils.getName(getTargetPath());
            LOG.warn(name + ": Failed to remove VTN from MD-SAL datastore.",
                     t);
        }
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
        // Put a new VTN into the MD-SAL datastore.
        String tenantName;
        VnodeName vname;
        VTNManagerProvider provider;
        try {
            provider = checkService();
            tenantName = VTenantUtils.getName(path);
            vname = VTenantUtils.checkName(tenantName);
            AddTenantTask task = new AddTenantTask(vname);
            VTNFuture<VtnUpdateType> f = provider.post(task);
            f.checkedGet();
        } catch (VTNException e) {
            return e.getStatus();
        }

        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        VTNFuture<?> rmf;
        Status status;
        try {
            checkTenantConfig(tconf);
            VTenantImpl vtn = new VTenantImpl(containerName, tenantName,
                                              tconf);
            if (tenantDB.putIfAbsent(tenantName, vtn) != null) {
                throw VTenantUtils.getNameConflictException(tenantName);
            }

            vtn.saveConfig(null);
            VTenant vtenant = vtn.getVTenant();
            enqueueEvent(path, vtenant, UpdateType.ADDED);

            return new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            InstanceIdentifier<Vtn> vpath = VTenantUtils.getIdentifier(vname);
            RemoveTenantTask task = new RemoveTenantTask(vpath);
            rmf = provider.post(task);
            status = e.getStatus();
        } finally {
            data.cleanUp(this);
        }

        try {
            rmf.checkedGet();
        } catch (VTNException e) {
            // Ignore error.
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
        VTNThreadData data = VTNThreadData.create(rwLock.readLock());
        try {
            checkTenantConfig(tconf);
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.setVTenantConfig(this, path, tconf, all);
            return new Status(StatusCode.SUCCESS, null);
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
        VTNFuture<?> rmf;
        try {
            String tenantName = VTenantUtils.getName(path);
            VTNManagerProvider provider = checkService();

            // Make the specified tenant invisible.
            VTenantImpl vtn = tenantDB.remove(tenantName);
            if (vtn == null) {
                throw getTenantNotFoundException(tenantName);
            }

            // Remove the specified VTN from the MD-SAL datastore.
            InstanceIdentifier<Vtn> vpath =
                VTenantUtils.getIdentifier(tenantName);
            RemoveTenantTask task = new RemoveTenantTask(vpath);
            rmf = provider.post(task);

            VTenant vtenant = vtn.getVTenant();
            vtn.destroy(this);

            // Purge all VTN flows in this tenant.
            VTNThreadData.removeFlows(this, tenantName);
            enqueueEvent(path, vtenant, UpdateType.REMOVED);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
        }

        try {
            rmf.checkedGet();
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

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
        checkDefault();

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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.addBridge(this, path, bconf);
            return new Status(StatusCode.SUCCESS, null);
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.modifyBridge(this, path, bconf, all);
            return new Status(StatusCode.SUCCESS, null);
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeBridge(this, path);
            return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

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
        checkDefault();

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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.addTerminal(this, path, vtconf);
            return new Status(StatusCode.SUCCESS, null);
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.modifyTerminal(this, path, vtconf, all);
            return new Status(StatusCode.SUCCESS, null);
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeTerminal(this, path);
            return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

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
        checkDefault();

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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.addInterface(this, path, iconf);
            return new Status(StatusCode.SUCCESS, null);
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
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();
            VTenantImpl vtn = getTenantImpl(path);
            ctx = provider.newTxContext();
            vtn.modifyInterface(this, ctx, path, iconf, all);
            return new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
            if (ctx != null) {
                ctx.cancelTransaction();
            }
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeInterface(this, path);
            return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

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
        checkDefault();

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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.addInterface(this, path, iconf);
            return new Status(StatusCode.SUCCESS, null);
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
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();
            VTenantImpl vtn = getTenantImpl(path);
            ctx = provider.newTxContext();
            vtn.modifyInterface(this, ctx, path, iconf, all);
            return new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
            if (ctx != null) {
                ctx.cancelTransaction();
            }
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeInterface(this, path);
            return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

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
        checkDefault();

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
        checkDefault();

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
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();
            VTenantImpl vtn = getTenantImpl(path);
            ctx = provider.newTxContext();
            return vtn.addVlanMap(this, ctx, path, vlconf);
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
            checkService();

            VTenantImpl vtn = getTenantImpl(path);
            vtn.removeVlanMap(this, path, mapId);
            return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            checkService();
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
        checkDefault();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            checkService();
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
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();
            VTenantImpl vtn = getTenantImpl(path);
            ctx = provider.newTxContext();
            vtn.setPortMap(this, ctx, path, pmconf);
            return new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
            if (ctx != null) {
                ctx.cancelTransaction();
            }
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
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();
            VTenantImpl vtn = getTenantImpl(path);
            ctx = provider.newTxContext();
            vtn.setPortMap(this, ctx, path, pmconf);
            return new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            data.cleanUp(this);
            if (ctx != null) {
                ctx.cancelTransaction();
            }
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
        checkDefault();

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
                                             VtnAclType aclType)
        throws VTNException {
        checkDefault();

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
        checkDefault();

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
        checkDefault();

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
     * @param op      A {@link VtnUpdateOperationType} instance which indicates
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
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                MacMapConfig mcconf) throws VTNException {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkService();

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
     * @param op        A {@link VtnUpdateOperationType} instance which
     *                  indicates how to change the MAC mapping configuration.
     * @param aclType   The type of access control list.
     * @param dlhosts   A set of {@link DataLinkHost} instances.
     * @return          A {@link UpdateType} object which represents the result
     *                  of the operation is returned.
     *                  {@code null} is returned if the configuration was not
     *                  changed.
     * @throws VTNException  An error occurred.
     */
    @Override
    public UpdateType setMacMap(VBridgePath path, VtnUpdateOperationType op,
                                VtnAclType aclType,
                                Set<? extends DataLinkHost> dlhosts)
        throws VTNException {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        try {
            checkService();

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
        if (nonDefault) {
            return;
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: findHost() called: addr={}, pathSet={}",
                      containerName, addr, pathSet);
        }

        VTNManagerProvider provider;
        try {
            provider = checkService();
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.debug("{}: findHost: Ignore request for {}: " +
                      "pathSet={}, status={}", containerName, addr, pathSet,
                      status.getDescription());
            return;
        }

        // Create an ARP request with specifying broadcast address.
        Ethernet ether = new ArpPacketBuilder().
            build(getVTNConfig().getControllerMacAddress(), addr);
        if (ether == null) {
            return;
        }

        TxContext ctx = provider.newTxContext();
        PacketContext pctx = new PacketContext(ether, null, ctx);
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
            ctx.cancelTransaction();
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
        if (nonDefault) {
            return false;
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: probeHost() called: host={}", containerName, host);
        }

        if (host == null) {
            return false;
        }

        NodeConnector nc = host.getnodeConnector();
        VTNManagerProvider provider;
        try {
            provider = checkService();
        } catch (VTNException e) {
            Status status = e.getStatus();
            LOG.debug("{}: probeHost: Ignore request for {}: {}",
                      containerName, host, status.getDescription());
            return false;
        }

        SalPort sport = SalPort.create(nc);
        if (sport == null) {
            LOG.debug("{}: probeHost: Ignore request for {}: " +
                      "Unsupported port {}",
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
        Ethernet ether = new ArpPacketBuilder((int)vlan).
            build(getVTNConfig().getControllerMacAddress(),
                  new EtherAddress(dst), target);
        if (ether == null) {
            LOG.debug("{}: probeHost: Ignore request for {}: " +
                      "Invalid IP address", containerName, host);
            return false;
        }

        TxContext ctx = provider.newTxContext();
        PacketContext pctx = new PacketContext(ether, sport, ctx);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            InventoryReader reader = ctx.getInventoryReader();
            VtnPort vport = reader.get(sport);
            if (vport == null) {
                LOG.debug("{}: probeHost: Ignore request for {}: " +
                          "Unknown port {}",
                          containerName, host, sport);
                return false;
            }

            if (!InventoryUtils.isEdge(vport)) {
                LOG.debug("{}: probeHost: Ignore request for {}: " +
                          "Internal port {}", containerName, host, sport);
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
            ctx.cancelTransaction();
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
        checkDefault();

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
        checkDefault();

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
            checkService();

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
            checkService();

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
     * @param path      A {@link VTenantPath} object that specifies the
     *                  position of the VTN.
     * @param mode      A {@link DataFlowMode} instance which specifies
     *                  behavior of this method.
     * @param filter    If a {@link DataFlowFilter} instance is specified,
     *                  only data flows that meet the condition specified by
     *                  {@link DataFlowFilter} instance is returned.
     *                  All data flows in the VTN is returned if {@code null}
     *                  is specified.
     * @param interval  Time interval in seconds for retrieving the average
     *                  statistics.
     * @return  A list of {@link DataFlow} instances which represents
     *          information about data flows.
     * @throws VTNException  An error occurred.
     */
    @Override
    public List<DataFlow> getDataFlows(VTenantPath path, DataFlowMode mode,
                                       DataFlowFilter filter, int interval)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that obtains a list of data flows.
        VnodeName vname = VTenantUtils.getVnodeName(path);
        GetDataFlowInputBuilder builder = new GetDataFlowInputBuilder().
            setTenantName(vname.getValue()).
            setMode(mode).
            setAverageInterval(interval);

        if (filter != null && !setInput(builder, filter)) {
            // Invalid filter condition is specified.
            // Thus no data flow should be selected.
            return Collections.<DataFlow>emptyList();
        }

        // Invoke RPC and await its completion.
        List<DataFlowInfo> result = getDataFlows(provider, builder.build());

        // Convert the result.
        List<DataFlow> list = new ArrayList<>(result.size());
        for (DataFlowInfo dfi: result) {
            list.add(FlowUtils.toDataFlow(dfi, mode));
        }

        return list;
    }

    /**
     * Return information about the specified data flow in the VTN.
     *
     * @param path      A {@link VTenantPath} object that specifies the position
     *                  of the VTN.
     * @param flowId    An identifier of the data flow.
     * @param mode      A {@link DataFlowMode} instance which specifies
     *                  behavior of this method.
     * @param interval  Time interval in seconds for retrieving the average
     *                  statistics.
     * @return  A {@link DataFlow} instance which represents information
     *          about the specified data flow.
     *          {@code null} is returned if the specified data flow was not
     *          found.
     * @throws VTNException  An error occurred.
     */
    @Override
    public DataFlow getDataFlow(VTenantPath path, long flowId,
                                DataFlowMode mode, int interval)
        throws VTNException {
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that obtains information about the specified
        // data flow.
        VnodeName vname = VTenantUtils.getVnodeName(path);
        VtnFlowId fid = new VtnFlowId(NumberUtils.getUnsigned(flowId));
        GetDataFlowInput input = new GetDataFlowInputBuilder().
            setTenantName(vname.getValue()).
            setMode(mode).
            setAverageInterval(interval).
            setFlowId(fid).
            build();

        // Invoke RPC and await its completion.
        List<DataFlowInfo> result = getDataFlows(provider, input);

        // Convert the result.
        if (result.size() > 1) {
            // This should never happen.
            throw new VTNException("Unexpected data flow list: " + result);
        }

        return (result.isEmpty())
            ? null
            : FlowUtils.toDataFlow(result.get(0), mode);
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that obtains the number of data flows.
        VnodeName vname = VTenantUtils.getVnodeName(path);
        GetDataFlowCountInput input = new GetDataFlowCountInputBuilder().
            setTenantName(vname.getValue()).
            build();

        // Invoke RPC and await its completion.
        VtnFlowService rpc = provider.getVtnRpcService(VtnFlowService.class);
        GetDataFlowCountOutput output =
            getRpcOutput(rpc.getDataFlowCount(input));
        Integer count = output.getCount();
        if (count == null) {
            throw new VTNException("Flow count is unavailable.");
        }

        return count.intValue();
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
        ArrayList<FlowCondition> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            List<VTNFlowCondition> vlist =
                FlowCondUtils.readFlowConditions(rtx);
            for (VTNFlowCondition vfcond: vlist) {
                list.add(vfcond.toFlowCondition());
            }
        } finally {
            ctx.cancelTransaction();
        }

        // Sort flow conditions by their name.
        VTNIdentifiableComparator<String> comparator =
            new VTNIdentifiableComparator<>(String.class);
        Collections.sort(list, comparator);

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
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VTNFlowCondition vfcond =
                FlowCondUtils.readFlowCondition(rtx, name);
            return vfcond.toFlowCondition();
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that replaces the flow condition specified
        // by the given name.
        VTNFlowCondition vfcond = new VTNFlowCondition(name, fcond);
        SetFlowConditionInput input = vfcond.toSetFlowConditionInputBuilder().
            setOperation(VtnUpdateOperationType.SET).build();

        // Invoke RPC and await its completion.
        VtnFlowConditionService rpc =
            provider.getVtnRpcService(VtnFlowConditionService.class);
        SetFlowConditionOutput output =
            getRpcOutput(rpc.setFlowCondition(input));

        // Convert the result.
        return MiscUtils.toUpdateType(output.getStatus());
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
        try {
            VTNManagerProvider provider = checkService();
            RemoveFlowConditionInput input =
                new RemoveFlowConditionInputBuilder().setName(name).build();

            // Invoke RPC and await its completion.
            VtnFlowConditionService rpc =
                provider.getVtnRpcService(VtnFlowConditionService.class);
            getRpcOutput(rpc.removeFlowCondition(input), true);
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the flow conditions.
     *
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearFlowCondition() {
        try {
            VTNManagerProvider provider = checkService();

            // Invoke RPC and await its completion.
            VtnFlowConditionService rpc =
                provider.getVtnRpcService(VtnFlowConditionService.class);
            ClearFlowConditionOutput output =
                getRpcOutput(rpc.clearFlowCondition());
            if (output.getStatus() == null) {
                // The flow condition container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
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
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VTNFlowMatch vfmatch =
                FlowCondUtils.readFlowMatch(rtx, name, index);
            return (vfmatch == null) ? null : vfmatch.toFlowMatch();
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Complete FlowMatch instance.
        FlowMatch mt;
        if (match == null) {
            // Create an empty flow match.
            mt = new FlowMatch(index, null, null, null);
        } else {
            // Ensure that the match index is assigned.
            mt = match.assignIndex(index);
        }

        VTNFlowMatch vfmatch = new VTNFlowMatch(mt);
        List<FlowMatchList> fml =
            Collections.singletonList(vfmatch.toFlowMatchListBuilder().build());

        // Construct an RPC input that adds the given flow match configuration
        // to the specified flow condition.
        SetFlowConditionMatchInput input = new SetFlowConditionMatchInputBuilder().
            setName(name).setFlowMatchList(fml).build();

        // Invoke RPC and await its completion.
        VtnFlowConditionService rpc =
            provider.getVtnRpcService(VtnFlowConditionService.class);
        SetFlowConditionMatchOutput output =
            getRpcOutput(rpc.setFlowConditionMatch(input));
        SetMatchResult result =
            getRpcOutput(output.getSetMatchResult(), 0, false);
        Integer idx = vfmatch.getIdentifier();
        if (!idx.equals(result.getIndex())) {
            throw new VTNException("Unexpected match index in RPC output: " +
                                   output.getSetMatchResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
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
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes the flow match associated
            // with the given index in the flow condition.
            RemoveFlowConditionMatchInput input = new RemoveFlowConditionMatchInputBuilder().
                setName(name).setMatchIndex(Collections.singletonList(idx)).
                build();

            // Invoke RPC and await its completion.
            VtnFlowConditionService rpc =
                provider.getVtnRpcService(VtnFlowConditionService.class);
            RemoveFlowConditionMatchOutput output =
                getRpcOutput(rpc.removeFlowConditionMatch(input));
            RemoveMatchResult result =
                getRpcOutput(output.getRemoveMatchResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException(
                    "Unexpected match index in RPC output: " +
                    output.getRemoveMatchResult());
            }
            if (result.getStatus() == null) {
                // The specified match index was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
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
        List<Integer> list = new ArrayList<>();
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            for (VtnPathPolicy vpp: PathPolicyUtils.readVtnPathPolicies(rtx)) {
                list.add(vpp.getId());
            }
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathPolicy vpp = PathPolicyUtils.readVtnPathPolicy(rtx, id);
            return PathPolicyUtils.toPathPolicy(vpp);
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that replaces the path policy configuration
        // with the given onfiguration.
        Integer pid = Integer.valueOf(id);
        SetPathPolicyInput input = new PathPolicyConfigBuilder.Rpc().
            set(policy, pid).getBuilder().
            setOperation(VtnUpdateOperationType.SET).build();

        // Invoke RPC and await its completion.
        VtnPathPolicyService rpc =
            provider.getVtnRpcService(VtnPathPolicyService.class);
        SetPathPolicyOutput output = getRpcOutput(rpc.setPathPolicy(input));

        // Convert the result.
        return MiscUtils.toUpdateType(output.getStatus());
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
        try {
            VTNManagerProvider provider = checkService();
            RemovePathPolicyInput input = new RemovePathPolicyInputBuilder().
                setId(Integer.valueOf(id)).build();

            // Invoke RPC and await its completion.
            VtnPathPolicyService rpc =
                provider.getVtnRpcService(VtnPathPolicyService.class);
            getRpcOutput(rpc.removePathPolicy(input), true);
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the path policies.
     *
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearPathPolicy() {
        try {
            VTNManagerProvider provider = checkService();

            // Invoke RPC and await its completion.
            VtnPathPolicyService rpc =
                provider.getVtnRpcService(VtnPathPolicyService.class);
            ClearPathPolicyOutput output = getRpcOutput(rpc.clearPathPolicy());
            if (output.getStatus() == null) {
                // The path policy container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
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
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathPolicy vpp = PathPolicyUtils.readVtnPathPolicy(rtx, id);
            Long c = vpp.getDefaultCost();
            return (c == null) ? PathPolicy.COST_UNDEF : c.longValue();
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that updates only the default cost.
        PathPolicyConfigBuilder.Rpc builder =
            PathPolicyUtils.createRpcInput(id);
        SetPathPolicyInput input = builder.setDefaultCost(Long.valueOf(cost)).
            getBuilder().build();

        // Invoke RPC and await its completion.
        VtnPathPolicyService rpc =
            provider.getVtnRpcService(VtnPathPolicyService.class);
        SetPathPolicyOutput output = getRpcOutput(rpc.setPathPolicy(input));

        // Convert the result.
        return (output.getStatus() != null);
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
        checkDefault();

        VtnPortDesc vdesc;
        try {
            vdesc = NodeUtils.toVtnPortDesc(ploc);
        } catch (VTNException e) {
            // This means that the given PortLocation contains an invalid
            // value.
            return PathPolicy.COST_UNDEF;
        }

        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathCost vpc = PathPolicyUtils.readVtnPathCost(rtx, id, vdesc);
            if (vpc != null) {
                Long c = vpc.getCost();
                if (c != null) {
                    return c.longValue();
                }
            }
            return PathPolicy.COST_UNDEF;
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that adds the given link cost configuration.
        VtnPortDesc vdesc = NodeUtils.toVtnPortDesc(ploc);
        PathCostList pcl = new PathCostConfigBuilder.Rpc().
            setPortDesc(vdesc).setCost(cost).getBuilder().build();
        SetPathCostInput input = new SetPathCostInputBuilder().
            setId(Integer.valueOf(id)).
            setPathCostList(Collections.singletonList(pcl)).build();

        // Invoke RPC and await its completion.
        VtnPathPolicyService rpc =
            provider.getVtnRpcService(VtnPathPolicyService.class);
        SetPathCostOutput output = getRpcOutput(rpc.setPathCost(input));
        SetPathCostResult result =
            getRpcOutput(output.getSetPathCostResult(), 0, false);
        if (!vdesc.equals(result.getPortDesc())) {
            throw new VTNException("Unexpected port desc in RPC output: " +
                                   output.getSetPathCostResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
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
        TxContext ctx = null;
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes the given link cost
            // configuration.
            VtnPortDesc vdesc;
            try {
                vdesc = NodeUtils.toVtnPortDesc(ploc);
            } catch (VTNException e) {
                // This means that the given PortLocation contains an invalid
                // value.
                ctx = provider.newTxContext();
                ReadTransaction rtx = ctx.getTransaction();
                PathPolicyUtils.readVtnPathPolicy(rtx, id);
                return null;
            }

            RemovePathCostInput input = new RemovePathCostInputBuilder().
                setId(Integer.valueOf(id)).
                setPortDesc(Collections.singletonList(vdesc)).build();

            // Invoke RPC and await its completion.
            VtnPathPolicyService rpc =
                provider.getVtnRpcService(VtnPathPolicyService.class);
            RemovePathCostOutput output =
                getRpcOutput(rpc.removePathCost(input));
            RemovePathCostResult result =
                getRpcOutput(output.getRemovePathCostResult(), 0, false);
            if (!vdesc.equals(result.getPortDesc())) {
                throw new VTNException("Unexpected port desc in RPC output: " +
                                       output.getRemovePathCostResult());
            }
            if (result.getStatus() == null) {
                // The specified path cost configuration did not exist.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            if (ctx != null) {
                ctx.cancelTransaction();
            }
        }

        return new Status(StatusCode.SUCCESS, null);
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
        VTNManagerProvider provider = checkService();
        List<PathMap> list = new ArrayList<>();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            for (VtnPathMap vpm: PathMapUtils.readPathMaps(rtx)) {
                list.add(PathMapUtils.toPathMap(vpm));
            }
        } finally {
            ctx.cancelTransaction();
        }

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
        VTNManagerProvider provider = checkService();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathMap vpm = PathMapUtils.readPathMap(rtx, index);
            return PathMapUtils.toPathMap(vpm);
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that adds the given path map configuration
        // to the global path map list.
        Integer idx = Integer.valueOf(index);
        PathMapList pml =
            PathMapUtils.toPathMapListBuilder(idx, pmap).build();
        List<PathMapList> pmlist = Collections.singletonList(pml);
        SetPathMapInput input = new SetPathMapInputBuilder().
            setPathMapList(pmlist).build();

        // Invoke RPC and await its completion.
        VtnPathMapService rpc =
            provider.getVtnRpcService(VtnPathMapService.class);
        SetPathMapOutput output = getRpcOutput(rpc.setPathMap(input));
        SetPathMapResult result =
            getRpcOutput(output.getSetPathMapResult(), 0, false);
        if (!idx.equals(result.getIndex())) {
            throw new VTNException("Unexpected map index in RPC output: " +
                                   output.getSetPathMapResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
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
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes the global path map
            // associated with the given index in the global path map list.
            RemovePathMapInput input = new RemovePathMapInputBuilder().
                setMapIndex(Collections.singletonList(idx)).build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            RemovePathMapOutput output =
                getRpcOutput(rpc.removePathMap(input));
            RemovePathMapResult result =
                getRpcOutput(output.getRemovePathMapResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException("Unexpected map index in RPC output: " +
                                       output.getRemovePathMapResult());
            }
            if (result.getStatus() == null) {
                // The specified global path map was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the container path maps.
     *
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearPathMap() {
        try {
            VTNManagerProvider provider = checkService();

            // Construct an RPC input that removes all the global path maps.
            ClearPathMapInput input = new ClearPathMapInputBuilder().build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            ClearPathMapOutput output =
                getRpcOutput(rpc.clearPathMap(input));
            if (output.getStatus() == null) {
                // The global path map container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
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
        VTNManagerProvider provider = checkService();
        VnodeName vname = VTenantUtils.getVnodeName(path);
        List<PathMap> list = new ArrayList<>();
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            for (VtnPathMap vpm: PathMapUtils.readPathMaps(rtx, vname)) {
                list.add(PathMapUtils.toPathMap(vpm));
            }
        } finally {
            ctx.cancelTransaction();
        }

        return list;
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
        VTNManagerProvider provider = checkService();
        VnodeName vname = VTenantUtils.getVnodeName(path);
        TxContext ctx = provider.newTxContext();
        try {
            ReadTransaction rtx = ctx.getTransaction();
            VtnPathMap vpm = PathMapUtils.readPathMap(rtx, vname, index);
            return PathMapUtils.toPathMap(vpm);
        } finally {
            ctx.cancelTransaction();
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
        VTNManagerProvider provider = checkService();

        // Construct an RPC input that adds the given path map configuration
        // to the path map list in the specified VTN.
        String tname = VTenantUtils.getName(path);
        Integer idx = Integer.valueOf(index);
        PathMapList pml;
        try {
            pml = PathMapUtils.toPathMapListBuilder(idx, pmap).build();
        } catch (VTNException e) {
            // Check to see if the target VTN is present.
            VnodeName vname = VTenantUtils.getVnodeName(tname);
            TxContext ctx = provider.newTxContext();
            try {
                ReadTransaction rtx = ctx.getTransaction();
                VTenantUtils.readVtn(rtx, vname);
            } finally {
                ctx.cancelTransaction();
            }
            throw e;
        }

        List<PathMapList> pmlist = Collections.singletonList(pml);
        SetPathMapInput input = new SetPathMapInputBuilder().
            setTenantName(tname).setPathMapList(pmlist).build();

        // Invoke RPC and await its completion.
        VtnPathMapService rpc =
            provider.getVtnRpcService(VtnPathMapService.class);
        SetPathMapOutput output = getRpcOutput(rpc.setPathMap(input));
        SetPathMapResult result =
            getRpcOutput(output.getSetPathMapResult(), 0, false);
        if (!idx.equals(result.getIndex())) {
            throw new VTNException("Unexpected map index in RPC output: " +
                                   output.getSetPathMapResult());
        }

        // Convert the result.
        return MiscUtils.toUpdateType(result.getStatus());
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
        Integer idx = Integer.valueOf(index);
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);

            // Construct an RPC input that removes the VTN path map
            // associated with the given index in the given VTN.
            RemovePathMapInput input = new RemovePathMapInputBuilder().
                setTenantName(tname).
                setMapIndex(Collections.singletonList(idx)).build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            RemovePathMapOutput output =
                getRpcOutput(rpc.removePathMap(input));
            RemovePathMapResult result =
                getRpcOutput(output.getRemovePathMapResult(), 0, false);
            if (!idx.equals(result.getIndex())) {
                throw new VTNException("Unexpected map index in RPC output: " +
                                       output.getRemovePathMapResult());
            }
            if (result.getStatus() == null) {
                // The specified VTN path map was not found.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all the VTN path maps configured in the specified VTN.
     *
     * @param path   A {@link VTenantPath} object that specifies the position
     *               of the VTN.
     * @return  A {@link Status} object which represents the result of the
     *          operation is returned.
     */
    @Override
    public Status clearPathMap(VTenantPath path) {
        try {
            VTNManagerProvider provider = checkService();
            String tname = VTenantUtils.getName(path);

            // Construct an RPC input that removes all the VTN path maps
            // in the given VTN.
            ClearPathMapInput input = new ClearPathMapInputBuilder().
                setTenantName(tname).build();

            // Invoke RPC and await its completion.
            VtnPathMapService rpc =
                provider.getVtnRpcService(VtnPathMapService.class);
            ClearPathMapOutput output =
                getRpcOutput(rpc.clearPathMap(input));
            if (output.getStatus() == null) {
                // The VTN path map container is empty.
                return null;
            }
        } catch (VTNException e) {
            return e.getStatus();
        }

        return new Status(StatusCode.SUCCESS, null);
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
        checkDefault();

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
        checkDefault();

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
        // Acquire writer lock in order to block notifyPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        LockStack lstack = new LockStack();
        try {
            checkService();

            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            UpdateType result = ffmap.set(this, index, filter);
            if (result != null) {
                export(vtn);
                vtn.saveConfigImpl(null);
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
        // Acquire writer lock in order to block notifyPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        LockStack lstack = new LockStack();
        try {
            checkService();

            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            Status result = ffmap.remove(this, index);
            if (result != null) {
                export(vtn);
                vtn.saveConfigImpl(null);
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
        // Acquire writer lock in order to block notifyPacket().
        VTNThreadData data = VTNThreadData.create(rwLock.writeLock());
        LockStack lstack = new LockStack();
        try {
            checkService();

            VTenantImpl vtn = getTenantImpl(fid);
            FlowFilterMap ffmap = vtn.getFlowFilterMap(lstack, fid, false);
            Status result = ffmap.clear(this);
            if (result != null) {
                export(vtn);
                vtn.saveConfigImpl(null);
            }
            return result;
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            lstack.clear();
            data.cleanUp(this);
        }
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

        if (CACHE_MAC.equals(cacheName)) {
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
            // Save VTN configurations.
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.saveConfig(null);
            }

            return new Status(StatusCode.SUCCESS, null);
        } catch (VTNException e) {
            return e.getStatus();
        } finally {
            unlock(rdlock);
        }
    }

    // VTNInventoryListener

    /**
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnNode(VtnNodeEvent ev) throws VTNException {
        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (ev.getUpdateType() == VtnUpdateType.REMOVED) {
                // Flush MAC address table entries detected on the removed
                // node.
                SalNode snode = ev.getSalNode();
                Node node = snode.getAdNode();
                for (MacAddressTable table: macTableMap.values()) {
                    table.flush(node);
                }
            }

            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNode(this, ev);
            }
        } finally {
            unlock(wrlock);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnPort(VtnPortEvent ev) throws VTNException {
        SalPort sport = ev.getSalPort();
        NodeConnector nc = sport.getAdNodeConnector();

        // Acquire writer lock because this operation may change existing
        // virtual network mapping.
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            // Flush MAC address table entries affected by the port if it has
            // been disabled or changed to ISL port.
            Boolean isl = ev.getInterSwitchLinkChange();
            if (ev.isDisabled() || Boolean.TRUE.equals(isl)) {
                for (MacAddressTable table: macTableMap.values()) {
                    table.flush(nc);
                }
            }

            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.notifyNodeConnector(this, ev);
            }
        } finally {
            unlock(wrlock);
        }
    }

    // VTNPacketListener

    /**
     * Invoked when a packet has been received.
     *
     * @param ev  A {@link PacketInEvent} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    public void notifyPacket(PacketInEvent ev) throws VTNException {
        // Create a packet context.
        PacketContext pctx = new PacketContext(ev);

        EtherAddress src = pctx.getSourceAddress();
        EtherAddress ctlrMac = getVTNConfig().getControllerMacAddress();
        if (src.equals(ctlrMac)) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Ignore self-originated packet: {}",
                          containerName, pctx.getDescription());
            }
            return;
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            SalPort ingress = ev.getIngressPort();
            InventoryReader reader = ev.getTxContext().getInventoryReader();
            VtnPort vport = reader.get(ingress);
            if (vport == null) {
                LOG.debug("{}: Ignore packet from unknown port: {}",
                          containerName, ingress);
                return;
            }

            NodeConnector nc = ingress.getAdNodeConnector();
            if (InventoryUtils.hasPortLink(vport)) {
                LOG.debug("{}: Ignore packet from internal node connector: {}",
                          containerName, ingress);
                // This PACKET_IN may be caused by an obsolete flow entry.
                // So all flow entries related to this port should be removed.
                PortFlowRemover remover = new PortFlowRemover(ingress);
                ev.getTxContext().getProvider().removeFlows(remover);
                return;
            }

            // Determine virtual network mapping that maps the packet.
            short vlan = (short)pctx.getVlan();
            byte[] srcMac = src.getBytes();
            MapReference ref =
                resourceManager.getMapReference(srcMac, nc, vlan);
            if (ref != null && containerName.equals(ref.getContainerName())) {
                pctx.setMapReference(ref);
                VNodePath path = ref.getPath();
                VTenantImpl vtn = getTenantImpl(path);
                vtn.receive(this, ref, pctx);
                return;
            }
        } catch (VTNException e) {
            LOG.error(containerName + ": Ignore packet: " + e.getMessage(), e);
        } catch (Exception e) {
            logException(LOG, null, e, pctx.getDescription());
        } finally {
            unlock(rdlock);
        }
    }

    // VTNRoutingListener

    /**
     * {@inheritDoc}
     */
    @Override
    public void routingUpdated(RoutingEvent ev) throws VTNException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (VTenantImpl vtn: tenantDB.values()) {
                vtn.recalculateDone(this, ev);
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
            if (isActive()) {
                findHost(networkAddress, null);
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
            if (isActive()) {
                probeHost(host);
            }
        } finally {
            unlock(rdlock);
        }
    }
}
