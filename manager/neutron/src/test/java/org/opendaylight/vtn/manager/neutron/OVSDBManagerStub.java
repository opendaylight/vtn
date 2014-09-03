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
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.ovsdb.lib.notation.OvsdbSet;
import org.opendaylight.ovsdb.lib.notation.Row;
import org.opendaylight.ovsdb.lib.notation.UUID;
import org.opendaylight.ovsdb.lib.notation.OvsdbMap;
import org.opendaylight.ovsdb.schema.openvswitch.Bridge;
import org.opendaylight.ovsdb.schema.openvswitch.Interface;
import org.opendaylight.ovsdb.lib.schema.GenericTableSchema;
import org.opendaylight.ovsdb.lib.schema.typed.TypedBaseTable;
import org.opendaylight.ovsdb.plugin.OvsdbConfigService;
import org.opendaylight.ovsdb.plugin.api.StatusWithUuid;

import java.util.concurrent.ConcurrentMap;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.ovsdb.plugin package.
 */
public class OVSDBManagerStub extends TestBase implements OvsdbConfigService {
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
            for (String [] rowRemove : ROW_REMOVE_ARRAY) {
                if (rowRemove[ROW_REMOVE_NODE_ID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                    Interface intfStubRow = new InterfaceStub();
                    intfStubRow.setName(rowRemove[ROW_REMOVE_TABLE_NAME]);
                    OvsdbMap<String, String> mapOvsdb = new OvsdbMap<String, String>();

                    if (!rowRemove[ROW_REMOVE_SET_PROPERTIES].equalsIgnoreCase("2")) {
                        mapOvsdb.delegate().put("iface-id", rowRemove[ROW_REMOVE_NODE_UUID]);
                    }

                    if (!rowRemove[ROW_REMOVE_SET_PROPERTIES].equalsIgnoreCase("1")) {
                        intfStubRow.setExternalIds(mapOvsdb.delegate());
                    }

                    if (!rowRemove[ROW_REMOVE_SET_PROPERTIES].equalsIgnoreCase("3")) {
                        return (T)intfStubRow;
                    }
                }
            }
        } else if (currentCalledMethod == NODE_ADDED) {
            Bridge bridge = new BridgeStub();

            boolean isValidPeNodeIdOf5 = false;
            if ((String.valueOf(node.getID()).length() == UUID_LEN)
                    && (BRIDGE_UUID_2.equalsIgnoreCase(String.valueOf(Long.parseLong(String.valueOf(node.getID()).substring(UUID_POSITION_IN_NODE), HEX_RADIX))))) {
                isValidPeNodeIdOf5 = true;
            }
            if ((!BRIDGE_UUID_1.equalsIgnoreCase(String.valueOf(node.getID())))
                    && (!isValidPeNodeIdOf5)) {
                bridge.setName("br-int");
            }

            return (T)bridge;
        } else if (currentCalledMethod == ROW_UPDATED) {
            if (typedClass.getSimpleName().equalsIgnoreCase("Interface")) {
                for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
                    if (rowUpdate[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                        Interface intfStubRow = new InterfaceStub();

                        if (!rowUpdate[ROW_UPDATE_PARENT_UUID].equalsIgnoreCase(INTERFACE_PARENT_UUID)) {
                            intfStubRow.setName(getTableName(node, typedClass));
                            OvsdbMap<String, String> mapOvsdb = new OvsdbMap<String, String>();

                            mapOvsdb.delegate().put(rowUpdate[ROW_UPDATE_INTERFACE_NAME], rowUpdate[ROW_UPDATE_INTERFACE_PORT_ID]);
                            intfStubRow.setExternalIds(mapOvsdb.delegate());

                            OvsdbSet<Long> ofPortColumn = new OvsdbSet<>();
                            ofPortColumn.delegate().add(new Long("1"));
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
                for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
                    if (rowUpdate[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                        uu = new UUID(rowUpdate[ROW_UPDATE_PARENT_UUID]);
                    }
                }
                ovsdbset.delegate().add(uu);
                PortStub port = new PortStub();
                for (String [] rowUpdate : ROW_UPDATE_INPUT_ARRAY) {
                    if (rowUpdate[ROW_UPDATE_UUID].equalsIgnoreCase(String.valueOf(node.getID()))) {
                        if (!rowUpdate[ROW_UPDATE_PARENT_UUID].equalsIgnoreCase(PORT_PARENT_UUID)) {
                            port.setInterfaces(ovsdbset.delegate());
                        }
                    }
                }
                return (T)port;
            }
        }
        return null;
    }

    // Following methods are Unused in UnitTest.
    @Override
    public List<String> getTables(Node node) {
        return null;
    }

    @Override
    public Boolean setOFController(Node node, String bridgeUUID) {
        return null;
    }
}
