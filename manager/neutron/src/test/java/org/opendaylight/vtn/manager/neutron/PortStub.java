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
import org.opendaylight.ovsdb.schema.openvswitch.Port;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.neutron.spi package.
 */

public class PortStub implements Port{

    /*
     * GenericTableSchema declaration for creating Tableschema of Port
     */
    private GenericTableSchema tableSchema = new GenericTableSchema("Port");

    /*
     * Row declaration for creating Port Row
     */
    private Row<GenericTableSchema> row = new Row<>(tableSchema);

    /*
    * Set<UUID> declaration for creating Set<UUID> for PortInterfaces
    */
    private Set<UUID> interfaces;

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
    public void setInterfaces(Set<UUID> interfaces) {
        this.interfaces = interfaces;
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getInterfacesColumn() {
        if (interfaces != null) {
            return new Column<GenericTableSchema, Set<UUID>>(null, interfaces);
        }
        return null;
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
    public void setName(String name) {
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getTrunksColumn() {
        return null;
    }

    @Override
    public void setTrunks(Set<Long> trunks) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getTagColumn() {
        return null;
    }

    @Override
    public void setTag(Set<Long> tag) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getVlanModeColumn() {
        return null;
    }

    @Override
    public void setVlanMode(Set<String> vlanMode) {
    }

    @Override
    public Column<GenericTableSchema, Set<UUID>> getQosColumn() {
        return null;
    }

    @Override
    public void setQos(Set<UUID> qos) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getMacColumn() {
        return null;
    }

    @Override
    public void setMac(Set<String> mac) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getBondTypeColumn() {
        return null;
    }

    @Override
    public void setBondType(Set<String> bondType) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getBondModeColumn() {
        return null;
    }

    @Override
    public void setBondMode(Set<String> bondMode) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getLacpColumn() {
        return null;
    }

    @Override
    public void setLacp(Set<String> lacp) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getBondUpDelayColumn() {
        return null;
    }

    @Override
    public void setBondUpDelay(Set<Long> bondUpDelay) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getBondDownDelayColumn() {
        return null;
    }

    @Override
    public void setBondDownDelay(Set<Long> bondDownDelay) {
    }

    @Override
    public Column<GenericTableSchema, Set<Boolean>> getBondFakeInterfaceColumn() {
        return null;
    }

    @Override
    public void setBondFakeInterface(Set<Boolean> bondFakeInterface) {
    }

    @Override
    public Column<GenericTableSchema, Set<Boolean>> getFakeBridgeColumn() {
        return null;
    }

    @Override
    public void setFakeBridge(Set<Boolean> fakeBridge) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getStatusColumn() {
        return null;
    }

    @Override
    public void setStatus(Map<String, String> status) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, Long>> getStatisticsColumn() {
        return null;
    }

    @Override
    public void setStatistics(Map<String, Long> statistics) {
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
}
