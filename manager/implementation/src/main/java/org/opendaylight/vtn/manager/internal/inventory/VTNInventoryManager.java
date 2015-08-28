/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicBoolean;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Internal inventory manager.
 */
public final class VTNInventoryManager
    extends MultiDataStoreListener<VtnNode, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNInventoryManager.class);

    /**
     * Comparator for the target type of instance identifier that specifies
     * the order of data change event processing.
     *
     * <p>
     *   This comparator associates {@link VtnPort} class with the order
     *   smaller than {@link VtnNode}.
     * </p>
     */
    private static final IdentifierTargetComparator  PATH_COMPARATOR;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A list of VTN inventory listeners.
     */
    private final CopyOnWriteArrayList<VTNInventoryListener>  vtnListeners =
        new CopyOnWriteArrayList<VTNInventoryListener>();

    /**
     * MD-SAL datastore transaction queue for inventory information.
     */
    private final TxQueueImpl  inventoryQueue;

    /**
     * Internal state.
     */
    private final AtomicBoolean  serviceState = new AtomicBoolean(true);

    /**
     * Initialize static fields.
     */
    static {
        PATH_COMPARATOR = new IdentifierTargetComparator().
            setOrder(VtnNode.class, 1).
            setOrder(VtnPort.class, 2);
    }

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     */
    public VTNInventoryManager(VTNManagerProvider provider) {
        super(VtnNode.class);
        vtnProvider = provider;
        TxQueueImpl queue = new TxQueueImpl("VTN Inventory", provider);
        inventoryQueue = queue;
        addCloseable(queue);

        // Initialize MD-SAL inventory listeners.
        DataBroker broker = provider.getDataBroker();
        try {
            addCloseable(new NodeListener(queue, broker));
            addCloseable(new NodeConnectorListener(queue, broker));
            addCloseable(new TopologyListener(queue, broker));

            // Register VTN inventory listener.
            registerListener(broker, LogicalDatastoreType.OPERATIONAL,
                             DataChangeScope.SUBTREE);
        } catch (Exception e) {
            String msg = "Failed to initialize inventory service.";
            LOG.error(msg, e);
            close();
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Create a new static network topology manager.
     *
     * @return  A {@link StaticTopologyManager} instance.
     */
    public StaticTopologyManager newStaticTopologyManager() {
        return new StaticTopologyManager(vtnProvider, inventoryQueue);
    }

    /**
     * Start the inventory service.
     */
    public void start() {
        inventoryQueue.start();
    }

    /**
     * Add the given VTN inventory listener.
     *
     * @param l  A VTN inventory listener.
     */
    public void addListener(VTNInventoryListener l) {
        vtnListeners.addIfAbsent(l);
    }

    /**
     * Determine whether the VTN inventory service is alive or not.
     *
     * @return  {@code true} only if the VTN inventory service is alive.
     */
    public boolean isAlive() {
        return serviceState.get();
    }

    /**
     * Shutdown listener service.
     */
    public void shutdown() {
        serviceState.set(false);
        vtnListeners.clear();
    }

    /**
     * Handle inventory creation or removal event.
     *
     * @param data  A created or removed data object.
     * @param type  {@link VtnUpdateType#CREATED} on added,
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onCreatedOrRemoved(IdentifiedData<?> data,
                                    VtnUpdateType type) {
        IdentifiedData<VtnNode> nodeData = data.checkType(VtnNode.class);
        if (nodeData != null) {
            VtnNode vnode = nodeData.getValue();
            LOG.info("Node has been {}: id={}, proto={}",
                     MiscUtils.toLowerCase(type.name()),
                     vnode.getId().getValue(), vnode.getOpenflowVersion());
            postVtnNodeEvent(vnode, type);
            return;
        }

        IdentifiedData<VtnPort> portData = data.checkType(VtnPort.class);
        if (portData != null) {
            VtnPort vport = portData.getValue();
            Boolean isl = Boolean.valueOf(InventoryUtils.hasPortLink(vport));
            LOG.info("Port has been {}: {}",
                     MiscUtils.toLowerCase(type.name()),
                     InventoryUtils.toString(vport));
            postVtnPortEvent(vport, isl, type);
            return;
        }

        // This should never happen.
        LOG.warn("{}: Unexpected event: path={}, data={}",
                 type, data.getIdentifier(), data.getValue());
    }

    /**
     * Handle VTN node change event.
     *
     * @param oldNode  A {@link VtnNode} instance before updated.
     * @param newNode  An updated {@link VtnNode} instance.
     */
    private void onChanged(VtnNode oldNode, VtnNode newNode) {
        VtnOpenflowVersion oldVer = oldNode.getOpenflowVersion();
        VtnOpenflowVersion newVer = newNode.getOpenflowVersion();
        if (oldVer == null && newVer != null) {
            LOG.info("{}: Protocol version has been detected: {}",
                     newNode.getId().getValue(), newVer);
        }
    }

    /**
     * Handle VTN port change event.
     *
     * @param oldPort  A {@link VtnPort} instance before updated.
     * @param newPort  An updated {@link VtnPort} instance.
     */
    private void onChanged(VtnPort oldPort, VtnPort newPort) {
        boolean oldIsl = InventoryUtils.hasPortLink(oldPort);
        boolean newIsl = InventoryUtils.hasPortLink(newPort);
        Boolean isl = (oldIsl == newIsl)
            ? null : Boolean.valueOf(newIsl);
        LOG.info("Port has been changed: old={}, new={}",
                 InventoryUtils.toString(oldPort),
                 InventoryUtils.toString(newPort));
        postVtnPortEvent(newPort, isl, VtnUpdateType.CHANGED);
    }

    /**
     * Post a VTN node event.
     *
     * @param vnode  A {@link VtnNode} instance.
     * @param type   A {@link VtnUpdateType} instance.
     */
    private void postVtnNodeEvent(VtnNode vnode, VtnUpdateType type) {
        VtnNodeEvent ev = null;
        for (VTNInventoryListener l: vtnListeners) {
            ev = (ev == null)
                ? new VtnNodeEvent(l, vnode, type)
                : new VtnNodeEvent(l, ev);
            vtnProvider.post(ev);
        }
    }

    /**
     * Post a VTN port event.
     *
     * @param vport  A {@link VtnPort} instance.
     * @param isl    A {@link Boolean} instance which describes the change of
     *               inter-switch link state.
     * @param type   A {@link VtnUpdateType} instance.
     */
    private void postVtnPortEvent(VtnPort vport, Boolean isl,
                                  VtnUpdateType type) {
        VtnPortEvent ev = null;
        for (VTNInventoryListener l: vtnListeners) {
            ev = (ev == null)
                ? new VtnPortEvent(l, vport, isl, type)
                : new VtnPortEvent(l, ev);
            vtnProvider.post(ev);
        }
    }

    // AutoCloseable

    /**
     * Close the VTN inventory service.
     */
    @Override
    public void close() {
        shutdown();
        super.close();
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
        // Removal events should be processed from inner to outer.
        // Other events should be processed from outer to inner.
        return (type != VtnUpdateType.REMOVED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Void enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(Void ectx) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(Void ectx, IdentifiedData<?> data) {
        onCreatedOrRemoved(data, VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx, ChangedData<?> data) {
        ChangedData<VtnNode> nodeData = data.checkType(VtnNode.class);
        if (nodeData != null) {
            onChanged(nodeData.getOldValue(), nodeData.getValue());
            return;
        }

        ChangedData<VtnPort> portData = data.checkType(VtnPort.class);
        if (portData != null) {
            onChanged(portData.getOldValue(), portData.getValue());
            return;
        }

        // This should never happen.
        LOG.warn("CHANGED: Unexpected event: path={}, old={}, new={}",
                 data.getIdentifier(), data.getOldValue(), data.getValue());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx, IdentifiedData<?> data) {
        onCreatedOrRemoved(data, VtnUpdateType.REMOVED);
    }

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnNode> getWildcardPath() {
        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class).build();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
