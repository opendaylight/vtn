/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.filter;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.TreeMap;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;

/**
 * {@code FlowFilterList} describes a list of flow filter configurations.
 */
@XmlRootElement(name = "vtn-flow-filter-list")
@XmlAccessorType(XmlAccessType.NONE)
public final class FlowFilterList {
    /**
     * A pseudo VLAN ID which indicates the VLAN ID is not specified.
     */
    public static final int  VLAN_UNSPEC = -1;

    /**
     * A list of flow filters.
     */
    @XmlElements({
        @XmlElement(name = "pass-filter", type = VTNPassFilter.class),
        @XmlElement(name = "drop-filter", type = VTNDropFilter.class),
        @XmlElement(name = "redirect-filter", type = VTNRedirectFilter.class)})
    private List<VTNFlowFilter>  flowFilters;

    /**
     * The identifier of this flow filter list.
     */
    private FlowFilterListId  listId;

    /**
     * Create a new instance from the given {@link VtnFlowFilterList} instance.
     *
     * @param vflist    A {@link VtnFlowFilterList} instance.
     * @param nillable  Determine whether {@code null} should be returned
     *                  if the given flow filter list is empty.
     * @return  A {@link FlowFilterList} if {@code vflist} contains at least
     *          one flow filter configuration.
     *          {@code null} if the given flow filter list is empty and
     *          {@code nillable} is {@code true}.
     * @throws RpcException
     *    The given flow filter list is invalid.
     */
    public static FlowFilterList create(VtnFlowFilterList vflist,
                                        boolean nillable)
        throws RpcException {
        FlowFilterList filters;
        if (vflist == null) {
            filters = (nillable) ? null : new FlowFilterList();
        } else {
            filters = new FlowFilterList(vflist);
            if (nillable && filters.isEmpty()) {
                filters = null;
            }
        }

        return filters;
    }

    /**
     * Determine whether the given flow filter list is empty or not.
     *
     * @param vflist  A {@link VtnFlowFilterList} instance.
     * @return  {@code true} only if the given flow filter list is empty.
     */
    public static boolean isEmpty(VtnFlowFilterList vflist) {
        return (vflist == null ||
                MiscUtils.isEmpty(vflist.getVtnFlowFilter()));
    }

    /**
     * Construct an empty flow filter list.
     */
    public FlowFilterList() {
    }

    /**
     * Construct a new instance.
     *
     * @param vflist  A {@link VtnFlowFilterList} instance.
     * @throws RpcException
     *    The given flow filter list is invalid.
     */
    private FlowFilterList(VtnFlowFilterList vflist) throws RpcException {
        List<VtnFlowFilter> list = vflist.getVtnFlowFilter();
        if (list != null && !list.isEmpty()) {
            Map<Integer, VTNFlowFilter> map = new TreeMap<>();
            for (VtnFlowFilter vff: list) {
                VTNFlowFilter ff = VTNFlowFilter.create(vff);
                map.put(ff.getIdentifier(), ff);
            }
            flowFilters = MiscUtils.toValueList(map);
        }
    }

    /**
     * Return a list of flow filters in this list.
     *
     * @return  A list of {@link VTNFlowFilter} instances.
     *          {@link VTNFlowFilter} instances in the returned collection are
     *          always sorted in ascending order of filter indices.
     */
    public List<VTNFlowFilter> getFlowFilters() {
        return (flowFilters == null)
            ? Collections.<VTNFlowFilter>emptyList()
            : Collections.unmodifiableList(flowFilters);
    }

    /**
     * Convert this instance into a list of {@link VtnFlowFilter} instances.
     *
     * @param ident   A {@link VNodeIdentifier} instance that specifies the
     *                virtual node that contains this flow filter list.
     * @return  A list of {@link VtnFlowFilter} instances if this instance
     *          contains at least one flow filter.
     *          Otherwise {@code null}.
     * @throws RpcException  The flow filter list is broken.
     */
    public List<VtnFlowFilter> toVtnFlowFilterList(VNodeIdentifier<?> ident)
        throws RpcException {
        List<VtnFlowFilter> list;
        if (isEmpty()) {
            list = null;
        } else {
            // Need to sort flow filters by filter index.
            Map<Integer, VtnFlowFilter> map = new TreeMap<>();
            for (VTNFlowFilter ff: flowFilters) {
                ff.verify();
                ff.canSet(ident);
                Integer index = ff.getIdentifier();
                if (map.put(index, ff.toVtnFlowFilter()) != null) {
                    throw RpcException.getBadArgumentException(
                        "Duplicate flow filter index: " + index);
                }
            }
            list = MiscUtils.toValueList(map);
        }

        return list;
    }

    /**
     * Determine whether the flow filter list is empty or not.
     *
     * @return  {@code true} only if the flow filter list is empty.
     */
    public boolean isEmpty() {
        return (flowFilters == null || flowFilters.isEmpty());
    }

    /**
     * Return the identifier for this flow filter list.
     *
     * @return  A {@link FlowFilterListId} if the identifier is configured in
     *          this instance. {@code null} if not configured.
     */
    public FlowFilterListId getListId() {
        return listId;
    }

    /**
     * Set the identifier for this flow filter list.
     *
     * @param id  A {@link FlowFilterListId} instance.
     */
    public void setListId(FlowFilterListId id) {
        listId = id;
    }

    /**
     * Evaluate flow filters configured in this instance.
     *
     * <ul>
     *   <li>
     *     Note that the caller must ensure that this flow filter list is not
     *     empty.
     *   </li>
     *   <li>
     *     Note that the caller must configure the identifier of this list
     *     by calling {@link #setListId(FlowFilterListId)} in advance.
     *   </li>
     * </ul>
     *
     * @param fctx   A flow filter context which contains the packet.
     * @param vid    A VLAN ID to be used for packet matching.
     *               A VLAN ID configured in the given packet is used if a
     *               negative value is specified.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter configured in
     *    this instance.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter configured in
     *    this instance.
     */
    public void evaluate(FlowFilterContext fctx, int vid)
        throws DropFlowException, RedirectFlowException {
        if (vid >= 0) {
            // Use the given VLAN ID for packet matching.
            fctx.setVlanId(vid);
        }

        FlowFilterListId flid = listId;
        for (VTNFlowFilter ff: flowFilters) {
            if (ff.evaluate(fctx, flid)) {
                break;
            }
        }
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
            FlowFilterList ffl = (FlowFilterList)o;
            ret = Objects.equals(flowFilters, ffl.flowFilters);
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
        return Objects.hash(getClass(), flowFilters);
    }
}
