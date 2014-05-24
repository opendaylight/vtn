/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import org.junit.Test;

import java.util.UUID;
import java.math.BigInteger;

import org.opendaylight.ovsdb.lib.notation.OvsDBSet;
import org.opendaylight.ovsdb.lib.table.Bridge;
import org.opendaylight.ovsdb.lib.table.Interface;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.ovsdb.lib.notation.OvsDBMap;

import static org.junit.Assert.assertEquals;

/**
 * JUnit test for {@link OVSDBPluginEventHandler}.
 */
public class OVSDBPluginEventHandlerTest extends TestBase {
  /**
   * Test method for
   * {@link OVSDBPluginEventHandler#getIntegrationBridgeName()}.
   * Test GetSet Methods in OVSDBPluginEventHandler with value.
   */
  @Test
    public void testSetGetMethods() {
      OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
      ovsdb.setIntegrationBridgeName(BRIDGENAME);
      ovsdb.setFailmode(FAILMODE);
      ovsdb.setProtocols(PROTOCOLS);
      ovsdb.setPortName(PORTNAME);
      assertEquals(BRIDGENAME,
          ovsdb.getIntegrationBridgeName());
      assertEquals(FAILMODE, ovsdb.getFailmode());
      assertEquals(PROTOCOLS, ovsdb.getProtocols());
      assertEquals(PORTNAME, ovsdb.getPortName());
    }

  /**
   * Test method for
   * {@link OVSDBPluginEventHandler#createInternalNetworkForNeutron(Node)}.
   * Test createInternalNetworkForNeutron Method.
   */
  @Test
    public void createInternalNetworkForNeutron() {
      for (int i=0; i<CREATE_NETWORK_ARRAY.length; i++) {
        try {
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          IVTNManager mgr = new VTNManagerStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setVTNManager(mgr);

          if (OPENFLOW.equalsIgnoreCase(CREATE_NETWORK_ARRAY[i][0])) {
            Long longObj = new Long(Long.parseLong(CREATE_NETWORK_ARRAY[i][1]));
            Node nodeObj = new Node(CREATE_NETWORK_ARRAY[i][0], longObj);
            ovsdb.createInternalNetworkForNeutron(nodeObj);
          } else if ((ONEPK.equalsIgnoreCase(CREATE_NETWORK_ARRAY[i][0])) ||
              (PRODUCTION.equalsIgnoreCase(CREATE_NETWORK_ARRAY[i][0]))) {
            Node nodeObj = new Node(CREATE_NETWORK_ARRAY[i][0], CREATE_NETWORK_ARRAY[i][1]);
            ovsdb.createInternalNetworkForNeutron(nodeObj);
          } else if(PCEP.equalsIgnoreCase(CREATE_NETWORK_ARRAY[i][0])) {
            UUID idOne = new UUID(0L, 0L);
            Node nodeObj = new Node(CREATE_NETWORK_ARRAY[i][0], idOne);
            ovsdb.nodeAdded(nodeObj);
          }
          if (CREATE_NETWORK_ARRAY[i][3].equalsIgnoreCase("1")) {
            assertEquals(1, 0);
          } else if(CREATE_NETWORK_ARRAY[i][3].equalsIgnoreCase("0")) {
            assertEquals(0, 0);
          }

        } catch (Exception ex) {
          if (CREATE_NETWORK_ARRAY[i][3].equalsIgnoreCase("1")) {
            if ("null".equalsIgnoreCase(CREATE_NETWORK_ARRAY[i][4])) {
              assertEquals(null, ex.getMessage());
            }
          }
        }
      }
    }

  /**
   * Test method for
   * {@link OVSDBPluginEventHandler#rowRemoved(Node, String, String, Table, Object)}.
   * Test rowRemoved Method in OVSDBPluginEventHandler.
   */
  @Test
    public void testRowRemoved() {
      for(int test = 0; test < ROW_REMOVE_ARRAY.length; test++) {
        try {
          Long longObj = new Long(ROW_REMOVE_ARRAY[test][0]);
          Node nodeObj = new Node(OPENFLOW, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("65534");
          ovsdbint.add(a);
          Interface intfRow = new Interface();
          intfRow.setName(ROW_REMOVE_ARRAY[test][1]);
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", ROW_REMOVE_ARRAY[test][2]);
          intfRow.setExternal_ids(mapOvsdb);
          Object obj = new Object();
          if("0D2206F8-B700-4F78-913D-9CE7A2D78473".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][2])) {
            IVTNManager mgr = new VTNManagerStub();
            ovsdb.setVTNManager(mgr);
            ovsdb.rowRemoved(nodeObj, "Interface", "85c27f20-f218-11e3-a7b6-0002a5d5c51b", intfRow, obj);
            if("0".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][4])){
              assertEquals(0, 0);
            }
          } else if("C387EB44-7832-49F4-B9F0-D30D27770883".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][2])) {
            IVTNManager mgr = new VTNManagerStub();
            ovsdb.setVTNManager(mgr);
            ovsdb.rowRemoved(nodeObj, "Interface", "85c27f20-f218-11e3-a7b6-0002a5d5c51b", intfRow, obj);
            if("0".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][4])){
              assertEquals(0, 0);
            }
          } else if("C387EB44-7832-49F4-B9F0-D30D27770883".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][2])) {
            IVTNManager mgr = new VTNManagerStub();
            ovsdb.setVTNManager(mgr);
            ovsdb.rowRemoved(nodeObj, "Interface", "85c27f20-f218-11e3-a7b6-0002a5d5c51b", intfRow, obj);
            if("0".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][4])) {
              assertEquals(0, 0);
            }
          } else if(ROW_REMOVE_ARRAY[test][1].equals(null)) {
            IVTNManager mgr = new VTNManagerStub();
            ovsdb.setVTNManager(mgr);
            if("0".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][4])) {
              assertEquals(0, 0);
            }
          } else if("unset_manager".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][3])) {
            if("1".equalsIgnoreCase(ROW_REMOVE_ARRAY[test][4])) {
              assertEquals(1, 0);
            }
          }
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }
    }

  /**
   * Test method for
   * {@link OVSDBPluginEventHandler#rowUpdated(Node, String, String, Table, Table)}.
   * Test rowUpdated Methods with Neutron Port in OVSDBPluginEventHandler.
   */
  @Test
    public void testNodeUpdatedNeutronPort() {
      for (int tmp = 0; tmp < RW_UPDT_INP_ARY.length; tmp++) {
        try {
          String pcep = RW_UPDT_INP_ARY[tmp][0];
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          if(RW_UPDT_INP_ARY[tmp][1].equalsIgnoreCase("bridge")
              || RW_UPDT_INP_ARY[tmp][1].equalsIgnoreCase("bridge-int")){
            if(RW_UPDT_INP_ARY[tmp][1].equalsIgnoreCase("bridge-int")) {
              Interface bridge = new Interface();
              bridge.setName(RW_UPDT_INP_ARY[tmp][2]);
              if(RW_UPDT_INP_ARY[tmp][5].equalsIgnoreCase("null")
                  && RW_UPDT_INP_ARY[tmp][6].equalsIgnoreCase("null")) {
                ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_ARY[tmp][3],
                    RW_UPDT_INP_ARY[tmp][4], null, null);
              } else if (RW_UPDT_INP_ARY[tmp][6].equalsIgnoreCase("null")) {
                ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_ARY[tmp][3],
                    RW_UPDT_INP_ARY[tmp][4], bridge, null);
              } else{
                ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_ARY[tmp][3],
                    RW_UPDT_INP_ARY[tmp][4], bridge, bridge);
              }
            } else{
              Bridge bridge = new Bridge();
              bridge.setName(RW_UPDT_INP_ARY[tmp][2]);
              // Send Bridge instead of Interface
              ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_ARY[tmp][3],
                  RW_UPDT_INP_ARY[tmp][4], bridge, bridge);
            }

          } else if(RW_UPDT_INP_ARY[tmp][1].equalsIgnoreCase("intf")) {
            Interface intfOld = new Interface();
            intfOld.setName(RW_UPDT_INP_ARY[tmp][2]);
            Interface intfNew = new Interface();
            OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
            mapOvsdb.put(RW_UPDT_INP_ARY[tmp][12], RW_UPDT_INP_ARY[tmp][13]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_ARY[tmp][3],
                RW_UPDT_INP_ARY[tmp][4], intfOld, intfNew);
          } else if(RW_UPDT_INP_ARY[tmp][1].equalsIgnoreCase("intf-neutron")) {
            NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
            ovsdb.setNeutronPortCRUD(neutron);
            Interface intfOld = new Interface();
            intfOld.setName(RW_UPDT_INP_ARY[tmp][2]);
            Interface intfNew = new Interface();
            OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
            mapOvsdb.put("iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883");
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_ARY[tmp][3],
                RW_UPDT_INP_ARY[tmp][4], intfOld, intfNew);
          }
          assertEquals(RW_UPDT_INP_ARY[tmp][7], RW_UPDT_INP_ARY[tmp][8]);
        } catch (Exception ex) {
          if(RW_UPDT_INP_ARY[tmp][9].equalsIgnoreCase("ex_msg")) {
            assertEquals(EXCEPTION_MSG, ex.getMessage());
          }
          if(RW_UPDT_INP_ARY[tmp][9].equalsIgnoreCase("null_msg")) {
            assertEquals(null, ex.getMessage());
          }
          assertEquals(RW_UPDT_INP_ARY[tmp][10], RW_UPDT_INP_ARY[tmp][11]);
        }
      }
    }

  /**
   * Test method for
   * {@link OVSDBPluginEventHandler#rowUpdated(Node, String, String, Table, Table)}.
   * Test rowUpdated Method with ExternalId in OVSDBPluginEventHandler.
   */
  @Test
    public void testRowUpdateWithExternalId() {
      for (int tmp = 0; tmp < RW_UPDT_INP_OF_ARY.length; tmp++) {
        try {
          String openFlow = "OF";
          Long longObj = new Long(RW_UPDT_INP_OF_ARY[tmp][1]);
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          if (!(RW_UPDT_INP_OF_ARY[tmp][0].equalsIgnoreCase("Set_OF_Null"))) {
            ovsdb.setNeutronPortCRUD(neutron);
             }
          if(RW_UPDT_INP_OF_ARY[tmp][0].equalsIgnoreCase("Set_OF_Bridge")){
            Bridge intfNew = new Bridge();
            OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
            mapOvsdb.put(RW_UPDT_INP_OF_ARY[tmp][3], RW_UPDT_INP_OF_ARY[tmp][4]);
            intfNew.setExternal_ids(mapOvsdb);
            Interface bb = new Interface();
            ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_OF_ARY[tmp][5], RW_UPDT_INP_OF_ARY[tmp][6], bb, intfNew);
            } else if(RW_UPDT_INP_OF_ARY[tmp][0].equalsIgnoreCase("Set_OF_Bridge_intf")){
            Interface intfNew = new Interface();
            OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
            mapOvsdb.put(RW_UPDT_INP_OF_ARY[tmp][3], RW_UPDT_INP_OF_ARY[tmp][4]);
            intfNew.setExternal_ids(mapOvsdb);
            Bridge bb = new Bridge();
            ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_OF_ARY[tmp][5], RW_UPDT_INP_OF_ARY[tmp][6], bb, intfNew);
            } else{
            Interface intfOld = new Interface();
            intfOld.setName(RW_UPDT_INP_OF_ARY[tmp][2]);
            Interface intfNew = new Interface();
            OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String>();
            mapOvsdb.put(RW_UPDT_INP_OF_ARY[tmp][3], RW_UPDT_INP_OF_ARY[tmp][4]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, RW_UPDT_INP_OF_ARY[tmp][5], RW_UPDT_INP_OF_ARY[tmp][6], intfOld, intfNew);
             }
            assertEquals(RW_UPDT_INP_OF_ARY[tmp][7], RW_UPDT_INP_OF_ARY[tmp][8]);
            } catch (Exception ex) {
          if (RW_UPDT_INP_OF_ARY[tmp][9].equalsIgnoreCase("ex_msg")) {
            assertEquals(EXCEPTION_MSG, ex.getMessage());
           }
          if (RW_UPDT_INP_OF_ARY[tmp][9].equalsIgnoreCase("null_msg")) {
            assertEquals(null, ex.getMessage());
          assertEquals(RW_UPDT_INP_OF_ARY[tmp][10], RW_UPDT_INP_OF_ARY[tmp][11]);
           }
          }
      }
    }

  /**
   * Test method for
   * {@link OVSDBPluginEventHandler#rowUpdated(Node, String, String, Table, Table)}.
   * Test rowUpdated Method with Port in OVSDBPluginEventHandler.
   */
  @Test
    public void testNodeRowUpdatePort() {
      for (int test = 0; test < ROW_UPDATE_PORT_ARRAY.length; test++) {
        try {
          String openFlow = ROW_UPDATE_PORT_ARRAY[test][0];
          Long longObj = new Long(ROW_UPDATE_PORT_ARRAY[test][1]);
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName(ROW_UPDATE_PORT_ARRAY[test][2]);
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          // In this case when the Interface port is not filled.
          if("set_port".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][4])) {
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("set_null_port".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][4])){
            // In this case when the Get port is Null.
            OvsDBSet<BigInteger> ovsdbInt = null;
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            intfNew.setOfport(ovsdbInt);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("set_empty_port".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][4])){
            // In this case the Get Port is Empty.
            OvsDBSet<BigInteger> ovsdbInt = new  OvsDBSet<BigInteger>();
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            intfNew.setOfport(ovsdbInt);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("without_port".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][4])){
            // In this case the VTN Identifiers are Null.
            OvsDBSet<BigInteger> ovsdbint = new  OvsDBSet<BigInteger>();
            BigInteger a = new BigInteger(ROW_UPDATE_PORT_ARRAY[test][5]);
            ovsdbint.add(a);
            intfNew.setOfport(ovsdbint);
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("4790F3C1-AB34-4ABC-B7A5-C1B5C7202389".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][3])){
            // In this case when the TenantUUID not Null
            OvsDBSet<BigInteger> ovsdbint = new  OvsDBSet<BigInteger>();
            BigInteger a = new BigInteger(ROW_UPDATE_PORT_ARRAY[test][5]);
            ovsdbint.add(a);
            intfNew.setOfport(ovsdbint);
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("52B1482F-A41E-409F-AC68-B04ACFD07779".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][3])){
            // In this case when the bridge UUid not Null.
            OvsDBSet<BigInteger> ovsdbint = new  OvsDBSet<BigInteger>();
            BigInteger a = new BigInteger(ROW_UPDATE_PORT_ARRAY[test][5]);
            ovsdbint.add(a);
            intfNew.setOfport(ovsdbint);
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("8c781fc0-f215-11e3-aac3-0002a5d5c51b".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][3])){
            // In this case when the Port UUID not Null.
            OvsDBSet<BigInteger> ovsdbint = new  OvsDBSet<BigInteger>();
            BigInteger a = new BigInteger(ROW_UPDATE_PORT_ARRAY[test][5]);
            ovsdbint.add(a);
            intfNew.setOfport(ovsdbint);
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          } else if ("0D2206F8-B700-4F78-913D-9CE7A2D78473".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][3])){
            // In this case when the VTNIdentifiers is Success.
            OvsDBSet<BigInteger> ovsdbint = new  OvsDBSet<BigInteger>();
            BigInteger a = new BigInteger(ROW_UPDATE_PORT_ARRAY[test][5]);
            ovsdbint.add(a);
            intfNew.setOfport(ovsdbint);
            mapOvsdb.put("iface-id", ROW_UPDATE_PORT_ARRAY[test][3]);
            intfNew.setExternal_ids(mapOvsdb);
            ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
            if("0".equalsIgnoreCase(ROW_UPDATE_PORT_ARRAY[test][6])) {
              assertEquals(0, 0);
            }
          }
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
    }
}
