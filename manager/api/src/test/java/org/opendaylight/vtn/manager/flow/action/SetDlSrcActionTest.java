/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetDlSrcAction}.
 */
public class SetDlSrcActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetDlSrcAction} class.
     */
    private static final String  XML_ROOT = "setdlsrc";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            EtherAddress ea = (eaddr == null)
                ? null : new EtherAddress(eaddr);
            SetDlSrcAction act = new SetDlSrcAction(bytes);
            byte[] addr = act.getAddress();
            assertTrue(Arrays.equals(bytes, addr));
            assertEquals(null, act.getValidationStatus());
            assertEquals(ea, act.getEtherAddress());

            act = new SetDlSrcAction(ea);
            addr = act.getAddress();
            assertTrue(Arrays.equals(bytes, addr));
            assertEquals(null, act.getValidationStatus());
            assertEquals(ea, act.getEtherAddress());
        }
    }

    /**
     * Test case for {@link SetDlSrcAction#equals(Object)} and
     * {@link SetDlSrcAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<EthernetAddress> eaddrs = createEthernetAddresses();
        for (EthernetAddress eaddr: eaddrs) {
            byte[] bytes1 = (eaddr == null) ? null : eaddr.getValue();
            byte[] bytes2 = (bytes1 == null) ? null : bytes1.clone();
            SetDlSrcAction act1 = new SetDlSrcAction(bytes1);
            SetDlSrcAction act2 = new SetDlSrcAction(bytes2);
            testEquals(set, act1, act2);
        }

        assertEquals(eaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetDlSrcAction#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "SetDlSrcAction[";
        String suffix = "]";
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlSrcAction act = new SetDlSrcAction(bytes);
            byte[] addr = act.getAddress();
            String a = (bytes == null)
                ? null
                : "addr=" + ByteUtils.toHexString(bytes);
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, act.toString());
        }
    }

    /**
     * Ensure that {@link SetDlSrcAction} is serializable.
     */
    @Test
    public void testSerialize() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlSrcAction act = new SetDlSrcAction(bytes);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetDlSrcAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlSrcAction act = new SetDlSrcAction(bytes);
            SetDlSrcAction newobj =
                jaxbTest(act, SetDlSrcAction.class, XML_ROOT);
            assertEquals(null, newobj.getValidationStatus());
        }

        // Specifying invalid MAC address.
        String[] invalidAddrs = {
            "invalid_address",
            "1z:22:33:44:55:66",
            "",
            "123456789012345",
        };

        Unmarshaller um = createUnmarshaller(SetDlSrcAction.class);
        try {
            for (String addr: invalidAddrs) {
                StringBuilder builder = new StringBuilder(XML_DECLARATION);
                String xml = builder.append('<').append(XML_ROOT).
                    append(" address=\"").append(addr).append("\" />").
                    toString();
                SetDlSrcAction act = unmarshal(um, xml, SetDlSrcAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Ensure that {@link SetDlSrcAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlSrcAction act = new SetDlSrcAction(bytes);
            jsonTest(act, SetDlSrcAction.class);
        }

        // Specifying invalid MAC address.
        ObjectMapper mapper = getJsonObjectMapper();
        String[] invalidAddrs = {
            "invalid_address",
            "1z:22:33:44:55:66",
            "",
            "123456789012345",
        };

        try {
            for (String addr: invalidAddrs) {
                JSONObject json = new JSONObject();
                json.put("address", addr);
                SetDlSrcAction act =
                    mapper.readValue(json.toString(), SetDlSrcAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
