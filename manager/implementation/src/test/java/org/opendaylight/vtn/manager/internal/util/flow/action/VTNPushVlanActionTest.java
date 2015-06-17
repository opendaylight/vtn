/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.push.vlan.action._case.VtnPushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.vtn.push.vlan.action._case.VtnPushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.push.vlan.action._case.PushVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

/**
 * JUnit test for {@link VTNPushVlanAction}.
 */
public class VTNPushVlanActionTest extends TestBase {
    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNPushVlanAction#VTNPushVlanAction()}</li>
     *   <li>{@link VTNPushVlanAction#VTNPushVlanAction(VlanType)}</li>
     *   <li>{@link VTNPushVlanAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNPushVlanAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNPushVlanAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNPushVlanAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNPushVlanAction#getDescription(Action)}</li>
     *   <li>{@link VTNFlowAction#toVtnFlowActionBuilder(Integer)}</li>
     *   <li>{@link VTNFlowAction#toActionBuilder(Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetSet() throws Exception {
        VlanType[] vtypes = {null, VlanType.VLAN};
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnPushVlanActionCaseBuilder vacBuilder =
            new VtnPushVlanActionCaseBuilder();
        for (VlanType vtype: vtypes) {
            VtnPushVlanAction vact = new VtnPushVlanActionBuilder().
                setVlanType(vtype).build();
            VtnPushVlanActionCase vac = vacBuilder.
                setVtnPushVlanAction(vact).build();
            Integer etype = (vtype == null) ? null : vtype.getIntValue();
            PushVlanAction ma = new PushVlanActionBuilder().
                setEthernetType(etype).build();
            PushVlanActionCase mact = new PushVlanActionCaseBuilder().
                setPushVlanAction(ma).build();
            VTNPushVlanAction va = new VTNPushVlanAction(vtype);
            assertEquals(vtype, va.getVlanType());

            for (Integer order: orders) {
                VtnFlowActionBuilder vbuilder =
                    va.toVtnFlowActionBuilder(order);
                assertEquals(order, vbuilder.getOrder());
                assertEquals(vac, vbuilder.getVtnAction());

                ActionBuilder mbuilder = va.toActionBuilder(order);
                assertEquals(order, mbuilder.getOrder());
                assertEquals(mact, mbuilder.getAction());
            }

            // toFlowAction() test.
            va = new VTNPushVlanAction();
            assertEquals(VlanType.VLAN, va.getVlanType());
            assertEquals(0x8100, va.getVlanType().getIntValue());

            VtnAction vaction = vac;
            if (vtype == null) {
                RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
                StatusCode ecode = StatusCode.BADREQUEST;
                String emsg = "VTNPushVlanAction: No VLAN type: " + vaction;
                try {
                    va.toFlowAction(vaction);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(ecode, st.getCode());
                    assertEquals(emsg, st.getDescription());
                }

                vaction = vacBuilder.
                    setVtnPushVlanAction(new VtnPushVlanActionBuilder().
                                         build()).
                    build();
                emsg = "VTNPushVlanAction: No VLAN type: " + vaction;
                try {
                    va.toFlowAction(vaction);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(ecode, st.getCode());
                    assertEquals(emsg, st.getDescription());
                }

                vaction = VTNPopVlanAction.newVtnAction();
                etag = RpcErrorTag.BAD_ELEMENT;
                ecode = StatusCode.BADREQUEST;
                emsg = "VTNPushVlanAction: Unexpected type: " + vaction;
                try {
                    va.toFlowAction(vaction);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(ecode, st.getCode());
                    assertEquals(emsg, st.getDescription());
                }
            } else {
                org.opendaylight.vtn.manager.flow.action.PushVlanAction vad =
                    new org.opendaylight.vtn.manager.flow.action.
                    PushVlanAction(vtype.getIntValue());
                assertEquals(vad, va.toFlowAction(vaction));
            }

            // toFlowAction() should never affect instance variables.
            assertEquals(VlanType.VLAN, va.getVlanType());
            assertEquals(0x8100, va.getVlanType().getIntValue());

            // toVtnAction() test.
            Action action = mact;
            if (etype == null) {
                RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
                StatusCode ecode = StatusCode.BADREQUEST;
                String emsg = "VTNPushVlanAction: No VLAN type: " + action;
                try {
                    va.toVtnAction(action);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(ecode, st.getCode());
                    assertEquals(emsg, st.getDescription());
                }

                action = new PushVlanActionCaseBuilder().build();
                emsg = "VTNPushVlanAction: No VLAN type: " + action;
                try {
                    va.toVtnAction(action);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(ecode, st.getCode());
                    assertEquals(emsg, st.getDescription());
                }

                Integer badType = 0x88a8;
                action = new PushVlanActionCaseBuilder().
                    setPushVlanAction(new PushVlanActionBuilder().
                                      setEthernetType(badType).build()).
                    build();
                emsg = "VTNPushVlanAction: Unsupported VLAN type: " + badType;
                etag = RpcErrorTag.BAD_ELEMENT;
                try {
                    va.toVtnAction(action);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    Status st = e.getStatus();
                    assertEquals(ecode, st.getCode());
                    assertEquals(emsg, st.getDescription());
                }

                action = new PopVlanActionCaseBuilder().build();
                emsg = "VTNPushVlanAction: Unexpected type: " + action;
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
            } else {
                assertEquals(vac, va.toVtnAction(action));
            }

            // toVtnAction() should never affect instance variables.
            assertEquals(VlanType.VLAN, va.getVlanType());
            assertEquals(0x8100, va.getVlanType().getIntValue());

            // getDescription() test.
            action = mact;
            String etherType = (etype == null)
                ? null
                : "0x" + Integer.toHexString(etype.intValue());
            String desc = "PUSH_VLAN(type=" + etherType + ")";
            assertEquals(desc, va.getDescription(action));

            // getDescription() should never affect instance variables.
            assertEquals(VlanType.VLAN, va.getVlanType());
            assertEquals(0x8100, va.getVlanType().getIntValue());
        }
    }

    /**
     * Test case for {@link VTNPushVlanAction#newVtnAction(VlanType)}.
     */
    @Test
    public void testNewVtnAction() {
        List<VlanType> vtypes = new ArrayList<>();
        vtypes.add(null);
        for (VlanType vtype: VlanType.values()) {
            vtypes.add(vtype);
        }

        for (VlanType vtype: vtypes) {
            VtnPushVlanActionCase ac = VTNPushVlanAction.newVtnAction(vtype);
            VtnPushVlanAction vaction = ac.getVtnPushVlanAction();
            assertEquals(vtype, vaction.getVlanType());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNPushVlanAction#equals(Object)}</li>
     *   <li>{@link VTNPushVlanAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        List<VlanType> vtypes = new ArrayList<>();
        vtypes.add(null);
        for (VlanType vtype: VlanType.values()) {
            vtypes.add(vtype);
        }

        for (VlanType vtype: vtypes) {
            VTNPushVlanAction va1 = new VTNPushVlanAction(vtype);
            VTNPushVlanAction va2 = new VTNPushVlanAction(vtype);
            testEquals(set, va1, va2);
        }

        assertEquals(vtypes.size(), set.size());
    }

    /**
     * Test case for {@link VTNPushVlanAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        VlanType[] vtypes = {null, VlanType.VLAN};
        for (VlanType vtype: vtypes) {
            VTNPushVlanAction va = new VTNPushVlanAction(vtype);
            String expected = "VTNPushVlanAction[type=" + vtype + "]";
            assertEquals(expected, va.toString());
        }
    }
}
