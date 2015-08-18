/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

/**
 * {@code DataTypeMismatchException} indicates that the type of data does not
 * match the expected type.
 */
public final class DataTypeMismatchException extends Exception {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1L;

    /**
     * A class which indicates the expected data type.
     */
    private Class<?>  targetType;

    /**
     * An object that caused this exception.
     */
    private Object  object;

    /**
     * Construct a new instance.
     *
     * @param type  A class which indicates the expected data type.
     * @param obj   An object.
     */
    public DataTypeMismatchException(Class<?> type, Object obj) {
        targetType = type;
        object = obj;
    }

    /**
     * Return a class which indicates the expected data type.
     *
     * @return  A class which indicates the expected data type.
     */
    public Class<?> getTargetType() {
        return targetType;
    }

    /**
     * Return an object that caused this exception.
     *
     * @return  An object.
     */
    public Object getObject() {
        return object;
    }
}
