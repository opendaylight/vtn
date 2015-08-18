/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.DropFilter;

import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link DropFlowFilterImpl}.
 */
public class DropFlowFilterImplTest extends TestBase {
    /**
     * Test case for all methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void test() throws Exception {
        FlowFilterNode ffNode = Mockito.mock(FlowFilterNode.class);
        String container = "default";
        String tenant = "vtn_1";
        VTenantPath tpath = new VTenantPath(tenant);
        Mockito.when(ffNode.getContainerName()).thenReturn(container);
        Mockito.when(ffNode.getPath()).thenReturn(tpath);

        int index = 1;
        String cond = "cond_name";
        DropFilter type = new DropFilter();
        FlowFilter filter = new FlowFilter(index, cond, type, null);
        FlowFilterMap ffMap = FlowFilterMap.createIncoming(ffNode);
        FlowFilterImpl impl = FlowFilterImpl.create(ffNode, index, filter);
        assertTrue(impl instanceof DropFlowFilterImpl);
        assertEquals(true, impl.isMulticastSupported());
        assertEquals(false, impl.needFlowAction());
        assertEquals(type, impl.getFilterType());
        assertNotNull(impl.getLogger());
        assertEquals(cond, impl.getFlowConditionName());
        assertEquals(index, impl.getIndex());

        PacketContext pctx = Mockito.mock(PacketContext.class);
        Mockito.when(pctx.isFlooding()).thenReturn(true);
        try {
            impl.apply(null, pctx, ffMap);
            unexpected();
        } catch (DropFlowException e) {
        }

        Mockito.verify(pctx).isFlooding();
        Mockito.verify(pctx).installDropFlow();
        Mockito.verify(pctx, Mockito.never()).getDescription();
        Mockito.reset(pctx);

        String desc = "test packet";
        Mockito.when(pctx.isFlooding()).thenReturn(false);
        Mockito.when(pctx.getDescription()).thenReturn(desc);
        try {
            impl.apply(null, pctx, ffMap);
            unexpected();
        } catch (DropFlowException e) {
        }

        Mockito.verify(pctx).isFlooding();
        Mockito.verify(pctx).installDropFlow();
        Mockito.verify(pctx).getDescription();
    }
}
