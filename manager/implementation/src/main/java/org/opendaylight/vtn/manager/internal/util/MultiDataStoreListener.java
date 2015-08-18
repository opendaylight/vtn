/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Abstract base class for MD-SAL datastore change listener that listens
 * multiple data objects in the same data model tree.
 *
 * @param <T>  The type of data object for the top level data model to listen.
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
     * Sort instance identifiers notified by a data change event.
     *
     * <p>
     *   This method is used to determine order of data change event
     *   processing.
     * </p>
     *
     * @param set    A set of {@link InstanceIdentifier} instances.
     * @param type   A {@link VtnUpdateType} instance which specifies the type
     *               of event.
     * @return  A sorted list of {@link InstanceIdentifier} instances.
     */
    private List<InstanceIdentifier<?>> sortPath(
        Set<InstanceIdentifier<?>> set, VtnUpdateType type) {
        List<InstanceIdentifier<?>> list = new ArrayList<>(set.size());
        IdentifierTargetComparator idcomp = getComparator();
        boolean asc = getOrder(type);
        Comparator<InstanceIdentifier<?>> comp = (asc)
            ? idcomp : Collections.reverseOrder(idcomp);

        // Eliminate unwanted paths.
        for (InstanceIdentifier<?> path: set) {
            if (checkPath(path, type)) {
                Class<?> target = path.getTargetType();
                if (idcomp.getOrder(target) == null) {
                    unwantedIdentifier(path, type);
                } else {
                    list.add(path);
                }
            }
        }
        Collections.sort(list, comp);

        return list;
    }

    /**
     * Create an {@link IdentifiedData} that represents a data object notified
     * by a data change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param label  A label only for logging.
     * @param <D>    The type of the target data.
     * @return  An {@link IdentifiedData} on success.
     *          Otherwise {@code null}.
     */
    private <D extends DataObject> IdentifiedData<D> createIdentifiedData(
        InstanceIdentifier<D> path, DataObject value, Object label) {
        try {
            return IdentifiedData.create(path, value);
        } catch (DataTypeMismatchException e) {
            unexpectedData(path, e.getObject(), label);
        }

        return null;
    }

    /**
     * Create an {@link ChangedData} that represents a change of a data object
     * notified by a data change event.
     *
     * @param path   An instance identifier that specifies data.
     * @param value  An object associated with {@code path}.
     * @param old    An object prior to the change.
     * @param <D>    The type of the target data.
     * @return  An {@link ChangedData} on success.
     *          Otherwise {@code null}.
     */
    private <D extends DataObject> ChangedData<D> createChangedData(
        InstanceIdentifier<D> path, DataObject value, DataObject old) {
        try {
            return ChangedData.create(path, value, old);
        } catch (DataTypeMismatchException e) {
            unexpectedData(path, e.getObject(), VtnUpdateType.CHANGED);
        }

        return null;
    }

    /**
     * Return an {@link IdentifierTargetComparator} instance that determines
     * the order of event processing.
     *
     * @return  An {@link IdentifierTargetComparator} instance.
     */
    protected abstract IdentifierTargetComparator getComparator();

    /**
     * Return a boolean value which specifies the order of event processing.
     *
     * @param type  A {@link VtnUpdateType} instance which specifies the type
     *              of event.
     * @return  {@code true} means that data change events should be processed
     *          in ascending order.
     *          {@code false} means that data change events should be processed
     *          in descending order.
     */
    protected abstract boolean getOrder(VtnUpdateType type);

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
    protected final void onCreated(
        C ectx, Map<InstanceIdentifier<?>, DataObject> created) {
        if (created == null) {
            return;
        }

        VtnUpdateType type = VtnUpdateType.CREATED;
        for (InstanceIdentifier<?> path: sortPath(created.keySet(), type)) {
            DataObject value = created.get(path);
            IdentifiedData<?> data = createIdentifiedData(path, value, type);
            if (data != null) {
                onCreated(ectx, data);
            }
        }
    }

    /**
     * Handle data update events.
     *
     * @param ectx      An event context created by this instance.
     * @param updated   A map that contains information about updated data
     *                  objects.
     * @param original  A map that contains information about data objects
     *                  before update.
     */
    @Override
    protected final void onUpdated(
        C ectx, Map<InstanceIdentifier<?>, DataObject> updated,
        Map<InstanceIdentifier<?>, DataObject> original) {
        if (updated == null) {
            return;
        }

        VtnUpdateType type = VtnUpdateType.CHANGED;
        for (InstanceIdentifier<?> path: sortPath(updated.keySet(), type)) {
            DataObject value = updated.get(path);
            DataObject org = original.get(path);
            ChangedData<?> data = createChangedData(path, value, org);
            if (data != null) {
                onUpdated(ectx, data);
            }
        }
    }

    /**
     * Handle data update events.
     *
     * @param ectx      An event context created by this instance.
     * @param removed   A set of paths for removed data objects.
     *                  objects.
     * @param original  A map that contains information about data objects
     *                  before removal.
     */
    @Override
    protected final void onRemoved(
        C ectx, Set<InstanceIdentifier<?>> removed,
        Map<InstanceIdentifier<?>, DataObject> original) {
        if (removed == null) {
            return;
        }

        VtnUpdateType type = VtnUpdateType.REMOVED;
        for (InstanceIdentifier<?> path: sortPath(removed, type)) {
            DataObject value = original.get(path);
            IdentifiedData<?> data = createIdentifiedData(path, value, type);
            if (data != null) {
                onRemoved(ectx, data);
            }
        }
    }
}
