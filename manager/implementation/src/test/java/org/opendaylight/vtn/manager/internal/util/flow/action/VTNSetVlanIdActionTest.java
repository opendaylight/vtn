/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanIdActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.set.vlan.id.action._case.SetVlanIdActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link VTNSetVlanIdAction}.
 */
public class VTNSetVlanIdActionTest extends TestBase {
    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNSetVlanIdAction#VTNSetVlanIdAction(int)}</li>
     *   <li>{@link VTNSetVlanIdAction#getVlanId()}</li>
     *   <li>{@link VTNSetVlanIdAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNSetVlanIdAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNSetVlanIdAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNSetVlanIdAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNSetVlanIdAction#getDescription(Action)}</li>
     *   <li>{@link VTNFlowAction#toVtnFlowActionBuilder(Integer)}</li>
     *   <li>{@link VTNFlowAction#toActionBuilder(Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetSet() throws Exception {
        int[] vlanIds = {
            1, 2, 10, 44, 72, 128, 256, 3333, 4094, 4095,
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        for (int vid: vlanIds) {
            VtnSetVlanIdAction vact = new VtnSetVlanIdActionBuilder().
                setVlanId(vid).build();
            SetVlanIdAction ma = new SetVlanIdActionBuilder().
                setVlanId(new VlanId(vid)).build();
            SetVlanIdActionCase mact = new SetVlanIdActionCaseBuilder().
                setSetVlanIdAction(ma).build();
            VTNSetVlanIdAction va = new VTNSetVlanIdAction(vid);
            assertEquals(vid, va.getVlanId());

            for (Integer order: orders) {
                VtnFlowActionBuilder vbuilder =
                    va.toVtnFlowActionBuilder(order);
                assertEquals(order, vbuilder.getOrder());
                assertEquals(vact, vbuilder.getVtnAction());

                ActionBuilder mbuilder = va.toActionBuilder(order);
                assertEquals(order, mbuilder.getOrder());
                assertEquals(mact, mbuilder.getAction());
            }

            // toFlowAction() test.
            va = new VTNSetVlanIdAction();
            VtnAction vaction = vact;
            org.opendaylight.vtn.manager.flow.action.SetVlanIdAction vad =
                new org.opendaylight.vtn.manager.flow.action.
                SetVlanIdAction((short)vid);
            assertEquals(vad, va.toFlowAction(vaction));

            vaction = new VtnPushVlanActionBuilder().build();
            RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
            StatusCode ecode = StatusCode.BADREQUEST;
            String emsg = "VTNSetVlanIdAction: Unexpected type: " + vaction;
            try {
                va.toFlowAction(vaction);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            // toFlowAction() should never affect instance variables.
            assertEquals(0, va.getVlanId());

            // toVtnAction() test.
            Action action = mact;
            vaction = vact;
            assertEquals(vaction, va.toVtnAction(action));

            action = new PushVlanActionCaseBuilder().build();
            emsg = "VTNSetVlanIdAction: Unexpected type: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            try {
                va.getDescription(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            etag = RpcErrorTag.MISSING_ELEMENT;
            action = new SetVlanIdActionCaseBuilder().build();
            emsg = "VTNSetVlanIdAction: No VLAN ID: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            action = new SetVlanIdActionCaseBuilder().
                setSetVlanIdAction(new SetVlanIdActionBuilder().build()).
                build();
            emsg = "VTNSetVlanIdAction: No VLAN ID: " + action;
            try {
                va.toVtnAction(action);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(ecode, st.getCode());
                assertEquals(emsg, st.getDescription());
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(0, va.getVlanId());

            // getDescription() test.
            action = mact;
            String desc = "SET_VLAN_ID(vid=" + vid + ")";
            assertEquals(desc, va.getDescription(action));

            // getDescription() should never affect instance variables.
            assertEquals(0, va.getVlanId());
        }

        VTNSetVlanIdAction va = new VTNSetVlanIdAction();
        Action action = new SetVlanIdActionCaseBuilder().build();
        String desc = "SET_VLAN_ID(vid=null)";
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getVlanId());

        action = new SetVlanIdActionCaseBuilder().
            setSetVlanIdAction(new SetVlanIdActionBuilder().build()).
            build();
        assertEquals(desc, va.getDescription(action));
        assertEquals(0, va.getVlanId());
    }

    /**
     * Test case for {@link VTNSetVlanIdAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        int[] vlanIds = {
            1, 2, 10, 44, 72, 128, 256, 3333, 4094, 4095,
        };
        for (int vid: vlanIds) {
            VTNSetVlanIdAction va = new VTNSetVlanIdAction(vid);
            String expected = "VTNSetVlanIdAction[vlan=" + vid + "]";
            assertEquals(expected, va.toString());
        }
    }
}
