/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.junit.Assert.assertEquals;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;

/**
 * This class describes the number of flow entries per switch.
 */
public final class FlowCounter {
    /**
     * A map that keeps the number of flow entries per switch.
     */
    private final Map<String, Integer>  countMap = new HashMap<>();

    /**
     * openflowplugin mock-up service.
     */
    private final OfMockService  ofMockService;

    /**
     * Construct a new instance.
     *
     * @param ofmock  openflowplugin mock-up service.
     */
    public FlowCounter(OfMockService ofmock) {
        ofMockService = ofmock;
    }

    /**
     * Count the given flow entry.
     *
     * @param flow  A flow entry.
     * @return  This instance.
     */
    public FlowCounter add(OfMockFlow flow) {
        String nid = flow.getNodeIdentifier();
        Integer count = countMap.get(nid);
        int c = (count == null) ? 1 : (count.intValue() + 1);
        countMap.put(nid, Integer.valueOf(c));
        return this;
    }

    /**
     * Add the number of flow entries in the given unicast flow to this
     * instance.
     *
     * @param unicast  A {@link UnicastFlow} instance.
     * @return  This instance.
     */
    public FlowCounter add(UnicastFlow unicast) {
        for (OfMockFlow flow: unicast.getFlowList()) {
            add(flow);
        }
        return this;
    }

    /**
     * Add the number of flow entries in the given list of unicast flows
     * to this instance.
     *
     * @param list  A list of {@link UnicastFlow} instances.
     * @return  This instance.
     */
    public FlowCounter add(List<UnicastFlow> list) {
        for (UnicastFlow unicast: list) {
            add(unicast);
        }
        return this;
    }

    /**
     * Verify the number of flow entries installed in all switches.
     *
     * @return  This instance.
     */
    public FlowCounter verify() {
        for (String nid: ofMockService.getNodes()) {
            Integer c = countMap.get(nid);
            int count = (c == null) ? 0 : c.intValue();
            assertEquals(count, ofMockService.getFlowCount(nid));
        }

        return this;
    }

    /**
     * Reset the counter.
     *
     * @return  This instance.
     */
    public FlowCounter clear() {
        countMap.clear();
        return this;
    }
}
