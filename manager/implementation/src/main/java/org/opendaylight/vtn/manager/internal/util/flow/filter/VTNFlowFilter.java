/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.TreeMap;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.slf4j.Logger;

import com.google.common.collect.ImmutableSet;

import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.filter.FilterType;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;
import org.opendaylight.vtn.manager.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.flow.filter.PassFilter;
import org.opendaylight.vtn.manager.util.VTNIdentifiable;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowActionConverter;
import org.opendaylight.vtn.manager.internal.util.flow.action.FlowFilterAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetDlSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpCodeAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetIcmpTypeAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDscpAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetInetSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortDstAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetPortSrcAction;
import org.opendaylight.vtn.manager.internal.util.flow.action.VTNSetVlanPcpAction;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondUtils;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.packet.UnsupportedPacketException;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.action.rev150410.vtn.flow.action.list.VtnFlowAction;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.VtnFlowFilterType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnDropFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnPassFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.type.fields.vtn.flow.filter.type.VtnRedirectFilterCase;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;

/**
 * {@code VTNFlowFilter} describes configuration information about flow filter.
 */
@XmlRootElement(name = "vtn-flow-filter")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VTNPassFilter.class, VTNDropFilter.class, VTNRedirectFilter.class})
public abstract class VTNFlowFilter implements VTNIdentifiable<Integer> {
    /**
     * A set of output flow filter types.
     */
    private static final Set<Class<?>>  OUTPUT_FILTERS;

    /**
     * An integer number assigned to this flow filter.
     */
    @XmlElement(required = true)
    private Integer  index;

    /**
     * The name of the flow condition which selects packets.
     */
    @XmlElement(required = true)
    private VnodeName  condition;

    /**
     * A list of flow actions which modify the packet when this flow filter
     * is applied to the packet.
     */
    @XmlElementWrapper
    @XmlElements({
        @XmlElement(name = "vtn-set-dl-src", type = VTNSetDlSrcAction.class),
        @XmlElement(name = "vtn-set-dl-dst", type = VTNSetDlDstAction.class),
        @XmlElement(name = "vtn-set-inet-src",
                    type = VTNSetInetSrcAction.class),
        @XmlElement(name = "vtn-set-inet-dst",
                    type = VTNSetInetDstAction.class),
        @XmlElement(name = "vtn-set-inet-dscp",
                    type = VTNSetInetDscpAction.class),
        @XmlElement(name = "vtn-set-port-src",
                    type = VTNSetPortSrcAction.class),
        @XmlElement(name = "vtn-set-port-dst",
                    type = VTNSetPortDstAction.class),
        @XmlElement(name = "vtn-set-icmp-type",
                    type = VTNSetIcmpTypeAction.class),
        @XmlElement(name = "vtn-set-icmp-code",
                    type = VTNSetIcmpCodeAction.class),
        @XmlElement(name = "vtn-set-vlan-pcp",
                    type = VTNSetVlanPcpAction.class)})
    private List<FlowFilterAction>  actions;

    /**
     * Initialize static fields.
     */
    static {
        OUTPUT_FILTERS = ImmutableSet.<Class<?>>builder().
            add(VbridgeOutputFilter.class).
            add(VinterfaceOutputFilter.class).
            build();
    }

    /**
     * Determine whether the flow filter list specified by the given instance
     * identifier is output filter or not.
     *
     * @param path  Path to the flow filter list.
     * @return  {@code true} only if the specified flow filter list is output
     *          filter.
     */
    public static final boolean isOutput(
        InstanceIdentifier<VtnFlowFilter> path) {
        for (PathArgument arg: path.getPathArguments()) {
            if (OUTPUT_FILTERS.contains(arg.getType())) {
                return true;
            }
        }

        return false;
    }

    /**
     * Return a brief description about the given flow filter type.
     *
     * @param vffc  A {@link VtnFlowFilterConfig} instance which contains
     *              flow filter configuration.
     * @return  A brief description about the given flow filter type.
     */
    public static final String getTypeDescription(VtnFlowFilterConfig vffc) {
        VtnFlowFilterType vftype = vffc.getVtnFlowFilterType();
        String desc;
        if (vftype instanceof VtnPassFilterCase) {
            desc = "PASS";
        } else if (vftype instanceof VtnDropFilterCase) {
            desc = "DROP";
        } else if (vftype instanceof VtnRedirectFilterCase) {
            desc = "REDIRECT";
        } else {
            // This should never happen.
            desc = "UNKNOWN";
        }

        return desc;
    }

    /**
     * Create a new flow filter.
     *
     * @param vffc  A {@link VtnFlowFilterConfig} instance which contains
     *              flow filter configuration.
     * @return  A {@link VTNFlowFilter} instance.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    public static final VTNFlowFilter create(VtnFlowFilterConfig vffc)
        throws RpcException {
        if (vffc == null) {
            throw RpcException.getNullArgumentException("vtn-flow-filter");
        }

        VTNFlowFilter vff;
        VtnFlowFilterType vftype = vffc.getVtnFlowFilterType();
        if (vftype instanceof VtnPassFilterCase) {
            vff = new VTNPassFilter(vffc, (VtnPassFilterCase)vftype);
        } else if (vftype instanceof VtnDropFilterCase) {
            vff = new VTNDropFilter(vffc, (VtnDropFilterCase)vftype);
        } else if (vftype instanceof VtnRedirectFilterCase) {
            vff = new VTNRedirectFilter(vffc, (VtnRedirectFilterCase)vftype);
        } else if (vftype == null) {
            throw RpcException.getNullArgumentException(
                "vtn-flow-filter-type");
        } else {
            // This should never happen.
            throw getUnknownTypeException(vftype);
        }

        return vff;
    }

    /**
     * Create a new flow filter.
     *
     * @param idx     An index number to be assigned.
     * @param filter  A {@link FlowFilter} instance which contains flow filter
     *                configuration.
     * @return  A {@link VTNFlowFilter} instance.
     * @throws RpcException
     *    {@code filter} contains invalid value.
     */
    public static final VTNFlowFilter create(int idx, FlowFilter filter)
        throws RpcException {
        if (filter == null) {
            throw RpcException.getNullArgumentException("Flow filter");
        }

        VTNFlowFilter vff;
        FilterType type = filter.getFilterType();
        if (type instanceof PassFilter) {
            vff = new VTNPassFilter(idx, filter);
        } else if (type instanceof DropFilter) {
            vff = new VTNDropFilter(idx, filter);
        } else if (type instanceof RedirectFilter) {
            vff = new VTNRedirectFilter(idx, filter, (RedirectFilter)type);
        } else if (type == null) {
            throw RpcException.getNullArgumentException("Flow filter type");
        } else {
            // This should never happen.
            throw getUnknownTypeException(type);
        }

        return vff;
    }

    /**
     * Ensure that the given flow filter index is not null.
     *
     * @param index  A flow filter index to be tested.
     * @return  {@code index}.
     * @throws RpcException  The given index is {@code null}.
     */
    public static Integer checkIndexNotNull(Integer index) throws RpcException {
        if (index == null) {
            throw RpcException.getNullArgumentException("Filter index");
        }
        return index;
    }

    /**
     * Create an exception that indicates unknown flow filter type is
     * detected.
     *
     * @param type  An object that indicates the flow filter type.
     * @return  An {@link RpcException} instance.
     */
    private static RpcException getUnknownTypeException(Object type) {
        return RpcException.getBadArgumentException(
            "Unexpected flow filter type: " + type);
    }

    /**
     * Constructor only for JAXB.
     */
    VTNFlowFilter() {
    }

    /**
     * Construct a new instance.
     *
     * @param vffc  A {@link VtnFlowFilterConfig} instance which contains
     *              flow filter configuration.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNFlowFilter(VtnFlowFilterConfig vffc) throws RpcException {
        index = verifyIndex(vffc.getIndex());
        condition = vffc.getCondition();
        FlowCondUtils.checkName(condition);

        FlowActionConverter conv = FlowActionConverter.getInstance();
        List<VtnFlowAction> vactions = vffc.getVtnFlowAction();
        Map<Integer, FlowFilterAction> map;
        if (vactions == null || vactions.isEmpty()) {
            map = null;
        } else {
            map = new TreeMap<>();
            for (VtnFlowAction vact: vactions) {
                FlowFilterAction ffact = conv.toFlowFilterAction(vact);
                putFlowAction(map, ffact);
            }
        }
        actions = MiscUtils.toValueList(map);
    }

    /**
     * Construct a new instance.
     *
     * @param idx     An index number to be assigned to the flow filter.
     * @param filter  A {@link FlowFilter} instance which contains flow filter
     *                configuration.
     * @throws RpcException
     *    {@code vffc} contains invalid value.
     */
    VTNFlowFilter(int idx, FlowFilter filter) throws RpcException {
        index = verifyIndex(idx);
        condition = FlowCondUtils.checkName(filter.getFlowConditionName());

        FlowActionConverter conv = FlowActionConverter.getInstance();
        List<FlowAction> factions = filter.getActions();
        Map<Integer, FlowFilterAction> map;
        if (factions == null || factions.isEmpty()) {
            map = null;
        } else {
            map = new TreeMap<>();
            int order = MiscUtils.ORDER_MIN;
            for (FlowAction act: factions) {
                FlowFilterAction ffact = conv.toFlowFilterAction(order, act);
                putFlowAction(map, ffact);
                order++;
            }
        }
        actions = MiscUtils.toValueList(map);
    }

    /**
     * Return the name of the flow condition that selects packets to be
     * filtered.
     *
     * @return  The name of the flow condition.
     */
    public final String getCondition() {
        return condition.getValue();
    }

    /**
     * Convert this instance into a {@link VtnFlowFilter} instance.
     *
     * @return  A {@link VtnFlowFilter} instance.
     */
    public final VtnFlowFilter toVtnFlowFilter() {
        List<VtnFlowAction> vfactions;
        if (actions == null || actions.isEmpty()) {
            vfactions = null;
        } else {
            vfactions = new ArrayList<>(actions.size());
            for (FlowFilterAction ffact: actions) {
                vfactions.add(ffact.toVtnFlowAction());
            }
        }

        return new VtnFlowFilterBuilder().
            setIndex(index).
            setCondition(condition).
            setVtnFlowAction(vfactions).
            setVtnFlowFilterType(getVtnFlowFilterType()).
            build();
    }

    /**
     * Convert this instance into a {@link FlowFilter} instance.
     *
     * @return  A {@link FlowFilter} instance.
     */
    public final FlowFilter toFlowFilter() {
        List<FlowAction> vfactions;
        if (actions == null || actions.isEmpty()) {
            vfactions = null;
        } else {
            vfactions = new ArrayList<>(actions.size());
            for (FlowFilterAction ffact: actions) {
                vfactions.add(ffact.toFlowAction());
            }
        }

        return new FlowFilter(index.intValue(), condition.getValue(),
                              getFilterType(), vfactions);
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    public final void verify() throws RpcException {
        verifyIndex(index);
        FlowCondUtils.checkPresent(condition);

        // Need to sort flow actions by action order.
        Map<Integer, FlowFilterAction> map;
        if (actions == null || actions.isEmpty()) {
            map = null;
        } else {
            map = new TreeMap<>();
            for (FlowFilterAction ffact: actions) {
                ffact.verify();
                putFlowAction(map, ffact);
            }
        }
        actions = MiscUtils.toValueList(map);
        verifyImpl();
    }

    /**
     * Return a list of flow actions which modifies the packet when this
     * flow filter is applied to the packet.
     *
     * @return  A list of flow actions.
     *          Flow actions in the returned collection are always sorted in
     *          ascending order of action order values.
     */
    public final List<FlowFilterAction> getActions() {
        return (actions == null)
            ? Collections.<FlowFilterAction>emptyList()
            : Collections.unmodifiableList(actions);
    }

    /**
     * Return a string that indicates this flow filter.
     *
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     * @return  A string that indicates this flow filter.
     */
    public final String getPathString(FlowFilterListId flid) {
        return flid.getFilterId(index.intValue());
    }

    /**
     * Evaluate this flow filter against the given packet.
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     * @return  {@code true} is returned if this flow filter was applied to
     *          the given packet. Otherwise {@code false} is returned.
     * @throws DropFlowException
     *    The given packet was discarded by this flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by this flow filter.
     */
    public final boolean evaluate(FlowFilterContext fctx,
                                  FlowFilterListId flid)
        throws DropFlowException, RedirectFlowException {
        boolean ret = false;
        Logger logger = getLogger();
        try {
            VTNFlowCondition vfcond = fctx.getFlowCondition(this);
            if (vfcond.match(fctx)) {
                // Apply this flow filter.
                if (needFlowAction()) {
                    applyFlowActions(fctx, flid);
                }
                apply(fctx, flid);
                ret = true;
                if (logger.isTraceEnabled()) {
                    logger.trace("{}: Packet matched the condition: {}",
                                 getPathString(flid), condition.getValue());
                }
            } else if (logger.isTraceEnabled()) {
                logger.trace("{}: Packet did not match the condition: {}",
                             getPathString(flid), condition.getValue());
            }
        } catch (UnsupportedPacketException e) {
            if (logger.isDebugEnabled()) {
                String msg = e.getMessage();
                logger.debug("{}: Ignore flow filter: {}: {}",
                             getPathString(flid), msg, condition.getValue());
            }
        }

        return ret;
    }

    /**
     * Determine whether this flow filter can handle multicast packets or not.
     *
     * <p>
     *   This method always returns {@code false} which indicates that this
     *   flow filter should ignore multicast packets.
     *   Subclass may override this method to support multicast packets.
     * </p>
     *
     * @return  {@code false}.
     */
    public boolean isMulticastSupported() {
        return false;
    }

    /**
     * Determine whether this flow filter needs to apply flow actions to the
     * packet.
     *
     * <p>
     *   This method returns {@code true} which indicates that this flow filter
     *   needs to apply flow actions configured in this instance.
     *   Subclass may override this method to ignore flow actions.
     * </p>
     *
     * @return  {@code true}.
     */
    public boolean needFlowAction() {
        return true;
    }

    /**
     * Determine whether this flow filter supports packet flooding or not.
     *
     * <p>
     *   This method returns {@code true} which indicates that this flow filter
     *   needs to be evaluated even when the packet is going to be broadcasted
     *   in the vBridge.
     *   Subclass may override this method to ignore flow actions.
     * </p>
     *
     * @return  {@code true}.
     */
    public boolean isFloodingSuppoted() {
        return true;
    }

    /**
     * Verify the given filter index.
     *
     * @param index  A filter index to be tested.
     * @return  The given {@code index} is returned.
     * @throws RpcException  Verification failed.
     */
    private Integer verifyIndex(Integer index) throws RpcException {
        checkIndexNotNull(index);

        try {
            new VtnFlowFilterBuilder().setIndex(index);
        } catch (RuntimeException e) {
            String msg = "Invalid filter index: " + index;
            RpcException re = RpcException.getBadArgumentException(msg);
            re.initCause(e);
            throw re;
        }

        return index;
    }

    /**
     * Put the given flow action into the given map.
     *
     * @param map    A map which keeps pairs of action orders and flow actions.
     * @param ffact  A {@link FlowFilterAction} instance.
     * @throws RpcException  An error occurred.
     */
    private void putFlowAction(Map<Integer, FlowFilterAction> map,
                               FlowFilterAction ffact) throws RpcException {
        Integer order = ffact.getOrder();
        if (map.put(order, ffact) != null) {
            throw RpcException.getBadArgumentException(
                "Duplicate action order: " + order);
        }
    }

    /**
     * Apply flow actions to the given packet.
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     */
    private void applyFlowActions(FlowFilterContext fctx,
                                  FlowFilterListId flid) {
        if (actions != null) {
            Logger logger = getLogger();
            boolean doTrace = logger.isTraceEnabled();
            for (FlowFilterAction fact: actions) {
                boolean result = fact.apply(fctx);
                if (doTrace) {
                    logger.trace("{}: Flow action was {}: {}",
                                 getPathString(flid),
                                 (result) ? "applied" : "ignored", fact);
                }
            }
        }
    }

    /**
     * Determine whether this flow filter can be configured into the specified
     * virtual node.
     *
     * @param ident  A {@link VNodeIdentifier} instance which specifies the
     *               target virtual node.
     * @throws RpcException
     *    This flow filter cannot be configured into the virtual node specified
     *    by {@code ident}.
     */
    public abstract void canSet(VNodeIdentifier<?> ident) throws RpcException;

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    protected abstract void verifyImpl() throws RpcException;

    /**
     * Return a {@link FilterType} instance which indicates the type of
     * this flow filter.
     *
     * @return  A {@link FilterType} instance.
     */
    protected abstract FilterType getFilterType();

    /**
     * Return a {@link VtnFlowFilterType} instance which indicates the type of
     * this flow filter.
     *
     * @return  A {@link VtnFlowFilterType} instance.
     */
    protected abstract VtnFlowFilterType getVtnFlowFilterType();

    /**
     * Return a logger instance.
     *
     * @return  A {@link Logger} instance.
     */
    protected abstract Logger getLogger();

    /**
     * Apply this flow filter to the given packet.
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param flid   A {@link FlowFilterListId} instance that specifies the
     *               location of this flow filter list.
     * @throws DropFlowException
     *    A packet was discarded by this flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by this flow filter.
     */
    protected abstract void apply(FlowFilterContext fctx,
                                  FlowFilterListId flid)
        throws DropFlowException, RedirectFlowException;

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * @return  The index number assigned to this flow filter.
     */
    @Override
    public final Integer getIdentifier() {
        return index;
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
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            VTNFlowFilter vff = (VTNFlowFilter)o;
            ret = (Objects.equals(index, vff.index) &&
                   Objects.equals(condition, vff.condition) &&
                   Objects.equals(actions, vff.actions));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), index, condition, actions);
    }
}
