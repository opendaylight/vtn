/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.Mockito.isA;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.output.SetPathCostResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link SetPathCostTask}.
 */
public class SetPathCostTaskTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>
     *     {@link SetPathCostTask#create(TopologyGraph, SetPathCostInput)}
     *   </li>
     *   <li>{@link SetPathCostTask#getOutputType()}</li>
     *   <li>{@link SetPathCostTask#createOutput(List)}</li>
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
            SetPathCostTask.create(null, null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        msg = "Path policy ID cannot be null";
        SetPathCostInput input = new SetPathCostInputBuilder().build();
        try {
            SetPathCostTask.create(null, input);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        Integer pid = 3;
        List<List<PathCostList>> costCases = new ArrayList<>();
        Collections.addAll(costCases, null,
                           Collections.<PathCostList>emptyList());
        msg = "At least one switch port must be specified.";
        for (List<PathCostList> costs: costCases) {
            input = new SetPathCostInputBuilder().
                setId(pid).setPathCostList(costs).build();
            try {
                SetPathCostTask.create(null, input);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        msg = "vtn-port-desc cannot be null";
        input = new SetPathCostInputBuilder().
            setId(pid).
            setPathCostList(
                Collections.singletonList(new PathCostListBuilder().build())).
            build();
        try {
            SetPathCostTask.create(null, input);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        VtnPortDesc dup = new VtnPortDesc("openflow:1,4,");
        List<PathCostList> costs = new ArrayList<>();
        Collections.addAll(
            costs,
            new PathCostListBuilder().setPortDesc(dup).setCost(123L).build(),
            new PathCostListBuilder().setPortDesc(
                new VtnPortDesc("openflow:3,,port-3")).setCost(5L).build(),
            new PathCostListBuilder().setPortDesc(dup).setCost(3L).build());
        input = new SetPathCostInputBuilder().
            setId(pid).setPathCostList(costs).build();
        msg = "Duplicate port descriptor: " + dup.getValue();
        etag = RpcErrorTag.BAD_ELEMENT;
        try {
            SetPathCostTask.create(null, input);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        VtnPortDesc desc1 = new VtnPortDesc("openflow:1,2,port-2");
        VtnPortDesc desc2 = new VtnPortDesc("openflow:2,,port-123");
        VtnPortDesc desc3 = new VtnPortDesc("openflow:3,45,");
        PathCostList cost1 = new PathCostListBuilder().
            setPortDesc(desc1).setCost(1L).build();
        PathCostList cost2 = new PathCostListBuilder().
            setPortDesc(desc2).setCost(1234567L).build();
        PathCostList cost3 = new PathCostListBuilder().
            setPortDesc(desc3).setCost(7777L).build();
        costs.clear();
        Collections.addAll(costs, cost1, cost2, cost3);
        Map<VtnPortDesc, VtnUpdateType> results = new LinkedHashMap<>();
        assertEquals(null, results.put(desc1, VtnUpdateType.CREATED));
        assertEquals(null, results.put(desc2, null));
        assertEquals(null, results.put(desc3, VtnUpdateType.CHANGED));

        input = new SetPathCostInputBuilder().
            setId(pid).setPathCostList(costs).build();

        SetPathCostTask task = SetPathCostTask.create(null, input);
        assertNotNull(task);
        assertEquals(SetPathCostOutput.class, task.getOutputType());

        List<SetCostTask> subTasks = task.getSubTasks();
        assertEquals(results.size(), subTasks.size());
        Iterator<Entry<VtnPortDesc, VtnUpdateType>> it =
            results.entrySet().iterator();
        List<VtnUpdateType> res = new ArrayList<>();
        for (SetCostTask sub: subTasks) {
            Entry<VtnPortDesc, VtnUpdateType> entry = it.next();
            assertEquals(entry.getKey(), sub.getPortDesc());
            res.add(entry.getValue());
        }

        it = results.entrySet().iterator();
        SetPathCostOutput out = task.createOutput(res);
        for (SetPathCostResult r: out.getSetPathCostResult()) {
            Entry<VtnPortDesc, VtnUpdateType> entry = it.next();
            assertEquals(entry.getKey(), r.getPortDesc());
            assertEquals(entry.getValue(), r.getStatus());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link SetPathCostTask#onStarted(TxContext)}</li>
     *   <li>{@link SetPathCostTask#onSuccess(VTNManagerProvider,List)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnStarted() throws Exception {
        TxContext ctx = mock(TxContext.class);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        TopologyGraph topo = new TopologyGraph(provider);
        Integer[] policies = {1, 2, 3};

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        VtnPortDesc desc1 = new VtnPortDesc("openflow:1,2,port-2");
        VtnPortDesc desc2 = new VtnPortDesc("openflow:2,,port-123");
        VtnPortDesc desc3 = new VtnPortDesc("openflow:3,45,");
        PathCostList cost1 = new PathCostListBuilder().
            setPortDesc(desc1).setCost(1L).build();
        PathCostList cost2 = new PathCostListBuilder().
            setPortDesc(desc2).setCost(1234567L).build();
        PathCostList cost3 = new PathCostListBuilder().
            setPortDesc(desc3).setCost(7777L).build();
        List<PathCostList> costs = new ArrayList<>();
        Collections.addAll(costs, cost1, cost2, cost3);

        List<VtnUpdateType> unchanged = new ArrayList<>();
        Collections.addAll(unchanged, null, null, null);

        List<VtnUpdateType> created = new ArrayList<>();
        Collections.addAll(created, null, null, VtnUpdateType.CREATED);

        List<VtnUpdateType> changed = new ArrayList<>();
        Collections.addAll(changed, null, VtnUpdateType.CHANGED, null);

        for (Integer id: policies) {
            InstanceIdentifier<VtnPathPolicy> path = InstanceIdentifier.
                builder(VtnPathPolicies.class).
                child(VtnPathPolicy.class, new VtnPathPolicyKey(id)).
                build();

            reset(provider, ctx, tx);
            assertEquals(null, topo.getResolver(id));

            SetPathCostInput input = new SetPathCostInputBuilder().
                setId(id).setPathCostList(costs).build();
            SetPathCostTask task = SetPathCostTask.create(topo, input);
            assertNotNull(task);

            // The target path policy is not present.
            VtnPathPolicy vpp = null;
            when(ctx.getReadWriteTransaction()).thenReturn(tx);
            when(tx.read(oper, path)).thenReturn(getReadResult(vpp));

            RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
            VtnErrorTag vtag = VtnErrorTag.NOTFOUND;
            String msg = id + ": Path policy does not exist.";
            try {
                task.onStarted(ctx);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            verify(ctx).getReadWriteTransaction();
            verify(tx).read(oper, path);
            verifyNoMoreInteractions(provider, ctx, tx);
            reset(provider, ctx, tx);

            // The target path policy is present.
            vpp = new VtnPathPolicyBuilder().setId(id).build();
            when(ctx.getReadWriteTransaction()).thenReturn(tx);
            when(tx.read(oper, path)).thenReturn(getReadResult(vpp));
            task.onStarted(ctx);

            verify(ctx).getReadWriteTransaction();
            verify(tx).read(oper, path);
            verifyNoMoreInteractions(provider, ctx, tx);

            // No path cost is updated.
            task.onSuccess(provider, unchanged);
            verifyZeroInteractions(provider);
            assertEquals(0, task.getBackgroundTasks().size());

            // A path cost is created.
            SettableVTNFuture<Void> future = new SettableVTNFuture<>();
            List<VTNFuture<?>> futures = new ArrayList<>();
            futures.add(future);
            topo.updateResolver(id);
            ArgumentCaptor<PathPolicyFlowRemover> captor =
                ArgumentCaptor.forClass(PathPolicyFlowRemover.class);
            when(provider.removeFlows(isA(PathPolicyFlowRemover.class))).
                thenReturn(future);
            task.onSuccess(provider, created);
            assertEquals(futures, task.getBackgroundTasks());
            verify(provider).removeFlows(captor.capture());
            verify(provider).getDataBroker();
            List<PathPolicyFlowRemover> frs = captor.getAllValues();
            assertEquals(1, frs.size());
            PathPolicyFlowRemover fr = frs.get(0);
            Set<Integer> idSet = Collections.singleton(id);
            assertEquals(idSet, fr.getPathPolicyIds());
            verifyNoMoreInteractions(provider, ctx, tx);
            reset(provider, ctx, tx);

            // A path cost is changed.
            future = new SettableVTNFuture<>();
            futures.add(future);
            captor = ArgumentCaptor.forClass(PathPolicyFlowRemover.class);
            when(provider.removeFlows(isA(PathPolicyFlowRemover.class))).
                thenReturn(future);
            task.onSuccess(provider, changed);
            assertEquals(futures, task.getBackgroundTasks());
            verify(provider).removeFlows(captor.capture());
            frs = captor.getAllValues();
            assertEquals(1, frs.size());
            fr = frs.get(0);
            assertEquals(idSet, fr.getPathPolicyIds());
            verifyNoMoreInteractions(provider, ctx, tx);
        }
    }
}
