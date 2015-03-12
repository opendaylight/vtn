/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.math.BigInteger;
import java.util.Dictionary;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceRegistration;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.IPluginInFlowProgrammerService;
import org.opendaylight.controller.sal.inventory.IListenInventoryUpdates;
import org.opendaylight.controller.sal.inventory.IPluginInInventoryService;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;

/**
 * AD-SAL inventory management.
 */
public final class AdSalInventory implements IInventoryListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(AdSalInventory.class);

    /**
     * The number of milliseconds to wait for AD-SAL inventory to be
     * synchronized.
     */
    private static final long  SYNC_TIMEOUT = 30000L;

    /**
     * The number of milliseconds between polls.
     */
    private static final long  POLL_INTERVAL = 500L;

    /**
     * AD-SAL connection manager service.
     */
    private final IConnectionManager  connectionManager;

    /**
     * An OSGi bundle context for this OSGi bundle.
     */
    private final BundleContext  bundleContext;

    /**
     * A set of AD-SAL nodes detected by the switch manager.
     */
    private final Set<Node>  nodeSet = new HashSet<>();

    /**
     * A set of AD-SAL node connectors detected by the switch manager.
     */
    private final Set<NodeConnector>  portSet = new HashSet<>();

    /**
     * OSGi service registration for {@link IInventoryListener}.
     */
    private final AtomicReference<ServiceRegistration<?>> inventoryListener =
        new AtomicReference<>();

    /**
     * Convert the given MD-SAL node identifier into AD-SAL node.
     *
     * @param nid  The MD-SAL node identifier.
     * @return  A {@link Node} instance.
     */
    public static Node toAdNode(String nid) {
        int idx = nid.lastIndexOf(':');
        if (idx > 0) {
            BigInteger bi = new BigInteger(nid.substring(idx + 1));
            Long dpid = Long.valueOf(bi.longValue());
            Node node = NodeCreator.createOFNode(dpid);
            if (node != null) {
                return node;
            }
        }

        throw new IllegalArgumentException("Invalid node identifier: " + nid);
    }

    /**
     * Convert the given MD-SAL node connector identifier into AD-SAL
     * node connector.
     *
     * @param pid  The MD-SAL node connector identifier.
     * @return  A {@link Node} instance.
     */
    public static NodeConnector toAdNodeConnector(String pid) {
        int idx = pid.lastIndexOf(':');
        if (idx > 0) {
            String nid = pid.substring(0, idx);
            Node node = toAdNode(nid);
            Long l = Long.valueOf(pid.substring(idx + 1));
            NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(
                Short.valueOf(l.shortValue()), node);
            if (nc != null) {
                return nc;
            }
        }

        throw new IllegalArgumentException(
            "Invalid node connector identifier: " + pid);
    }

    /**
     * Construct a new instance.
     *
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public AdSalInventory() throws InterruptedException {
        Bundle bundle = FrameworkUtil.getBundle(AdSalInventory.class);
        if (bundle == null) {
            // This should never happen.
            throw new IllegalStateException("Failed to get OSGi bundle.");
        }

        BundleContext bc = bundle.getBundleContext();
        if (bc == null) {
            throw new IllegalStateException(
                "Failed to get OSGi bundle context.");
        }
        bundleContext = bc;

        // Wait for mandatory OSGi services to be registered.
        String container = GlobalConstants.DEFAULT.toString();
        new ServiceWaiter<ISwitchManager>(bc, ISwitchManager.class, container).
            await();
        connectionManager = new ServiceWaiter<IConnectionManager>(
            bc, IConnectionManager.class).await();

        // Register AD-SAL inventory listener.
        Dictionary<String, Object> props = new Hashtable<>();
        props.put(ServiceWaiter.PROP_CONTAINER,
                  GlobalConstants.DEFAULT.toString());

        try {
            ServiceRegistration<?> reg =
                bundleContext.registerService(IInventoryListener.class, this,
                                              props);
            inventoryListener.set(reg);
        } catch (RuntimeException e) {
            throw new IllegalStateException(
                "Failed to register IInventoryListener.", e);
        }

        // Wait for sal-compatibility services to be registered.
        new ServiceWaiter<IPluginInInventoryService>(
            bc, IPluginInInventoryService.class, container).await();
        new ServiceWaiter<IPluginInFlowProgrammerService>(
            bc, IPluginInFlowProgrammerService.class).await();

        LOG.debug("AD-SAL inventory management has been initialized.");
    }

    /**
     * Wait for the AD-SAL switch manager to detect the given node.
     *
     * @param nid  The MD-SAL node identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void awaitNode(String nid)
        throws InterruptedException {
        Node node = toAdNode(nid);
        long timeout = SYNC_TIMEOUT;
        long deadline = System.currentTimeMillis() + timeout;
        synchronized (this) {
            if (!nodeSet.contains(node)) {
                LOG.trace("Waiting for AD-SAL node to be created: {}", nid);
                awaitNode(node, timeout);
                LOG.trace("AD-SAL node has been created: {}", nid);
            }
        }

        ConnectionLocality cl = connectionManager.getLocalityStatus(node);
        if (cl == ConnectionLocality.LOCAL) {
            return;
        }

        timeout = deadline - System.currentTimeMillis();
        if (timeout > 0) {
            LOG.trace("Waiting for AD-SAL node to be connected: {}", nid);
            do {
                Thread.sleep(POLL_INTERVAL);
                cl = connectionManager.getLocalityStatus(node);
                if (cl == ConnectionLocality.LOCAL) {
                    LOG.trace("AD-SAL node has been connected: {}", nid);
                    return;
                }
                timeout = deadline - System.currentTimeMillis();
            } while (timeout > 0);
        }

        StringBuilder builder = new StringBuilder(
            "AD-SAL node was not detected by the connection manager: ");
        String msg = builder.append(node).append(", ").append(cl).
            toString();
        LOG.error(msg);
        throw new IllegalStateException(msg);
    }

    /**
     * Wait for the AD-SAL switch manager to detect the given port.
     *
     * @param pid  The MD-SAL port identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public synchronized void awaitPort(String pid)
        throws InterruptedException {
        NodeConnector nc = toAdNodeConnector(pid);

        if (!portSet.contains(nc)) {
            LOG.trace("Waiting for AD-SAL port to be created: {}", pid);

            long timeout = SYNC_TIMEOUT;
            long deadline = System.currentTimeMillis() + timeout;
            do {
                wait(timeout);
                if (portSet.contains(nc)) {
                    LOG.trace("AD-SAL port has been created: {}", pid);
                    return;
                }
                timeout = deadline - System.currentTimeMillis();
            } while (timeout > 0);

            String msg = "AD-SAL node connector was not created: " + pid;
            LOG.error(msg);
            throw new IllegalStateException(msg);
        }
    }

    /**
     * Stop the service.
     */
    public void close() {
        ServiceRegistration<?> reg = inventoryListener.getAndSet(null);
        if (reg != null) {
            try {
                reg.unregister();
            } catch (RuntimeException e) {
                LOG.warn("Failed to unregister IInventoryListener.", e);
            }
        }
    }

    /**
     * Wait for the given AD-SAL node to be created.
     *
     * @param node     A {@link Node} instance.
     * @param timeout  The number of milliseconds to wait.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private synchronized void awaitNode(Node node, long timeout)
        throws InterruptedException {
        long tm = timeout;
        long deadline = System.currentTimeMillis() + timeout;
        do {
            wait(tm);
            if (nodeSet.contains(node)) {
                return;
            }
            tm = deadline - System.currentTimeMillis();
        } while (tm > 0);

        String msg = "AD-SAL node was not created: " + node;
        LOG.error(msg);
        throw new IllegalStateException(msg);
    }

    // IInventoryListener

    /**
     * Invoked when the AD-SAL switch manager detects change of a node.
     *
     * @param node     A {@link Node} instance.
     * @param type     The type of event.
     * @param propMap  A property map.
     */
    @Override
    public synchronized void notifyNode(Node node, UpdateType type,
                                        Map<String, Property> propMap) {
        LOG.trace("AD-SAL node has been {}: {}, prop={}", type.getName(),
                  node, propMap);
        boolean changed;
        if (type == UpdateType.REMOVED) {
            changed = nodeSet.remove(node);
            if (changed) {
                // Remove all node connectors associated with the given node.
                for (Iterator<NodeConnector> it = portSet.iterator();
                     it.hasNext();) {
                    NodeConnector nc = it.next();
                    if (node.equals(nc.getNode())) {
                        it.remove();
                    }
                }
            }
        } else {
            changed = nodeSet.add(node);
            if (changed && type != UpdateType.ADDED &&
                connectionManager instanceof IListenInventoryUpdates) {
                // sal-compatibility may notify CHANGED event instead of ADDED
                // event when a new node is detected. But AD-SAL connection
                // manager always ignores CHANGED events for node.
                // So we need to propagate ADDED event.
                IListenInventoryUpdates listener =
                    (IListenInventoryUpdates)connectionManager;
                Set<Property> props = new HashSet<>(propMap.values());
                LOG.debug("Notifying the AD-SAL connection manager of " +
                          "node ADDED event: node={}, props={}", node, props);
                listener.updateNode(node, UpdateType.ADDED, props);
            }
        }

        if (changed) {
            notifyAll();
        }
    }

    /**
     * Invoked when the AD-SAL switch manager detects change of a node
     * connector.
     *
     * @param nc       A {@link NodeConnector} instance.
     * @param type     The type of event.
     * @param propMap  A property map.
     */
    @Override
    public synchronized void notifyNodeConnector(
        NodeConnector nc, UpdateType type, Map<String, Property> propMap) {
        LOG.trace("AD-SAL node connector has been {}: {}, prop={}",
                  type.getName(), nc, propMap);
        boolean changed;
        if (type == UpdateType.REMOVED) {
            changed = portSet.remove(nc);
        } else {
            changed = portSet.add(nc);
        }

        if (changed) {
            notifyAll();
        }
    }
}
