/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link MacMapPortBusyException}.
 */
public class MacMapPortBusyExceptionTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VnodeName vtnName = new VnodeName("tenant");
        RpcErrorTag etag = RpcErrorTag.IN_USE;
        VtnErrorTag vtag = VtnErrorTag.CONFLICT;
        String msg = "VLAN on a switch port is reserved";

        for (int i = 0; i < 10; i++) {
            VnodeName vbrName = new VnodeName("bridge" + i);
            MacMapIdentifier mpath = new MacMapIdentifier(vtnName, vbrName);
            VBridgeIfIdentifier ifId = new VBridgeIfIdentifier(
                new VnodeName("vtn"), new VnodeName("vbr"),
                new VnodeName("if_" + i));
            MacVlan mvlan = new MacVlan((long)(i + 10), (short)i);
            MacMapPortBusyException e =
                new MacMapPortBusyException(mvlan, mpath, ifId);

            assertEquals(mvlan, e.getHost());
            assertEquals(mpath, e.getIdentifier());
            assertEquals(ifId, e.getAnotherMapping());

            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());

            String emsg = "Switch port is reserved by " + ifId;
            assertEquals(emsg, e.getErrorMessage());
        }
    }
}
