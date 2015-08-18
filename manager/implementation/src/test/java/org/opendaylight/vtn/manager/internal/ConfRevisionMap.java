/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.concurrent.ConcurrentHashMap;

import org.opendaylight.vtn.manager.internal.cluster.MapType;

/**
 * An instance of this class is used as a cluster cache which keeps
 * revision number of VTN Manager's virtual network mapping configuration.
 *
 * <p>
 *   The purpose of this class is to test failure case due to concurrent
 *   update by another cluster node.
 * </p>
 */
public class ConfRevisionMap extends ConcurrentHashMap<MapType, Integer> {
    /**
     * The number of concurrent update by another cluster node.
     */
    private int  failureCount;

    /**
     * The number of actual {@link #putIfAbsent(MapType, Integer)} call.
     */
    private int  putIfAbsentCount;

    /**
     * The number of actual {@link #replace(MapType, Integer, Integer)} call.
     */
    private int  replaceCount;

    /**
     * The number of actual {@link #get(Object)} call.
     */
    private int  getCount;

    /**
     * Construct a new object.
     */
    public ConfRevisionMap() {
        super();
    }

    /**
     * Activate or inactivate {@link #replace(MapType, Integer, Integer)} test.
     *
     * @param num  The number of concurrent update by another cluster node.
     *             The test is inactivated if {@code 0} is specified.
     */
    public synchronized void activateTest(int num) {
        resetCounters();
        failureCount = num;
    }

    /**
     * Reset all counters.
     */
    public synchronized void resetCounters() {
        putIfAbsentCount = 0;
        replaceCount = 0;
        getCount = 0;
    }

    /**
     * Return the number of {@link #putIfAbsent(MapType, Integer)} call.
     *
     * @return  The number of {@link #putIfAbsent(MapType, Integer)} call.
     */
    public synchronized int getPutIfAbsentCount() {
        return putIfAbsentCount;
    }

    /**
     * Return the number of {@link #replace(MapType, Integer, Integer)} call.
     *
     * @return  The number of {@link #replace(MapType, Integer, Integer)} call.
     */
    public synchronized int getReplaceCount() {
        return replaceCount;
    }

    /**
     * Return the number of {@link #get(Object)} call.
     *
     * @return  The number of {@link #get(Object)} call.
     */
    public synchronized int getGetCount() {
        return getCount;
    }

    /**
     * Associated a value to the specified key only if this map does not
     * contain the key,
     *
     * @param key    A key with which the given value is associated.
     * @param value  A value to be associated with the given key.
     * @return       The previous value associated with the given key, or
     *               {@code null} if there was no mapping for the key.
     */
    @Override
    public Integer putIfAbsent(MapType key, Integer value) {
        synchronized (this) {
            putIfAbsentCount++;
        }

        return super.putIfAbsent(key, value);
    }

    /**
     * Replace the entry for a key only if currently mapped to the given value.
     *
     * @param key       A key with which the given value is associated.
     * @param oldValue  A value expected to be associated with the given key.
     * @param newValue  A value to be associated with the given key.
     * @return         {@code true} is returned only if the value was replaced.
     */
    @Override
    public boolean replace(MapType key, Integer oldValue, Integer newValue) {
        synchronized (this) {
            replaceCount++;
            if (failureCount > 0) {
                if (replaceCount <= failureCount) {
                    return false;
                }
            }
        }

        return super.replace(key, oldValue, newValue);
    }

    /**
     * Return a value associated with the given key in this map.
     *
     * @param key  The key whose associated value is to be returned.
     * @return     A value associated with the given key, or {@code null}
     *             if there was no mapping for the key.
     */
    @Override
    public Integer get(Object key) {
        synchronized (this) {
            getCount++;
            if (failureCount > 0) {
                if (getCount <= failureCount) {
                    return Integer.valueOf(-1);
                }
            }
        }

        return super.get(key);
    }
}
