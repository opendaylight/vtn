/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;

import org.junit.Test;

import org.opendaylight.controller.networkconfig.neutron.NeutronNetwork;

import org.opendaylight.vtn.manager.IVTNManager;

/**
 * JUnit test for {@link NetworkHandler}
 */
public class NetworkHandlerTest extends TestBase {
    /**
     * Test method for
     * {@link NetworkHandler#canCreateNetwork(NeutronNetwork)}.
     */
    @Test
    public void testCanCreateNetwork() {
        NetworkHandler nh = new NetworkHandler();
        IVTNManager mgr = new VTNManagerStub();
        nh.setVTNManager(mgr);

        // In the cases that the method returns HTTP_CREATED.
        NeutronNetwork network = new NeutronNetwork();
        network.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        network.setNetworkName("network1");
        network.setTenantID(TENANT_ID_ARRAY[0]);
        assertEquals(HttpURLConnection.HTTP_CREATED,
                     nh.canCreateNetwork(network));

        network = new NeutronNetwork();
        network.setNetworkUUID(NEUTRON_UUID_ARRAY[1]);
        network.setNetworkName("network2");
        network.setTenantID(TENANT_ID_ARRAY[1]);
        network.setProviderNetworkType("vlan");
        network.setProviderSegmentationID("100");
        assertEquals(HttpURLConnection.HTTP_CREATED,
                     nh.canCreateNetwork(network));

        // In the case that user specify segmentation ID
        // without network type.
        network = new NeutronNetwork();
        network.setNetworkUUID(NEUTRON_UUID_ARRAY[2]);
        network.setNetworkName("network3");
        network.setTenantID(TENANT_ID_ARRAY[1]);
        network.setProviderSegmentationID("300");
        assertEquals(HttpURLConnection.HTTP_CREATED,
                     nh.canCreateNetwork(network));

        // In the cases that the method returns HTTP_NOT_ACCEPTABLE.
        network = new NeutronNetwork();
        network.setNetworkUUID(NEUTRON_UUID_ARRAY[3]);
        network.setTenantID(TENANT_ID_ARRAY[3]);
        network.setShared(true);
        assertEquals(HttpURLConnection.HTTP_NOT_ACCEPTABLE,
                     nh.canCreateNetwork(network));

        // In the cases that the method returns HTTP_CONFLICT.
        network = new NeutronNetwork();
        network.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        network.setTenantID(VTNManagerStub.TENANT_1_UUID);
        assertEquals(HttpURLConnection.HTTP_CONFLICT,
                     nh.canCreateNetwork(network));
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkCreated(NeutronNetwork)}.
     */
    @Test
    public void testNeutronNetworkCreated() {
        NetworkHandler nh = new NetworkHandler();
        IVTNManager mgr = new VTNManagerStub();
        nh.setVTNManager(mgr);

        NeutronNetwork network1 = new NeutronNetwork();
        network1.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        network1.setNetworkName("network1");
        network1.setTenantID(TENANT_ID_ARRAY[0]);
        nh.neutronNetworkCreated(network1);

        NeutronNetwork network2 = new NeutronNetwork();
        network2.setNetworkUUID(NEUTRON_UUID_ARRAY[1]);
        network2.setNetworkName("network2");
        network2.setTenantID(TENANT_ID_ARRAY[1]);
        network2.setProviderNetworkType("vlan");
        network2.setProviderSegmentationID("100");
        nh.neutronNetworkCreated(network2);
    }

    /**
     * Test method for
     * {@link NetworkHandler#canUpdateNetwork(NeutronNetwork, NeutronNetwork)}.
     */
    @Test
    public void testCanUpdateNetwork() {
        NetworkHandler nh = new NetworkHandler();
        IVTNManager mgr = new VTNManagerStub();
        nh.setVTNManager(mgr);

        // In the cases that the method returns HTTP_BAD_REQUEST.
        NeutronNetwork delta = null;
        NeutronNetwork original = null;
        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     nh.canUpdateNetwork(delta, original));

        delta = new NeutronNetwork();
        delta.setNetworkName("network1 updated");
        original = null;
        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                     nh.canUpdateNetwork(delta, original));

        // In the cases that the method returns HTTP_NOT_FOUND.
        delta = new NeutronNetwork();
        delta.setNetworkName("network1 updated");
        original = new NeutronNetwork();
        original.setTenantID(VTNManagerStub.TENANT_1_UUID);
        original.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        original.setNetworkName("network1");
        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     nh.canUpdateNetwork(delta, original));

        // In the cases that the method returns HTTP_NOT_ACCEPTABLE.
        delta = new NeutronNetwork();
        delta.setNetworkName("network1 updated");
        delta.setShared(true);
        original = new NeutronNetwork();
        original.setTenantID(VTNManagerStub.TENANT_1_UUID);
        original.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        original.setNetworkName("network1");
        assertEquals(HttpURLConnection.HTTP_NOT_ACCEPTABLE,
                     nh.canUpdateNetwork(delta, original));

        // In the cases that the method returns HTTP_OK.
        delta = new NeutronNetwork();
        delta.setNetworkName("network1 updated");
        original = new NeutronNetwork();
        original.setTenantID(VTNManagerStub.TENANT_1_UUID);
        original.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        original.setNetworkName("network1");
        assertEquals(HttpURLConnection.HTTP_OK,
                     nh.canUpdateNetwork(delta, original));

        delta = new NeutronNetwork();
        delta.setNetworkName("network1 updated");
        original = new NeutronNetwork();
        original.setTenantID(VTNManagerStub.TENANT_1_UUID);
        original.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        original.setNetworkName("network1");
        original.setProviderNetworkType("vlan");
        original.setProviderSegmentationID("1000");
        assertEquals(HttpURLConnection.HTTP_OK,
                     nh.canUpdateNetwork(delta, original));

        delta = new NeutronNetwork();
        delta.setProviderNetworkType("vlan");
        delta.setProviderSegmentationID("999");
        original = new NeutronNetwork();
        original.setTenantID(VTNManagerStub.TENANT_1_UUID);
        original.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        assertEquals(HttpURLConnection.HTTP_OK,
                     nh.canUpdateNetwork(delta, original));

        delta = new NeutronNetwork();
        delta.setProviderNetworkType("vlan");
        delta.setProviderSegmentationID("4066");
        original = new NeutronNetwork();
        original.setNetworkName("network1");
        original.setTenantID(VTNManagerStub.TENANT_1_UUID);
        original.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        assertEquals(HttpURLConnection.HTTP_OK,
                     nh.canUpdateNetwork(delta, original));
    }

    /**
     * Test method for
     * {@link NetworkHandler#neutronNetworkUpdated(NeutronNetwork)}.
     */
    @Test
    public void testNeutronNetworkUpdated() {
        NetworkHandler nh = new NetworkHandler();
        IVTNManager mgr = new VTNManagerStub();
        nh.setVTNManager(mgr);

        // In the cases that the method returns HTTP_OK.
        NeutronNetwork network = new NeutronNetwork();
        network.setTenantID(VTNManagerStub.TENANT_1_UUID);
        network.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        network.setNetworkName("network1 updated");
        nh.neutronNetworkUpdated(network);

        network = new NeutronNetwork();
        network.setTenantID(VTNManagerStub.TENANT_1_UUID);
        network.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        network.setNetworkName("network1 updated");
        network.setProviderNetworkType("vlan");
        network.setProviderSegmentationID("1000");
        nh.neutronNetworkUpdated(network);

        network = new NeutronNetwork();
        network.setTenantID(VTNManagerStub.TENANT_1_UUID);
        network.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        network.setProviderNetworkType("vlan");
        network.setProviderSegmentationID("999");
        nh.neutronNetworkUpdated(network);

        network = new NeutronNetwork();
        network.setNetworkName("network1");
        network.setProviderNetworkType("vlan");
        network.setProviderSegmentationID("4066");
        nh.neutronNetworkUpdated(network);
    }

    /**
     * Test method for
     * {@link NetworkHandler#canDeleteNetwork(NeutronNetwork)}.
     */
    @Test
    public void testCanDeleteNetwork() {
        NetworkHandler nh = new NetworkHandler();
        IVTNManager mgr = new VTNManagerStub();
        nh.setVTNManager(mgr);

        // In the cases that the method returns HTTP_NOT_FOUND.
        NeutronNetwork network = new NeutronNetwork();
        network.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        network.setNetworkName("network1");
        network.setTenantID(TENANT_ID_ARRAY[0]);

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     nh.canDeleteNetwork(network));

        network = new NeutronNetwork();
        network.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        network.setNetworkName("network1");
        network.setTenantID(VTNManagerStub.TENANT_1_UUID);

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     nh.canDeleteNetwork(network));

        // In the cases that the method returns HTTP_OK.
        network = new NeutronNetwork();
        network.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        network.setTenantID(VTNManagerStub.TENANT_1_UUID);

        assertEquals(HttpURLConnection.HTTP_OK,
                     nh.canDeleteNetwork(network));
    }
}
