/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;


import static org.junit.Assert.assertTrue;
import static org.opendaylight.vtn.manager.internal.TestBase.newItemModification;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.ItemBuildHelper;
import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPathsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatusBuilder;

/**
 * Helper class for building bridge-status instance.
 */
public final class BridgeStatusBuildHelper
    extends ItemBuildHelper<BridgeStatus, BridgeStatusBuilder> {
    /**
     * A set of faulted-paths instances in the bridge-status.
     */
    private final Map<FaultedPathsKey, FaultedPathsBuildHelper>  faultedPaths =
        new LinkedHashMap<>();

    /**
     * Construct a new instance.
     */
    public BridgeStatusBuildHelper() {
        super(new BridgeStatusBuilder());
    }

    /**
     * Create the {@link FaultedPathsBuildHelper} instance for the specified
     * path.
     *
     * @param src  A {@link SalNode} instance that specifies the source node.
     * @param dst  A {@link SalNode} instance that specifies the destination
     *             node.
     * @return  A {@link FaultedPathsBuildHelper} instance.
     */
    public FaultedPathsBuildHelper createFaultedPaths(
        SalNode src, SalNode dst) {
        FaultedPathsKey key = new FaultedPathsKey(
            dst.getNodeId(), src.getNodeId());
        return createFaultedPaths(key);
    }

    /**
     * Create the {@link FaultedPathsBuildHelper} instance for the specified
     * key.
     *
     * @param key  A {@link FaultedPathsKey} instance that specifies the
     *             faulted path.
     * @return  A {@link FaultedPathsBuildHelper} instance.
     */
    public FaultedPathsBuildHelper createFaultedPaths(FaultedPathsKey key) {
        checkNotFrozen();
        FaultedPathsBuildHelper fphelper = faultedPaths.get(key);
        if (fphelper == null) {
            fphelper = new FaultedPathsBuildHelper(key);
            faultedPaths.put(key, fphelper);
        }

        return fphelper;
    }

    /**
     * Return the {@link FaultedPathsBuildHelper} instance associated with the
     * specified {@link SalNode} instances.
     *
     * @param src  A {@link SalNode} instance that specifies the source node
     *             of the path.
     * @param dst  A {@link SalNode} instance taht specifies the destination
     *             node of the path.
     * @return  A {@link FaultedPathsBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public FaultedPathsBuildHelper getFaultedPath(SalNode src, SalNode dst) {
        FaultedPathsKey key = new FaultedPathsKey(
            src.getNodeId(), dst.getNodeId());
        return faultedPaths.get(key);
    }

    /**
     * Return the {@link FaultedPathsBuildHelper} instance associated with the
     * specified {@link SalNode} instance.
     *
     * @param key  A {@link FaultedPathsKey} instance.
     * @return  A {@link FaultedPathsBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public FaultedPathsBuildHelper getFaultedPath(FaultedPathsKey key) {
        return faultedPaths.get(key);
    }

    /**
     * Return a set of faulted-paths identifiers.
     *
     * @return  A set of {@link FaultedPathsKey} instances.
     */
    public Set<FaultedPathsKey> getFaultedPathKeys() {
        return new LinkedHashSet<>(faultedPaths.keySet());
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
        MultiEventCollector collector, InstanceIdentifier<BridgeStatus> path,
        VtnUpdateType utype) {
        boolean depth;
        if (collector.isLeafNode(BridgeStatus.class)) {
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
     * @param before     A {@link BridgeStatusBuildHelper} instance that
     *                   indicates the bridge-status value before modification.
     * @return  {@code true} if modifications of children need to be collected
     *          after this object.
     *          {@code false} if modification of children need to be ignored.
     */
    private boolean collectChildrenEventsFirst(
        MultiEventCollector collector, InstanceIdentifier<BridgeStatus> path,
        VtnUpdateType utype, BridgeStatusBuildHelper before) {
        boolean depth;
        if (collector.isLeafNode(BridgeStatus.class)) {
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
        MultiEventCollector collector, InstanceIdentifier<BridgeStatus> path,
        VtnUpdateType utype) {
        for (Entry<FaultedPathsKey, FaultedPathsBuildHelper> entry:
                 faultedPaths.entrySet()) {
            InstanceIdentifier<FaultedPaths> fpath = path.
                child(FaultedPaths.class, entry.getKey());
            FaultedPathsBuildHelper child = entry.getValue();
            child.collectEvents(collector, fpath, utype);
        }
    }

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      A {@link VtnUpdateType} instance that specifies the
     *                   type of modification.
     * @param before     A {@link BridgeStatusBuildHelper} instance that
     *                   indicates the bridge-status value before modification.
     */
    private void collectChildrenEvents(
        MultiEventCollector collector, InstanceIdentifier<BridgeStatus> path,
        VtnUpdateType utype, BridgeStatusBuildHelper before) {
        if (utype == VtnUpdateType.CHANGED) {
            Set<FaultedPathsKey> oldKeys = before.getFaultedPathKeys();
            for (Entry<FaultedPathsKey, FaultedPathsBuildHelper> entry:
                     faultedPaths.entrySet()) {
                FaultedPathsKey key = entry.getKey();
                oldKeys.remove(key);
                InstanceIdentifier<FaultedPaths> fpath = path.
                    child(FaultedPaths.class, key);
                FaultedPathsBuildHelper child = entry.getValue();
                FaultedPathsBuildHelper old = before.getFaultedPath(key);
                child.collectEvents(collector, fpath, old);
            }
            for (FaultedPathsKey key: oldKeys) {
                InstanceIdentifier<FaultedPaths> fpath = path.
                    child(FaultedPaths.class, key);
                FaultedPathsBuildHelper old = before.getFaultedPath(key);
                old.collectEvents(collector, fpath, VtnUpdateType.REMOVED);
            }
        } else {
            // All the children should be CREATED or REMOVED.
            collectChildrenEvents(collector, path, utype);
        }
    }

    // ItemBuildHelper

    /**
     * Create a new data object modification that represents creation of
     * this bridge-status.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public DataObjectModification<BridgeStatus> newCreatedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        for (FaultedPathsBuildHelper fphelper: faultedPaths.values()) {
            children.add(fphelper.newCreatedModification());
        }

        return newItemModification(null, build(), children);
    }

    /**
     * Create a new data object modification that represents deletion of
     * this bridge-status.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public DataObjectModification<BridgeStatus> newDeletedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        for (FaultedPathsBuildHelper fphelper: faultedPaths.values()) {
            children.add(fphelper.newDeletedModification());
        }

        return newItemModification(build(), null, children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * bridge-status made by merge operation.
     *
     * @param bf  A {@link BridgeStatusBuildHelper} instance that indicates
     *            the bridge-status value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public DataObjectModification<BridgeStatus> newMergeModification(
        ItemBuildHelper<BridgeStatus, BridgeStatusBuilder> bf) {
        assertTrue(bf instanceof BridgeStatusBuildHelper);
        BridgeStatusBuildHelper before = (BridgeStatusBuildHelper)bf;
        List<DataObjectModification<?>> children = new ArrayList<>();

        Set<FaultedPathsKey> oldKeys = before.getFaultedPathKeys();
        for (Entry<FaultedPathsKey, FaultedPathsBuildHelper> entry:
                 faultedPaths.entrySet()) {
            FaultedPathsKey key = entry.getKey();
            FaultedPathsBuildHelper child = entry.getValue();
            if (oldKeys.remove(key)) {
                FaultedPathsBuildHelper old = before.getFaultedPath(key);
                if (child.isChanged(old)) {
                    children.add(child.newMergeModification(old));
                }
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (FaultedPathsKey key: oldKeys) {
            FaultedPathsBuildHelper old = before.getFaultedPath(key);
            children.add(old.newDeletedModification());
        }

        return newItemModification(before.build(), build(), children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * bridge-status made by put operation.
     *
     * @param bf  A {@link BridgeStatusBuildHelper} instance that indicates
     *            the bridge-status value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public DataObjectModification<BridgeStatus> newPutModification(
        ItemBuildHelper<BridgeStatus, BridgeStatusBuilder> bf) {
        assertTrue(bf instanceof BridgeStatusBuildHelper);
        BridgeStatusBuildHelper before = (BridgeStatusBuildHelper)bf;
        List<DataObjectModification<?>> children = new ArrayList<>();

        Set<FaultedPathsKey> oldKeys = before.getFaultedPathKeys();
        for (Entry<FaultedPathsKey, FaultedPathsBuildHelper> entry:
                 faultedPaths.entrySet()) {
            FaultedPathsKey key = entry.getKey();
            FaultedPathsBuildHelper child = entry.getValue();
            if (oldKeys.remove(key)) {
                FaultedPathsBuildHelper old = before.getFaultedPath(key);
                children.add(child.newPutModification(old));
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (FaultedPathsKey key: oldKeys) {
            FaultedPathsBuildHelper old = before.getFaultedPath(key);
            children.add(old.newDeletedModification());
        }

        BridgeStatus vnode = build();
        return newItemModification(BridgeStatus.class, ModificationType.WRITE,
                                   before.build(), vnode, children);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void collectEvents(MultiEventCollector collector,
                              InstanceIdentifier<BridgeStatus> path,
                              VtnUpdateType utype) {
        boolean children = collectChildrenEventsFirst(collector, path, utype);
        collector.add(path, build(), utype);
        if (children) {
            collectChildrenEvents(collector, path, utype);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void collectEvents(
        MultiEventCollector collector, InstanceIdentifier<BridgeStatus> path,
        ItemBuildHelper<BridgeStatus, BridgeStatusBuilder> bf) {
        VtnUpdateType utype = collector.getUpdateType(path);
        if (utype != null) {
            BridgeStatusBuildHelper before;
            if (bf == null) {
                before = null;
            } else {
                assertTrue(bf instanceof BridgeStatusBuildHelper);
                before = (BridgeStatusBuildHelper)bf;
            }
            boolean children = collectChildrenEventsFirst(
                collector, path, utype, before);
            collector.add(path, before, build(), utype);
            if (children) {
                collectChildrenEvents(collector, path, utype, before);
            }
        }
    }


    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        for (FaultedPathsBuildHelper child: faultedPaths.values()) {
            child.freeze();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(BridgeStatusBuilder builder) {
        List<FaultedPaths> fpaths = new ArrayList<>();
        for (FaultedPathsBuildHelper child: faultedPaths.values()) {
            fpaths.add(child.build());
        }
        if (!fpaths.isEmpty()) {
            builder.setFaultedPaths(fpaths);
        }
    }
}
