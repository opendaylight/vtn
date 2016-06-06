/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.VERBOSE_LOG;

import javax.annotation.Nonnull;

import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for MD-SAL datastore listener.
 *
 * @param <T>  Type of data object in the datastore to listen.
 * @param <C>  Type of event context.
 */
public abstract class DataStoreListener<T extends DataObject, C>
    extends AbstractDataChangeListener<T, C> {
    /**
     * Construct a new instance.
     *
     * @param cls  A {@link Class} instance that represents the target type.
     */
    protected DataStoreListener(Class<T> cls) {
        super(cls);
    }

    /**
     * Notify the created tree node.
     *
     * @param ctx    Runtime information of the notification.
     * @param ectx   An event context created by this instance.
     * @param value  The created tree node.
     */
    private void notifyCreated(TreeChangeContext<T> ctx, C ectx,
                               @Nonnull T value) {
        VtnUpdateType utype = VtnUpdateType.CREATED;
        if (isRequiredEvent(utype)) {
            InstanceIdentifier<T> path = ctx.getRootPath();
            if (checkPath(path, utype)) {
                IdentifiedData<T> data = new IdentifiedData<>(path, value);
                onCreated(ectx, data);
            }
        }
    }

    /**
     * Notify the updated tree node.
     *
     * @param ctx     Runtime information of the notification.
     * @param ectx    An event context created by this instance.
     * @param before  The target tree node before modification.
     * @param after   The target tree node after modification.
     */
    private void notifyUpdated(TreeChangeContext<T> ctx, C ectx,
                               @Nonnull T before, @Nonnull T after) {
        VtnUpdateType utype = VtnUpdateType.CHANGED;
        if (isRequiredEvent(utype)) {
            InstanceIdentifier<T> path = ctx.getRootPath();
            if (checkPath(path, utype)) {
                if (isUpdated(before, after)) {
                    ChangedData<T> data =
                        new ChangedData<>(path, after, before);
                    onUpdated(ectx, data);
                } else if (VERBOSE_LOG.isTraceEnabled()) {
                    VERBOSE_LOG.trace(
                        "{}: Tree node is not changed: path={}, value={}",
                        getClass().getSimpleName(), ctx.getRootPath(), after);
                }
            }
        }
    }

    /**
     * Notify the removed tree node.
     *
     * @param ctx    Runtime information of the notification.
     * @param ectx   An event context created by this instance.
     * @param value  The removed tree node.
     */
    private void notifyRemoved(TreeChangeContext<T> ctx, C ectx, T value) {
        VtnUpdateType utype = VtnUpdateType.REMOVED;
        if (isRequiredEvent(utype)) {
            InstanceIdentifier<T> path = ctx.getRootPath();
            if (value == null) {
                nullValue(path, utype);
            } else if (checkPath(path, utype)) {
                IdentifiedData<T> data = new IdentifiedData<>(path, value);
                onRemoved(ectx, data);
            }
        }
    }

    /**
     * Determine whether the specified data was updated or not.
     *
     * <p>
     *   {@link #onUpdated(Object, ChangedData)} is called only if this method
     *   returns {@code true}.
     * </p>
     *
     * @param before  The target data object before modification.
     * @param after   The target data object after modification.
     * @return  {@code true} if the target data was updated.
     *          {@code false} otherwise.
     */
    protected abstract boolean isUpdated(@Nonnull T before, @Nonnull T after);

    /**
     * Invoked when a new data has been created in the datastore.
     *
     * @param ectx  An event context created by this instance.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              created data.
     */
    protected abstract void onCreated(C ectx, IdentifiedData<T> data);

    /**
     * Invoked when a data in the datastore has been updated.
     *
     * @param ectx  An event context created by this instance.
     * @param data  A {@link ChangedData} instance which contains the
     *              changed data.
     */
    protected abstract void onUpdated(C ectx, ChangedData<T> data);

    /**
     * Invoked when a data has been removed from the datastore.
     *
     * @param ectx  An event context created by this instance.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              removed data.
     */
    protected abstract void onRemoved(C ectx, IdentifiedData<T> data);

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void handleTree(TreeChangeContext<T> ctx, C ectx,
                                    DataTreeModification<T> change) {
        DataObjectModification<T> mod = change.getRootNode();
        ModificationType modType = mod.getModificationType();
        T before = mod.getDataBefore();
        if (modType == ModificationType.DELETE) {
            // The target data has been removed.
            notifyRemoved(ctx, ectx, before);
        } else {
            T after = mod.getDataAfter();
            if (after == null) {
                // This should never happen.
                nullValue(ctx.getRootPath(), "handleTree");
            } else if (before == null) {
                // The target data has been created.
                notifyCreated(ctx, ectx, after);
            } else {
                // The target data has been updated.
                notifyUpdated(ctx, ectx, before, after);
            }
        }
    }
}
