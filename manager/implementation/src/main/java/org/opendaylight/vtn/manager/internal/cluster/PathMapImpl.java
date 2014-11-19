/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * This class provides base implementation of path map.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class PathMapImpl implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4357453759022403623L;

    /**
     * The minimum value of index.
     */
    private static final int  INDEX_MIN = 1;

    /**
     * The maximum value of index.
     */
    private static final int  INDEX_MAX = 65535;

    /**
     * A pseudo flow timeout value which represents an undefined value.
     */
    private static final int  FLOW_TIMEOUT_UNDEF = -1;

    /**
     * Mask value which represents valid bits in a flow timeout value.
     */
    private static final int  FLOW_TIMEOUT_MASK = 0xffff;

    /**
     * An index value assigned to this instance.
     */
    private final int  index;

    /**
     * The name of the flow condition.
     */
    private final String  condition;

    /**
     * The identifier of the path policy.
     */
    private final int  policyId;

    /**
     * The number of seconds to be set to {@code idle_timeout} field in
     * flow entries.
     */
    private final int  idleTimeout;

    /**
     * The number of seconds to be set to {@code hard_timeout} field in
     * flow entries.
     */
    private final int  hardTimeout;

    /**
     * Construct a new instance.
     *
     * @param idx    An index number to be assigned.
     * @param ptmap  A {@link PathMap} instance.
     * @throws VTNException
     *    {@code ptmap} contains invalid value.
     */
    protected PathMapImpl(int idx, PathMap ptmap) throws VTNException {
        if (ptmap == null) {
            Status st = MiscUtils.argumentIsNull("Path map");
            throw new VTNException(st);
        }

        if (idx < INDEX_MIN || idx > INDEX_MAX) {
            String msg = "Invalid index: " + idx;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        index = idx;
        condition = ptmap.getFlowConditionName();
        MiscUtils.checkName("Flow condition", condition);

        policyId = ptmap.getPathPolicyId();
        if (policyId != PathPolicyImpl.POLICY_ID &&
            policyId != PathPolicyImpl.POLICY_DEFAULT) {
            String msg = "Invalid policy ID: " + policyId;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        idleTimeout = getTimeout(ptmap.getIdleTimeout(), "idle");
        hardTimeout = getTimeout(ptmap.getHardTimeout(), "hard");
        if (hardTimeout < 0) {
            if (idleTimeout >= 0) {
                String msg = "Hard timeout must be specified";
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        } else if (idleTimeout < 0) {
            String msg = "Idle timeout must be specified";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        } else if (idleTimeout != 0 && hardTimeout != 0 &&
                   idleTimeout >= hardTimeout) {
            String msg = "Idle timeout must be less than hard timeout";
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Return an index value assigned to this path map.
     *
     * @return  An index value assigned to this path map.
     */
    public final int getIndex() {
        return index;
    }

    /**
     * Return a {@link PathMap} instance which represents the path map
     * configuration.
     *
     * @return  A {@link PathMap} instance.
     */
    public final PathMap getPathMap() {
        if (idleTimeout < 0) {
            assert hardTimeout < 0;
            return new PathMap(index, condition, policyId);
        }

        return new PathMap(index, condition, policyId, idleTimeout,
                           hardTimeout);
    }

    /**
     * Return flow timeout value in the specified instance.
     *
     * @param i     An {@link Integer} instance.
     * @param desc  A brief description about the value.
     * @return  An integer value in the specified instance.
     *          {@link #FLOW_TIMEOUT_UNDEF} is returned if {@code null} is
     *          specified.
     * @throws VTNException  An invalid value is specified.
     */
    private int getTimeout(Integer i, String desc) throws VTNException {
        if (i == null) {
            return FLOW_TIMEOUT_UNDEF;
        }

        int timeout = i.intValue();
        if ((timeout & ~FLOW_TIMEOUT_MASK) != 0) {
            StringBuilder builder = new StringBuilder("Invalid ");
            builder.append(desc).append(" timeout: ").append(timeout);
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }

        return timeout;
    }

    /**
     * Evaluate this path map.
     *
     * <p>
     *   This method must be called with holding the VTN Manager lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param pctx  A packet context which contains the packet.
     * @return  A {@link RouteResolver} instance if this path map matched the
     *          packet. Otherwise {@code null}.
     */
    public RouteResolver evaluate(VTNManagerImpl mgr, PacketContext pctx) {
        FlowCondImpl fc = mgr.getFlowCondDB().get(condition);
        Logger logger = getLogger();
        if (fc == null) {
            logger.debug("{}{}: Ignore path map: condition not found: {}",
                         mgr.getContainerName(), getLogPrefix(), condition);
            return null;
        }

        RouteResolver rr = mgr.getRouteResolver(policyId);
        if (rr == null || !mgr.pathPolicyExists(policyId)) {
            logger.debug("{}{}: Ignore path map: path policy not found: {}",
                         mgr.getContainerName(), getLogPrefix(), policyId);
            return null;
        }

        if (fc.match(mgr, pctx)) {
            if (idleTimeout >= 0) {
                // Set flow timeout.
                pctx.setFlowTimeout(idleTimeout, hardTimeout);
            }
        } else {
            rr = null;
        }

        if (logger.isTraceEnabled()) {
            NodeConnector nc = pctx.getIncomingNodeConnector();
            logger.trace("{}{}: Path map {}: cond={}, policy={}, " +
                         "packet={}", mgr.getContainerName(), getLogPrefix(),
                         (rr == null) ? "not matched" : "matched",
                         condition, policyId, pctx.getDescription(nc));
        }

        return rr;
    }

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    protected abstract Logger getLogger();

    /**
     * Return a prefix of log record.
     *
     * @return  A prefix of log record.
     */
    protected abstract String getLogPrefix();

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        PathMapImpl pm = (PathMapImpl)o;
        return (index == pm.index && condition.equals(pm.condition) &&
                policyId == pm.policyId && idleTimeout == pm.idleTimeout &&
                hardTimeout == pm.hardTimeout);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return getClass().getName().hashCode() + index +
            condition.hashCode() * 3 + (policyId * 7) +
            (idleTimeout * 19) + (hardTimeout * 23);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        builder.append("[index=").append(index).
            append(",cond=").append(condition).
            append(",policy=").append(policyId);
        if (idleTimeout >= 0) {
            builder.append(",idle=").append(idleTimeout).
                append(",hard=").append(hardTimeout);
        }

        return builder.append(']').toString();
    }
}
