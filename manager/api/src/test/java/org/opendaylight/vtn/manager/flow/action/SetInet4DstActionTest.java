/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetInet4DstAction}.
 */
public class SetInet4DstActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetInet4DstAction} class.
     */
    private static final String  XML_ROOT = "setinet4dst";

    /**
     * Test case for getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        for (InetAddress iaddr: createInet4Addresses()) {
            IpNetwork ipn = IpNetwork.create(iaddr);
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            assertEquals(iaddr, act.getAddress());
            assertEquals(ipn, act.getIpNetwork());
            assertEquals(null, act.getValidationStatus());

            act = new SetInet4DstAction(ipn);
            assertEquals(iaddr, act.getAddress());
            assertEquals(ipn, act.getIpNetwork());
            assertEquals(null, act.getValidationStatus());

            if (iaddr != null) {
                for (int prefix = 1; prefix <= 31; prefix++) {
                    IpNetwork bad = IpNetwork.create(iaddr, prefix);
                    try {
                        new SetInet4DstAction(bad);
                        unexpected();
                    } catch (IllegalArgumentException e) {
                        String msg = "SetInet4DstAction: Unexpected address: " +
                            bad.getText();
                        assertEquals(msg, e.getMessage());
                    }
                }
            }
        }

        InetAddress v6addr =
            InetAddress.getByName("2001:420:281:1004:e123:e688:d655:a1b0");
        try {
            new SetInet4DstAction(v6addr);
            unexpected();
        } catch (IllegalArgumentException e) {
            String msg = "SetInet4DstAction: Unexpected address: " + v6addr;
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link SetInet4DstAction#equals(Object)} and
     * {@link SetInet4DstAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<InetAddress> iaddrs = createInet4Addresses();
        for (InetAddress iaddr: iaddrs) {
            SetInet4DstAction act1 = new SetInet4DstAction(iaddr);
            SetInet4DstAction act2 = new SetInet4DstAction(copy(iaddr));
            testEquals(set, act1, act2);
        }

        assertEquals(iaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetInet4DstAction#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "SetInet4DstAction[";
        String suffix = "]";
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            String a = (iaddr == null)
                ? null
                : "addr=" + iaddr.getHostAddress();
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, act.toString());
        }
    }

    /**
     * Ensure that {@link SetInet4DstAction} is serializable.
     */
    @Test
    public void testSerialize() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetInet4DstAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            SetInet4DstAction newobj =
                jaxbTest(act, SetInet4DstAction.class, XML_ROOT);
            assertEquals(null, newobj.getValidationStatus());
        }

        // Specifying invalid address.
        String[] invalid = {
            // Invalid address
            "",
            "  ",
            "invalid_address",
            "100.200.300.400",

            // IPv6 address
            "2001:420:281:1004:e123:e688:d655:a1b0",
            "::1",
        };

        Unmarshaller um = createUnmarshaller(SetInet4DstAction.class);
        try {
            for (String addr: invalid) {
                StringBuilder builder = new StringBuilder(XML_DECLARATION);
                String xml = builder.append('<').append(XML_ROOT).
                    append(" address=\"").append(addr).append("\" />").
                    toString();
                SetInet4DstAction act =
                    unmarshal(um, xml, SetInet4DstAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Ensure that {@link SetInet4DstAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4DstAction act = new SetInet4DstAction(iaddr);
            jsonTest(act, SetInet4DstAction.class);
        }

        // Specifying invalid address.
        ObjectMapper mapper = getJsonObjectMapper();
        String[] invalid = {
            // Invalid address
            "invalid_address",
            "100.200.300.400",

            // IPv6 address
            "2001:420:281:1004:e123:e688:d655:a1b0",
            "::1",
        };

        try {
            for (String addr: invalid) {
                JSONObject json = new JSONObject();
                json.put("address", addr);
                SetInet4DstAction act =
                    mapper.readValue(json.toString(), SetInet4DstAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
