/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link VTNOutputAction}.
 */
public class VTNOutputActionTest extends TestBase {
    /**
     * Test case for constructors and instance methods.
     *
     * <ul>
     *   <li>{@link VTNOutputAction#VTNOutputAction(SalPort)}</li>
     *   <li>{@link VTNOutputAction#getOutputPort()}</li>
     *   <li>{@link VTNOutputAction#set(VtnFlowActionBuilder)}</li>
     *   <li>{@link VTNOutputAction#set(ActionBuilder)}</li>
     *   <li>{@link VTNOutputAction#toFlowAction(VtnAction)}</li>
     *   <li>{@link VTNOutputAction#toVtnAction(Action)}</li>
     *   <li>{@link VTNOutputAction#getDescription(Action)}</li>
     *   <li>{@link VTNFlowAction#toVtnFlowActionBuilder(Integer)}</li>
     *   <li>{@link VTNFlowAction#toActionBuilder(Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetSet() throws Exception {
        String[] ports = {
            "openflow:1:1",
            "openflow:1:2",
            "openflow:18446744073709551615:0",
            "openflow:18446744073709551615:1",
            "openflow:18446744073709551615:4294967040",
        };
        Integer[] orders = {
            Integer.MIN_VALUE, -1000, -1,
            0, 1, 2, 32000, Integer.MAX_VALUE,
        };

        VTNOutputAction vout = new VTNOutputAction();
        for (String portId: ports) {
            SalPort sport = SalPort.create(portId);
            assertNotNull(sport);

            int len = 0xffff;
            OutputAction ma = new OutputActionBuilder().
                setOutputNodeConnector(new NodeConnectorId(portId)).
                setMaxLength(len).build();
            OutputActionCase mact = new OutputActionCaseBuilder().
                setOutputAction(ma).build();
            VTNOutputAction va = new VTNOutputAction(sport);
            assertEquals(sport, va.getOutputPort());

            String desc = "OUTPUT(port=" + sport + ", len=" + len + ")";
            assertEquals(desc, vout.getDescription((Action)mact));

            // getDescription() should not affect instance variables.
            assertEquals(null, vout.getOutputPort());

            for (Integer order: orders) {
                try {
                    va.toVtnFlowActionBuilder(order);
                    unexpected();
                } catch (IllegalStateException e) {
                }

                ActionBuilder mbuilder = va.toActionBuilder(order);
                assertEquals(order, mbuilder.getOrder());
                assertEquals(mact, mbuilder.getAction());
            }

            try {
                va.toFlowAction((VtnAction)null);
                unexpected();
            } catch (IllegalStateException e) {
            }

            try {
                va.toVtnAction((Action)null);
                unexpected();
            } catch (IllegalStateException e) {
            }
        }

        try {
            new VTNOutputAction((SalPort)null);
            unexpected();
        } catch (IllegalArgumentException e) {
        }

        Action action = new PopVlanActionCaseBuilder().build();
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        StatusCode ecode = StatusCode.BADREQUEST;
        String emsg = "VTNOutputAction: Unexpected type: " + action;
        try {
            vout.getDescription(action);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(ecode, st.getCode());
            assertEquals(emsg, st.getDescription());
        }
        assertEquals(null, vout.getOutputPort());

        int len = 12345;
        String port = "openflow:333:444";
        OutputAction ma = new OutputActionBuilder().
            setOutputNodeConnector(new NodeConnectorId(port)).
            setMaxLength(len).build();
        action = new OutputActionCaseBuilder().
            setOutputAction(ma).build();
        String desc = "OUTPUT(port=" + port + ", len=" + len + ")";
        assertEquals(desc, vout.getDescription(action));
        assertEquals(null, vout.getOutputPort());

        action = new OutputActionCaseBuilder().build();
        desc = "OUTPUT(port=null, len=null)";
        assertEquals(desc, vout.getDescription(action));
        assertEquals(null, vout.getOutputPort());

        action = new OutputActionCaseBuilder().
            setOutputAction(new OutputActionBuilder().build()).build();
        assertEquals(desc, vout.getDescription(action));
        assertEquals(null, vout.getOutputPort());
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link VTNOutputAction#equals(Object)}</li>
     *   <li>{@link VTNOutputAction#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();
        String[] ports = {
            "openflow:1:1",
            "openflow:1:2",
            "openflow:2:1",
            "openflow:2:2",
            "openflow:18446744073709551615:0",
            "openflow:18446744073709551615:1",
            "openflow:18446744073709551615:4294967040",
        };

        for (String portId: ports) {
            VTNOutputAction va1 = new VTNOutputAction(SalPort.create(portId));
            VTNOutputAction va2 = new VTNOutputAction(SalPort.create(portId));
            testEquals(set, va1, va2);
        }

        assertEquals(ports.length, set.size());
    }

    /**
     * Test case for {@link VTNOutputAction#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        String[] ports = {
            "openflow:1:1",
            "openflow:1:2",
            "openflow:18446744073709551615:0",
            "openflow:18446744073709551615:1",
            "openflow:18446744073709551615:4294967040",
        };
        for (String portId: ports) {
            SalPort sport = SalPort.create(portId);
            assertNotNull(sport);
            VTNOutputAction va = new VTNOutputAction(sport);
            String expected = "VTNOutputAction[port=" + portId + "]";
            assertEquals(expected, va.toString());
        }
    }
}
