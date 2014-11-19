/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.flow.DataFlowFilter;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.util.NodeUtils;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code DataFlowFilterImpl} implements actual work for data flow filtering.
 */
public final class DataFlowFilterImpl {
    /**
     * A flag which indicates the data flow query should be done by sequential
     * scan.
     */
    public static final int  INDEX_NONE = 0;

    /**
     * A flag which indicates the source L2 host index should be used.
     */
    public static final int  INDEX_L2SRC = 1;

    /**
     * A flag which indicates the physical switch index should be used.
     */
    public static final int  INDEX_SWITCH = 2;

    /**
     * A flag which indicates the physical switch port index should be used.
     */
    public static final int  INDEX_PORT = 3;

    /**
     * A {@link MacVlan} instance which represents the source L2 host
     * specified by {@link DataFlowFilter} instance.
     */
    private final MacVlan  sourceHost;

    /**
     * A {@link Node} instance specified by a {@link DataFlowFilter} instance.
     */
    private final Node  node;

    /**
     * A {@link NodeConnector} instance specified by a {@link DataFlowFilter}
     * instance.
     */
    private final NodeConnector  port;

    /**
     * Set {@code true} if no data flow should be selected.
     */
    private final boolean  notMatch;

    /**
     * An integer value which specifies the type of data flow index
     * to be used.
     */
    private final int  indexType;

    /**
     * Construct a new instance.
     *
     * @param mgr     VTN Manager service.
     * @param filter  A {@link DataFlowFilter} instance.
     */
    public DataFlowFilterImpl(VTNManagerImpl mgr, DataFlowFilter filter) {
        if (filter == null || filter.isEmpty()) {
            // Select all data flows.
            sourceHost = null;
            node = null;
            port = null;
            notMatch = false;
            indexType = INDEX_NONE;
            return;
        }

        MacVlan src = null;
        Node nd = null;
        NodeConnector nc = null;
        boolean nm = false;
        nd = filter.getNode();
        if (nd != null) {
            SwitchPort swport = filter.getSwitchPort();
            if (swport != null) {
                // Determine the specified switch port.
                nc = NodeUtils.findPort(mgr, nd, swport);
                if (nc == null) {
                    // The specified port does not exist.
                    // Thus no data flow should be selected.
                    nm = true;
                } else {
                    // NodeConnector is used as index.
                    // Thus Node does not need to be checked.
                    nd = null;
                }
            }
        }

        // Check source L2 host.
        DataLinkHost dlhost = filter.getSourceHost();
        if (dlhost != null) {
            // Try to convert DataLinkHost instance into MacVlan instance.
            try {
                src = new MacVlan(dlhost);
            } catch (Exception e) {
                // The source host is specified by unsupported address type,
                // or an invalid VLAN ID is specified.
                // Thus no data flow should be selected.
                nm = true;
            }
        }

        // Determine index type to be used.
        if (src != null) {
            indexType = INDEX_L2SRC;
        } else if (nc != null) {
            indexType = INDEX_PORT;
        } else if (nd != null) {
            indexType = INDEX_SWITCH;
        } else {
            indexType = INDEX_NONE;
        }

        sourceHost = src;
        node = nd;
        port = nc;
        notMatch = nm;
    }

    /**
     * Determine whether the all data flows should be filtered out or not.
     *
     * @return  {@code true} is returned if all data flows should be filtered
     *          out. Otherwise {@code false} is returned.
     */
    public boolean isNotMatch() {
        return notMatch;
    }

    /**
     * Return an integer value which specifies the type of data flow index
     * to be used.
     *
     * @return  An integer vlaue which specifies the type of data flow index.
     */
    public int getIndexType() {
        return indexType;
    }

    /**
     * Return a {@link MacVlan} instance which represents the condition for
     * source L2 host.
     *
     * @return  A {@link MacVlan} instance if configured in this instance.
     *          {@code null} if not configured.
     */
    public MacVlan getSourceHost() {
        return sourceHost;
    }

    /**
     * Return a {@link NodeConnector} instance which represents the condition
     * for physical switch port.
     *
     * @return  A {@link NodeConnector} instance if configured in this
     *          instance. {@code null} if not configured.
     */
    public NodeConnector getPort() {
        return port;
    }

    /**
     * Return a {@link Node} instance which represents the condition for
     * physical switch.
     *
     * @return  A {@link Node} instance if configured in this instance.
     *          {@code null} if not configured.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Determine whether the specified VTN flow should be selected or not.
     *
     * @param vflow  A VTN flow.
     * @return  {@code true} if the specified flow should be selected.
     *          {@code false} if it should not be selected.
     */
    public boolean select(VTNFlow vflow) {
        if (indexType == INDEX_NONE) {
            // Currently an index will be used at least one condition is
            // specified. So INDEX_NONE means that all flows should be
            // selected.
            return true;
        }

        if (sourceHost != null && indexType != INDEX_L2SRC) {
            // Check source L2 host of the VTN flow.
            L2Host host = vflow.getSourceHost();
            if (host == null || !sourceHost.equals(host.getHost())) {
                // Source L2 host does not match.
                return false;
            }
        }

        // Check whether the VTN flow depends on the specified switch port
        // or not.
        if (port != null && indexType != INDEX_PORT &&
            !vflow.getFlowPorts().contains(port)) {
            // Switch port does not match.
            return false;
        }

        // Check whether the VTN flow depends on the specified switch or not.
        if (node != null && indexType != INDEX_SWITCH &&
            !vflow.getFlowNodes().contains(node)) {
            // Switch does not match.
            return false;
        }

        return true;
    }
}
