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

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.AllFlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link SetPathPolicyTask}.
 */
public class SetPathPolicyTaskTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>
     *     {@link SetPathPolicyTask#create(TopologyGraph, SetPathPolicyInput)}
     *   </li>
     *   <li>{@link SetPathPolicyTask#fixMissingParents()}</li>
     *   <li>{@link SetPathPolicyTask#getOutputType()}</li>
     *   <li>{@link SetPathPolicyTask#createOutput(VtnUpdateType)}</li>
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
            SetPathPolicyTask.create(null, null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        VtnUpdateOperationType op = VtnUpdateOperationType.REMOVE;
        SetPathPolicyInputBuilder builder = new SetPathPolicyInputBuilder().
            setOperation(op);
        etag = RpcErrorTag.BAD_ELEMENT;
        msg = "Invalid operation type: " + op;
        try {
            SetPathPolicyTask.create(null, builder.build());
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        builder.setOperation(VtnUpdateOperationType.SET);
        etag = RpcErrorTag.MISSING_ELEMENT;
        msg = "Path policy ID cannot be null";
        try {
            SetPathPolicyTask.create(null, builder.build());
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        for (int id = 1; id <= 3; id++) {
            builder.setOperation(VtnUpdateOperationType.SET).setId(id).
                setDefaultCost(null);

            SetPathPolicyTask task =
                SetPathPolicyTask.create(null, builder.build());
            assertNotNull(task);
            assertEquals(true, task.fixMissingParents());
            assertEquals(SetPathPolicyOutput.class, task.getOutputType());

            for (VtnUpdateType type: VtnUpdateType.values()) {
                SetPathPolicyOutput output = task.createOutput(type);
                assertEquals(type, output.getStatus());
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>
     *     {@link SetPathPolicyTask#onStarted(TxContext,org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy)}
     *   </li>
     *   <li>
     *     {@link SetPathPolicyTask#onSuccess(VTNManagerProvider,VtnUpdateType)}
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

        for (Integer id: policies) {
            Mockito.reset(provider);
            assertEquals(null, topo.getResolver(id));

            SetPathPolicyInputBuilder builder = new SetPathPolicyInputBuilder().
                setId(id).setOperation(VtnUpdateOperationType.SET).
                setPresent(true);
            SetPathPolicyInput input = builder.build();
            SetPathPolicyTask task =
                SetPathPolicyTask.create(topo, input);
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

            input = builder.setPresent(false).build();
            task = SetPathPolicyTask.create(topo, input);
            assertNotNull(task);
            task.onStarted(ctx, null);
            assertEquals(0, task.getBackgroundTasks().size());
            Mockito.verifyZeroInteractions(ctx);

            // Path policy is not changed.
            task.onSuccess(provider, null);
            Mockito.verifyZeroInteractions(provider);
            assertEquals(0, task.getBackgroundTasks().size());

            // Path policy is created.
            SettableVTNFuture<Void> future = new SettableVTNFuture<>();
            List<VTNFuture<?>> futures = new ArrayList<>();
            futures.add(future);
            VtnUpdateType result = VtnUpdateType.CREATED;
            topo.updateResolver(id);
            Mockito.when(
                provider.removeFlows(Mockito.isA(AllFlowRemover.class))).
                thenReturn(future);
            task.onSuccess(provider, result);
            assertEquals(futures, task.getBackgroundTasks());
            Mockito.verify(provider).
                removeFlows(Mockito.isA(AllFlowRemover.class));
            Mockito.verify(provider, Mockito.never()).
                removeFlows(Mockito.isA(PathPolicyFlowRemover.class));

            // Path policy is updated.
            future = new SettableVTNFuture<>();
            futures.add(future);
            Mockito.reset(provider);
            result = VtnUpdateType.CHANGED;
            Mockito.when(provider.removeFlows(
                             Mockito.isA(PathPolicyFlowRemover.class))).
                thenReturn(future);
            task.onSuccess(provider, result);
            assertEquals(futures, task.getBackgroundTasks());
            Mockito.verify(provider, Mockito.never()).
                removeFlows(Mockito.isA(AllFlowRemover.class));
            Mockito.verify(provider).
                removeFlows(Mockito.isA(PathPolicyFlowRemover.class));
        }
    }
}
