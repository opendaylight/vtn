/*
 * Copyright (c) 2014 NEC Corporation
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

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link SetDlDstAction}.
 */
public class SetDlDstActionTest extends TestBase {
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
            SetDlDstAction newobj = (SetDlDstAction)jaxbTest(act, "setdldst");
            assertEquals(null, newobj.getValidationStatus());
        }

        // Specifying invalid MAC address.
        String xml =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" +
            "<setdldst address=\"invalid_address\" />";
        try {
            ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
            SetDlDstAction act = JAXB.unmarshal(in, SetDlDstAction.class);
            Status st = act.getValidationStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
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
            jsonTest(act);
        }

        // Specifying invalid MAC address.
        ObjectMapper mapper = getJsonObjectMapper();
        try {
            JSONObject json = new JSONObject();
            json.put("address", "invalid_address");
            SetDlDstAction act =
                mapper.readValue(json.toString(), SetDlDstAction.class);
            Status st = act.getValidationStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
