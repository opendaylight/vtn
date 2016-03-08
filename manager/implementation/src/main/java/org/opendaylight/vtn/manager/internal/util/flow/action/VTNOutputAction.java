/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowActionBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCaseBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.output.action._case.OutputActionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.ActionBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Uri;

/**
 * {@code VTNOutputAction} describes the flow action that transmits packet to
 * the specified switch port.
 */
public final class VTNOutputAction extends VTNFlowAction {
    /**
     * The fixed value to be set in `max_len' field.
     */
    private static final int  OUTPUT_MAX_LEN = 0xffff;

    /**
     * The switch port to which packet is transmitted.
     */
    private SalPort  outputPort;

    /**
     * Construct an empty instance.
     */
    VTNOutputAction() {
    }

    /**
     * Create a new MD-SAL output action that transmits packets to the
     * specified port.
     *
     * @param port  The URI that specifies the output port.
     * @return  An {@link OutputActionCase} instance.
     */
    public static OutputActionCase newOutputActionCase(Uri port) {
        OutputAction out = new OutputActionBuilder().
            setOutputNodeConnector(port).
            setMaxLength(OUTPUT_MAX_LEN).
            build();
        return new OutputActionCaseBuilder().
            setOutputAction(out).
            build();
    }

    /**
     * Construct a new instance that transmits packet to the given switch port.
     *
     * @param sport  A {@link SalPort} instance.
     * @throws IllegalArgumentException  {@code sport} is {@code null}.
     */
    public VTNOutputAction(SalPort sport) {
        if (sport == null) {
            throw new IllegalArgumentException("Output port cannot be null.");
        }

        outputPort = sport;
    }

    /**
     * Return the output swich port.
     *
     * @return  A {@link SalPort} instance corresponding to the output switch
     *          port.
     */
    public SalPort getOutputPort() {
        return outputPort;
    }

    // VTNFlowAction

    /**
     * This method is not supported.
     *
     * @param act  An {@link Action} instance.
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public VtnAction toVtnAction(Action act) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription(Action act) throws RpcException {
        OutputActionCase ac = cast(OutputActionCase.class, act);
        OutputAction action = ac.getOutputAction();
        String port = null;
        Integer len = null;
        if (action != null) {
            len = action.getMaxLength();
            Uri uri = action.getOutputNodeConnector();
            if (uri != null) {
                port = uri.getValue();
            }
        }

        return new StringBuilder("OUTPUT(port=").
            append(port).append(", len=").append(len).append(')').toString();
    }

    /**
     * This method is not supported.
     *
     * @param builder  Unused.
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    protected VtnFlowActionBuilder set(VtnFlowActionBuilder builder) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected ActionBuilder set(ActionBuilder builder) {
        OutputActionCase ac =
            newOutputActionCase(outputPort.getNodeConnectorId());
        return builder.setAction(ac);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void appendContents(StringBuilder builder) {
        builder.append("port=").append(outputPort);
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        VTNOutputAction va = (VTNOutputAction)o;
        return outputPort.equals(va.outputPort);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() * HASH_PRIME + outputPort.hashCode();
    }
}
