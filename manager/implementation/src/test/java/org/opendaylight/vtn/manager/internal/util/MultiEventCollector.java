/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.Deque;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import javax.annotation.Nonnull;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * A class used to collect data objects to be passed to
 * {@link MultiDataStoreListener}.
 */
public final class MultiEventCollector {
    /**
     * A list of events to be notified.
     */
    private final List<NotifiedEvent<?>>  events = new ArrayList<>();

    /**
     * A map that keeps paths to updated data objects.
     */
    private final Map<InstanceIdentifier<?>, VtnUpdateType>  updatedPaths =
        new HashMap<>();

    /**
     * An event attribute that determines behavior of
     * {@link MultiDataStoreListener}.
     */
    private final MultiEventAttr  attributes;

    /**
     * Construct a new instance with default attribute.
     */
    public MultiEventCollector() {
        this(new MultiEventAttr());
    }

    /**
     * Construct a new instance.
     *
     * @param attr  A {@link MultiEventAttr} instance.
     */
    public MultiEventCollector(MultiEventAttr attr) {
        attributes = attr;
    }

    /**
     * Return a list of events to be notified.
     *
     * @return  A list of {@link NotifiedEvent} instances.
     */
    public List<NotifiedEvent<?>> getEvents() {
        return events;
    }

    /**
     * Add the path to the updated data object.
     *
     * @param path   The path to the updated data object.
     * @param utype  A {@link VtnUpdateType} instance that indicates the type of
     *               update.
     * @return  This instance.
     */
    public MultiEventCollector addUpdated(InstanceIdentifier<?> path,
                                          VtnUpdateType utype) {
        assertEquals(null, updatedPaths.put(path, utype));

        // Mark parents as CHANGED.
        Deque<PathArgument> args = new LinkedList<>();
        for (PathArgument arg: path.getPathArguments()) {
            args.addLast(arg);
        }
        args.removeLast();

        while (!args.isEmpty()) {
            InstanceIdentifier<?> cpath = InstanceIdentifier.create(args);
            VtnUpdateType old = updatedPaths.put(cpath, VtnUpdateType.CHANGED);
            if (old != null) {
                assertEquals(VtnUpdateType.CHANGED, old);
            }
            args.removeLast();
        }

        return this;
    }

    /**
     * Return the update type applied to the specified data object.
     *
     * @param path  The instance identifier that specifies the data object.
     * @return
     *   <ul>
     *     <li>
     *       {@link VtnUpdateType#CREATED} if the specified data object
     *       is created.
     *     </li>
     *     <li>
     *       {@link VtnUpdateType#REMOVED} if the specified data object
     *       is removed.
     *     </li>
     *     <li>
     *       {@link VtnUpdateType#CHANGED} if the specified data object
     *       is changed.
     *     </li>
     *     <li>{@code null} if the specified data object is not updated.</li>
     *   </ul>
     */
    public VtnUpdateType getUpdateType(InstanceIdentifier<?> path) {
        return updatedPaths.get(path);
    }

    /**
     * Return a boolean value which specifies the order of tree traversal.
     *
     * @param utype  A {@link VtnUpdateType} instance which specifies the
     *               type of event.
     * @return  {@code true} means that each tree node's children need to
     *          be processed after the tree node itself (from outer to innter).
     *          {@code false} means that each tree node's children need to
     *          be processed before the tree node itself (from inner to outer).
     */
    public boolean isDepth(@Nonnull VtnUpdateType utype) {
        return attributes.isDepth(utype);
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
        return attributes.isLeafNode(type);
    }

    /**
     * Add the given data object to the event list if it meets the condition.
     *
     * @param path   The path to the data object.
     * @param data   The data object to be added.
     * @param utype  {@link VtnUpdateType#CREATED} indicates a creation event.
     *               {@link VtnUpdateType#REMOVED} indicates a removal event.
     * @param <T>    The type of the data object.
     */
    public <T extends DataObject> void add(InstanceIdentifier<T> path, T data,
                                           VtnUpdateType utype) {
        if (attributes.isRequiredEvent(utype)) {
            Class<T> type = path.getTargetType();
            if (attributes.isRequiredType(type)) {
                events.add(new NotifiedEvent<T>(path, data, null, utype));
            }
        }
    }

    /**
     * Add the given changed data object to the event list if it meets the
     * condition.
     *
     * @param path    The path to the data object.
     * @param before  The data object before modification.
     * @param after   The data object after modification.
     * @param <T>     The type of the data object.
     */
    public <T extends DataObject> void add(InstanceIdentifier<T> path,
                                           T before, T after) {
        VtnUpdateType utype = VtnUpdateType.CHANGED;
        if (attributes.isRequiredEvent(utype)) {
            Class<T> type = path.getTargetType();
            if (attributes.isRequiredType(type)) {
                events.add(new NotifiedEvent<T>(path, after, before, utype));
            }
        }
    }

    /**
     * Add the given changed data object to the event list if it meets the
     * condition.
     *
     * @param path    The path to the data object.
     * @param before  The builder that contains the data object before
     *                modification.
     * @param after   The data object after modification.
     * @param utype   A {@link VtnUpdateType} instance that specifies the type
     *                of modification.
     * @param <T>     The type of the data object.
     */
    public <T extends DataObject> void add(
        InstanceIdentifier<T> path, Builder<T> before, T after,
        VtnUpdateType utype) {
        if (utype == VtnUpdateType.CREATED) {
            // The target data object has been created.
            add(path, after, utype);
        } else {
            if (utype == VtnUpdateType.CHANGED) {
                // The target data object has been changed.
                add(path, before.build(), after);
            } else if (utype == VtnUpdateType.REMOVED) {
                // The target data object has been removed.
                add(path, before.build(), utype);
            }
        }
    }
}
