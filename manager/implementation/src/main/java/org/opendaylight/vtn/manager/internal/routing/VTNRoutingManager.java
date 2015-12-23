/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Future;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.RoutingUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.RoutingUpdatedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.AddedLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.AddedLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.RemovedLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.RemovedLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Internal routing manager.
 */
public final class VTNRoutingManager
    extends DataStoreListener<VtnLink, TopologyEventContext>
    implements VTNSubSystem, VtnPathPolicyService {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNRoutingManager.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A graph that keeps network topology.
     */
    private final TopologyGraph  topology;

    /**
     * Path policy listener.
     */
    private final PathPolicyListener  pathPolicyListener;

    /**
     * A list of VTN routing listeners.
     */
    private final CopyOnWriteArrayList<VTNRoutingListener>  vtnListeners =
        new CopyOnWriteArrayList<>();

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     */
    public VTNRoutingManager(VTNManagerProvider provider) {
        super(VtnLink.class);
        vtnProvider = provider;
        topology = new TopologyGraph(provider);
        ReadOnlyTransaction rtx =
            provider.getDataBroker().newReadOnlyTransaction();
        try {
            initTopology(rtx);
        } catch (Exception e) {
            String msg = "Failed to initialize VTN topology manager.";
            LOG.error(msg, e);
            throw new IllegalStateException(msg, e);
        } finally {
            rtx.close();
        }

        try {
            registerListener(provider.getDataBroker(),
                             LogicalDatastoreType.OPERATIONAL,
                             DataChangeScope.SUBTREE, true);
            pathPolicyListener = new PathPolicyListener(provider, topology);
            addCloseable(pathPolicyListener);
        } catch (RuntimeException e) {
            LOG.error("Failed to initialize routing manager.", e);
            close();
            throw e;
        }
    }

    /**
     * Add the given VTN routing listener.
     *
     * @param l  A VTN routing listener.
     */
    public void addListener(VTNRoutingListener l) {
        vtnListeners.addIfAbsent(l);
    }

    /**
     * Return a packet route resolver.
     *
     * @param id  The identifier of the path policy.
     * @return  A {@link RouteResolver} instance if found.
     *          {@code null} if not fonud.
     */
    public RouteResolver getRouteResolver(Integer id) {
        return topology.getResolver(id);
    }

    /**
     * Initialize the network topology graph.
     *
     * @param rtx  A {@link ReadOnlyTransaction} instance.
     * @throws VTNException  An error occurred.
     */
    private void initTopology(ReadOnlyTransaction rtx) throws VTNException {
        InstanceIdentifier<VtnTopology> vtPath =
            InstanceIdentifier.create(VtnTopology.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VtnTopology vtopo = DataStoreUtils.read(rtx, oper, vtPath).orNull();
        if (vtopo != null) {
            topology.initialize(vtopo.getVtnLink());
        }
    }

    /**
     * Publish a routing-updated notification.
     *
     * @param created  A list of created inter-switch links.
     * @param removed  A list of removed inter-switch links.
     */
    private void notifyRoutingUpdated(List<VtnLink> created,
                                      List<VtnLink> removed) {
        List<AddedLink> addedLinks = new ArrayList<>();
        for (VtnLink vlink: created) {
            addedLinks.add(new AddedLinkBuilder(vlink).build());
        }

        List<RemovedLink> removedLinks = new ArrayList<>();
        for (VtnLink vlink: removed) {
            removedLinks.add(new RemovedLinkBuilder(vlink).build());
        }

        RoutingUpdated updated = new RoutingUpdatedBuilder().
            setAddedLink(addedLinks).setRemovedLink(removedLinks).build();
        vtnProvider.publish(updated);
    }

    // AutoCloseable

    /**
     * Close the VTN routing service.
     */
    @Override
    public void close() {
        super.close();
        vtnListeners.clear();
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected TopologyEventContext enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return new TopologyEventContext();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(TopologyEventContext ectx) {
        List<VtnLink> created = ectx.getCreated();
        List<VtnLink> removed = ectx.getRemoved();
        if (topology.update(created, removed) &&
            vtnProvider.isOwner(VTNEntityType.INVENTORY)) {
            for (VTNRoutingListener l: vtnListeners) {
                RoutingEvent rev = new RoutingEvent(l);
                vtnProvider.post(rev);
            }

            LOG.info("Routing table has been updated.");
            notifyRoutingUpdated(created, removed);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(TopologyEventContext ectx,
                             IdentifiedData<VtnLink> data) {
        VtnLink vlink = data.getValue();
        LOG.info("Inter-switch link has been created: {}",
                 InventoryUtils.toString(vlink));
        ectx.addCreated(vlink);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(TopologyEventContext ectx,
                             ChangedData<VtnLink> data) {
        VtnLink old = data.getOldValue();
        VtnLink vlink = data.getValue();
        if (old.getSource().equals(vlink.getSource()) &&
            old.getDestination().equals(vlink.getDestination())) {
            // Termination points were not changed.
            // In this case we do not need to update routing table.
            LOG.info("Inter-switch link attributes have been changed: " +
                     "old={}, new={}", InventoryUtils.toString(old),
                     InventoryUtils.toString(vlink));
        } else {
            LOG.info("Inter-switch link has been changed: old={}, new={}",
                     InventoryUtils.toString(old),
                     InventoryUtils.toString(vlink));
            ectx.addRemoved(data.getOldValue());
            ectx.addCreated(data.getValue());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(TopologyEventContext ectx,
                             IdentifiedData<VtnLink> data) {
        VtnLink vlink = data.getValue();
        LOG.info("Inter-switch link has been removed: {}",
                 InventoryUtils.toString(vlink));
        ectx.addRemoved(vlink);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnLink> getWildcardPath() {
        return InstanceIdentifier.builder(VtnTopology.class).
            child(VtnLink.class).build();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    // VTNConfigProvider

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<?> initConfig(boolean master) {
        return pathPolicyListener.initConfig(vtnProvider, master);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        regs.add(rpcReg.
                 addRpcImplementation(VtnPathPolicyService.class, this));
    }

    // VtnPathPolicyService

    /**
     * Create or modify the path policy.
     *
     * <ul>
     *   <li>
     *     If the path policy specified by the path policy ID does not exist,
     *     a new path policy will be associated with the specified ID.
     *   </li>
     *   <li>
     *     If the path policy specified by the path policy ID already exists,
     *     it will be modified as specified by the RPC input.
     *   </li>
     * </ul>
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetPathPolicyOutput>> setPathPolicy(
        SetPathPolicyInput input) {
        try {
            // Create a task that updates the configuration.
            SetPathPolicyTask task = SetPathPolicyTask.create(topology, input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, SetPathPolicyOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(SetPathPolicyOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the path policy specified by the path policy identifier.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> removePathPolicy(
        RemovePathPolicyInput input) {
        try {
            // Create a task that removes the specified path policy.
            RemovePathPolicyTask task =
                RemovePathPolicyTask.create(topology, input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, Void>(taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(Void.class, e).buildFuture();
        }
    }

    /**
     * Associate the cost of transmitting a packet with the specified
     * switch port in the specified path policy.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetPathCostOutput>> setPathCost(
        SetPathCostInput input) {
        try {
            // Create a task that updates the configuration.
            SetPathCostTask task = SetPathCostTask.create(topology, input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, SetPathCostOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(SetPathCostOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove link cost configurations for all the given switch ports in the
     * given path policy configuration.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemovePathCostOutput>> removePathCost(
        RemovePathCostInput input) {
        try {
            // Create a task that remove link cost configuration.
            RemovePathCostTask task =
                RemovePathCostTask.create(topology, input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, RemovePathCostOutput>(
                taskFuture, task);
        } catch (Exception e) {
            return RpcUtils.getErrorBuilder(RemovePathCostOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove all the path policies.
     *
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<ClearPathPolicyOutput>> clearPathPolicy() {
        // Create a task that removes all the path policies.
        ClearPathPolicyTask task = new ClearPathPolicyTask();
        VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
        return new RpcFuture<VtnUpdateType, ClearPathPolicyOutput>(
            taskFuture, task);
    }
}
