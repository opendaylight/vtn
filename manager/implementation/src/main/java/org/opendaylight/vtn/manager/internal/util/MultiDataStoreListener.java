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
 * Abstract base class for MD-SAL datastore change listener that listens
 * multiple data objects in the same data model tree.
 *
 * @param <T>  The type of the root node to listen.
 * @param <C>  The type of event context.
 */
public abstract class MultiDataStoreListener<T extends DataObject, C>
    extends AbstractDataChangeListener<T, C> {
    /**
     * Construct a new instance.
     *
     * @param cls  A {@link Class} instance that represents the target type.
     */
    protected MultiDataStoreListener(Class<T> cls) {
        super(cls);
    }

    /**
     * Determine whether the specified type of the tree node should be
     * treated as a leaf node.
     *
     * <p>
     *   If this method returns {@code true}, modifications for children of
     *   the specified data type are simply ignored.
     * </p>
     * <p>
     *   By default this method always returns {@code false}.
     *   The subclass may want to override this behavior.
     * </p>
     *
     * @param type  A class that specifies the type of the tree node.
     *              Note that this value may not be the target data type
     *              specified by {@link #isRequiredType(Class)}.
     * @return  {@code true} if the specified type of the tree node should
     *          be treated as a leaf node. {@code false} otherwise.
     */
    protected boolean isLeafNode(@Nonnull Class<?> type) {
        return false;
    }

    /**
     * Create an {@link IdentifiedData} that represents a data object notified
     * by a data change event.
     *
     * @param type   The type of the data object.
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param label  A label only for logging.
     * @param <D>    The type of the target data.
     * @return  An {@link IdentifiedData} on success.
     *          Otherwise {@code null}.
     */
    private <D extends DataObject> IdentifiedData<D> createIdentifiedData(
        Class<D> type, InstanceIdentifier<?> path, D value, Object label) {
        IdentifiedData<D> data;
        InstanceIdentifier<D> dpath = cast(type, path, label);
        if (dpath == null) {
            data = null;
        } else if (value == null) {
            nullValue(path, label);
            data = null;
        } else {
            data = new IdentifiedData<>(dpath, value);
        }

        return data;
    }

    /**
     * Create an {@link ChangedData} that represents a change of a data object
     * notified by a data change event.
     *
     * @param type   The type of the data object.
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param old    An object prior to the change.
     * @param <D>    The type of the target data.
     * @return  An {@link ChangedData} on success.
     *          Otherwise {@code null}.
     */
    private <D extends DataObject> ChangedData<D> createChangedData(
        Class<D> type, InstanceIdentifier<?> path, D value, D old) {
        VtnUpdateType utype = VtnUpdateType.CHANGED;
        ChangedData<D> data;
        InstanceIdentifier<D> dpath = cast(type, path, utype);
        if (dpath == null) {
            data = null;
        } else if (value == null || old == null) {
            nullValue(path, utype);
            data = null;
        } else {
            data = new ChangedData<>(dpath, value, old);
        }

        return data;
    }

    /**
     * Notify the created tree node.
     *
     * @param ctx    Runtime information of the notification.
     * @param ectx   An event context created by this instance.
     * @param mod    The data object modification associated with the tree
     *               node.
     * @param value  The created tree node.
     * @param <D>    The type of the tree node.
     */
    private <D extends DataObject> void notifyCreated(
        TreeChangeContext<T> ctx, C ectx, DataObjectModification<D> mod,
        @Nonnull D value) {
        VtnUpdateType utype = VtnUpdateType.CREATED;
        Class<D> dtype = mod.getDataType();
        boolean children = handleChildrenFirst(ctx, ectx, mod, dtype, utype);

        if (isRequiredEvent(utype) && isRequiredType(dtype)) {
            IdentifiedData<D> data = createIdentifiedData(
                dtype, ctx.getPath(), value, utype);
            if (data != null) {
                onCreated(ectx, data);
            }
        }

        if (children) {
            // Children need to be handled after the current node.
            handleChildren(ctx, ectx, mod);
        }
    }

    /**
     * Notify the updated tree node.
     *
     * @param ctx     Runtime information of the notification.
     * @param ectx    An event context created by this instance.
     * @param mod     The data object modification associated with the tree
     *                node.
     * @param before  The target tree node before modification.
     * @param after   The target tree node after modification.
     * @param <D>    The type of the tree node.
     */
    private <D extends DataObject> void notifyUpdated(
        TreeChangeContext<T> ctx, C ectx, DataObjectModification<D> mod,
        @Nonnull D before, @Nonnull D after) {
        VtnUpdateType utype = VtnUpdateType.CHANGED;
        Class<D> dtype = mod.getDataType();
        boolean children = handleChildrenFirst(ctx, ectx, mod, dtype, utype);

        if (isRequiredEvent(utype) && isRequiredType(dtype)) {
            ChangedData<D> data = createChangedData(
                dtype, ctx.getPath(), after, before);
            if (data != null) {
                if (isUpdated(ectx, data)) {
                    onUpdated(ectx, data);
                } else if (VERBOSE_LOG.isTraceEnabled()) {
                    VERBOSE_LOG.trace(
                        "{}: Tree node is not changed: path={}, value={}",
                        getClass().getSimpleName(), ctx.getPath(), after);
                }
            }
        }

        if (children) {
            // Children need to be handled after the current node.
            handleChildren(ctx, ectx, mod);
        }
    }

    /**
     * Notify the removed tree node.
     *
     * @param ctx    Runtime information of the notification.
     * @param ectx   An event context created by this instance.
     * @param mod    The data object modification associated with the removed
     *               tree nod.
     * @param value  The removed tree node.
     * @param <D>    The type of the tree node.
     */
    private <D extends DataObject> void notifyRemoved(
        TreeChangeContext<T> ctx, C ectx, DataObjectModification<D> mod,
        D value) {
        VtnUpdateType utype = VtnUpdateType.REMOVED;
        Class<D> dtype = mod.getDataType();
        boolean children = handleChildrenFirst(ctx, ectx, mod, dtype, utype);

        if (isRequiredEvent(utype) && isRequiredType(dtype)) {
            IdentifiedData<D> data = createIdentifiedData(
                dtype, ctx.getPath(), value, utype);
            if (data != null) {
                onRemoved(ectx, data);
            }
        }

        if (children) {
            // Children need to be handled after the current node.
            handleChildren(ctx, ectx, mod);
        }
    }

    /**
     * Handle modification event for the current tree node and its direct and
     * indirect children.
     *
     * @param ctx   Runtime information of the notification.
     * @param ectx  An event context created by this instance.
     * @param mod   The data object modification associated with the current
     *              tree node.
     * @param <D>   The type of the current tree node.
     */
    private <D extends DataObject> void handleTreeNode(
        TreeChangeContext<T> ctx, C ectx, DataObjectModification<D> mod) {
        ModificationType modType = mod.getModificationType();
        D before = mod.getDataBefore();
        if (modType == ModificationType.DELETE) {
            // The target data has been removed.
            notifyRemoved(ctx, ectx, mod, before);
        } else {
            D after = mod.getDataAfter();
            if (after == null) {
                // This should never happen.
                nullValue(ctx.getPath(), "handleTreeNode");
            } else if (before == null) {
                // The target data has been created.
                notifyCreated(ctx, ectx, mod, after);
            } else {
                // The target data has been updated.
                notifyUpdated(ctx, ectx, mod, before, after);
            }
        }
    }

    /**
     * Handle modification events for direct children of the current tree node
     * before the current node.
     *
     * <p>
     *   This method must be called before the modification of the current
     *   tree node is notified.
     * </p>
     *
     * @param ctx     Runtime information of the notification.
     * @param ectx    An event context created by this instance.
     * @param mod     The data object modification associated with the current
     *                tree node.
     * @param dtype   A class that specifies the type of the current tree node.
     * @param utype   A {@link VtnUpdateType} instance that specifies the type
     *                of notification.
     * @param <D>     The type of the current tree node.
     * @return  {@code true} if modifications of children need to be notified
     *          after notification of the current tree node.
     *          {@code false} if modifications of children need to be ignored.
     */
    private <D extends DataObject> boolean handleChildrenFirst(
        TreeChangeContext<T> ctx, C ectx, DataObjectModification<D> mod,
        Class<D> dtype, VtnUpdateType utype) {
        boolean depth;
        if (isLeafNode(dtype)) {
            // Modifications for children should be ignored completely
            // if the current tree node is a leaf node.
            depth = false;
        } else {
            depth = isDepth(utype);
            if (!depth) {
                // Children need to be handled before the current tree node.
                handleChildren(ctx, ectx, mod);
            }
        }

        return depth;
    }

    /**
     * Handle modification events for direct children of the current tree node.
     *
     * @param ctx     Runtime information of the notification.
     * @param ectx    An event context created by this instance.
     * @param mod     The data object modification associated with the current
     *                tree node.
     */
    private void handleChildren(TreeChangeContext<T> ctx, C ectx,
                                DataObjectModification<?> mod) {
        for (DataObjectModification<?> child: mod.getModifiedChildren()) {
            ctx.push(child);
            handleTreeNode(ctx, ectx, child);
            ctx.pop();
        }
    }

    /**
     * Cast the given instance identifier for the given target type.
     *
     * @param type   A class which indicates the target type.
     * @param path   An instance identifier that specifies data.
     * @param label  A label only for logging.
     * @param <D>    The expected type of the target object.
     * @return  An {@link InstanceIdentifier} instance on sucecss.
     *          {@code null} on failure.
     */
    private <D extends DataObject> InstanceIdentifier<D> cast(
        Class<D> type, @Nonnull InstanceIdentifier<?> path, Object label) {
        InstanceIdentifier<D> ret;
        if (checkPath(path, label)) {
            ret = DataStoreUtils.cast(type, path);
            if (ret == null) {
                getLogger().warn("{}: Unexpected instance identifier type: " +
                                 "path={}, expected={}", label, path, type);
            }
        } else {
            ret = null;
        }

        return ret;
    }

    /**
     * Return a boolean value which specifies the order of tree traversal.
     *
     * @param type  A {@link VtnUpdateType} instance which specifies the type
     *              of event.
     * @return  {@code true} means that each tree node's children need to be
     *          processed after the tree node itself (from outer to innter).
     *          {@code false} means that each tree node's children need to be
     *          processed before the tree node itself (from inner to outer).
     */
    protected abstract boolean isDepth(@Nonnull VtnUpdateType type);

    /**
     * Determine whether the specified type of the tree node is required
     * to be notified or not.
     *
     * @param type  A class that specifies the type of the tree node.
     * @return  {@code true} if the specified type of the tree node needs
     *          to be notified. {@code false} otherwise.
     */
    protected abstract boolean isRequiredType(@Nonnull Class<?> type);

    /**
     * Determine whether the specified data was updated or not.
     *
     * <p>
     *   {@link #onUpdated(Object, ChangedData)} is called only if this method
     *   returns {@code true}.
     *   This method is called only if {@link #isRequiredType(Class)} with
     *   specifying {@code type} returns {@code true}.
     * </p>
     *
     * @param ectx  An event context created by this instance.
     * @param data  A {@link ChangedData} instance which contains the
     *              changed data.
     * @return  {@code true} if the target data was updated.
     *          {@code false} otherwise.
     */
    protected abstract boolean isUpdated(C ectx, ChangedData<?> data);

    /**
     * Invoked when a new data has been created in the datastore.
     *
     * @param ectx  An event context created by this instance.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              created data.
     */
    protected abstract void onCreated(C ectx, IdentifiedData<?> data);

    /**
     * Invoked when a data in the datastore has been updated.
     *
     * @param ectx  An event context created by this instance.
     * @param data  A {@link ChangedData} instance which contains the
     *              changed data.
     */
    protected abstract void onUpdated(C ectx, ChangedData<?> data);

    /**
     * Invoked when a data has been removed from the datastore.
     *
     * @param ectx  An event context created by this instance.
     * @param data  An {@link IdentifiedData} instance which contains the
     *              removed data.
     */
    protected abstract void onRemoved(C ectx, IdentifiedData<?> data);

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void handleTree(TreeChangeContext<T> ctx, C ectx,
                                    DataTreeModification<T> change) {
        handleTreeNode(ctx, ectx, change.getRootNode());
    }
}
