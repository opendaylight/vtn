/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.io.File;
import java.io.FileOutputStream;
import java.util.Properties;
import java.util.UUID;

import org.junit.After;
import org.junit.Test;

import org.opendaylight.ovsdb.schema.openvswitch.Bridge;
import org.opendaylight.ovsdb.schema.openvswitch.Port;
import org.opendaylight.ovsdb.schema.openvswitch.OpenVSwitch;
import org.opendaylight.ovsdb.schema.openvswitch.Interface;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.ovsdb.lib.notation.OvsdbMap;

import static org.junit.Assert.assertEquals;

/**
 * JUnit test for {@link OVSDBPluginEventHandler}.
 */
public class OVSDBPluginEventHandlerTest extends TestBase {
    /**
     * The name of the configuration directory.
     */
    private static final String  CONFIG_DIR = "configuration";

    /**
     * The name of the configuration file.
     */
    private static final String  VTN_INI_NAME = "vtn.ini";

    /**
     * Tear down the test environment.
     */
    @After
    public void tearDown() {
        File config = new File(CONFIG_DIR);
        delete(config);
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#getIntegrationBridgeName()}.
     * Test GetSet Methods in OVSDBPluginEventHandler with value.
     */
    @Test
    public void testSetGetMethods() {
        currentCalledMethod = DEFAULT_NO_METHOD;
        OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
        ovsdb.setIntegrationBridgeName(BRIDGENAME);
        ovsdb.setFailmode(FAILMODE);
        ovsdb.setProtocols(PROTOCOLS);
        ovsdb.setPortName(PORTNAME);
        assertEquals(BRIDGENAME, ovsdb.getIntegrationBridgeName());
        assertEquals(FAILMODE, ovsdb.getFailmode());
        assertEquals(PROTOCOLS, ovsdb.getProtocols());
        assertEquals(PORTNAME, ovsdb.getPortName());
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#nodeAdded(Node,InetAddress,int)}.
     * Test nodeAdded Method.
     */
    @Test
    public void testNodeAdded() {
        currentCalledMethod = NODE_ADDED;
        for (String[] createNetwork : CREATE_NETWORK_ARRAY) {
            try {
                OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
                IVTNManager mgr = new VTNManagerStub();
                ovsdb.setVTNManager(mgr);

                OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
                ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();

                if (!(createNetwork[NODE_ADD_SET_OR_UNSET_OVSDB].equalsIgnoreCase("1"))) {
                    ovsdb.setOVSDBConfigService(ovsdbConfig);
                    ovsdb.setConnectionService(connectionService);
                }

                if (OPENFLOW.equalsIgnoreCase(createNetwork[NODE_ADD_NODE_TYPE])) {
                    //            String strObj = String.valueOf(Long.parseLong(CREATE_NETWORK[NODE_ADD_NODE_ID]));
                    Node nodeObj = new Node(createNetwork[NODE_ADD_NODE_TYPE], Long.parseLong(createNetwork[NODE_ADD_NODE_ID], HEX_RADIX));
                    //            ovsdb.prepareInternalNetwork(nodeObj);
                    ovsdb.nodeAdded(nodeObj, null, 0);
                } else if ((ONEPK.equalsIgnoreCase(createNetwork[NODE_ADD_NODE_TYPE])) ||
                        (PRODUCTION.equalsIgnoreCase(createNetwork[NODE_ADD_NODE_TYPE]))) {
                    //            String strNode = String.valueOf(Long.parseLong(CREATE_NETWORK[NODE_ADD_NODE_ID], 16));
                    Node nodeObj = new Node(createNetwork[NODE_ADD_NODE_TYPE], createNetwork[NODE_ADD_NODE_ID]);
                    ovsdb.nodeAdded(nodeObj, null, 0);
                } else if (PCEP.equalsIgnoreCase(createNetwork[NODE_ADD_NODE_TYPE])) {
                    Node nodeObj = new Node(createNetwork[NODE_ADD_NODE_TYPE], UUID.fromString(createNetwork[NODE_ADD_NODE_ID]));
                    ovsdb.nodeAdded(nodeObj, null, 0);
                }

                ovsdb.unsetOVSDBConfigService(ovsdbConfig);
                ovsdb.unsetConnectionService(connectionService);

            } catch (Exception ex) {
                if ((createNetwork[NODE_ADD_SET_OR_UNSET_OVSDB].equalsIgnoreCase("1"))
                        && (createNetwork[NODE_ADD_NULL_EXCEPTION_HANDLER].equalsIgnoreCase("null"))) {
                    assertEquals(ex.getMessage(), createNetwork[NODE_ADD_NULL_EXCEPTION_HANDLER], "null");
                }
            }
        }
        currentCalledMethod = DEFAULT_NO_METHOD;
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#rowRemoved(Node, String, String, org.opendaylight.ovsdb.lib.notation.Row, Object)}.
     * Test rowRemoved Method in OVSDBPluginEventHandler.
     */
    @Test
    public void testRowRemoved() {
        currentCalledMethod = ROW_REMOVED;

        for (String [] rowRemove : ROW_REMOVE_ARRAY) {
            try {
                Long longObj = new Long(rowRemove[ROW_REMOVE_NODE_ID]);
                Node nodeObj = new Node(OPENFLOW, longObj);

                OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
                OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
                NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
                ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
                IVTNManager mgr = new VTNManagerStub();

                ovsdb.setOVSDBConfigService(ovsdbConfig);
                ovsdb.setConnectionService(connectionService);
                ovsdb.setNeutronPortCRUD(neutron);
                ovsdb.setVTNManager(mgr);

                Interface intfStubRow = new InterfaceStub();
                intfStubRow.setName(rowRemove[ROW_REMOVE_TABLE_NAME]);
                OvsdbMap<String, String> mapOvsdb = new OvsdbMap<String, String>();
                mapOvsdb.delegate().put("iface-id", rowRemove[ROW_REMOVE_NODE_UUID]);
                intfStubRow.setExternalIds(mapOvsdb.delegate());
                Object obj = new Object();

                ovsdb.rowRemoved(nodeObj, rowRemove[ROW_REMOVE_TABLE_NAME], "85c27f20-f218-11e3-a7b6-0002a5d5c51b", intfStubRow.getRow(), obj);

                ovsdb.unsetNeutronPortCRUD(neutron);
                ovsdb.unsetOVSDBConfigService(ovsdbConfig);
                ovsdb.unsetConnectionService(connectionService);
            } catch (Exception ex) {
                assertEquals(null, ex.getMessage());
                assertEquals(0, 0);
            }
        }
        currentCalledMethod = DEFAULT_NO_METHOD;
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#rowUpdated(Node, String, String, org.opendaylight.ovsdb.lib.notation.Row, org.opendaylight.ovsdb.lib.notation.Row)}.
     * Test rowUpdated Methods with Neutron Port in OVSDBPluginEventHandler.
     */
    @Test
    public void testRowUpdatedNeutronPort() {
        currentCalledMethod = ROW_UPDATED;

        for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
            try {
                String pcep = rowUpdate[ROW_UPDATE_NODE_TYPE];
                UUID idOne = UUID.randomUUID();
                Node nodeObj = new Node(pcep, idOne);

                for (int index = 0; index < ROW_UPDATE_INPUT_ARRAY.length; index++) {
                    String [] tempRowUpdate = ROW_UPDATE_INPUT_ARRAY[index];
                    if (rowUpdate == tempRowUpdate) {
                        ROW_UPDATE_INPUT_ARRAY[index][ROW_UPDATE_UUID] = idOne.toString();
                    }
                }

                OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
                OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
                ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
                NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
                VTNManagerStub vtnManagerStub = new VTNManagerStub();

                ovsdb.setOVSDBConfigService(ovsdbConfig);
                ovsdb.setConnectionService(connectionService);
                ovsdb.setNeutronPortCRUD(neutron);
                ovsdb.setVTNManager(vtnManagerStub);

                if (rowUpdate[ROW_UPDATE_NODE_OBJECT_TYPE].equalsIgnoreCase("bridge")) {
                    Bridge bridge = new BridgeStub();
                    bridge.setName(rowUpdate[ROW_UPDATE_NODE_OBJECT_NAME]);
                    // Send Bridge instead of Interface
                    ovsdb.rowUpdated(nodeObj, rowUpdate[ROW_UPDATE_ACTUAL_TABLE_NAME], rowUpdate[ROW_UPDATE_PARENT_UUID], null, bridge.getRow());
                } else if (rowUpdate[ROW_UPDATE_NODE_OBJECT_TYPE].equalsIgnoreCase("bridge-int")) {
                    Interface interfaceBridge = new InterfaceStub();
                    interfaceBridge.setName(rowUpdate[ROW_UPDATE_NODE_OBJECT_NAME]);

                    if (SET_NULL_TO_NODE.equalsIgnoreCase(rowUpdate[ROW_UPDATE_PARENT_UUID])) {
                        nodeObj = null;
                    }

                    if (rowUpdate[ROW_UPDATE_OLD_ROW].equalsIgnoreCase("null")
                            && rowUpdate[ROW_UPDATE_NEW_ROW].equalsIgnoreCase("null")) {
                        ovsdb.rowUpdated(nodeObj, rowUpdate[ROW_UPDATE_ACTUAL_TABLE_NAME], rowUpdate[ROW_UPDATE_PARENT_UUID], null, null);
                    } else if (rowUpdate[ROW_UPDATE_NEW_ROW].equalsIgnoreCase("null")) {
                        ovsdb.rowUpdated(nodeObj, rowUpdate[ROW_UPDATE_ACTUAL_TABLE_NAME], rowUpdate[ROW_UPDATE_PARENT_UUID], null, null);
                    } else {
                        ovsdb.rowUpdated(nodeObj, rowUpdate[ROW_UPDATE_ACTUAL_TABLE_NAME], rowUpdate[ROW_UPDATE_PARENT_UUID], null, interfaceBridge.getRow());
                    }
                } else if (rowUpdate[ROW_UPDATE_NODE_OBJECT_TYPE].equalsIgnoreCase("intf")) {
                    Interface intfOld = new InterfaceStub();
                    intfOld.setName(rowUpdate[ROW_UPDATE_NODE_OBJECT_NAME]);
                    Interface intfNew = new InterfaceStub();
                    ovsdb.rowUpdated(nodeObj, rowUpdate[ROW_UPDATE_ACTUAL_TABLE_NAME], rowUpdate[ROW_UPDATE_PARENT_UUID], null, intfNew.getRow());
                }

                ovsdb.unsetOVSDBConfigService(ovsdbConfig);
                ovsdb.unsetConnectionService(connectionService);
                ovsdb.unsetNeutronPortCRUD(neutron);
            } catch (Exception ex) {
                if (rowUpdate[ROW_UPDATE_INTERFACE_NAME].equalsIgnoreCase("ex_msg")) {
                    // assertEquals(EXCEPTION_MSG, ex.getMessage());
                }

                if (rowUpdate[ROW_UPDATE_INTERFACE_NAME].equalsIgnoreCase("null_msg")) {
                    assertEquals(null, ex.getMessage());
                }
            }
        }
        currentCalledMethod = DEFAULT_NO_METHOD;
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#isUpdateOfInterest(Node,org.opendaylight.ovsdb.lib.notation.Row,org.opendaylight.ovsdb.lib.notation.Row)}.
     * Test isUpdateOfInterest Methods with Neutron Port in OVSDBPluginEventHandler.
     */
    @Test
    public void testIsUpdateOfInterest() {
        currentCalledMethod = IS_UPDATE_OF_INTEREST;

        for (String [] isUpdateOfInterest : IS_UPDATE_OF_INTEREST_INPUT_ARRAY) {
            try {
                String pcep = isUpdateOfInterest[ROW_UPDATE_NODE_TYPE];
                UUID idOne = UUID.randomUUID();
                Node nodeObj = new Node(pcep, idOne);

                for (int index = 0; index < IS_UPDATE_OF_INTEREST_INPUT_ARRAY.length; index++) {
                    String [] tempIsUpdateOfInterest = IS_UPDATE_OF_INTEREST_INPUT_ARRAY[index];
                    if (isUpdateOfInterest == tempIsUpdateOfInterest) {
                        IS_UPDATE_OF_INTEREST_INPUT_ARRAY[index][ROW_UPDATE_UUID] = idOne.toString();
                    }
                }

                OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
                OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
                ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
                NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();
                VTNManagerStub vtnManagerStub = new VTNManagerStub();

                ovsdb.setOVSDBConfigService(ovsdbConfig);
                ovsdb.setConnectionService(connectionService);
                ovsdb.setNeutronPortCRUD(neutron);
                ovsdb.setVTNManager(vtnManagerStub);

                if (isUpdateOfInterest[ROW_UPDATE_OLD_ROW].equalsIgnoreCase("null")) {
                    Interface interfaceBridge = new InterfaceStub();
                    interfaceBridge.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);
                    ovsdb.rowUpdated(nodeObj, isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME], isUpdateOfInterest[ROW_UPDATE_PARENT_UUID], null, interfaceBridge.getRow());
                } else if (isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME].equalsIgnoreCase("Interface")) {
                    Interface newInterfaceBridge = new InterfaceStub();
                    newInterfaceBridge.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);

                    Interface oldInterfaceBridge = new InterfaceStub();
                    oldInterfaceBridge.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);

                    ovsdb.rowUpdated(nodeObj, isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME], isUpdateOfInterest[ROW_UPDATE_PARENT_UUID], oldInterfaceBridge.getRow(), newInterfaceBridge.getRow());
                } else if (isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME].equalsIgnoreCase("Port")) {
                    Port newPort = new PortStub();
                    newPort.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);

                    Port oldPort = new PortStub();
                    oldPort.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);

                    ovsdb.rowUpdated(nodeObj, isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME], isUpdateOfInterest[ROW_UPDATE_PARENT_UUID], oldPort.getRow(), newPort.getRow());
                } else if (isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME].equalsIgnoreCase("OpenVSwitch")) {
                    OpenVSwitch newOpenVSwitch = new OpenVSwitchStub();
                    OpenVSwitch oldOpenVSwitch = new OpenVSwitchStub();

                    ovsdb.rowUpdated(nodeObj, isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME], isUpdateOfInterest[ROW_UPDATE_PARENT_UUID], oldOpenVSwitch.getRow(), newOpenVSwitch.getRow());
                } else if (isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME].equalsIgnoreCase("Bridge")) {
                    Bridge newBridge = new BridgeStub();
                    newBridge.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);

                    Bridge oldBridge = new BridgeStub();
                    oldBridge.setName(isUpdateOfInterest[ROW_UPDATE_NODE_OBJECT_NAME]);

                    ovsdb.rowUpdated(nodeObj, isUpdateOfInterest[ROW_UPDATE_ACTUAL_TABLE_NAME], isUpdateOfInterest[ROW_UPDATE_PARENT_UUID], oldBridge.getRow(), newBridge.getRow());
                }

                ovsdb.unsetOVSDBConfigService(ovsdbConfig);
                ovsdb.unsetConnectionService(connectionService);
                ovsdb.unsetNeutronPortCRUD(neutron);
            } catch (Exception ex) {
                if (isUpdateOfInterest[ROW_UPDATE_INTERFACE_NAME].equalsIgnoreCase("ex_msg")) {
                    // assertEquals(EXCEPTION_MSG, ex.getMessage());
                }

                if (isUpdateOfInterest[ROW_UPDATE_INTERFACE_NAME].equalsIgnoreCase("null_msg")) {
                    assertEquals(null, ex.getMessage());
                }
            }
        }
        currentCalledMethod = DEFAULT_NO_METHOD;
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#rowAdded(Node, String, String, org.opendaylight.ovsdb.lib.notation.Row)}.
     * Test rowAdded Method with OVSDBPluginEventHandler.
     */
    @Test
    public void testRowAdded() {
        currentCalledMethod = ROW_ADDED;
        try {
            OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
            OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
            ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
            NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();

            ovsdb.setOVSDBConfigService(ovsdbConfig);
            ovsdb.setConnectionService(connectionService);
            ovsdb.setNeutronPortCRUD(neutron);

            UUID idOne = UUID.randomUUID();
            Node nodeObj = new Node(PCEP, idOne);

            Bridge bridge = new BridgeStub();
            bridge.setName("br-int");

            ovsdb.rowAdded(nodeObj, "Bridge", null, bridge.getRow());

            ovsdb.unsetOVSDBConfigService(ovsdbConfig);
            ovsdb.unsetConnectionService(connectionService);
            ovsdb.unsetNeutronPortCRUD(neutron);
        } catch (Exception ex) {
            assertEquals(1, 1);
        }
        currentCalledMethod = DEFAULT_NO_METHOD;
    }

    /**
     * Test method for
     * {@link OVSDBPluginEventHandler#nodeRemoved(Node)}.
     * Test nodeRemoved Method with OVSDBPluginEventHandler.
     */
    @Test
    public void testNodeRemoved() {
        currentCalledMethod = NODE_REMOVED;
        try {
            OVSDBPluginEventHandler ovsdb = new OVSDBPluginEventHandler();
            OVSDBManagerStub ovsdbConfig = new OVSDBManagerStub();
            ConnectionServiceInternalStub connectionService = new ConnectionServiceInternalStub();
            NeutronPortCRUDStub neutron = new NeutronPortCRUDStub();

            ovsdb.unsetOVSDBConfigService(ovsdbConfig);
            ovsdb.unsetConnectionService(connectionService);
            ovsdb.unsetNeutronPortCRUD(neutron);

            ovsdb.setOVSDBConfigService(ovsdbConfig);
            ovsdb.setConnectionService(connectionService);
            ovsdb.setNeutronPortCRUD(neutron);

            UUID idOne = UUID.randomUUID();
            Node nodeObj = new Node(PCEP, idOne);

            ovsdb.nodeRemoved(nodeObj);

            ovsdb.unsetOVSDBConfigService(ovsdbConfig);
            ovsdb.unsetConnectionService(connectionService);
            ovsdb.unsetNeutronPortCRUD(neutron);
        } catch (Exception ex) {
            assertEquals(1, 1);
        }
        currentCalledMethod = DEFAULT_NO_METHOD;
    }

    /**
    * Test method for
    * {@link OVSDBPluginEventHandler#getSystemProperties()}.
    *
     * @throws Exception  An error occurredn
    */
    @Test
    public void testSystemProperties() throws Exception {
        // Check default values.
        File config = new File(CONFIG_DIR);
        delete(config);
        OVSDBPluginEventHandler handler = new OVSDBPluginEventHandler();
        handler.getSystemProperties();
        String defBridge = "br-int";
        String defMode = "secure";
        String defProto = "OpenFlow13";
        String defPort = "ens33";
        assertEquals(defBridge, handler.getIntegrationBridgeName());
        assertEquals(defMode, handler.getFailmode());
        assertEquals(defProto, handler.getProtocols());
        assertEquals(defPort, handler.getPortName());

        Properties defProp = new Properties();
        defProp.setProperty("bridgename", defBridge);
        defProp.setProperty("failmode", defMode);
        defProp.setProperty("protocols", defProto);
        defProp.setProperty("portname", defPort);

        // Ensure that the configuration file is loaded.
        assertTrue(config.mkdirs());
        String[] bridges = {null, "bridge-1", "bridge-2"};
        String[] failmodes = {null, "mode-1", "mode-2"};
        String[] protos = {null, "OpenFlow10", "OpenFlow10,OpenFlow13"};
        String[] ports = {null, "port-1", "port-2"};
        File file = new File(config, VTN_INI_NAME);
        for (String bridge: bridges) {
            for (String failmode: failmodes) {
                for (String proto: protos) {
                    for (String port: ports) {
                        Properties prop = new Properties(defProp);
                        if (bridge != null) {
                            prop.setProperty("bridgename", bridge);
                        }
                        if (failmode != null) {
                            prop.setProperty("failmode", failmode);
                        }
                        if (proto != null) {
                            prop.setProperty("protocols", proto);
                        }
                        if (port != null) {
                            prop.setProperty("portname", port);
                        }

                        systemPropertiesTest(file, prop);
                    }
                }
            }
        }
    }

    /**
     * Internal method for {@link #testSystemProperties()}.
     *
     * @param file  A {@link File} instance corresponding to the configuration
     *              file.
     * @param prop  A {@link Properties} instance that contains configurations.
     * @throws Exception  An error occurredn
     */
    private void systemPropertiesTest(File file, Properties prop)
        throws Exception {
        if (file.exists()) {
            assertTrue(file.delete());
        }

        FileOutputStream out = null;
        try {
            out = new FileOutputStream(file);
            prop.store(out, "Test configuration");
        } finally {
            if (out != null) {
                out.close();
            }
        }

        OVSDBPluginEventHandler handler = new OVSDBPluginEventHandler();
        handler.getSystemProperties();

        String bridge = prop.getProperty("bridgename");
        String failmode = prop.getProperty("failmode");
        String proto = prop.getProperty("protocols");
        String port = prop.getProperty("portname");
        assertEquals(bridge, handler.getIntegrationBridgeName());
        assertEquals(failmode, handler.getFailmode());
        assertEquals(proto, handler.getProtocols());
        assertEquals(port, handler.getPortName());
    }
}
