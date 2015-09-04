/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.util.EtherAddress;

/**
 * {@code VTNConfig} describes the global configuration of the VTN Manager.
 */
public interface VTNConfig {
    /**
     * Return the number of milliseconds to wait for node edges to be detected.
     *
     * @return  The number of milliseconds to wait for node edges.
     */
    int getNodeEdgeWait();

    /**
     * Return priority value for layer 2 flow entries.
     *
     * @return  Priority value for layer 2 flow entries.
     */
    int getL2FlowPriority();

    /**
     * Return the number of milliseconds to wait for completion of a single
     * FLOW_MOD request.
     *
     * @return  The number of milliseconds to wait.
     */
    int getFlowModTimeout();

    /**
     * Return the number of milliseconds to wait for completion of bulk
     * FLOW_MOD requests.
     *
     * @return  The number of milliseconds to wait.
     */
    int getBulkFlowModTimeout();

    /**
     * Return the number of milliseconds to wait for another controller in the
     * cluster to complete initialization.
     *
     * @return  The number of milliseconds to wait.
     */
    int getInitTimeout();

    /**
     * Return the maximum number of packet redirections per a flow.
     *
     * @return  The maximum number of packet redirections per a flow.
     */
    int getMaxRedirections();

    /**
     * Return MAC address of the controller used as source MAC address of
     * ARP packet.
     *
     * @return  An {@link EtherAddress} instance.
     */
    EtherAddress getControllerMacAddress();
}
