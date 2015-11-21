/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.Collection;

import org.opendaylight.vtn.manager.internal.util.packet.PacketHeader;

/**
 * {@code FlowActionContext} describes a context to apply flow actions
 * configured in a flow filter to packet.
 */
public interface FlowActionContext extends PacketHeader {
    /**
     * Append the given flow action to the action list to be applied to packet.
     *
     * <p>
     *   This method is used to apply flow actions configured in a flow filter.
     * </p>
     *
     * @param fa  A {@link FlowFilterAction} instance.
     */
    void addFilterAction(FlowFilterAction fa);

    /**
     * Remove the specified flow action from the flow filter action list.
     *
     * @param actClass  A class of flow action to be removed.
     */
    void removeFilterAction(Class<? extends FlowFilterAction> actClass);

    /**
     * Return a collection of flow actions configured by a flow filter.
     *
     * @return  A collection of flow actions.
     *          {@code null} is returned if no flow action was created by
     *          flow filter.
     */
    Collection<FlowFilterAction> getFilterActions();
}
