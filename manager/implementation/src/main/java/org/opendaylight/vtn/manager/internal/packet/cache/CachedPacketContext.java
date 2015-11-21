/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet.cache;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionContext;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchContext;

/**
 * {@code CachedPacketContext} describes a runtime context to handle packet
 * represented by {@link CachedPacket}.
 */
public interface CachedPacketContext
    extends FlowMatchContext, FlowActionContext {
}
