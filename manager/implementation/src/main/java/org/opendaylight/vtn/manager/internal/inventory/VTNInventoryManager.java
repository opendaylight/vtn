/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicInteger;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
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
    implements AutoCloseable, DataChangeListener {
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
     *   This comparator associates {@link VtnNode} class with the order
     *   smaller than {@link VtnPort}.
     * </p>
     */
    private static final IdentifierTargetComparator  PATH_COMPARATOR;

    /**
     * Internal state that represents the service is alive.
     */
    private static final int  STATE_ALIVE = 0;

    /**
     * Internal state that represents the service is shut down.
     */
    private static final int  STATE_SHUTDOWN = 1;

    /**
     * Internal state that represents the service is closed.
     */
    private static final int  STATE_CLOSED = 2;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * Transaction submit queue for the VTN inventory model.
     */
    private final TxQueueImpl  inventoryQueue;

    /**
     * MD-SAL datastore listeners that listens MD-SAL inventory and topology
     * models.
     */
    private final CompositeAutoCloseable  dataListeners =
        new CompositeAutoCloseable(LOG);

    /**
     * A list of VTN inventory listeners.
     */
    private final CopyOnWriteArrayList<VTNInventoryListener>  vtnListeners =
        new CopyOnWriteArrayList<VTNInventoryListener>();

    /**
     * Internal state.
     */
    private final AtomicInteger  serviceState = new AtomicInteger(STATE_ALIVE);

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
        vtnProvider = provider;
        TxQueueImpl queue = new TxQueueImpl("VTN Inventory", provider);
        inventoryQueue = queue;

        // Initialize MD-SAL inventory listeners.
        DataBroker broker = provider.getDataBroker();
        InstanceIdentifier<VtnNode> path =
            InstanceIdentifier.builder(VtnNodes.class).child(VtnNode.class).
            build();
        try {
            dataListeners.add(new NodeListener(queue, broker));
            dataListeners.add(new NodeConnectorListener(queue, broker));
            dataListeners.add(new TopologyListener(queue, broker));
            inventoryQueue.start();

            // Register VTN inventory listener.
            dataListeners.add(broker.registerDataChangeListener(
                                  LogicalDatastoreType.OPERATIONAL, path, this,
                                  DataChangeScope.SUBTREE));
        } catch (Exception e) {
            String msg = "Failed to initialize inventory service.";
            LOG.error(msg, e);
            dataListeners.close();
            inventoryQueue.close();
            throw new IllegalStateException(msg, e);
        }
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
        return (serviceState.get() == STATE_ALIVE);
    }

    /**
     * Shutdown listener service.
     */
    public void shutdown() {
        serviceState.set(STATE_SHUTDOWN);
        vtnListeners.clear();
    }

    /**
     * Verify an instance identifier notified by a data change event.
     *
     * @param path  An instance identifier that specifies data.
     * @param type  A {@link VtnUpdateType} instance.
     * @return  {@code true} only if a given instance identifier  is valid.
     */
    private boolean checkPath(InstanceIdentifier<?> path, VtnUpdateType type) {
        if (path == null) {
            LOG.warn("{}: Null instance identifier.", type);
            return false;
        }
        if (path.isWildcarded()) {
            LOG.trace("{}: Ignore wildcard path: {}", type, path);
            return false;
        }

        Class<?> target = path.getTargetType();
        if (PATH_COMPARATOR.getOrder(target) == null) {
            LOG.trace("{}: Ignore unwanted path: {}", type, path);
            return false;
        }

        return true;
    }

    /**
     * Sort instance identifiers in the given set.
     *
     * <p>
     *   This method is used to determine order of data change event
     *   processing.
     * </p>
     *
     * @param set   A set of {@link InstanceIdentifier} instances.
     * @param type  A {@link VtnUpdateType} instance which specifies the type
     *              of data change event.
     * @param asc   If {@code true}, instance identifiers are sorted in
     *              ascending order. This means that all node events should be
     *              processed prior to port events.
     *              If {@code false}, instance identifiers are sorted in
     *              descending order. This means that all port events should be
     *              processed prior to node events.
     * @return  A sorted list of {@link InstanceIdentifier} instances.
     */
    private List<InstanceIdentifier<?>> sortPath(
        Set<InstanceIdentifier<?>> set, VtnUpdateType type, boolean asc) {
        List<InstanceIdentifier<?>> list = new ArrayList<>(set.size());
        Comparator<InstanceIdentifier<?>> comp = (asc)
            ? PATH_COMPARATOR
            : Collections.reverseOrder(PATH_COMPARATOR);

        // Eliminate unwanted paths.
        for (InstanceIdentifier<?> path: set) {
            if (checkPath(path, type)) {
                list.add(path);
            }
        }
        Collections.sort(list, comp);

        return list;
    }

    /**
     * Handle inventory creation or removal event.
     *
     * @param path  An instance identifier that specifies data.
     * @param data  A created or removed data object.
     * @param type  {@link VtnUpdateType#CREATED} on added,
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onCreatedOrRemoved(InstanceIdentifier<?> path,
                                    DataObject data, VtnUpdateType type) {
        String unexpected = null;
        Class<?> target = path.getTargetType();
        if (VtnNode.class.equals(target)) {
            if (data instanceof VtnNode) {
                VtnNode vnode = (VtnNode)data;
                LOG.info("Node has been {}: id={}, proto={}",
                         type.name().toLowerCase(Locale.ENGLISH),
                         vnode.getId().getValue(), vnode.getOpenflowVersion());
                postVtnNodeEvent(vnode, type);
                return;
            }
            unexpected = "node";
        } else if (VtnPort.class.equals(target)) {
            if (data instanceof VtnPort) {
                VtnPort vport = (VtnPort)data;
                Boolean isl = Boolean.valueOf(
                    InventoryUtils.hasPortLink(vport));
                LOG.info("Port has been {}: {}",
                         type.name().toLowerCase(Locale.ENGLISH),
                         InventoryUtils.toString(vport));
                postVtnPortEvent(vport, isl, type);
                return;
            }
            unexpected = "port";
        }

        if (unexpected != null) {
            LOG.warn("{}: Unexpected VTN {}: path={}, data={}",
                     type, unexpected, path, data);
        }
    }

    /**
     * Handle inventory change event.
     *
     * @param path     An instance identifier that specifies data.
     * @param oldData  A data object prior to the change.
     * @param newData  A changed data object.
     */
    private void onChanged(InstanceIdentifier<?> path, DataObject oldData,
                           DataObject newData) {
        String unexpected = null;
        Class<?> target = path.getTargetType();
        if (VtnNode.class.equals(target)) {
            if ((oldData instanceof VtnNode) && (newData instanceof VtnNode)) {
                onChanged((VtnNode)oldData, (VtnNode)newData);
                return;
            }
            unexpected = "node";
        } else if (VtnPort.class.equals(target)) {
            if ((oldData instanceof VtnPort) && (newData instanceof VtnPort)) {
                onChanged((VtnPort)oldData, (VtnPort)newData);
                return;
            }
            unexpected = "port";
        }

        if (unexpected != null) {
            LOG.warn("onChanged: Unexpected VTN {}: path={}, old={}, new={}",
                     unexpected, path, oldData, newData);
        }
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
        if (serviceState.getAndSet(STATE_CLOSED) != STATE_CLOSED) {
            vtnListeners.clear();
            dataListeners.close();
            inventoryQueue.close();
        }
    }

    // DataChangeListener

    /**
     * Invoked when VTN inventory data has been changed.
     *
     * @param ev  An {@link AsyncDataChangeEvent} instance.
     */
    @Override
    public void onDataChanged(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        if (ev == null) {
            LOG.warn("Null data change event.");
            return;
        }

        Map<InstanceIdentifier<?>, DataObject> created = ev.getCreatedData();
        if (created != null) {
            // Process creation events.
            // VtnNode events should be processed before VtnPort events.
            VtnUpdateType type = VtnUpdateType.CREATED;
            for (InstanceIdentifier<?> path:
                     sortPath(created.keySet(), type, true)) {
                DataObject value = created.get(path);
                onCreatedOrRemoved(path, value, type);
            }
        }

        Map<InstanceIdentifier<?>, DataObject> original = ev.getOriginalData();
        if (original == null) {
            original = Collections.
                <InstanceIdentifier<?>, DataObject>emptyMap();
        }

        Map<InstanceIdentifier<?>, DataObject> updated = ev.getUpdatedData();
        if (updated != null) {
            // Process change events.
            // VtnNode events should be processed before VtnPort events.
            VtnUpdateType type = VtnUpdateType.CHANGED;
            for (InstanceIdentifier<?> path:
                     sortPath(updated.keySet(), type, true)) {
                DataObject value = updated.get(path);
                DataObject org = original.get(path);
                onChanged(path, org, value);
            }
        }

        Set<InstanceIdentifier<?>> removed = ev.getRemovedPaths();
        if (removed != null) {
            // Process removal events.
            // VtnPort events should be processed before VtnNode events.
            VtnUpdateType type = VtnUpdateType.REMOVED;
            for (InstanceIdentifier<?> path: sortPath(removed, type, false)) {
                DataObject value = original.get(path);
                onCreatedOrRemoved(path, value, type);
            }
        }
    }
}
