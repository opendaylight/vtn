/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.Set;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlowKey;

/**
 * {@code VTNDataFlow} describes a VTN data flow information in the MD-SAL
 * datastore.
 */
public interface VTNDataFlow {
    /**
     * Return the key assocaited with the VTN data flow in the MD-SAL
     * datastore.
     *
     * @return  A {@link VtnDataFlowKey} instance.
     */
    VtnDataFlowKey getKey();

    /**
     * Return a string which represents the condition configured in the ingress
     * flow entry.
     *
     * @return  A string which representse the ingress match condition.
     * @throws VTNException  An error occurred.
     */
    String getIngressMatchKey() throws VTNException;

    /**
     * Return a set of switches related to this data flow.
     *
     * <p>
     *   Note that this method returns a read-only set.
     * </p>
     *
     * @return  A set of {@link SalNode} instances.
     */
    Set<SalNode> getFlowNodes();

    /**
     * Return a set of switch ports related to this data flow.
     *
     * <p>
     *   Note that this method returns a read-only set.
     * </p>
     *
     * @return  A set of {@link SalPort} instances.
     */
    Set<SalPort> getFlowPorts();

    /**
     * Return the source L2 host of this data flow.
     *
     * @return  A {@link SourceHostFlowsKey} instance which specifies the
     *          source L2 host. {@code null} if the source L2 host could not
     *          be determined.
     */
    SourceHostFlowsKey getSourceHostFlowsKey();
}
