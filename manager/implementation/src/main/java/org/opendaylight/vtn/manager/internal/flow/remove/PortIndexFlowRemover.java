/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.PortFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.flows.VtnFlowTableKey;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows in the specified VTN using port flow index.
 */
public abstract class PortIndexFlowRemover
    extends TenantIndexFlowRemover<PortFlows> {
    /**
     * The target port.
     */
    private final SalPort  flowPort;

    /**
     * The key which specifies the target port in the port flow index.
     */
    private final PortFlowsKey  portKey;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the target VTN.
     * @param sport  A {@link SalPort} instance.
     */
    public PortIndexFlowRemover(String tname, SalPort sport) {
        super(tname);
        flowPort = sport;
        portKey = new PortFlowsKey(sport.getNodeConnectorId());
    }

    /**
     * Return the target switch port.
     *
     * @return  A {@link SalPort} instance whcih specifies the target switch
     *          port.
     */
    public final SalPort getFlowPort() {
        return flowPort;
    }

    // TenantIndexFlowRemover

    /**
     * Return the path to the port flow index.
     *
     * @param key  A {@link VtnFlowTableKey} instance which specifies the
     *             target VTN flow table.
     * @return  Path to the port flow index.
     */
    @Override
    protected final InstanceIdentifier<PortFlows> getPath(VtnFlowTableKey key) {
        return FlowUtils.getIdentifier(key, portKey);
    }
}
