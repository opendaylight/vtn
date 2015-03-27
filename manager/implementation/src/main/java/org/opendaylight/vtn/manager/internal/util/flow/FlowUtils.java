/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnFlowTimeoutConfig;

/**
 * {@code FlowUtils} class is a collection of utility class methods for
 * flow entry management.
 */
public final class FlowUtils {
    /**
     * A bitmask that represents valid bits in a flow timeout value.
     */
    private static final int  FLOW_TIMEOUT_MASK = 0xffff;

    /**
     * A brief description about flow idle-timeout value.
     */
    private static final String DESC_IDLE_TIMEOUT = "idle-timeout";

    /**
     * A brief description about flow hard-timeout value.
     */
    private static final String DESC_HARD_TIMEOUT = "hard-timeout";

    /**
     * Private constructor that protects this class from instantiating.
     */
    private FlowUtils() {}

    /**
     * Verify the given flow timeout values.
     *
     * @param idle  The idle-timeout value.
     * @param hard  The hard-timeout value.
     * @throws RpcException  The given flow timeout values are invalid.
     */
    public static void verifyFlowTimeout(Integer idle, Integer hard)
        throws RpcException {
        verifyFlowTimeout(idle, DESC_IDLE_TIMEOUT);
        verifyFlowTimeout(hard, DESC_HARD_TIMEOUT);

        if (hard == null) {
            if (idle != null) {
                String msg = new StringBuilder(DESC_HARD_TIMEOUT).
                    append(" must be specified.").toString();
                throw RpcException.getBadArgumentException(msg);
            }
        } else if (idle == null) {
            String msg = new StringBuilder(DESC_IDLE_TIMEOUT).
                append(" must be specified.").toString();
            throw RpcException.getBadArgumentException(msg);
        } else {
            int it = idle.intValue();
            int ht = hard.intValue();
            if (it != 0 && ht != 0 &&  it >= ht) {
                String msg = new StringBuilder(DESC_IDLE_TIMEOUT).
                    append(" must be less than ").append(DESC_HARD_TIMEOUT).
                    append('.').toString();
                throw RpcException.getBadArgumentException(msg);
            }
        }
    }

    /**
     * Verify timeout values in the given {@link VtnFlowTimeoutConfig}
     * instance.
     *
     * @param vftc  A {@link VtnFlowTimeoutConfig} instance.
     * @throws RpcException
     *    The given instance contains invalid timeout values.
     */
    public static void verifyFlowTimeout(VtnFlowTimeoutConfig vftc)
        throws RpcException {
        verifyFlowTimeout(vftc.getIdleTimeout(), vftc.getHardTimeout());
    }

    /**
     * Verify the given flow timeout value.
     *
     * @param value  A value which indicates the flow timeout.
     * @param desc   A brief description about the given value.
     * @throws RpcException  The given flow timeout value is invalid.
     */
    private static void verifyFlowTimeout(Integer value, String desc)
        throws RpcException {
        if (value != null) {
            if ((value.intValue() & ~FLOW_TIMEOUT_MASK) != 0) {
                String msg = new StringBuilder("Invalid ").append(desc).
                    append(": ").append(value).toString();
                throw RpcException.getBadArgumentException(msg);
            }
        }
    }
}
