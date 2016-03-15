/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;

/**
 * {@code BarrierSender} describes an internal service that send barrier
 * request to the specified switch.
 */
public interface BarrierSender {
    /**
     * Send a barrier request to the specified switch asynchronously.
     *
     * @param nref  Reference to the target node.
     */
    void asyncBarrier(NodeRef nref);
}
