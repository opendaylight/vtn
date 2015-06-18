/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.uci.ics.jung.algorithms.shortestpath.DijkstraShortestPath;
import edu.uci.ics.jung.graph.SparseMultigraph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.graph.util.Pair;

import org.apache.commons.collections15.Transformer;

import org.opendaylight.vtn.manager.internal.RouteResolver;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.concurrent.TimeoutCounter;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;

/**
 * An implementation of {@link edu.uci.ics.jung.graph.Graph} that keeps
 * network topology graph for VTN internal use.
 */
final class TopologyGraph extends SparseMultigraph<SalNode, LinkEdge> {
    /**
     * Version number for serialization.
     */
    private static final long  serialVersionUID = 1L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(TopologyGraph.class);

    /**
     * Revision number for undefined route resolver.
     */
    private static final long  REV_UNDEF = 0L;

    /**
     * The number of milliseconds to wait for the route resolver to be
     * updated.
     */
    private static final long  UPDATE_TIMEOUT = 5000;

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * A map that keeps shortest paths calculated by Dijkstra algorithm.
     */
    private final Map<Integer, DijkstraResolver>  resolvers = new HashMap<>();

    /**
     * Revision numbers for path policies.
     */
    private final Map<Integer, Long>  revisions = new HashMap<>();

    /**
     * An implementation of {@link RouteResolver} that resolves the packet
     * route in physical network using Dijkstra algorithm.
     */
    private class DijkstraResolver
        extends DijkstraShortestPath<SalNode, LinkEdge>
        implements RouteResolver {
        /**
         * An index of this route resolver.
         */
        private final int  index;

        /**
         * Construct a new instance for the unweighted graph.
         *
         * @param idx  An index to be assigned.
         */
        private DijkstraResolver(int idx) {
            super(TopologyGraph.this);
            index = idx;
        }

        /**
         * Construct a new instance for the weighted graph.
         *
         * @param idx  An index to be assigned.
         * @param xf   A {@link Transformer} instance that is responsible for
         *             the weights of edges.
         */
        protected DijkstraResolver(int idx, Transformer<LinkEdge, Long> xf) {
            super(TopologyGraph.this, xf);
            index = idx;
        }

        /**
         * Return the index of this resolver.
         *
         * @return  Index of this resolver.
         */
        protected int getIndex() {
            return index;
        }

        /**
         * Set inventory reader used to determine link costs.
         *
         * @param rdr  A {@link InventoryReader} instance.
         * @return  A {@link InventoryReader} instance previously used.
         */
        protected InventoryReader setReader(InventoryReader rdr) {
            // Nothing to do.
            return null;
        }

        // RouteResolver

        /**
         * {@inheritDoc}
         */
        @Override
        public final List<LinkEdge> getRoute(InventoryReader rdr, SalNode src,
                                             SalNode dst) {
            if (src.getNodeNumber() == dst.getNodeNumber()) {
                return Collections.<LinkEdge>emptyList();
            }

            synchronized (TopologyGraph.this) {
                InventoryReader old = setReader(rdr);
                try {
                    List<LinkEdge> path = getPath(src, dst);
                    if (!path.isEmpty()) {
                        return path;
                    }
                } catch (Exception e) {
                    FixedLogger logger = new FixedLogger.Trace(LOG);
                    logger.log(e, "%d: A vertex is not yet known: %s -> %s",
                               index, src, dst);
                } finally {
                    setReader(old);
                }
            }

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public final int getPathPolicyId() {
            return index;
        }
    }

    /**
     * An implementation of {@link RouteResolver} that resolves the packet
     * route in physical network using path policy configuration.
     */
    private class PathPolicyResolver extends DijkstraResolver {
        /**
         * A {@link PathPolicyTransformer} instance assigned to this
         * resolver.
         */
        private final PathPolicyTransformer  xformer;

        /**
         * Construct a new instance.
         *
         * @param idx  An index to be assigned.
         * @param xf   A {@link Transformer} instance that is responsible for
         *             the weights of edges.
         */
        protected PathPolicyResolver(int idx, PathPolicyTransformer xf) {
            super(idx, xf);
            xformer = xf;
        }

        // DijkstraResolver

        /**
         * {@inheritDoc}
         */
        @Override
        protected InventoryReader setReader(InventoryReader rdr) {
            return xformer.setReader(rdr);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     */
    TopologyGraph(VTNManagerProvider provider) {
        vtnProvider = provider;

        // Create the default route resolver.
        int id = PathPolicyUtils.DEFAULT_POLICY;
        DijkstraResolver resolver = new DijkstraResolver(id);
        resolvers.put(Integer.valueOf(id), resolver);
    }

    /**
     * Initialize the network topology.
     *
     * @param vlinks  A list of existing {@link VtnLink} instances.
     */
    synchronized void initialize(List<VtnLink> vlinks) {
        if (vlinks != null) {
            for (VtnLink vlink: vlinks) {
                addLink(vlink);
            }
        }
    }

    /**
     * Update the route resolver for the given path policy.
     *
     * <p>
     *   The route resolver for the given path policy will be created if it
     *   is not present.
     * </p>
     *
     * @param index  The index of the route resolver.
     * @return  {@code true} if the resolver has been created.
     *          {@code false} if the resolver is already present.
     */
    synchronized boolean updateResolver(Integer index) {
        int id = index.intValue();
        if (id == PathPolicyUtils.DEFAULT_POLICY) {
            // This should never happen.
            LOG.debug("Default resolver is always present.");
            return false;
        }

        DijkstraResolver res = resolvers.get(index);
        boolean ret;
        if (res != null) {
            res.reset();
            LOG.debug("{}: Resolver has been reset.", index);
            ret = false;
        } else {
            PathPolicyTransformer xf = new PathPolicyTransformer(
                vtnProvider.getDataBroker(), id);
            PathPolicyResolver pres = new PathPolicyResolver(id, xf);
            resolvers.put(index, pres);
            LOG.debug("{}: New resolver has been added.", index);
            ret = true;
        }

        updateRevision(index);
        return ret;
    }

    /**
     * Remove the route resolver for the given path policy.
     *
     * @param index  The index of the route resolver.
     * @return  {@code true} if the resolver has been removed.
     *          {@code false} if the resolver is not present.
     */
    synchronized boolean removeResolver(Integer index) {
        int id = index.intValue();
        if (id == PathPolicyUtils.DEFAULT_POLICY) {
            // This should never happen.
            LOG.warn("Default resolver cannot be removed.");
            return false;
        }

        DijkstraResolver res = resolvers.remove(index);
        boolean ret = (res != null);
        if (ret) {
            LOG.debug("{}: Resolver has been removed.", index);
            updateRevision(index);
        } else {
            LOG.warn("{}: Resolver is not present.", index);
        }

        return ret;
    }

    /**
     * Return a packet route resolver.
     *
     * @param index  The index of the route resolver.
     * @return  A {@link RouteResolver} instance if found.
     *          {@code null} if not fonud.
     */
    synchronized RouteResolver getResolver(Integer index) {
        return resolvers.get(index);
    }

    /**
     * Update the network topology.
     *
     * @param added    A list of added {@link VtnLink} instances.
     * @param removed  A list of removed {@link VtnLink} instances.
     * @return  {@code true} if the network topology has been changed.
     *          {@code false} if unchanged.
     */
    synchronized boolean update(List<VtnLink> added, List<VtnLink> removed) {
        boolean updated = false;
        for (VtnLink vlink: added) {
            if (addLink(vlink)) {
                updated = true;
            }
        }

        for (VtnLink vlink: removed) {
            if (removeLink(vlink)) {
                updated = true;
            }
        }

        if (updated) {
            // Reset cached paths.
            for (DijkstraResolver resolver: resolvers.values()) {
                resolver.reset();
            }
        }

        return updated;
    }

    /**
     * Return the revision number for the given route resolver.
     *
     * @param index  The index of the route resolver.
     * @return  A {@link Long} instance which represents the revision number.
     */
    synchronized Long getRevision(Integer index) {
        Long rev = revisions.get(index);
        if (rev == null) {
            rev = Long.valueOf(REV_UNDEF);
        }
        return rev;
    }

    /**
     * Wait for the given route resolver to be updated.
     *
     * @param index  The index of the route resolver.
     * @param rev    Base revision number for the route resolver.
     */
    synchronized void awaitRevisionUpdated(Integer index, Long rev) {
        if (!getRevision(index).equals(rev)) {
            return;
        }

        TimeoutCounter tc =
            TimeoutCounter.newTimeout(UPDATE_TIMEOUT, TimeUnit.MILLISECONDS);
        Exception cause;
        try {
            do {
                tc.await(this);
            } while (getRevision(index).equals(rev));
            return;
        } catch (Exception e) {
            cause = e;
        }

        if (!getRevision(index).equals(rev)) {
            return;
        }

        if (cause instanceof TimeoutException) {
            LOG.warn("Route resolver was not updated: id={}, rev={}",
                     index, rev);
        } else {
            LOG.warn("Failed to wait for the route resolver: id=" + index +
                     ", rev=" + rev, cause);
        }
    }

    /**
     * Bump up the revision number for the given route resolver.
     *
     * @param index  The index of the route resolver.
     */
    private synchronized void updateRevision(Integer index) {
        Long rev = getRevision(index);
        rev = Long.valueOf(rev.longValue() + 1L);
        revisions.put(index, rev);
        notifyAll();
    }

    /**
     * Add the given link to the network topology.
     *
     * @param vlink  A {@link VtnLink} instance to be added.
     * @return  {@code true} if the network topology was updated.
     *          {@code false} if the network topology was not changed.
     */
    private synchronized boolean addLink(VtnLink vlink) {
        try {
            LinkEdge le = new LinkEdge(vlink);
            SalPort src = le.getSourcePort();
            SalPort dst = le.getDestinationPort();

            // Ensure that the link is not present in the topology.
            if (!containsEdge(le)) {
                SalNode srcNode = src.getSalNode();
                SalNode dstNode = dst.getSalNode();
                Pair<SalNode> ep = new Pair<SalNode>(srcNode, dstNode);

                if (addEdge(le, ep, EdgeType.DIRECTED)) {
                    LOG.trace("Edge added: {} -> {}", src, dst);
                    return true;
                }
            }
        } catch (Exception e) {
            LOG.error("Failed to add link to the topology: " + vlink, e);
        }

        return false;
    }

    /**
     * Remove the given link from the network topology.
     *
     * @param vlink  A {@link VtnLink} instance to be removed.
     * @return  {@code true} if the network topology was updated.
     *          {@code false} if the network topology was not changed.
     */
    private synchronized boolean removeLink(VtnLink vlink) {
        try {
            LinkEdge le = new LinkEdge(vlink);
            boolean updated = removeEdge(le);
            if (updated) {
                SalPort src = le.getSourcePort();
                SalPort dst = le.getDestinationPort();
                LOG.trace("Edge removed: {} -> {}", src, dst);
                removeNode(src.getSalNode());
                removeNode(dst.getSalNode());
            }

            return updated;
        } catch (Exception e) {
            LOG.error("Failed to remove link from the topology: " + vlink, e);
        }

        return false;
    }

    /**
     * Remove the given {@link SalNode} from the network topology graph
     * if it is not linked to any nodes.
     *
     * @param snode  A {@link SalNode} instance.
     */
    private synchronized void removeNode(SalNode snode) {
        Collection<LinkEdge> in = getInEdges(snode);
        if (in == null || in.size() == 0) {
            Collection<LinkEdge> out = getOutEdges(snode);
            if ((out == null || out.size() == 0) && removeVertex(snode)) {
                LOG.trace("Node removed: {}", snode);
            }
        }
    }
}
