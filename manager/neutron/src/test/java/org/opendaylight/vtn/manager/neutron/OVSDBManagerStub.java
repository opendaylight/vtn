/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.sal.core.Node;

import org.opendaylight.ovsdb.lib.notation.OvsdbMap;
import org.opendaylight.ovsdb.lib.notation.OvsdbSet;
import org.opendaylight.ovsdb.lib.notation.Row;
import org.opendaylight.ovsdb.lib.notation.UUID;
import org.opendaylight.ovsdb.lib.schema.GenericTableSchema;
import org.opendaylight.ovsdb.lib.schema.typed.TypedBaseTable;
import org.opendaylight.ovsdb.compatibility.plugin.api.OvsdbConfigurationService;
import org.opendaylight.ovsdb.compatibility.plugin.api.StatusWithUuid;
import org.opendaylight.ovsdb.compatibility.plugin.error.OvsdbPluginException;
import org.opendaylight.ovsdb.schema.openvswitch.Bridge;
import org.opendaylight.ovsdb.schema.openvswitch.Interface;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.ovsdb.compatibility.plugin package.
 */
public class OVSDBManagerStub extends TestBase implements OvsdbConfigurationService {
    /**
     * String Declaration for OpenFlow.
     */
    private static final String OPENFLOW = "OF";
    /**
     * String Declaration for Onepk.
     */
    protected static final String ONEPK = "PK";

    /**
     * String Declaration for Pcep.
     */
    protected static final String PCEP = "PE";

    // Following methods are Used in Unit Test.
    @Override
    public StatusWithUuid insertRow(Node node, String tableName, String parentUUID, Row<GenericTableSchema> row) {
        UUID uu = new UUID("0d0ff2a0-f219-11e3-a482-0002a5d5c51b");
        if (OPENFLOW == node.getType()) {
            return new StatusWithUuid(StatusCode.SUCCESS, uu);
        } else if (ONEPK == node.getType()) {
            return new StatusWithUuid(StatusCode.SUCCESS);
        } else if (PCEP == node.getType()) {
            return new StatusWithUuid(StatusCode.CREATED, uu);
        } else {
            return new StatusWithUuid(StatusCode.UNDEFINED);
        }
    }

    @Override
    public Status updateRow(Node node, String tableName, String parentUUID, String rowUUID, Row row) {
        Long testValue = new Long("66767645");
        Object obj = node.getID();
        if (OPENFLOW.equals(node.getType())) {
            if (testValue == Long.valueOf(obj.toString())) {
                return new Status(StatusCode.SUCCESS);
            } else {
                return new Status(StatusCode.UNDEFINED);
            }
        } else if (ONEPK.equals(node.getType())) {
            return new Status(StatusCode.SUCCESS);
        } else {
            return new Status(StatusCode.UNDEFINED);
        }
    }

    @Override
    public Row getRow(Node node, String tableName, String uuid) {
        Long portIdTest1 = new Long("12345678");
        Long portIdTest2 = new Long("98765432");
        Long portIdTest3 = new Long("12344321");
        Long portIdTest4 = new Long("23456789");
        Long portIdTest5 = new Long("56456644");
        Long portIdTest6 = new Long("66767644");
        Object obj = node.getID();
        //      Object portIdTest3 = new Long(12344321L);

        if (tableName.equals("Port")) {
            if (portIdTest1.equals(Long.valueOf(uuid))) {
                OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
                UUID uu = new UUID("85c27f20-f218-11e3-a7b6-0002a5d5c51b");
                ovsdbset.delegate().add(uu);
                PortStub port = new PortStub();
                port.setInterfaces(ovsdbset.delegate());
                return port.getRow();
            } else if (portIdTest2.equals(Long.valueOf(obj.toString()))) {
                PortStub port = new PortStub();
                return port.getRow();
            } else if (portIdTest3.equals(Long.valueOf(obj.toString()))) {
                PortStub port = new PortStub();
                OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
                port.setInterfaces(ovsdbset.delegate());
                return port.getRow();
            } else if (portIdTest4.equals(Long.valueOf(obj.toString()))) {
                PortStub port = new PortStub();
                OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
                UUID uu = new UUID("c09b7fc0-f218-11e3-bf2f-0002a5d5c51b");
                ovsdbset.delegate().add(uu);
                port.setInterfaces(ovsdbset.delegate());
                return port.getRow();
            } else {
                return null;
            }
        }
        return null;
    }

    @Override
    public ConcurrentMap<String, Row> getRows(Node node, String tableName) {
        ConcurrentMap<String, Row>  rows = new ConcurrentHashMap<>();
        Object obj = node.getID();

        Long nodeIdTest1 = new Long("56456644");
        Long nodeIdTest2 = new Long("66767644");

        if (PCEP == node.getType()) {
            Bridge bridge = new BridgeStub();
            bridge.setName("br-int");
            rows.put("1", bridge.getRow());
            return rows;
        } else if (ONEPK == node.getType()) {
            Bridge bridge = new BridgeStub();
            bridge.setName("br-int");
            return rows;
        } else if (OPENFLOW == node.getType()) {
            if (!(nodeIdTest1.equals(Long.valueOf(obj.toString()))) &&
                    !(nodeIdTest2.equals(Long.valueOf(obj.toString())))) {
                Bridge bridge = new BridgeStub();
                OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
                UUID uu = new UUID("0d0ff2a0-f219-11e3-a482-0002a5d5c51b");
                ovsdbset.delegate().add(uu);
                bridge.setPorts(ovsdbset.delegate());
                bridge.setName("br-int");
                rows.put("1", bridge.getRow());
                return rows;
            } else if (nodeIdTest1.equals(Long.valueOf(obj.toString()))) {
                Bridge bridge = new BridgeStub();
                OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
                OvsdbSet<String> ovsdbdatapath = new OvsdbSet<String>();
                UUID uu = new UUID("0d0ff2a0-f219-11e3-a482-0002a5d5c51b");
                ovsdbset.delegate().add(uu);
                bridge.setPorts(ovsdbset.delegate());
                bridge.setDatapathId(ovsdbdatapath.delegate());
                bridge.setName("br-int");
                rows.put("1", bridge.getRow());
                ovsdbdatapath.delegate().add("str_value");
                Bridge bridgeValueDatapath = new BridgeStub();
                bridgeValueDatapath.setDatapathId(ovsdbdatapath.delegate());
                bridgeValueDatapath.setPorts(ovsdbset.delegate());
                bridgeValueDatapath.setName("br-int");
                rows.put("2", bridgeValueDatapath.getRow());
                return rows;
            } else if (nodeIdTest2.equals(Long.valueOf(obj.toString()))) {
                OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
                OvsdbSet<String> ovsdbdatapath = new OvsdbSet<String>();
                UUID uu = new UUID("0d0ff2a0-f219-11e3-a482-0002a5d5c51b");
                ovsdbset.delegate().add(uu);
                ovsdbdatapath.delegate().add("128");
                Bridge bridgeValueDatapath = new BridgeStub();
                bridgeValueDatapath.setDatapathId(ovsdbdatapath.delegate());
                bridgeValueDatapath.setPorts(ovsdbset.delegate());
                bridgeValueDatapath.setName("br-int");
                rows.put("3", bridgeValueDatapath.getRow());
                return rows;
            }
        } else {
            return null;
        }
        return null;
    }

    @Override
    public Status deleteRow(Node node, String tableName, String rowUUID) {
        return new Status(StatusCode.SUCCESS);
    }

    @Override
    public <T extends TypedBaseTable<?>> T createTypedRow(Node node, Class<T> typedClass) {
        if (typedClass.getSimpleName().equalsIgnoreCase("Bridge")) {
            return (T)new BridgeStub();
        } else if (typedClass.getSimpleName().equalsIgnoreCase("Port")) {
            return (T)new PortStub();
        } else {
            return null;
        }
    }

    @Override
    public <T extends TypedBaseTable<?>> String getTableName(Node node, Class<T> typedClass) {
        if (typedClass != null) {
            return typedClass.getSimpleName();
        }

        return null;
    }

    @Override
    public <T extends TypedBaseTable<?>> T getTypedRow(Node node, Class<T> typedClass, Row row) {
        if (currentCalledMethod == ROW_REMOVED) {
            return validateRowRemovedAndReturnTypedRow(node, typedClass, row);
        } else if (currentCalledMethod == NODE_ADDED) {
            return validateNodeAddedAndReturnTypedRow(node, typedClass, row);
        } else if (currentCalledMethod == ROW_UPDATED) {
            return validateRowUpdateAndReturnTypedRow(node, typedClass, row);
        } else if (currentCalledMethod == IS_UPDATE_OF_INTEREST) {
            return validateIsUpdateOfInterestAndReturnTypedRow(node, typedClass, row);
        }
        return null;
    }

    private <T extends TypedBaseTable<?>> T validateRowRemovedAndReturnTypedRow(Node node, Class<T> typedClass, Row row) {
        for (String [] rowRemove : ROW_REMOVE_ARRAY) {
            if (rowRemove[ROW_REMOVE_NODE_ID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                Interface intfStubRow = new InterfaceStub();
                intfStubRow.setName(rowRemove[ROW_REMOVE_TABLE_NAME]);
                OvsdbMap<String, String> mapOvsdb = new OvsdbMap<String, String>();
                mapOvsdb.delegate().put("iface-id", rowRemove[ROW_REMOVE_NODE_UUID]);

                if (rowRemove[ROW_REMOVE_SET_PROPERTIES].equalsIgnoreCase("1")) {
                    mapOvsdb = new OvsdbMap<String, String>();
                    mapOvsdb.delegate().put("not an iface-id", rowRemove[ROW_REMOVE_NODE_UUID]);
                }

                if (rowRemove[ROW_REMOVE_SET_PROPERTIES].equalsIgnoreCase("3")) {
                    mapOvsdb = new OvsdbMap<String, String>();
                }

                intfStubRow.setExternalIds(mapOvsdb.delegate());

                if (!rowRemove[ROW_REMOVE_SET_PROPERTIES].equalsIgnoreCase("2")) {
                    return (T)intfStubRow;
                }
            }
        }
        return null;
    }

    private <T extends TypedBaseTable<?>> T validateNodeAddedAndReturnTypedRow(Node node, Class<T> typedClass, Row row) {
        Bridge bridge = new BridgeStub();
        bridge.setName("invalid Bridge name");

        boolean isValidPeNodeIdOf5 = false;
        if (NODE_ID_2.equalsIgnoreCase(node.getID().toString())) {
            isValidPeNodeIdOf5 = true;
        }
        try {
            if ((isValidPeNodeIdOf5)
                    || (NODE_ID_1.equalsIgnoreCase(Long.toHexString(Long.parseLong(node.getID().toString()))))) {
                bridge.setName("br-int");
            }
        } catch (Exception e) { }

        return (T)bridge;
    }

    private <T extends TypedBaseTable<?>> T validateRowUpdateAndReturnTypedRow(Node node, Class<T> typedClass, Row row) {
        if (typedClass.getSimpleName().equalsIgnoreCase("Interface")) {
            for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
                if (rowUpdate[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    Interface intfStubRow = new InterfaceStub();

                    intfStubRow.setName(getTableName(node, typedClass));
                    OvsdbMap<String, String> mapOvsdb = new OvsdbMap<String, String>();

                    mapOvsdb.delegate().put(rowUpdate[ROW_UPDATE_INTERFACE_NAME], rowUpdate[ROW_UPDATE_INTERFACE_PORT_ID]);
                    intfStubRow.setExternalIds(mapOvsdb.delegate());
                    OvsdbSet<Long> ofPortColumn = new OvsdbSet<>();

                    if (rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(OF_PORT_ARRAY_IS_NULL)) {
                        // Added Dumy port and in InterfaceStub, for this Dumyport OFPort will be returned as NULL
                        ofPortColumn.delegate().add(Long.valueOf(0));
                    } else if (!rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(OF_PORT_ARRAY_IS_EMPTY)) {
                        ofPortColumn.delegate().add(Long.valueOf(1));
                    }
                    intfStubRow.setOpenFlowPort(ofPortColumn.delegate());

                    return (T)intfStubRow;
                }
            }
        } else if (typedClass.getSimpleName().equalsIgnoreCase("Bridge")) {
            Bridge bridge = new BridgeStub();

            OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
            UUID uu = new UUID("12345678");
            ovsdbset.delegate().add(uu);
            bridge.setPorts(ovsdbset.delegate());

            OvsdbSet<String> ovsdbdatapath = new OvsdbSet<String>();
            ovsdbdatapath.delegate().add("1");
            bridge.setDatapathId(ovsdbdatapath.delegate());

            for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
                if (rowUpdate[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    if (rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_NULL)) {
                        // NULL to Bridge's DatapathId
                        bridge.setDatapathId(null);
                    } else if (rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(SET_BRIDGES_DATA_PATH_ID_COLUMN_TO_EMPTY_SET)) {
                        // Empty to Bridge's DatapathId
                        bridge.setDatapathId(new OvsdbSet<String>());
                    }
                }
            }
            bridge.setName("br-int");

            return (T)bridge;
        } else if (typedClass.getSimpleName().equalsIgnoreCase("Port")) {
            OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
            UUID uu = null;

            for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
                if (rowUpdate[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    if (rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(SET_PORT_OBJECT_TO_NULL)) {
                        return null;
                    } else if (rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(SET_PORT_OBJECT_TO_EMPTY_SET)) {
                        // Set NULL to ovsdbset
                        ovsdbset = null;
                    } else if (rowUpdate[ROW_UPDATE_OPTION].equalsIgnoreCase(SET_PORT_OBJECT_WITH_WRONG_UUID)) {
                        // Dumy-Wrong one
                        uu = new UUID("E6E005D3A24542FCB03897730A5150E2");
                        ovsdbset.delegate().add(uu);
                    } else {
                        uu = new UUID(rowUpdate[ROW_UPDATE_PARENT_UUID]);
                        ovsdbset.delegate().add(uu);
                    }
                }
            }

            PortStub port = new PortStub();
            port.setInterfaces(ovsdbset.delegate());
            return (T)port;
        }
        return null;
    }

    private <T extends TypedBaseTable<?>> T validateIsUpdateOfInterestAndReturnTypedRow(Node node, Class<T> typedClass, Row row) {
        if (typedClass.getSimpleName().equalsIgnoreCase("Interface")) {
            for (String [] isUpdateOfInterest : IS_UPDATE_OF_INTEREST_INPUT_ARRAY) {
                if (isUpdateOfInterest[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    Interface intfStubRow = new InterfaceStub();

                    if (!isUpdateOfInterest[ROW_UPDATE_PARENT_UUID].equalsIgnoreCase(INTERFACE_PARENT_UUID)) {
                        intfStubRow.setName(getTableName(node, typedClass));
                        OvsdbMap<String, String> mapOvsdb = new OvsdbMap<String, String>();

                        mapOvsdb.delegate().put(isUpdateOfInterest[ROW_UPDATE_INTERFACE_NAME], isUpdateOfInterest[ROW_UPDATE_INTERFACE_PORT_ID]);
                        intfStubRow.setExternalIds(mapOvsdb.delegate());

                        OvsdbSet<Long> ofPortColumn = new OvsdbSet<>();
                        ofPortColumn.delegate().add(Long.valueOf(1));
                        intfStubRow.setOpenFlowPort(ofPortColumn.delegate());
                    }
                    return (T)intfStubRow;
                }
            }
        } else if (typedClass.getSimpleName().equalsIgnoreCase("Bridge")) {
            Bridge bridge = new BridgeStub();

            OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
            UUID uu = new UUID("12345678");
            ovsdbset.delegate().add(uu);
            bridge.setPorts(ovsdbset.delegate());

            OvsdbSet<String> ovsdbdatapath = new OvsdbSet<String>();
            ovsdbdatapath.delegate().add("1");
            bridge.setDatapathId(ovsdbdatapath.delegate());

            bridge.setName("br-int");

            return (T)bridge;
        } else if (typedClass.getSimpleName().equalsIgnoreCase("Port")) {
            OvsdbSet<UUID> ovsdbset = new OvsdbSet<UUID>();
            UUID uu = null;
            for (String [] isUpdateOfInterest : IS_UPDATE_OF_INTEREST_INPUT_ARRAY) {
                if (isUpdateOfInterest[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    uu = new UUID(isUpdateOfInterest[ROW_UPDATE_PARENT_UUID]);
                }
            }
            ovsdbset.delegate().add(uu);
            PortStub port = new PortStub();
            for (String [] isUpdateOfInterest : IS_UPDATE_OF_INTEREST_INPUT_ARRAY) {
                if (isUpdateOfInterest[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    if (!isUpdateOfInterest[ROW_UPDATE_PARENT_UUID].equalsIgnoreCase(PORT_PARENT_UUID)) {
                        port.setInterfaces(ovsdbset.delegate());
                    }
                }
            }
            return (T)port;
        } else if (typedClass.getSimpleName().equalsIgnoreCase("OpenVSwitch")) {
            OpenVSwitchStub openVSwitch = new OpenVSwitchStub();
            return (T)openVSwitch;
        }
        return null;
    }

    // Following methods are Unused in UnitTest.
    @Override
    public UUID insertRow(Node node, String databaseName, String tableName, UUID parentRowUuid,
                            Row<GenericTableSchema> row) throws OvsdbPluginException {
        return null;
    }

    @Override
    public UUID insertRow(Node node, String databaseName, String tableName, String parentTable,
                            UUID parentRowUuid, String parentColumn, Row<GenericTableSchema> row) throws OvsdbPluginException {
        return null;
    }

    @Override
    public Row<GenericTableSchema> insertTree(Node node, String databaseName, String tableName, UUID parentRowUuid,
                                                Row<GenericTableSchema> row) throws OvsdbPluginException {
        return null;
    }

    @Override
    public Row<GenericTableSchema> insertTree(Node node, String databaseName, String tableName, String parentTable,
                                                UUID parentRowUuid, String parentColumn, Row<GenericTableSchema> row) throws OvsdbPluginException {
        return null;
    }

    @Override
    public Row<GenericTableSchema> updateRow(Node node, String databaseName, String tableName, UUID rowUuid,
                                                Row<GenericTableSchema> row, boolean overwrite) throws OvsdbPluginException {
        return null;
    }

    @Override
    public void deleteRow(Node node, String databaseName, String tableName, UUID rowUuid) throws OvsdbPluginException {
    }

    @Override
    public void deleteRow(Node node, String databaseName, String tableName, String parentTable, UUID parentRowUuid,
                            String parentColumn, UUID rowUuid) throws OvsdbPluginException {
    }

    @Override
    public Row<GenericTableSchema> getRow(Node node, String databaseName, String tableName, UUID uuid) throws OvsdbPluginException {
        return null;
    }

    @Override
    public ConcurrentMap<UUID, Row<GenericTableSchema>> getRows(Node node, String databaseName, String tableName) throws OvsdbPluginException {
        return null;
    }

    @Override
    public ConcurrentMap<UUID, Row<GenericTableSchema>> getRows(Node node, String databaseName, String tableName,
                                                                    String fiqlQuery) throws OvsdbPluginException {
        return null;
    }

    @Override
    public List<String> getTables(Node node, String databaseName) throws OvsdbPluginException {
        return null;
    }

    @Override
    public List<String> getTables(Node node) {
        return null;
    }

    @Override
    public Boolean setOFController(Node node, String bridgeUUID) {
        return null;
    }
}
