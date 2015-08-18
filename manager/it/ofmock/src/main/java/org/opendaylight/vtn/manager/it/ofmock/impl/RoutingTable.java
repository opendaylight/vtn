/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.uci.ics.jung.algorithms.shortestpath.DijkstraShortestPath;
import edu.uci.ics.jung.graph.SparseMultigraph;
import edu.uci.ics.jung.graph.util.EdgeType;
import edu.uci.ics.jung.graph.util.Pair;

import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.controller.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.NotificationListener;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.RoutingUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopologyListener;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.AddedLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.routing.updated.RemovedLink;

/**
 * {@code RoutingTable} describes the packet routing table maintained by the
 * VTN Manager.
 */
public final class RoutingTable extends SparseMultigraph<String, OfMockLink>
    implements AutoCloseable, VtnTopologyListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(RoutingTable.class);

    /**
     * Shortest path resolver.
     */
    private final transient DijkstraShortestPath<String, OfMockLink>  resolver;

    /**
     * Registration of the notification listener.
     */
    private final AtomicReference<ListenerRegistration<NotificationListener>> registration =
        new AtomicReference<>();

    /**
     * Construct a new instance.
     *
     * @param nsv  A {@link NotificationService} service instance.
     */
    public RoutingTable(NotificationService nsv) {
        try {
            registration.set(nsv.registerNotificationListener(this));
        } catch (Exception e) {
            String msg = "Failed to register VTN topology listener.";
            LOG.error(msg, e);
            throw new IllegalStateException(msg, e);
        }

        resolver = new DijkstraShortestPath<String, OfMockLink>(this);
    }

    /**
     * Return the packet route from the source to the destination switch.
     *
     * @param src  A MD-SAL node identifier corresponding to the source
     *             switch.
     * @param dst  A {MD-SAL node identifier corresponding to the destination
     *             switch.
     * @return     A list of {@link OfMockLink} instances which represents the
     *             packet route.
     *             An empty list is returned if the destination node is the
     *             same as the source.
     *             {@code null} is returned if no route was found.
     */
    public List<OfMockLink> getRoute(String src, String dst) {
        if (src.equals(dst)) {
            return Collections.<OfMockLink>emptyList();
        }

        synchronized (this) {
            try {
                return resolver.getPath(src, dst);
            } catch (Exception e) {
                LOG.trace("A vertex is not yet known: {} -> {}", src, dst);
            }
        }

        return null;
    }

    /**
     * Determine whether the given inter-switch link is present in the
     * packet routing table or not.
     *
     * @param src    The source port identifier of the inter-switch link.
     * @param dst    The destination port identifier of the inter-switch link.
     *               {@code null} matches any ports.
     * @return  {@code true} if the given link is present.
     *          {@code false} if not present.
     */
    public synchronized boolean hasLink(String src, String dst) {
        OfMockLink link = new OfMockLink(src, dst);
        return containsEdge(link);
    }

    /**
     * Wait for the given inter-switch link to be created or removed.
     *
     * @param src    The source port identifier of the inter-switch link.
     * @param dst    The destination port identifier of the inter-switch link.
     *               {@code null} matches any ports.
     * @param state  If {@code true}, this method waits for the specified
     *               link to be created.
     *               If {@code false}, this method waits for the specified
     *               link to be removed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws IllegalStateException
     *    The specified link did not created or removed.
     */
    public void awaitLink(String src, String dst, boolean state)
        throws InterruptedException {
        if (!awaitLinkImpl(src, dst, state, OfMockProvider.TASK_TIMEOUT)) {
            StringBuilder builder = new StringBuilder("The link was not ");
            if (state) {
                builder.append("created");
            } else {
                builder.append("removed");
            }
            builder.append(": ").append(src).append(" -> ").append(dst);
            String msg = builder.toString();
            LOG.error(msg);
            throw new IllegalStateException(msg);
        }
    }

    /**
     * Wait for the given inter-switch link to be created.
     *
     * @param src      The source port identifier of the inter-switch link.
     * @param dst      The destination port identifier of the inter-switch
     *                 link.
     *                 {@code null} matches any ports.
     * @param timeout  The number of milliseconds to wait.
     * @return  {@code true} if the given inter-switch link has been created.
     *          Otherwise {@code false}.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws IllegalStateException
     *    The specified link did not created or removed.
     */
    public boolean awaitLinkUp(String src, String dst, long timeout)
        throws InterruptedException {
        return awaitLinkImpl(src, dst, true, timeout);
    }

    /**
     * Add the given inter-switch links to the packet routing table.
     *
     * @param added  A list of {@link AddedLink} instances.
     * @return  {@code true} only if the routing table was updated.
     */
    private synchronized boolean addLinks(List<AddedLink> added) {
        if (added == null) {
            return false;
        }

        boolean updated = false;
        for (AddedLink al: added) {
            String src = OfMockUtils.getPortIdentifier(al.getSource());
            if (src == null) {
                continue;
            }
            String dst = OfMockUtils.getPortIdentifier(al.getDestination());
            if (dst == null) {
                continue;
            }

            if (addLink(src, dst)) {
                updated = true;
            }
        }

        return updated;
    }

    /**
     * Add the given inter-switch link to the packet routing table.
     *
     * @param src    The source port identifier of the inter-switch link.
     * @param dst    The destination port identifier of the inter-switch link.
     * @return  {@code true} only if the routing table was updated.
     */
    private synchronized boolean addLink(String src, String dst) {
        try {
            // Ensure that the link is not present.
            OfMockLink link = new OfMockLink(src, dst);
            boolean updated = !containsEdge(link);
            if (updated) {
                String srcNode = OfMockUtils.getNodeIdentifier(src);
                String dstNode = OfMockUtils.getNodeIdentifier(dst);
                Pair<String> ep = new Pair<String>(srcNode, dstNode);

                if (addEdge(link, ep, EdgeType.DIRECTED)) {
                    LOG.debug("Inter-switch link has been added: {} -> {}",
                              src, dst);
                    return true;
                }
            }
        } catch (Exception e) {
            String msg = "Failed to add inter-switch link: " + src +
                " -> " + dst;
            LOG.error(msg, e);
        }

        return false;
    }

    /**
     * Remove the given inter-switch links from the packet routing table.
     *
     * @param removed  A list of {@link RemovedLink} instances.
     * @return  {@code true} only if the routing table was updated.
     */
    private synchronized boolean removeLinks(List<RemovedLink> removed) {
        if (removed == null) {
            return false;
        }

        boolean updated = false;
        for (RemovedLink rl: removed) {
            String src = OfMockUtils.getPortIdentifier(rl.getSource());
            if (src == null) {
                continue;
            }
            String dst = OfMockUtils.getPortIdentifier(rl.getDestination());
            if (dst == null) {
                continue;
            }

            if (removeLink(src, dst)) {
                updated = true;
            }
        }

        return updated;
    }

    /**
     * Remove the given inter-switch links from the packet routing table.
     *
     * @param src    The source port identifier of the inter-switch link.
     * @param dst    The destination port identifier of the inter-switch link.
     * @return  {@code true} only if the routing table was updated.
     */
    private synchronized boolean removeLink(String src, String dst) {
        try {
            OfMockLink link = new OfMockLink(src, dst);
            boolean updated = removeEdge(link);
            if (updated) {
                LOG.debug("Inter-switch link has been removed: {} -> {}",
                          src, dst);
                String srcNode = OfMockUtils.getNodeIdentifier(src);
                String dstNode = OfMockUtils.getNodeIdentifier(dst);
                removeNode(srcNode);
                removeNode(dstNode);
            }

            return updated;
        } catch (Exception e) {
            String msg = "Failed to remove inter-switch link: " + src +
                " -> " + dst;
            LOG.error(msg, e);
        }

        return false;
    }

    /**
     * Remove the given node from the network topology graph if it is not
     * linked to any nodes.
     *
     * @param nid  A node identifier.
     */
    private synchronized void removeNode(String nid) {
        Collection<OfMockLink> in = getInEdges(nid);
        if (in == null || in.size() == 0) {
            Collection<OfMockLink> out = getOutEdges(nid);
            if (out == null || out.size() == 0) {
                if (removeVertex(nid)) {
                    LOG.debug("Node removed from the topology graph: {}", nid);
                }
            }
        }
    }

    /**
     * Wait for the given inter-switch link to be created or removed.
     *
     * @param src      The source port identifier of the inter-switch link.
     * @param dst      The destination port identifier of the inter-switch
     *                 link.
     *                 {@code null} matches any ports.
     * @param state    If {@code true}, this method waits for the specified
     *                 link to be created.
     *                 If {@code false}, this method waits for the specified
     *                 link to be removed.
     * @param timeout  The number of milliseconds to wait.
     * @return  {@code true} if the given link has been created or removed.
     *          {@code false} if the given link did not created or removed.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private synchronized boolean awaitLinkImpl(String src, String dst,
                                               boolean state, long timeout)
        throws InterruptedException {
        OfMockLink link = new OfMockLink(src, dst);
        if (containsEdge(link) == state) {
            return true;
        }

        long tm = timeout;
        long deadline = System.currentTimeMillis() + timeout;
        do {
            wait(tm);
            if (containsEdge(link) == state) {
                return true;
            }
            tm = deadline - System.currentTimeMillis();
        } while (tm > 0);

        return (containsEdge(link) == state);
    }

    // AutoCloseable

    /**
     * Close the VTN topology listener.
     */
    @Override
    public void close() {
        ListenerRegistration<NotificationListener> reg =
            registration.getAndSet(null);
        if (reg != null) {
            try {
                reg.close();
            } catch (Exception e) {
                String msg = "Failed to unregister VTN topology listener.";
                LOG.error(msg, e);
            }
        }
    }

    // VtnTopologyListener

    /**
     * Invoked when the packet routing table of the VTN Manager has been
     * updated.
     *
     * @param notification  A {@link RoutingUpdated} instance.
     */
    @Override
    public void onRoutingUpdated(RoutingUpdated notification) {
        LOG.trace("onRoutingUpdated: {}", notification);
        if (notification == null) {
            return;
        }

        synchronized (this) {
            boolean updated = addLinks(notification.getAddedLink());
            if (removeLinks(notification.getRemovedLink())) {
                updated = true;
            }

            if (updated) {
                resolver.reset();
                notifyAll();
            }
        }
    }
}
