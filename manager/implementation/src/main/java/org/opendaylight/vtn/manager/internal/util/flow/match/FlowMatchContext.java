/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.internal.util.packet.PacketHeader;

/**
 * {@code FlowMatchContext} describes a context to build flow match.
 */
public interface FlowMatchContext extends PacketHeader {
    /**
     * Add a match field to be configured into a flow entry.
     *
     * @param type  A match type to be added.
     */
    void addMatchField(FlowMatchType type);

    /**
     * Determine whether the given match field will be configured in a flow
     * entry or not.
     *
     * @param type  A match type to be tested.
     * @return  {@code true} only if the given match type will be configured
     *          in a flow entry.
     */
    boolean hasMatchField(FlowMatchType type);

    /**
     * Add match fields to be configured into an unicast flow entry.
     */
    void addUnicastMatchFields();
}
