/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import java.util.Timer;
import org.junit.Test;
import org.opendaylight.vtn.manager.internal.TestBase;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.SettableFuture;

/**
 * JUnit test for {@link FutureCanceller}.
 */
public class FutureCancellerTest extends TestBase {

    /**
     * Instance of ListenableFuture to perform unit testing.
     */
    ListenableFuture listenableFuture;

    /**
     * Instance of Timer to perform unit testing.
     */
    Timer timer = new Timer();

    /**
     * Instance of FutureCanceller to perform unit testing.
     */
    FutureCanceller futureCanceller;


    /**
     * Test method for
     * {@link FutureCanceller#set(Timer,long,ListenableFuture)} and
     * {@link FutureCanceller#set(Timer,long,ListenableFuture,boolean)}.
     */
    @Test
    public void testSet() {
        listenableFuture = SettableFuture.create();
        FutureCanceller.set(timer, 10L, listenableFuture);
        FutureCanceller.set(timer, 0, listenableFuture);
        FutureCanceller.set(timer, 10L, listenableFuture, true);
        FutureCanceller.set(timer, 10L, listenableFuture, false);

    }
}
