/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.concurrent.CancellationException;
import java.util.concurrent.TimeUnit;

import org.junit.Assert;
import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link CanceledFuture}.
 */
public class CanceledFutureTest extends TestBase {

    /**
     * To test cancel(boolean), isCancelled() and isDone() methods in CanceledFuture.
     */
    @Test
    public void testCanceledFutureMethods() {
        CanceledFuture<Void> cancelFuture = new CanceledFuture<Void>();
        Assert.assertEquals(true, cancelFuture.isCancelled());
        Assert.assertEquals(true, cancelFuture.isDone());
        Assert.assertEquals(true, cancelFuture.cancel(true));
        Assert.assertEquals(true, cancelFuture.cancel(false));
    }

    /**
     * To test get method in CanceledFuture.
     */
    @Test
    public void testGet() {
        try {
            CanceledFuture<Void> cancelFuture = new CanceledFuture<Void>();
            cancelFuture.get();
        } catch (CancellationException e) {
        }
    }

    /**
     * To test get method(long, TimeUnit) in CanceledFuture.
     */
    @Test
    public void testGetWithArg() {
        try {
            CanceledFuture<Void> cancelFuture = new CanceledFuture<Void>();
            long timeout = 0;
            TimeUnit unit = null;
            cancelFuture.get(timeout, unit);
        } catch (CancellationException e) {
        }
    }
}
