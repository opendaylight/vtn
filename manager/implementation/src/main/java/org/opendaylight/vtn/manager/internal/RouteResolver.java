/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.Path;

/**
 * {@code RouteResolver} provides interfaces to be implemented by classes
 * which resolves the packet route in physical network.
 */
public interface RouteResolver {
    /**
     * Return the packet route from the source to the destination switch.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param src  A {@link Node} instance corresponding th the source switch.
     * @param dst  A {@link Node} instance corresponding th the destination
     *             switch.
     * @return     A {@link Path} instance which represents the packet route.
     *             {@code null} is returned if no route was found.
     */
    Path getRoute(Node src, Node dst);
}
