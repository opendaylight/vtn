/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.newTreeModification;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;

/**
 * Utility class for building vtn-inventory.
 */
public final class VtnInventoryBuilder {
    /**
     * Set {@code true} if this builder is frozen.
     */
    private boolean  frozen;

    /**
     * A set of vtn-node builders.
     */
    private final Map<SalNode, VtnNodeBuildHelper>  nodeBuilders =
        new HashMap<>();

    /**
     * Return the {@link VtnNodeBuildHelper} instance associated with the
     * specified {@link SalNode} instance.
     *
     * @param snode  A {@link SalNode} instance.
     * @return  A {@link VtnNodeBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VtnNodeBuildHelper getNode(SalNode snode) {
        return nodeBuilders.get(snode);
    }

    /**
     * Create the {@link VtnNodeBuildHelper} instance for the specified switch.
     *
     * @param snode  A {@link SalNode} instance.
     * @return  A {@link VtnNodeBuildHelper} instance.
     */
    public VtnNodeBuildHelper createNode(SalNode snode) {
        assertEquals(false, frozen);
        VtnNodeBuildHelper helper = nodeBuilders.get(snode);
        if (helper == null) {
            helper = new VtnNodeBuildHelper(snode);
            nodeBuilders.put(snode, helper);
        }

        return helper;
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
        SalNode snode = sport.getSalNode();
        VtnNodeBuildHelper helper = nodeBuilders.get(snode);
        return (helper == null) ? null : helper.getPort(sport);
    }

    /**
     * Create the {@link VtnPortBuildHelper} instance for the specified switch
     * port.
     *
     * @param sport  A {@link SalPort} instance.
     * @return  A {@link VtnPortBuildHelper} instance.
     */
    public VtnPortBuildHelper createPort(SalPort sport) {
        SalNode snode = sport.getSalNode();
        return createNode(snode).createPort(sport);
    }

    /**
     * Freeze the VTN inventory information.
     */
    public void freeze() {
        frozen = true;

        for (VtnNodeBuildHelper helper: nodeBuilders.values()) {
            helper.freeze();
        }
    }

    /**
     * Create data tree modifications that represent changes of the VTN
     * inventory information.
     *
     * @param before  A {@link VtnInventoryBuilder} instance that indicates
     *                VTN inventory information before modification.
     * @param merge   Use merge operation if {@code true}.
     *                Use put operation if {@code false}.
     * @return  A collection of {@link DataTreeModification} instances.
     */
    public Collection<DataTreeModification<VtnNode>> createEvent(
        VtnInventoryBuilder before, boolean merge) {
        List<DataTreeModification<VtnNode>> changes = new ArrayList<>();

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Set<SalNode> oldNodes = new HashSet<>(before.nodeBuilders.keySet());
        for (Entry<SalNode, VtnNodeBuildHelper> entry:
                 nodeBuilders.entrySet()) {
            SalNode snode = entry.getKey();
            VtnNodeBuildHelper helper = entry.getValue();
            DataObjectModification<VtnNode> mod;
            if (oldNodes.remove(snode)) {
                VtnNodeBuildHelper ohelper = before.nodeBuilders.get(snode);
                mod = (merge)
                    ? helper.newMergeModification(ohelper)
                    : helper.newPutModification(ohelper);
            } else {
                mod = helper.newCreatedModification();
            }

            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            changes.add(newTreeModification(path, oper, mod));
        }
        for (SalNode snode: oldNodes) {
            VtnNodeBuildHelper ohelper = before.nodeBuilders.get(snode);
            DataObjectModification<VtnNode> mod =
                ohelper.newDeletedModification();
            InstanceIdentifier<VtnNode> path = snode.getVtnNodeIdentifier();
            changes.add(newTreeModification(path, oper, mod));
        }

        return changes;
    }
}
