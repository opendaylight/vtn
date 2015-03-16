/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.util.VTNIdentifiable;
import org.opendaylight.vtn.manager.util.VTNIdentifiableComparator;

import org.opendaylight.vtn.manager.internal.util.flow.match.FlowMatchContext;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.SetFlowConditionInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowCondConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VTNFlowCondition} describes configuration for a flow condition.
 */
@XmlRootElement(name = "vtn-flow-condition")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNFlowCondition implements VTNIdentifiable<String> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNFlowCondition.class);

    /**
     * The name of the flow condition.
     */
    @XmlElement(required = true)
    private String  name;

    /**
     * A list of {@link VTNFlowMatch} instances sorted by match index.
     */
    @XmlElementWrapper(name = "vtn-flow-matches")
    @XmlElement(name = "vtn-flow-match")
    private List<VTNFlowMatch>  matches;

    /**
     * A {@link VnodeName} instance that contains the name of this flow
     * condition.
     */
    private VnodeName  nodeName;

    /**
     * {@code MatchInitializer} initializes the list of {@link VTNFlowMatch}
     * instances.
     *
     * @param <T>  The type of object to be used to create a
     *             {@link VTNFlowMatch} instance.
     */
    private abstract static class MatchInitializer<T> {
        /**
         * Initialize the list of {@link VTNFlowMatch} instances.
         *
         * @param srcList  A list of objects used to create a list of
         *                 {@link VTNFlowMatch} instances.
         * @return  A list of {@link VTNFlowMatch} instance.
         * @throws RpcException  An error occurred.
         */
        protected final List<VTNFlowMatch> initialize(List<T> srcList)
            throws RpcException {
            if (srcList == null || srcList.isEmpty()) {
                return Collections.<VTNFlowMatch>emptyList();
            }

            // Ensure that all match indices are unique.
            List<VTNFlowMatch> list = newList(srcList);
            Set<Integer> indices = new HashSet<>();
            for (T obj: srcList) {
                VTNFlowMatch vfmatch = convert(obj);
                Integer index = vfmatch.getIdentifier();
                FlowCondUtils.verifyMatchIndex(indices, index);
                add(list, vfmatch);
            }

            // Sort matches by index.
            VTNIdentifiableComparator<Integer> comparator =
                new VTNIdentifiableComparator<>(Integer.class);
            Collections.sort(list, comparator);

            return list;
        }

        /**
         * Convert the given object into a {@link VTNFlowMatch} instance.
         *
         * @param obj  An object to be converted.
         * @return  A {@link VTNFlowMatch} instance.
         * @throws RpcException  An error occurred.
         */
        protected abstract VTNFlowMatch convert(T obj) throws RpcException;

        /**
         * Return a new list of {@link VTNFlowMatch} instances.
         *
         * @param srcList  A list of objects used to create a list of
         *                 {@link VTNFlowMatch} instances.
         * @return  A list of {@link VTNFlowMatch} instance.
         */
        protected abstract List<VTNFlowMatch> newList(List<T> srcList);

        /**
         * Add the given {@link VTNFlowMatch} instance into the tail of the
         * given list.
         *
         * @param list     A list of {@link VTNFlowMatch} instances.
         * @param vfmatch  A {@link VTNFlowMatch} instance to be added.
         */
        protected abstract void add(List<VTNFlowMatch> list,
                                    VTNFlowMatch vfmatch);
    }

    /**
     * {@code MatchVerifier} verifies the given list of {@link VTNFlowMatch}
     * instances.
     */
    private static final class MatchVerifier
        extends MatchInitializer<VTNFlowMatch> {
        /**
         * Convert the given object into a {@link VTNFlowMatch} instance.
         *
         * @param vfmatch  A {@link VTNFlowMatch} instance.
         * @return  {@code vfmatch}.
         * @throws RpcException  An error occurred.
         */
        @Override
        protected VTNFlowMatch convert(VTNFlowMatch vfmatch)
            throws RpcException {
            vfmatch.verify();
            return vfmatch;
        }

        /**
         * This method does nothing.
         *
         * @param srcList  A list of {@link VTNFlowMatch} instances.
         * @return  {@code srcList} is always returned.
         */
        @Override
        protected List<VTNFlowMatch> newList(List<VTNFlowMatch> srcList) {
            return srcList;
        }

        /**
         * This method does nothing.
         *
         * @param list     Unused.
         * @param vfmatch  Unused.
         */
        @Override
        protected void add(List<VTNFlowMatch> list, VTNFlowMatch vfmatch) {
        }
    }

    /**
     * {@code MatchConverter} converts the given list into a list of
     * {@link VTNFlowMatch} instances.
     *
     * @param <T>  The type of object to be used to create a
     *             {@link VTNFlowMatch} instance.
     */
    private abstract static class MatchConverter<T>
        extends MatchInitializer<T> {
        /**
         * {@inheritDoc}
         */
        @Override
        protected final List<VTNFlowMatch> newList(List<T> srcList) {
            return new ArrayList<VTNFlowMatch>(srcList.size());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected final void add(List<VTNFlowMatch> list,
                                 VTNFlowMatch vfmatch) {
            list.add(vfmatch);
        }
    }

    /**
     * {@code FlowMatchConverter} converts the given list of {@link FlowMatch}
     * instances into a list of {@link VTNFlowMatch} instances.
     */
    public static final class FlowMatchConverter
        extends MatchConverter<FlowMatch> {
        /**
         * Convert the given {@link FlowMatch} instance into a
         * {@link VTNFlowMatch} instance.
         *
         * @param fm  A {@link FlowMatch} instance to be converted.
         * @return  A {@link VTNFlowMatch} instance.
         * @throws RpcException  An error occurred.
         */
        protected VTNFlowMatch convert(FlowMatch fm) throws RpcException {
            return new VTNFlowMatch(fm);
        }
    }

    /**
     * {@code VtnFlowMatchConverter} converts the given list of
     * {@link VtnFlowMatch}
     * instances into a list of {@link VTNFlowMatch} instances.
     */
    public static final class VtnFlowMatchConverter
        extends MatchConverter<VtnFlowMatch> {
        /**
         * Convert the given {@link VtnFlowMatch} instance into a
         * {@link VTNFlowMatch} instance.
         *
         * @param vfm  A {@link VtnFlowMatch} instance to be converted.
         * @return  A {@link VTNFlowMatch} instance.
         * @throws RpcException  An error occurred.
         */
        protected VTNFlowMatch convert(VtnFlowMatch vfm) throws RpcException {
            return new VTNFlowMatch(vfm);
        }
    }

    /**
     * Convert the given {@link VtnFlowCondition} instance into a
     * {@link VTNFlowCondition} instance.
     *
     * @param vfc  A {@link VtnFlowCondition} instance to be converted.
     * @return  A {@link VTNFlowCondition} instance on success.
     *          {@code null} on failure.
     */
    public static VTNFlowCondition create(VtnFlowCondition vfc) {
        try {
            return new VTNFlowCondition(vfc);
        } catch (Exception e) {
            LOG.warn("Ignore broken flow condition: " + vfc, e);
        }

        return null;
    }

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private VTNFlowCondition() {
    }

    /**
     * Construct a new instance from the given name and {@link FlowCondition}
     * instance.
     *
     * @param nm     The name of the flow condition.
     * @param fcond  A {@link FlowCondition} instance.
     * @throws RpcException   An error occurred.
     */
    public VTNFlowCondition(String nm, FlowCondition fcond)
        throws RpcException {
        nodeName = FlowCondUtils.checkName(nm);
        name = nm;

        List<FlowMatch> list = (fcond == null)
            ? null : fcond.getMatches();
        matches = new FlowMatchConverter().initialize(list);
    }

    /**
     * Construct a new instance from the given {@link VtnFlowCondConfig}
     * instance.
     *
     * @param vfconf  A {@link VtnFlowCondConfig} instance.
     * @throws RpcException   An error occurred.
     */
    public VTNFlowCondition(VtnFlowCondConfig vfconf) throws RpcException {
        nodeName = vfconf.getName();
        name = FlowCondUtils.checkName(nodeName);

        List<VtnFlowMatch> list = vfconf.getVtnFlowMatch();
        matches = new VtnFlowMatchConverter().initialize(list);
    }

    /**
     * Return a {@link FlowCondition} instance which represents this condition.
     *
     * @return  A {@link FlowCondition} instance.
     */
    public FlowCondition toFlowCondition() {
        List<FlowMatch> list;
        if (matches.isEmpty()) {
            list = null;
        } else {
            list = new ArrayList<FlowMatch>();
            for (VTNFlowMatch vfmatch: matches) {
                list.add(vfmatch.toFlowMatch());
            }
        }

        return new FlowCondition(name, list);
    }

    /**
     * Return a {@link VtnFlowConditionBuilder} instance which contains the
     * configuration configured in this instance.
     *
     * @return  A {@link VtnFlowConditionBuilder} instance.
     */
    public VtnFlowConditionBuilder toVtnFlowConditionBuilder() {
        VtnFlowConditionBuilder builder = new VtnFlowConditionBuilder().
            setName(nodeName);

        List<VtnFlowMatch> list;
        if (matches != null && !matches.isEmpty()) {
            list = new ArrayList<VtnFlowMatch>();
            for (VTNFlowMatch vfmatch: matches) {
                list.add(vfmatch.toVtnFlowMatchBuilder().build());
            }
            builder.setVtnFlowMatch(list);
        }

        return builder;
    }

    /**
     * Return a {@link SetFlowConditionInputBuilder} instance which contains
     * the configuration configured in this instance.
     *
     * @return  A {@link SetFlowConditionInputBuilder} instance.
     */
    public SetFlowConditionInputBuilder toSetFlowConditionInputBuilder() {
        VtnFlowConditionBuilder builder = toVtnFlowConditionBuilder();
        return new SetFlowConditionInputBuilder(builder.build());
    }

    /**
     * Return the path to this flow condition.
     *
     * @return  An {@link InstanceIdentifier} instance that represents the path
     *          to this flow condition.
     */
    public InstanceIdentifier<VtnFlowCondition> getPath() {
        return InstanceIdentifier.builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class, new VtnFlowConditionKey(nodeName)).
            build();
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verifycation failed.
     */
    public void verify() throws RpcException {
        nodeName = FlowCondUtils.checkName(name);
        matches = new MatchVerifier().initialize(matches);
    }

    /**
     * Determine whether this flow condition matches the given packet header
     * or not.
     *
     * @param ctx  A {@link FlowMatchContext} instance.
     * @return  {@code true} if this flow condition matches the packet header
     *          specified by {@code ctx}. Otherwise {@code false}.
     */
    public boolean match(FlowMatchContext ctx) {
        boolean empty = matches.isEmpty();
        if (empty) {
            traceMatch(ctx, "Matched an empty condition");
            return true;
        }

        for (VTNFlowMatch vfmatch: matches) {
            if (vfmatch.match(ctx)) {
                traceMatch(ctx, "Matched the condition", vfmatch);
                return true;
            } else {
                traceMatch(ctx, "Does not match", vfmatch);
            }
        }

        traceMatch(ctx, "Unmatched");
        return false;
    }

    /**
     * Record a trace log for the result of {@link #match(FlowMatchContext)}.
     *
     * @param ctx      A {@link FlowMatchContext} instance.
     * @param msg      A message to be logged.
     */
    private void traceMatch(FlowMatchContext ctx, String msg) {
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: {}: packet=[{}]", name, msg,
                      ctx.getHeaderDescription());
        }
    }

    /**
     * Record a trace log for the result of {@link #match(FlowMatchContext)}.
     *
     * @param ctx      A {@link FlowMatchContext} instance.
     * @param msg      A message to be logged.
     * @param vfmatch  A {@link VTNFlowMatch} instance that matched the packet.
     */
    private void traceMatch(FlowMatchContext ctx, String msg,
                            VTNFlowMatch vfmatch) {
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: {}: match=[{}], packet=[{}]", name, msg,
                      vfmatch.getConditionKey(), ctx.getHeaderDescription());
        }
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * @return  The name of the flow condition.
     */
    @Override
    public String getIdentifier() {
        return name;
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        VTNFlowCondition vfcond = (VTNFlowCondition)o;
        return (Objects.equals(name, vfcond.name) &&
                Objects.equals(matches, vfcond.matches));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(name, matches);
    }
}
