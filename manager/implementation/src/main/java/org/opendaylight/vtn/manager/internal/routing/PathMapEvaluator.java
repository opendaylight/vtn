/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.flow.cond.FlowCondReader;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;
import org.opendaylight.vtn.manager.internal.util.pathmap.PathMapUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code PathMapEvaluator} is a utility class to evaluate path maps.
 */
public final class PathMapEvaluator {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PathMapEvaluator.class);

    /**
     * A pseudo VTN name that indicates the global path map.
     */
    private static final String  GLOBAL = "<global>";

    /**
     * A runtime context to evaluate path map.
     */
    private final PathMapContext  context;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * Flow condition reader.
     */
    private final FlowCondReader  conditionReader;

    /**
     * Construct a new instance.
     *
     * @param pmctx   A {@link PathMapContext} instance.
     */
    public PathMapEvaluator(PathMapContext pmctx) {
        context = pmctx;
        vtnProvider = pmctx.getProvider();
        conditionReader = pmctx.getFlowCondReader();
    }

    /**
     * Evaluate path maps against the given packet.
     *
     * @param ident  The identifier for the virtual node in the target VTN.
     * @return  A {@link RouteResolver} instance is returned if a path map
     *          matched the given packet. The default route resolver is
     *          returned if no path map matched the packet.
     */
    public RouteResolver evaluate(VNodeIdentifier<?> ident) {
        // Evaluate VTN path map list.
        VTenantIdentifier vtnId = new VTenantIdentifier(ident.getTenantName());
        String tname = vtnId.getTenantNameString();
        ReadTransaction rtx = conditionReader.getReadTransaction();
        try {
            List<VtnPathMap> list = PathMapUtils.readPathMaps(rtx, vtnId);
            RouteResolver rr = evaluate(tname, list);
            if (rr != null) {
                return rr;
            }
        } catch (Exception e) {
            LOG.warn(tname + ": Ignore unreadable VTN path map.", e);
        }

        // Evaluate global path map list.
        try {
            List<VtnPathMap> list = PathMapUtils.readPathMaps(rtx);
            RouteResolver rr = evaluate(GLOBAL, list);
            if (rr != null) {
                return rr;
            }
        } catch (Exception e) {
            LOG.warn(tname + ": Ignore unreadable global path map.", e);
        }

        // Use the system default resolver.
        return vtnProvider.getRouteResolver();
    }

    /**
     * Evaluate path maps in the given list against the given packet.
     *
     * @param tname  The name of the VTN.
     * @param list   A list of {@link VtnPathMap} instances.
     * @return  A {@link RouteResolver} instance is returned if a path map
     *          matched the given packet. {@code null} is returned if no path
     *          path map matched the packet.
     */
    private RouteResolver evaluate(String tname, List<VtnPathMap> list) {
        for (VtnPathMap vpm: list) {
            RouteResolver rr = evaluate(tname, vpm);
            if (rr != null) {
                return rr;
            }
        }

        return null;
    }

    /**
     * Evaluate the given path map against the given packet.
     *
     * @param tname  The name of the VTN.
     * @param vpm    A {@link VtnPathMap} instance.
     * @return  A {@link RouteResolver} instance is returned if the given path
     *          map matched the given packet. Otherwise {@code null}.
     */
    private RouteResolver evaluate(String tname, VtnPathMap vpm) {
        VnodeName vcond = vpm.getCondition();
        if (vcond == null) {
            // This should never happen.
            LOG.warn("{}.{}: Ignore path map: no condition: {}",
                     tname, vpm.getIndex(), vpm);
            return null;
        }

        String cond = vcond.getValue();
        VTNFlowCondition vfcond = conditionReader.get(cond);
        if (vfcond == null) {
            LOG.debug("{}.{}: Ignore path map: condition not found: {}",
                      tname, vpm.getIndex(), cond);
            return null;
        }

        Integer policy = vpm.getPolicy();
        RouteResolver rr = vtnProvider.getRouteResolver(policy);
        if (rr == null) {
            LOG.debug("{}.{}: Ignore path map: path policy not found: {}",
                      tname, vpm.getIndex(), policy);
        } else {
            if (vfcond.match(context)) {
                Integer idle = vpm.getIdleTimeout();
                if (idle != null) {
                    // Set flow timeout.
                    Integer hard = vpm.getHardTimeout();
                    context.setFlowTimeout(idle.intValue(), hard.intValue());
                }
            } else {
                rr = null;
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Path map {}: cond={}, policy={}, " +
                          "packet={}", tname, vpm.getIndex(),
                          (rr == null) ? "not matched" : "matched",
                          cond, policy, context.getDescription());
            }
        }

        return rr;
    }
}
