/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.ArrayList;
import java.util.List;

/**
 * {@code FlowEntryRemover} is used to remove flow entries that match the
 * specified condition.
 */
class FlowEntryRemover implements FlowTableScanner {
    /**
     * A list of flow entries removed from the flow table.
     */
    private final List<OfMockFlowEntry>  removedFlows = new ArrayList<>();

    /**
     * Return removed flow entries.
     *
     * @return  A list of flow entries removed from the flow table.
     */
    public List<OfMockFlowEntry> getRemovedFlows() {
        return removedFlows;
    }

    // FlowTableScanner

    /**
     * Invoked when a flow entry to be removed has been found.
     *
     * @param ofent  A flow entry to be removed.
     * @return  {@code true}.
     */
    @Override
    public boolean flowEntryFound(OfMockFlowEntry ofent) {
        removedFlows.add(ofent);
        return true;
    }
}
