/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.VtnDladdrActionFields;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VTNDlAddrAction} describes the flow action that sets the MAC address
 * into Ethernet header.
 */
@XmlRootElement(name = "vtn-dladdr-action")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VTNSetDlSrcAction.class, VTNSetDlDstAction.class})
public abstract class VTNDlAddrAction extends FlowFilterAction {
    /**
     * The MAC address to be set.
     */
    @XmlElement(required = true)
    private EtherAddress  address;

    /**
     * Construct an empty instance.
     */
    VTNDlAddrAction() {
    }

    /**
     * Construct a new instance without specifying action order.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     */
    protected VTNDlAddrAction(EtherAddress addr) {
        address = addr;
    }

    /**
     * Construct a new instance with specifying action order.
     *
     * @param addr  An {@link EtherAddress} instance which represents the
     *              MAC address to be set.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     */
    protected VTNDlAddrAction(EtherAddress addr, Integer ord) {
        super(ord);
        address = addr;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A {@link VtnDladdrActionFields} instance.
     * @param ord  An integer which determines the order of flow actions
     *             in a flow entry.
     * @throws RpcException  An invalid argument is specified.
     */
    protected VTNDlAddrAction(VtnDladdrActionFields act, Integer ord)
        throws RpcException {
        super(ord);
        if (act != null) {
            address = MiscUtils.toEtherAddress(act.getAddress());
        }
        verify();
    }

    /**
     * Return the MAC address to be set.
     *
     * @return  The MAC address to be set.
     */
    public final EtherAddress getAddress() {
        return address;
    }

    /**
     * Return the MAC address to be set as a {@link MacAddress} instance.
     *
     * @return  A {@link MacAddress} instance.
     */
    public final MacAddress getMacAddress() {
        return address.getMacAddress();
    }

    /**
     * Return an exception which indicates no MAC address is specified.
     *
     * @param obj  An object to be added to the error message.
     * @return  An {@link RpcException} instance.
     */
    protected final RpcException noMacAddress(Object obj) {
        String msg = getErrorMessage("No MAC address", obj);
        return RpcException.getMissingArgumentException(msg);
    }

    /**
     * Return a description about the specified MD-SAL action.
     *
     * @param name  The name of the flow action.
     * @param mac   A {@link MacAddress} instance.
     * @return  A description about the specified MD-SAL action.
     */
    protected final String getDescription(String name, MacAddress mac) {
        String addr = (mac == null) ? null : mac.getValue();
        return new StringBuilder(name).append("(address=").
            append(addr).append(')').toString();
    }

    /**
     * Return a description about the specified VTN action.
     *
     * @param name  The name of the flow action.
     * @param act   A {@link VtnDladdrActionFields} instance.
     * @return  A description about the specified VTN action.
     */
    protected final String getDescription(String name,
                                          VtnDladdrActionFields act) {
        MacAddress mac = act.getAddress();
        String addr = (mac == null) ? null : mac.getValue();
        return name + "(" + addr + ")";
    }

    // FlowFilterAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void verifyImpl() throws RpcException {
        if (address == null) {
            String msg = getErrorMessage("MAC address");
            throw RpcException.getNullArgumentException(msg);
        }

        if (address.isBroadcast()) {
            String msg = getErrorMessage(
                "Broadcast address cannot be specified.");
            throw RpcException.getBadArgumentException(msg);
        }
        if (!address.isUnicast()) {
            String msg = getErrorMessage(
                "Multicast address cannot be specified", address.getText());
            throw RpcException.getBadArgumentException(msg);
        }
        if (address.getAddress() == 0L) {
            String msg = getErrorMessage("Zero cannot be specified.");
            throw RpcException.getBadArgumentException(msg);
        }
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void appendContents(StringBuilder builder) {
        builder.append("addr=").append(address.getText());
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

        VTNDlAddrAction va = (VTNDlAddrAction)o;
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
