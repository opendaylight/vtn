/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import org.opendaylight.vtn.manager.flow.action.FlowAction;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPopVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnPopVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.pop.vlan.action._case.PopVlanAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.pop.vlan.action._case.PopVlanActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

/**
 * {@code VTNPopVlanAction} describes the flow action that strips the outermost
 * VLAN tag.
 */
public final class VTNPopVlanAction extends VTNFlowAction {
    /**
     * Construct a new instance without specifying action order.
     */
    public VTNPopVlanAction() {
    }

    // VTNFlowAction

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowAction toFlowAction(VtnAction vact) throws RpcException {
        cast(VtnPopVlanAction.class, vact);
        return new org.opendaylight.vtn.manager.flow.action.PopVlanAction();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnPopVlanAction toVtnAction(Action act) throws RpcException {
        cast(PopVlanActionCase.class, act);
        return new VtnPopVlanActionBuilder().build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        cast(PopVlanActionCase.class, act);
        return "POP_VLAN";
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        return builder.setVtnAction(new VtnPopVlanActionBuilder().build());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        PopVlanAction pop = new PopVlanActionBuilder().build();
        return builder.setAction(new PopVlanActionCaseBuilder().
                                 setPopVlanAction(pop).build());
    }
}
