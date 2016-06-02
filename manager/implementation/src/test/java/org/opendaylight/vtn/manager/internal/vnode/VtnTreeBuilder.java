/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.TestBase.newTreeModification;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Utility class for building VTN tree.
 */
public final class VtnTreeBuilder {
    /**
     * Set {@code true} if this builder is frozen.
     */
    private boolean  frozen;

    /**
     * A set of VTNs.
     */
    private final Map<VnodeName, VtnBuildHelper>  vTenants =
        new LinkedHashMap<>();

    /**
     * Return the {@link VtnBuildHelper} instance associated with the specified
     * VTN.
     *
     * @param name  The name of the VTN.
     * @return  A {@link VtnBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VtnBuildHelper getVtn(String name) {
        return getVtn(new VnodeName(name));
    }

    /**
     * Return the {@link VtnBuildHelper} instance associated with the specified
     * VTN.
     *
     * @param vname  The name of the VTN.
     * @return  A {@link VtnBuildHelper} instance if found.
     *          {@code null} if not found.
     */
    public VtnBuildHelper getVtn(VnodeName vname) {
        return vTenants.get(vname);
    }

    /**
     * Create the {@link VtnBuildHelper} instance for the specified VTN.
     *
     * @param name  The name of the VTN.
     * @return  A {@link VtnBuildHelper} instance.
     */
    public VtnBuildHelper createVtn(String name) {
        return createVtn(new VnodeName(name));
    }

    /**
     * Create the {@link VtnBuildHelper} instance for the specified VTN.
     *
     * @param vname  The name of the VTN.
     * @return  A {@link VtnBuildHelper} instance.
     */
    public VtnBuildHelper createVtn(VnodeName vname) {
        assertEquals(false, frozen);
        VtnBuildHelper vhelper = vTenants.get(vname);
        if (vhelper == null) {
            vhelper = new VtnBuildHelper(vname);
            vTenants.put(vname, vhelper);
        }

        return vhelper;
    }

    /**
     * Return a list of VTNs.
     *
     * @return  A list of {@link VtnBuildHelper} instances.
     */
    public List<VtnBuildHelper> getVtns() {
        return new ArrayList<>(vTenants.values());
    }

    /**
     * Return a set of VTN names.
     *
     * @return  A set of {@link VnodeName} instances.
     */
    public Set<VnodeName> getVtnKeys() {
        return new LinkedHashSet<>(vTenants.keySet());
    }

    /**
     * Freeze the VTN tree.
     */
    public void freeze() {
        frozen = true;

        for (VtnBuildHelper child: vTenants.values()) {
            child.freeze();
        }
    }

    /**
     * Create data tree modifications that represent changes of the VTN
     * tree.
     *
     * @param before  A {@link VtnTreeBuilder} instance that indicates
     *                VTN tree before modification.
     * @param merge   Use merge operation if {@code true}.
     *                Use put operation if {@code false}.
     * @return  A collection of {@link DataTreeModification} instances.
     */
    public Collection<DataTreeModification<Vtn>> createEvent(
        VtnTreeBuilder before, boolean merge) {
        List<DataTreeModification<Vtn>> changes = new ArrayList<>();

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Set<VnodeName> oldKeys = before.getVtnKeys();
        for (Entry<VnodeName, VtnBuildHelper> entry: vTenants.entrySet()) {
            VnodeName vname = entry.getKey();
            VtnBuildHelper child = entry.getValue();
            DataObjectModification<Vtn> mod;
            if (oldKeys.remove(vname)) {
                VtnBuildHelper old = before.getVtn(vname);
                mod = (merge)
                    ? child.newMergeModification(old)
                    : child.newPutModification(old);
            } else {
                mod = child.newCreatedModification();
            }

            Vtn vtn = child.build();
            InstanceIdentifier<Vtn> path = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, vtn.getKey()).
                build();
            changes.add(newTreeModification(path, oper, mod));
        }
        for (VnodeName vname: oldKeys) {
            VtnBuildHelper old = before.getVtn(vname);
            DataObjectModification<Vtn> mod = old.newDeletedModification();
            Vtn vtn = old.build();
            InstanceIdentifier<Vtn> path = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, vtn.getKey()).
                build();
            changes.add(newTreeModification(path, oper, mod));
        }

        return changes;
    }

    /**
     * Collect data change events to be notified.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param before     A {@link VtnTreeBuilder} instance that contains
     *                   data objects before modification.
     */
    public void collectEvents(MultiEventCollector collector,
                              VtnTreeBuilder before) {
        Set<VnodeName> oldKeys = before.getVtnKeys();
        for (Entry<VnodeName, VtnBuildHelper> entry: vTenants.entrySet()) {
            VnodeName vname = entry.getKey();
            oldKeys.remove(vname);
            InstanceIdentifier<Vtn> path = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, new VtnKey(vname)).
                build();
            VtnBuildHelper child = entry.getValue();
            VtnBuildHelper old = before.getVtn(vname);
            child.collectEvents(collector, path, old);
        }

        for (VnodeName vname: oldKeys) {
            InstanceIdentifier<Vtn> path = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, new VtnKey(vname)).
                build();
            VtnBuildHelper old = before.getVtn(vname);
            old.collectEvents(collector, path, VtnUpdateType.REMOVED);
        }
    }
}
