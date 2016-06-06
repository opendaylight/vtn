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

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * Helper class for building VTN.
 */
public final class VtnBuildHelper extends BuildHelper<Vtn, VtnBuilder> {
    /**
     * The VTN configuration.
     */
    private VtenantConfigBuildHelper  config;

    /**
     * A set of vBridges.
     */
    private Map<VnodeName, VbridgeBuildHelper>  vBridges =
        new LinkedHashMap<>();

    /**
     * A set of vTerminals.
     */
    private Map<VnodeName, VterminalBuildHelper>  vTerminals =
        new LinkedHashMap<>();

    /**
     * Construct a new instance.
     *
     * @param name  The name of the VTN.
     */
    public VtnBuildHelper(String name) {
        this(new VnodeName(name));
    }

    /**
     * Construct a new instance.
     *
     * @param vname  The name of the VTN.
     */
    public VtnBuildHelper(VnodeName vname) {
        super(new VtnBuilder().setName(vname));
    }

    /**
     * Set the VTN configuration.
     *
     * @param desc  A brief description about the VTN.
     * @param idle  The idle-timeout value for flow entries.
     * @param hard  The hard-timeout value for flow entries.
     * @return  This instance.
     */
    public VtnBuildHelper setConfig(String desc, Integer idle, Integer hard) {
        VtenantConfigBuildHelper cfb = config;
        if (cfb == null) {
            cfb = new VtenantConfigBuildHelper();
            cfb.getBuilder().setDescription(desc).
                setIdleTimeout(idle).
                setHardTimeout(hard);
            config = cfb;
        }

        return this;
    }

    /**
     * Return the VTN configuration.
     *
     * @return  A {@link VtenantConfigBuildHelper} instance or {@code null}.
     */
    public VtenantConfigBuildHelper getConfig() {
        return config;
    }

    /**
     * Create the {@link VbridgeBuildHelper} instance for the specified
     * vBridge.
     *
     * @param name  The name of the vBridge.
     * @return  A {@link VbridgeBuildHelper} instance.
     */
    public VbridgeBuildHelper createBridge(String name) {
        return createBridge(new VnodeName(name));
    }

    /**
     * Create the {@link VbridgeBuildHelper} instance for the specified
     * vBridge.
     *
     * @param vname  The name of the vBridge.
     * @return  A {@link VbridgeBuildHelper} instance.
     */
    public VbridgeBuildHelper createBridge(VnodeName vname) {
        VbridgeBuildHelper bhelper = vBridges.get(vname);
        if (bhelper == null) {
            bhelper = new VbridgeBuildHelper(vname);
            vBridges.put(vname, bhelper);
        }

        return bhelper;
    }

    /**
     * Return the {@link VbridgeBuildHelper} instance associated with the
     * specified vBridge.
     *
     * @param name  The name of the vBridge.
     * @return  A {@link VbridgeBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VbridgeBuildHelper getBridge(String name) {
        return getBridge(new VnodeName(name));
    }

    /**
     * Return the {@link VbridgeBuildHelper} instance associated with the
     * specified vBridge.
     *
     * @param vname  The name of the vBridge.
     * @return  A {@link VbridgeBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VbridgeBuildHelper getBridge(VnodeName vname) {
        return vBridges.get(vname);
    }

    /**
     * Return a list of vBridges.
     *
     * @return  A list of {@link VbridgeBuildHelper} instances.
     */
    public List<VbridgeBuildHelper> getBridges() {
        return new ArrayList<>(vBridges.values());
    }

    /**
     * Return a set of vBridge names.
     *
     * @return  A set of {@link VnodeName} instances.
     */
    public Set<VnodeName> getBridgeKeys() {
        return new LinkedHashSet<>(vBridges.keySet());
    }

    /**
     * Create the {@link VterminalBuildHelper} instance for the specified
     * vTerminal.
     *
     * @param name  The name of the vTerminal.
     * @return  A {@link VterminalBuildHelper} instance.
     */
    public VterminalBuildHelper createTerminal(String name) {
        return createTerminal(new VnodeName(name));
    }

    /**
     * Create the {@link VterminalBuildHelper} instance for the specified
     * vTerminal.
     *
     * @param vname  The name of the vTerminal.
     * @return  A {@link VterminalBuildHelper} instance.
     */
    public VterminalBuildHelper createTerminal(VnodeName vname) {
        VterminalBuildHelper thelper = vTerminals.get(vname);
        if (thelper == null) {
            thelper = new VterminalBuildHelper(vname);
            vTerminals.put(vname, thelper);
        }

        return thelper;
    }

    /**
     * Return the {@link VterminalBuildHelper} instance associated with the
     * specified vTerminal.
     *
     * @param name  The name of the vTerminal.
     * @return  A {@link VterminalBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VterminalBuildHelper getTerminal(String name) {
        return getTerminal(new VnodeName(name));
    }

    /**
     * Return the {@link VterminalBuildHelper} instance associated with the
     * specified vTerminal.
     *
     * @param vname  The name of the vTerminal.
     * @return  A {@link VterminalBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VterminalBuildHelper getTerminal(VnodeName vname) {
        return vTerminals.get(vname);
    }

    /**
     * Return a list of vTerminals.
     *
     * @return  A list of {@link VterminalBuildHelper} instances.
     */
    public List<VterminalBuildHelper> getTerminals() {
        return new ArrayList<>(vTerminals.values());
    }

    /**
     * Return a set of vTerminal names.
     *
     * @return  A set of {@link VnodeName} instances.
     */
    public Set<VnodeName> getTerminalKeys() {
        return new LinkedHashSet<>(vTerminals.keySet());
    }

    /**
     * Create a new data object modification that represents creation of
     * the data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vtn> newCreatedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newCreatedModification(children, config);

        for (VbridgeBuildHelper child: vBridges.values()) {
            children.add(child.newCreatedModification());
        }
        for (VterminalBuildHelper child: vTerminals.values()) {
            children.add(child.newCreatedModification());
        }

        return newKeyedModification(null, build(), children);
    }

    /**
     * Create a new data object modification that represents deletion of
     * the data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vtn> newDeletedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newDeletedModification(children, config);

        for (VbridgeBuildHelper child: vBridges.values()) {
            children.add(child.newDeletedModification());
        }
        for (VterminalBuildHelper child: vTerminals.values()) {
            children.add(child.newDeletedModification());
        }

        return newKeyedModification(build(), null, children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by merge operation.
     *
     * @param before  A {@link VtnBuildHelper} instance that indicates the
     *                data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vtn> newMergeModification(
        VtnBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newMergeModification(
            children, before.getConfig(), config);

        Set<VnodeName> oldKeys = before.getBridgeKeys();
        for (Entry<VnodeName, VbridgeBuildHelper> entry:
                 vBridges.entrySet()) {
            VnodeName vname = entry.getKey();
            VbridgeBuildHelper child = entry.getValue();
            if (oldKeys.remove(vname)) {
                VbridgeBuildHelper old = before.getBridge(vname);
                if (child.isChanged(old)) {
                    children.add(child.newMergeModification(old));
                }
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (VnodeName vname: oldKeys) {
            VbridgeBuildHelper old = before.getBridge(vname);
            children.add(old.newDeletedModification());
        }

        oldKeys = before.getTerminalKeys();
        for (Entry<VnodeName, VterminalBuildHelper> entry:
                 vTerminals.entrySet()) {
            VnodeName vname = entry.getKey();
            VterminalBuildHelper child = entry.getValue();
            if (oldKeys.remove(vname)) {
                VterminalBuildHelper old = before.getTerminal(vname);
                if (child.isChanged(old)) {
                    children.add(child.newMergeModification(old));
                }
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (VnodeName vname: oldKeys) {
            VterminalBuildHelper old = before.getTerminal(vname);
            children.add(old.newDeletedModification());
        }

        return newKeyedModification(before.build(), build(), children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by put operation.
     *
     * @param before  A {@link VtnBuildHelper} instance that indicates
     *                the data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vtn> newPutModification(
        VtnBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newPutModification(
            children, before.getConfig(), config);

        Set<VnodeName> oldKeys = before.getBridgeKeys();
        for (Entry<VnodeName, VbridgeBuildHelper> entry:
                 vBridges.entrySet()) {
            VnodeName vname = entry.getKey();
            VbridgeBuildHelper child = entry.getValue();
            if (oldKeys.remove(vname)) {
                VbridgeBuildHelper old = before.getBridge(vname);
                children.add(child.newPutModification(old));
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (VnodeName vname: oldKeys) {
            VbridgeBuildHelper old = before.getBridge(vname);
            children.add(old.newDeletedModification());
        }

        oldKeys = before.getTerminalKeys();
        for (Entry<VnodeName, VterminalBuildHelper> entry:
                 vTerminals.entrySet()) {
            VnodeName vname = entry.getKey();
            VterminalBuildHelper child = entry.getValue();
            if (oldKeys.remove(vname)) {
                VterminalBuildHelper old = before.getTerminal(vname);
                children.add(child.newPutModification(old));
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (VnodeName vname: oldKeys) {
            VterminalBuildHelper old = before.getTerminal(vname);
            children.add(old.newDeletedModification());
        }

        Vtn vtn = build();
        return newKeyedModification(Vtn.class, ModificationType.WRITE,
                                    vtn.getKey(), before.build(), vtn,
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
    public void collectEvents(MultiEventCollector collector,
                              InstanceIdentifier<Vtn> path,
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
     * @param before     A {@link VtnBuildHelper} instance that indicates the
     *                   data object before modification.
     */
    public void collectEvents(
        MultiEventCollector collector, InstanceIdentifier<Vtn> path,
        VtnBuildHelper before) {
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
        MultiEventCollector collector, InstanceIdentifier<Vtn> path,
        VtnUpdateType utype) {
        boolean depth;
        if (collector.isLeafNode(Vtn.class)) {
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
     * @param before     A {@link VtnBuildHelper} instance that indicates the
     *                   data object before modification.
     * @return  {@code true} if modifications of children need to be collected
     *          after this object.
     *          {@code false} if modification of children need to be ignored.
     */
    private boolean collectChildrenEventsFirst(
        MultiEventCollector collector, InstanceIdentifier<Vtn> path,
        VtnUpdateType utype, VtnBuildHelper before) {
        boolean depth;
        if (collector.isLeafNode(Vtn.class)) {
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
        MultiEventCollector collector, InstanceIdentifier<Vtn> path,
        VtnUpdateType utype) {
        if (config != null) {
            InstanceIdentifier<VtenantConfig> cpath = path.
                child(VtenantConfig.class);
            config.collectEvents(collector, cpath, utype);
        }

        for (Entry<VnodeName, VbridgeBuildHelper> entry:
                 vBridges.entrySet()) {
            InstanceIdentifier<Vbridge> bpath = path.
                child(Vbridge.class, new VbridgeKey(entry.getKey()));
            VbridgeBuildHelper child = entry.getValue();
            child.collectEvents(collector, bpath, utype);
        }

        for (Entry<VnodeName, VterminalBuildHelper> entry:
                 vTerminals.entrySet()) {
            InstanceIdentifier<Vterminal> tpath = path.
                child(Vterminal.class, new VterminalKey(entry.getKey()));
            VterminalBuildHelper child = entry.getValue();
            child.collectEvents(collector, tpath, utype);
        }
    }

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      A {@link VtnUpdateType} instance that specifies the
     *                   type of modification.
     * @param before     A {@link VtnBuildHelper} instance that indicates the
     *                   data object before modification.
     */
    private void collectChildrenEvents(
        MultiEventCollector collector, InstanceIdentifier<Vtn> path,
        VtnUpdateType utype, VtnBuildHelper before) {
        if (utype == VtnUpdateType.CHANGED) {
            InstanceIdentifier<VtenantConfig> cpath = path.
                child(VtenantConfig.class);
            ItemBuildHelper.collectEvents(
                collector, cpath, before.getConfig(), config);

            Set<VnodeName> oldKeys = before.getBridgeKeys();
            for (Entry<VnodeName, VbridgeBuildHelper> entry:
                     vBridges.entrySet()) {
                VnodeName vname = entry.getKey();
                oldKeys.remove(vname);
                InstanceIdentifier<Vbridge> bpath = path.
                    child(Vbridge.class, new VbridgeKey(vname));
                VbridgeBuildHelper child = entry.getValue();
                VbridgeBuildHelper old = before.getBridge(vname);
                child.collectEvents(collector, bpath, old);
            }
            for (VnodeName vname: oldKeys) {
                InstanceIdentifier<Vbridge> bpath = path.
                    child(Vbridge.class, new VbridgeKey(vname));
                VbridgeBuildHelper old = before.getBridge(vname);
                old.collectEvents(collector, bpath, VtnUpdateType.REMOVED);
            }

            oldKeys = before.getTerminalKeys();
            for (Entry<VnodeName, VterminalBuildHelper> entry:
                     vTerminals.entrySet()) {
                VnodeName vname = entry.getKey();
                oldKeys.remove(vname);
                InstanceIdentifier<Vterminal> tpath = path.
                    child(Vterminal.class, new VterminalKey(vname));
                VterminalBuildHelper child = entry.getValue();
                VterminalBuildHelper old = before.getTerminal(vname);
                child.collectEvents(collector, tpath, old);
            }
            for (VnodeName vname: oldKeys) {
                InstanceIdentifier<Vterminal> tpath = path.
                    child(Vterminal.class, new VterminalKey(vname));
                VterminalBuildHelper old = before.getTerminal(vname);
                old.collectEvents(collector, tpath, VtnUpdateType.REMOVED);
            }
        } else {
            collectChildrenEvents(collector, path, utype);
        }
    }

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        VtenantConfigBuildHelper cfb = config;
        if (cfb != null) {
            cfb.freeze();
        }

        for (VbridgeBuildHelper child: vBridges.values()) {
            child.freeze();
        }
        for (VterminalBuildHelper child: vTerminals.values()) {
            child.freeze();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(VtnBuilder builder) {
        VtenantConfigBuildHelper cfb = config;
        if (cfb != null) {
            builder.setVtenantConfig(cfb.build());
        }

        List<Vbridge> blist = new ArrayList<>();
        for (VbridgeBuildHelper child: vBridges.values()) {
            blist.add(child.build());
        }
        if (!blist.isEmpty()) {
            builder.setVbridge(blist);
        }

        List<Vterminal> tlist = new ArrayList<>();
        for (VterminalBuildHelper child: vTerminals.values()) {
            tlist.add(child.build());
        }
        if (!tlist.isEmpty()) {
            builder.setVterminal(tlist);
        }
    }
}
