/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.TestBase.newKeyedModification;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.BuildHelper;
import org.opendaylight.vtn.manager.internal.ItemBuildHelper;
import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.Identifiable;
import org.opendaylight.yangtools.yang.binding.Identifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;

/**
 * Helper class for building virtual bridge instance.
 *
 * @param <T>  The type of the data object.
 * @param <B>  The type of the builder class.
 * @param <K>  The type of the key.
 */
public abstract class BridgeBuildHelper
    <T extends VtnPortMappableBridge & Identifiable<K>, B extends Builder<T>,
     K extends Identifier<T>> extends BuildHelper<T, B> {
    /**
     * A class that specifies the target data type.
     */
    private final Class<T>  targetType;

    /**
     * The virtual bridge status.
     */
    private BridgeStatusBuildHelper  status;

    /**
     * A set of virtual interfaces.
     */
    private Map<VnodeName, VinterfaceBuildHelper>  vInterfaces =
        new LinkedHashMap<>();

    /**
     * Construct a new instance.
     *
     * @param type     A class that specifies the target type.
     * @param builder  The builder instance.
     */
    protected BridgeBuildHelper(Class<T> type, B builder) {
        super(builder);
        targetType = type;
    }

    /**
     * Create the bridge status.
     *
     * @param state    The state of the bridge.
     * @param pfaults  The number of faulted paths.
     * @return  A {@link BridgeStatusBuildHelper} instance.
     */
    public final BridgeStatusBuildHelper setStatus(
        VnodeState state, Integer pfaults) {
        BridgeStatusBuildHelper stb = status;
        if (stb == null) {
            stb = new BridgeStatusBuildHelper();
            stb.getBuilder().
                setState(state).
                setPathFaults(pfaults);
            status = stb;
        }

        return stb;
    }

    /**
     * Return the bridge status.
     *
     * @return  A {@link BridgeStatusBuildHelper} instance or {@code null}.
     */
    public final BridgeStatusBuildHelper getStatus() {
        return status;
    }

    /**
     * Create the {@link VinterfaceBuildHelper} instance for the specified
     * virtual interface.
     *
     * @param name  The name of the virtual interface.
     * @return  A {@link VinterfaceBuildHelper} instance.
     */
    public final VinterfaceBuildHelper createInterface(String name) {
        return createInterface(new VnodeName(name));
    }

    /**
     * Create the {@link VinterfaceBuildHelper} instance for the specified
     * virtual interface.
     *
     * @param vname  The name of the virtual interface.
     * @return  A {@link VinterfaceBuildHelper} instance.
     */
    public final VinterfaceBuildHelper createInterface(VnodeName vname) {
        checkNotFrozen();
        VinterfaceBuildHelper ihelper = vInterfaces.get(vname);
        if (ihelper == null) {
            ihelper = new VinterfaceBuildHelper(vname);
            vInterfaces.put(vname, ihelper);
        }

        return ihelper;
    }

    /**
     * Return the {@link VinterfaceBuildHelper} instance associated with the
     * specified virtual interface.
     *
     * @param name  The name of the virtual interface.
     * @return  A {@link VinterfaceBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public final VinterfaceBuildHelper getInterface(String name) {
        return getInterface(new VnodeName(name));
    }

    /**
     * Return the {@link VinterfaceBuildHelper} instance associated with the
     * specified virtual interface.
     *
     * @param vname  The name of the virtual interface.
     * @return  A {@link VinterfaceBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public final VinterfaceBuildHelper getInterface(VnodeName vname) {
        return vInterfaces.get(vname);
    }

    /**
     * Return a list of virtual interfaces.
     *
     * @return  A list of {@link VinterfaceBuildHelper} instances.
     */
    public final List<VinterfaceBuildHelper> getInterfaces() {
        return new ArrayList<>(vInterfaces.values());
    }

    /**
     * Return a set of virtual interface names.
     *
     * @return  A set of {@link VnodeName} instances.
     */
    public final Set<VnodeName> getInterfaceKeys() {
        return new LinkedHashSet<>(vInterfaces.keySet());
    }

    /**
     * Create a new data object modification that represents creation of
     * this bridge.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public final DataObjectModification<T> newCreatedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        newCreatedChildModification(children);
        ItemBuildHelper.newCreatedModification(children, status);

        for (VinterfaceBuildHelper child: vInterfaces.values()) {
            children.add(child.newCreatedModification());
        }

        return newKeyedModification(null, build(), children);
    }

    /**
     * Create a new data object modification that represents deletion of
     * this bridge.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<T> newDeletedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        newDeletedChildModification(children);
        ItemBuildHelper.newDeletedModification(children, status);

        for (VinterfaceBuildHelper child: vInterfaces.values()) {
            children.add(child.newDeletedModification());
        }

        return newKeyedModification(build(), null, children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by merge operation.
     *
     * @param before  A {@link BridgeBuildHelper} instance that indicates
     *                the data object value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<T> newMergeModification(
        BridgeBuildHelper<T, B, K> before) {
        List<DataObjectModification<?>> children = new ArrayList<>();
        newMergeChildModification(children, before);
        ItemBuildHelper.newMergeModification(
            children, before.getStatus(), status);

        Set<VnodeName> oldKeys = before.getInterfaceKeys();
        for (Entry<VnodeName, VinterfaceBuildHelper> entry:
                 vInterfaces.entrySet()) {
            VnodeName vname = entry.getKey();
            VinterfaceBuildHelper child = entry.getValue();
            if (oldKeys.remove(vname)) {
                VinterfaceBuildHelper old = before.getInterface(vname);
                if (child.isChanged(old)) {
                    children.add(child.newMergeModification(old));
                }
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (VnodeName vname: oldKeys) {
            VinterfaceBuildHelper old = before.getInterface(vname);
            children.add(old.newDeletedModification());
        }

        return newKeyedModification(before.build(), build(), children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by put operation.
     *
     * @param before  A {@link BridgeBuildHelper} instance that indicates
     *                the data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<T> newPutModification(
        BridgeBuildHelper<T, B, K> before) {
        List<DataObjectModification<?>> children = new ArrayList<>();
        newPutChildModification(children, before);
        ItemBuildHelper.newPutModification(
            children, before.getStatus(), status);

        Set<VnodeName> oldKeys = before.getInterfaceKeys();
        for (Entry<VnodeName, VinterfaceBuildHelper> entry:
                 vInterfaces.entrySet()) {
            VnodeName vname = entry.getKey();
            VinterfaceBuildHelper child = entry.getValue();
            if (oldKeys.remove(vname)) {
                VinterfaceBuildHelper old = before.getInterface(vname);
                children.add(child.newPutModification(old));
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (VnodeName vname: oldKeys) {
            VinterfaceBuildHelper old = before.getInterface(vname);
            children.add(old.newDeletedModification());
        }

        T value = build();
        return newKeyedModification(targetType, ModificationType.WRITE,
                                    value.getKey(), before.build(), value,
                                    children);
    }

    /**
     * Collect data change events to be notified.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      {@link VtnUpdateType#CREATED} indicates that creation
     *                   events should be collected.
     *                   {@link VtnUpdateType#REMOVED} indicates that removal
     *                   events should be collected.
     */
    public final void collectEvents(MultiEventCollector collector,
                                    InstanceIdentifier<T> path,
                                    VtnUpdateType utype) {
        boolean children = collectChildrenEventsFirst(collector, path, utype);
        collector.add(path, build(), utype);
        if (children) {
            collectChildrenEvents(collector, path, utype);
        }
    }

    /**
     * Collect data change events to be notified.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param before     A {@link BridgeBuildHelper} instance that indicates
     *                   the data object value before modification.
     */
    public final void collectEvents(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        BridgeBuildHelper<T, B, K> before) {
        VtnUpdateType utype = collector.getUpdateType(path);
        if (utype != null) {
            boolean children = collectChildrenEventsFirst(
                collector, path, utype, before);
            collector.add(path, before, build(), utype);
            if (children) {
                collectChildrenEvents(collector, path, utype, before);
            }
        }
    }

    /**
     * Collect data change events for children before this instance.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      {@link VtnUpdateType#CREATED} indicates that creation
     *                   events should be collected.
     *                   {@link VtnUpdateType#REMOVED} indicates that removal
     *                   events should be collected.
     * @return  {@code true} if modifications of children need to be collected
     *          after this object.
     *          {@code false} if modification of children need to be ignored.
     */
    private boolean collectChildrenEventsFirst(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        VtnUpdateType utype) {
        boolean depth;
        if (collector.isLeafNode(targetType)) {
            depth = false;
        } else {
            depth = collector.isDepth(utype);
            if (!depth) {
                collectChildrenEvents(collector, path, utype);
            }
        }

        return depth;
    }

    /**
     * Collect data change events for children before this instance.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      A {@link VtnUpdateType} instance that specifies the
     *                   type of modification.
     * @param before     A {@link BridgeBuildHelper} instance that indicates
     *                   the data object value before modification.
     * @return  {@code true} if modifications of children need to be collected
     *          after this object.
     *          {@code false} if modification of children need to be ignored.
     */
    private boolean collectChildrenEventsFirst(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        VtnUpdateType utype, BridgeBuildHelper<T, B, K> before) {
        boolean depth;
        if (collector.isLeafNode(targetType)) {
            depth = false;
        } else {
            depth = collector.isDepth(utype);
            if (!depth) {
                collectChildrenEvents(collector, path, utype, before);
            }
        }

        return depth;
    }

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      {@link VtnUpdateType#CREATED} indicates that creation
     *                   events should be collected.
     *                   {@link VtnUpdateType#REMOVED} indicates that removal
     *                   events should be collected.
     */
    private void collectChildrenEvents(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        VtnUpdateType utype) {
        collectChildrenEventsImpl(collector, path, utype);
        if (status != null) {
            InstanceIdentifier<BridgeStatus> bspath = path.
                child(BridgeStatus.class);
            status.collectEvents(collector, bspath, utype);
        }

        for (Entry<VnodeName, VinterfaceBuildHelper> entry:
                 vInterfaces.entrySet()) {
            InstanceIdentifier<Vinterface> ipath = path.
                child(Vinterface.class, new VinterfaceKey(entry.getKey()));
            VinterfaceBuildHelper child = entry.getValue();
            child.collectEvents(collector, ipath, utype);
        }
    }

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      A {@link VtnUpdateType} instance that specifies the
     *                   type of modification.
     * @param before     A {@link BridgeBuildHelper} instance that indicates
     *                   the data object value before modification.
     */
    private void collectChildrenEvents(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        VtnUpdateType utype, BridgeBuildHelper<T, B, K> before) {
        if (utype == VtnUpdateType.CHANGED) {
            collectChildrenEventsImpl(collector, path, before);
            InstanceIdentifier<BridgeStatus> bspath = path.
                child(BridgeStatus.class);
            ItemBuildHelper.collectEvents(
                collector, bspath, before.getStatus(), status);

            Set<VnodeName> oldKeys = before.getInterfaceKeys();
            for (Entry<VnodeName, VinterfaceBuildHelper> entry:
                     vInterfaces.entrySet()) {
                VnodeName vname = entry.getKey();
                oldKeys.remove(vname);
                InstanceIdentifier<Vinterface> ipath = path.
                    child(Vinterface.class, new VinterfaceKey(vname));
                VinterfaceBuildHelper child = entry.getValue();
                VinterfaceBuildHelper old = before.getInterface(vname);
                child.collectEvents(collector, ipath, old);
            }
            for (VnodeName vname: oldKeys) {
                InstanceIdentifier<Vinterface> ipath = path.
                    child(Vinterface.class, new VinterfaceKey(vname));
                VinterfaceBuildHelper old = before.getInterface(vname);
                old.collectEvents(collector, ipath, VtnUpdateType.REMOVED);
            }
        } else {
            collectChildrenEvents(collector, path, utype);
        }
    }

    /**
     * Set data object modifications for created children into the specified
     * list.
     *
     * @param children  A list to store data object modifications.
     */
    protected abstract void newCreatedChildModification(
        List<DataObjectModification<?>> children);

    /**
     * Set data object modifications for deleted children into the specified
     * list.
     *
     * @param children  A list to store data object modifications.
     */
    protected abstract void newDeletedChildModification(
        List<DataObjectModification<?>> children);

    /**
     * Set data object modificatios for children updated by merge operation.
     *
     * @param children  A list to store data object modifications.
     * @param before    A {@link BridgeBuildHelper} instance that indicates
     *                  the data object before modification.
     */
    protected abstract void newMergeChildModification(
        List<DataObjectModification<?>> children,
        BridgeBuildHelper<T, B, K> before);

    /**
     * Set data object modificatios for children updated by put operation.
     *
     * @param children  A list to store data object modifications.
     * @param before    A {@link BridgeBuildHelper} instance that indicates
     *                  the data object before modification.
     */
    protected abstract void newPutChildModification(
        List<DataObjectModification<?>> children,
        BridgeBuildHelper<T, B, K> before);

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      {@link VtnUpdateType#CREATED} indicates that creation
     *                   events should be collected.
     *                   {@link VtnUpdateType#REMOVED} indicates that removal
     *                   events should be collected.
     */
    protected abstract void collectChildrenEventsImpl(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        VtnUpdateType utype);

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param bf         A {@link BridgeBuildHelper} instance that indicates
     *                   the data object value before modification.
     */
    protected abstract void collectChildrenEventsImpl(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        BridgeBuildHelper<T, B, K> bf);

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        BridgeStatusBuildHelper stb = status;
        if (stb != null) {
            stb.freeze();
        }

        for (VinterfaceBuildHelper child: vInterfaces.values()) {
            child.freeze();
        }
    }
}
