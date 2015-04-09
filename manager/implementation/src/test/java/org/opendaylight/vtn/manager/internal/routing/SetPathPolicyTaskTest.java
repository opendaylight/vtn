/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;
import org.opendaylight.vtn.manager.PathPolicy;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TxContext;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyConfigBuilder;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * JUnit test for {@link SetPathPolicyTask}.
 *
 */
public class SetPathPolicyTaskTest extends TestBase {
    /**
     * method to test create of {@link SetPathPolicyTask}
     *
     * @throws Exception
     */
    @Test
    public void testCreate() throws Exception {
        VTNManagerProvider provider = new VTNProviderStub();
        PathPolicy pp = new PathPolicy(1, 1L, null);

        // operation type as set
        SetPathPolicyInput input = new PathPolicyConfigBuilder.Rpc().set(pp)
                .getBuilder().setOperation(VtnUpdateOperationType.SET).build();

        SetPathPolicyTask task = SetPathPolicyTask.create(new TopologyGraph(
                provider), input);

        // operation type as remove
        try {

            input = new PathPolicyConfigBuilder.Rpc().set(pp).getBuilder()
                    .setOperation(VtnUpdateOperationType.REMOVE).build();

            task = SetPathPolicyTask.create(new TopologyGraph(provider), input);
        } catch (RpcException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
        }

        // null input
        try {
            task = SetPathPolicyTask.create(new TopologyGraph(provider), null);
        } catch (RpcException e) {
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
        }

    }

    /**
     * method to test createOutput of {@link SetPathPolicyTask}
     *
     * @throws Exception
     */
    @Test
    public void testCreateOutput() throws Exception {

        List<Integer> ids = new ArrayList<>();
        for (int i = 1; i <= 3; i++) {
            Integer id = Integer.valueOf(i);
            ids.add(id);
        }
        Long[] defCosts = {null, Long.valueOf(1L), Long.valueOf(777L),
                Long.valueOf(3333L), Long.valueOf(9999999L),
                Long.valueOf(Long.MAX_VALUE), };

        VTNManagerProvider provider = new VTNProviderStub();
        for (Integer id : ids) {
            for (Long defCost : defCosts) {
                long defc = (defCost == null) ? PathPolicy.COST_UNDEF : defCost
                        .longValue();
                try {
                    Long dc = Long.valueOf(defc);
                    PathPolicy pp = new PathPolicy(id, defc, null);
                    SetPathPolicyInput input = new PathPolicyConfigBuilder.Rpc()
                            .set(pp).getBuilder().setPresent(true).build();

                    SetPathPolicyTask task = SetPathPolicyTask.create(
                            new TopologyGraph(provider), input);

                    task.fixMissingParents();
                    Class<SetPathPolicyOutput> c = task.getOutputType();
                    assertEquals(c, SetPathPolicyOutput.class);
                    SetPathPolicyOutput output = task
                            .createOutput(VtnUpdateType.forValue(0));
                    assertEquals(output.getStatus(), VtnUpdateType.forValue(0));
                    testSuccess(task, provider);
                    VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(id)
                            .setDefaultCost(dc).build();
                    testStarted(task, provider, vpp);
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }
    }

    /**
     * method to test onSuccess of {@link SetPathPolicyTask}
     *
     * @param task
     * @param provider
     */
    public void testSuccess(SetPathPolicyTask task, VTNManagerProvider provider) {
        try {
            task.onSuccess(provider, VtnUpdateType.forValue(0));
            task.onSuccess(provider, VtnUpdateType.forValue(1));
            task.onSuccess(provider, null);

        } catch (Exception e) {
            unexpected(e);
        }
    }

    /**
     * method to test onStarted of {@link SetPathPolicyTask}
     *
     * @param task
     * @param provider
     * @param vpp
     */
    public void testStarted(SetPathPolicyTask task,
            VTNManagerProvider provider, VtnPathPolicy vpp) {
        try {
            TxContext ctx = provider == null ? null : provider.newTxContext();
            task.onStarted(ctx, vpp);
            task.onStarted(ctx, null);
        } catch (RpcException r) {
            Status st = r.getStatus();
            assertEquals(StatusCode.NOTFOUND, st.getCode());
        } catch (Exception e) {
            unexpected(e);
        }
    }

}
