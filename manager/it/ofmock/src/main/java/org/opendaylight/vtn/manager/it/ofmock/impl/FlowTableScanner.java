/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

/**
 * {@code FlowTableScanner} is used to scan flow entries in a flow table
 * in a mock-up of OpenFlow switch.
 */
interface FlowTableScanner {
    /**
     * Invoked when a flow entry that satisfies the specified condition has
     * been found.
     *
     * @param ofent  A flow entry.
     * @return  {@code true} means the given flow entry should be removed from
     *          the flow table. {@code false} means the given flow entry should
     *          be retained.
     */
    boolean flowEntryFound(OfMockFlowEntry ofent);
}
