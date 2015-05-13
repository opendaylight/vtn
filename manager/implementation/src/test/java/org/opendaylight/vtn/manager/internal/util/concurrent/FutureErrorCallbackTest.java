/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.concurrent;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.junit.Test;

/**
 * JUnit test for {@link FutureErrorCallback}
 */
public class FutureErrorCallbackTest {

    /**
     * Test method for all the methods in FutureErrorCallback.
     */
    @Test
    public void testFutureErrorCallback() {
        Object[] argument = null;
        Logger logger = LoggerFactory.getLogger(FutureErrorCallback.class);
        String format = "Fox";
        FutureErrorCallback futureErrorCallback = new FutureErrorCallback(logger, format, argument);

        // Sending throwable as null object
        try {
            futureErrorCallback.onFailure(null);
        } catch (Exception exception) {
        }

        // With dummy exception
        format = String.format("%s dummy %s", "This is", "exception");
        Object[] arguments = {"Club", "Hearts"};
        futureErrorCallback = new FutureErrorCallback(logger, format, arguments);

        Throwable t = new Exception("Dummy exception");
        futureErrorCallback.onFailure(t);

        // onSuccess method
        futureErrorCallback.onSuccess(null);
    }
}
