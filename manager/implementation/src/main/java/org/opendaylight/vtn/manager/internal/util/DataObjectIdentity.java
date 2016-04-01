/*
 * Copyright (c) 2013, 2016 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.annotation.Nonnull;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.Identifiable;
import org.opendaylight.yangtools.yang.binding.Identifier;

/**
 * {@code DataObjectIdentity} describes identify of {@link DataObject}
 * instance.
 *
 * <p>
 *   A list of {@link DataObject} contained in a {@link DataObject} may be
 *   reorderd by the MD-SAL. So {@link Object#equals(Object)} and
 *   {@link Object#hashCode()} cannot be used to compare {@link DataObject}
 *   instances.
 * </p>
 * <ul>
 *   <li>Note that this class does not support augmentation.</li>
 *   <li>
 *     This class expects any list of {@link DataObject} instances has no
 *     duplicate.
 *   </li>
 * </ul>
 */
public final class DataObjectIdentity {
    /**
     * The name prefix of the getter method.
     */
    private static final String  METHOD_PREFIX_GETTER = "get";

    /**
     * The name prefix of the getter method that returns a boolean value.
     */
    private static final String  METHOD_PREFIX_IS = "is";

    /**
     * The name of the method that returns the primary key instance.
     */
    private static final String  METHOD_GET_KEY = "getKey";

    /**
     * The name of the method that returns the interface type implemented by
     * the data object.
     */
    private static final String METHOD_GET_IMPL = "getImplementedInterface";

    /**
     * A map that keeps values in the target {@link DataObject} instance.
     */
    private final Map<String, Object>  dataMap = new HashMap<>();

    /**
     * Pre-computed hash code.
     */
    private final int  hash;

    /**
     * Type of the target {@link DataObject} instance.
     */
    private final Class<?>  dataType;

    /**
     * Construct a new instance.
     *
     * @param data  A {@link DataObject} instance.
     * @throws IllegalStateException
     *    Unexpected data object is passed.
     */
    public DataObjectIdentity(DataObject data) {
        dataType = data.getImplementedInterface();

        try {
            initialize(data);
        } catch (IllegalAccessException | InvocationTargetException e) {
            String msg = "Failed to fetch contents of DataObject.";
            throw new IllegalStateException(msg, e);
        }

        hash = dataType.getName().hashCode() + dataMap.hashCode() * HASH_PRIME;
    }

    /**
     * Fetch contents of the given {@link DataObject} instance.
     *
     * @param data  A {@link DataObject} instance.
     * @throws IllegalAccessException
     *    Failed to access methods of the given instance.
     * @throws InvocationTargetException
     *    Caught an exception while fetching contents of the given instance.
     */
    private void initialize(DataObject data)
        throws IllegalAccessException, InvocationTargetException {
        for (Method m: dataType.getMethods()) {
            // Getter method should be a public instance method.
            int mod = m.getModifiers();
            if (!Modifier.isPublic(mod) || Modifier.isStatic(mod)) {
                continue;
            }

            Class<?>[] paramTypes = m.getParameterTypes();
            if (paramTypes.length > 0) {
                continue;
            }

            // Eliminate special methods.
            String name = m.getName();
            if (name.equals(METHOD_GET_KEY) || name.equals(METHOD_GET_IMPL)) {
                continue;
            }

            if (name.startsWith(METHOD_PREFIX_IS)) {
                // isXXX() should return a Boolean instance.
                if (!Boolean.class.equals(m.getReturnType())) {
                    continue;
                }
            } else if (!name.startsWith(METHOD_PREFIX_GETTER)) {
                continue;
            }

            // Fetch value.
            Object value = fetch(data, m);
            dataMap.put(name, value);
        }
    }

    /**
     * Fetch the value returned by the given getter method.
     *
     * @param data  A {@link DataObject} instance.
     * @param m     A {@link Method} instance associated with a getter method.
     * @return  An object returned by the given method.
     * @throws IllegalAccessException
     *    Failed to access the given method.
     * @throws InvocationTargetException
     *    Caught an exception while calling the given method.
     */
    private Object fetch(DataObject data, Method m)
        throws IllegalAccessException, InvocationTargetException {
        // Invoke the given getter method.
        Object value = m.invoke(data);
        if (value == null) {
            return null;
        }

        // Determine return type.
        Type retType = m.getGenericReturnType();
        if (retType instanceof ParameterizedType) {
            return fetchList((ParameterizedType)retType, m, value);
        }

        if (retType instanceof Class) {
            return (value instanceof DataObject)
                ? new DataObjectIdentity((DataObject)value)
                : value;
        }

        String msg = String.format(
            "Unexpected return type: type=%s, method=%s, ret=%s",
            dataType, m.getName(), retType);
        throw new IllegalArgumentException(msg);
    }

    /**
     * Fetch a field value as a list.
     *
     * @param pt     A {@link ParameterizedType} instance which indicates the
     *               type of the list.
     * @param m      A {@link Method} associated with a getter method.
     * @param value  A value returned by the getter method specified by
     *               {@code m}.
     * @return  An object that contains fetched list elements.
     */
    private Object fetchList(ParameterizedType pt, Method m,
                             @Nonnull Object value) {
        // We need to fetch list elements as a map because the order of the
        // elements in the list defined by YANG is unspecified.
        // So elements in the list may be reorderd by the MD-SAL.
        if (!(value instanceof List)) {
            String msg = String.format(
                "Unexpected return type: type=%s, method=%s, type=%s, value=%s",
                dataType, m.getName(), pt, value);
            throw new IllegalArgumentException(msg);
        }
        List<?> list = (List<?>)value;
        Class<?> type = getListParameterType(pt, m);
        Object ret;
        if (Identifiable.class.isAssignableFrom(type)) {
            // Fetch all the elements in the keyed list.
            @SuppressWarnings("unchecked")
            List<Identifiable<?>> keyed = (List<Identifiable<?>>)list;
            ret = fetchKeyedDataObjectList(m, keyed);
        } else {
            // The given list may have duplicate element.
            Map<Object, Integer> map = new HashMap<>();
            for (Object o: list) {
                Object elem = o;
                if (o instanceof DataObject) {
                    elem = new DataObjectIdentity((DataObject)o);
                }

                Integer oldCount = map.put(elem, 1);
                if (oldCount != null) {
                    map.put(elem, oldCount.intValue() + 1);
                }
            }
            ret = map;
        }

        return ret;
    }

    /**
     * Fetch a keyed list of {@link DataObject} instances.
     *
     * @param m     A {@link Method} associated with a getter method.
     * @param list  A list of keyed {@link DataObject} instances.
     * @return  An object that contains fetched {@link DataObject} instances.
     */
    private Object fetchKeyedDataObjectList(
        Method m, @Nonnull List<Identifiable<?>> list) {
        Map<Object, Object> map = new HashMap<>();
        for (Identifiable<?> elem: list) {
            // An element in a keyed list must be a DataObject.
            @SuppressWarnings("unchecked")
            DataObject data = (DataObject)elem;
            Identifier<?> key = elem.getKey();
            if (map.put(key, new DataObjectIdentity(data)) != null) {
                String msg = String.format(
                    "Keyed list in DataObject should have no duplicate: " +
                    "type=%s, method=%s, key=%s, value=%s", dataType,
                    m.getName(), key, elem);
                throw new IllegalArgumentException(msg);
            }
        }

        return map;
    }

    /**
     * Return a class that indicates the type of list parameter.
     *
     * @param pt     A {@link ParameterizedType} instance that indicates the
     *               type of a {@link DataObject} list.
     * @param m      A {@link Method} associated with a getter method.
     * @return  A class that indicates the type of list parameter.
     */
    private Class<?> getListParameterType(ParameterizedType pt, Method m) {
        Type[] actual = pt.getActualTypeArguments();
        if (actual.length != 1) {
            String msg = String.format(
                "Unexpected list parameter length: type=%s, method=%s, " +
                "length=%d", dataType, m.getName(), actual.length);
            throw new IllegalArgumentException(msg);
        }

        Type t = actual[0];
        if (!(t instanceof Class)) {
            String msg = String.format(
                "Unexpected list parameter: type=%s, method=%s, paramType=%s",
                dataType, m.getName(), t);
            throw new IllegalArgumentException(msg);
        }

        return (Class<?>)t;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        DataObjectIdentity doi = (DataObjectIdentity)o;
        return (dataType.equals(doi.dataType) && dataMap.equals(doi.dataMap));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return hash;
    }
}
