/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.List;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link RemovePathPolicyTask}
 */
public class RemovePathPolicyTaskTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>
     *     {@link RemovePathPolicyTask#create(TopologyGraph, RemovePathPolicyInput)}
     *   </li>
     *   <li>{@link RemovePathPolicyTask#getOutputType()}</li>
     *   <li>{@link RemovePathPolicyTask#createOutput(VtnUpdateType)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "RPC input cannot be null";
        try {
            RemovePathPolicyTask.create(null, null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        RemovePathPolicyInputBuilder builder =
            new RemovePathPolicyInputBuilder();
        msg = "Path policy ID cannot be null";
        try {
            RemovePathPolicyTask.create(null, builder.build());
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        for (int id = 1; id <= 3; id++) {
            builder.setId(id);
            RemovePathPolicyTask task =
                RemovePathPolicyTask.create(null, builder.build());
            assertNotNull(task);
            assertEquals(Void.class, task.getOutputType());

            for (VtnUpdateType type: VtnUpdateType.values()) {
                assertEquals(null, task.createOutput(type));
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>
     *     {@link RemovePathPolicyTask#onStarted(TxContext,VtnPathPolicy)}
     *   </li>
     *   <li>
     *     {@link RemovePathPolicyTask#onSuccess(VTNManagerProvider,VtnUpdateType)}
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnStarted() throws Exception {
        TxContext ctx = Mockito.mock(TxContext.class);
        VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
        TopologyGraph topo = new TopologyGraph(provider);
        Integer[] policies = {1, 2, 3};

        VTNFuture<Void> f = new SettableVTNFuture<>();
        Mockito.when(provider.removeFlows(
                         Mockito.isA(PathPolicyFlowRemover.class))).
            thenReturn(f);

        int times = 0;
        for (Integer id: policies) {
            topo.updateResolver(id);

            RemovePathPolicyInput input = new RemovePathPolicyInputBuilder().
                setId(id).build();
            RemovePathPolicyTask task =
                RemovePathPolicyTask.create(topo, input);
            assertNotNull(task);

            RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
            VtnErrorTag vtag = VtnErrorTag.NOTFOUND;
            String msg = id + ": Path policy does not exist.";
            try {
                task.onStarted(ctx, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            VtnPathPolicy vpp = new VtnPathPolicyBuilder().
                setId(id).build();
            task.onStarted(ctx, vpp);
            assertEquals(0, task.getBackgroundTasks().size());
            Mockito.verifyZeroInteractions(ctx);
            Mockito.verify(provider, Mockito.times(times)).
                removeFlows(Mockito.any(FlowRemover.class));

            times++;
            topo.removeResolver(id);
            task.onSuccess(provider, VtnUpdateType.REMOVED);
            List<VTNFuture<?>> bg = task.getBackgroundTasks();
            assertEquals(1, bg.size());
            assertEquals(f, bg.get(0));
            Mockito.verify(provider, Mockito.times(times)).
                removeFlows(Mockito.isA(PathPolicyFlowRemover.class));
        }
    }
}
