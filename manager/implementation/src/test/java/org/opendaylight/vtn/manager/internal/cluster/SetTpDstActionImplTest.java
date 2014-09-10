/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetTpDstActionImpl}.
 */
public class SetTpDstActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            SetTpDstActionImpl impl = new SetTpDstActionImpl(act);
            assertEquals(act, impl.getFlowAction());
            assertEquals(port, impl.getPort());
        }

        // null action.
        try {
            new SetTpDstActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Specify invalid ports.
        int[] badPorts = {
            Integer.MIN_VALUE,
            -1000,
            -1,
            65536,
            65537,
            Integer.MAX_VALUE
        };

        for (int port: badPorts) {
            SetTpDstAction act = new SetTpDstAction(port);
            try {
                new SetTpDstActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetTpDstActionImpl#equals(Object)} and
     * {@link SetTpDstActionImpl#hashCode()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpDstAction act1 = new SetTpDstAction(port);
            SetTpDstAction act2 = new SetTpDstAction(port);
            SetTpDstActionImpl impl1 = new SetTpDstActionImpl(act1);
            SetTpDstActionImpl impl2 = new SetTpDstActionImpl(act2);
            testEquals(set, impl1, impl2);
        }

        assertEquals(ports.length, set.size());
    }

    /**
     * Test case for {@link SetTpDstActionImpl#toString()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "SetTpDstActionImpl[";
        String suffix = "]";
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            SetTpDstActionImpl impl = new SetTpDstActionImpl(act);
            String a = "port=" + port;
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, impl.toString());
        }
    }

    /**
     * Ensure that {@link SetTpDstActionImpl} is serializable.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpDstAction act = new SetTpDstAction(port);
            SetTpDstActionImpl impl = new SetTpDstActionImpl(act);
            serializeTest(impl);
        }
    }
}
