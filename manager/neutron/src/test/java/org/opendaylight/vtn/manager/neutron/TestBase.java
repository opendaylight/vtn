/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import org.junit.Assert;

/**
 * Abstract base class for JUnit tests.
 */
public abstract class TestBase extends Assert {
    /**
     * String Declaration for OpenFlow.
     */
    protected static final String OPENFLOW = "OF";

    /**
     * String Declaration for Production.
     */
    protected static final String PRODUCTION = "PR";

    /**
     * String Declaration for Onepk.
     */
    protected static final String ONEPK = "PK";

    /**
     * String Declaration for Pcep.
     */
    protected static final String PCEP = "PE";

    /**
     * String Declaration for Bridge Name.
     */
    protected static final String BRIDGENAME = "br-int";

    /**
     * String Declaration for Fail Mode.
     */
    protected static final String FAILMODE = "secure";

    /**
     * String Declaration for Protcols.
     */
    protected static final String PROTOCOLS = "OpenFlow10";

    /**
     * String Declaration for Port Name.
     */
    protected static final String PORTNAME = "ens33";

    /**
     * Integer Declaration for identifying the Calling method for Default case.
     */
    protected static final int DEFAULT_NO_METHOD = 0;

    /**
     * Integer Declaration for identifying the Calling method of NodeAdded.
     */
    protected static final int NODE_ADDED = 1;

    /**
     * Integer Declaration for identifying the Calling method of NodeRemoved.
     */
    protected static final int NODE_REMOVED = 2;

    /**
     * Integer Declaration for identifying the Calling method of RowAdded.
     */
    protected static final int ROW_ADDED = 3;

    /**
     * Integer Declaration for identifying the Calling method of RowUpdated.
     */
    protected static final int ROW_UPDATED = 4;

    /**
     * Integer Declaration for identifying the Calling method of RowRemoved.
     */
    protected static final int ROW_REMOVED = 5;

    /**
     * Integer Declaration for identifying the Called method.
     */
    protected static int currentCalledMethod = DEFAULT_NO_METHOD;

    /**
     * String Declaration for identifying the Bridge with below UUID.
     */
    protected static final String BRIDGE_UUID_1 = "44b72e5b15cfcddafccbc9c6f04db4a";

    /**
     * String Declaration for identifying the Bridge with below UUID.
     */
    protected static final String BRIDGE_UUID_2 = "44b72e5b15cfcddafccbc9c6f04db4b";

    /**
     * String Declaration for identifying the Interface with below Parent_UUID.
     */
    protected static final String INTERFACE_PARENT_UUID = "9b2b6560-f21e-11e3-a6a2-0002a5d5c51d";

    /**
     * String Declaration for identifying the Port with below Parent_UUID.
     */
    protected static final String PORT_PARENT_UUID = "9b2b6560-f21e-11e3-a6a2-0002a5d5c51e";

    /**
     * UUID node field starting byte length.
     */
    protected static final int UUID_POSITION_IN_NODE = 24;

    /**
     * Neutron UUID identifier length.
     */
    protected static final int UUID_LEN = 36;

    /**
     * Hex radix.
     */
    protected static final int HEX_RADIX = 16;

    /*
     *Magic numbers for Array index - ROW_UPDATE_INPUT_ARRAY
     */
    // NodeType index
    public static final int ROW_UPDATE_NODE_TYPE = 0;

    // NeutronObjectType index
    public static final int ROW_UPDATE_NODE_OBJECT_TYPE = 1;

    // NeutronObjectName index
    public static final int ROW_UPDATE_NODE_OBJECT_NAME = 2;

    // ActualTableName index
    public static final int ROW_UPDATE_ACTUAL_TABLE_NAME = 3;

    // UUID(Set by testing method) index
    public static final int ROW_UPDATE_UUID = 4;

    // ParentUUID index
    public static final int ROW_UPDATE_PARENT_UUID = 5;

    // OldRow index
    public static final int ROW_UPDATE_OLD_ROW = 6;

    // NewRow index
    public static final int ROW_UPDATE_NEW_ROW = 7;

    // Exception index
    public static final int ROW_UPDATE_EXCEPTION = 8;

    // InterfaceName index
    public static final int ROW_UPDATE_INTERFACE_NAME = 9;

    // InterfacePortId index
    public static final int ROW_UPDATE_INTERFACE_PORT_ID = 10;

    /*
     *Magic numbers for Array index - CREATE_NETWORK_ARRAY
     */
    // NodeType index
    public static final int NODE_ADD_NODE_TYPE = 0;

    // NodeID index
    public static final int NODE_ADD_NODE_ID = 1;

    // SetOrUnsetOVSDB index
    public static final int NODE_ADD_SET_OR_UNSET_OVSDB = 2;

    // NULL exception handler index
    public static final int NODE_ADD_NULL_EXCEPTION_HANDLER = 3;

    /*
     *Magic numbers for Array index - ROW_REMOVE_ARRAY
     */
    // NodeID index
    public static final int ROW_REMOVE_NODE_ID = 0;

    // TableName index
    public static final int ROW_REMOVE_TABLE_NAME = 1;

    // NodeUUID index
    public static final int ROW_REMOVE_NODE_UUID = 2;

    // SetProperties index
    public static final int ROW_REMOVE_SET_PROPERTIES = 3;

    /**
     * An array of UUIDs generated by Neutron.
     */
    protected static final String[] NEUTRON_UUID_ARRAY = {
        "C387EB44-7832-49F4-B9F0-D30D27770883",
        "4790F3C1-AB34-4ABC-B7A5-C1B5C7202389",
        "52B1482F-A41E-409F-AC68-B04ACFD07779",
        "6F3FCFCF-C000-4879-84C5-19157CBD1F6A",
        "0D2206F8-B700-4F78-913D-9CE7A2D78473"};

    /**
     * An array of tenat IDs generated by Keystone.
     */
    protected static final String[] TENANT_ID_ARRAY = {
        "E6E005D3A24542FCB03897730A5150E2",
        "B37B50456AE848DF8F981058FDD3A63D",
        "3655F990CC5348F2A668F77896D9D017",
        "E20F7C4511144D9BAEB844A4964B60DA",
        "5FB6A2BB77714CE6BF33282283245705"};

    /**
     * An array of MAC Addresses.
     */
    protected static final String[] MAC_ADDR_ARRAY = {
        "EA:75:F6:3A:30:50",
        "20:C1:F2:C0:A5:BF",
        "33:C2:B0:C3:B4:40",
        "9B:D9:F0:C4:31:B1",
        "70:A8:A6:C2:FA:D2"};
    /**
     * An array of elements to update row.
     */
    protected static final String[][] ROW_UPDATE_INPUT_ARRAY = {
        // Inputs to the function -
        // NodeType, NeutronObjectType, NeutronObjectName, ActualTableName, UUID(Set by testing method),
        // ParentUUID, OldRow,
        // NewRow, Exception, InterfaceName, InterfacePortId

        // Fail case - as Bridge
        {"PE", "bridge", "br-int", "Bridge", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null",
         "not_null", "ex_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Neutron Port is Null by setting wrong InterfacePortId
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null",
         "not_null", "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770881"},

        // Fail case for No switch is associated with interface, Node or interface Uuid is Null by setting UUID to null
        {"PE", "bridge-int", "br-int", "Interface", "",
         null, "null",
         "null", "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Set Port mapping failed for interface
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null",
         "not_null", "no_ex_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Success case
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null",
         "not_null", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

         // Test case for the method - OVSDBPluginEventHandler.isUpdateOfInterest(Node, Row, Row)
         // OldRow - Null
        {"PE", "intf-neutron", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51c", "null",
         "not_null", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

         // Interface - Not working
        {"PE", "intf-neutron", "interface1", "Interface", "",
         INTERFACE_PARENT_UUID, "not_null",
         "not_null", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

         // Port - Not working
        {"PE", "intf-neutron", "interface1", "Port", "",
         PORT_PARENT_UUID, "not_null",
         "not_null", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

         // OpenVSwitch - Not Possible case
        {"PE", "intf-neutron", "interface1", "OpenVSwitch", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51f", "not_null",
         "not_null", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"}};

    /**
     * Exception message to be checked.
     */
    protected static final String EXCEPTION_MSG =
            "org.opendaylight.ovsdb.lib.table.Bridge cannot be cast to org.opendaylight.ovsdb.lib.table.Interface";

    /**
     * An array of elements to update row with type OF.
     */
    protected static final String[][] RW_UPDT_INP_OF_ARY = {
        {"Set_OF_neutron", "0", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "12344321", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "12344321", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", null,
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "12344321", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Bridge",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "12345678", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "12345678", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "", "0", "0", "no_ex_msg", "1", "0" }  ,

        {"Set_OF_neutron", "23456789", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "85c27f20-f218-11e3-a7b6-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "23456789", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "c09b7fc0-f218-11e3-bf2f-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "23456789", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         null,
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_neutron", "98765432", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "85c27f20-f218-11e3-a7b6-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" },

        {"Set_OF_Null", "12345678", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "0d0ff2a0-f219-11e3-a482-0002a5d5c51b",
         "1", "0", "null_msg", "0", "0" },

        {"Set_OF_Bridge", "98765432", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "85c27f20-f218-11e3-a7b6-0002a5d5c51b",
         "1", "0", "ex_msg", "0", "0" },

        {"Set_OF_Bridge_intf", "98765432", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "85c27f20-f218-11e3-a7b6-0002a5d5c51b",
         "1", "0", "ex_msg", "0", "0" },

        {"Set_OF_neutron", "12345678", "interface1", "iface-id",
         "C387EB44-7832-49F4-B9F0-D30D27770883", "Interface",
         "85c27f20-f218-11e3-a7b6-0002a5d5c51b",
         "0", "0", "no_ex_msg", "1", "0" } };

    /**
     * An array of elements to create network.
     */
    protected static final String[][] CREATE_NETWORK_ARRAY = {
        // NodeType, NodeID, 1-UnsetOVSDB and 0-SetOVSDB, NULL Exception handler
        // Null exception case at getInternalBridgeUUID method when OVSDBConfigService is NULL
        {"OF", "11111110", "1", "null" },
        // Update case in Node added method
        {"OF", "11111111", "0", "" },
        // New NODE added case - Successful
        {"OF", BRIDGE_UUID_1, "0", "" },
        // returns Null beacuse in getInternalBridgeUUID method bridge is NULL when calling OVSDBConfigService.getRows
        {"PK", "11111113", "0", "" },
        // returns Null beacuse in getInternalBridgeUUID method bridge is NULL when calling OVSDBConfigService.getRows
        {"PR", "11111114", "0", "" },
        // Update case in Node added method
        {"PE", BRIDGE_UUID_2, "0", "" },
        // New NODE added case - Successful
        {"PE", "11111116", "0", "" }
    };

    /**
     * An array of elements to remove row.
     */
    protected static final String[][] ROW_REMOVE_ARRAY = {
        //   NodeId, TableName, NodeID, 0-Set ExternalIDS/1-UnSetExternalIds/2-SetExternalIdsAndUnsetPortId/3-NotAnInterface
        {"22222220", "Not an Interface", "C387EB44-7832-49F4-B9F0-D30D27770883", "0"},
        {"22222221", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78473", "3"},
        {"22222222", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78473", "1"},
        {"22222223", "Interface",        "C387EB44-7832-49F4-B9F0-D30D27770883", "2"},
        {"22222224", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78473", "0"},
    };

    /**
     * An array of elements to update row.
     */
    protected static final String[][] ROW_UPDATE_PORT_ARRAY = {
        {"OF", "56456644", "interface1", "C387EB44-7832-49F4-B9F0-D30D27770883", "set_port", "", "0" },
        {"OF", "56456644", "interface1", "C387EB44-7832-49F4-B9F0-D30D27770883", "set_null_port", "", "0" },
        {"OF", "56456644", "interface1", "C387EB44-7832-49F4-B9F0-D30D27770883", "set_empty_port", "", "0" },
        {"OF", "12345678", "interface1", "C387EB44-7832-49F4-B9F0-D30D27770883", "without_port", "65534", "0" },
        {"OF", "56456644", "interface1", "4790F3C1-AB34-4ABC-B7A5-C1B5C7202389", "", "65534", "0" },
        {"OF", "56456644", "interface1", "52B1482F-A41E-409F-AC68-B04ACFD07779", "", "65534", "0" },
        {"OF", "56456644", "interface1", "8c781fc0-f215-11e3-aac3-0002a5d5c51b", "", "65534", "0" },
        {"OF", "56456644", "interface1", "0D2206F8-B700-4F78-913D-9CE7A2D78473", "", "65534", "0" },
    };
}
