/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.TpPortAction;

import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of flow action that modifies port number in transport
 * protocol header.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class TpPortActionImpl extends FlowActionImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2892201590657891707L;

    /**
     * A port number to be set.
     */
    private final int  port;

    /**
     * Construct a new instance.
     *
     * @param act  A {@link TpPortAction} instance.
     * @throws VTNException
     *    {@code act} contains invalid value.
     */
    protected TpPortActionImpl(TpPortAction act) throws VTNException {
        super(act);

        port = act.getPort();
        if (!ProtocolUtils.isPortNumberValid(port)) {
            String msg = getErrorMessage(act, "Invalid port number: ", port);
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Return a port number to be set.
     *
     * @return  A port number.
     */
    protected int getPort() {
        return port;
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

        TpPortActionImpl act = (TpPortActionImpl)o;
        return (port == act.port);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return super.hashCode() + (port * 41);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        return builder.append("[port=").append(port).append(']').toString();
    }
}
