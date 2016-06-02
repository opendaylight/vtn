/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.opendaylight.vtn.manager.internal.TestBase.newKeyedModification;

import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.Identifiable;
import org.opendaylight.yangtools.yang.binding.Identifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for builder class that instantiate keyed leaf
 * YANG binding.
 *
 * @param <T>  The type of the data object.
 * @param <B>  The type of the builder class.
 * @param <K>  The type of the key.
 */
public abstract class KeyedLeafBuildHelper
    <T extends DataObject & Identifiable<K>, B extends Builder<T>,
     K extends Identifier<T>> extends BuildHelper<T, B> {
    /**
     * A class that specifies the type of the data object.
     */
    private final Class<T>  targetType;

    /**
     * Construct a new instance.
     *
     * @param type     A class that specifies the target data type.
     * @param builder  The builder instance.
     */
    protected KeyedLeafBuildHelper(Class<T> type, B builder) {
        super(builder);
        targetType = type;
    }

    /**
     * Create a new data object modification that represents creation of
     * this data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public final DataObjectModification<T> newCreatedModification() {
        return newKeyedModification(null, build(), null);
    }

    /**
     * Create a new data object modification that represents deletion of
     * this data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public final DataObjectModification<T> newDeletedModification() {
        return newKeyedModification(build(), null, null);
    }

    /**
     * Create a new data object modification that represents changes of this
     * data object made by merge operation.
     *
     * @param before  A {@link KeyedLeafBuildHelper} instance that indicates
     *                the data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public final DataObjectModification<T> newMergeModification(
        KeyedLeafBuildHelper<T, B, K> before) {
        return newKeyedModification(before.build(), build(), null);
    }

    /**
     * Create a new data object modification that represents changes of this
     * data object made by put operation.
     *
     * @param before  A {@link KeyedLeafBuildHelper} instance that indicates
     *                the data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public final DataObjectModification<T> newPutModification(
        KeyedLeafBuildHelper<T, B, K> before) {
        T value = build();
        return newKeyedModification(
            targetType, ModificationType.WRITE, value.getKey(), before.build(),
            value, null);
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
        collector.add(path, build(), utype);
    }

    /**
     * Collect data change events to be notified.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param before     A {@link KeyedLeafBuildHelper} instance that indicates
     *                   the data object before modification.
     */
    public final void collectEvents(
        MultiEventCollector collector, InstanceIdentifier<T> path,
        KeyedLeafBuildHelper<T, B, K> before) {
        VtnUpdateType utype = collector.getUpdateType(path);
        collector.add(path, before, build(), utype);
    }

    // BuildHelper

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean isChanged(BuildHelper<T, B> before) {
        T value = build();
        T old = before.build();
        return !value.equals(old);
    }

    /**
     * This method does nothing because this is a leaf node.
     */
    @Override
    protected final void freezeChildren() {
    }
}
