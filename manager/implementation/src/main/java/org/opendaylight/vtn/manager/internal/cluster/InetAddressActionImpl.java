/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.InetAddressAction;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.sal.utils.Status;

/**
 * Implementation of flow action that modifies IP address in IP header.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class InetAddressActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -6711928072439846526L;

    /**
     * IP address to be set.
     */
    private final IpNetwork  address;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link InetAddressAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    protected InetAddressActionImpl(InetAddressAction act)
        throws VTNException {
        super(act);

        Status st = act.getValidationStatus();
        if (st != null) {
            throw new VTNException(st);
        }

        address = act.getIpNetwork();
        if (address == null) {
            String msg = getErrorMessage(act, "IP address");
            throw RpcException.getNullArgumentException(msg);
        }
    }

    /**
     * Return an IP address to be set.
     *
     * @return  An {@link IpNetwork} instance.
     */
    protected final IpNetwork getAddress() {
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

        InetAddressActionImpl act = (InetAddressActionImpl)o;
        return address.equals(act.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return super.hashCode() + (address.hashCode() * 31);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        return builder.append("[addr=").append(address.getText()).
            append(']').toString();
    }
}
