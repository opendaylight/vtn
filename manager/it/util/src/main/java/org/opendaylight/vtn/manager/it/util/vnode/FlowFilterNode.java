/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;

/**
 * {@code FlowFilterNode} describes the configuration for the virtual node
 * that can contains flow filters.
 */
public interface FlowFilterNode {
    /**
     * Return the flow filter list for incoming packets.
     *
     * @return  A {@link FlowFilterList} instance that contains flow filters
     *          for incoming packets.
     */
    FlowFilterList getInputFilter();

    /**
     * Return the flow filter list for outgoing packets.
     *
     * @return  A {@link FlowFilterList} instance that contains flow filters
     *          for outgoing packets.
     */
    FlowFilterList getOutputFilter();
}
