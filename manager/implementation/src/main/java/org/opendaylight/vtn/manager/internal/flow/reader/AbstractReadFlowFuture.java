/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.reader;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code AbstractReadFlowFuture} is a base class for future associated with
 * the read operation which fetches data flow information.
 *
 * @param <T>  The type of the result of this future.
 */
public abstract class AbstractReadFlowFuture<T> extends SettableVTNFuture<T> {
    /**
     * MD-SAL transaction context.
     */
    private final TxContext  context;

    /**
     * The name of the target VTN.
     */
    private final VnodeName  tenantName;

    /**
     * Construct a new instance.
     *
     * @param ctx    A {@link TxContext} instance.
     * @param tname  The name of the target VTN.
     * @throws RpcException  An error occurred.
     */
    protected AbstractReadFlowFuture(TxContext ctx, String tname)
        throws RpcException {
        context = ctx;

        // Verify the name of the VTN.
        tenantName = VTenantUtils.getVnodeName(tname);
    }

    /**
     * Return the context for the MD-SAL datastore transaction.
     *
     * @return  A {@link TxContext} instance.
     */
    protected final TxContext getTxContext() {
        return context;
    }

    /**
     * Return the name of the target VTN.
     *
     * @return  The name of the target VTN.
     */
    protected final String getTenantName() {
        return tenantName.getValue();
    }

    /**
     * Ensure that the target VTN is present.
     *
     * <p>
     *   This method set an exception as the result of this future
     *   if {@code false} is returned.
     * </p>
     *
     * @return  {@code true} if the target VTN is present.
     *          {@code false} otherwise.
     */
    protected final boolean checkTenant() {
        // Ensure that the target VTN is present.
        try {
            VTenantUtils.readVtn(context.getTransaction(), tenantName);
        } catch (Exception e) {
            setFailure(e);
            return false;
        }

        return true;
    }
    /**
     * Set the result of this future.
     *
     * @param result  The result of this future.
     */
    protected final void setResult(T result) {
        set(result);
        context.cancelTransaction();
    }

    /**
     * Set the cause of failure.
     *
     * @param cause  A {@link Throwable} that indicates the cause of failure.
     */
    protected final void setFailure(Throwable cause) {
        setException(cause);
        context.cancelTransaction();
    }
}
