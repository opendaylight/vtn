/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.HashSet;
import java.util.List;

import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

import org.opendaylight.controller.forwardingrulesmanager.FlowEntry;
import org.opendaylight.controller.forwardingrulesmanager.
    IForwardingRulesManager;

/**
 * {@link VTNFlowMatch} implementation which accepts VTN flows which are
 * already removed.
 */
public class RemovedFlowMatch implements VTNFlowMatch {
    /**
     * Forwarding rules manager service.
     */
    private final IForwardingRulesManager  fwRulesManager;

    /**
     * Construct a new instance.
     *
     * @param frm  Forwarding rules manager service.
     */
    public RemovedFlowMatch(IForwardingRulesManager frm) {
        fwRulesManager = frm;
    }

    /**
     * Test if the specified VTN flow should be accepted or not.
     *
     * @param vflow  A {@link VTNFlow} instance to be tested.
     * @return  {@code true} if the specified flow is already removed.
     *          Otherwise {@code false}.
     */
    @Override
    public boolean accept(VTNFlow vflow) {
        HashSet<String> installed = new HashSet<String>();
        String group = vflow.getGroupId().toString();
        for (FlowEntry fent: fwRulesManager.getFlowEntriesForGroup(group)) {
            installed.add(fent.getFlowName());
        }

        List<FlowEntry> entries = vflow.getFlowEntries();
        if (entries.size() != installed.size()) {
            return true;
        }

        for (FlowEntry fent: entries) {
            if (!installed.contains(fent.getFlowName())) {
                return true;
            }
        }

        return false;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return "removed";
    }
}
