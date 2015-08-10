/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.yangtools.yang.binding.DataObject;

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

        hash = dataType.getName().hashCode() +
            dataMap.hashCode() * MiscUtils.HASH_PRIME;
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
            // This shold be a list of DataObject.
            return fetchDataObjectList((ParameterizedType)retType, m, value);
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
     * Fetch a list of {@link DataObject} instance.
     *
     * @param pt     A {@link ParameterizedType} instance which indicates the
     *               type of a {@link DataObject} list.
     * @param m      A {@link Method} associated with a getter method.
     * @param value  A value returned by the getter method specified by
     *               {@code m}.
     * @return  An object that contains fetched {@link DataObject} instances.
     */
    private Object fetchDataObjectList(ParameterizedType pt, Method m,
                                       Object value) {
        // We need to fetch DataObject instances as a set because a list of
        // DataObject instances may be reorder by the MD-SAL.
        List<?> list = checkDataObjectList(pt, m, value);
        Set<Object> set = new HashSet<>();
        for (Object o: list) {
            Object elem = o;
            if (o instanceof DataObject) {
                elem = new DataObjectIdentity((DataObject)o);
            }

            if (!set.add(elem)) {
                String msg = String.format(
                    "List in DataObject should have no duplicate: type=%s, " +
                    "method=%s, duplicate=%s", dataType, m.getName(), o);
                throw new IllegalArgumentException(msg);
            }
        }

        return set;
    }

    /**
     * Ensure that the given object is a list of {@link DataObject} instances.
     *
     * @param pt     A {@link ParameterizedType} instance which indicates the
     *               type of a {@link DataObject} list.
     * @param m      A {@link Method} associated with a getter method.
     * @param value  A value returned by the getter method specified by
     *               {@code m}.
     * @return  {@code value} casted as a list.
     */
    private List<?> checkDataObjectList(ParameterizedType pt, Method m,
                                        Object value) {
        if (!(value instanceof List)) {
            String msg = String.format(
                "Unexpected return type: type=%s, method=%s, type=%s, value=%s",
                dataType, m.getName(), pt, value);
            throw new IllegalArgumentException(msg);
        }

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

        return (List<?>)value;
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
