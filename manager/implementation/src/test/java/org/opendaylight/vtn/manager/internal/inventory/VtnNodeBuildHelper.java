/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.opendaylight.vtn.manager.internal.TestBase.newKeyedModification;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.BuildHelper;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * Helper class for building vtn-node instance.
 */
public final class VtnNodeBuildHelper
    extends BuildHelper<VtnNode, VtnNodeBuilder> {
    /**
     * A set of vtn-port builders in the vtn-node.
     */
    private final Map<SalPort, VtnPortBuildHelper>  portBuilders =
        new HashMap<>();

    /**
     * Construct a new instance.
     *
     * @param snode  A {@link SalNode} instance that specifies the switch.
     */
    public VtnNodeBuildHelper(SalNode snode) {
        this(snode.getNodeId());
    }

    /**
     * Construct a new instance.
     *
     * @param id  A {@link NodeId} instance that specifies the switch.
     */
    public VtnNodeBuildHelper(NodeId id) {
        super(new VtnNodeBuilder().setId(id));
    }

    /**
     * Create the {@link VtnPortBuildHelper} instance for the specified switch
     * port.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  A {@link VtnPortBuildHelper} instance.
     */
    public VtnPortBuildHelper createPort(SalPort sport) {
        checkNotFrozen();
        VtnPortBuildHelper phelper = portBuilders.get(sport);
        if (phelper == null) {
            phelper = new VtnPortBuildHelper(sport);
            portBuilders.put(sport, phelper);
        }

        return phelper;
    }

    /**
     * Return the {@link VtnPortBuildHelper} instance associated with the
     * specified {@link SalPort} instance.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  A {@link VtnPortBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VtnPortBuildHelper getPort(SalPort sport) {
        return portBuilders.get(sport);
    }

    /**
     * Return a set of port identifiers.
     *
     * @return  A set of {@link SalPort} instances.
     */
    public Set<SalPort> getPortKeys() {
        return new HashSet<SalPort>(portBuilders.keySet());
    }

    /**
     * Create a new data object modification that represents creation of
     * this vtn-node.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnNode> newCreatedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        for (VtnPortBuildHelper child: portBuilders.values()) {
            children.add(child.newCreatedModification());
        }

        return newKeyedModification(null, build(), children);
    }

    /**
     * Create a new data object modification that represents deletion of
     * this vtn-node.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnNode> newDeletedModification() {
        List<DataObjectModification<?>> children = new ArrayList<>();
        for (VtnPortBuildHelper child: portBuilders.values()) {
            children.add(child.newDeletedModification());
        }

        return newKeyedModification(build(), null, children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * vtn-node made by merge operation.
     *
     * @param before  A {@link VtnNodeBuildHelper} instance that indicates
     *                the vtn-node value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnNode> newMergeModification(
        VtnNodeBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();

        Set<SalPort> oldPorts = before.getPortKeys();
        for (Entry<SalPort, VtnPortBuildHelper> entry:
                 portBuilders.entrySet()) {
            SalPort sport = entry.getKey();
            VtnPortBuildHelper child = entry.getValue();
            if (oldPorts.remove(sport)) {
                VtnPortBuildHelper old = before.getPort(sport);
                if (child.isChanged(old)) {
                    children.add(child.newMergeModification(old));
                }
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (SalPort sport: oldPorts) {
            VtnPortBuildHelper old = before.getPort(sport);
            children.add(old.newDeletedModification());
        }

        return newKeyedModification(before.build(), build(), children);
    }

    /**
     * Create a new data object modification that represents changes of the
     * vtn-node made by put operation.
     *
     * @param before  A {@link VtnNodeBuildHelper} instance that indicates
     *                the vtn-node value before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public DataObjectModification<VtnNode> newPutModification(
        VtnNodeBuildHelper before) {
        List<DataObjectModification<?>> children = new ArrayList<>();

        Set<SalPort> oldPorts = before.getPortKeys();
        for (Entry<SalPort, VtnPortBuildHelper> entry:
                 portBuilders.entrySet()) {
            SalPort sport = entry.getKey();
            VtnPortBuildHelper child = entry.getValue();
            if (oldPorts.remove(sport)) {
                VtnPortBuildHelper old = before.getPort(sport);
                children.add(child.newPutModification(old));
            } else {
                children.add(child.newCreatedModification());
            }
        }
        for (SalPort sport: oldPorts) {
            VtnPortBuildHelper old = before.getPort(sport);
            children.add(old.newDeletedModification());
        }

        VtnNode vnode = build();
        return newKeyedModification(VtnNode.class, ModificationType.WRITE,
                                    vnode.getKey(), before.build(), vnode,
                                    children);
    }

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    protected void freezeChildren() {
        for (VtnPortBuildHelper child: portBuilders.values()) {
            child.freeze();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void prepare(VtnNodeBuilder builder) {
        List<VtnPort> ports = new ArrayList<>(portBuilders.size());
        for (VtnPortBuildHelper child: portBuilders.values()) {
            ports.add(child.build());
        }
        if (!ports.isEmpty()) {
            builder.setVtnPort(ports);
        }
    }
}
