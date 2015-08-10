/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VtnFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * {@code FlowFinder} is used to find the VTN data flow associated with the
 * specified VTN flow ID.
 */
public final class FlowFinder {
    /**
     * The MD-SAL datastore transaction.
     */
    private final ReadTransaction  readTx;

    /**
     * A list of VTN names.
     */
    private List<String>  tenantNames;

    /**
     * Construct a new instance.
     *
     * @param rtx  A {@link ReadTransaction} instance.
     * @throws VTNException  An error occurred.
     */
    public FlowFinder(ReadTransaction rtx) throws VTNException {
        readTx = rtx;

        // Collect VTN names in the vtn-flows container.
        InstanceIdentifier<VtnFlows> path =
            InstanceIdentifier.create(VtnFlows.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnFlows> opt = DataStoreUtils.read(rtx, oper, path);
        List<VtnFlowTable> tables = (opt.isPresent())
            ? opt.get().getVtnFlowTable() : null;
        if (tables == null || tables.isEmpty()) {
            tenantNames = Collections.<String>emptyList();
        } else {
            tenantNames = new ArrayList<>(tables.size());
            for (VtnFlowTable table: tables) {
                tenantNames.add(table.getTenantName());
            }
        }
    }

    /**
     * Search for the VTN data flow associated with the given ID.
     *
     * @param flowId  Identifier for the VTN data flow.
     * @return  An {@link IdentifiedData} instance which contains VTN data flow
     *          if found. {@code null} if not found.
     * @throws VTNException  An error occurred.
     */
    public IdentifiedData<VtnDataFlow> find(VtnFlowId flowId)
        throws VTNException {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        for (String tname: tenantNames) {
            InstanceIdentifier<VtnDataFlow> path =
                FlowUtils.getIdentifier(tname, flowId);
            Optional<VtnDataFlow> opt =
                DataStoreUtils.read(readTx, oper, path);
            if (opt.isPresent()) {
                return new IdentifiedData<VtnDataFlow>(path, opt.get());
            }
        }

        return null;
    }
}
