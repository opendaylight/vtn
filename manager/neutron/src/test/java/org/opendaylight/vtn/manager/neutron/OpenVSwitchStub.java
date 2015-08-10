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
import org.opendaylight.ovsdb.schema.openvswitch.OpenVSwitch;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.ovsdb.schema.openvswitch package.
 */
public class OpenVSwitchStub implements OpenVSwitch {
    /*
     * GenericTableSchema declaration for creating Tableschema of OpenVSwitch
     */
    private GenericTableSchema tableSchema = new GenericTableSchema("OpenVSwitch");

    /*
     * Row declaration for creating OpenVSwitch Row
     */
    private Row<GenericTableSchema> row = new Row<>(tableSchema);

    // Following methods are Used in Unit Test.
    @Override
    public GenericTableSchema getSchema() {
        return this.tableSchema;
    }

    @Override
    public Row<GenericTableSchema> getRow() {
        return this.row;
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
    public Column<GenericTableSchema, Set<UUID>> getBridgesColumn() {
        return null;
    }

    @Override
    public void setBridges(Set<UUID> bridges) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getManagersColumn() {
        return null;
    }

    @Override
    public void setManagers(Set<UUID> managers) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getManagerOptionsColumn() {
        return null;
    }

    @Override
    public void setManagerOptions(Set<UUID> managerOptions) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getSslColumn() {
        return null;
    }

    @Override
    public void setSsl(Set<UUID> ssl) {
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
    public Column<GenericTableSchema, Long> getNextConfigColumn() {
        return null;
    }

    @Override
    public void setNextConfig(Long nextConfig) {
    }

    @Override
    public Column<GenericTableSchema, Long> getCurrentConfigColumn() {
        return null;
    }

    @Override
    public void setCurrentConfig(Long currentConfig) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, UUID>> getCapabilitiesColumn() {
        return null;
    }

    @Override
    public void setCapabilities(Map<String, UUID> capabilities) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, Long>> getStatisticsColumn() {
        return null;
    }

    @Override
    public void setStatistics(Map<String, Long> statistics) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getOvsVersionColumn() {
        return null;
    }

    @Override
    public void setOvsVersion(Set<String> ovsVersion) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getDbVersionColumn() {
        return null;
    }

    @Override
    public void setDbVersion(Set<String> dbVersion) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getSystemTypeColumn() {
        return null;
    }

    @Override
    public void setSystemType(Set<String> systemType) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getSystemVersionColumn() {
        return null;
    }

    @Override
    public void setSystemVersion(Set<String> systemVersion) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getDatapathTypesColumn() {
        return null;
    }

    @Override
    public void setDatapathTypes(Set<String> datapathTypes) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getIfaceTypesColumn() {
        return null;
    }

    @Override
    public void setIfaceTypes(Set<String> ifaceTypes) {
    }
}
