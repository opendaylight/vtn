/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.io.ByteArrayInputStream;

import javax.xml.bind.JAXB;

import org.junit.Test;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.controller.sal.action.SetDlDst;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetDlDstAction}.
 */
public class SetDlDstActionTest extends TestBase {
    /**
     * Root XML element name associated with {@link SetDlDstAction} class.
     */
    private static final String  XML_ROOT = "setdldst";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            byte[] addr = act.getAddress();
            assertTrue(Arrays.equals(bytes, addr));
            assertEquals(null, act.getValidationStatus());

            if (bytes != null) {
                SetDlDst sact = new SetDlDst(bytes);
                act = new SetDlDstAction(sact);
                assertTrue(Arrays.equals(bytes, act.getAddress()));
                assertEquals(null, act.getValidationStatus());
            }
        }
    }

    /**
     * Test case for {@link SetDlDstAction#equals(Object)} and
     * {@link SetDlDstAction#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<EthernetAddress> eaddrs = createEthernetAddresses();
        for (EthernetAddress eaddr: eaddrs) {
            byte[] bytes1 = (eaddr == null) ? null : eaddr.getValue();
            byte[] bytes2 = (bytes1 == null) ? null : bytes1.clone();
            SetDlDstAction act1 = new SetDlDstAction(bytes1);
            SetDlDstAction act2 = new SetDlDstAction(bytes2);
            testEquals(set, act1, act2);
        }

        assertEquals(eaddrs.size(), set.size());
    }

    /**
     * Test case for {@link SetDlDstAction#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "SetDlDstAction[";
        String suffix = "]";
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            byte[] addr = act.getAddress();
            String a = (bytes == null)
                ? null
                : "addr=" + HexEncode.bytesToHexStringFormat(bytes);
            String required = joinStrings(prefix, suffix, ",", a);
            assertEquals(required, act.toString());
        }
    }

    /**
     * Ensure that {@link SetDlDstAction} is serializable.
     */
    @Test
    public void testSerialize() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            serializeTest(act);
        }
    }

    /**
     * Ensure that {@link SetDlDstAction} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            SetDlDstAction newobj = (SetDlDstAction)jaxbTest(act, XML_ROOT);
            assertEquals(null, newobj.getValidationStatus());
        }

        // Specifying invalid MAC address.
        String[] invalidAddrs = {
            "invalid_address",
            "1z:22:33:44:55:66",
            "",
            "123456789012345",
        };

        try {
            for (String addr: invalidAddrs) {
                StringBuilder builder = new StringBuilder(XML_DECLARATION);
                String xml = builder.append('<').append(XML_ROOT).
                    append(" address=\"").append(addr).append("\" />").
                    toString();
                ByteArrayInputStream in =
                    new ByteArrayInputStream(xml.getBytes());
                SetDlDstAction act = JAXB.unmarshal(in, SetDlDstAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * Ensure that {@link SetDlDstAction} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            byte[] bytes = (eaddr == null) ? null : eaddr.getValue();
            SetDlDstAction act = new SetDlDstAction(bytes);
            jsonTest(act, SetDlDstAction.class);
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
                SetDlDstAction act =
                    mapper.readValue(json.toString(), SetDlDstAction.class);
                Status st = act.getValidationStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
