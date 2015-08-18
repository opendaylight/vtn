/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.Collections;
import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which determines
 * VTN data flows to be removed by a sequential scan.
 *
 * <p>
 *   This flow remover affects flow entries only in the specified VTN.
 * </p>
 */
public abstract class TenantScanFlowRemover extends ScanFlowRemover {
    /**
     * The name of the target VTN.
     */
    private final String  tenantName;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     */
    protected TenantScanFlowRemover(String tname) {
        tenantName = tname;
    }

    /**
     * Return the name of the target VTN.
     *
     * @return  The name of the target VTN.
     */
    public final String getTenantName() {
        return tenantName;
    }

    // ScanFlowRemover

    /**
     * Return the target VTN flow table as a list.
     *
     * @param tx  A {@link ReadWriteTransaction} instance.
     * @return  A list of VTN flow tables.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final List<VtnFlowTable> getFlowTables(ReadWriteTransaction tx)
        throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnFlowTable> path =
            FlowUtils.getIdentifier(tenantName);
        Optional<VtnFlowTable> opt = DataStoreUtils.read(tx, oper, path);
        return (opt.isPresent())
            ? Collections.singletonList(opt.get())
            : Collections.<VtnFlowTable>emptyList();
    }
}
