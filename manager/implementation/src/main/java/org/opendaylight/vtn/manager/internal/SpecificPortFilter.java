/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.PortProperty;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@link PortFilter} implementation which accepts {@link NodeConnector}
 * instance which equals to the specified one.
 */
public class SpecificPortFilter implements PortFilter {
    /**
     * The target {@link NodeConnector} instance.
     */
    private final NodeConnector  targetPort;

    /**
     * Construct a new instance.
     *
     * @param port  The target {@link NodeConnector} instance.
     *              Specifying {@code null} results in undefined behavior.
     */
    public SpecificPortFilter(NodeConnector port) {
        targetPort = port;
    }

    /**
     * Test if the specified switch port is identical to the target port.
     *
     * @param port  A {@link NodeConnector} object corresponding to the
     *              switch port to be tested.
     * @param prop  Unused.
     * @return  {@code true} if the specified port is identical to the
     *          target port. Otherwise {@code false} is returned.
     */
    @Override
    public boolean accept(NodeConnector port, PortProperty prop) {
        return targetPort.equals(port);
    }
}
