/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.ClusterEvent;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;

/**
 * The listener interface for receiving posted cluster events.
 */
public interface ClusterEventListener {
    /**
     * Invoked when a new cluster event has been posted.
     *
     * @param id     Identifier of the posted cluster event.
     * @param event  A {@link ClusterEvent} instance.
     */
    void posted(ClusterEventId id, ClusterEvent event);
}
