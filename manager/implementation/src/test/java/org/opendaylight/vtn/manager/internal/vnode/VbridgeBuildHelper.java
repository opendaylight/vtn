/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.manager.internal.ItemBuildHelper;
import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;

/**
 * Helper class for building vBridge.
 */
public final class VbridgeBuildHelper
    extends BridgeBuildHelper<Vbridge, VbridgeBuilder, VbridgeKey> {
    /**
     * The vBridge configuration.
     */
    private VbridgeConfigBuildHelper  config;

    /**
     * Construct a new instance.
     *
     * @param name  The name of the vbridge.
     */
    public VbridgeBuildHelper(String name) {
        this(new VnodeName(name));
    }

    /**
     * Construct a new instance.
     *
     * @param vname  The name of the vbridge.
     */
    public VbridgeBuildHelper(VnodeName vname) {
        super(Vbridge.class, new VbridgeBuilder().setName(vname));
    }

    /**
     * Set the vbridge configuration.
     *
     * @param desc  A brief description about the vbridge.
     * @param age   The interval beteen MAC address aging.
     * @return  This instance.
     */
    public VbridgeBuildHelper setConfig(String desc, Integer age) {
        VbridgeConfigBuildHelper cfb = config;
        if (cfb == null) {
            cfb = new VbridgeConfigBuildHelper();
            cfb.getBuilder().setDescription(desc).setAgeInterval(age);
            config = cfb;
        }

        return this;
    }

    /**
     * Return the vBridge configuration.
     *
     * @return  A {@link VbridgeConfigBuildHelper} instance or {@code null}.
     */
    public VbridgeConfigBuildHelper getConfig() {
        return config;
    }

    // BridgeBuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void newCreatedChildModification(
        List<DataObjectModification<?>> children) {
        ItemBuildHelper.newCreatedModification(children, config);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void newDeletedChildModification(
        List<DataObjectModification<?>> children) {
        ItemBuildHelper.newDeletedModification(children, config);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void newMergeChildModification(
        List<DataObjectModification<?>> children,
        BridgeBuildHelper<Vbridge, VbridgeBuilder, VbridgeKey> before) {
        assertTrue(before instanceof VbridgeBuildHelper);
        VbridgeBuildHelper bf = (VbridgeBuildHelper)before;
        ItemBuildHelper.newMergeModification(children, bf.getConfig(), config);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void newPutChildModification(
        List<DataObjectModification<?>> children,
        BridgeBuildHelper<Vbridge, VbridgeBuilder, VbridgeKey> before) {
        assertTrue(before instanceof VbridgeBuildHelper);
        VbridgeBuildHelper bf = (VbridgeBuildHelper)before;
        ItemBuildHelper.newPutModification(children, bf.getConfig(), config);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void collectChildrenEventsImpl(
        MultiEventCollector collector, InstanceIdentifier<Vbridge> path,
        VtnUpdateType utype) {
        if (config != null) {
            InstanceIdentifier<VbridgeConfig> cpath =
                path.child(VbridgeConfig.class);
            config.collectEvents(collector, cpath, utype);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void collectChildrenEventsImpl(
        MultiEventCollector collector, InstanceIdentifier<Vbridge> path,
        BridgeBuildHelper<Vbridge, VbridgeBuilder, VbridgeKey> bf) {
        assertTrue(bf instanceof VbridgeBuildHelper);
        VbridgeBuildHelper before = (VbridgeBuildHelper)bf;
        InstanceIdentifier<VbridgeConfig> cpath =
            path.child(VbridgeConfig.class);
        ItemBuildHelper.collectEvents(
            collector, cpath, before.getConfig(), config);
    }

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        VbridgeConfigBuildHelper cfb = config;
        if (cfb != null) {
            cfb.freeze();
        }

        super.freezeChildren();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(VbridgeBuilder builder) {
        VbridgeConfigBuildHelper cfb = config;
        if (cfb != null) {
            builder.setVbridgeConfig(cfb.build());
        }
        BridgeStatusBuildHelper stb = getStatus();
        if (stb != null) {
            builder.setBridgeStatus(stb.build());
        }

        List<Vinterface> list = new ArrayList<>();
        for (VinterfaceBuildHelper child: getInterfaces()) {
            list.add(child.build());
        }

        if (!list.isEmpty()) {
            builder.setVinterface(list);
        }
    }
}
