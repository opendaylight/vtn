/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.List;

import org.opendaylight.neutron.spi.INeutronPortCRUD;
import org.opendaylight.neutron.spi.NeutronPort;


/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.controller.networkconfig.neutron package.
 */

public class NeutronPortCRUDStub implements INeutronPortCRUD {
    // Following methods are used in UnitTest.
    @Override
    public NeutronPort getPort(String uuid) {
        if (uuid.equals("C387EB44-7832-49F4-B9F0-D30D27770883")) {
            //Setting all IDs(TenantID, SwitchID, PortID) to NULL
            NeutronPort neutron = new NeutronPort();
            return neutron;
        } else if (uuid.equals("D387EB44-7832-49F4-B9F0-D30D27770884")) {
            //Setting SwitchID and PortID to NULL
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("D387EB44-7832-49F4-B9F0-D30D27770885");
            return neutron;
        } else if (uuid.equals("4790F3C1-AB34-4ABC-B7A5-C1B5C7202389")) {
            //Setting PortID to NULL
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("E6E005D3A24542FCB03897730A5150E2");
            neutron.setNetworkUUID("5e7e0900-f215-11e3-aa76-0002a5d5c51b");
            return neutron;
        } else if (uuid.equals("4790F3C1-AB34-4ABC-B7A5-C1B5C7202389")) {
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("E6E005D3A24542FCB03897730A5150E2");
            return neutron;
        } else if (uuid.equals("52B1482F-A41E-409F-AC68-B04ACFD07779")) {
            NeutronPort neutron = new NeutronPort();
            neutron.setNetworkUUID("5e7e0900-f215-11e3-aa76-0002a5d5c51b");
            return neutron;
        } else if (uuid.equals("8c781fc0-f215-11e3-aac3-0002a5d5c51b")) {
            NeutronPort neutron = new NeutronPort();
            neutron.setPortUUID("9c781fc0-f215-11e3-aac3-0002a5d5c51b");
            return neutron;
        } else if (uuid.equals("0D2206F8-B700-4F78-913D-9CE7A2D78473")) {
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("E6E005D3A24542FCB03897730A5150E2");
            neutron.setPortUUID("9c781fc0-f215-11e3-aac3-0002a5d5c51b");
            neutron.setNetworkUUID("5e7e0900-f215-11e3-aa76-0002a5d5c51b");
            return neutron;
        } else if (uuid.equals("0D2206F8-B700-4F78-913D-9CE7A2D78474")) {
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("E6E005D3A24542FCB03897730A5150E2");
            neutron.setPortUUID("9c781fc0-f215-11e3-aac3-0002a5d5c51b");
            neutron.setNetworkUUID(TestBase.CONFLICTED_NETWORK_UUID);
            return neutron;
        } else if (uuid.equals("0D2206F8-B700-4F78-913D-9CE7A2D78475")) {
            // Invalid TenantID
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("");
            neutron.setPortUUID("9c781fc0-f215-11e3-aac3-0002a5d5c51b");
            neutron.setNetworkUUID("5e7e0900-f215-11e3-aa76-0002a5d5c51b");
            return neutron;
        } else if (uuid.equals("0D2206F8-B700-4F78-913D-9CE7A2D78476")) {
            // Invalid NetworkID
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("E6E005D3A24542FCB03897730A5150E2");
            neutron.setPortUUID("9c781fc0-f215-11e3-aac3-0002a5d5c51b");
            neutron.setNetworkUUID("");
            return neutron;
        } else if (uuid.equals("0D2206F8-B700-4F78-913D-9CE7A2D78477")) {
            // Invalid PortID
            NeutronPort neutron = new NeutronPort();
            neutron.setTenantID("E6E005D3A24542FCB03897730A5150E2");
            neutron.setPortUUID("");
            neutron.setNetworkUUID("5e7e0900-f215-11e3-aa76-0002a5d5c51b");
            return neutron;
        }
        return null;
    }

    // Following methods are Unused in UnitTest.
    @Override
    public List<NeutronPort> getAllPorts() {
        return null;
    }

    @Override
    public boolean addPort(NeutronPort input) {
        return false;
    }

    @Override
    public boolean removePort(String uuid) {
        return false;
    }

    @Override
    public boolean updatePort(String uuid, NeutronPort delta) {
        return false;
    }

    @Override
    public boolean macInUse(String macAddress) {
        return false;
    }

    @Override
    public NeutronPort getGatewayPort(String subnetUUID) {
        return null;
    }

    @Override
    public boolean portExists(String uuid) {
        return true;
    }

    @Override
    public boolean inUse(String uuid) {
        return true;
    }

    @Override
    public boolean remove(String uuid) {
        return true;
    }

    @Override
    public boolean exists(String uuid) {
        return true;
    }

    @Override
    public boolean update(String uuid, NeutronPort delta) {
        return true;
    }

    @Override
    public boolean add(NeutronPort input) {
        return true;
    }

    @Override
    public List<NeutronPort> getAll() {
        return null;
    }

    @Override
    public NeutronPort get(String uuid) {
        return null;
    }
}

