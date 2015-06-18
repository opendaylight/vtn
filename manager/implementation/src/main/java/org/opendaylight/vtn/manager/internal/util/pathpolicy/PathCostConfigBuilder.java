/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import org.opendaylight.vtn.manager.PathCost;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * {@code PathCostConfigBuilder} describes the builder class that constructs
 * data object that represents path cost configuration.
 *
 * @param <T>  Actual type of the builder instance.
 */
public abstract class PathCostConfigBuilder<T> {
    /**
     * Construct an empty builder instance.
     */
    PathCostConfigBuilder() {}

    /**
     * Set the contents of the given {@link PathCost} instance into this
     * builder instance.
     *
     * @param pc  A {@link PathCost} instance.
     * @return  This instance.
     * @throws RpcException
     *    The given value contains an invalid value.
     */
    public final PathCostConfigBuilder<T> set(PathCost pc)
        throws RpcException {
        if (pc == null) {
            throw MiscUtils.getNullArgumentException("PathCost");
        }

        Long cost = pc.getCost();
        if (cost == null) {
            throw MiscUtils.getNullArgumentException("Cost in PathCost");
        }
        setCost(cost);

        VtnPortDesc vdesc = NodeUtils.toVtnPortDesc(pc.getLocation());
        setPortDescImpl(vdesc);
        return this;
    }

    /**
     * Set the contents of the given {@link VtnPathCostConfig} instance into
     * this builder instance.
     *
     * @param vpc  A {@link VtnPathCostConfig} instance.
     * @return  This instance.
     * @throws RpcException
     *    The given value contains an invalid value.
     */
    public final PathCostConfigBuilder<T> set(VtnPathCostConfig vpc)
        throws RpcException {
        if (vpc == null) {
            throw PathPolicyUtils.getNullPathCostException();
        }

        return setPortDesc(vpc.getPortDesc()).setCost(vpc.getCost());
    }

    /**
     * Set the switch port descriptor.
     *
     * @param value  A {@link VtnPortDesc} instance which descripes the
     *               location of the switch port.
     * @return  This instance.
     * @throws RpcException
     *    The given value is invalid.
     */
    public final PathCostConfigBuilder<T> setPortDesc(VtnPortDesc value)
        throws RpcException {
        if (value == null) {
            throw PathPolicyUtils.getNullPortDescException();
        }

        String desc = value.getValue();
        if (desc == null) {
            throw PathPolicyUtils.getNullPortDescException();
        }

        if (NodeUtils.toPortLocation(value) == null) {
            String msg = MiscUtils.joinColon("Invalid port descriptor",
                                             value.getValue());
            throw RpcException.getBadArgumentException(msg);
        }

        setPortDescImpl(value);
        return this;
    }

    /**
     * Set the link cost for the switch port.
     *
     * @param value  A {@link Long} instance which represents the link cost.
     * @return  This instance.
     * @throws RpcException
     *    The given value is invalid.
     */
    public final PathCostConfigBuilder<T> setCost(Long value)
        throws RpcException {
        Long cost = value;
        if (cost == null) {
            // Use default value.
            cost = PathPolicyUtils.DEFAULT_LINK_COST;
        }

        try {
            setCostImpl(cost);
        } catch (IllegalArgumentException e) {
            RpcException re = PathPolicyUtils.getInvalidCostException(value);
            re.initCause(e);
            throw re;
        }

        return this;
    }

    /**
     * Return the builder instance configured in this instance.
     *
     * @return  The builder instance.
     */
    public abstract T getBuilder();

    /**
     * Set the switch port descriptor.
     *
     * @param value  A {@link VtnPortDesc} instance which descripes the
     *               location of the switch port.
     * @throws IllegalArgumentException
     *    The given value is invalid.
     */
    protected abstract void setPortDescImpl(VtnPortDesc value);

    /**
     * Set the link cost for the switch port.
     *
     * @param value  A {@link Long} instance which represents the link cost.
     * @throws IllegalArgumentException
     *    The given value is invalid.
     */
    protected abstract void setCostImpl(Long value);

    /**
     * An implementation of {@link PathCostConfigBuilder} for
     * {@link VtnPathCostBuilder}.
     */
    public static final class Data
        extends PathCostConfigBuilder<VtnPathCostBuilder> {
        /**
         * A builder instance.
         */
        private final VtnPathCostBuilder  builder = new VtnPathCostBuilder();

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathCostBuilder getBuilder() {
            return builder;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setPortDescImpl(VtnPortDesc vdesc) {
            builder.setPortDesc(vdesc);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setCostImpl(Long value) {
            builder.setCost(value);
        }
    }

    /**
     * An implementation of {@link PathCostConfigBuilder} for
     * {@link PathCostListBuilder}.
     */
    public static final class Rpc
        extends PathCostConfigBuilder<PathCostListBuilder> {
        /**
         * The builder class.
         */
        private final PathCostListBuilder builder = new PathCostListBuilder();

        /**
         * {@inheritDoc}
         */
        @Override
        public PathCostListBuilder getBuilder() {
            return builder;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setPortDescImpl(VtnPortDesc vdesc) {
            builder.setPortDesc(vdesc);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void setCostImpl(Long value) {
            builder.setCost(value);
        }
    }
}
