/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetInet4DstActionImpl}.
 */
public class SetInet4DstActionImplTest extends TestBase {
    /**
     * Test case for getter methods.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testGetter() throws Exception {
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstActionImpl impl = new SetInet4DstActionImpl(act);
            assertEquals(act, impl.getFlowAction());
            assertEquals(iaddr, impl.getAddress());
        }

        // null action.
        try {
            new SetInet4DstActionImpl(null);
            unexpected();
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        // Specify invalid IP address via JAXB.
        String[] badAddrs = {
            null,
            "",
            "  ",
            "::1",
            "Bad IP address",
        };

        Unmarshaller um = createUnmarshaller(SetInet4DstAction.class);
        for (String addr: badAddrs) {
            StringBuilder builder = new StringBuilder(XML_DECLARATION);
            if (addr == null) {
                builder.append("<setinet4dst />");
            } else {
                builder.append("<setinet4dst address=\"").append(addr).
                    append("\" />");
            }
            String xml = builder.toString();
            SetInet4DstAction act =
                unmarshal(um, xml, SetInet4DstAction.class);
            try {
                new SetInet4DstActionImpl(act);
                unexpected();
            } catch (VTNException e) {
                assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
            }
        }
    }

    /**
     * Test case for {@link SetInet4DstActionImpl#equals(Object)} and
     * {@link SetInet4DstActionImpl#hashCode()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        List<InetAddress> iaddrs = createInet4Addresses(false);
        for (InetAddress iaddr: iaddrs) {
            byte[] raw = iaddr.getAddress();
            InetAddress iaddr2 = InetAddress.getByAddress(raw);
            SetInet4DstAction act1 = new SetInet4DstAction(iaddr);
            SetInet4DstAction act2 = new SetInet4DstAction(iaddr2);
            SetInet4DstActionImpl impl1 = new SetInet4DstActionImpl(act1);
            SetInet4DstActionImpl impl2 = new SetInet4DstActionImpl(act2);
            testEquals(set, impl1, impl2);
        }

        assertEquals(iaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetInet4DstActionImpl#toString()}.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testToString() throws Exception {
        String prefix = "SetInet4DstActionImpl[";
        String suffix = "]";
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstActionImpl impl = new SetInet4DstActionImpl(act);
            String a = "addr=" + iaddr.getHostAddress();
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, impl.toString());
        }
    }

    /**
     * Ensure that {@link SetInet4DstActionImpl} is serializable.
     *
     * @throws Exception  An unexpected exception occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        for (InetAddress iaddr: createInet4Addresses(false)) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstActionImpl impl = new SetInet4DstActionImpl(act);
            serializeTest(impl);
        }
    }
}
