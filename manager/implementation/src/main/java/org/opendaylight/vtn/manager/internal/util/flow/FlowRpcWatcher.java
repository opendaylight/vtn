/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeRpcWatcher;

/**
 * {@code FlowRpcWatcher} watches all the invocations of RPC that modify
 * flow entries.
 *
 * <p>
 *   If a swtich is removed, all the RPC invocations routed to the removed
 *   switch will be canceled.
 * </p>
 */
public interface FlowRpcWatcher extends NodeRpcWatcher, BarrierSender {
}
