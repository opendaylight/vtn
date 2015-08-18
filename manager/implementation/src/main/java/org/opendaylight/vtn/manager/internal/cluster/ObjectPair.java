/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

/**
 * A pair of two arbitrary objects.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 *
 * @param <L>   Type of the object to be located at the left side.
 * @param <R>   Type of the object to be located at the right side.
 */
public class ObjectPair<L, R> implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6758220741320387162L;

    /**
     * An arbitrary object located at the left side.
     */
    private final L  left;

    /**
     * An arbitrary object located at the right side.
     */
    private final R  right;

    /**
     * Construct a new object pair.
     *
     * @param left   An arbitrary object to set to left side.
     * @param right  An arbitrary object to set to right side.
     */
    public ObjectPair(L left, R right) {
        this.left = left;
        this.right = right;
    }

    /**
     * Return an object at the left side.
     *
     * @return  An object at the left side.
     */
    public L getLeft() {
        return left;
    }

    /**
     * Return an object at the right side.
     *
     * @return  An object at the right side.
     */
    public R getRight() {
        return right;
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
        if (!(o instanceof ObjectPair)) {
            return false;
        }

        ObjectPair another = (ObjectPair)o;
        if (left == null) {
            if (another.left != null) {
                return false;
            }
        } else if (!left.equals(another.left)) {
            return false;
        }

        if (right == null) {
            return (another.right == null);
        }

        return right.equals(another.right);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (left != null) {
            h += left.hashCode();
        }
        if (right != null) {
            h += right.hashCode();
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("ObjectPair[left=");
        builder.append(String.valueOf(left)).
            append(",right=").append(String.valueOf(right)).append(']');

        return builder.toString();
    }
}
