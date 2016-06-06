/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Collections;
import java.util.Map;
import java.util.Set;

import javax.annotation.Nonnull;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * A set of attributes that determine behavior of
 * {@link MultiDataStoreListener}.
 */
public final class MultiEventAttr {
    /**
     * A set of required event types.
     */
    private Set<VtnUpdateType>  requiredEvents;

    /**
     * A set of data object types to be handled.
     */
    private Set<Class<?>>  requiredTypes;

    /**
     * A set of event types to be handled as depth mode.
     */
    private Set<VtnUpdateType>  depthModes;

    /**
     * A set of data types to be treated as leaf nodes.
     */
    private Set<Class<?>>  leafTypes;

    /**
     * Values to be returned by
     * {@link MultiDataStoreListener#isUpdated(Object,ChangedData)}.
     */
    private Map<Class<?>, Boolean>  updatedTypes =
        Collections.<Class<?>, Boolean>emptyMap();

    /**
     * Set required event types.
     *
     * @param events  A set of required event types.
     * @return  This instance.
     */
    public MultiEventAttr setRequiredEvents(Set<VtnUpdateType> events) {
        requiredEvents = events;
        return this;
    }

    /**
     * Set required data types.
     *
     * @param types  A set of required data object types.
     * @return  This instance.
     */
    public MultiEventAttr setRequiredTypes(Set<Class<?>> types) {
        requiredTypes = types;
        return this;
    }

    /**
     * Set event types to be handled as depth mode.
     *
     * @param depth  A set of event types to be handled as depth mode.
     * @return  This instance.
     */
    public MultiEventAttr setDepthModes(Set<VtnUpdateType> depth) {
        depthModes = depth;
        return this;
    }

    /**
     * Set data types to be treated as leaf node.
     *
     * @param leaf  A set of data object types to be treated as leaf node.
     * @return  This instance.
     */
    public MultiEventAttr setLeafTypes(Set<Class<?>> leaf) {
        leafTypes = leaf;
        return this;
    }

    /**
     * Set data types to be treated as updated.
     *
     * @param updated  A map that determines whether the specified data type
     *                 is treated as updated.
     * @return  This instance.
     */
    public MultiEventAttr setUpdated(Map<Class<?>, Boolean> updated) {
        updatedTypes = updated;
        return this;
    }

    /**
     * Determine whether the specified event type should be handled or not.
     *
     * @param type  A {@link VtnUpdateType} instance which indicates the
     *              event type.
     * @return  {@code true} if the given event type should be handled.
     *          {@code false} otherwise.
     */
    public boolean isRequiredEvent(@Nonnull VtnUpdateType type) {
        return (requiredEvents == null || requiredEvents.contains(type));
    }

    /**
     * Determine whether the specified type of the tree node is required
     * to be notified or not.
     *
     * @param type  A class that specifies the type of the tree node.
     * @return  {@code true} if the specified type of the tree node needs
     *          to be notified. {@code false} otherwise.
     */
    public boolean isRequiredType(@Nonnull Class<?> type) {
        return (requiredTypes == null || requiredTypes.contains(type));
    }

    /**
     * Return a boolean value which specifies the order of tree traversal.
     *
     * @param type  A {@link VtnUpdateType} instance which specifies the
     *              type of event.
     * @return  {@code true} means that each tree node's children need to
     *          be processed after the tree node itself (from outer to innter).
     *          {@code false} means that each tree node's children need to
     *          be processed before the tree node itself (from inner to outer).
     */
    public boolean isDepth(@Nonnull VtnUpdateType type) {
        return (depthModes == null || depthModes.contains(type));
    }

    /**
     * Determine whether the specified type of the tree node should be
     * treated as a leaf node.
     *
     * @param type  A class that specifies the type of the tree node.
     * @return  {@code true} if the specified type of the tree node should
     *          be treated as a leaf node. {@code false} otherwise.
     */
    public boolean isLeafNode(@Nonnull Class<?> type) {
        return (leafTypes != null && leafTypes.contains(type));
    }

    /**
     * Determine whether the specified data was updated or not.
     *
     * @param data  A {@link ChangedData} instance that contains values before
     *              and after modification.
     * @return  {@code true} if the specified tree node needs to be treated
     *          as updated. {@code false} otherwise.
     */
    public boolean isUpdated(ChangedData<?> data) {
        boolean result;
        if (updatedTypes == null) {
            // Treat all the tree nodes as not updated.
            result = false;
        } else {
            Class<?> type = data.getIdentifier().getTargetType();
            Boolean b = updatedTypes.get(type);
            if (b == null) {
                // Check to see if the data is changed actually.
                DataObjectIdentity value =
                    new DataObjectIdentity(data.getValue());
                DataObjectIdentity old =
                    new DataObjectIdentity(data.getOldValue());
                result = !value.equals(old);
            } else {
                result = b.booleanValue();
            }
        }

        return result;
    }
}

