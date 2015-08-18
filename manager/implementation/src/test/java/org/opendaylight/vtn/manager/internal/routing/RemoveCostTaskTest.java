/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Assert;
import org.junit.Test;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
  * Unit Test for RemoveCostTask.
  */
public class RemoveCostTaskTest extends Assert {

    /**
     * Set the Port ID.
     */
    private static final int ID = 20;

    /**
     * Return the target switch port descriptor.
     *
     * Test Case for VtnPortDesc.
     */
    @Test
    public void testGetPortDesc() {
        VtnPortDesc[] portDescs = {
            null,
            new VtnPortDesc("openflow:1,,"),
            new VtnPortDesc("openflow:1,2,"),
            new VtnPortDesc("openflow:1,2,eth2")
        };
        for (VtnPortDesc port : portDescs) {
            RemoveCostTask removeCostTask = new RemoveCostTask(ID, port);
            assertEquals(port, removeCostTask.getPortDesc());
        }
    }
}
