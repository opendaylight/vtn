/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Objects;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * A class used to keep notified data objects.
 *
 * @param <T>  The type of the target data object.
 */
public final class NotifiedEvent<T extends DataObject> {
    /**
     * Path to the data object.
     */
    private final InstanceIdentifier<T>  path;

    /**
     * A new object.
     */
    private final T  newObject;

    /**
     * An old object.
     */
    private final T  oldObject;

    /**
     * The type of the event.
     */
    private final VtnUpdateType  updateType;

    /**
     * Construct a new instance.
     *
     * @param p       Path to the data object.
     * @param newObj  A new object.
     * @param oldObj  An old object.
     * @param utype   A {@link VtnUpdateType} instance.
     */
    public NotifiedEvent(InstanceIdentifier<T> p, T newObj, T oldObj,
                         VtnUpdateType utype) {
        path = p;
        newObject = newObj;
        oldObject = oldObj;
        updateType = utype;
    }

    /**
     * Construct a new instance.
     *
     * @param data   An {@link IdentifiedData} object.
     * @param utype  A {@link VtnUpdateType} instance.
     */
    public NotifiedEvent(IdentifiedData<T> data, VtnUpdateType utype) {
        this(data.getIdentifier(), data.getValue(), null, utype);
    }

    /**
     * Construct a new instance.
     *
     * @param data  A {@link ChangedData} object.
     */
    public NotifiedEvent(ChangedData<T> data) {
        this(data.getIdentifier(), data.getValue(), data.getOldValue(),
             VtnUpdateType.CHANGED);
    }

    /**
     * Return the path to the data object.
     *
     * @return  Path to the data object.
     */
    public InstanceIdentifier<T> getPath() {
        return path;
    }

    /**
     * Return a new object.
     *
     * @return  A new object.
     */
    public T getNewObject() {
        return newObject;
    }

    /**
     * Return an old object.
     *
     * @return  An old object.
     */
    public T getOldObject() {
        return oldObject;
    }

    /**
     * Return the type of the event.
     *
     * @return  A {@link VtnUpdateType} instance.
     */
    public VtnUpdateType getUpdateType() {
        return updateType;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            NotifiedEvent<?> nev = (NotifiedEvent<?>)o;
            ret = (Objects.equals(path, nev.path) &&
                   Objects.equals(newObject, nev.newObject) &&
                   Objects.equals(oldObject, nev.oldObject));
            if (ret) {
                ret = (updateType == nev.updateType);
            }
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(path, newObject, oldObject, updateType);
    }

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public String toString() {
        return "NotifiedEvent[path=" + path + ", new=" + newObject +
            ", old=" + oldObject + ", type=" + updateType + "]";
    }
}
