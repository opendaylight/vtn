/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link VTNException}.
 */
public class VTNExceptionTest extends TestBase {
    /**
     * Test case for {@link VTNException#VTNException(VtnErrorTag, String)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     * </ul>
     */
    @Test
    public void testException1() {
        for (String msg: createStrings("Exception")) {
            for (VtnErrorTag etag: VtnErrorTag.values()) {
                VTNException e = new VTNException(etag, msg);
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getVtnErrorTag());
            }

            VTNException e = new VTNException(null, msg);
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
        }
    }

    /**
     * Test case for
     * {@link VTNException#VTNException(VtnErrorTag, String, Throwable)} and
     * the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     * </ul>
     */
    @Test
    public void testException2() {
        Throwable[] causes = {
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
        };

        for (Throwable cause: causes) {
            for (String msg: createStrings("Exception")) {
                for (VtnErrorTag etag: VtnErrorTag.values()) {
                    VTNException e = new VTNException(etag, msg, cause);
                    assertEquals(msg, e.getMessage());
                    assertEquals(cause, e.getCause());
                    assertEquals(etag, e.getVtnErrorTag());
                }

                VTNException e = new VTNException(null, msg, cause);
                assertEquals(msg, e.getMessage());
                assertEquals(cause, e.getCause());
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            }
        }
    }

    /**
     * Test case for {@link VTNException#VTNException(String)}
     * and the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     * </ul>
     */
    @Test
    public void testException3() {
        for (String msg: createStrings("Exception")) {
            VTNException e = new VTNException(msg);
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
        }
    }

    /**
     * Test case for
     * {@link VTNException#VTNException(String, Throwable)} and
     * the followings.
     *
     * <ul>
     *   <li>{@link Throwable#getMessage()}</li>
     *   <li>{@link Throwable#getCause()}</li>
     *   <li>{@link VTNException#getVtnErrorTag()}</li>
     * </ul>
     */
    @Test
    public void testException4() {
        Throwable[] causes = {
            new NullPointerException(),
            new IllegalArgumentException(),
            new IllegalStateException(),
        };

        for (Throwable cause: causes) {
            for (String msg: createStrings("Exception")) {
                VTNException e = new VTNException(msg, cause);
                assertEquals(msg, e.getMessage());
                assertEquals(cause, e.getCause());
                assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            }
        }
    }
}
