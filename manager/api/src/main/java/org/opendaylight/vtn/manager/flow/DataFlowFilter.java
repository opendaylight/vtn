/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code DataFlowFilter} describes a set of conditions to select data flows
 * configured in the VTN.
 *
 * <p>
 *   The following conditions can be specified.
 *   If more than one contitions are specified, only data flows that meet all
 *   the conditions will be selected.
 * </p>
 * <dl style="margin-left: 1em;">
 *   <dt>Source L2 host
 *   <dd>
 *     Source L2 host is specified by a instance of {@link DataLinkHost}.
 *     If this condition is specified, only data flows that map packets
 *     sent by the specified L2 host will be selected.
 *
 *   <dt>Physical switch or physical switch port.
 *   <dd>
 *     Either a physical switch or a physical switch port can be specified.
 *     <ul>
 *       <li>
 *         If a physical switch is specified, only data flows that forward
 *         packets via the specified switch will be selected.
 *       </li>
 *       <li>
 *         If a physical switch port is specified, only data flows that forward
 *         packets via the specified port will be selected.
 *       </li>
 *     </ul>
 * </dl>
 *
 * <p>
 *   Note that this class is not synchronized.
 *   Concurrent access to an instance of this class by multiple threads
 *   must be synchronized externally.
 * </p>
 *
 * @since  Helium
 */
public final class DataFlowFilter {
    /**
     * A {@link DataLinkHost} instance corresponding to the source L2 host.
     */
    private DataLinkHost  sourceHost;

    /**
     * A {@link Node} instance corresponding to the physical switch.
     */
    private Node  node;

    /**
     * The location of the physical switch port with the physical switch
     * specified by {@link #node}.
     */
    private SwitchPort  port;

    /**
     * Construct a new instance that contains no filter condition.
     */
    public DataFlowFilter() {
    }

    /**
     * Determine whether the condition to select data flows is empty or not.
     *
     * @return  {@code true} is returned if no condition is configured in
     *          this instance. Otherwise {@code false} is returned.
     */
    public boolean isEmpty() {
        return (sourceHost == null && node == null && port == null);
    }

    /**
     * Return the source L2 host configured in this instance.
     *
     * @return  A {@link DataLinkHost} if the source L2 host is configured.
     *          {@code null} if not configured.
     */
    public DataLinkHost getSourceHost() {
        return sourceHost;
    }

    /**
     * Set the source L2 host used to select data flows.
     *
     * <ul>
     *   <li>
     *     If a {@link DataLinkHost} instance is specified, only data flows
     *     that map packets sent by the specified L2 host will be selected.
     *     <ul>
     *       <li>
     *         Currently the VTN Manager handles only ethernet frame.
     *         Thus, in reality, an
     *         {@link org.opendaylight.vtn.manager.EthernetHost} instance
     *         needs to be specified.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     The condition for the source L2 host is cleared if a {@code null}
     *     is specified.
     *   </li>
     * </ul>
     *
     * @param host  A {@link DataLinkHost} instance or {@code null}.
     * @return  This instance.
     */
    public DataFlowFilter setSourceHost(DataLinkHost host) {
        sourceHost = host;
        return this;
    }

    /**
     * Return a {@link Node} instance configured in this instance.
     *
     * @return  A {@link Node} instance corresponding to a physical switch
     *          if configured.
     *          {@code null} if not configured.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Set the physical switch used to select data flows.
     *
     * <ul>
     *   <li>
     *     If a {@link Node} instance is specified and the switch port is
     *     not configured, only data flows that forward packets via the
     *     specified switch will be selected.
     *   </li>
     *   <li>
     *     If a {@link Node} instance is specified and the switch port is
     *     configured by {@link #setSwitchPort(SwitchPort)}, only data flows
     *     that forward packets via the specified switch port will be selected.
     *   </li>
     *   <li>
     *     The condition for the physical switch and physical switch port
     *     are cleared if a {@code null} is specified.
     *   </li>
     * </ul>
     *
     * @param node  A {@link Node} instance or {@code null}.
     * @return  This instance.
     */
    public DataFlowFilter setNode(Node node) {
        this.node = node;
        return this;
    }

    /**
     * Return a {@link SwitchPort} instance configured in this instance.
     *
     * @return  A {@link SwitchPort} instance which specifies the location
     *          of the switch port if configured.
     *          {@code null} if not configured.
     */
    public SwitchPort getSwitchPort() {
        return port;
    }

    /**
     * Set the location of the physical switch used to select data flows.
     *
     * <ul>
     *   <li>
     *     If a {@link SwitchPort} instance is specified and the physical
     *     switch is configured by {@link #setNode(Node)}, only data flows
     *     that forward packets via the specified switch port will be selected.
     *     Note that the specified {@link SwitchPort} instance is ignored
     *     unless a {@link Node} instance is configured in this instance.
     *   </li>
     *   <li>
     *     The condition for the physical switch port is cleared if a
     *     {@code null} is specified.
     *   </li>
     * </ul>
     *
     * @param port  A {@link SwitchPort} instance or {@code null}.
     * @return  This instance.
     */
    public DataFlowFilter setSwitchPort(SwitchPort port) {
        this.port = port;
        return this;
    }
}
