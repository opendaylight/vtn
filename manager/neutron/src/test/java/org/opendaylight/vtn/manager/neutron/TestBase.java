/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.io.File;

import org.junit.Assert;
import java.util.UUID;

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
     * Integer Declaration for identifying the Calling method of isUpdateOfInterest.
     */
    protected static final int IS_UPDATE_OF_INTEREST = 6;
    /**
     * Integer Declaration for identifying the Called method.
     */
    protected static int currentCalledMethod = DEFAULT_NO_METHOD;

    /**
     * String Declaration for identifying the Bridge with below NodeID.
     */
    protected static final String NODE_ID_1 = "D30D27770881";

    /**
     * String Declaration for identifying the Bridge with below NodeID.
     */
    protected static final String NODE_ID_2 = UUID.randomUUID().toString();

    /**
     * String Declaration for identifying the Interface with below Parent_UUID.
     */
    protected static final String INTERFACE_PARENT_UUID = "9b2b6560-f21e-11e3-a6a2-0002a5d5c51d";

    /**
     * String Declaration for identifying the Port with below Parent_UUID.
     */
    protected static final String PORT_PARENT_UUID = "9b2b6560-f21e-11e3-a6a2-0002a5d5c51e";

    /*
     * String declaration for identifying the conflicted NetworkUUID
     */
    public static final String CONFLICTED_NETWORK_UUID = "5e7e0900f2151e3aa760002a5d5c51c";

    /*
     * String declaration for setting Node to NULL for the below UUID
     */
    public static final String SET_NULL_TO_NODE = "5e7e0900f2151e3aa760002a5d5c51d";

    /**
     * String Declaration for setting the OFPortArray to NULL.
     */
    protected static final String OF_PORT_ARRAY_IS_NULL = "OF_PORT_ARRAY_IS_NULL";

    /**
     * String Declaration for setting the OFPortArray to Empty array.
     */
    protected static final String OF_PORT_ARRAY_IS_EMPTY = "OF_PORT_ARRAY_IS_EMPTY";

    /**
     * String Declaration for setting the Port Object to NULL.
     */
    protected static final String SET_PORT_OBJECT_TO_NULL = "SET_PORT_OBJECT_TO_NULL";

    /**
     * String Declaration for setting the Port Object to Empty Set.
     */
    protected static final String SET_PORT_OBJECT_TO_EMPTY_SET = "SET_PORT_OBJECT_TO_EMPTY_SET";

    /**
     * String Declaration for setting the Port Object with wrong UUID.
     */
    protected static final String SET_PORT_OBJECT_WITH_WRONG_UUID = "SET_PORT_OBJECT_WITH_WRONG_UUID";

    /**
     * String Declaration for setting the Bridge object's DatapathIdColumn to NULL.
     */
    protected static final String SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_NULL = "SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_NULL";

    /**
     * String Declaration for setting the Bridge object's DatapathIdColumn to Empty Set.
     */
    protected static final String SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_EMPTY_SET = "SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_EMPTY_SET";

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
     *Magic numbers for Array index - ROW_UPDATE_INPUT_ARRAY and IS_UPDATE_OF_INTEREST_INPUT_ARRAY
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

    // Option index
    public static final int ROW_UPDATE_OPTION = 8;

    // Exception index
    public static final int ROW_UPDATE_EXCEPTION = 9;

    // InterfaceName index
    public static final int ROW_UPDATE_INTERFACE_NAME = 10;

    // InterfacePortId index
    public static final int ROW_UPDATE_INTERFACE_PORT_ID = 11;

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
        // ParentUUID, OldRow, NewRow,
        // Option, Exception, InterfaceName, InterfacePortId

        // Fail case - as Bridge
        {"PE", "bridge", "br-int", "Bridge", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         "", "ex_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Neutron Port is Null by setting wrong InterfacePortId
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         "", "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770881"},

        // Fail case for No switch is associated with interface, Node or interface Uuid is Null by setting Node to null
        // At present case, the below case failes
        {"PE", "bridge-int", "br-int", "Interface", "",
         SET_NULL_TO_NODE, "null", "null",
         "", "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

         // Fail case for No switch is associated with interface, Node or interface Uuid is Null by setting UUID to null
        {"PE", "bridge-int", "br-int", "Interface", "",
         null, "null", "null",
         "", "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Port is Null in the method-getSwitchIdFromInterface(), by setting Port to NULL
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "null", "null",
         SET_PORT_OBJECT_TO_NULL, "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Exception {} while getting switch identifier in the method-getSwitchIdFromInterface(), by setting InterfaceColumn Set to Empty
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "null", "null",
         SET_PORT_OBJECT_TO_EMPTY_SET, "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Failed to get switch Id related to the interface in the method-getSwitchIdFromInterface(), by setting wrong UUID in Port Set
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "null", "null",
         SET_PORT_OBJECT_WITH_WRONG_UUID, "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for data pathid is empty for bridge in the method-getDataPathIdFromBridge(), by setting DatapathIdColumn Set to NULL
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "null", "null",
         SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_NULL, "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for data pathid is empty for bridge in the method-getDataPathIdFromBridge(), by setting DatapathIdColumn Set to Empty
        {"PE", "bridge-int", "br-int", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "null", "null",
         SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_EMPTY_SET, "null_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case - Method getOfPortFormInterface() by setting NULL to OpenFlowPort
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         OF_PORT_ARRAY_IS_NULL, "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

        // Fail case - Method getOfPortFormInterface() by setting Empty to OpenFlowPort
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         OF_PORT_ARRAY_IS_EMPTY, "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

        // Fail case for Set Port mapping failed for interface
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         "", "no_ex_msg", "iface-id", "C387EB44-7832-49F4-B9F0-D30D27770883"},

        // Fail case for Failed to create PortMap in the method-setPortMapForInterface()
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         "", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78474"},

        // Success case
        {"PE", "intf", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51b", "not_null", "not_null",
         "", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"}};

    /**
     * An array of elements to isUpdateOfInterest.
     */
    protected static final String[][] IS_UPDATE_OF_INTEREST_INPUT_ARRAY = {
        // Inputs to the function -
        // NodeType, NeutronObjectType, NeutronObjectName, ActualTableName, UUID(Set by testing method),
        // ParentUUID, OldRow, NewRow,
        // Option, Exception, InterfaceName, InterfacePortId

        // Test case for the method - OVSDBPluginEventHandler.isUpdateOfInterest(Node, Row, Row)
        // OldRow - Null
        {"PE", "intf-neutron", "interface1", "Interface", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51c", "null", "not_null",
         "", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

        // Interface
        {"PE", "intf-neutron", "interface1", "Interface", "",
         INTERFACE_PARENT_UUID, "not_null", "not_null",
         "", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

        // Port
        {"PE", "intf-neutron", "interface1", "Port", "",
         PORT_PARENT_UUID, "not_null", "not_null",
         "", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

        // OpenVSwitch
        {"PE", "intf-neutron", "interface1", "OpenVSwitch", "",
         "9b2b6560-f21e-11e3-a6a2-0002a5d5c51f", "not_null", "not_null",
         "", "no_ex_msg", "iface-id", "0D2206F8-B700-4F78-913D-9CE7A2D78473"},

        // Bridge
        {"PE", "intf-neutron", "interface1", "Bridge", "",
         "", "not_null", "not_null",
         "", "no_ex_msg", "", ""}};

    /**
     * Exception message to be checked.
     */
    protected static final String EXCEPTION_MSG =
            "org.opendaylight.ovsdb.lib.table.Bridge cannot be cast to org.opendaylight.ovsdb.lib.table.Interface";

    /**
     * An array of elements to create network.
     */
    protected static final String[][] CREATE_NETWORK_ARRAY = {
        // NodeType, NodeID, 1-UnsetOVSDB and 0-SetOVSDB, NULL Exception handler
        // Null exception case at getInternalBridgeUUID method when OVSDBConfigService is NULL
        {"OF", "D30D27770880", "1", "null" },
        // Update case in Node added method
        {"OF", NODE_ID_1, "0", "" },
        // New NODE added case - Successful
        {"OF", "D30D27770882", "0", "" },
        // Exception Handler case
        {"PK", "D30D27770883", "0", "" },
        // returns Null beacuse in getInternalBridgeUUID method bridge is NULL when calling OVSDBConfigService.getRows
        {"PR", "D30D27770883", "0", "" },
        // Update case in Node added method
        {"PE", NODE_ID_2, "0", "" },
        // New NODE added case - Successful
        {"PE", UUID.randomUUID().toString(), "0", "" }
    };

    /**
     * An array of elements to remove row.
     * Also Method-getNeutronPortFormInterface(Interface) has been covered
     */
    protected static final String[][] ROW_REMOVE_ARRAY = {
        //   NodeId, TableName, UUID, Status
        // Fail case(Not an Interface) - Not an Interface
        {"22222220", "Not an Interface", "C387EB44-7832-49F4-B9F0-D30D27770883", "0"},
        // Fail case(Interface is NULL) - by setting Status to 2 to get NotAnInterface(returning NULL)
        {"22222221", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78473", "2"},
        // Fail case(External ID for Interface in NULL) - by setting Status to 3 and SET ExternalId to NULL
        {"22222222", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78473", "3"},
        // Fail case(Neutron Port ID is NULL) - by setting Status to 1 and SET ExternalIdsAndUnsetPortId
        {"22222223", "Interface",        "C387EB44-7832-49F4-B9F0-D30D27770883", "1"},
        // Fail case(Neutron Port is NULL) - by setting Status to 0 and SET ExternalId and wrong UUID to get NeutronPort to NULL
        {"22222224", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78483", "0"},
        // Fail case(processRowUpdated getVTNIdentifiers failed) - by setting NodeID and return HTTP_BAD_REQUEST in Method-deletePortMapForInterface()
        {"22222225", "Interface",        "C387EB44-7832-49F4-B9F0-D30D27770883", "0"},
        // Fail case(neutron identifiers not specified) - by setting invalid TennantID, SwitchID, PortID and return HTTP_BAD_REQUEST in Method-getVTNIdentifiers()
        {"22222226", "Interface",        "D387EB44-7832-49F4-B9F0-D30D27770884", "0"},
        // Fail case(neutron identifiers not specified) - by setting invalid SwitchID, PortID and return HTTP_BAD_REQUEST in Method-getVTNIdentifiers()
        {"22222226", "Interface",        "D387EB44-7832-49F4-B9F0-D30D27770885", "0"},
        // Fail case(neutron identifiers not specified) - by setting invalid PortID and return HTTP_BAD_REQUEST in Method-getVTNIdentifiers()
        {"22222226", "Interface",        "C387EB44-7832-49F4-B9F0-D30D27770883", "0"},
        // Fail case(Invalid tenant identifier) - by setting invalid TennantID and return HTTP_BAD_REQUEST in Method-getVTNIdentifiers()
        {"22222227", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78475", "0"},
        // Fail case(Invalid bridge identifier) - by setting invalid NetworkID and return HTTP_BAD_REQUEST in Method-getVTNIdentifiers()
        {"22222228", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78476", "0"},
        // Fail case(Invalid port identifier) - by setting invalid PorttID and return HTTP_BAD_REQUEST in Method-getVTNIdentifiers()
        {"22222229", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78477", "0"},
        // Fail case(Failed to delete PortMap) - by setting NodeID and return status to CONFLICTED in Method-deletePortMapForInterface()
        {"22222230", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78474", "0"},
        // Success case
        {"22222231", "Interface",        "0D2206F8-B700-4F78-913D-9CE7A2D78473", "0"},
    };

    /**
     * Delete the specified directory recursively.
     *
     * @param file  A {@link File} instance which represents a file or
     *              directory to be removed.
     */
    protected static void delete(File file) {
        if (!file.exists()) {
            return;
        }

        File[] files = file.listFiles();
        if (files == null) {
            // Delete the specified file.
            file.delete();
            return;
        }

        // Make the specified directory empty.
        for (File f: files) {
            delete(f);
        }

        // Delete the specified directory.
        file.delete();
    }
}
