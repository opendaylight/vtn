/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.flow.action.InetAddressAction;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.VtnIpaddrActionFields;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv6;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv6Prefix;

/**
 * {@code VTNInetAddrAction} describes the flow action that sets the IP address
 * into IP header.
 */
@XmlRootElement(name = "vtn-ipaddr-action")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VTNSetInetDstAction.class, VTNSetInetSrcAction.class})
public abstract class VTNInetAddrAction extends FlowFilterAction {
    /**
     * The IP address to be set.
     */
    @XmlElements({
        @XmlElement(name = "ipv4-address", type = Ip4Network.class,
                    required = true)})
    private IpNetwork  address;

    /**
     * Construct an empty instance.
     */
    VTNInetAddrAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param addr  An {@link IpNetwork} instance which represents the
     *              IP address to be set.
     */
    protected VTNInetAddrAction(IpNetwork addr) {
        address = addr;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link InetAddressAction} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    protected VTNInetAddrAction(InetAddressAction act, int ord)
        throws RpcException {
        super(Integer.valueOf(ord));
        Status st = act.getValidationStatus();
        if (st != null) {
            throw new RpcException(RpcErrorTag.BAD_ELEMENT, st);
        }
        address = act.getIpNetwork();
        verify();
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnIpaddrActionFields} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    protected VTNInetAddrAction(VtnIpaddrActionFields act, Integer ord)
        throws RpcException {
        super(ord);
        if (act != null) {
            address = IpNetwork.create(act.getAddress());
        }
        verify();
    }

    /**
     * Return the IP address to be set.
     *
     * @return  The IP address to be set.
     */
    public final IpNetwork getAddress() {
        return address;
    }

    /**
     * Return the IP address to be set as an {@link Address} instance.
     *
     * @return  An {@link Address} instance.
     */
    public final Address getMdAddress() {
        return address.getMdAddress();
    }

    /**
     * Return an exception which indicates no IP address is specified.
     *
     * @param obj  An object to be added to the error message.
     * @return  An {@link RpcException} instance.
     */
    protected final RpcException noIpAddress(Object obj) {
        String msg = getErrorMessage("No IP address", obj);
        return RpcException.getMissingArgumentException(msg);
    }

    /**
     * Return an exception which indicates an invalid IP address is specified.
     *
     * @param obj    An object to be added to the error message.
     * @param cause  A throwable which indicates the cause of error.
     * @return  An {@link RpcException} instance.
     */
    protected final RpcException invalidIpAddress(Object obj, Throwable cause) {
        String msg = getErrorMessage("Invalid IP address", obj);
        RpcException re = RpcException.getMissingArgumentException(msg);
        re.initCause(cause);
        return re;
    }

    /**
     * Return a description about the specified MD-SAL action.
     *
     * @param name  The name of the flow action.
     * @param addr  An {@link Address} instance.
     * @return  A description about the specified MD-SAL action.
     */
    protected final String getDescription(String name, Address addr) {
        StringBuilder builder = new StringBuilder(name).append('(');
        if (addr instanceof Ipv4) {
            Ipv4 v4 = (Ipv4)addr;
            Ipv4Prefix ipv4 = v4.getIpv4Address();
            String txt = (ipv4 == null) ? null : ipv4.getValue();
            builder.append("ipv4=").append(txt);
        } else if (addr instanceof Ipv6) {
            Ipv6 v6 = (Ipv6)addr;
            Ipv6Prefix ipv6 = v6.getIpv6Address();
            String txt = (ipv6 == null) ? null : ipv6.getValue();
            builder.append("ipv6=").append(txt);
        }

        return builder.append(')').toString();
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void verifyImpl() throws RpcException {
        if (address == null) {
            String msg = getErrorMessage("IP address");
            throw MiscUtils.getNullArgumentException(msg);
        }
        if (!address.isAddress()) {
            String msg = getErrorMessage("Netmask cannot be specified",
                                         address);
            throw RpcException.getBadArgumentException(msg);
        }
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void appendContents(StringBuilder builder) {
        builder.append("addr=").append(address.getHostAddress());
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

        VTNInetAddrAction va = (VTNInetAddrAction)o;
        return Objects.equals(address, va.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        int h = super.hashCode();
        if (address != null) {
            h = h * HASH_PRIME + address.hashCode();
        }

        return h;
    }
}
