/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.LOG_SEPARATOR;
import static org.opendaylight.vtn.manager.internal.vnode.MappingRegistry.getMapping;

import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.Future;

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
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionConverter;
import org.opendaylight.vtn.manager.internal.util.flow.filter.VTNFlowFilter;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcUtils;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodePathConverter;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeType;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.NodeMacFilter;
import org.opendaylight.vtn.manager.internal.util.vnode.mac.PortMacFilter;
import org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTenant;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.RemoveFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.SetFlowFilterOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappings;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappingsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapKey;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;

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
     * Comparator for the target type of instance identifier that specifies
     * the order of data change event processing.
     */
    private static final IdentifierTargetComparator  PATH_COMPARATOR;

    /**
     * Data change listeners for each data model in the VTN tree.
     */
    private static final Map<Class<?>, VNodeChangeListener<?>>  LISTENERS;

    /**
     * A tag that indicates the description.
     */
    private static final String  TAG_DESC = "desc=";

    /**
     * A tag that indicates the index number.
     */
    private static final String  TAG_INDEX = "index=";

    /**
     * A tag that indicates the physical switch.
     */
    private static final String  TAG_NODE = "node=";

    /**
     * A tag that indicates the state value.
     */
    private static final String  TAG_STATE = "state=";

    /**
     * A tag that indicates the condition name.
     */
    private static final String  TAG_COND = "cond=";

    /**
     * A tag that indicates the idle-timeout value.
     */
    private static final String  TAG_IDLE = "idle=";

    /**
     * A tag that indicates the hard-timeout value.
     */
    private static final String  TAG_HARD = "hard=";

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
    private final PathFaultResolver  pathFaultResolver
        = new PathFaultResolver();

    /**
     * Initialize static fields.
     */
    static {
        // Create a path comparator that assigns higher priority to inner data.
        int order = 0;
        PATH_COMPARATOR = new IdentifierTargetComparator().
            setOrder(VtnFlowAction.class, ++order).
            setOrder(VtnFlowFilter.class, ++order).
            setOrder(VinterfaceStatus.class, ++order).
            setOrder(PortMapConfig.class, ++order).
            setOrder(VinterfaceConfig.class, ++order).
            setOrder(VlanMapStatus.class, ++order).
            setOrder(VlanMapConfig.class, ++order).
            setOrder(MappedHost.class, ++order).
            setOrder(VlanHostDescList.class, ++order).
            setOrder(MacMapConfig.class, ++order).
            setOrder(FaultedPaths.class, ++order).
            setOrder(BridgeStatus.class, ++order).
            setOrder(VterminalConfig.class, ++order).
            setOrder(VbridgeConfig.class, ++order).
            setOrder(VtnPathMap.class, ++order).
            setOrder(VtenantConfig.class, ++order).
            setOrder(Vtn.class, ++order);

        // Create data change listeners.
        LISTENERS = ImmutableMap.<Class<?>, VNodeChangeListener<?>>builder().
            put(VtnFlowAction.class, new VtnFlowActionListener()).
            put(VtnFlowFilter.class, new VtnFlowFilterListener()).
            put(VinterfaceStatus.class, new VinterfaceStatusListener()).
            put(PortMapConfig.class, new PortMapConfigListener()).
            put(VinterfaceConfig.class, new VinterfaceConfigListener()).
            put(VlanMapStatus.class, new VlanMapStatusListener()).
            put(VlanMapConfig.class, new VlanMapConfigListener()).
            put(MappedHost.class, new MappedHostListener()).
            put(VlanHostDescList.class, new VlanHostDescListListener()).
            put(MacMapConfig.class, new MacMapConfigListener()).
            put(FaultedPaths.class, new FaultedPathsListener()).
            put(BridgeStatus.class, new BridgeStatusListener()).
            put(VterminalConfig.class, new VterminalConfigListener()).
            put(VbridgeConfig.class, new VbridgeConfigListener()).
            put(VtnPathMap.class, new VtnPathMapListener()).
            put(VtenantConfig.class, new VtenantConfigListener()).
            put(Vtn.class, new VtnListener()).
            build();
    }

    /**
     * Return a string that indicates the change of value.
     *
     * @param old    The old value.
     * @param value  The current value.
     * @param <T>    The type of value.
     * @return  A string that indicates the change of value.
     */
    private static <T> String getChangedMessage(T old, T value) {
        return (old.equals(value))
            ? old.toString()
            : "(" + old + "->" + value + ")";
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
     * Data change listener for vtn-flow-action in vtn-flow-filter.
     */
    private static final class VtnFlowActionListener
        extends VNodeLogListener<VtnFlowAction> {
        /**
         * Construct a new instance.
         */
        private VtnFlowActionListener() {
            super(VtnFlowAction.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VtnFlowAction> path) {
            String direction = (VTNFlowFilter.isOutput(path))
                ? "out" : "in";
            VtnFlowFilterKey key = path.firstKeyOf(VtnFlowFilter.class);
            Integer index = (key == null) ? null : key.getIndex();
            return direction + "." + index + ": Flow filter action";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VtnFlowAction value) {
            FlowActionConverter conv = FlowActionConverter.getInstance();
            return "order=" + value.getOrder() +
                ", action=" + conv.getDescription(value.getVtnAction());
        }
    }

    /**
     * Data change listener for vtn-flow-filter.
     */
    private static final class VtnFlowFilterListener
        extends VNodeLogListener<VtnFlowFilter> {
        /**
         * Construct a new instance.
         */
        private VtnFlowFilterListener() {
            super(VtnFlowFilter.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VtnFlowFilter> path) {
            return (VTNFlowFilter.isOutput(path))
                ? "Output flow filter" : "Input flow filter";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VtnFlowFilter value) {
            return TAG_INDEX + value.getIndex() + LOG_SEPARATOR +
                TAG_COND + value.getCondition().getValue() +
                ", type=" + VTNFlowFilter.getTypeDescription(value);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isChanged(VtnFlowFilter old, VtnFlowFilter value) {
            return !(Objects.equals(old.getCondition(),
                                    value.getCondition()) &&
                     Objects.equals(old.getVtnFlowFilterType(),
                                    value.getVtnFlowFilterType()));
        }
    }

    /**
     * Data change listener for vinterface-status.
     */
    private static final class VinterfaceStatusListener
        extends VNodeLogListener<VinterfaceStatus> {
        /**
         * Construct a new instance.
         */
        private VinterfaceStatusListener() {
            super(VinterfaceStatus.class, false);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VinterfaceStatus> path) {
            return "vInterface status";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VinterfaceStatus value) {
            StringBuilder builder = new StringBuilder(TAG_STATE).
                append(value.getState()).
                append(", entity-state=").append(value.getEntityState());
            String port = MiscUtils.getValue(value.getMappedPort());
            if (port != null) {
                builder.append(", mapped-port=").append(port);
            }

            return builder.toString();
        }
    }

    /**
     * Data change listener for port-map-config.
     */
    private static final class PortMapConfigListener
        extends VNodeLogListener<PortMapConfig> {
        /**
         * Construct a new instance.
         */
        private PortMapConfigListener() {
            super(PortMapConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<PortMapConfig> path) {
            return "Port mapping";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(PortMapConfig value) {
            StringBuilder builder = new StringBuilder(TAG_NODE).
                append(MiscUtils.getValue(value.getNode()));
            String id = value.getPortId();
            if (id != null) {
                builder.append(", port-id=").append(id);
            }
            String name = value.getPortName();
            if (name != null) {
                builder.append(", port-name=").append(name);
            }

            return builder.toString();
        }
    }

    /**
     * Data change listener for vinterface-config.
     */
    private static final class VinterfaceConfigListener
        extends VNodeLogListener<VinterfaceConfig> {
        /**
         * Construct a new instance.
         */
        private VinterfaceConfigListener() {
            super(VinterfaceConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VinterfaceConfig> path) {
            return VInterfaceIdentifier.DESCRIPTION;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VinterfaceConfig value) {
            String desc = value.getDescription();
            StringBuilder builder = new StringBuilder();
            if (desc != null) {
                builder.append(TAG_DESC).append(desc).append(LOG_SEPARATOR);
            }

            return builder.append("enabled=").append(value.isEnabled()).
                toString();
        }
    }

    /**
     * Data change listener for vlan-map-status.
     */
    private static final class VlanMapStatusListener
        extends VNodeChangeListener<VlanMapStatus> {
        /**
         * Construct a new instance.
         */
        private VlanMapStatusListener() {
            super(VlanMapStatus.class, false);
        }

        /**
         * Log the given status of VLAN mapping.
         *
         * @param data  A {@link IdentifiedData} instance.
         */
        private void onChanged(IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                IdentifiedData<VlanMapStatus> cdata = cast(data);
                InstanceIdentifier<VlanMapStatus> path = cdata.getIdentifier();
                VlanMapKey key = path.firstKeyOf(VlanMap.class);
                VlanMapStatus status = cdata.getValue();
                Boolean active = status.isActive();
                String desc = (Boolean.TRUE.equals(active))
                    ? "activated" : "inactivated";
                LOG.info("{}: VLAN mapping has been {}: map-id={}",
                         getVNodeIdentifier(path), desc, key.getMapId());
            }
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            onChanged(data);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            onChanged(data);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            // Nothing to do.
        }
    }

    /**
     * Data change listener for vlan-map-config.
     */
    private static final class VlanMapConfigListener
        extends VNodeLogListener<VlanMapConfig> {
        /**
         * Construct a new instance.
         */
        private VlanMapConfigListener() {
            super(VlanMapConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VlanMapConfig> path) {
            return "VLAN mapping";
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VlanMapConfig value) {
            String node = MiscUtils.getValue(value.getNode());
            StringBuilder builder = new StringBuilder();
            if (node != null) {
                builder.append(TAG_NODE).append(node).append(LOG_SEPARATOR);
            }

            return builder.append("vlan-id=").
                append(value.getVlanId().getValue()).
                toString();
        }
    }

    /**
     * Data change listener for mapped-host.
     */
    private static final class MappedHostListener
        extends VNodeChangeListener<MappedHost> {
        /**
         * Construct a new instance.
         */
        private MappedHostListener() {
            super(MappedHost.class, false);
        }

        /**
         * Log the given host information mapped by MAC mapping.
         *
         * @param data  A {@link IdentifiedData} instance.
         * @param msg   A message to be logged.
         */
        private void onChanged(IdentifiedData<?> data, String msg) {
            if (LOG.isInfoEnabled()) {
                IdentifiedData<MappedHost> cdata = cast(data);
                MappedHost host = cdata.getValue();
                VNodeIdentifier<?> ident =
                    getVNodeIdentifier(cdata.getIdentifier());
                LOG.info("{}: A host has been {} MAC mapping: addr={}, " +
                         "vlan-id={}, port={}", ident, msg,
                         host.getMacAddress().getValue(),
                         host.getVlanId().getValue(),
                         MiscUtils.getValue(host.getPortId()));
            }
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            onChanged(data, "registered to");
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            if (LOG.isInfoEnabled()) {
                ChangedData<MappedHost> cdata = cast(data);
                VNodeIdentifier<?> ident =
                    getVNodeIdentifier(cdata.getIdentifier());
                MappedHost host = cdata.getValue();
                MappedHost old = cdata.getOldValue();
                String vidMsg = getChangedMessage(
                    old.getVlanId().getValue(), host.getVlanId().getValue());
                String portMsg = getChangedMessage(
                    MiscUtils.getValue(old.getPortId()),
                    MiscUtils.getValue(host.getPortId()));
                LOG.info("{}: A MAC mapped host has been changed: " +
                         "addr={}, vlan-id={}, port={}", ident,
                         host.getMacAddress().getValue(), vidMsg, portMsg);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            onChanged(data, "unregistered from");
        }
    }

    /**
     * Data change listener for vlan-host-desc-list in the mac-map-config.
     */
    private static final class VlanHostDescListListener
        extends VNodeChangeListener<VlanHostDescList> {
        /**
         * Construct a new instance.
         */
        private VlanHostDescListListener() {
            super(VlanHostDescList.class, true);
        }

        /**
         * Invoked when a host informatiion has been added to or removed from
         * the access control list in the MAC mapping configuration.
         *
         * @param data  An {@link IdentifiedData} instance which contains
         *              added or removed data.
         * @param msg   A string used to construct a log message.
         */
        private void onChanged(IdentifiedData<?> data, String msg) {
            // Determine the name of the access control list.
            IdentifiedData<VlanHostDescList> cdata = cast(data);
            InstanceIdentifier<VlanHostDescList> path = cdata.getIdentifier();
            InstanceIdentifier<AllowedHosts> cpath =
                path.firstIdentifierOf(AllowedHosts.class);
            String acl = (cpath == null) ? "denied-hosts" : "allowed-hosts";

            LOG.info("{}: A host has been {} {}: {}",
                     getVNodeIdentifier(path), msg, acl,
                     cdata.getValue().getHost().getValue());
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data, "added to");
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            // Nothing to do.
            // The contents of vlan-host-desc-list should never be changed.
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data, "removed from");
            }
        }
    }

    /**
     * Data change listener for mac-map-config.
     */
    private static final class MacMapConfigListener
        extends VNodeChangeListener<MacMapConfig> {
        /**
         * Construct a new instance.
         */
        private MacMapConfigListener() {
            super(MacMapConfig.class, true);
        }

        /**
         * Invoked when the MAC mapping has been created or removed.
         *
         * @param data  An {@link IdentifiedData} instance which contains
         *              added or removed data.
         * @param type  {@link VtnUpdateType#CREATED} on added,
         *              {@link VtnUpdateType#REMOVED} on removed.
         */
        private void onChanged(IdentifiedData<?> data, VtnUpdateType type) {
            IdentifiedData<MacMapConfig> cdata = cast(data);
            InstanceIdentifier<MacMapConfig> path = cdata.getIdentifier();
            LOG.info("{}: MAC mapping has been {}.",
                     getVNodeIdentifier(path), MiscUtils.toLowerCase(type));
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data, VtnUpdateType.CREATED);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            // Nothing to do.
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                onChanged(data, VtnUpdateType.REMOVED);
            }
        }
    }

    /**
     * Data change listener for faulted-paths.
     */
    private static final class FaultedPathsListener
        extends VNodeChangeListener<FaultedPaths> {
        /**
         * Construct a new instance.
         */
        private FaultedPathsListener() {
            super(FaultedPaths.class, false);
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            IdentifiedData<FaultedPaths> cdata = cast(data);
            VNodeIdentifier<?> ident =
                getVNodeIdentifier(cdata.getIdentifier());
            FaultedPaths value = cdata.getValue();
            LOG.warn("{}: Path fault: {} -> {}", ident,
                     MiscUtils.getValue(value.getSource()),
                     MiscUtils.getValue(value.getDestination()));
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            // Nothing to do.
            // The contents of faulted-paths should never be changed.
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            // Record the given path fault into VTenantChange.
            // It will be logged unless the vBridge was removed.
            IdentifiedData<FaultedPaths> cdata = cast(data);
            VNodeIdentifier<?> ident =
                getVNodeIdentifier(cdata.getIdentifier());
            ectx.addResolvedPathFault(ident, cdata.getValue());
        }
    }

    /**
     * Data change listener for bridge-status.
     */
    private static final class BridgeStatusListener
        extends VNodeChangeListener<BridgeStatus> {
        /**
         * Construct a new instance.
         */
        private BridgeStatusListener() {
            super(BridgeStatus.class, false);
        }

        /**
         * Return a string that represents the contents of bridge-status.
         *
         * @param value  An instance of {@link BridgeStatus}.
         * @return  A string representation of the given value.
         */
        private String toString(BridgeStatus value) {
            // No need to log faulted-paths.
            return TAG_STATE + value.getState() +
                ", path-faults=" + value.getPathFaults();
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            if (LOG.isInfoEnabled()) {
                IdentifiedData<BridgeStatus> cdata = cast(data);
                VNodeIdentifier<?> ident =
                    getVNodeIdentifier(cdata.getIdentifier());
                VNodeType type = ident.getType();
                BridgeStatus value = cdata.getValue();
                LOG.info("{}: Initial {} status: {}",
                         ident, type.toString(), toString(value));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            if (LOG.isInfoEnabled()) {
                ChangedData<BridgeStatus> cdata = cast(data);
                VNodeIdentifier<?> ident =
                    getVNodeIdentifier(cdata.getIdentifier());

                // Log resolved path faults.
                for (FaultedPaths fp: ectx.getResolvedPathFaults(ident)) {
                    LOG.info("{}: Path fault has been resolved: {} -> {}",
                             ident, MiscUtils.getValue(fp.getSource()),
                             MiscUtils.getValue(fp.getDestination()));
                }

                VNodeType type = ident.getType();
                BridgeStatus value = cdata.getValue();
                BridgeStatus old = cdata.getOldValue();
                LOG.info("{}: {} status has been changed: old={{}}, new={{}}",
                         ident, type.toString(),
                         toString(old), toString(value));
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            // Nothing to do.
        }
    }

    /**
     * Data change listener for vterminal-config.
     */
    private static final class VterminalConfigListener
        extends VNodeLogListener<VterminalConfig> {
        /**
         * Construct a new instance.
         */
        private VterminalConfigListener() {
            super(VterminalConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VterminalConfig> path) {
            return VNodeType.VTERMINAL.toString();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VterminalConfig value) {
            String desc = value.getDescription();
            StringBuilder builder = new StringBuilder();
            if (desc != null) {
                builder.append(TAG_DESC).append(desc);
            }

            return builder.toString();
        }
    }

    /**
     * Data change listener for vbridge-config.
     */
    private static final class VbridgeConfigListener
        extends VNodeChangeListener<VbridgeConfig> {
        /**
         * Construct a new instance.
         */
        private VbridgeConfigListener() {
            super(VbridgeConfig.class, true);
        }

        /**
         * Return the identifier for the vBridge that contains the given data.
         *
         * @param data  An {@link IdentifiedData} instance.
         * @return  A {@link VBridgeIdentifier} instance.
         */
        private VBridgeIdentifier getVBridgeIdentifier(
            IdentifiedData<VbridgeConfig> data) {
            return new VNodePathConverter(data.getIdentifier()).
                getIdentifier(VBridgeIdentifier.class);
        }

        /**
         * Convert the vBridge configuration into a string.
         *
         * @param value  The vBridge configuration to be converted.
         * @return  A string representation of the given vBridge configuration.
         */
        private String toString(VbridgeConfig value) {
            String desc = value.getDescription();
            StringBuilder builder = new StringBuilder();
            if (desc != null) {
                builder.append(TAG_DESC).append(desc).append(LOG_SEPARATOR);
            }

            return builder.append("age-interval=").
                append(value.getAgeInterval()).
                toString();
        }

        /**
         * Record a log message for the given vBridge configuration.
         *
         * @param ident  The identifier for the vBridge.
         * @param vbrc   The vBridge configuration.
         * @param type   {@link VtnUpdateType#CREATED} on added,
         *               {@link VtnUpdateType#REMOVED} on removed.
         */
        protected void log(VBridgeIdentifier ident, VbridgeConfig vbrc,
                           VtnUpdateType type) {
            if (LOG.isInfoEnabled()) {
                LOG.info("{}: vBridge has been {}: config={{}}",
                         ident, MiscUtils.toLowerCase(type),
                         toString(vbrc));
            }
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            // Record a log message.
            IdentifiedData<VbridgeConfig> cdata = cast(data);
            VBridgeIdentifier ident = getVBridgeIdentifier(cdata);
            VbridgeConfig vbrc = cdata.getValue();
            log(ident, vbrc, VtnUpdateType.CREATED);

            // Create the entity of the vBridge.
            ectx.updateBridge(ident, vbrc);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            // Record a log message.
            ChangedData<VbridgeConfig> cdata = cast(data);
            VBridgeIdentifier ident = getVBridgeIdentifier(cdata);
            VbridgeConfig vbrc = cdata.getValue();
            if (LOG.isInfoEnabled()) {
                VbridgeConfig old = cdata.getOldValue();
                LOG.info("{}: vBridge has been changed: old={{}}, new={{}}",
                         ident, toString(old), toString(vbrc));
            }

            // Update the entity of the vBridge.
            ectx.updateBridge(ident, vbrc);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            // Record a log message.
            IdentifiedData<VbridgeConfig> cdata = cast(data);
            VBridgeIdentifier ident = getVBridgeIdentifier(cdata);
            VbridgeConfig vbrc = cdata.getValue();
            log(ident, vbrc, VtnUpdateType.REMOVED);

            // Remove the entity of the vBridge.
            ectx.removeBridge(ident);
        }
    }

    /**
     * Data change listener for vtn-path-map.
     */
    private static final class VtnPathMapListener
        extends VNodeChangeListener<VtnPathMap> {
        /**
         * Construct a new instance.
         */
        private VtnPathMapListener() {
            super(VtnPathMap.class, true);
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            PathMapUtils.log(LOG, data, VtnUpdateType.CREATED);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            PathMapUtils.log(LOG, data);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            PathMapUtils.log(LOG, data, VtnUpdateType.REMOVED);
        }
    }

    /**
     * Data change listener for vtenant-config.
     */
    private static final class VtenantConfigListener
        extends VNodeLogListener<VtenantConfig> {
        /**
         * Construct a new instance.
         */
        private VtenantConfigListener() {
            super(VtenantConfig.class, true);
        }

        // VNodeLogListener

        /**
         * {@inheritDoc}
         */
        @Override
        String getDescription(InstanceIdentifier<VtenantConfig> path) {
            return VNodeType.VTN.toString();
        }

        /**
         * {@inheritDoc}
         */
        @Override
        String toString(VtenantConfig value) {
            StringBuilder builder = new StringBuilder();
            String sep = "";

            String desc = value.getDescription();
            if (desc != null) {
                builder.append(TAG_DESC).append(desc);
                sep = LOG_SEPARATOR;
            }

            Integer idle = value.getIdleTimeout();
            if (idle != null) {
                builder.append(sep).append(TAG_IDLE).append(idle);
                sep = LOG_SEPARATOR;
            }

            Integer hard = value.getHardTimeout();
            if (hard != null) {
                builder.append(sep).append(TAG_HARD).append(hard);
            }

            return builder.toString();
        }
    }

    /**
     * Data change listener for VTN container.
     */
    private static final class VtnListener
        extends VNodeChangeListener<Vtn> {
        /**
         * Construct a new instance.
         */
        private VtnListener() {
            super(Vtn.class, false);
        }

        /**
         * Record the given VTN tree for saving configuration.
         *
         * @param ectx     A {@link VTenantChange} instance.
         * @param data     A {@link IdentifiedData} instance that contains a
         *                 VTN data tree.
         * @param created  {@code true} indicates that the given VTN has been
         *                 created.
         */
        private void onChanged(VTenantChange ectx, IdentifiedData<?> data,
                               boolean created) {
            IdentifiedData<Vtn> cdata = cast(data);
            Vtn vtn = cdata.getValue();
            String name = vtn.getName().getValue();
            ectx.addUpdatedVtn(name, vtn, created);
        }

        // VNodeChangeListener

        /**
         * {@inheritDoc}
         */
        @Override
        void onCreated(VTenantChange ectx, IdentifiedData<?> data) {
            onChanged(ectx, data, true);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onUpdated(VTenantChange ectx, ChangedData<?> data) {
            onChanged(ectx, data, false);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        void onRemoved(VTenantChange ectx, IdentifiedData<?> data) {
            IdentifiedData<Vtn> cdata = cast(data);
            Vtn vtn = cdata.getValue();
            String name = vtn.getName().getValue();
            ectx.addRemoved(name);
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
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE, true);
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
     * @param ectx     A {@link VTenantChange} instance.
     * @param data     A {@link IdentifiedData} instance.
     * @return  A {@link VNodeChangeListener} instance for the given data.
     *                 created.
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
    protected IdentifierTargetComparator getComparator() {
        return PATH_COMPARATOR;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean getOrder(VtnUpdateType type) {
        // Creation events should be processed from outer to inner.
        // Other events should be processed from inner to outer.
        return (type != VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VTenantChange enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
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
     * @throws VTNException  An error occurred.
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
