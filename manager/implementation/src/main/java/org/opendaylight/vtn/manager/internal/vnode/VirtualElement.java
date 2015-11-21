/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yangtools.yang.binding.DataObject;

/**
 * {@code VirtualNode} describes an abstracted data for a virtual network
 * element in the VTN.
 *
 * @param <T>  The type of the target data model.
 * @param <I>  The type of the virtual node identifier.
 */
public abstract class VirtualElement
    <T extends DataObject, I extends VNodeIdentifier<T>> {
    /**
     * The identifier for the virtual node.
     */
    private final I  identifier;

    /**
     * The data object passed to the constructor.
     */
    private final T  initialValue;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the virtual node.
     */
    protected VirtualElement(I ident) {
        this(ident, null);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the virtual node.
     * @param value  A data object read from the MD-SAL datastore.
     */
    protected VirtualElement(I ident, T value) {
        identifier = ident;
        initialValue = value;
    }

    /**
     * Return the identifier for the virtual node.
     *
     * @return  The identifier for the virtual node.
     */
    public final I getIdentifier() {
        return identifier;
    }

    /**
     * Return the initial data read from the MD-SAL datastore.
     *
     * @return  The initial data read from the MD-SAL datastore.
     */
    public final T getInitialValue() {
        return initialValue;
    }

    /**
     * Return a logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    protected abstract Logger getLogger();
}
