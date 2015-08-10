/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.List;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTable;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which determines
 * VTN data flows to be removed by a sequential scan.
 *
 * <p>
 *   This flow remover affects flow entries in all the VTNs.
 * </p>
 */
public abstract class GlobalScanFlowRemover extends ScanFlowRemover {
    // ScanFlowRemover

    /**
     * Return a list of all existing VTN flow tables.
     *
     * @param tx  A {@link ReadWriteTransaction} instance.
     * @return  A list of VTN flow tables.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected final List<VtnFlowTable> getFlowTables(ReadWriteTransaction tx)
        throws VTNException {
        return FlowUtils.getFlowTables(tx);
    }
}
