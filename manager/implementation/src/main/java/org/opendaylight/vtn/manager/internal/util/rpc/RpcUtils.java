/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import java.util.Collections;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * {@code RpcUtils} class is a collection of utility class methods for
 * RPC implementation.
 */
public final class RpcUtils {
    /**
     * A map that represents the rule for conversion from {@link StatusCode}
     * to {@link RpcErrorTag}.
     */
    private static final Map<StatusCode, RpcErrorTag>  RPC_ERROR_TAGS;

    /**
     * Initialize static fields.
     */
    static {
        Map<StatusCode, RpcErrorTag> map = new HashMap<>();
        map.put(StatusCode.BADREQUEST, RpcErrorTag.INVALID_VALUE);
        map.put(StatusCode.NOTFOUND, RpcErrorTag.DATA_MISSING);
        map.put(StatusCode.CONFLICT, RpcErrorTag.DATA_EXISTS);
        RPC_ERROR_TAGS = Collections.unmodifiableMap(map);
    }

    /**
     * Private constructor that protects this class from instantiating.
     */
    private RpcUtils() {}

    /**
     * Return a new {@link RpcException} that indicates the RPC request has
     * failed due to null RPC input.
     *
     * @return  An {@link RpcException} instance.
     */
    public static RpcException getNullInputException() {
        Status st = new Status(StatusCode.BADREQUEST,
                               "RPC input cannot be null");
        return new RpcException(RpcErrorTag.MISSING_ELEMENT, st);
    }

    /**
     * Return a new {@link RpcException} that indicates the RPC request has
     * failed due to invalid {@link VtnUpdateOperationType} value.
     *
     * @param op  A {@link VtnUpdateOperationType} value.
     * @return  An {@link RpcException} instance.
     */
    public static RpcException getInvalidOperationException(
        VtnUpdateOperationType op) {
        return RpcException.getBadArgumentException(
            "Invalid operation type: " + op);
    }

    /**
     * Construct a new RPC error builder.
     *
     * @param type    A {@link Class} that indicates the type of RPC output.
     * @param tag     An {@link RpcErrorTag} instance that indicates an error
     *                tag.
     * @param msg     An error message.
     * @param vtnTag  A {@link VtnErrorTag} instance to be set as
     *                application-specific tag.
     * @param info    An additional debugging information.
     * @param <T>     The type of RPC output.
     * @return  A {@link RpcResultBuilder} instance.
     */
    public static <T> RpcResultBuilder<T> getErrorBuilder(
        Class<T> type, RpcErrorTag tag, String msg, VtnErrorTag vtnTag,
        String info) {
        RpcResultBuilder<T> builder = RpcResultBuilder.<T>failed().
            withError(ErrorType.APPLICATION, tag.toString(), msg,
                      vtnTag.toString(), info, null);
        return builder;
    }

    /**
     * Construct a new RPC error builder from the given {@link Exception}.
     *
     * @param type  A {@link Class} that indicates the type of RPC output.
     * @param e     An {@link Exception} instance.
     * @param <T>   The type of RPC output.
     * @return  A {@link RpcResultBuilder} instance.
     */
    public static <T> RpcResultBuilder<T> getErrorBuilder(Class<T> type,
                                                          Exception e) {
        RpcErrorTag tag = null;
        String msg = null;
        VtnErrorTag vtnTag = null;

        if (e instanceof VTNException) {
            VTNException ve = (VTNException)e;
            Status st = ve.getStatus();
            StatusCode code = st.getCode();
            msg = st.getDescription();
            vtnTag = toVtnErrorTag(code);
            if (e instanceof RpcException) {
                RpcException re = (RpcException)e;
                tag = re.getErrorTag();
            } else {
                tag = RPC_ERROR_TAGS.get(code);
            }
        } else {
            msg = "Caught unexpected exception: " + e;
            vtnTag = VtnErrorTag.INTERNALERROR;
            Logger logger = LoggerFactory.getLogger(RpcUtils.class);
            logger.error(msg, e);
        }

        if (tag == null) {
            tag = RpcErrorTag.OPERATION_FAILED;
        }

        return getErrorBuilder(type, tag, msg, vtnTag, null);
    }

    /**
     * Convert the given {@link StatusCode} into a {@link VtnErrorTag}.
     *
     * @param code  A {@link StatusCode}.
     * @return  A {@link VtnErrorTag} instance associated with {@code code}.
     */
    public static VtnErrorTag toVtnErrorTag(StatusCode code) {
        try {
            return VtnErrorTag.valueOf(code.name());
        } catch (Exception e) {
            return VtnErrorTag.INTERNALERROR;
        }
    }

    /**
     * Check the given RPC result.
     *
     * @param result  An {@link RpcResult} instance.
     * @param desc    A brief description about the RPC.
     * @param logger  A {@link Logger} instance.
     * @param <T>     The type of the value returned by the RPC.
     * @return  The value returned by the RPC.
     * @throws VTNException  The RPC call was failed.
     */
    public static <T> T checkResult(RpcResult<T> result, String desc,
                                    Logger logger) throws VTNException {
        if (result == null) {
            String msg = "RPC did not return the result";
            logger.error("{}: {}", desc, msg);
            throw new VTNException(msg);
        }

        if (result.isSuccessful()) {
            // Successfully completed.
            return result.getResult();
        }

        // Determine the cause of failure.
        Collection<RpcError> errors = result.getErrors();
        Throwable cause = null;
        if (errors != null) {
            for (RpcError re: errors) {
                Throwable t = re.getCause();
                if (t != null) {
                    cause = t;
                    break;
                }
            }
        }

        String msg = "RPC returned error";
        if (cause == null) {
            logger.error("{}: {}: errors={}", desc, msg, errors);
        } else {
            StringBuilder builder = new StringBuilder(desc).
                append(": ").append(msg).append(": errors=").append(errors);
            logger.error(builder.toString(), cause);
        }

        throw new VTNException(msg);
    }
}
