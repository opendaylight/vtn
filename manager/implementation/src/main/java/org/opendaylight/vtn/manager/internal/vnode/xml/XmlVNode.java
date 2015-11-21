/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListId.getFlowDirectionName;

import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code XmlVNode} is a base class for classes that provides XML binding
 * to the data model for virtual node.
 */
@XmlRootElement(name = "vnode")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class XmlVNode {
    /**
     * The name of the virtual node.
     */
    @XmlElement(required = true)
    private VnodeName  name;

    /**
     * A brief description about the virtual node.
     */
    @XmlElement
    private String  description;

    /**
     * Convert the given {@link FlowFilterList} instance into a list of
     * {@link VtnFlowFilter} instances.
     *
     * @param xlogger  A {@link XmlLogger} instance.
     * @param ident    A {@link VNodeIdentifier} instance that specifies the
     *                 virtual node that contains this flow filter list.
     * @param filters  A {@link FlowFilterList} instance.
     * @param output   {@code true} indicates that the given flow filter list
     *                 is for outgoing packets.
     * @return  A list of {@link VtnFlowFilter} instance if {@code filters}
     *          contains at least one flow filter configuration.
     *          {@code null} otherwise.
     */
    static List<VtnFlowFilter> toVtnFlowFilterList(
        XmlLogger xlogger, VNodeIdentifier<?> ident, FlowFilterList filters,
        boolean output) {
        List<VtnFlowFilter> vfilters;
        if (filters == null) {
            vfilters = null;
        } else {
            try {
                vfilters = filters.toVtnFlowFilterList(ident);
                if (vfilters != null) {
                    xlogger.log(VTNLogLevel.DEBUG,
                                "{}: {} flow filters have been loaded.",
                                 ident, getFlowDirectionName(output));
                }
            } catch (RpcException | RuntimeException e) {
                xlogger.log(VTNLogLevel.WARN, e,
                            "%s: %s: Ignore broken flow filters.", ident,
                            getFlowDirectionName(output));
                vfilters = null;
            }
        }

        return vfilters;
    }

    /**
     * Constructor only for JAXB.
     */
    protected XmlVNode() {
    }

    /**
     * Construct a new instance.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               virtual node.
     */
    protected XmlVNode(VnodeName vname) {
        name = vname;
    }

    /**
     * Return the name of the virtual node.
     *
     * @return  The name of the virtual node.
     */
    public final VnodeName getName() {
        return name;
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
     * @param desc  A brief description about the virtual node.
     */
    final void setDescription(String desc) {
        description = desc;
    }

    /**
     * Ensure that the name of the virtual node is present.
     *
     * @param desc  A brief description about the virtual node.
     * @return  The name of the virtual node.
     * @throws RpcException  The name of the virtual node is not present.
     */
    VnodeName checkName(String desc) throws RpcException {
        MiscUtils.checkPresent(desc, name);
        return name;
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
        boolean ret = (o != null && getClass().equals(o.getClass()));
        if (ret) {
            XmlVNode xvn = (XmlVNode)o;
            ret = (Objects.equals(name, xvn.name) &&
                   Objects.equals(description, xvn.description));
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
        return Objects.hash(getClass(), name, description);
    }
}
