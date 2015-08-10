/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;

/**
 * An instance to keep topology changes.
 */
final class TopologyEventContext {
    /**
     * A list of {@link VtnLink} instances to be created.
     */
    private final List<VtnLink>  created = new ArrayList<>();

    /**
     * A list of {@link VtnLink} instances to be removed.
     */
    private final List<VtnLink>  removed = new ArrayList<>();

    /**
     * Construct a new instance.
     */
    TopologyEventContext() {
    }

    /**
     * Add the given {@link VtnLink} instance to the link list to be added.
     *
     * @param vlink  A {@link VtnLink} instance to be added.
     */
    void addCreated(VtnLink vlink) {
        created.add(vlink);
    }

    /**
     * Add the given {@link VtnLink} instance to the link list to be removed.
     *
     * @param vlink  A {@link VtnLink} instance to be added.
     */
    void addRemoved(VtnLink vlink) {
        removed.add(vlink);
    }

    /**
     * Return a list of links to be created.
     *
     * @return  A list of {@link VtnLink} instances.
     */
    List<VtnLink> getCreated() {
        return created;
    }

    /**
     * Return a list of links to be removed.
     *
     * @return  A list of {@link VtnLink} instances.
     */
    List<VtnLink> getRemoved() {
        return removed;
    }
}
