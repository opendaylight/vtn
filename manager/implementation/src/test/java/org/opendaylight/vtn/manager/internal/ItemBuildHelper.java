/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;

import org.opendaylight.vtn.manager.internal.util.MultiEventCollector;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for builder class that instantiate unkeyed YANG binding.
 *
 * @param <T>  The type of the data object.
 * @param <B>  The type of the builder class.
 */
public abstract class ItemBuildHelper
    <T extends DataObject, B extends Builder<T>> extends BuildHelper<T, B> {
    /**
     * Create a new data object modification that represents creation of
     * the data object.
     *
     * @param children  A list to store data object modification.
     * @param after     A {@link ItemBuildHelper} instance that indicates the
     *                  data object after modification.
     * @param <D>       The type of the data object.
     * @param <L>       The type of the builder class.
     */
    public static final <D extends DataObject, L extends Builder<D>> void newCreatedModification(
        List<DataObjectModification<?>> children,
        ItemBuildHelper<D, L> after) {
        if (after != null) {
            children.add(after.newCreatedModification());
        }
    }

    /**
     * Create a new data object modification that represents deletion of
     * the data object.
     *
     * @param children  A list to store data object modification.
     * @param before    A {@link ItemBuildHelper} instance that indicates the
     *                  data object before modification.
     * @param <D>       The type of the data object.
     * @param <L>       The type of the builder class.
     */
    public static final <D extends DataObject, L extends Builder<D>> void newDeletedModification(
        List<DataObjectModification<?>> children,
        ItemBuildHelper<D, L> before) {
        if (before != null) {
            children.add(before.newDeletedModification());
        }
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by merge operation.
     *
     * @param children  A list to store data object modification.
     * @param before    A {@link LeafBuildHelper} instance that indicates the
     *                  data object before modification.
     * @param after     A {@link LeafBuildHelper} instance that indicates the
     *                  data object after modification.
     * @param <D>       The type of the data object.
     * @param <L>       The type of the builder class.
     */
    public static final <D extends DataObject, L extends Builder<D>> void newMergeModification(
        List<DataObjectModification<?>> children,
        ItemBuildHelper<D, L> before, ItemBuildHelper<D, L> after) {
        if (after != null) {
            if (before == null) {
                children.add(after.newCreatedModification());
            } else if (after.isChanged(before)) {
                children.add(after.newMergeModification(before));
            }
        } else if (after != null) {
            children.add(after.newDeletedModification());
        }
    }

    /**
     * Create a new data object modification that represents changes of the
     * data object made by put operation.
     *
     * @param children  A list to store data object modification.
     * @param before    A {@link LeafBuildHelper} instance that indicates the
     *                  data object before modification.
     * @param after     A {@link LeafBuildHelper} instance that indicates the
     *                  data object after modification.
     * @param <D>       The type of the data object.
     * @param <L>       The type of the builder class.
     */
    public static final <D extends DataObject, L extends Builder<D>> void newPutModification(
        List<DataObjectModification<?>> children,
        ItemBuildHelper<D, L> before, ItemBuildHelper<D, L> after) {
        if (after != null) {
            if (before == null) {
                children.add(after.newCreatedModification());
            } else {
                children.add(after.newPutModification(before));
            }
        } else if (after != null) {
            children.add(after.newDeletedModification());
        }
    }

    /**
     * Collect data change events for children.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param before     A {@link LeafBuildHelper} instance that indicates the
     *                   data object before modification.
     * @param after      A {@link LeafBuildHelper} instance that indicates the
     *                   data object after modification.
     * @param <D>        The type of the data object.
     * @param <L>        The type of the builder class.
     */
    public static final <D extends DataObject, L extends Builder<D>> void collectEvents(
        MultiEventCollector collector, InstanceIdentifier<D> path,
        ItemBuildHelper<D, L> before, ItemBuildHelper<D, L> after) {
        if (after == null) {
            before.collectEvents(collector, path, VtnUpdateType.REMOVED);
        } else {
            after.collectEvents(collector, path, before);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param builder  The builder instance.
     */
    protected ItemBuildHelper(B builder) {
        super(builder);
    }

    /**
     * Create a new data object modification that represents creation of
     * this data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public abstract DataObjectModification<T> newCreatedModification();

    /**
     * Create a new data object modification that represents deletion of
     * this data object.
     *
     * @return  A {@link DataObjectModification} instance.
     */
    public abstract DataObjectModification<T> newDeletedModification();

    /**
     * Create a new data object modification that represents changes of this
     * data object made by merge operation.
     *
     * @param before  A {@link BuildHelper} instance that indicates the
     *                data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public abstract DataObjectModification<T> newMergeModification(
        ItemBuildHelper<T, B> before);

    /**
     * Create a new data object modification that represents changes of this
     * data object made by put operation.
     *
     * @param before  A {@link BuildHelper} instance that indicates the
     *                data object before modification.
     * @return  A {@link DataObjectModification} instance.
     */
    public abstract DataObjectModification<T> newPutModification(
        ItemBuildHelper<T, B> before);

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
    public abstract void collectEvents(MultiEventCollector collector,
                                       InstanceIdentifier<T> path,
                                       VtnUpdateType utype);

    /**
     * Collect data change events to be notified.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param path       The path to this data object.
     * @param bf         An {@link ItemBuildHelper} instance that indicates
     *                   the data object before modification.
     */
    public abstract void collectEvents(MultiEventCollector collector,
                                       InstanceIdentifier<T> path,
                                       ItemBuildHelper<T, B> bf);
}
