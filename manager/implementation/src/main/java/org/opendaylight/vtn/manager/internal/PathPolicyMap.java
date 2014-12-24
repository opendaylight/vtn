/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.routing.IRouting;

/**
 * {@code PathPolicyMap} class describes a set of link costs configured in
 * a path policy.
 *
 * <p>
 *   An instance of this class is used to tell the link cost to the
 *   SAL routing. Although this class implements {@link Map} interface,
 *   only {@link #get(Object)} is available.
 * </p>
 */
public final class PathPolicyMap implements Map<Edge, Number>, RouteResolver {
    /**
     * VTN Manager service.
     */
    private final VTNManagerImpl  vtnManager;

    /**
     * The identifier of the path policy.
     */
    private final Integer  policyId;

    /**
     * Construct a new instance.
     *
     * @param mgr  VTN Manager service.
     * @param id   The identifier of the path policy corresponding to this
     *             instance.
     */
    public PathPolicyMap(VTNManagerImpl mgr, int id) {
        vtnManager = mgr;
        policyId = Integer.valueOf(id);
    }

    /**
     * Register this map to the SAL routing.
     *
     * @return  {@code true} if succeeded. Otherwise {@code false}.
     */
    public boolean register() {
        // Currently the SAL routing does not provide the way to detect
        // initialization failure.
        IRouting routing = vtnManager.getRouting();
        routing.initMaxThroughput(this);
        return true;
    }

    /**
     * Reset internal state.
     */
    public void reset() {
        IRouting routing = vtnManager.getRouting();
        routing.clearMaxThroughput();
    }

    // RouteResolver

    /**
     * {@inheritDoc}
     */
    @Override
    public Path getRoute(Node src, Node dst) {
        IRouting routing = vtnManager.getRouting();
        return routing.getMaxThroughputRoute(src, dst);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public int getPathPolicyId() {
        return policyId;
    }

    // Map

    /**
     * Always returns zero.
     *
     * @return      Zero.
     * @deprecated  Not implemented.
     */
    @Override
    public int size() {
        return 0;
    }

    /**
     * Always returns {@code true}.
     *
     * @return      {@code true}.
     * @deprecated  Not implemented.
     */
    @Override
    public boolean isEmpty() {
        return true;
    }

    /**
     * Always returns {@code false}.
     *
     * @param key   Always ignored.
     * @return      {@code false}.
     * @deprecated  Not implemented.
     */
    @Override
    public boolean containsKey(Object key) {
        return false;
    }

    /**
     * Always returns {@code false}.
     *
     * @param value  Always ignored.
     * @return       {@code false}.
     * @deprecated   Not implemented.
     */
    @Override
    public boolean containsValue(Object value) {
        return false;
    }

    /**
     * Return the link cost of transmitting a packet via the specified edge.
     *
     * @param key  A {@link Edge} instance.
     * @return     A {@link Number} instance which represents the link cost.
     * @throws ClassCastException
     *    The specified key is not an instance of {@link Edge}.
     */
    @Override
    public Number get(Object key) {
        // Remarks:
        //   This method will be called with holding the SAL routing service
        //   lock. So this method must not call any methods which acquire
        //   VTN Manager lock.

        // Determine the cost configured to the tail node connector.
        Edge edge = (Edge)key;
        NodeConnector port = edge.getTailNodeConnector();

        return vtnManager.getLinkCost(policyId, port);
    }

    /**
     * Always returns {@code null}.
     *
     * @param key    Always ignored.
     * @param value  Always ignored.
     * @return       {@code null}.
     * @deprecated   Not implemented.
     */
    @Override
    public Number put(Edge key, Number value) {
        return null;
    }

    /**
     * Always returns {@code null}.
     *
     * @param key  Always ignored.
     * @return     {@code null}.
     * @deprecated Not implemented.
     */
    @Override
    public Number remove(Object key) {
        return null;
    }

    /**
     * Does nothing.
     *
     * @param m     Always ignored.
     * @deprecated  Not implemented.
     */
    @Override
    public void putAll(Map<? extends Edge, ? extends Number> m) {
    }

    /**
     * Does nothing.
     *
     * @deprecated  Not implemented.
     */
    @Override
    public void clear() {
    }

    /**
     * Always returns an empty set.
     *
     * @return     An empty set.
     * @deprecated Not implemented.
     */
    @Override
    public Set<Edge> keySet() {
        return new HashSet<Edge>();
    }

    /**
     * Always returns an empty collection.
     *
     * @return     An empty collection.
     * @deprecated Not implemented.
     */
    @Override
    public Collection<Number> values() {
        return new ArrayList<Number>(0);
    }

    /**
     * Always returns an empty set.
     *
     * @return     An empty set.
     * @deprecated Not implemented.
     */
    @Override
    public Set<Map.Entry<Edge, Number>> entrySet() {
        return new HashSet<Map.Entry<Edge, Number>>();
    }
}
