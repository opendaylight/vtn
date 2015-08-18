/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.Map;
import java.util.HashMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;

/**
 * Helper class to read flow conditions using the given MD-SAL datastore
 * transaction.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public final class FlowCondReader {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(FlowCondReader.class);

    /**
     * Cached flow conditions.
     */
    private final Map<String, VTNFlowCondition> conditionCache =
        new HashMap<>();

    /**
     * Read transaction.
     */
    private final ReadTransaction  readTx;

    /**
     * Construct a new instance.
     *
     * @param rtx  A {@link ReadTransaction} instance.
     */
    public FlowCondReader(ReadTransaction rtx) {
        readTx = rtx;
    }

    /**
     * Return read transaction.
     *
     * @return  A {@link ReadTransaction} instance.
     */
    public ReadTransaction getReadTransaction() {
        return readTx;
    }

    /**
     * Get the flow condition specified by the name.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link VTNFlowCondition} instance if found.
     *          {@code null} if not found.
     */
    public VTNFlowCondition get(String name) {
        VTNFlowCondition vfcond = conditionCache.get(name);
        if (vfcond == null && !conditionCache.containsKey(name)) {
            try {
                vfcond = read(name);
            } catch (VTNException e) {
                LOG.warn(name + ": Ignore unreadable flow condition.", e);
                vfcond = null;
            }
            conditionCache.put(name, vfcond);
        }

        return vfcond;
    }

    /**
     * Read the flow condition specified by the name.
     *
     * @param name  The name of the flow condition.
     * @return  A {@link VTNFlowCondition} instance if found.
     *          {@code null} if not found.
     * @throws VTNException
     *    Failed to read flow condition.
     */
    private VTNFlowCondition read(String name) throws VTNException {
        InstanceIdentifier<VtnFlowCondition> path =
            FlowCondUtils.getIdentifier(name);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        Optional<VtnFlowCondition> opt =
            DataStoreUtils.read(readTx, oper, path);
        return (opt.isPresent()) ? new VTNFlowCondition(opt.get()) : null;
    }
}
