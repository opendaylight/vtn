/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnDropAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnDropActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPopVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.DropActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.DropActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.drop.action._case.DropAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.drop.action._case.DropActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

/**
 * JUnit test for {@link VTNDropAction}.
 */
public class VTNDropActionTest extends TestBase {
    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNDropAction#VTNDropAction()}</li>
     *   <li>{@link VTNDropAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNDropAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNDropAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNDropAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNDropAction#getDescription(Action)}</li>
     *   <li>{@link VTNFlowAction#toVtnFlowActionBuilder(Integer)}</li>
     *   <li>{@link VTNFlowAction#toActionBuilder(Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetSet() throws Exception {
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VtnDropAction vact = new VtnDropActionBuilder().build();
        DropAction ma = new DropActionBuilder().build();
        DropActionCase mact = new DropActionCaseBuilder().
            setDropAction(ma).build();
        VTNDropAction va = new VTNDropAction();

        for (Integer order: orders) {
            VtnFlowActionBuilder vbuilder = va.toVtnFlowActionBuilder(order);
            assertEquals(order, vbuilder.getOrder());
            assertEquals(vact, vbuilder.getVtnAction());

            ActionBuilder mbuilder = va.toActionBuilder(order);
            assertEquals(order, mbuilder.getOrder());
            assertEquals(mact, mbuilder.getAction());
        }

        org.opendaylight.vtn.manager.flow.action.DropAction vad =
            new org.opendaylight.vtn.manager.flow.action.DropAction();
        VtnAction vaction = vact;
        assertEquals(vad, va.toFlowAction(vaction));

        Action action = mact;
        assertEquals(vact, va.toVtnAction(action));
        assertEquals("DROP", va.getDescription(action));

        vaction = new VtnPopVlanActionBuilder().build();
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNDropAction: Unexpected type: " + vaction;
        try {
            va.toFlowAction(vaction);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }

        action = new PopVlanActionCaseBuilder().build();
        emsg = "VTNDropAction: Unexpected type: " + action;
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
    }

    /**
     * Ensure that {@link VTNDropAction} can be distinguished from
     * {@link VTNPopVlanAction}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();

        testEquals(set, new VTNDropAction(), new VTNDropAction());
        testEquals(set, new VTNPopVlanAction(), new VTNPopVlanAction());
        assertEquals(2, set.size());
    }

    /**
     * Test case for {@link VTNDropAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        VTNDropAction va = new VTNDropAction();
        assertEquals("VTNDropAction[]", va.toString());
    }
}
