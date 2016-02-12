/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VNodeConfig} describes common configuration about a virtual node.
 */
public abstract class VNodeConfig {
    /**
     * The name of the virtual node.
     */
    private final String  name;

    /**
     * Description about the virtual node.
     */
    private final String  description;

    /**
     * Construct a new instance.
     *
     * @param nm    The name of the virtual node.
     * @param desc  Description about the virtual node.
     */
    protected VNodeConfig(String nm, String desc) {
        name = nm;
        description = desc;
    }

    /**
     * Return the name of the virtual node.
     *
     * @return  The name of the virtual node.
     */
    public final String getName() {
        return name;
    }

    /**
     * Return a description about the virtual node.
     *
     * @return  Description about the virtual node.
     */
    public final String getDescription() {
        return description;
    }

    /**
     * Return the name of the virtual node as vnode-name.
     *
     * @return  A {@link VnodeName} instance.
     */
    public final VnodeName getVnodeName() {
        return new VnodeName(name);
    }

    /**
     * Ensure that the given {@link XmlVNode} instance is identical to this
     * instance.
     *
     * @param xvn  A {@link XmlVNode} instance.
     */
    public final void verify(XmlVNode xvn) {
        assertEquals(name, xvn.getName().getValue());
        assertEquals(description, xvn.getDescription());
    }

    /**
     * Set the virtual node configuration into the specified XML node.
     *
     * @param xnode   A {@link XmlNode} instance.
     */
    public final void setXml(XmlNode xnode) {
        xnode.add(new XmlNode("name", name));
        if (description != null) {
            xnode.add(new XmlNode("description", description));
        }
    }
}
