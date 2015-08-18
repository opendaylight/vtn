/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.junit.Test;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link VTNException}.
 */
public class VTNExceptionTest extends TestBase {

    /**
     * Test method for
     * {@link VTNException#VTNException(Status)},
     * {@link VTNException#VTNException(StatusCode, String)},
     * {@link VTNException#VTNException(Status, Throwable)},
     * {@link VTNException#getStatus()},
     * {@link Throwable#getMessage()},
     * {@link Throwable#getCause()}.
     */
    @Test
    public void testGetter() {
        for (StatusCode sc : StatusCode.values()) {
            for (String msg : createStrings("Exception")) {
                String emsg = "StatusCode=" + sc.toString()
                        + ",msg=" + msg;

                // test for a object created
                // by VTNException(String)
                Status st = new Status(sc, msg);
                VTNException e = new VTNException(copy(st));
                assertEquals(emsg, null, e.getCause());
                assertEquals(emsg, st.toString(), e.getMessage());
                assertEquals(emsg, st, e.getStatus());

                // test for a object created
                // by VTNException(StatusCode, String)
                e = new VTNException(sc, copy(msg));
                assertEquals(emsg, null, e.getCause());
                assertEquals(emsg, st.toString(), e.getMessage());
                assertEquals(emsg, st, e.getStatus());

                // test for a object created
                // by VTNException(String, Throwable)
                st = new Status(StatusCode.INTERNALERROR, copy(msg));
                VTNException et = new VTNException(sc, copy(msg));
                e = new VTNException(copy(msg), et);
                assertEquals(emsg, et, e.getCause());
                assertEquals(emsg, st.toString(), e.getMessage());
                assertEquals(emsg,
                        new Status(StatusCode.INTERNALERROR, copy(msg)),
                        e.getStatus());
            }
        }

        // in case null is specified.
        VTNException e = new VTNException((Status)null);
        assertEquals(null, e.getCause());
        assertEquals(null, e.getMessage());
        assertEquals(null, e.getStatus());

        VTNException et = new VTNException(StatusCode.INTERNALERROR, "message");
        e = new VTNException((Status)null, et);
        assertEquals(et, e.getCause());
        assertEquals(null, e.getMessage());
        assertEquals(null, e.getStatus());

        e = new VTNException((String)null);
        Status st = new Status(StatusCode.INTERNALERROR, null);
        assertEquals(null, e.getCause());
        assertEquals(st.toString(), e.getMessage());
        assertEquals(st, e.getStatus());

        String msg = "This is an error message.";
        e = new VTNException(msg);
        st = new Status(StatusCode.INTERNALERROR, msg);
        assertEquals(null, e.getCause());
        assertEquals(st.toString(), e.getMessage());
        assertEquals(st, e.getStatus());
    }

    /**
     * Test method for {@link Throwable#toString()}.
     */
    @Test
    public void testToString() {
        for (StatusCode sc : StatusCode.values()) {
            for (String msg : createStrings("Exception")) {
                String emsg = sc.toString() + "," + msg;

                // test for a object created by VTNException(String)
                Status st = new Status(sc, msg);
                VTNException e = new VTNException(copy(st));
                String expected = e.getClass().getName() + ": " + st.toString();
                assertEquals(emsg, expected, e.toString());

                // test for a object created by VTNException(StatusCode, String)
                e = new VTNException(sc, copy(msg));
                assertEquals(emsg, expected, e.toString());

                // test for a object created by VTNException(String, Throwable)
                st = new Status(StatusCode.INTERNALERROR, msg);
                expected = e.getClass().getName() + ": " + st.toString();
                VTNException et = new VTNException(sc, copy(msg));
                e = new VTNException(copy(msg), et);
                assertEquals(emsg, expected, e.toString());
            }
        }
    }

}
