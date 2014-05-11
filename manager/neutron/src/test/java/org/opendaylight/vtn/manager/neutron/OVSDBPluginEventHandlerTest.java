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
import org.opendaylight.ovsdb.lib.table.internal.Table;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.vtn.manager.IVTNManager;
import static org.junit.Assert.assertEquals;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import org.opendaylight.ovsdb.lib.notation.OvsDBMap;
/**
 * JUnit test for {@link OVSDBPluginEventHandler}.
 */
  public class OVSDBPluginEventHandlerTest extends TestBase {
    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#getIntegrationBridgeName()}.
     * Test IntegrationBridgeName with value.
     */
    @Test
      public void testIntegrationBridgeNameValue() {
        OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
        ovsdb.setIntegrationBridgeName("value");
        assertEquals("value",
            ovsdb.getIntegrationBridgeName());
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#getFailmode()}.
      * Test getFailmode method.
      */
    @Test
      public void testFailModeValue() {
        OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
        ovsdb.setFailmode("fail_value");
        assertEquals("fail_value", ovsdb.getFailmode());
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#getFailmode()}.
      * Test getProtocol method.
      */
    @Test
      public void testgetProtocolsValue() {
        OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
        ovsdb.setProtocols("protocols_value");
        assertEquals("protocols_value", ovsdb.getProtocols());
      }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#getPortName()}.
     * Test getPortName method.
     */
    @Test
      public void testgetPortValue() {
        OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
        ovsdb.setPortName("ports_value");
        assertEquals("ports_value", ovsdb.getPortName());
      }
    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#createInternalNetworkForNeutron(Node)}.
     * Test Node Add method when Config Service is Null.
     */
    @Test
      public void testNodeAddedOVSDBConfigServiceNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("333");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          IVTNManager mgr = new VTNManagerStub();
          ovsdb.setVTNManager(mgr);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          assertEquals(0, 1);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          //OVSDB Config Service object is Null
          assertEquals(0, 0);
        }
      }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#createInternalNetworkForNeutron(Node)}.
     * Test Node Add method with proper values.
     */
    @Test
      public void testNodeAddedSuccesfully() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("51");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          assertEquals(0, 0);
        } catch (Exception ex) {
          // Node Add is Successful , so this catch part should not be executed
          assertEquals(0, 1);
        }
      }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#createInternalNetworkForNeutron(Node)}.
     * Test Node Add method when Status uuid is Null.
     */
    @Test
      public void testNodeAddStatusUuidNull() {
        try {
          String onePk = "PK";
          String retVal = "return";
          Node nodeObj = new Node(onePk, retVal);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          // Node not added createInternalNetworkForNeutron throws exception
          assertEquals(0, 1);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          // Node Add is not Successful , so this catch part should be executed
          assertEquals(0, 0);
        }
      }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#createInternalNetworkForNeutron(Node)}.
     * Test Node add Method when status uuid is undefined.
     */
    @Test
      public void testNodeAddStatusUuidUndefined() {
        try {
          String retVal = "return";
          String production = "PR";

          Node nodeObj = new Node(production, retVal);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();

          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          // Node not added createInternalNetworkForNeutron
          assertEquals(0, 0);
        } catch (Exception ex) {
          // Node Add is not Successful , but catch part should not be executed
          assertEquals(0, 1);
        }
      }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#createInternalNetworkForNeutron(Node)}.
     * Test Node Add method when Status uuid is create.
     */
    @Test
      public void testNodeAddStatusUuidCreate() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();

          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          // Node added createInternalNetworkForNeutron
          assertEquals(0, 0);
        } catch (Exception ex) {
          // Node Add is Successful , so catch part should not be executed
          assertEquals(0, 1);
        }
      }
    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
     * Test update Row method.
     */
  @Test
      public void testUpdateRow() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();


          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);

          ConcurrentMap<String, Table<?>>  oldTable = new ConcurrentHashMap();
          ConcurrentMap<String, Table<?>>  newTable = new ConcurrentHashMap();

          Bridge bridge = new Bridge();
          bridge.setName("br-int");
          oldTable.put("1", bridge);

          Bridge newBridge = new Bridge();
          newBridge.setName("br-int1");
          newTable.put("1", newBridge);
        } catch (Exception ex) {
          // Node Add is Successful , so catch part should not be executed
          assertEquals(0, 0);
        }
      }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
     * Test Node update method when Neutron Port as Bridge.
     */
    @Test
      public void testNodeUpdatedNeutronPortAsBridge() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Bridge bridge = new Bridge();
          bridge.setName("br-int");
          // Send Bridge instead of Interface
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", bridge, bridge);
          assertEquals(0, 1);
        } catch (Exception ex) {
          assertEquals("org.opendaylight.ovsdb.lib.table.Bridge cannot be cast to org.opendaylight.ovsdb.lib.table.Interface", ex.getMessage());
          assertEquals(0, 0);
        }
      }
    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
     * Test Node Update method when the Neutron Port is Null.
     */
    @Test
      public void testNodeUpdatedNeutronPortNull() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Interface bridge = new Interface();
          bridge.setName("br-int");
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", null, null);
          // Node added createInternalNetworkForNeutron
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          // Node Add is Successful , so catch part should not be executed
          assertEquals(0, 1);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node Update Method when Neutron port new row is Null.
      */
    @Test
      public void testNodeUpdatedNeutronPortNewRowNull() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Interface bridge = new Interface();
          bridge.setName("br-int");
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", bridge, null);
          assertEquals(0, 1);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when there is no external id.
      */
    @Test
      public void testNodeUpdatedNeutronPortNoExternalId() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Interface bridge = new Interface();
          bridge.setName("br-int");
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", bridge, bridge);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(0, 1);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the external id is wrong.
      */
   @Test
     public void testNodeUpdatedExternalIdWrong() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("1iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(0, 1);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Nodeupdate method when the Neutron Port CRUD is Null.
      */
   @Test
      public void testNodeUpdatedNeutronCURDNull() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(1, 0);
        } catch (Exception ex) {
          // Expected Catch should be called NeutronCURD is Null
          assertEquals(0, 0);
          assertEquals(null, ex.getMessage());
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node Update method when the NeutronPort is Null.
      */
    @Test
      public void testNodeUpdatedNeutronCURDNotNullNeutronPortNull() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node Update Method when Neutron Port is Not Null.
      */
     @Test
      public void testNodeUpdatedNeutronCURDNotNullNeutronPortNotNull() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node Update Method when there is No port.
      */
    @Test
      public void testNodeUpdatedSwitchIdFromInterfaceNoPort() {
        try {
          String pcep = "PE";
          UUID idOne = UUID.randomUUID();
          Node nodeObj = new Node(pcep, idOne);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}
      *  Make getRows Null.
      *  Test Node update method when the Interface Get row is Null.
      */
    @Test
      public void testNodeUpdatedSwitchIdFromInterfacePortGetRowNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("0");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}
      * Get Interfaces alone Null.
      * Test Node update method when the Get Interface is Null.
      */
     @Test
      public void testNodeUpdatedSwitchIdFromInterfacePortGetInterfacesNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("3");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "edf", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the Get Interface Row is Null.
      */
    @Test
      public void testNodeUpdatedSwitchIdFromInterfacePortGetInterfacesRowNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("5");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node Update method when the Uuid is Null.
      */
    @Test
      public void testNodeUpdatedInterfaceUuidNameNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("5");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", null, intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update Method when the Node is Null.
      */
    @Test
      public void testNodeUpdatedNodeNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("5");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(null, "Interface", "bb", intfOld, intfNew);
          assertEquals(1, 0);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test the Node update method when the interface not matches.
      */
    @Test
      public void testNodeUpdatedInterfaceNotMatches() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("2");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when there is Empty port.
      */
   @Test
      public void testNodeUpdateEmptyPort() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("4");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when there is no Interface table.
      */
    @Test
      public void testNodeUpdateNewTableNotInterfaceTable() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("4");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Bridge intfNew = new Bridge();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          Interface bb = new Interface();
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", bb, intfNew);
          assertEquals(1, 0);
          // Node added createInternalNetworkForNeutron
        } catch (Exception ex) {
          // Expected Catch should be called because Bridge is sent instead of Interface
          assertEquals("org.opendaylight.ovsdb.lib.table.Bridge cannot be cast to org.opendaylight.ovsdb.lib.table.Interface", ex.getMessage());
          assertEquals(0, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method not a interface table.
      */
    @Test
      public void testNodeUpdateNotInterfaceTable() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("4");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);

          Bridge bb = new Bridge();
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", bb, intfNew);
          assertEquals(1, 0);
          // Node added createInternalNetworkForNeutron
        } catch (Exception ex) {
          // Expected Catch should be called because Bridge is sent instead of Interface
          assertEquals("org.opendaylight.ovsdb.lib.table.Bridge cannot be cast to org.opendaylight.ovsdb.lib.table.Interface", ex.getMessage());
          assertEquals(0, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the getDataPathId is Null.
      */
   @Test
      public void testNodeUpdategetDataPathIdNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("5");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the Interface port is not filled.
      */
   @Test
      public void testNodeUpdategetDataPathIdNotNullInterfaceGetPortNotFilled() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("100");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the Get port is Null.
      */
   @Test
      public void testNodeUpdategetDataPathIdNotNullInterfaceGetPortNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("100");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbInt = null;
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          intfOld.setOfport(ovsdbInt);
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the Get Port is Empty.
      */
   @Test
      public void testNodeUpdategetDataPathIdNotNullInterfaceGetPortEmpty() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("100");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbInt = new  OvsDBSet<BigInteger>();
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          intfOld.setOfport(ovsdbInt);
          Interface intfNew = new Interface();
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the VTN Identifiers are Null.
      */
    @Test
      public void testNodeUpdatesetPortMapForInterfaceLongValueSuccessgetVTNIdentifiersNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("5");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          intfNew.setOfport(ovsdbint);
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the TenantUUID not Null.
      */
    @Test
      public void testNodeUpdategetVTNIdentifiersTenentUUIDNotNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          intfNew.setOfport(ovsdbint);
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb1");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node Update Method when the bridge UUid not Null.
      */
    @Test
      public void testNodeUpdategetVTNIdentifiersbridgeUUIDNotNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          intfNew.setOfport(ovsdbint);
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb_bridgeUUID");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update Method when the Port UUID not Null.
      */
    @Test
      public void testNodeUpdategetVTNIdentifiersPortUUIDNotNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          intfNew.setOfport(ovsdbint);
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb_portUUID");
          intfNew.setExternal_ids(mapOvsdb);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowUpdated(Node, String , String , Table, Table)}.
      * Test Node update method when the VTNIdentifiers is Success.
      */
    @Test
      public void testNodeUpdategetVTNIdentifiersSuccess() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.createInternalNetworkForNeutron(nodeObj);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfOld = new Interface();
          intfOld.setName("intf-int");
          Interface intfNew = new Interface();
          intfNew.setOfport(ovsdbint);
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "neutron_fill");
          intfNew.setExternal_ids(mapOvsdb);
          IVTNManager mgr = new VTNManagerStub();
          ovsdb.setVTNManager(mgr);
          ovsdb.rowUpdated(nodeObj, "Interface", "intf-row1", intfOld, intfNew);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowRemoved(Node, String , String , Table, Object)}.
      * Test node row remove Method with VTNIdentifiers success.
      */
     @Test
        public void testRowRemovedVTNIdentifiersSuccess() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfRow = new Interface();
          intfRow.setName("intf-int");
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "neutron_fill");
          intfRow.setExternal_ids(mapOvsdb);
          IVTNManager mgr = new VTNManagerStub();
          ovsdb.setVTNManager(mgr);
          Object obj = new Object();
          ovsdb.rowRemoved(nodeObj, "Interface", "intf-row1", intfRow, obj);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(1, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowRemoved(Node , String , String , Table, Object)}.
      * Test node row remove method without setmanager method.
      */
   @Test
       public void testRowRemovedVTNIdentifiersWithoutSetManager() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfRow = new Interface();
          intfRow.setName("intf-int");
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "neutron_fill");
          intfRow.setExternal_ids(mapOvsdb);
          IVTNManager mgr = new VTNManagerStub();
          Object obj = new Object();
          ovsdb.rowRemoved(nodeObj, "Interface", "intf-row1", intfRow, obj);
          assertEquals(1, 0);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }

     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowRemoved(Node , String , String , Table, Object)}.
      * Test method for Node row remove method when interface value is NULL.
      */
    @Test
       public void testRowRemovedVTNIdentifiersIntfNull() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfRow = new Interface();
          intfRow.setName("intf-int");
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "neutron_fill");
          intfRow.setExternal_ids(mapOvsdb);
          IVTNManager mgr = new VTNManagerStub();
          Object obj = new Object();
          ovsdb.rowRemoved(nodeObj, "Interface", "intf-row1", null, obj);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }
     /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowRemoved(Node , String , String , Table, Object)}.
      * Test for Node Row remove method when this method returns HTTP NOT OK.
      */
   @Test
       public void testRowRemovedVTNIdentifiersHTTPNotOK() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("101");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfRow = new Interface();
          intfRow.setName("intf-int");
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "ovsdb");
          intfRow.setExternal_ids(mapOvsdb);
          IVTNManager mgr = new VTNManagerStub();
          Object obj = new Object();
          ovsdb.rowRemoved(nodeObj, "Interface", "intf-row1", intfRow, obj);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }
      /**
      * Test method for
      * {@link OVSDBPluginEventHandler#rowRemoved(Node , String, String , Table, Object)}.
      * Test for Node row remove method when neutron Undefined value is not NULL.
      */
    @Test
       public void testRowRemovedVTNIdentifiersPortUndefined() {
        try {
          String openFlow = "OF";
          Long longObj = new Long("5");
          Node nodeObj = new Node(openFlow, longObj);
          OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
          NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
          ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
          OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
          ovsdb.setOVSDBConfigService(ovsdbConfig);
          ovsdb.setConnectionService(connectionService);
          ovsdb.setNeutronPortCRUD(neutron);
          OvsDBSet<BigInteger> ovsdbint = new OvsDBSet<BigInteger>();
          BigInteger a = new BigInteger("33333");
          ovsdbint.add(a);
          Interface intfRow = new Interface();
          intfRow.setName("intf-int");
          OvsDBMap<String, String> mapOvsdb = new OvsDBMap<String, String> ();
          mapOvsdb.put("iface-id", "neutron_undefined");
          intfRow.setExternal_ids(mapOvsdb);
          IVTNManager mgr = new VTNManagerStub();
          ovsdb.setVTNManager(mgr);
          Object obj = new Object();
          ovsdb.rowRemoved(nodeObj, "Interface", "intf-row1", intfRow, obj);
          assertEquals(0, 0);
        } catch (Exception ex) {
          assertEquals(null, ex.getMessage());
          assertEquals(0, 0);
        }
      }
}
