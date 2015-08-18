/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * JUnit test for {@link FlowCondReader}.
 */
public class FlowCondReaderTest extends TestBase {
    /**
     * Test case for {@link FlowCondReader#get(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGet() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        Map<String, VTNFlowCondition> expected = new HashMap<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Valid flow conditions.
        Map<FlowCondParams, FlowCondParams> cases =
            FlowCondParams.createFlowConditions();
        for (FlowCondParams params: cases.values()) {
            VTNFlowCondition vfcond = params.toVTNFlowCondition();
            VtnFlowCondition vfc = params.toVtnFlowConditionBuilder().build();
            InstanceIdentifier<VtnFlowCondition> path = vfcond.getPath();
            String name = params.getName();
            assertEquals(false, expected.containsKey(name));
            assertEquals(null, expected.put(name, vfcond));
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfc));
        }

        // Flow conditions that are not present.
        VtnFlowCondition vfcNull = null;
        for (int i = 0; i < 10; i++) {
            String name = "notfound" + i;
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(name);
            assertEquals(false, expected.containsKey(name));
            assertEquals(null, expected.put(name, null));
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vfcNull));
        }

        // Broken flow conditions.
        int badIndex = 5;
        List<VtnFlowMatch> vmatches = new ArrayList<>();
        for (int i = 1; i <= badIndex + 5; i++) {
            vmatches.add(new VtnFlowMatchBuilder().setIndex(i).build());
        }
        vmatches.add(new VtnFlowMatchBuilder().setIndex(badIndex).build());

        for (int i = 0; i < 5; i++) {
            String name = "broken" + i;
            VnodeName vname = new VnodeName(name);
            VtnFlowCondition vfc = new VtnFlowConditionBuilder().
                setName(vname).
                setVtnFlowMatch(new ArrayList<VtnFlowMatch>(vmatches)).build();
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(vname);
            assertEquals(false, expected.containsKey(name));
            assertEquals(null, expected.put(name, null));
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfc));
        }

        // Run tests.
        FlowCondReader reader = new FlowCondReader(rtx);
        assertSame(rtx, reader.getReadTransaction());

        for (int i = 0; i < 10; i++) {
            for (Map.Entry<String, VTNFlowCondition> entry:
                     expected.entrySet()) {
                String name = entry.getKey();
                VTNFlowCondition vfcond = entry.getValue();
                assertEquals(vfcond, reader.get(name));
            }
        }

        for (String name: expected.keySet()) {
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(name);
            Mockito.verify(rtx).read(oper, path);
        }
    }
}
