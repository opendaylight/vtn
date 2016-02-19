/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.table.Flow;

/**
 * {@code RemovedFlows} describes a set of flow entries removed by a bulk
 * remove-flow operation.
 */
public final class RemovedFlows {
    /**
     * The flow table ID.
     */
    private final Short  tableId;

    /**
     * A list of MD-SAL flow entries to be retained.
     */
    private final List<Flow>  retainedFlows = new ArrayList<>();

    /**
     * A list of flow IDs to be removed from the flow table.
     */
    private final List<String>  removedIds = new ArrayList<>();

    /**
     * Construct a new instance.
     *
     * @param id  The flow table ID.
     */
    public RemovedFlows(Short id) {
        tableId = id;
    }

    /**
     * Add a MD-SAL flow entry to be retained in the flow table.
     *
     * @param flow  A MD-SAL flow entry to be retained in the flow table.
     */
    public void addRetained(Flow flow) {
        retainedFlows.add(flow);
    }

    /**
     * Add a MD-SAL flow entry to be removed from the flow table.
     *
     * @param flow  A MD-SAL flow entry to be removed from the flow table.
     */
    public void addRemoved(Flow flow) {
        removedIds.add(flow.getId().getValue());
    }

    /**
     * Return a list of flow IDs to be removed from the flow table.
     *
     * @return  A list of flow IDs to be removed from the flow table.
     */
    public List<String> getRemoved() {
        return removedIds;
    }

    /**
     * Return a new flow table to be put into the MD-SAL datastore.
     *
     * @return  A MD-SAL flow table.
     */
    public Table getTable() {
        return new TableBuilder().
            setId(tableId).
            setFlow(retainedFlows).
            build();
    }

    /**
     * Determine whether the flow table was changed or not.
     *
     * @return  {@code true} if the flow table was changed.
     *          {@code false} otherwise.
     */
    public boolean isChanged() {
        return !removedIds.isEmpty();
    }
}
