/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.assertFalse;

import java.util.Collection;
import java.util.Iterator;

/**
 * {@code LoopIterator} is used to iterate elements in the specified
 * collection forever.
 *
 * @param <E>  The type of elements in the target collection.
 */
public final class LoopIterator<E> {
    /**
     * The target collection.
     */
    private final Collection<E>  targetCollection;

    /**
     * The current iterator.
     */
    private Iterator<E>  targetIterator;

    /**
     * Construct a new instance.
     *
     * @param c  The target collection.
     */
    public LoopIterator(Collection<E> c) {
        assertFalse(c.isEmpty());
        targetCollection = c;
        targetIterator = c.iterator();
    }

    /**
     * Return the next element in the target collection.
     *
     * @return  An element in the target collection.
     */
    public E next() {
        if (!targetIterator.hasNext()) {
            targetIterator = targetCollection.iterator();
        }

        return targetIterator.next();
    }
}
