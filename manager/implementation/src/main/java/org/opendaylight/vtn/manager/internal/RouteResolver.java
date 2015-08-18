/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.List;

import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

/**
 * {@code RouteResolver} provides interfaces to be implemented by classes
 * which resolves the packet route in physical network.
 */
public interface RouteResolver {
    /**
     * Return the packet route from the source to the destination switch.
     *
     * @param rdr  A {@link InventoryReader} instance which contains active
     *             read transaction for the MD-SAL datastore.
     * @param src  A {@link SalNode} instance corresponding to the source
     *             switch.
     * @param dst  A {@link SalNode} instance corresponding to the destination
     *             switch.
     * @return     A list of {@link LinkEdge} instances which represents the
     *             packet route.
     *             An empty list is returned if the destination node is the
     *             same as the source.
     *             {@code null} is returned if no route was found.
     */
    List<LinkEdge> getRoute(InventoryReader rdr, SalNode src, SalNode dst);

    /**
     * Return the identifier of the path policy associated with this
     * route resolver.
     *
     * @return  The identifier of the path policy.
     */
    int getPathPolicyId();
}
