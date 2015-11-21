/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchContext;

/**
 * {@code PathMapContext} describes a runtime context to evaluate path map
 * against packets.
 */
public interface PathMapContext extends FlowMatchContext {
    /**
     * Return the VTN Manager provider.
     *
     * @return  A {@link VTNManagerProvider} instance.
     */
    VTNManagerProvider getProvider();

    /**
     * Return the flow condition reader associated with the current MD-SAL
     * datastore transaction.
     *
     * @return  A {@link FlowCondReader} instance.
     */
    FlowCondReader getFlowCondReader();

    /**
     * Set timeout for the ingress flow.
     *
     * @param idle  An idle timeout for the ingress flow.
     * @param hard  A hard timeout for the ingress flow.
     */
    void setFlowTimeout(int idle, int hard);

    /**
     * Return a brief description of the ethernet frame in this context.
     *
     * @return  A brief description of the ethernet frame in ths context.
     */
    String getDescription();
}
