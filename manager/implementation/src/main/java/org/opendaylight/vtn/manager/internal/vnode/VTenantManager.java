/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.getMapping;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Future;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;
import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNSubSystem;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryListener;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.packet.PacketInEvent;
import org.opendaylight.vtn.manager.internal.packet.VTNPacketListener;
import org.opendaylight.vtn.manager.internal.routing.RoutingEvent;
import org.opendaylight.vtn.manager.internal.routing.VTNRoutingListener;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.NodeMacFilter;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.PortMacFilter;
import org.opendaylight.vtn.manager.internal.vnode.MacMapListener.MappedHostListener;
import org.opendaylight.vtn.manager.internal.vnode.MacMapListener.VlanHostDescListListener;
import org.opendaylight.vtn.manager.internal.vnode.VbridgeListener.BridgeStatusListener;
import org.opendaylight.vtn.manager.internal.vnode.VbridgeListener.FaultedPathsListener;
import org.opendaylight.vtn.manager.internal.vnode.VbridgeListener.VbridgeConfigListener;
import org.opendaylight.vtn.manager.internal.vnode.VinterfaceListener.PortMapConfigListener;
import org.opendaylight.vtn.manager.internal.vnode.VinterfaceListener.VinterfaceConfigListener;
import org.opendaylight.vtn.manager.internal.vnode.VinterfaceListener.VinterfaceStatusListener;
import org.opendaylight.vtn.manager.internal.vnode.VlanMapListener.VlanMapConfigListener;
import org.opendaylight.vtn.manager.internal.vnode.VlanMapListener.VlanMapStatusListener;
import org.opendaylight.vtn.manager.internal.vnode.VterminalListener.VterminalConfigListener;
import org.opendaylight.vtn.manager.internal.vnode.VtnFlowFilterListener.VtnFlowActionListener;
import org.opendaylight.vtn.manager.internal.vnode.VtnListener.VtenantConfigListener;
import org.opendaylight.vtn.manager.internal.vnode.VtnListener.VtnPathMapListener;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenant;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappings;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappingsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTablesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * Virtual tenant manager.
 */
public final class VTenantManager
    extends MultiDataStoreListener<Vtn, VTenantChange>
    implements VTNSubSystem, VTNInventoryListener, VTNRoutingListener,
               VTNPacketListener, VtnService, VtnVterminalService,
               VtnFlowFilterService {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VTenantManager.class);

    /**
     * Data change listeners for each data model in the VTN tree.
     */
    private static final Map<Class<?>, VNodeChangeListener<?>>  LISTENERS;

    /**
     * A tag that indicates the description.
     */
    static final String  TAG_DESC = "desc=";

    /**
     * A tag that indicates the index number.
     */
    static final String  TAG_INDEX = "index=";

    /**
     * A tag that indicates the physical switch.
     */
    static final String  TAG_NODE = "node=";

    /**
     * A tag that indicates the state value.
     */
    static final String  TAG_STATE = "state=";

    /**
     * A tag that indicates the condition name.
     */
    static final String  TAG_COND = "cond=";

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * vBridge manager.
     */
    private final VBridgeManager  vBridgeManager;

    /**
     * VTN node event handler.
     */
    private final VtnNodeEventHandler  nodeHandler = new VtnNodeEventHandler();

    /**
     * VTN port event handler.
     */
    private final VtnPortEventHandler  portHandler = new VtnPortEventHandler();

    /**
     * Path fault resolver.
     */
    private final PathFaultResolver  pathFaultResolver =
        new PathFaultResolver();

    /**
     * Initialize static fields.
     */
    static {
        // Create data change listeners.
        LISTENERS = ImmutableMap.<Class<?>, VNodeChangeListener<?>>builder().
            put(VtnFlowAction.class, new VtnFlowActionListener()).
            put(VtnFlowFilter.class, new VtnFlowFilterListener()).
            put(PortMapConfig.class, new PortMapConfigListener()).
            put(VinterfaceStatus.class, new VinterfaceStatusListener()).
            put(VinterfaceConfig.class, new VinterfaceConfigListener()).
            put(Vinterface.class, new VinterfaceListener()).
            put(VlanMapStatus.class, new VlanMapStatusListener()).
            put(VlanMapConfig.class, new VlanMapConfigListener()).
            put(VlanMap.class, new VlanMapListener()).
            put(MappedHost.class, new MappedHostListener()).
            put(VlanHostDescList.class, new VlanHostDescListListener()).
            put(MacMap.class, new MacMapListener()).
            put(FaultedPaths.class, new FaultedPathsListener()).
            put(BridgeStatus.class, new BridgeStatusListener()).
            put(VterminalConfig.class, new VterminalConfigListener()).
            put(Vterminal.class, new VterminalListener()).
            put(VbridgeConfig.class, new VbridgeConfigListener()).
            put(Vbridge.class, new VbridgeListener()).
            put(VtnPathMap.class, new VtnPathMapListener()).
            put(VtenantConfig.class, new VtenantConfigListener()).
            put(Vtn.class, new VtnListener()).
            build();
    }

    /**
     * {@code VTenantSaveTask} describes a MD-SAL datastore transaction task
     * that saves the VTN configurations.
     *
     * <p>
     *   This task returns the root container of all VTNs.
     * </p>
     */
    private static final class VTenantSaveTask extends AbstractTxTask<Vtns> {
        /**
         * Set {@code true} if the root container for the VTN tree has been
         * created.
         */
        private boolean  vtnCreated;

        /**
         * Set {@code true} if the root container for the MAC address table
         * tree has been created.
         */
        private boolean  macCreated;

        /**
         * Set {@code true} if the root container for the virtual network
         * mapping has been created.
         */
        private boolean  vmapCreated;

        // AbstractTxTask

        /**
         * {@inheritDoc}
         */
        @Override
        public Vtns execute(TxContext ctx) throws VTNException {
            vtnCreated = false;
            macCreated = false;
            vmapCreated = false;

            // Load current configuration.
            InstanceIdentifier<Vtns> path =
                InstanceIdentifier.create(Vtns.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            Optional<Vtns> vopt = DataStoreUtils.read(tx, oper, path);
            Vtns root;
            if (vopt.isPresent()) {
                root = vopt.get();
            } else {
                // Initialize the VTN container.
                root = new VtnsBuilder().build();
                tx.put(oper, path, root, true);
                vtnCreated = true;
            }

            // Ensure that the MAC address table container is present.
            InstanceIdentifier<MacTables> mpath = InstanceIdentifier.
                create(MacTables.class);
            Optional<MacTables> mopt = DataStoreUtils.read(tx, oper, mpath);
            if (!mopt.isPresent()) {
                // Initialize the MAC address table container.
                MacTables mtable = new MacTablesBuilder().build();
                tx.put(oper, mpath, mtable, true);
                macCreated = true;
            }

            // Ensure that the virtual network mapping container is present.
            InstanceIdentifier<VtnMappings> vmpath = InstanceIdentifier.
                create(VtnMappings.class);
            Optional<VtnMappings> vmopt =
                DataStoreUtils.read(tx, oper, vmpath);
            if (!vmopt.isPresent()) {
                // Prepare an empty virtual mapping container.
                tx.put(oper, vmpath, new VtnMappingsBuilder().build(), true);
                vmapCreated = true;
            }

            return root;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Vtns result) {
            if (vtnCreated) {
                LOG.info("An empty VTN container has been created.");
            }
            if (macCreated) {
                LOG.info("An empty mac-tables container has been created.");
            }
            if (vmapCreated) {
                LOG.info("An empty vtn-mappings container has been created.");
            }

            XmlConfigFile.Type ftype = XmlConfigFile.Type.VTN;
            Set<String> names = new HashSet<>();
            List<Vtn> vlist = result.getVtn();
            if (vlist != null) {
                for (Vtn vtn: vlist) {
                    // Save configuration into a file.
                    try {
                        XmlVTenant xvtn = new XmlVTenant(vtn);
                        String name = xvtn.getName().getValue();
                        XmlConfigFile.save(ftype, name, xvtn);
                        LOG.trace("{}: VTN configuration has been saved.",
                                  name);
                        names.add(name);
                    } catch (RpcException | RuntimeException e) {
                        LOG.warn("Ignore broken VTN: " + vtn, e);
                    }
                }

                // Remove obsolete configuration files.
                XmlConfigFile.deleteAll(ftype, names);
            }
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public VTenantManager(VTNManagerProvider provider) {
        super(Vtn.class);
        vtnProvider = provider;
        vBridgeManager = new VBridgeManager(provider);
        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL, true);
    }

    /**
     * Trigger the system shutdownn sequence.
     */
    public void shutdown() {
        vBridgeManager.shutdown();
    }

    /**
     * Return a data change listener for the given data.
     *
     * <p>
     *   If the virual node specified by {@code data} contains the VTN
     *   configuration, this method marks that VTN as updated.
     * </p>
     *
     * @param ectx  A {@link VTenantChange} instance.
     * @param data  A {@link IdentifiedData} instance.
     * @return  A {@link VNodeChangeListener} instance for the given data.
     */
    private VNodeChangeListener<?> getChangeListener(
        VTenantChange ectx, IdentifiedData<?> data) {
        InstanceIdentifier<?> path = data.getIdentifier();
        Class<?> type = path.getTargetType();
        VNodeChangeListener<?> listener = LISTENERS.get(type);
        if (listener.isConfiguration()) {
            // The VTN that contains the given data needs to be saved.
            VtnKey key = path.firstKeyOf(Vtn.class);
            String name = key.getName().getValue();
            ectx.addTargetVtn(name);
        }

        return listener;
    }

    /**
     * Start a background task that executes remove-flow-filter RPC.
     *
     * @param input  A {@link RemoveFlowFilterInput} instance.
     * @return  A {@link Future} associated with the RPC task.
     * @throws RpcException  An error occurred.
     */
    private Future<RpcResult<RemoveFlowFilterOutput>> startRpc(
        RemoveFlowFilterInput input) throws RpcException {
        if (input == null) {
            throw RpcUtils.getNullInputException();
        }

        // Determine the target flow filter list.
        VNodeIdentifier<?> ident = VNodeIdentifier.create(input, true);
        boolean output = Boolean.TRUE.equals(input.isOutput());
        List<Integer> indices = input.getIndices();

        Future<RpcResult<RemoveFlowFilterOutput>> ret;
        if (indices == null || indices.isEmpty()) {
            // Remove all the flow filters in the specified list.
            ClearFlowFilterTask task = new ClearFlowFilterTask(ident, output);
            VTNFuture<List<FlowFilterResult>> f = vtnProvider.postSync(task);
            ret = new RpcFuture<List<FlowFilterResult>, RemoveFlowFilterOutput>(
                f, task);
        } else {
            // Remove only the specified flow filter.
            RemoveFlowFilterTask task =
                RemoveFlowFilterTask.create(ident, output, indices);
            VTNFuture<List<VtnUpdateType>> f = vtnProvider.postSync(task);
            ret = new RpcFuture<List<VtnUpdateType>, RemoveFlowFilterOutput>(
                f, task);
        }

        return ret;
    }

    /**
     * Receive a packet from the switch.
     *
     * @param pctx  A runtime context for a received packet.
     * @param src   The source MAC address of the packet.
     * @throws VTNException  An error occurred.
     */
    private void receive(PacketContext pctx, EtherAddress src)
        throws VTNException {
        // Determine the ingress port.
        TxContext ctx = pctx.getTxContext();
        SalPort ingress = pctx.getIngressPort();
        InventoryReader reader = pctx.getInventoryReader();
        VtnPort vport = reader.get(ingress);
        if (vport == null) {
            ctx.log(LOG, VTNLogLevel.WARN,
                    "Ignore packet from unknown port: {}",
                    pctx.getDescription());
            return;
        }

        if (InventoryUtils.hasPortLink(vport)) {
            ctx.log(LOG, VTNLogLevel.DEBUG,
                    "Ignore packet from internal node connector: {}",
                    ingress);
            return;
        }

        // Determine virtual network mapping that maps the packet.
        int vid = pctx.getVlanId();
        TenantNodeIdentifier<?, ?> ref = getMapping(ctx, src, ingress, vid);
        if (ref != null) {
            // Determine the VTN that maps the packet.
            VTenantIdentifier vtnId =
                new VTenantIdentifier(ref.getTenantName());
            Vtn vtn = vtnId.fetch(ctx.getReadWriteTransaction());
            new VTenant(pctx, vtnId, vtn).receive(pctx, ref);
        } else if (LOG.isTraceEnabled()) {
            LOG.trace("Ignore packet that is not mapped: {}",
                      pctx.getDescription());
        }
    }

    // VTNSubSystem

    /**
     * {@inheritDoc}
     */
    @Override
    public VTNFuture<?> initConfig(boolean master) {
        TxTask<?> task = (master)
            ? new VTenantLoadTask(vBridgeManager)
            : new VTenantSaveTask();
        return vtnProvider.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void initRpcServices(RpcProviderRegistry rpcReg,
                                CompositeAutoCloseable regs) {
        regs.add(rpcReg.addRpcImplementation(VtnService.class, this));
        regs.add(rpcReg.addRpcImplementation(VtnVterminalService.class, this));
        regs.add(rpcReg.addRpcImplementation(VtnFlowFilterService.class, this));

        vBridgeManager.initRpcServices(rpcReg, regs);

        VInterfaceService vifServ = new VInterfaceService(vtnProvider);
        vifServ.initRpcServices(rpcReg, regs);
    }

    // MultiDataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isDepth(VtnUpdateType type) {
        // Creation events should be processed from outer to inner.
        // Other events should be processed from inner to outer.
        return (type == VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isRequiredType(@Nonnull Class<?> type) {
        return LISTENERS.containsKey(type);
    }

    /**
     * Determine whether the specified type of the tree node should be
     * treated as a leaf node.
     *
     * @param type  A class that specifies the type of the tree node.
     *              Note that this value may not be the target data type
     *              specified by {@link #isRequiredType(Class)}.
     * @return  {@code true} if the specified type of the tree node should
     *          be treated as a leaf node. {@code false} otherwise.
     */
    @Override
    protected boolean isLeafNode(@Nonnull Class<?> type) {
        VNodeChangeListener<?> listener = LISTENERS.get(type);
        return (listener == null) ? false : listener.isLeafNode();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isUpdated(VTenantChange ectx, ChangedData<?> data) {
        Class<?> type = data.getIdentifier().getTargetType();
        VNodeChangeListener<?> listener = LISTENERS.get(type);
        return listener.isUpdated(data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VTenantChange enterEvent() {
        return new VTenantChange(vtnProvider, vBridgeManager);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(VTenantChange ectx) {
        ectx.apply(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
        VNodeChangeListener<?> listener = getChangeListener(ectx, data);
        listener.onCreated(ectx, data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(VTenantChange ectx, ChangedData<?> data) {
        VNodeChangeListener<?> listener = getChangeListener(ectx, data);
        listener.onUpdated(ectx, data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
        VNodeChangeListener<?> listener = getChangeListener(ectx, data);
        listener.onRemoved(ectx, data);
    }

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<Vtn> getWildcardPath() {
        return InstanceIdentifier.builder(Vtns.class).
            child(Vtn.class).build();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    // VTNInventoryListener

    /**
     * Invoked when a node information has been added or removed.
     *
     * @param ev  A {@link VtnNodeEvent} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    public void notifyVtnNode(VtnNodeEvent ev) throws VTNException {
        TxContext ctx = ev.getTxContext();
        if (ev.getUpdateType() == VtnUpdateType.REMOVED) {
            // Flush MAC address table entries detected on the removed node.
            SalNode snode = ev.getSalNode();
            NodeMacFilter filter = new NodeMacFilter(snode);
            try {
                new MacEntryRemover(filter).scan(ctx);
            } catch (VTNException | RuntimeException e) {
                ctx.log(LOG, VTNLogLevel.ERROR, e,
                        "Failed to flush MAC address table entry on node " +
                        "event: node=%s", snode);
                // FALLTHROUGH
            }
        }

        nodeHandler.scan(ctx, ev);
    }

    /**
     * Invoked when a port information has been added, removed, or changed.
     *
     * @param ev  A {@link VtnPortEvent} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    public void notifyVtnPort(VtnPortEvent ev) throws VTNException {
        TxContext ctx = ev.getTxContext();
        Boolean isl = ev.getInterSwitchLinkChange();
        if (ev.isDisabled() || Boolean.TRUE.equals(isl)) {
            // Flush MAC address table entries affected by the port if it has
            // been disabled or changed to ISL port.
            SalPort sport = ev.getSalPort();
            PortMacFilter filter = new PortMacFilter(sport);
            try {
                new MacEntryRemover(filter).scan(ctx);
            } catch (VTNException | RuntimeException e) {
                ctx.log(LOG, VTNLogLevel.ERROR, e,
                        "Failed to flush MAC address table entry on port " +
                        "event: port=%s", sport);
                // FALLTHROUGH
            }
        }

        portHandler.scan(ctx, ev);
    }

    // VTNRoutingListener

    /**
     * Invoked when the packet routing table has been updated.
     *
     * @param ev  A {@link RoutingEvent} instance.
     * @throws VTNException  An error occurred.
     */
    @Override
    public void routingUpdated(RoutingEvent ev) throws VTNException {
        // Remove resolved path faults.
        pathFaultResolver.scan(ev.getTxContext(), null);
    }

    // VTNPacketListener

    /**
     * Invoked when a packet has been received.
     *
     * @param ev  A {@link PacketInEvent} instance.
     */
    @Override
    public void notifyPacket(PacketInEvent ev) {
        PacketContext pctx = new PacketContext(ev);
        EtherAddress src = pctx.getSourceAddress();
        if (src.equals(pctx.getControllerAddress())) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("Ignore self-originated packet: {}",
                          pctx.getDescription());
            }
        } else {
            try {
                receive(pctx, src);
            } catch (VTNException | RuntimeException e) {
                TxContext ctx = ev.getTxContext();
                ctx.log(LOG, VTNLogLevel.ERROR, e,
                        "Ignore received packet due to error: %s",
                        pctx.getDescription());
            }
        }
    }

    // VtnService

    /**
     * Create or modify the specified VTN.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateVtnOutput>> updateVtn(UpdateVtnInput input) {
        try {
            // Create a task that updates the VTN.
            UpdateVtnTask task = UpdateVtnTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, UpdateVtnOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(UpdateVtnOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the specified VTN.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> removeVtn(RemoveVtnInput input) {
        try {
            // Create a task that removes the specified VTN.
            RemoveVtnTask task = RemoveVtnTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, Void>(taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(Void.class, e).buildFuture();
        }
    }

    // VtnVterminalService

    /**
     * Create or modify the specified vTerminal.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<UpdateVterminalOutput>> updateVterminal(
        UpdateVterminalInput input) {
        try {
            // Create a task that updates the vTerminal.
            UpdateVterminalTask task = UpdateVterminalTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, UpdateVterminalOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(UpdateVterminalOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the specified vTerminal.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<Void>> removeVterminal(RemoveVterminalInput input) {
        try {
            // Create a task that removes the specified vTerminal.
            RemoveVterminalTask task = RemoveVterminalTask.create(input);
            VTNFuture<VtnUpdateType> taskFuture = vtnProvider.postSync(task);
            return new RpcFuture<VtnUpdateType, Void>(taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(Void.class, e).buildFuture();
        }
    }

    // VtnFlowFilterService

    /**
     * Create or modify the flow filter.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<SetFlowFilterOutput>> setFlowFilter(
        SetFlowFilterInput input) {
        try {
            // Create a task that updates the flow filters.
            SetFlowFilterTask task = SetFlowFilterTask.create(input);
            VTNFuture<List<VtnUpdateType>> taskFuture =
                vtnProvider.postSync(task);
            return new RpcFuture<List<VtnUpdateType>, SetFlowFilterOutput>(
                taskFuture, task);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(SetFlowFilterOutput.class, e).
                buildFuture();
        }
    }

    /**
     * Remove the specified flow filters.
     *
     * @param input  Input of the RPC.
     * @return  A {@link Future} associated with the RPC task.
     */
    @Override
    public Future<RpcResult<RemoveFlowFilterOutput>> removeFlowFilter(
        RemoveFlowFilterInput input) {
        try {
            // Create a task that removes the specified flow filters.
            return startRpc(input);
        } catch (RpcException | RuntimeException e) {
            return RpcUtils.getErrorBuilder(RemoveFlowFilterOutput.class, e).
                buildFuture();
        }
    }
}
