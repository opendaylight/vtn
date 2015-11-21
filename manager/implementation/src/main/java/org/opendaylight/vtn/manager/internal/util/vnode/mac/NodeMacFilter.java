/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * {@code NodeMacFilter} describes a MAC address table entry filter that
 * selects entries detected on the specified physical switch.
 */
public class NodeMacFilter implements MacEntryFilter {
    /**
     * The target switch.
     */
    private final NodeId  targetNode;

    /**
     * Construct a new instance.
     *
     * @param snode  A {@link SalNode} instance which specifies the target
     *               physical switch.
     */
    public NodeMacFilter(SalNode snode) {
        targetNode = snode.getNodeId();
    }

    // MacEntryFilter

    /**
     * Test if the specified MAC address table entry should be accepted or not.
     *
     * @param ment  A {@link MacTableEntry} instance to be tested.
     * @return  {@code true} only if the specified entry was detected on the
     *          specified physical switch.
     */
    @Override
    public boolean accept(MacTableEntry ment) {
        return targetNode.equals(ment.getNode());
    }
}
