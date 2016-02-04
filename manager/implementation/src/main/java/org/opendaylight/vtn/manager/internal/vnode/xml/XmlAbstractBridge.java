/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;

/**
 * {@code XmlAbstractBridge} provides XML binding to the data model for the
 * virtual node that contains a list of virtual interfaces.
 */
@XmlRootElement(name = "vtn-bridge")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class XmlAbstractBridge extends XmlVNode {
    /**
     * A list of virtual interfaces.
     */
    @XmlElementWrapper(name = "vinterfaces")
    @XmlElement(name = "vinterface")
    private List<XmlVInterface>  interfaces;

    /**
     * Constructor only for JAXB.
     */
    protected XmlAbstractBridge() {
    }

    /**
     * Construct a new instance.
     *
     * @param vname   A {@link VnodeName} instance that contains the name of
     *                the virtual node.
     * @param ifList  A list of {@link Vinterface} instances.
     * @throws RpcException
     *    The given argument is invalid.
     */
    protected XmlAbstractBridge(VnodeName vname, List<Vinterface> ifList)
        throws RpcException {
        super(vname);
        interfaces = XmlVInterface.toList(ifList);
    }

    /**
     * Return a list of virtual interface configurations.
     *
     * @return  A list of {@link XmlVInterface} instances or {@code null}.
     */
    public final List<XmlVInterface> getInterfaces() {
        return (interfaces == null || interfaces.isEmpty())
            ? null : interfaces;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && super.equals(o)) {
            XmlAbstractBridge xb = (XmlAbstractBridge)o;
            ret = MiscUtils.equalsAsSet(interfaces, xb.interfaces);
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
        int h = super.hashCode() * HASH_PRIME;
        if (interfaces != null) {
            for (XmlVInterface xvif: interfaces) {
                h += xvif.hashCode();
            }
        }

        return h;
    }
}
