/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.math.BigInteger;
import java.util.List;

import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionConverter;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionUtils;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatch;
import org.opendaylight.vtn.manager.internal.util.OrderedComparator;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.VtnFlowEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.list.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * {@code FlowEntryDesc} class is used to a brief description about the
 * specified flow entry.
 *
 * <p>
 *   This class is used to embed information about the specified flow entry
 *   into a log record.
 * </p>
 */
public final class FlowEntryDesc {
    /**
     * The target flow entry.
     */
    private final VtnFlowEntry  flowEntry;

    /**
     * Determine whether flow instructions should be embedded into a log
     * record or not.
     */
    private final boolean  needInstruction;

    /**
     * Construct a new instance which describes a log record including
     * flow instructions.
     *
     * @param vfent  A {@link VtnFlowEntry} instance.
     */
    public FlowEntryDesc(VtnFlowEntry vfent) {
        this(vfent, true);
    }

    /**
     * Construct a new instance which describes a log record.
     *
     * @param vfent  A {@link VtnFlowEntry} instance.
     * @param inst   Flow instructions are embedded into a log record
     *               if {@code true}.
     */
    public FlowEntryDesc(VtnFlowEntry vfent, boolean inst) {
        flowEntry = vfent;
        needInstruction = inst;
    }

    // Object

    /**
     * Return a brief description about the flow entry.
     *
     * @return  A brief description about the flow entry.
     */
    @Override
    public String toString() {
        BigInteger cookie = flowEntry.getCookie().getValue();
        Match match = flowEntry.getMatch();
        String ingress;
        String cond;
        if (match == null) {
            ingress = null;
            cond = null;
        } else {
            NodeConnectorId ncId = match.getInPort();
            ingress = (ncId == null) ? null : ncId.getValue();
            try {
                VTNMatch vmatch = new VTNMatch(match);
                cond = vmatch.getConditionKey();
            } catch (Exception e) {
                // This should never happen.
                cond = match.toString();
            }
        }

        StringBuilder builder = new StringBuilder("flow=[id=").
            append(Long.toHexString(cookie.longValue())).append('-').
            append(flowEntry.getOrder()).
            append(", pri=").append(flowEntry.getPriority()).
            append(", timeout=(").append(flowEntry.getIdleTimeout()).
            append(',').append(flowEntry.getHardTimeout()).
            append("), node=").append(flowEntry.getNode().getValue()).
            append(", ingress=").append(ingress).
            append(", cond={").append(cond);

        if (needInstruction) {
            builder.append("}, actions={");
            FlowActionConverter converter = FlowActionConverter.getInstance();
            OrderedComparator comp = new OrderedComparator();
            List<Action> actions = FlowActionUtils.
                getActions(flowEntry.getInstructions(), comp);
            String sep = "";
            for (Action action: actions) {
                builder.append(sep).
                    append(converter.getDescription(action.getAction()));
                sep = ", ";
            }
        }

        return builder.append('}').toString();
    }
}
