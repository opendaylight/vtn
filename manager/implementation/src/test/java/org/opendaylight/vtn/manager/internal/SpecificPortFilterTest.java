/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.Assert;
import org.junit.Test;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

/**
 * JUnit test for {@link SpecificPortFilter}
 */
public class SpecificPortFilterTest extends TestBase {
    /**
     * Test method for
     * {@link SpecificPortFilter#accept(NodeConnector,VtnPort)}
     */
    @Test
    public void testAccept() {
        SpecificPortFilter specificPortFilter = null;
        try {
            for (long port = 1L; port <= 4L; port++) {
                int ncid = 4;
                for (long dpid = 1L; dpid <= 4L; dpid++) {
                    for (NodeConnector nc: createNodeConnectors(ncid)) {
                        SalPort sport = new SalPort(dpid, port);
                        VtnPort vport = createVtnPortBuilder(sport).build();
                        SpecificPortFilter spesificPortFilter = new SpecificPortFilter(nc);
                        Assert.assertEquals(true,  specificPortFilter.accept(nc, vport));
                    }
                }
            }
        } catch (Exception e) {
            Assert.assertTrue(e instanceof NullPointerException);
        }
    }
}
