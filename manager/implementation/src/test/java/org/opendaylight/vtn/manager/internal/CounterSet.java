/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.AbstractSet;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * This class implements {@link Set} interface, which counts the number of
 * instances.
 */
public class CounterSet<E> extends AbstractSet<E> implements Set<E> {
    /**
     * A map which keeps pairs of instance and the number of instances.
     */
    private final Map<E, Integer>  map = new HashMap<E, Integer>();

    /**
     * Construct an empty set.
     */
    public CounterSet() {
    }

    /**
     * Construct a new set containing elements in the specified collection.
     *
     * @param c  The collection whose elements are to be added into this set.
     */
    public CounterSet(Collection<? extends E> c) {
        addAll(c);
    }

    /**
     * Return an iterator over the elements in this set.
     *
     * @return  An iterator over the elements in this set.
     */
    @Override
    public Iterator<E> iterator() {
        return map.keySet().iterator();
    }

    /**
     * Return the number of elements in this set.
     *
     * @return  The number of elements in this set.
     */
    @Override
    public int size() {
        return map.size();
    }

    /**
     * Determine whether this set is empty or not.
     *
     * @return  {@code true} only if this set contains no elements.
     */
    @Override
    public boolean isEmpty() {
        return map.isEmpty();
    }

    /**
     * Determine whether this set contains the specified element or not.
     *
     * @param o  An element to be tested.
     * @return  {@code true} only if this set contains the specified element.
     */
    @Override
    public boolean contains(Object o) {
        return map.containsKey(o);
    }

    /**
     * Add the specified element into this set.
     *
     * <p>
     *   If this set contains the specified element, the counter for the
     *   specified element is incremented.
     * </p>
     *
     * @param e  An element to be added.
     * @return  {@code true} only if the specified element was actually added.
     */
    @Override
    public boolean add(E e) {
        Integer cnt = map.get(e);
        int count = (cnt == null) ? 1 : cnt.intValue() + 1;
        map.put(e, Integer.valueOf(count));

        return (cnt == null);
    }

    /**
     * Remove the specified element from this set.
     *
     * <p>
     *   If the counter for the specified element is greater than 1,
     *   this method only decrements the counter.
     * </p>
     *
     * @param o  An element to be removed.
     * @return  {@code true} only if the specified element was actually removed.
     */
    @Override
    public boolean remove(Object o) {
        Integer cnt = map.remove(o);
        if (cnt != null) {
            int count = cnt.intValue();
            if (count == 1) {
                return true;
            }
            map.put((E)o, Integer.valueOf(count - 1));
        }
        return false;
    }

    /**
     * Remove all elements in this set.
     */
    @Override
    public void clear() {
        map.clear();
    }

    /**
     * Return the counter for the specified instance.
     *
     * @param e  An element to be tested its counter.
     * @return  The counter for the specified instance.
     */
    public int getCounter(E e) {
        Integer cnt = map.get(e);
        return (cnt == null) ? 0 : cnt.intValue();
    }

    /**
     * Remove the counter for the specified element.
     *
     * @param e  An element to be removed.
     */
    public void removeCounter(E e) {
        map.remove(e);
    }
}
