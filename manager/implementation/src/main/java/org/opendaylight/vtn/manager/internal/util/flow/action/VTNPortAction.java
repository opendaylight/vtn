/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.flow.action.TpPortAction;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.VtnPortActionFields;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNPortAction} describes the flow action that sets the port number
 * for IP transport layer protocol into packet.
 */
@XmlRootElement(name = "vtn-port-action")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VTNSetPortDstAction.class, VTNSetPortSrcAction.class})
public abstract class VTNPortAction extends FlowFilterAction {
    /**
     * Default port number.
     */
    private static final int  DEFAULT_VALUE = 0;

    /**
     * The port number to be set.
     */
    @XmlElement
    private int  port;

    /**
     * Construct an empty instance.
     */
    VTNPortAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param p  The port numbet to be set.
     */
    protected VTNPortAction(int p) {
        port = p;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link TpPortAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    protected VTNPortAction(TpPortAction act, int ord)
        throws RpcException {
        super(Integer.valueOf(ord));
        port = act.getPort();
        verify();
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnPortActionFields} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    protected VTNPortAction(VtnPortActionFields act, Integer ord)
        throws RpcException {
        super(ord);
        port = getPortNumber(act);
        verify();
    }

    /**
     * Return the port number to be set.
     *
     * @return  The port number to be set.
     */
    public final int getPort() {
        return port;
    }

    /**
     * Return the port number to be set as a {@link PortNumber} instance.
     *
     * @return  A {@link PortNumber} instance.
     */
    public final PortNumber getPortNumber() {
        return new PortNumber(Integer.valueOf(port));
    }

    /**
     * Return the port number configured in the given
     * {@link VtnPortActionFields} instance.
     *
     * @param vport  A {@link VtnPortActionFields} instance.
     * @return  The port number.
     */
    protected final int getPortNumber(VtnPortActionFields vport) {
        if (vport != null) {
            PortNumber pnum = vport.getPort();
            if (pnum != null) {
                Integer v = pnum.getValue();
                if (v != null) {
                    return v.intValue();
                }
            }
        }

        return DEFAULT_VALUE;
    }

    /**
     * Return an exception which indicates no port number is specified.
     *
     * @param obj  An object to be added to the error message.
     * @return  An {@link RpcException} instance.
     */
    protected final RpcException noPortNumber(Object obj) {
        String msg = getErrorMessage("No port number", obj);
        return RpcException.getMissingArgumentException(msg);
    }

    /**
     * Return a description about the specified MD-SAL action.
     *
     * @param name  The name of the flow action.
     * @param port  A {@link PortNumber} instance.
     * @return  A description about the specified MD-SAL action.
     */
    protected final String getDescription(String name, PortNumber port) {
        Integer num = (port == null) ? null : port.getValue();
        return new StringBuilder(name).append("(port=").
            append(num).append(')').toString();
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void verifyImpl() throws RpcException {
        if (!ProtocolUtils.isPortNumberValid(port)) {
            String msg = getErrorMessage("Invalid port number", port);
            throw RpcException.getBadArgumentException(msg);
        }
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void appendContents(StringBuilder builder) {
        builder.append("port=").append(port);
        super.appendContents(builder);
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        VTNPortAction va = (VTNPortAction)o;
        return (port == va.port);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return super.hashCode() * MiscUtils.HASH_PRIME + port;
    }
}
