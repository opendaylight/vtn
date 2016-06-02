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
import java.util.List;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.BuildHelper;
import org.opendaylight.vtn.manager.internal.ItemBuildHelper;
import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;

/**
 * Helper class for building vinterface instance.
 */
public final class VinterfaceBuildHelper
    extends BuildHelper<Vinterface, VinterfaceBuilder> {
    /**
     * Builder for vinterface-config.
     */
    private VinterfaceConfigBuildHelper  config;

    /**
     * Builder for vinterface-status.
     */
    private VinterfaceStatusBuildHelper  status;

    /**
     * Construct a new instance.
     *
     * @param name  The name of the vinterface.
     */
    public VinterfaceBuildHelper(String name) {
        this(new VnodeName(name));
    }

    /**
     * Construct a new instance.
     *
     * @param vname  The name of the vinterface.
     */
    public VinterfaceBuildHelper(VnodeName vname) {
        super(new VinterfaceBuilder().setName(vname));
    }

    /**
     * Set the virtual interface configuration.
     *
     * @param desc     A brief description about the vinterface.
     * @param enabled  A boolean value that indicates whether the vinterafce
     *                 is to be enabled or not.
     * @return  This instance.
     */
    public VinterfaceBuildHelper setConfig(String desc, Boolean enabled) {
        VinterfaceConfigBuildHelper cfb = config;
        if (cfb == null) {
            cfb = new VinterfaceConfigBuildHelper();
            cfb.getBuilder().
                setDescription(desc).
                setEnabled(enabled);
            config = cfb;
        }

        return this;
    }

    /**
     * Return the virtual interface configuration.
     *
     * @return  A {@link VinterfaceConfigBuildHelper} instance or {@code null}.
     */
    public VinterfaceConfigBuildHelper getConfig() {
        return config;
    }

    /**
     * Set the vinterface status.
     *
     * @param state   The state of the vinterface.
     * @param estate  The state of the entity mapped to the vinterface.
     * @param mapped  A {@link SalPort} instance that specifies the port
     *                mapped to the vinterface.
     * @return  This instance.
     */
    public VinterfaceBuildHelper setStatus(VnodeState state, VnodeState estate,
                                           SalPort mapped) {
        VinterfaceStatusBuildHelper stb = status;
        if (stb == null) {
            stb = new VinterfaceStatusBuildHelper();

            VinterfaceStatusBuilder builder = stb.getBuilder();
            builder.setState(state).
                setEntityState(estate);
            if (mapped != null) {
                builder.setMappedPort(mapped.getNodeConnectorId());
            }
            status = stb;
        }

        return this;
    }

    /**
     * Return the vinterface status.
     *
     * @return  A {@link VinterfaceStatusBuildHelper} instance or {@code null}.
     */
    public VinterfaceStatusBuildHelper getStatus() {
        return status;
    }

    /**
     * Create a new data object modification that represents creation of
     * the data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vinterface> newCreatedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newCreatedModification(children, config);
        ItemBuildHelper.newCreatedModification(children, status);

        return newKeyedModification(null, build(), children);
    }

    /**
     * Create a new data object modification that represents deletion of
     * the data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vinterface> newDeletedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newDeletedModification(children, config);
        ItemBuildHelper.newDeletedModification(children, status);

        return newKeyedModification(build(), null, children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by merge operation.
     *
     * @param before  A {@link VinterfaceBuildHelper} instance that indicates
     *                the data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vinterface> newMergeModification(
        VinterfaceBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newMergeModification(
            children, before.getConfig(), config);
        ItemBuildHelper.newMergeModification(
            children, before.getStatus(), status);

        return newKeyedModification(before.build(), build(), children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by put operation.
     *
     * @param before  A {@link VinterfaceBuildHelper} instance that indicates
     *                the data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<Vinterface> newPutModification(
        VinterfaceBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();
        ItemBuildHelper.newPutModification(
            children, before.getConfig(), config);
        ItemBuildHelper.newPutModification(
            children, before.getStatus(), status);

        Vinterface vif = build();
        return newKeyedModification(Vinterface.class, ModificationType.WRITE,
                                    vif.getKey(), before.build(), vif,
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
                              InstanceIdentifier<Vinterface> path,
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
     * @param before     A {@link VinterfaceBuildHelper} instance that
     *                   indicates the data object before modification.
     */
    public void collectEvents(
        MultiEventCollector collector, InstanceIdentifier<Vinterface> path,
        VinterfaceBuildHelper before) {
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
        MultiEventCollector collector, InstanceIdentifier<Vinterface> path,
        VtnUpdateType utype) {
        boolean depth;
        if (collector.isLeafNode(Vinterface.class)) {
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
     * @param before     A {@link VinterfaceBuildHelper} instance that
     *                   indicates the data object before modification.
     * @return  {@code true} if modifications of children need to be collected
     *          after this object.
     *          {@code false} if modification of children need to be ignored.
     */
    private boolean collectChildrenEventsFirst(
        MultiEventCollector collector, InstanceIdentifier<Vinterface> path,
        VtnUpdateType utype, VinterfaceBuildHelper before) {
        boolean depth;
        if (collector.isLeafNode(Vinterface.class)) {
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
        MultiEventCollector collector, InstanceIdentifier<Vinterface> path,
        VtnUpdateType utype) {
        if (config != null) {
            InstanceIdentifier<VinterfaceConfig> cpath = path.
                child(VinterfaceConfig.class);
            config.collectEvents(collector, cpath, utype);
        }
        if (status != null) {
            InstanceIdentifier<VinterfaceStatus> spath = path.
                child(VinterfaceStatus.class);
            status.collectEvents(collector, spath, utype);
        }
    }

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param utype      A {@link VtnUpdateType} instance that specifies the
     *                   type of modification.
     * @param before     A {@link VinterfaceBuildHelper} instance that
     *                   indicates the data object before modification.
     */
    private void collectChildrenEvents(
        MultiEventCollector collector, InstanceIdentifier<Vinterface> path,
        VtnUpdateType utype, VinterfaceBuildHelper before) {
        if (utype == VtnUpdateType.CHANGED) {
            InstanceIdentifier<VinterfaceConfig> cpath = path.
                child(VinterfaceConfig.class);
            ItemBuildHelper.collectEvents(
                collector, cpath, before.getConfig(), config);
            InstanceIdentifier<VinterfaceStatus> spath = path.
                child(VinterfaceStatus.class);
            ItemBuildHelper.collectEvents(
                collector, spath, before.getStatus(), status);
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
        VinterfaceConfigBuildHelper cfb = config;
        if (cfb != null) {
            cfb.freeze();
        }
        VinterfaceStatusBuildHelper stb = status;
        if (stb != null) {
            stb.freeze();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(VinterfaceBuilder builder) {
        VinterfaceConfigBuildHelper cfb = config;
        if (cfb != null) {
            builder.setVinterfaceConfig(cfb.build());
        }
        VinterfaceStatusBuildHelper stb = status;
        if (stb != null) {
            builder.setVinterfaceStatus(stb.build());
        }
    }
}
