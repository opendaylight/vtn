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
import java.util.concurrent.ConcurrentHashMap;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.ovsdb.lib.notation.OvsDBSet;
import org.opendaylight.ovsdb.lib.notation.UUID;
import org.opendaylight.ovsdb.lib.table.Bridge;
import org.opendaylight.ovsdb.lib.table.Port;
import org.opendaylight.ovsdb.lib.table.internal.Table;
import org.opendaylight.ovsdb.plugin.OVSDBConfigService;
import org.opendaylight.ovsdb.plugin.StatusWithUuid;

import org.opendaylight.controller.sal.core.Node;
import java.util.concurrent.ConcurrentMap;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.vtn.manager package.
 */
public class OVSDBManagerStub implements OVSDBConfigService {
  /**
   * Member used to store table details in ConcurrentMap.
   */
  private ConcurrentMap<String, Table<?>>  table = new ConcurrentHashMap();

  @Override
    public StatusWithUuid insertRow (Node node, String tableName, String parentUUID, Table<?> row) {
      String openFlow = "OF";
      String onePk = "PK";
      String pcep = "PE";
      UUID uu = new UUID("bbb");
      if (openFlow == node.getType()) {
        return new StatusWithUuid(StatusCode.SUCCESS, uu);
      } else if(onePk == node.getType()) {
        return new StatusWithUuid(StatusCode.SUCCESS);
      } else if (pcep == node.getType()) {
        return new StatusWithUuid(StatusCode.CREATED, uu);
      } else {
        return new StatusWithUuid(StatusCode.UNDEFINED);
      }
    }

  @Override
    public Status deleteRow (Node node, String tableName, String rowUUID) {
      return new Status(StatusCode.SUCCESS);
    }

  @Override
    public Status updateRow (Node node, String tableName, String parentUUID, String rowUUID, Table<    ?> row) {
      Long testValue = new Long("51");
      String openFlow = "OF";
      Object obj = node.getID();
      if (openFlow == node.getType()) {
        if (testValue == Long.valueOf(obj.toString())) {
          return new Status(StatusCode.SUCCESS);
        } else {
          return new Status(StatusCode.UNDEFINED);
        }
      } else if ("PE".equals(node.getType())) {
        return new Status(StatusCode.SUCCESS);
      } else {
        return new Status(StatusCode.UNDEFINED);
      }
    }
  @Override
    public String getSerializedRow(Node node, String tableName, String uuid) throws Exception {
      return "Row";
    }

  @Override
    public String getSerializedRows(Node node, String tableName) throws Exception {
      return "Rows";
    }

  @Override
    public Table<?> getRow(Node node, String tableName, String uuid) throws Exception {
      Long portIdTest1 = new Long("5");
      Long portIdTest2 = new Long("4");
      Long portIdTest3 = new Long("3");
      Long portIdTest4 = new Long("2");
      Long portIdTest5 = new Long("100");
      Long portIdTest6 = new Long("101");
      Object obj = node.getID();
      if (tableName.equals("Port")) {
        Long longObj = new Long("5");
        if((portIdTest1 == Long.valueOf(obj.toString())) ||
           (portIdTest5 == Long.valueOf(obj.toString())) ||
           (portIdTest6 == Long.valueOf(obj.toString()))) {
          OvsDBSet<UUID> ovsdbset = new OvsDBSet<UUID>();
          UUID uu = new UUID("intf-row1");
          ovsdbset.add(uu);
          Port port = new Port();
          port.setInterfaces(ovsdbset);
          return port;
        } else if (portIdTest2 ==  Long.valueOf(obj.toString())) {
          Port port = new Port();
          return port;
        } else if (portIdTest3 ==  Long.valueOf(obj.toString())) {
          Port port = new Port();
          OvsDBSet<UUID> ovsdbset = new OvsDBSet<UUID>();
          port.setInterfaces(ovsdbset);
          return port;
        } else if (portIdTest4 ==  Long.valueOf(obj.toString())) {
          Port port = new Port();
          OvsDBSet<UUID> ovsdbset = new OvsDBSet<UUID>();
          UUID uu = new UUID("intf-row2");
          ovsdbset.add(uu);
          port.setInterfaces(ovsdbset);
          return port;
        } else {
          return null;
        }
      }
      return null;
    }

  @Override
    public ConcurrentMap<String, Table<?>> getRows(Node node, String tableName) throws Exception {
      Object obj = node.getID();
      Long nodeIdTest1 = new Long("100");
      Long nodeIdTest2 = new Long("101");
      String pcep = "PE";
      String openFlow = "OF";
      if (pcep == node.getType()) {
        Bridge bridge = new Bridge();
        bridge.setName("br-int");
        table.put("1", bridge);
        return table;
      } else if (openFlow == node.getType()) {
        if ((nodeIdTest1 != Long.valueOf(obj.toString())) &&
            (nodeIdTest2 != Long.valueOf(obj.toString()))) {
          Bridge bridge = new Bridge();
          OvsDBSet<UUID> ovsdbset = new OvsDBSet<UUID>();
          UUID uu = new UUID("bbb");
          ovsdbset.add(uu);
          bridge.setPorts(ovsdbset);
          bridge.setName("br-int");
          table.put("1", bridge);
          return table;
        } else if (nodeIdTest1 == Long.valueOf(obj.toString())) {
          Bridge bridge = new Bridge();
          OvsDBSet<UUID> ovsdbset = new OvsDBSet<UUID>();
          OvsDBSet<String> ovsdbdatapath = new OvsDBSet<String>();
          UUID uu = new UUID("bbb");
          ovsdbset.add(uu);
          bridge.setPorts(ovsdbset);
          bridge.setDatapath_id(ovsdbdatapath);
          bridge.setName("br-int");
          table.put("1", bridge);
          ovsdbdatapath.add("str_value");
          Bridge bridgeValueDatapath = new Bridge();
          bridgeValueDatapath.setDatapath_id(ovsdbdatapath);
          bridgeValueDatapath.setPorts(ovsdbset);
          bridgeValueDatapath.setName("br-int");
          table.put("2", bridgeValueDatapath);
          return table;
        } else if (nodeIdTest2 == Long.valueOf(obj.toString())) {
          OvsDBSet<UUID> ovsdbset = new OvsDBSet<UUID>();
          OvsDBSet<String> ovsdbdatapath = new OvsDBSet<String>();
          UUID uu = new UUID("bbb");
          ovsdbset.add(uu);
          ovsdbdatapath.add("128");
          Bridge bridgeValueDatapath = new Bridge();
          bridgeValueDatapath.setDatapath_id(ovsdbdatapath);
          bridgeValueDatapath.setPorts(ovsdbset);
          bridgeValueDatapath.setName("br-int");
          table.put("3", bridgeValueDatapath);
          return table;
        }
      } else {
        return null;
      }
      return null;
    }
  @Override
    public List<String> getTables(Node node) throws Exception {
      return null;
    }
}
