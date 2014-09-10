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
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetTpSrcActionImpl}.
 */
public class SetTpSrcActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl = new SetTpSrcActionImpl(act);
            assertEquals(act, impl.getFlowAction());
            assertEquals(port, impl.getPort());
        }

        // null action.
        try {
            new SetTpSrcActionImpl(null);
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
            SetTpSrcAction act = new SetTpSrcAction(port);
            try {
                new SetTpSrcActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetTpSrcActionImpl#equals(Object)} and
     * {@link SetTpSrcActionImpl#hashCode()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act1 = new SetTpSrcAction(port);
            SetTpSrcAction act2 = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl1 = new SetTpSrcActionImpl(act1);
            SetTpSrcActionImpl impl2 = new SetTpSrcActionImpl(act2);
            testEquals(set, impl1, impl2);
        }

        assertEquals(ports.length, set.size());
    }

    /**
     * Test case for {@link SetTpSrcActionImpl#toString()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "SetTpSrcActionImpl[";
        String suffix = "]";
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl = new SetTpSrcActionImpl(act);
            String a = "port=" + port;
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, impl.toString());
        }
    }

    /**
     * Ensure that {@link SetTpSrcActionImpl} is serializable.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        int[] ports = {0, 1, 50, 100, 2000, 30000, 40000, 50000, 65535};
        for (int port: ports) {
            SetTpSrcAction act = new SetTpSrcAction(port);
            SetTpSrcActionImpl impl = new SetTpSrcActionImpl(act);
            serializeTest(impl);
        }
    }
}
