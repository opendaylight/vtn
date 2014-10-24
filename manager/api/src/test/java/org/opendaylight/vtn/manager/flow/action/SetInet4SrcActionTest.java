/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.io.ByteArrayInputStream;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.JAXB;

import org.junit.Test;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.action.SetNwSrc;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetInet4SrcAction}.
 */
public class SetInet4SrcActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetInet4SrcAction} class.
     */
    private static final String  XML_ROOT = "setinet4src";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            assertEquals(iaddr, act.getAddress());
            assertEquals(null, act.getValidationStatus());

            if (iaddr != null) {
                SetNwSrc sact = new SetNwSrc(iaddr);
                act = new SetInet4SrcAction(sact);
                assertEquals(iaddr, act.getAddress());
                assertEquals(null, act.getValidationStatus());
            }
        }

        try {
            InetAddress v6addr =
                InetAddress.getByName("2001:420:281:1004:e123:e688:d655:a1b0");
            try {
                new SetInet4SrcAction(v6addr);
                fail("An exception must be thrown.");
            } catch (IllegalArgumentException e) {
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Test case for {@link SetInet4SrcAction#equals(Object)} and
     * {@link SetInet4SrcAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<InetAddress> iaddrs = createInet4Addresses();
        for (InetAddress iaddr: iaddrs) {
            SetInet4SrcAction act1 = new SetInet4SrcAction(iaddr);
            SetInet4SrcAction act2 = new SetInet4SrcAction(copy(iaddr));
            testEquals(set, act1, act2);
        }

        assertEquals(iaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetInet4SrcAction#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "SetInet4SrcAction[";
        String suffix = "]";
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            String a = (iaddr == null)
                ? null
                : "addr=" + iaddr.getHostAddress();
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, act.toString());
        }
    }

    /**
     * Ensure that {@link SetInet4SrcAction} is serializable.
     */
    @Test
    public void testSerialize() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetInet4SrcAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            SetInet4SrcAction newobj =
                (SetInet4SrcAction)jaxbTest(act, XML_ROOT);
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

        try {
            for (String addr: invalid) {
                StringBuilder builder = new StringBuilder(XML_DECLARATION);
                String xml = builder.append('<').append(XML_ROOT).
                    append(" address=\"").append(addr).append("\" />").
                    toString();
                ByteArrayInputStream in =
                    new ByteArrayInputStream(xml.getBytes());
                SetInet4SrcAction act =
                    JAXB.unmarshal(in, SetInet4SrcAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Ensure that {@link SetInet4SrcAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (InetAddress iaddr: createInet4Addresses()) {
            SetInet4SrcAction act = new SetInet4SrcAction(iaddr);
            jsonTest(act);
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
                SetInet4SrcAction act =
                    mapper.readValue(json.toString(), SetInet4SrcAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
