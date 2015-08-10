/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLinkBuilder;

/**
 * JUnit test for {@link TopologyEventContext}.
 */
public class TopologyEventContextTest extends TestBase {

    TopologyEventContext event = new TopologyEventContext();
    VtnLinkBuilder vlb = new VtnLinkBuilder();
    VtnLink vlink  = vlb.build();

    /**
     * Test case for {@link TopologyEventContext#addCreated(VtnLink)}.
     */
    @Test
    public void testCreated() {
        event.addCreated(vlink);
        assertEquals(vlink, (event.getCreated()).get(0));
    }

    /**
     * Test case for {@link TopologyEventContext#addRemoved(VtnLink)}.
     */
    @Test
    public void testRemoved() {
        event.addRemoved(vlink);
        assertEquals(vlink, (event.getRemoved()).get(0));
    }
}
