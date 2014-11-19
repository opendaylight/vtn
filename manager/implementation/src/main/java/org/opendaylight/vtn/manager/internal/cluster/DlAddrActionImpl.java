/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Arrays;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.DlAddrAction;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of flow action that modifies MAC address in Ethernet frame.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class DlAddrActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -6583151077348712604L;

    /**
     * MAC address to be set.
     */
    private final byte[]  address;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link DlAddrAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    protected DlAddrActionImpl(DlAddrAction act) throws VTNException {
        super(act);

        Status st = act.getValidationStatus();
        if (st != null) {
            throw new VTNException(st);
        }

        byte[] addr = act.getAddress();
        if (addr == null) {
            String msg = getErrorMessage(act, "MAC address");
            st = MiscUtils.argumentIsNull(msg);
            throw new VTNException(st);
        }

        if (addr.length != NetUtils.MACAddrLengthInBytes) {
            String msg = getErrorMessage(
                act, "Invalid MAC address length: ",
                HexEncode.bytesToHexStringFormat(addr));
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        if (NetUtils.isBroadcastMACAddr(addr)) {
            String msg = getErrorMessage(
                act, "Broadcast address cannot be specified");
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
        if (!NetUtils.isUnicastMACAddr(addr)) {
            String msg = getErrorMessage(
                act, "Multicast address cannot be specified: ",
                HexEncode.bytesToHexStringFormat(addr));
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
        if (NetUtils.byteArray6ToLong(addr) == 0) {
            String msg = getErrorMessage(act, "Zero cannot be specified");
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        address = addr.clone();
    }

    /**
     * Return a raw bytes of MAC address.
     *
     * @return  An array of bytes which represents a MAC address.
     */
    protected final byte[] getAddress() {
        return address;
    }

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

        DlAddrActionImpl act = (DlAddrActionImpl)o;
        return Arrays.equals(address, act.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return super.hashCode() ^ Arrays.hashCode(address);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        return builder.append("[addr=").
            append(HexEncode.bytesToHexStringFormat(address)).
            append(']').toString();
    }
}
