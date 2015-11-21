/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import java.lang.reflect.Constructor;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

/**
 * {@code TxSpecific} keeps MD-SAL datastore transaction specific data.
 *
 * @param <C>  The type of data passed to the constructor.
 */
public final class TxSpecific<C> {
    /**
     * A map that keeps constructors for transaction specific data.
     */
    private static final ConcurrentMap<Class<?>, Constructor<?>>  CONSTRUCTORS =
        new ConcurrentHashMap<>();

    /**
     * A class that specifies the type of argument for data constructor.
     */
    private final Class<C>  argumentType;

    /**
     * A map that keeps transaction specific data.
     */
    private final Map<Class<?>, Object>  dataMap = new HashMap<>();

    /**
     * Return a constructor for the given data.
     *
     * @param type     A class that specifies the type of data.
     * @param argType  A class that specifies the type of argument passed to
     *                 the constructor.
     * @param <T>      The type of the data.
     * @return  A {@link Constructor} instance.
     * @throws NoSuchMethodException
     *    The constructor for the specified data was not found.
     */
    private static <T> Constructor<T> getConstructor(
        Class<T> type, Class<?> argType) throws NoSuchMethodException {
        Constructor<?> c = CONSTRUCTORS.get(type);
        if (c == null) {
            c = type.getConstructor(argType);
            Constructor<?> old = CONSTRUCTORS.putIfAbsent(type, c);
            if (old != null) {
                c = old;
            }
        }

        @SuppressWarnings("unchecked")
        Constructor<T> ctor = (Constructor<T>)c;

        return ctor;
    }

    /**
     * Construct a new instance.
     *
     * @param argType  A class that specifies the type of argument for data
     *                 constructor.
     */
    public TxSpecific(Class<C> argType) {
        argumentType = argType;
    }

    /**
     * Return the MD-SAL datastore transaction specific data.
     *
     * @param type  A class which specifies the type of data.
     *              Note that the class must have a constructor that takes
     *              one parameter.
     * @param arg   An argument passed to the constructor of transaction
     *              specific data.
     * @param <T>  The type of transaction specific data.
     * @return  A transaction specific data specified by {@code type}.
     */
    public <T> T get(Class<T> type, C arg) {
        Object data = dataMap.get(type);
        if (data == null) {
            // Construct a new data.
            data = newData(type, arg);
            dataMap.put(type, data);
        }

        return type.cast(data);
    }

    /**
     * Clear all the cached transaction specific data.
     */
    public void clear() {
        dataMap.clear();
    }

    /**
     * Construct a new transaction specific data.
     *
     * @param type  A class that specifies the type of data.
     * @param arg   An argument passed to the constructor of transaction
     *              specific data.
     * @param <T>  The type of transaction specific data.
     * @return  A new transaction specific data.
     */
    private <T> T newData(Class<T> type, C arg) {
        try {
            Constructor<T> ctor = getConstructor(type, argumentType);
            return ctor.newInstance(arg);
        } catch (Exception e) {
            // This should never happen.
            String msg = "Unable to instantiate transaction specific data: " +
                "type=" + type + ", cause=" + e;
            throw new IllegalStateException(msg, e);
        }
    }
}
