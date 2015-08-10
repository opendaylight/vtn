/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron;

import java.util.Map;
import java.util.Set;

import org.opendaylight.ovsdb.lib.notation.Column;
import org.opendaylight.ovsdb.lib.notation.Row;
import org.opendaylight.ovsdb.lib.notation.UUID;
import org.opendaylight.ovsdb.lib.schema.GenericTableSchema;
import org.opendaylight.ovsdb.schema.openvswitch.Bridge;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.neutron.spi package.
 */

public class BridgeStub implements Bridge {

    /*
     * String declaration for BridgeName
     */
    private String name = "";

    /*
      * GenericTableSchema declaration for creating Tableschema of Bridge
      */
    private GenericTableSchema tableSchema = new GenericTableSchema("Bridge");

    /*
     * Row declaration for creating Bridge Row
     */
    private Row<GenericTableSchema> row = new Row<>(tableSchema);

    /*
     * Set<UUID> declaration for creating Set<UUID> for Ports
     */
    private Set<UUID> ports;

    /*
     * Set<String> declaration for creating Set<String> of datapathId for Bridge
     */
    private Set<String> datapathId;

    // Following methods are Used in Unit Test.
    @Override
    public GenericTableSchema getSchema() {
        return tableSchema;
    }

    @Override
    public Row<GenericTableSchema> getRow() {
        return row;
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getPortsColumn() {
        return new Column<GenericTableSchema, Set<UUID>>(null, ports);
    }

    @Override
    public void setPorts(Set<UUID> ports) {
        this.ports = ports;
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getDatapathIdColumn() {
        return new Column<GenericTableSchema, Set<String>>(null, datapathId);
    }

    @Override
    public void setDatapathId(Set<String> datapathId) {
        this.datapathId = datapathId;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public void setName(String name) {
        this.name = name;
    }

    // Following methods are Unused in Unit Test.
    @Override
    public UUID getUuid() {
        return null;
    }

    @Override
    public Column<GenericTableSchema, UUID> getUuidColumn() {
        return null;
    }

    @Override
    public UUID getVersion() {
        return null;
    }

    @Override
    public Column<GenericTableSchema, UUID> getVersionColumn() {
        return null;
    }

    @Override
    public Column<GenericTableSchema, String> getNameColumn() {
        return null;
    }

    @Override
    public Column<GenericTableSchema, String> getDatapathTypeColumn() {
        return null;
    }

    @Override
    public void setDatapathType(String datapathType) {
    }

    @Override
    public Column<GenericTableSchema, Boolean> getStpEnableColumn() {
        return null;
    }

    @Override
    public void setStpEnable(Boolean stpEnable) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getMirrorsColumn() {
        return null;
    }

    @Override
    public void setMirrors(Set<UUID> mirrors) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getNetflowColumn() {
        return null;
    }

    @Override
    public void setNetflow(Set<UUID> netflow) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getSflowColumn() {
        return null;
    }

    @Override
    public void setSflow(Set<UUID> sflow) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getIpfixColumn() {
        return null;
    }

    @Override
    public void setIpfix(Set<UUID> ipfix) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getControllerColumn() {
        return null;
    }

    @Override
    public void setController(Set<UUID> controller) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getProtocolsColumn() {
        return null;
    }

    @Override
    public void setProtocols(Set<String> protocols) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getFailModeColumn() {
        return null;
    }

    @Override
    public void setFailMode(Set<String> failMode) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getStatusColumn() {
        return null;
    }

    @Override
    public void setStatus(Map<String, String> status) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getOtherConfigColumn() {
        return null;
    }

    @Override
    public void setOtherConfig(Map<String, String> otherConfig) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getExternalIdsColumn() {
        return null;
    }

    @Override
    public void setExternalIds(Map<String, String> externalIds) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getFloodVlansColumn() {
        return null;
    }

    @Override
    public void setFloodVlans(Set<Long> vlans) {
    }

    @Override
    public Column<GenericTableSchema, Map<Long, UUID>> getFlowTablesColumn() {
        return null;
    }

    @Override
    public void setFlowTables(Map<Long, UUID> flowTables) {
    }

}
