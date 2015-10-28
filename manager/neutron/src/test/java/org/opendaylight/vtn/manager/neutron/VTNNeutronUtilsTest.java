/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;
import org.junit.Test;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;


/**
 * JUnit test for {@link VTNNeutronUtils}
 */
public class VTNNeutronUtilsTest extends TestBase {

    /**
     * Test method for
     * {@link VTNNeutronUtils#getException(Status)}.
     */
    @Test
    public void testGetException() {
        Status status = new Status(StatusCode.BADREQUEST);
        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     VTNNeutronUtils.getException(status));

        status = new Status(StatusCode.CONFLICT);
        assertEquals(HttpURLConnection.HTTP_CONFLICT,
                     VTNNeutronUtils.getException(status));

        status = new Status(StatusCode.NOTACCEPTABLE);
        assertEquals(HttpURLConnection.HTTP_NOT_ACCEPTABLE,
                     VTNNeutronUtils.getException(status));

        status = new Status(StatusCode.NOTFOUND);
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     VTNNeutronUtils.getException(status));

        status = new Status(StatusCode.INTERNALERROR);
        assertEquals(HttpURLConnection.HTTP_INTERNAL_ERROR,
                     VTNNeutronUtils.getException(status));
    }

    /**
     * Test method for
     * {@link VTNNeutronUtils#isValidNeutronID(String)}.
     */
    @Test
    public void testIsValidNeutronID() {
        assertFalse(VTNNeutronUtils.isValidNeutronID(null));

        String invalidId = "";
        assertFalse(VTNNeutronUtils.isValidNeutronID(invalidId));

        // The length is 37.
        invalidId = "X550e8400-e29b-41d4-a716-446655440000";
        assertFalse(VTNNeutronUtils.isValidNeutronID(invalidId));

        // The length is 35.
        invalidId = "50e8400-e29b-41d4-a716-446655440000";
        assertFalse(VTNNeutronUtils.isValidNeutronID(invalidId));

        // The length is 33.
        invalidId = "550e8400-e29b41d4a716446655440000";
        assertFalse(VTNNeutronUtils.isValidNeutronID(invalidId));

        // The length is 36 but invalid UUID format for IllegalArgumentException.
        invalidId = "550e8400-e29b-41d4--716-446655440000";
        assertFalse(VTNNeutronUtils.isValidNeutronID(invalidId));

        for (String neutronId: NEUTRON_UUID_ARRAY) {
            assertTrue(VTNNeutronUtils.isValidNeutronID(neutronId));
        }

        for (String tenantId: TENANT_ID_ARRAY) {
            assertTrue(VTNNeutronUtils.isValidNeutronID(tenantId));
        }
    }

    /**
     * Test method for
     * {@link VTNNeutronUtils#convertNeutronIDToVTNKey(String)}.
     */
    @Test
    public void testConvertNeutronIDToVTNKey() {
        assertNull(VTNNeutronUtils.convertNeutronIDToVTNKey(null));

        String invalidId = "";
        assertNull(VTNNeutronUtils.convertNeutronIDToVTNKey(invalidId));

        // The length is 37.
        invalidId = "X550e8400-e29b-41d4-a716-446655440000";
        assertNull(VTNNeutronUtils.convertNeutronIDToVTNKey(invalidId));

        // The length is 35.
        invalidId = "50e8400-e29b-41d4-a716-446655440000";
        assertNull(VTNNeutronUtils.convertNeutronIDToVTNKey(invalidId));

        // The length is 33.
        invalidId = "550e8400-e29b41d4a716446655440000";
        assertNull(VTNNeutronUtils.convertNeutronIDToVTNKey(invalidId));

        // The length is 32 but invalid UUID format for IllegalArgumentException.
        invalidId = "550e8400e29b-1d4a716446655440000";
        assertNull(VTNNeutronUtils.convertNeutronIDToVTNKey(invalidId));

        String validId = "550e8400-e29b-41d4-a716-446655440000";
        assertEquals("550e8400e29b1d4a716446655440000",
                     VTNNeutronUtils.convertNeutronIDToVTNKey(validId));

        validId = "550e8400e29b41d4a716446655440000";
        assertEquals("550e8400e29b1d4a716446655440000",
                     VTNNeutronUtils.convertNeutronIDToVTNKey(validId));
    }

    /**
     * Test method for
     * {@link VTNNeutronUtils#isTenantExist(String)}.
     */
    @Test
    public void testIsTenantExist() {
        VTNNeutronUtils utils = new VTNNeutronUtils();
        utils.setVTNManager(new VTNManagerStub());

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isTenantExist(null));

        String tenantID = "a";
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     utils.isTenantExist(tenantID));

        tenantID = VTNManagerStub.TENANT_1_UUID;
        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isTenantExist(tenantID));

        tenantID = VTNManagerStub.TENANT_1_NAME;
        assertEquals(HttpURLConnection.HTTP_OK,
                     utils.isTenantExist(tenantID));
    }

    /**
     * Test method for
     * {@link VTNNeutronUtils#isBridgeExist(String, String)}.
     */
    @Test
    public void testIsBridgeExist() {
        VTNNeutronUtils utils = new VTNNeutronUtils();
        utils.setVTNManager(new VTNManagerStub());

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeExist(null, null));

        String tenantID = "a";
        String bridgeID = "a";

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeExist(tenantID, null));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     utils.isBridgeExist(tenantID, bridgeID));

        tenantID = VTNManagerStub.TENANT_1_NAME;
        bridgeID = "b";
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     utils.isBridgeExist(tenantID, bridgeID));

        tenantID = "c";
        bridgeID = VTNManagerStub.BRIDGE_1_NAME;
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     utils.isBridgeExist(tenantID, bridgeID));

        tenantID = VTNManagerStub.TENANT_1_UUID;
        bridgeID = VTNManagerStub.BRIDGE_1_NAME;
        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeExist(tenantID, bridgeID));

        tenantID = VTNManagerStub.TENANT_1_NAME;
        bridgeID = VTNManagerStub.BRIDGE_1_NAME;
        assertEquals(HttpURLConnection.HTTP_OK,
                     utils.isBridgeExist(tenantID, bridgeID));
    }

    /**
     * Test method for
     * {@link VTNNeutronUtils#isBridgeInterfaceExist(String, String, String)}.
     */
    @Test
    public void testIsBridgeInterfaceExist() {
        VTNNeutronUtils utils = new VTNNeutronUtils();
        utils.setVTNManager(new VTNManagerStub());

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeInterfaceExist(null, null, null));

        String tenantID = "a";
        String bridgeID = "a";
        String portID = "a";

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeInterfaceExist(tenantID, null, null));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeInterfaceExist(tenantID, bridgeID, null));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     utils.isBridgeInterfaceExist(tenantID, bridgeID, portID));

        tenantID = VTNManagerStub.TENANT_1_NAME;
        bridgeID = "a";
        portID = "a";
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     utils.isBridgeInterfaceExist(tenantID, bridgeID, portID));

        tenantID = VTNManagerStub.TENANT_1_NAME;
        bridgeID = VTNManagerStub.BRIDGE_1_NAME;
        portID = VTNManagerStub.VBR_IF_3_NAME;
        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     utils.isBridgeInterfaceExist(tenantID, bridgeID, portID));

        tenantID = VTNManagerStub.TENANT_1_NAME;
        bridgeID = VTNManagerStub.BRIDGE_1_NAME;
        portID = VTNManagerStub.VBR_IF_1_NAME;
        assertEquals(HttpURLConnection.HTTP_OK,
                     utils.isBridgeInterfaceExist(tenantID, bridgeID, portID));
    }
}
