/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeoutException;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.common.api.data.OptimisticLockFailedException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link AbstractVTNFuture}.
 */
public class AbstractVTNFutureTest extends TestBase {
    /**
     * Test case for {@link AbstractVTNFuture#getCause(Throwable)}.
     */
    @Test
    public void testGetCause() {
        Throwable[] causes = {
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
            new AssertionError(),
        };

        for (Throwable cause: causes) {
            assertSame(cause, AbstractVTNFuture.getCause(cause));

            Throwable t = cause;
            for (int i = 0; i < 10; i++) {
                ExecutionException ee = new ExecutionException(t);
                assertSame(cause, AbstractVTNFuture.getCause(ee));
                t = ee;
            }
        }

        ExecutionException cause = new ExecutionException((Throwable)null);
        Throwable t = cause;
        for (int i = 0; i < 10; i++) {
            ExecutionException ee = new ExecutionException(t);
            assertSame(cause, AbstractVTNFuture.getCause(ee));
            t = ee;
        }

        assertEquals(null, AbstractVTNFuture.getCause(null));
    }

    /**
     * Test case for {@link AbstractVTNFuture#getException(Throwable)}.
     */
    @Test
    public void testGetException() {
        getExceptionTest(null, VtnErrorTag.INTERNALERROR,
                         "Failed to wait for the computation.");

        String[] msgs = {null, "error 1", "error 2"};
        for (String msg: msgs) {
            // CONFLICT error.
            if (msg != null) {
                Throwable cause = new OptimisticLockFailedException(msg);
                getExceptionTest(cause, VtnErrorTag.CONFLICT, msg);
            }

            // TIMEOUT error.
            Throwable cause = new TimeoutException(msg);
            getExceptionTest(cause, VtnErrorTag.TIMEOUT, msg);

            // Interrupted.
            cause = new InterruptedException(msg);
            String expected = (msg == null) ? "Interrupted." : msg;
            VtnErrorTag vtag = VtnErrorTag.INTERNALERROR;
            getExceptionTest(cause, vtag, expected);

            // Others.
            cause = new IllegalArgumentException(msg);
            getExceptionTest(cause, vtag, msg);
            cause = new IllegalStateException(msg);
            getExceptionTest(cause, vtag, msg);
            cause = new UnsupportedOperationException(msg);
            getExceptionTest(cause, vtag, msg);
        }
    }

    /**
     * Call {@link AbstractVTNFuture#getException(Throwable)} and
     * verify result.
     *
     * @param cause  A throwable that indicates the cause of error.
     * @param vtag   The expected error tag.
     * @param msg    The expected message.
     */
    private void getExceptionTest(Throwable cause, VtnErrorTag vtag,
                                  String msg) {
        VTNException ve = AbstractVTNFuture.getException(cause);
        assertEquals(vtag, ve.getVtnErrorTag());
        assertEquals(msg, ve.getMessage());
        assertSame(ve, AbstractVTNFuture.getException(ve));

        ExecutionException ee = new ExecutionException(cause);
        ve = AbstractVTNFuture.getException(ee);
        if (cause == null) {
            assertEquals(VtnErrorTag.INTERNALERROR, ve.getVtnErrorTag());
            assertEquals(ee.getMessage(), ve.getMessage());
        } else {
            assertEquals(vtag, ve.getVtnErrorTag());
            assertEquals(msg, ve.getMessage());
        }
        assertSame(ve, AbstractVTNFuture.getException(ve));
    }
}
