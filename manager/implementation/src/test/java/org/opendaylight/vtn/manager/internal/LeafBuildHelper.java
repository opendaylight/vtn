/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.opendaylight.vtn.manager.internal.TestBase.newItemModification;

import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for builder class that instantiate unkeyed leaf
 * YANG binding.
 *
 * @param <T>  The type of the data object.
 * @param <B>  The type of the builder class.
 */
public abstract class LeafBuildHelper
    <T extends DataObject, B extends Builder<T>>
    extends ItemBuildHelper<T, B> {
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
    protected LeafBuildHelper(Class<T> type, B builder) {
        super(builder);
        targetType = type;
    }

    // ItemBuildHelper

    /**
     * Create a new data object modification that represents creation of
     * this data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public final DataObjectModification<T> newCreatedModification() {
        return newItemModification(null, build(), null);
    }

    /**
     * Create a new data object modification that represents deletion of
     * this data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public final DataObjectModification<T> newDeletedModification() {
        return newItemModification(build(), null, null);
    }

    /**
     * Create a new data object modification that represents changes of this
     * data object made by merge operation.
     *
     * @param before  A {@link ItemBuildHelper} instance that indicates the
     *                data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public final DataObjectModification<T> newMergeModification(
        ItemBuildHelper<T, B> before) {
        return newItemModification(before.build(), build(), null);
    }

    /**
     * Create a new data object modification that represents changes of this
     * data object made by put operation.
     *
     * @param before  A {@link ItemBuildHelper} instance that indicates the
     *                data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    @Override
    public final DataObjectModification<T> newPutModification(
        ItemBuildHelper<T, B> before) {
        return newItemModification(
            targetType, ModificationType.WRITE, before.build(), build(), null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final void collectEvents(MultiEventCollector collector,
                                    InstanceIdentifier<T> path,
                                    VtnUpdateType utype) {
        collector.add(path, build(), utype);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final void collectEvents(MultiEventCollector collector,
                                    InstanceIdentifier<T> path,
                                    ItemBuildHelper<T, B> before) {
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
