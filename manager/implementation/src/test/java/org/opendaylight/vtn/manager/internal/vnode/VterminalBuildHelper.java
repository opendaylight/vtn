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

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.vtn.manager.internal.ItemBuildHelper;
import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * Helper class for building vTerminal.
 */
public final class VterminalBuildHelper
    extends BridgeBuildHelper<Vterminal, VterminalBuilder, VterminalKey> {
    /**
     * The vTerminal configuration.
     */
    private VterminalConfigBuildHelper  config;

    /**
     * Construct a new instance.
     *
     * @param name  The name of the vTerminal.
     */
    public VterminalBuildHelper(String name) {
        this(new VnodeName(name));
    }

    /**
     * Construct a new instance.
     *
     * @param vname  The name of the vTerminal.
     */
    public VterminalBuildHelper(VnodeName vname) {
        super(Vterminal.class, new VterminalBuilder().setName(vname));
    }

    /**
     * Set the vTerminal configuration.
     *
     * @param desc  A brief description about the vTerminal.
     * @return  This instance.
     */
    public VterminalBuildHelper setConfig(String desc) {
        VterminalConfigBuildHelper cfb = config;
        if (cfb == null) {
            cfb = new VterminalConfigBuildHelper();
            cfb.getBuilder().setDescription(desc);
            config = cfb;
        }

        return this;
    }

    /**
     * Return the vTerminal configuration.
     *
     * @return  A {@link VterminalConfigBuildHelper} instance or {@code null}.
     */
    public VterminalConfigBuildHelper getConfig() {
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
        BridgeBuildHelper<Vterminal, VterminalBuilder, VterminalKey> before) {
        assertTrue(before instanceof VterminalBuildHelper);
        VterminalBuildHelper bf = (VterminalBuildHelper)before;
        ItemBuildHelper.newMergeModification(children, bf.getConfig(), config);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void newPutChildModification(
        List<DataObjectModification<?>> children,
        BridgeBuildHelper<Vterminal, VterminalBuilder, VterminalKey> before) {
        assertTrue(before instanceof VterminalBuildHelper);
        VterminalBuildHelper bf = (VterminalBuildHelper)before;
        ItemBuildHelper.newPutModification(children, bf.getConfig(), config);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void collectChildrenEventsImpl(
        MultiEventCollector collector, InstanceIdentifier<Vterminal> path,
        VtnUpdateType utype) {
        if (config != null) {
            InstanceIdentifier<VterminalConfig> cpath =
                path.child(VterminalConfig.class);
            config.collectEvents(collector, cpath, utype);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void collectChildrenEventsImpl(
        MultiEventCollector collector, InstanceIdentifier<Vterminal> path,
        BridgeBuildHelper<Vterminal, VterminalBuilder, VterminalKey> bf) {
        assertTrue(bf instanceof VterminalBuildHelper);
        VterminalBuildHelper before = (VterminalBuildHelper)bf;
        InstanceIdentifier<VterminalConfig> cpath =
            path.child(VterminalConfig.class);
        ItemBuildHelper.collectEvents(
            collector, cpath, before.getConfig(), config);
    }

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        VterminalConfigBuildHelper cfb = config;
        if (cfb != null) {
            cfb.freeze();
        }

        super.freezeChildren();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(VterminalBuilder builder) {
        VterminalConfigBuildHelper cfb = config;
        if (cfb != null) {
            builder.setVterminalConfig(cfb.build());
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
