/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.controller.networkconfig.neutron.NeutronPort;
import org.opendaylight.controller.networkconfig.neutron.Neutron_IPs;

/**
 * JUnit test for {@link PortHandler}
 */
public class PortHandlerTest extends TestBase {
    /**
     * Test method for
     * {@link PortHandler#canCreatePort(NeutronPort)}.
     */
    @Test
    public void testCanCreatePort() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        // In the cases that the method returns HTTP_NOT_FOUND.
        NeutronPort port = new NeutronPort();
        port.setTenantID(TENANT_ID_ARRAY[0]);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     ph.canCreatePort(port));

        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                     ph.canCreatePort(port));

        // In the cases that the method returns HTTP_CREATED.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_CREATED,
                     ph.canCreatePort(port));

        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[2]);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);

        assertEquals(HttpURLConnection.HTTP_CREATED,
                     ph.canCreatePort(port));

        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[3]);
        port.setMacAddress(MAC_ADDR_ARRAY[2]);
        List<Neutron_IPs> ips = new ArrayList<Neutron_IPs>(1);
        Neutron_IPs ip = new Neutron_IPs(NEUTRON_UUID_ARRAY[4]);
        ip.setIpAddress("192.168.0.1");
        ips.add(ip);
        port.setFixedIPs(ips);
        port.setAdminStateUp(Boolean.FALSE);

        assertEquals(HttpURLConnection.HTTP_CREATED,
                     ph.canCreatePort(port));

        // In the cases that the method returns HTTP_CONFLICT.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_CONFLICT,
                     ph.canCreatePort(port));
    }

    /**
     * Test method for
     * {@link PortHandler#neutronPortCreated(NeutronPort)}.
     */
    @Test
    public void testNeutronPortCreated() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        NeutronPort port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortCreated(port);
    }
}
