/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.rpc;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link RpcException}.
 */
public class RpcExceptionTest extends TestBase {
    /**
     * Test case for {@link RpcException#getNullArgumentException(String)}.
     */
    @Test
    public void testGetNullArgumentException() {
        String desc = "Virtual node";
        RpcException e = RpcException.getNullArgumentException(desc);
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(desc + " cannot be null", e.getMessage());
        assertEquals(null, e.getCause());
    }

    /**
     * Test case for {@link RpcException#getBadArgumentException(String)} and
     * {@link RpcException#getBadArgumentException(String, Throwable)}.
     */
    @Test
    public void testGetBadArgumentException() {
        String desc = "Bad argument.";
        RpcException e = RpcException.getBadArgumentException(desc);
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(null, e.getCause());

        NullPointerException cause = new NullPointerException();
        e = RpcException.getBadArgumentException(desc, cause);
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(cause, e.getCause());
    }

    /**
     * Test case for {@link RpcException#getMissingArgumentException(String)}.
     */
    @Test
    public void testGetMissingArgumentException() {
        String desc = "Missing argument.";
        RpcException e = RpcException.getMissingArgumentException(desc);
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(null, e.getCause());
    }

    /**
     * Test case for {@link RpcException#getNotFoundException(String)} and
     * {@link RpcException#getNotFoundException(String, Throwable)}.
     */
    @Test
    public void testGetNotFoundException() {
        String desc = "Resource not found.";
        RpcException e = RpcException.getNotFoundException(desc);
        assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(null, e.getCause());

        NullPointerException cause = new NullPointerException();
        e = RpcException.getNotFoundException(desc, cause);
        assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(cause, e.getCause());
    }

    /**
     * Test case for {@link RpcException#getDataExistsException(String)} and
     * {@link RpcException#getDataExistsException(String, Throwable)}.
     */
    @Test
    public void testGetDataExistsException() {
        String desc = "Resource already exists.";
        RpcException e = RpcException.getDataExistsException(desc);
        assertEquals(VtnErrorTag.CONFLICT, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.DATA_EXISTS, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(null, e.getCause());

        NullPointerException cause = new NullPointerException();
        e = RpcException.getDataExistsException(desc, cause);
        assertEquals(VtnErrorTag.CONFLICT, e.getVtnErrorTag());
        assertEquals(RpcErrorTag.DATA_EXISTS, e.getErrorTag());
        assertEquals(desc, e.getMessage());
        assertEquals(cause, e.getCause());
    }

    /**
     * Test case for {@link RpcException#getErrorTag()}.
     */
    @Test
    public void testGetErrorTag() {
        VtnErrorTag etag = VtnErrorTag.NOSERVICE;
        for (RpcErrorTag tag: RpcErrorTag.values()) {
            RpcException e = new RpcException(tag, etag, null);
            assertEquals(tag, e.getErrorTag());
            assertEquals(etag, e.getVtnErrorTag());
            assertEquals(null, e.getMessage());
            assertEquals(null, e.getCause());
        }

        RpcException e = new RpcException(null, etag, null);
        assertEquals(RpcErrorTag.OPERATION_FAILED, e.getErrorTag());
        assertEquals(etag, e.getVtnErrorTag());
        assertEquals(null, e.getMessage());
        assertEquals(null, e.getCause());

        IllegalStateException cause = new IllegalStateException();
        String msg = "Internal error.";
        e = new RpcException(msg, cause);
        assertEquals(RpcErrorTag.OPERATION_FAILED, e.getErrorTag());
        assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
        assertEquals(msg, e.getMessage());
        assertEquals(cause, e.getCause());
    }
}
