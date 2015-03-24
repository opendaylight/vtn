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
import java.util.Collections;
import java.util.Dictionary;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.FrameworkUtil;
import org.osgi.framework.ServiceRegistration;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;

import org.opendaylight.controller.connectionmanager.IConnectionManager;
import org.opendaylight.controller.sal.connection.ConnectionLocality;
import org.opendaylight.controller.sal.core.AdvertisedBandwidth;
import org.opendaylight.controller.sal.core.Bandwidth;
import org.opendaylight.controller.sal.core.Buffers;
import org.opendaylight.controller.sal.core.Capabilities;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.MacAddress;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.PeerBandwidth;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.SupportedBandwidth;
import org.opendaylight.controller.sal.core.Tables;
import org.opendaylight.controller.sal.core.TimeStamp;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.IPluginInFlowProgrammerService;
import org.opendaylight.controller.sal.inventory.IListenInventoryUpdates;
import org.opendaylight.controller.sal.inventory.IPluginInInventoryService;
import org.opendaylight.controller.sal.inventory.IPluginOutInventoryService;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FeatureCapability;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowFeatureCapabilityFlowStats;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.flow.node.SwitchFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.FlowCapablePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.State;

/**
 * AD-SAL inventory management.
 */
public final class AdSalInventory
    implements IInventoryListener, AutoCloseable {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(AdSalInventory.class);

    /**
     * The number of bytes in a MAC address.
     */
    private static final int  MAC_ADDRESS_LENGTH = 6;

    /**
     * The number of milliseconds to wait for AD-SAL inventory to be
     * synchronized.
     */
    private static final long  SYNC_TIMEOUT = 30000L;

    /**
     * The number of milliseconds to wait for sal-compatibility to update
     * the AD-SAL inventory.
     */
    private static final long  COMPAT_TIMEOUT = 2000L;

    /**
     * The number of milliseconds between polls.
     */
    private static final long  POLL_INTERVAL = 500L;

    /**
     * AD-SAL switch manager service.
     */
    private final ISwitchManager  switchManager;

    /**
     * AD-SAL connection manager service.
     */
    private final IConnectionManager  connectionManager;

    /**
     * AD-SAL inventory output service.
     */
    private final IPluginOutInventoryService  inventoryOutGlobal;

    /**
     * AD-SAL inventory output service for the default container.
     */
    private final IPluginOutInventoryService  inventoryOut;

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
     * A set of MD-SAL listeners.
     */
    private final AtomicReference<Set<DataStoreListener<?, ?>>> dataListeners =
        new AtomicReference<>();

    /**
     * A map that keeps current AD-SAL nodes.
     */
    private final Map<Node, Set<Property>>  adNodes = new Hashtable<>();

    /**
     * A map that keeps current AD-SAL node connectors.
     */
    private final Map<NodeConnector, Set<Property>> adNodeConnectors =
        new Hashtable<>();

    /**
     * Timer thread used to maintain AD-SAL inventory.
     */
    private final AtomicReference<Timer>  inventoryTimer =
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
     * Convert the given MD-SAL flow-node into a set of AD-SAL node properties.

     * @param node  A {@link Node} instance.
     * @param fn    A {@link FlowNode} instance.
     * @return  A set of AD-SAL node properties.
     */
    public static Set<Property> toProperties(Node node, FlowNode fn) {
        Set<Property> props = new HashSet<>();
        Object id = node.getID();
        long dpid = (id instanceof Number) ? ((Number)id).longValue() : 0;
        byte[] mac = new byte[MAC_ADDRESS_LENGTH];
        for (int i = 1; i <= mac.length; i++) {
            mac[mac.length - i] = (byte)dpid;
            dpid >>>= Byte.SIZE;
        }
        props.add(new MacAddress(mac));
        props.add(new TimeStamp(System.currentTimeMillis(), "connectedSince"));

        SwitchFeatures swf = fn.getSwitchFeatures();
        if (swf != null) {
            Short tables = swf.getMaxTables();
            if (tables != null) {
                props.add(new Tables(tables.byteValue()));
            }

            Long buffers = swf.getMaxBuffers();
            if (buffers != null) {
                props.add(new Buffers(buffers.intValue()));
            }

            setNodeCapabilities(props, swf.getCapabilities());
        }

        return Collections.unmodifiableSet(props);
    }

    /**
     * Convert the given MD-SAL flow-capable-port into a set of AD-SAL node
     * connector properties.
     *
     * @param fcp  A {@link FlowCapablePort} instance.
     * @return  A set of AD-SAL node connector properties.
     */
    public static Set<Property> toProperties(FlowCapablePort fcp) {
        Set<Property> props = toBandwidthProps(fcp);
        String name = fcp.getName();
        if (name != null) {
            props.add(new Name(name));
        }

        PortConfig pc = fcp.getConfiguration();
        if (pc != null) {
            short cfg = (pc.isPORTDOWN())
                ? Config.ADMIN_DOWN : Config.ADMIN_UP;
            props.add(new Config(cfg));
        }

        State state = fcp.getState();
        if (state != null) {
            short st = (state.isLinkDown())
                ? org.opendaylight.controller.sal.core.State.EDGE_DOWN
                : org.opendaylight.controller.sal.core.State.EDGE_UP;
            props.add(new org.opendaylight.controller.sal.core.State(st));
        }

        return Collections.unmodifiableSet(props);
    }

    /**
     * Set AD-SAL node properties from the given capability list.
     *
     * @param props  A set of AD-SAL node properties.
     * @param fclist A list of MD-SAL node capability classes.
     */
    public static void setNodeCapabilities(
        Set<Property> props, List<Class<? extends FeatureCapability>> fclist) {
        if (fclist != null) {
            int bits = 0;
            for (Class<? extends FeatureCapability> fc: fclist) {
                if (fc.equals(FlowFeatureCapabilityFlowStats.class)) {
                    bits |= Capabilities.CapabilitiesType.
                        FLOW_STATS_CAPABILITY.getValue();
                }
            }

            props.add(new Capabilities(bits));
        }
    }

    /**
     * Create AD-SAL node connector properties related to bandwidth.
     *
     * @param fcp    A {@link FlowCapablePort} instance.
     * @return  A set of AD-SAL node connector properties.
     */
    public static Set<Property> toBandwidthProps(FlowCapablePort fcp) {
        Set<Property> props = new HashSet<>();
        PortFeatures pf = fcp.getCurrentFeature();
        if (pf != null) {
            long bw = toBandwidth(pf);
            if (bw != Bandwidth.BWUNK) {
                props.add(new Bandwidth(bw));
            }
        }

        pf = fcp.getAdvertisedFeatures();
        if (pf != null) {
            long bw = toBandwidth(pf);
            if (bw != Bandwidth.BWUNK) {
                props.add(new AdvertisedBandwidth(bw));
            }
        }

        pf = fcp.getSupported();
        if (pf != null) {
            long bw = toBandwidth(pf);
            if (bw != Bandwidth.BWUNK) {
                props.add(new SupportedBandwidth(bw));
            }
        }

        pf = fcp.getPeerFeatures();
        if (pf != null) {
            long bw = toBandwidth(pf);
            if (bw != Bandwidth.BWUNK) {
                props.add(new PeerBandwidth(bw));
            }
        }

        return props;
    }

    /**
     * Convert the given MD-SAL port features into AD-SAL bandwidth value.
     *
     * @param pf  A {@link PortFeatures} instance.
     * @return  An AD-SAL bandwidth value.
     */
    public static long toBandwidth(PortFeatures pf) {
        if (pf.isTenMbHd() || pf.isTenMbFd()) {
            return Bandwidth.BW10Mbps;
        }
        if (pf.isHundredMbHd() || pf.isHundredMbFd()) {
            return Bandwidth.BW100Mbps;
        }
        if (pf.isOneGbHd() || pf.isOneGbFd()) {
            return Bandwidth.BW1Gbps;
        }
        if (pf.isOneGbFd()) {
            return Bandwidth.BW10Gbps;
        }
        if (pf.isTenGbFd()) {
            return Bandwidth.BW10Gbps;
        }
        if (pf.isFortyGbFd()) {
            return Bandwidth.BW40Gbps;
        }
        if (pf.isHundredGbFd()) {
            return Bandwidth.BW100Gbps;
        }
        if (pf.isOneTbFd()) {
            return Bandwidth.BW1Tbps;
        }

        return Bandwidth.BWUNK;
    }

    /**
     * Construct a new instance.
     *
     * @param broker  Data broker service.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public AdSalInventory(DataBroker broker) throws InterruptedException {
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
        switchManager = new ServiceWaiter<ISwitchManager>(
            bc, ISwitchManager.class, container).await();
        connectionManager = new ServiceWaiter<IConnectionManager>(
            bc, IConnectionManager.class).
            setFilter(ServiceWaiter.PROP_SCOPE, ServiceWaiter.SCOPE_GLOBAL).
            await();
        inventoryOut = new ServiceWaiter<IPluginOutInventoryService>(
            bc, IPluginOutInventoryService.class, container).await();
        inventoryOutGlobal = new ServiceWaiter<IPluginOutInventoryService>(
            bc, IPluginOutInventoryService.class).
            setFilter(ServiceWaiter.PROP_SCOPE, ServiceWaiter.SCOPE_GLOBAL).
            await();

        inventoryTimer.set(new Timer("AD-SAL inventory timer"));

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
        new ServiceWaiter<IPluginInInventoryService>(
            bc, IPluginInInventoryService.class).await();
        new ServiceWaiter<IPluginInFlowProgrammerService>(
            bc, IPluginInFlowProgrammerService.class).await();

        // Register MD-SAL inventory listeners.
        Set<DataStoreListener<?, ?>> mdSet = new HashSet<>();
        dataListeners.set(mdSet);
        mdSet.add(new MdNodeListener(broker, this));
        mdSet.add(new MdPortListener(broker, this));

        LOG.debug("AD-SAL inventory management has been initialized.");
    }

    /**
     * Notify that the given node has been updated.
     *
     * @param nid   The MD-SAL node identifier.
     * @param fn    A {@link FlowNode} instance.
     * @param type  An {@link UpdateType} instance which indicates the type of
     *              the event.
     */
    public void notifyNodeUpdated(String nid, FlowNode fn, UpdateType type) {
        final Node node = toAdNode(nid);
        Set<Property> props = toProperties(node, fn);
        adNodes.put(node, props);

        if (type == UpdateType.ADDED) {
            Timer timer = inventoryTimer.get();
            if (timer != null) {
                TimerTask task = new TimerTask() {
                    @Override
                    public void run() {
                        publishNodeAdded(node);
                    }
                };
                timer.schedule(task, COMPAT_TIMEOUT);
            }
        } else {
            inventoryOutGlobal.updateNode(node, type, props);
            inventoryOut.updateNode(node, type, props);
        }
    }

    /**
     * Notify that the given node has been removed.
     *
     * @param nid  The MD-SAL node identifier.
     */
    public void notifyNodeRemoved(String nid) {
        Node node = toAdNode(nid);
        adNodes.remove(node);

        Set<Property> props = Collections.<Property>emptySet();
        UpdateType type = UpdateType.REMOVED;
        inventoryOutGlobal.updateNode(node, type, props);
        inventoryOut.updateNode(node, type, props);
    }

    /**
     * Notify that the given node connector has been updated.
     *
     * @param pid   The MD-SAL port identifier.
     * @param fcp   A {@link FlowCapablePort} instance.
     * @param type  An {@link UpdateType} instance which indicates the type of
     *              the event.
     */
    public void notifyPortUpdated(String pid, FlowCapablePort fcp,
                                  UpdateType type) {
        final NodeConnector nc = toAdNodeConnector(pid);
        Set<Property> props = toProperties(fcp);
        adNodeConnectors.put(nc, props);

        if (type == UpdateType.ADDED) {
            Timer timer = inventoryTimer.get();
            if (timer != null) {
                TimerTask task = new TimerTask() {
                    @Override
                    public void run() {
                        publishPortAdded(nc);
                    }
                };
                timer.schedule(task, COMPAT_TIMEOUT);
            }
        } else {
            inventoryOutGlobal.updateNodeConnector(nc, type, props);
            inventoryOut.updateNodeConnector(nc, type, props);
        }
    }

    /**
     * Notify that the given node connector has been removed.
     *
     * @param pid  The MD-SAL node connector identifier.
     */
    public void notifyPortRemoved(String pid) {
        NodeConnector nc = toAdNodeConnector(pid);
        adNodeConnectors.remove(nc);

        Set<Property> props = Collections.<Property>emptySet();
        UpdateType type = UpdateType.REMOVED;
        inventoryOutGlobal.updateNodeConnector(nc, type, props);
        inventoryOut.updateNodeConnector(nc, type, props);
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

    /**
     * Publish AD-SAL node creation event by force.
     *
     * @param node  A {@link Node} instance.
     */
    private void publishNodeAdded(Node node) {
        Set<Property> props = adNodes.get(node);
        if (props != null && !switchManager.getNodes().contains(node)) {
            LOG.warn("Notifying AD-SAL node creation: {}", node);
            UpdateType type = UpdateType.ADDED;
            inventoryOutGlobal.updateNode(node, type, props);
            inventoryOut.updateNode(node, type, props);
        }
    }

    /**
     * Publish AD-SAL node connector creation event by force.
     *
     * @param nc  A {@link NodeConnector} instance.
     */
    private void publishPortAdded(NodeConnector nc) {
        Set<Property> props = adNodeConnectors.get(nc);
        if (props == null) {
            // The target node connector is already removed.
            return;
        }

        Node node = nc.getNode();
        Set<NodeConnector> ncSet = switchManager.getNodeConnectors(node);
        if (ncSet == null || !ncSet.contains(nc)) {
            LOG.warn("Notifying AD-SAL node connector creation: {}", nc);
            UpdateType type = UpdateType.ADDED;
            inventoryOutGlobal.updateNodeConnector(nc, type, props);
            inventoryOut.updateNodeConnector(nc, type, props);
        }
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

    // AutoCloseable

    /**
     * Close this instance.
     */
    @Override
    public void close() {
        Timer timer = inventoryTimer.getAndSet(null);
        if (timer != null) {
            timer.cancel();
        }

        ServiceRegistration<?> reg = inventoryListener.getAndSet(null);
        if (reg != null) {
            try {
                reg.unregister();
            } catch (RuntimeException e) {
                LOG.debug("Failed to unregister IInventoryListener.", e);
            }
        }

        Set<DataStoreListener<?, ?>> mdSet = dataListeners.getAndSet(null);
        if (mdSet != null) {
            for (DataStoreListener<?, ?> dsl: mdSet) {
                dsl.close();
            }
        }
    }
}
