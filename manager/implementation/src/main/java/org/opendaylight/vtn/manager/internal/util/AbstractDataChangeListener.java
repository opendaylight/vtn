/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Collection;

import javax.annotation.Nonnull;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeService;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for MD-SAL datastore listener.
 *
 * @param <T>  The type of the target data model.
 * @param <C>  Type of event context.
 */
public abstract class AbstractDataChangeListener<T extends DataObject, C>
    extends CloseableContainer implements DataTreeChangeListener<T> {
    /**
     * The type of the target data.
     */
    private final Class<T>  targetType;

    /**
     * A clustered data tree change listener that wraps the specified data
     * tree change listener.
     *
     * @param <T>  The type of the target data model.
     */
    private static final class ClusteredListener<T extends DataObject>
        implements ClusteredDataTreeChangeListener<T> {
        /**
         * A data tree change listener that listens data change events.
         */
        private final DataTreeChangeListener<T>  theListener;

        /**
         * Construct a new instance.
         *
         * @param listener  A data change listener that listens data change
         *                  events.
         */
        private ClusteredListener(@Nonnull DataTreeChangeListener<T> listener) {
            theListener = listener;
        }

        // DataTreeChangeListener

        /**
         * Invoked when at least an entry in the datastore has been changed.
         *
         * @param changes  A collection of data tree modifications.
         */
        @Override
        public void onDataTreeChanged(
            @Nonnull Collection<DataTreeModification<T>> changes) {
            theListener.onDataTreeChanged(changes);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param cls  A {@link Class} instance that represents the target type.
     */
    protected AbstractDataChangeListener(Class<T> cls) {
        targetType = cls;
    }

    /**
     * Return the type of the target data model.
     *
     * @return  A {@link Class} instance that represents the target type.
     */
    public final Class<T> getTargetType() {
        return targetType;
    }

    /**
     * Register this instance as a data tree change listener.
     *
     * @param service    A {@link DataTreeChangeService} instance.
     * @param store      A {@link LogicalDatastoreType} instance used to
     *                   determine the target datastore.
     * @param clustered  If {@code true}, data change events will be delivered
     *                   to all the nodes in the cluster.
     *                   If {@code false}, data change events will be delivered
     *                   to only the data shard leader.
     */
    protected final void registerListener(DataTreeChangeService service,
                                          LogicalDatastoreType store,
                                          boolean clustered) {
        InstanceIdentifier<T> path = getWildcardPath();
        DataTreeIdentifier<T> ident = new DataTreeIdentifier<>(store, path);
        DataTreeChangeListener<T> listener = (clustered)
            ? new ClusteredListener<T>(this) : this;
        try {
            ListenerRegistration<DataTreeChangeListener<T>> reg = service.
                registerDataTreeChangeListener(ident, listener);
            addCloseable(reg);
        } catch (Exception e) {
            String msg = new StringBuilder(
                "Failed to register data tree change listener: type=").
                append(targetType.getName()).
                append(", path=").append(path).
                toString();
            getLogger().error(msg, e);
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Verify an instance identifier notified by a data tree change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param label  A label only for logging.
     * @return  {@code true} only if a given instance identifier  is valid.
     */
    protected final boolean checkPath(@Nonnull InstanceIdentifier<?> path,
                                      Object label) {
        boolean wild = path.isWildcarded();
        if (wild) {
            getLogger().warn("{}: Ignore wildcard path: {}", label, path);
        }

        return !wild;
    }

    /**
     * Record a log that indicates a {@code null} is notified as the value of
     * the tree node.
     *
     * @param path   An {@link InstanceIdentifier} instance.
     * @param label  A label only for logging.
     */
    protected final void nullValue(InstanceIdentifier<?> path, Object label) {
        getLogger().warn("{}: Null value is notified: path={}", label, path);
    }

    /**
     * Determine whether the specified event type should be handled or not.
     *
     * <p>
     *   Note that this method of this class always returns {@code true},
     *   which means all event types should be listened. Subclass can override
     *   this method to filter out events.
     * </p>
     *
     * @param type  A {@link VtnUpdateType} instance which indicates the event
     *              type.
     * @return  {@code true} if the given event type should be handled.
     *          {@code false} otherwise.
     */
    protected boolean isRequiredEvent(@Nonnull VtnUpdateType type) {
        return true;
    }

    /**
     * Invoked when {@link #onDataTreeChanged(Collection)} has just been
     * called.
     *
     * @return  A new event context.
     */
    protected abstract C enterEvent();

    /**
     * Invoked when {@link #onDataTreeChanged(Collection)} is going to leave.
     *
     * <p>
     *   Note that this method may be called even if no modified data was
     *   notified.
     * </p>
     *
     * @param ectx  An event context created by {@link #enterEvent()}.
     */
    protected abstract void exitEvent(C ectx);

    /**
     * Handle the given data tree modification.
     *
     * @param ctx     Runtime information of the notification.
     * @param ectx    An event context created by {@link #enterEvent()}.
     * @param change  A {@link DataTreeModification} instance.
     */
    protected abstract void handleTree(TreeChangeContext<T> ctx, C ectx,
                                       DataTreeModification<T> change);

    /**
     * Return a wildcard instance identifier that specifies data objects
     * to be listened.
     *
     * @return  A wildcard instance identifier.
     */
    protected abstract InstanceIdentifier<T> getWildcardPath();

    // DataTreeChangeListener

    /**
     * Invoked when the specified data tree has been modified.
     *
     * @param changes  A collection of data tree modifications.
     */
    @Override
    public final void onDataTreeChanged(
        @Nonnull Collection<DataTreeModification<T>> changes) {
        TreeChangeContext<T> ctx = new TreeChangeContext<>();
        C ectx = enterEvent();
        try {
            for (DataTreeModification<T> change: changes) {
                ctx.setRootPath(change.getRootPath().getRootIdentifier());
                handleTree(ctx, ectx, change);
            }
        } catch (RuntimeException e) {
            getLogger().error(
                "Unexpected exception in data tree change event listener.", e);
        } finally {
            exitEvent(ectx);
        }
    }
}
