/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.net.HttpURLConnection;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.neutron.spi.NeutronPort;
import org.opendaylight.neutron.spi.Neutron_IPs;

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

        // Failure Case - Method returns HTTP_BAD_REQUEST by setting NULL to NeutronPort object.
        NeutronPort port = null;

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Failure Case - Method returns HTTP_NOT_FOUND by setting wrong TenantID.
        port = new NeutronPort();
        port.setTenantID(TENANT_ID_ARRAY[0]);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                ph.canCreatePort(port));

        // Failure Case - Method returns HTTP_NOT_FOUND by setting wrong NetworkID.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                ph.canCreatePort(port));

        // Failure Case - Method returns HTTP_CONFLICT by setting existing UUID to Port UUID
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_CONFLICT,
                ph.canCreatePort(port));

        // Failure Case - Method returns HTTP_BAD_REQUEST by setting wrong UUID to Port UUID.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_3_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Success Case - Method returns HTTP_CREATED.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[2]);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);

        assertEquals(HttpURLConnection.HTTP_CREATED,
                ph.canCreatePort(port));

        // Success Case - Method returns HTTP_CREATED.
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
    }

    /**
     * Test method for
     * {@link PortHandler#getVTNIdentifiers(NeutronPort, String[])}.
     */
    @Test
    public void testGetVTNIdentifiers() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        NeutronPort port = null;
        // Failure Case - By setting NULL to NeutronPort object
        port = null;

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Failure Case - By setting NULL to TennantID
        port = new NeutronPort();
        port.setTenantID((String)null);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Failure Case - By setting NULL to NetworkID
        port = new NeutronPort();
        port.setTenantID(NEUTRON_UUID_ARRAY[0]);
        port.setNetworkUUID(null);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Failure Case - By setting NULL to PortID
        port = new NeutronPort();
        port.setTenantID(NEUTRON_UUID_ARRAY[0]);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[1]);
        port.setPortUUID(null);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));
        // Failure Case - By setting invalid UUID to TennantID
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.INVALID_UUID);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Failure Case - By setting invalid UUID to NetworkID
        port = new NeutronPort();
        port.setTenantID(TENANT_ID_ARRAY[0]);
        port.setNetworkUUID(VTNManagerStub.INVALID_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Failure Case - By setting invalid UUID to PortID
        port = new NeutronPort();
        port.setTenantID(TENANT_ID_ARRAY[0]);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(VTNManagerStub.INVALID_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canCreatePort(port));

        // Success Case - Only for Method(getVTNIdentifiers)
        port = new NeutronPort();
        port.setTenantID(TENANT_ID_ARRAY[0]);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
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

        // Failure Case - Method returns HTTP_CONFLICT and failed to create New NeutronPort.
        NeutronPort port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortCreated(port);

        // Failure Case - Method returns HTTP_CONFLICT and failed to create New NeutronPort.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_2_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortCreated(port);

        // Failure Case - Method returns HTTP_CONFLICT and failed to create Bridge Interface.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_4_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortCreated(port);

        // Success Case - Method successfully executes.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[2]);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);

        ph.neutronPortCreated(port);
    }

    /**
     * Test method for
     * {@link PortHandler#canUpdatePort(NeutronPort, NeutronPort)}.
     */
    @Test
    public void testCanUpdatePort() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        // Failure Case - Method returns HTTP_BAD_REQUEST by setting NULL to both NeutronPort object.
        NeutronPort portDelta = null;
        NeutronPort portOriginal = null;

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canUpdatePort(portDelta, portOriginal));

        // Failure Case - Method returns HTTP_BAD_REQUEST by setting NULL to portOriginal object.
        portDelta = new NeutronPort();

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canUpdatePort(portDelta, portOriginal));

        // Failure Case - Method returns HTTP_BAD_REQUEST by setting NULL to TenantID.
        portOriginal = new NeutronPort();
        portOriginal.setTenantID((String)null);
        portOriginal.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        portOriginal.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        portOriginal.setMacAddress(MAC_ADDR_ARRAY[0]);
        portOriginal.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canUpdatePort(portDelta, portOriginal));

        // Failure Case - Method returns HTTP_NOT_FOUND(Interface does not exist) by setting wrong PortUUID.
        portOriginal = new NeutronPort();
        portOriginal.setTenantID(VTNManagerStub.TENANT_1_UUID);
        portOriginal.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        portOriginal.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        portOriginal.setMacAddress(MAC_ADDR_ARRAY[0]);
        portOriginal.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                ph.canUpdatePort(portDelta, portOriginal));

        // Success Case - Method returns HTTP_OK.
        portOriginal = new NeutronPort();
        portOriginal.setTenantID(VTNManagerStub.TENANT_1_UUID);
        portOriginal.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        portOriginal.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        portOriginal.setMacAddress(MAC_ADDR_ARRAY[1]);
        portOriginal.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_OK,
                ph.canUpdatePort(portDelta, portOriginal));
    }

    /**
     * Test method for
     * {@link PortHandler#neutronPortUpdated(NeutronPort)}.
     */
    @Test
    public void testNeutronPortUpdated() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        // Failure Case - Method returns HTTP_BAD_REQUEST by setting NULL to TenantID.
        NeutronPort port = new NeutronPort();
        port.setTenantID((String)null);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortUpdated(port);

        // Failure Case - Method returns HTTP_NOT_FOUND(Interface does not exist for tenant-id) by setting wrong PortUUID.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortUpdated(port);

        // Failure Case - Method(getBridgeInterface) returns NULL(throws VTNException) by setting Wrong Port UUID
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_3_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        VTNManagerStub.isReturnValidVInterface = true;
        try {
            ph.neutronPortUpdated(port);
        } catch (NullPointerException exception) {
            assertEquals(exception.getMessage(), exception.toString(), new NullPointerException().toString());
        }

        // Failure Case - Method(canModifyInterface returns false) by setting Port-AdminStateUp to null
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortUpdated(port);

        // Failure Case - Modifying bridge interface failed
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.FALSE);

        ph.neutronPortUpdated(port);

        // Success Case
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_2_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);
        port.setName("Neutron-port");

        ph.neutronPortUpdated(port);
    }

    /**
     * Test method for
     * {@link PortHandler#canModifyInterface(NeutronPort, VInterface)}.
     */
    @Test
    public void testCanModifyInterface() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        // Case - returns False by setting Port-AdminStateUp to null
        NeutronPort port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortUpdated(port);

        // Case - returns True by setting Port-AdminStateUp to False
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.FALSE);

        ph.neutronPortUpdated(port);

        // Case - returns True by setting Port-AdminStateUp to False and valid name to NeutronPort
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.FALSE);
        port.setName("Neutron-port");

        ph.neutronPortUpdated(port);

        // Case - returns True by setting Port-AdminStateUp to True and valid Description to VBridgeIfConfig
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_2_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);

        ph.neutronPortUpdated(port);

        // Case - returns True by setting Port-AdminStateUp to True, valid Description to VBridgeIfConfig and valid name to NeutronPort
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_2_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);
        port.setName("Neutron-port");

        ph.neutronPortUpdated(port);

        // Case - returns True by setting Port-AdminStateUp to True, valid Description to VBridgeIfConfig and same name of port object to VInterfaceConfig
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_2_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));
        port.setAdminStateUp(Boolean.TRUE);
        port.setName("br-int config");

        ph.neutronPortUpdated(port);
    }

    /**
     * Test method for
     * {@link PortHandler#canDeletePort(NeutronPort)}.
     */
    @Test
    public void testCanDeletePort() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        // Case - Method returns HTTP_BAD_REQUEST by setting NULL to TenantID.
        NeutronPort port = new NeutronPort();
        port.setTenantID((String)null);
        port.setNetworkUUID(NEUTRON_UUID_ARRAY[0]);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_BAD_REQUEST,
                ph.canDeletePort(port));

        // Case - returns HTTP_NOT_FOUND(Interface does not exist) by setting wrong PortUUID.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_NOT_FOUND,
                ph.canDeletePort(port));

        // Case - Method returns HTTP_OK.
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        assertEquals(HttpURLConnection.HTTP_OK,
                ph.canDeletePort(port));
    }

    /**
     * Test method for
     * {@link PortHandler#neutronPortDeleted(NeutronPort)}.
     */
    @Test
    public void testNeutronPortDeleted() {
        PortHandler ph = new PortHandler();
        ph.setVTNManager(new VTNManagerStub());

        // Case - returns HTTP_NOT_FOUND(Interface does not exist) by setting wrong PortUUID.
        NeutronPort port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(NEUTRON_UUID_ARRAY[1]);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortDeleted(port);

        // Case - returns HTTP_NOT_FOUND(deleteBridgeInterface failed) by setting wrong PortUUID
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_2_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[0]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortDeleted(port);

        // Success Case
        port = new NeutronPort();
        port.setTenantID(VTNManagerStub.TENANT_1_UUID);
        port.setNetworkUUID(VTNManagerStub.BRIDGE_1_UUID);
        port.setPortUUID(VTNManagerStub.VBR_IF_1_UUID);
        port.setMacAddress(MAC_ADDR_ARRAY[1]);
        port.setFixedIPs(new ArrayList<Neutron_IPs>(0));

        ph.neutronPortDeleted(port);
    }
}
