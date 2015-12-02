/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.action;

import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.VtnOrderedFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.VtnAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpCodeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetIcmpTypeActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDscpActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetInetSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetPortSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.action.fields.vtn.action.VtnSetVlanPcpActionCase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.Action;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.DropActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.OutputActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PopVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.PushVlanActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetDlSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetNwTosActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpDstActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetTpSrcActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanIdActionCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.action.action.SetVlanPcpActionCase;

/**
 * {@code FlowActionConverter} is an utility for conversion of flow action
 * variants.
 */
public final class FlowActionConverter {
    /**
     * An innter class that holds the single global instance of
     * {@link FlowActionConverter}.
     */
    private static final class Holder {
        /**
         * Private constructor that protects this class from instantiating.
         */
        private Holder() {}

        /**
         * The single global instance of {@link FlowActionConverter}.
         */
        private static final FlowActionConverter  INSTANCE =
            new FlowActionConverter();
    }

    /**
     * Return the single global instance.
     *
     * @return  A {@link FlowActionConverter} instance.
     */
    public static FlowActionConverter getInstance() {
        return Holder.INSTANCE;
    }

    /**
     * A map that keeps adapters for conversion from {@link VtnAction} to
     * {@link FlowFilterAction}.
     */
    private final Map<Class<?>, FlowFilterAction>  filterConverters;

    /**
     * A map that keeps adapters for conversion from MD-SAL action to
     * {@link VtnAction}.
     */
    private final Map<Class<?>, VTNFlowAction>  mdConverters;

    /**
     * A map that keeps adapters for conversion from layer 4 MD-SAL action to
     * {@link VtnAction}.
     */
    private final Map<Short, Map<Class<?>, VTNFlowAction>>  l4Converters;

    /**
     * A map that keeps adapters for conversion from MD-SAL action to a string.
     *
     * <p>
     *   Note that this map keeps adapters that are not present in
     *   {@link #mdConverters}.
     * </p>
     */
    private final Map<Class<?>, VTNFlowAction>  mdStringifiers;

    /**
     * Construct a new instance.
     */
    private FlowActionConverter() {
        VTNDropAction drop = new VTNDropAction();
        VTNPopVlanAction popVlan = new VTNPopVlanAction();
        VTNPushVlanAction pushVlan = new VTNPushVlanAction();
        VTNSetDlDstAction dlDst = new VTNSetDlDstAction();
        VTNSetDlSrcAction dlSrc = new VTNSetDlSrcAction();
        VTNSetIcmpCodeAction icmpCode = new VTNSetIcmpCodeAction();
        VTNSetIcmpTypeAction icmpType = new VTNSetIcmpTypeAction();
        VTNSetInetDscpAction dscp = new VTNSetInetDscpAction();
        VTNSetInetDstAction inetDst = new VTNSetInetDstAction();
        VTNSetInetSrcAction inetSrc = new VTNSetInetSrcAction();
        VTNSetPortDstAction portDst = new VTNSetPortDstAction();
        VTNSetPortSrcAction portSrc = new VTNSetPortSrcAction();
        VTNSetVlanIdAction vlanId = new VTNSetVlanIdAction();
        VTNSetVlanPcpAction vlanPcp = new VTNSetVlanPcpAction();
        VTNOutputAction output = new VTNOutputAction();

        // Initialize adapters for conversion from VtnAction into
        // FlowFilterAction.
        filterConverters = ImmutableMap.<Class<?>, FlowFilterAction>builder().
            put(VtnSetDlDstActionCase.class, dlDst).
            put(VtnSetDlSrcActionCase.class, dlSrc).
            put(VtnSetIcmpCodeActionCase.class, icmpCode).
            put(VtnSetIcmpTypeActionCase.class, icmpType).
            put(VtnSetInetDscpActionCase.class, dscp).
            put(VtnSetInetDstActionCase.class, inetDst).
            put(VtnSetInetSrcActionCase.class, inetSrc).
            put(VtnSetPortDstActionCase.class, portDst).
            put(VtnSetPortSrcActionCase.class, portSrc).
            put(VtnSetVlanPcpActionCase.class, vlanPcp).
            build();

        // Initialize adapters for conversion from MD-SAL action into
        // VtnAction.
        mdConverters = ImmutableMap.<Class<?>, VTNFlowAction>builder().
            put(DropActionCase.class, drop).
            put(PopVlanActionCase.class, popVlan).
            put(PushVlanActionCase.class, pushVlan).
            put(SetDlDstActionCase.class, dlDst).
            put(SetDlSrcActionCase.class, dlSrc).
            put(SetNwTosActionCase.class, dscp).
            put(SetNwDstActionCase.class, inetDst).
            put(SetNwSrcActionCase.class, inetSrc).
            put(SetVlanIdActionCase.class, vlanId).
            put(SetVlanPcpActionCase.class, vlanPcp).
            build();

        // Initialize adapters for conversion from layer 4 MD-SAL action into
        // VtnAction.

        // For TCP and TCP.
        Map<Class<?>, VTNFlowAction> tcpMap = ImmutableMap.
            <Class<?>, VTNFlowAction>builder().
            put(SetTpDstActionCase.class, portDst).
            put(SetTpSrcActionCase.class, portSrc).
            build();

        // For ICMP.
        Map<Class<?>, VTNFlowAction> icmpMap = ImmutableMap.
            <Class<?>, VTNFlowAction>builder().
            put(SetTpDstActionCase.class, icmpCode).
            put(SetTpSrcActionCase.class, icmpType).
            build();

        l4Converters = ImmutableMap.
            <Short, Map<Class<?>, VTNFlowAction>>builder().
            put(InetProtocols.TCP.shortValue(), tcpMap).
            put(InetProtocols.UDP.shortValue(), tcpMap).
            put(InetProtocols.ICMP.shortValue(), icmpMap).
            build();

        // Initialize stringifiers for MD-SAL action.
        mdStringifiers = ImmutableMap.<Class<?>, VTNFlowAction>builder().
            put(OutputActionCase.class, output).
            put(SetTpDstActionCase.class, portDst).
            put(SetTpSrcActionCase.class, portSrc).
            build();
    }

    /**
     * Convert the given VTN action into a {@link FlowFilterAction} instance.
     *
     * @param vaction  A VTN flow action to be converted.
     * @return  A converted {@link FlowFilterAction} instance.
     * @throws RpcException
     *    Failed to convert the given instance.
     */
    public FlowFilterAction toFlowFilterAction(VtnOrderedFlowAction vaction)
        throws RpcException {
        if (vaction == null) {
            throw RpcException.getNullArgumentException("vtn-flow-action");
        }

        VtnAction vact = vaction.getVtnAction();
        if (vact == null) {
            throw RpcException.getNullArgumentException("vtn-action");
        }

        Class<?> type = vact.getImplementedInterface();
        FlowFilterAction conv = filterConverters.get(type);
        if (conv == null) {
            throw RpcException.getBadArgumentException(
                "Unsupported vtn-action: " + type.getName());
        }

        return conv.toFlowFilterAction(vact, vaction.getOrder());
    }

    /**
     * Convert the given MD-SAL action instance into a {@link VtnAction}
     * instance.
     *
     * @param act      A MD-SAL action to be converted.
     * @param ipproto  An IP protocol number configured in the flow match.
     * @return  A {@link VtnAction} instance.
     *          {@code null} if the given MD-SAL action is not bound to
     *          {@code VtnAction} class.
     * @throws RpcException
     *    Failed to convert the given instance.
     */
    public VtnAction toVtnAction(Action act, Short ipproto)
        throws RpcException {
        if (act != null) {
            Class<?> type = act.getImplementedInterface();
            VTNFlowAction conv = getMdConverter(type, ipproto);
            if (conv != null) {
                return conv.toVtnAction(act);
            }
        }

        return null;
    }

    /**
     * Return a brief description about the given MD-SAL action.
     *
     * @param act  A MD-SAL action.
     * @return  A string which describes the given MD-SAL action.
     */
    public String getDescription(Action act) {
        if (act != null) {
            Class<?> type = act.getImplementedInterface();
            VTNFlowAction conv = getMdStringifier(type);
            if (conv != null) {
                try {
                    return conv.getDescription(act);
                } catch (RpcException e) {
                    // Ignore any error.
                }
            }

            return act.toString();
        }

        return "Action(null)";
    }

    /**
     * Return adapter for conversion from MD-SAL action into {@link VtnAction}.
     *
     * @param type     A class which specifies the type of MD-SAL action.
     * @param ipproto  An IP protocol number configured in the flow match.
     * @return  A {@link VTNFlowAction} instance if found.
     *          {@code null} if not found.
     */
    private VTNFlowAction getMdConverter(Class<?> type, Short ipproto) {
        VTNFlowAction conv = mdConverters.get(type);
        if (conv == null) {
            // Try converters for L4 action.
            Map<Class<?>, VTNFlowAction> map = l4Converters.get(ipproto);
            if (map != null) {
                conv = map.get(type);
            }
        }

        return conv;
    }

    /**
     * Return adapter for conversion from MD-SAL action into a string.
     *
     * @param type     A class which specifies the type of MD-SAL action.
     * @return  A {@link VTNFlowAction} instance if found.
     *          {@code null} if not found.
     */
    private VTNFlowAction getMdStringifier(Class<?> type) {
        VTNFlowAction conv = mdConverters.get(type);
        if (conv == null) {
            conv = mdStringifiers.get(type);
        }

        return conv;
    }
}
