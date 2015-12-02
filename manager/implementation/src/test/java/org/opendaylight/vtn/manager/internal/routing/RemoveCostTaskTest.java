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
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MAX;
import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MIN;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TxContext;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link RemoveCostTask}.
 */
public class RemoveCostTaskTest extends TestBase {
    /**
     * Test case for
     * {@link RemoveCostTask#RemoveCostTask(Integer, VtnPortDesc)} and
     * {@link RemoveCostTask#getPortDesc()}.
     */
    @Test
    public void testConstructor() {
        VtnPortDesc[] vdescs = {
            new VtnPortDesc("openflow:1,3,"),
            new VtnPortDesc("openflow:2,,port-5"),
            new VtnPortDesc("openflow:345,12,port-12"),
        };

        for (VtnPortDesc vdesc: vdescs) {
            VtnPathCostKey ckey = new VtnPathCostKey(vdesc);
            for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
                VtnPathPolicyKey pkey = new VtnPathPolicyKey(id);
                InstanceIdentifier<VtnPathCost> path = InstanceIdentifier.
                    builder(VtnPathPolicies.class).
                    child(VtnPathPolicy.class, pkey).
                    child(VtnPathCost.class, ckey).
                    build();

                RemoveCostTask task = new RemoveCostTask(id, vdesc);
                assertEquals(path, task.getTargetPath());
                assertEquals(LogicalDatastoreType.OPERATIONAL,
                             task.getDatastoreType());
                assertEquals(vdesc, task.getPortDesc());
            }
        }
    }

    /**
     * Ensure that {@link SetCostTask} removes the path cost.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecute() throws Exception {
        Integer policyId = 2;
        VtnPortDesc vdesc = new VtnPortDesc("openflow:999,8,");
        InstanceIdentifier<VtnPathCost> path = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, new VtnPathPolicyKey(policyId)).
            child(VtnPathCost.class, new VtnPathCostKey(vdesc)).
            build();
        RemoveCostTask task = new RemoveCostTask(policyId, vdesc);

        // In case where the target path cost is present.
        VtnPathCost target = new VtnPathCostBuilder().
            setPortDesc(vdesc).
            setCost(111L).
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        when(tx.read(oper, path)).thenReturn(getReadResult(target));
        assertEquals(VtnUpdateType.REMOVED, task.execute(ctx, 0));
        verify(tx).read(oper, path);
        verify(tx).delete(oper, path);
        verifyNoMoreInteractions(tx);

        // In case where the target path cost is not present.
        reset(tx);
        target = null;
        when(tx.read(oper, path)).thenReturn(getReadResult(target));
        assertEquals(null, task.execute(ctx, 0));
        verify(tx).read(oper, path);
        verifyNoMoreInteractions(tx);
    }
}
