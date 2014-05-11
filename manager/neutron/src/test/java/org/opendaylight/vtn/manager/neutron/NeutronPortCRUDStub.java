/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.neutron;

import java.util.List;
import org.opendaylight.controller.networkconfig.neutron.INeutronPortCRUD;
import org.opendaylight.controller.networkconfig.neutron.NeutronPort;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.vtn.manager package.
 */

public class NeutronPortCRUDStub implements INeutronPortCRUD {
  @Override
    public boolean portExists(String uuid) {
      return false;
    }

  @Override
    public NeutronPort getPort(String uuid) {
      if (uuid.equals("ovsdb")) {
        NeutronPort neutron = new NeutronPort();
        return neutron;
      } else if (uuid.equals("ovsdb1")) {
        NeutronPort neutron = new NeutronPort();
        neutron.setTenantID("tenantid");
        return neutron;
      } else if (uuid.equals("ovsdb_bridgeUUID")) {
        NeutronPort neutron = new NeutronPort();
        neutron.setNetworkUUID("bridgeUUID");
        return neutron;
      } else if (uuid.equals("ovsdb_portUUID")) {
        NeutronPort neutron = new NeutronPort();
        neutron.setPortUUID("portUUID");
        return neutron;
      } else if (uuid.equals("neutron_fill")) {
        NeutronPort neutron = new NeutronPort();
        neutron.setTenantID("tenantid");
        neutron.setPortUUID("portUUID");
        neutron.setNetworkUUID("bridgeUUID");
        return neutron;
      }
      return null;

    }

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
}
