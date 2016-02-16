/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.Layer4Match;

/**
 * Test case for {@link VTNLayer4Match}.
 */
public class VTNLayer4MatchTest extends TestBase {
    /**
     * Error test case for {@link VTNLayer4Match#create(VtnLayer4Match)}.
     */
    @Test
    public void testCreateError1() {
        VtnLayer4Match vl4 = mock(VtnLayer4Match.class);
        try {
            VTNLayer4Match.create(vl4);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Unexpected VTN L4 match instance: " + vl4,
                         e.getMessage());
        }
    }

    /**
     * Error test case for {@link VTNLayer4Match#create(Match)}.
     */
    @Test
    public void testCreateError2() {
        Layer4Match l4 = mock(Layer4Match.class);
        Match match = mock(Match.class);
        when(match.getLayer4Match()).thenReturn(l4);
        try {
            VTNLayer4Match.create(match);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Unsupported MD-SAL L4 match instance: " + l4,
                         e.getMessage());
        }
    }
}
