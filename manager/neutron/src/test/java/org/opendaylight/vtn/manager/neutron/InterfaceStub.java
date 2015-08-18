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
import org.opendaylight.ovsdb.schema.openvswitch.Interface;

/**
 * Stub class for unit tests.
 *
 * This stub provides APIs implemented in
 * org.opendaylight.neutron.spi package.
 */

public class InterfaceStub implements Interface {

    /*
     * String declaration for name of the interface
     */
    private String name;

    /*
     * Map declaration for externalIds of the interface
     */
    private Map<String, String> externalIds;

    /*
     * Set<Long> declaration for openFlowPort of the interface
     */
    private Set<Long> openFlowPort;

    /*
     * GenericTableSchema declaration for creating Tableschema of Bridge
     */
    private GenericTableSchema tableSchema = new GenericTableSchema("Interface");

    /*
     * Row declaration for creating Interfacee Row
     */
    private Row<GenericTableSchema> row = new Row<>(tableSchema);

    // Following methods are used in UnitTest.
    @Override
    public void setName(String name) {
        this.name = name;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public void setExternalIds(Map<String, String> externalIds) {
        this.externalIds = externalIds;
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getExternalIdsColumn() {
        Column<GenericTableSchema, Map<String, String>> column = null;
        if (externalIds != null) {
            if (externalIds.size() == 0) {
                column = new Column(null, null);
            } else {
                column = new Column(null, this.externalIds);
            }
        }
        return column;
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getOpenFlowPortColumn() {
        Set<Long> portSet = openFlowPort;
        if (portSet != null) {
            if (portSet.contains(Long.valueOf(0))) {
                portSet = null;
            }
            return new Column<GenericTableSchema, Set<Long>>(null, portSet);
        }

        return null;
    }

    @Override
    public void setOpenFlowPort(Set<Long> openFlowPort) {
        this.openFlowPort = openFlowPort;
    }

    @Override
    public GenericTableSchema getSchema() {
        return tableSchema;
    }

    @Override
    public Row<GenericTableSchema> getRow() {
        return row;
    }

    // Following methods are Unused in UnitTest.
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
    public Column<GenericTableSchema, String> getTypeColumn() {
        return null;
    }

    @Override
    public void setType(String type) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getOptionsColumn() {
        return null;
    }

    @Override
    public void setOptions(Map<String, String> options) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getIngressPolicingRateColumn() {
        return null;
    }

    @Override
    public void setIngressPolicingRate(Set<Long> ingressPolicingRate) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getIngressPolicingBurstColumn() {
        return null;
    }

    @Override
    public void setIngressPolicingBurst(Set<Long> ingressPolicingBurst) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getMacInUseColumn() {
        return null;
    }

    @Override
    public void setMacInUse(Set<String> macInUse) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getMacColumn() {
        return null;
    }

    @Override
    public void setMac(Set<String> mac) {
    }

    @Override
    public Column<GenericTableSchema, Long> getIfIndexColumn() {
        return null;
    }

    @Override
    public void setIfIndex(Long ifIndex) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getOpenFlowPortRequestColumn() {
        return null;
    }

    @Override
    public void setOpenFlowPortRequest(Set<Long> openFlowPortRequest) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getBfdColumn() {
        return null;
    }

    @Override
    public void setBfd(Map<String, String> bfd) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getBfdStatusColumn() {
        return null;
    }

    @Override
    public void setBfdStatus(Map<String, String> bfdStatus) {
    }

    @Override
    public Column<GenericTableSchema, String> getMonitorColumn() {
        return null;
    }

    @Override
    public void setMonitor(String monitor) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getCfmMpidColumn() {
        return null;
    }

    @Override
    public void setCfmMpid(Set<Long> cfmMpid) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getCfmRemoteMpidColumn() {
        return null;
    }

    @Override
    public void setCfmRemoteMpid(Set<Long> cfmRemoteMpid) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getCfmRemoteMpidsColumn() {
        return null;
    }

    @Override
    public void setCfmRemoteMpids(Set<Long> cfmRemoteMpids) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getCfmFlapCountColumn() {
        return null;
    }

    @Override
    public void setCfmFlapCount(Set<Long> cfmFlapCount) {
    }

    @Override
    public Column<GenericTableSchema, Set<Boolean>> getCfmFaultColumn() {
        return null;
    }

    @Override
    public void setCfmFault(Set<Boolean> cfmFault) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getCfmFaultStatusColumn() {
        return null;
    }

    @Override
    public void setCfmFaultStatus(Set<String> cfmFaultStatus) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getCfmRemoteOpStateColumn() {
        return null;
    }

    @Override
    public void setCfmRemoteOpState(Set<String> cfmRemoteOpState) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getCfmHealthColumn() {
        return null;
    }

    @Override
    public void setCfmHealth(Set<Long> cfmHealth) {
    }

    @Override
    public Column<GenericTableSchema, Set<Boolean>> getLacpCurrentColumn() {
        return null;
    }

    @Override
    public void setLacpCurrent(Set<Boolean> lacpCurrent) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getOtherConfigColumn() {
        return null;
    }

    @Override
    public void setOtherConfig(Map<String, String> otherConfig) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, Long>> getStatisticsColumn() {
        return null;
    }

    @Override
    public void setStatistics(Map<String, Long> statistics) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getStatusColumn() {
        return null;
    }

    @Override
    public void setStatus(Map<String, String> status) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getAdminStateColumn() {
        return null;
    }

    @Override
    public void setAdminState(Set<String> adminState) {
    }

    @Override
    public Column<GenericTableSchema, Map<String, String>> getLinkStateColumn() {
        return null;
    }

    @Override
    public void setLinkState(Map<String, String> linkState) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getLinkResetsColumn() {
        return null;
    }

    @Override
    public void setLinkResets(Set<String> linkResets) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getLinkSpeedColumn() {
        return null;
    }

    @Override
    public void setLinkSpeed(Set<Long> linkSpeed) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getDuplexColumn() {
        return null;
    }

    @Override
    public void setDuplex(Set<Long> duplex) {
    }

    @Override
    public Column<GenericTableSchema, Set<Long>> getMtuColumn() {
        return null;
    }

    @Override
    public void setMtu(Set<Long> mtu) {
    }

    @Override
    public Column<GenericTableSchema, Set<String>> getErrorColumn() {
        return null;
    }

    @Override
    public void setError(Set<String> error) {
    }

}
