/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

/**
 * {@code VNodeConfig} describes the configuration of a virtual node.
 *
 * @param <C>  The type of this instance.
 */
public abstract class VNodeConfig<C extends VNodeConfig> {
    /**
     * Description about the virtual node.
     */
    private String  description;

    /**
     * Construct a new instance without specifying the description.
     */
    protected VNodeConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The description about the virtual node.
     */
    protected VNodeConfig(String desc) {
        description = desc;
    }

    /**
     * Return the description about the virtual node.
     *
     * @return  The description about the virtual node.
     */
    public final String getDescription() {
        return description;
    }

    /**
     * Set the description about the virtual node.
     *
     * @param desc  The description about the virtual node.
     * @return  This instance.
     */
    public final C setDescription(String desc) {
        description = desc;
        return getThis();
    }

    /**
     * Return this instance.
     *
     * @return  This instance.
     */
    protected final C getThis() {
        @SuppressWarnings("unchecked")
        C ret = (C)this;
        return ret;
    }
}
