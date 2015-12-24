/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.RemoveFlowInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.service.rev130819.SalFlowService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.FlowCookie;

/**
 * JUnit test for {@link RemoveFlowRpcList}.
 */
public class RemoveFlowRpcListTest extends TestBase {
    /**
     * Test case for
     * {@link RemoveFlowRpcList#add(SalNode, RemoveFlowInputBuilder)} and
     * {@link RemoveFlowRpcList#invoke(SalFlowService)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInvoke() throws Exception {
        RemoveFlowRpcList rpcList = new RemoveFlowRpcList();
        SalFlowService sfs = mock(SalFlowService.class);
        assertEquals(Collections.<RemoveFlowRpc>emptyList(),
                     rpcList.invoke(sfs));
        verifyZeroInteractions(sfs);

        // Remove flow entries in a single switch.
        rpcList = new RemoveFlowRpcList();
        SalNode snode1 = new SalNode(1L);

        List<RemoveFlowInput> inputs = new ArrayList<>();
        for (long l = 1L; l <= 10L; l++) {
            FlowCookie fc = new FlowCookie(BigInteger.valueOf(l));
            RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder().
                setCookie(fc);
            rpcList.add(snode1, builder);

            Boolean barrier = (l == 10L);
            RemoveFlowInput input = new RemoveFlowInputBuilder().
                setCookie(fc).
                setBarrier(barrier).
                build();
            inputs.add(input);
        }

        List<RemoveFlowRpc> rpcs = rpcList.invoke(sfs);
        Iterator<RemoveFlowInput> it = inputs.iterator();
        for (RemoveFlowRpc rpc: rpcs) {
            RemoveFlowInput input = it.next();
            assertEquals(input, rpc.getInput());
            verify(sfs).removeFlow(input);
        }
        assertEquals(false, it.hasNext());
        verifyNoMoreInteractions(sfs);

        // Remove flow entries in multiple switches.
        reset(sfs);
        rpcList = new RemoveFlowRpcList();
        SalNode[] snodes = {
            snode1,
            new SalNode(-1L),
            new SalNode(1234567L),
            new SalNode(99887766554433L),
            new SalNode(112233445566L),
            new SalNode(-12345L),
            new SalNode(7777777777777L),
        };
        Random rand = new Random(0x123456789abcdeL);
        Map<SalNode, FlowCookie> lastCookie = new HashMap<>();
        List<RemoveFlowInputBuilder> builders = new ArrayList<>();
        for (long l = 1L; l <= 50L; l++) {
            FlowCookie fc = new FlowCookie(BigInteger.valueOf(l));
            SalNode snode = snodes[rand.nextInt(snodes.length)];
            RemoveFlowInputBuilder builder = new RemoveFlowInputBuilder().
                setCookie(fc);
            rpcList.add(snode, builder);
            lastCookie.put(snode, fc);
            builders.add(new RemoveFlowInputBuilder().setCookie(fc));
        }

        Set<FlowCookie> lastSet = new HashSet<>(lastCookie.values());
        inputs.clear();
        for (RemoveFlowInputBuilder builder: builders) {
            FlowCookie fc = builder.getCookie();
            Boolean barrier = lastSet.contains(fc);
            inputs.add(builder.setBarrier(barrier).build());
        }

        rpcs = rpcList.invoke(sfs);
        it = inputs.iterator();
        for (RemoveFlowRpc rpc: rpcs) {
            RemoveFlowInput input = it.next();
            assertEquals(input, rpc.getInput());
            verify(sfs).removeFlow(input);
        }
        assertEquals(false, it.hasNext());
        verifyNoMoreInteractions(sfs);
    }
}
