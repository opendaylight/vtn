/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.assertNull;

import org.opendaylight.vtn.manager.internal.util.DataObjectIdentity;

import org.opendaylight.yangtools.concepts.Builder;
import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * Abstract base class for builder class that instantiate YANG binding.
 *
 * @param <T>  The type of the data object.
 * @param <B>  The type of the builder class.
 */
public abstract class BuildHelper<T extends DataObject, B extends Builder<T>>
    implements Builder<T> {
    /**
     * The builder instance.
     */
    private final B  theBuilder;

    /**
     * The fixed data object.
     */
    private T  theValue;

    /**
     * Construct a new instance.
     *
     * @param builder  The builder instance.
     */
    protected BuildHelper(B builder) {
        theBuilder = builder;
    }

    /**
     * Return the builder instance.
     *
     * @return  A {@link Builder} instance.
     */
    public B getBuilder() {
        checkNotFrozen();
        return theBuilder;
    }

    /**
     * Freeze this builder.
     *
     * <p>
     *   Values in this builder instance are fixed if this builder is frozen.
     * </p>
     */
    public final void freeze() {
        checkNotFrozen();
        freezeChildren();
        theValue = newObject();
    }

    /**
     * Determine whether the data object is changed or not.
     *
     * @param before  The data object builder that indicates the data object
     *                value before modification.
     * @return  {@code true} if the target data object is changed.
     *          {@code false} otherwise.
     */
    public boolean isChanged(BuildHelper<T, B> before) {
        DataObjectIdentity value = new DataObjectIdentity(build());
        DataObjectIdentity old = new DataObjectIdentity(before.build());
        return !value.equals(old);
    }

    /**
     * Ensure that values in this builder instance are not fixed.
     */
    protected final void checkNotFrozen() {
        assertNull(getClass().getSimpleName() + ": This builder is frozen.",
                   theValue);
    }

    /**
     * Prepare builder instance for instantiating a new object.
     *
     * <p>
     *   Subclass can override this method as needed.
     * </p>
     *
     * @param builder  The builder instance.
     */
    protected void prepare(B builder) {
    }

    /**
     * Freeze all children builders.
     */
    protected abstract void freezeChildren();

    /**
     * Construct a new instance.
     *
     * @return  A new instance.
     */
    private T newObject() {
        prepare(theBuilder);
        return theBuilder.build();
    }

    // Builder

    /**
     * Construct a new instance.
     *
     * @return  A new instance.
     */
    @Override
    public final T build() {
        return (theValue == null) ? newObject() : theValue;
    }
}
