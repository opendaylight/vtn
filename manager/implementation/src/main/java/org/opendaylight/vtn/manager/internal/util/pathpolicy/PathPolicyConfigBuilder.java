/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PathPolicy;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;

/**
 * {@code PathPolicyConfigBuilder} describes the builder class that constructs
 * data object that represents path policy configuration.
 *
 * @param <T>  Actual type of the builder instance.
 */
public abstract class PathPolicyConfigBuilder<T> {
    /**
     * Construct an empty builder instance.
     */
    PathPolicyConfigBuilder() {}

    /**
     * Set the contents of the given {@link PathPolicy} instance into this
     * builder instance.
     *
     * @param pp  A {@link PathPolicy} instance.
     * @return  This instance.
     * @throws RpcException
     *    The given value contains an invalid value.
     */
    public final PathPolicyConfigBuilder<T> set(PathPolicy pp)
        throws RpcException {
        return set(pp, null);
    }

    /**
     * Set the contents of the given {@link PathPolicy} instance into this
     * builder instance.
     *
     * @param pp   A {@link PathPolicy} instance.
     * @param pid  Path policy ID to be set to the builder instance.
     *             If {@code null} is specified, the path policy ID configured
     *             in {@code pp} is used.
     * @return  This instance.
     * @throws RpcException
     *    The given value contains an invalid value.
     */
    public final PathPolicyConfigBuilder<T> set(PathPolicy pp, Integer pid)
        throws RpcException {
        if (pp == null) {
            throw MiscUtils.getNullArgumentException("PathPolicy");
        }

        Integer id = pid;
        if (id == null) {
            id = pp.getPolicyId();
        }
        setId(id).setDefaultCost(Long.valueOf(pp.getDefaultCost()));

        Set<String> descSet = new HashSet<>();
        List<VtnPathCost> list = new ArrayList<>();
        for (PathCost pc: pp.getPathCosts()) {
            VtnPathCost vpc = new PathCostConfigBuilder.Data().set(pc).
                getBuilder().build();
            String desc = vpc.getPortDesc().getValue();
            if (!descSet.add(desc)) {
                throw PathPolicyUtils.
                    getDuplicatePortException(pc.getLocation());
            }
            list.add(vpc);
        }

        if (list.isEmpty()) {
            list = null;
        }

        return setVtnPathCost(list);
    }

    /**
     * Set the contents of the given {@link VtnPathPolicyConfig} instance
     * into this builder instance.
     *
     * @param vpp  A {@link VtnPathPolicyConfig} instance.
     * @return  This instance.
     * @throws RpcException
     *    The given value contains an invalid value.
     * @throws NullPointerException
     *    {@code vpc} is {@code null}.
     */
    public final PathPolicyConfigBuilder<T> set(VtnPathPolicyConfig vpp)
        throws RpcException {
        setId(vpp.getId()).setDefaultCost(vpp.getDefaultCost());

        List<VtnPathCost> costs = vpp.getVtnPathCost();
        if (costs != null) {
            Set<String> descSet = new HashSet<>();
            List<VtnPathCost> newCosts = new ArrayList<>(costs.size());
            for (VtnPathCost vpc: costs) {
                VtnPathCost v = new PathCostConfigBuilder.Data().set(vpc).
                    getBuilder().build();
                String desc = v.getPortDesc().getValue();
                if (!descSet.add(desc)) {
                    throw PathPolicyUtils.getDuplicatePortException(desc);
                }
                newCosts.add(v);
            }
            setVtnPathCost(newCosts);
        }

        return this;
    }

    /**
     * Set the identifier of the path policy.
     *
     * @param value  A {@link Integer} instance which represents the path
     *               policy identifier.
     * @return  This instance.
     * @throws RpcException
     *    The given value is invalid.
     */
    public final PathPolicyConfigBuilder<T> setId(Integer value)
        throws RpcException {
        if (value == null) {
            throw PathPolicyUtils.getNullPolicyIdException();
        }

        try {
            setIdImpl(value);
        } catch (IllegalArgumentException e) {
            RpcException re = PathPolicyUtils.
                getInvalidPolicyIdException(value);
            re.initCause(e);
            throw re;
        }

        return this;
    }

    /**
     * Set the default cost of the path policy.
     *
     * @param value  A {@link Long} instance which represents the default cost
     *               of the path policy.
     * @return  This instance.
     * @throws RpcException
     *    The given value is invalid.
     */
    public final PathPolicyConfigBuilder<T> setDefaultCost(Long value)
        throws RpcException {
        try {
            setDefaultCostImpl(value);
        } catch (IllegalArgumentException e) {
            RpcException re = PathPolicyUtils.
                getInvalidDefaultCostException(value);
            re.initCause(e);
            throw re;
        }

        return this;
    }

    /**
     * Set the link cost configurtion.
     *
     * @param value  A list of {@link VtnPathCost} instances.
     * @return  This instance.
     */
    public final PathPolicyConfigBuilder<T> setVtnPathCost(
        List<VtnPathCost> value) {
        setVtnPathCostImpl(value);
        return this;
    }

    /**
     * Return the builder instance configured in this instance.
     *
     * @return  The builder instance.
     */
    public abstract T getBuilder();

    /**
     * Set the identifier of the path policy.
     *
     * @param value  A {@link Integer} instance which represents the path
     *               policy identifier.
     * @throws IllegalArgumentException
     *    The given value is invalid.
     */
    protected abstract void setIdImpl(Integer value);

    /**
     * Set the default cost of the path policy.
     *
     * @param value  A {@link Long} instance which represents the default cost
     *               of the path policy.
     * @throws IllegalArgumentException
     *    The given value is invalid.
     */
    protected abstract void setDefaultCostImpl(Long value);

    /**
     * Set the link cost configurtion.
     *
     * @param value  A list of {@link VtnPathCost} instances.
     */
    protected abstract void setVtnPathCostImpl(List<VtnPathCost> value);

    /**
     * An implementation of {@link PathPolicyConfigBuilder} for
     * {@link VtnPathPolicyBuilder}.
     */
    public static final class Data
        extends PathPolicyConfigBuilder<VtnPathPolicyBuilder> {
        /**
         * A builder instance.
         */
        private final VtnPathPolicyBuilder  builder =
            new VtnPathPolicyBuilder();

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathPolicyBuilder getBuilder() {
            return builder;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setIdImpl(Integer value) {
            builder.setId(value);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setDefaultCostImpl(Long value) {
            builder.setDefaultCost(value);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setVtnPathCostImpl(List<VtnPathCost> value) {
            builder.setVtnPathCost(value);
        }
    }

    /**
     * An implementation of {@link PathPolicyConfigBuilder} for
     * {@link SetPathPolicyInputBuilder}.
     */
    public static final class Rpc
        extends PathPolicyConfigBuilder<SetPathPolicyInputBuilder> {
        /**
         * A builder instance.
         */
        private final SetPathPolicyInputBuilder  builder =
            new SetPathPolicyInputBuilder();

        /**
         * {@inheritDoc}
         */
        @Override
        public SetPathPolicyInputBuilder getBuilder() {
            return builder;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setIdImpl(Integer value) {
            builder.setId(value);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setDefaultCostImpl(Long value) {
            builder.setDefaultCost(value);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setVtnPathCostImpl(List<VtnPathCost> value) {
            builder.setVtnPathCost(value);
        }
    }
}
