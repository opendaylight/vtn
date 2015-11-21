/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

/**
 * {@code CacheMap} describes a lazy cache for data indexed by a key.
 *
 * <p>
 *   This class is used to cache YANG data objects in a keyed list.
 *   A cache for list element is instantiated on the first access to it.
 * </p>
 * <ul>
 *   <li>
 *     Note that this class is not synchronized.
 *   </li>
 *   <li>
 *     The size of the list cannot be changed.
 *   </li>
 * </ul>
 *
 * @param <T>  The type of the raw data.
 * @param <K>  The type of key that specifies data.
 * @param <C>  The type of the cache for the raw data.
 */
public abstract class CacheMap<T, K, C> implements Iterable<C> {
    /**
     * A list of raw data.
     */
    private final List<T>  rawData;

    /**
     * A map that keeps cached data.
     */
    private final Map<K, C>  cacheMap = new HashMap<>();

    /**
     * Iterator for cached data.
     */
    private final class CacheIterator implements Iterator<C> {
        /**
         * Iterator for raw data list.
         */
        private final Iterator<T>  rawIterator = rawData.iterator();

        // Iterator

        /**
         * Determine whether the iteration has more elements or not.
         *
         * @return {@code true} only if the iteration has more elements.
         */
        @Override
        public boolean hasNext() {
            return rawIterator.hasNext();
        }

        /**
         * Return the next cached data in the iteration.
         *
         * @return  The next cached data in the iteration.
         */
        @Override
        public C next() {
            T data = rawIterator.next();
            K key = getKey(data);
            C cache = cacheMap.get(key);
            if (cache == null) {
                cache = newCache(key, data);
            }

            return cache;
        }

        /**
         * This method is not supported.
         *
         * @throws UnsupportedOperationException  Always thrown.
         */
        @Override
        public void remove() {
            throw new UnsupportedOperationException();
        }
    }

    /**
     * Construct a new instance.
     *
     * @param list  A list of raw data.
     *              Note that the caller must not modify this list.
     */
    protected CacheMap(List<T> list) {
        rawData = (list == null) ? Collections.<T>emptyList() : list;
    }

    /**
     * Return the cache for the object associated with the given key.
     *
     * @param key  A key that specifies data.
     * @return  A cached data if found. {@code null} if not found.
     */
    public final C get(K key) {
        C cache = cacheMap.get(key);
        if (cache == null) {
            for (T data: rawData) {
                K k = getKey(data);
                if (k.equals(key)) {
                    cache = newCache(k, data);
                    break;
                }
            }
        }

        return cache;
    }

    /**
     * Return the cached data associated with the given key.
     *
     * @param key  A key that specifies data.
     * @return  A cached data if cached. {@code null} if not cached.
     */
    public final C getCached(K key) {
        return cacheMap.get(key);
    }

    /**
     * Associate the given cache with the given key in this instance.
     *
     * @param key    A key that specifies data.
     *               Note that the key must be contained in the raw data list
     *               passed to constructor.
     * @param value  A new cache for data specified by {@code key}.
     * @return  If the give key is contained by this instance, a cached data
     *          associated with the given key is returned.
     *          Otherwise {@code cache} is returned.
     */
    public final C put(K key, C value) {
        C cache = cacheMap.get(key);
        if (cache == null) {
            cacheMap.put(key, value);
            cache = value;
        }

        return cache;
    }

    /**
     * Return a collection of cached data.
     *
     * @return  A collection of cached data.
     */
    public final Collection<C> cachedValues() {
        return Collections.unmodifiableCollection(cacheMap.values());
    }

    /**
     * Create a new cached data.
     *
     * @param key   A key that specifies data.
     * @param data  A raw data.
     * @return  A cached data for the given data.
     */
    private C newCache(K key, T data) {
        C cache = newCache(data);
        cacheMap.put(key, cache);
        return cache;
    }

    /**
     * Return a key in the given raw data.
     *
     * @param data  A raw data.
     * @return  A key for the given data.
     */
    protected abstract K getKey(T data);

    /**
     * Create a new cached data.
     *
     * @param data  A raw data.
     * @return  A cached data for the given data.
     */
    protected abstract C newCache(T data);

    // Iterable

    /**
     * Return an iterator over a set of cached data.
     *
     * @return  An iterator for cached data.
     */
    @Override
    public Iterator<C> iterator() {
        return new CacheIterator();
    }
}
