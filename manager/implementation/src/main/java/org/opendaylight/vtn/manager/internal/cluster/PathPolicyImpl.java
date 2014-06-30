/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.ContainerConfig;
import org.opendaylight.vtn.manager.internal.MiscUtils;
import org.opendaylight.vtn.manager.internal.NodeUtils;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of path policy.
 *
 * <p>
 *   Although this interface is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class PathPolicyImpl implements Serializable, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2408724215085822312L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PathPolicyImpl.class);

    /**
     * Pseudo path policy identifier which indicates the system default
     * routing table.
     */
    public static final int  POLICY_DEFAULT = 0;

    /**
     * The identifier to be assigned to the policy ID.
     */
    public static final int  POLICY_ID = 1;

    /**
     * The identifier of this path policy.
     */
    private final int  policyId;

    /**
     * The default cost for unspecified link.
     */
    private long  defaultCost;

    /**
     * A map which keeps link costs configured to switch ports.
     */
    private Map<PortLocation, Long> portCosts;

    /**
     * Read write lock to synchronize this instance.
     */
    private transient ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

    /**
     * Construct a new instance.
     *
     * @param id      The identifier of the path policy.
     * @param policy  A {@link PathPolicy} instance.
     *                {@code null} cannot be specified.
     * @throws VTNException   An error occurred.
     */
    public PathPolicyImpl(int id, PathPolicy policy) throws VTNException {
        if (id != POLICY_ID) {
            String msg = "Invalid policy ID: " + id;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        policyId = id;
        setDefaultCostImpl(policy.getDefaultCost());
        portCosts = createPortCosts(policy.getPathCosts());
    }

    /**
     * Return the identifier of this path policy.
     *
     * @return  The identifier of this path policy.
     */
    public int getPolicyId() {
        return policyId;
    }

    /**
     * Return the default link cost configured in this path policy.
     *
     * @return  The default link cost configured in this path policy.
     */
    public long getDefaultCost() {
        // Return value without holding the lock.
        return defaultCost;
    }

    /**
     * Change the default link cost.
     *
     * @param cost  The default cost value to be set.
     * @return      The cost value previously configured is returned.
     *              {@code null} is returned if not changed.
     * @throws VTNException   An error occurred.
     */
    public Long setDefaultCost(long cost) throws VTNException {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (defaultCost == cost) {
                return null;
            }
            Long old = Long.valueOf(defaultCost);
            setDefaultCostImpl(cost);

            return old;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a {@link PathPolicy} instance which represents this path policy.
     *
     * @return  A {@link PathPolicy} instance.
     */
    public PathPolicy getPathPolicy() {
        Integer id = Integer.valueOf(policyId);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            List<PathCost> list = new ArrayList<PathCost>(portCosts.size());
            for (Map.Entry<PortLocation, Long> entry: portCosts.entrySet()) {
                PortLocation ploc = entry.getKey();
                Long cost = entry.getValue();
                list.add(new PathCost(ploc, cost.longValue()));
            }

            return new PathPolicy(id, defaultCost, list);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Modify the path policy as specified by a {@link PathPolicy} instance.
     *
     * <p>
     *   Note that the policy ID is never modified.
     * </p>
     *
     * @param policy  A {@link PathPolicy} instance.
     *                {@code null} cannot be specified.
     * @return  {@code true} if the policy was actually changed.
     *          {@code false} if the policy was not changed.
     * @throws VTNException   An error occurred.
     */
    public boolean setPathPolicy(PathPolicy policy) throws VTNException {
        boolean changed = false;
        long dc = policy.getDefaultCost();
        Map<PortLocation, Long> newCosts =
            createPortCosts(policy.getPathCosts());

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            if (dc != defaultCost) {
                setDefaultCostImpl(dc);
                changed = true;
            }
            if (!portCosts.equals(newCosts)) {
                portCosts = newCosts;
                changed = true;
            }

            return changed;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return the cost value associated with the specified port location
     * in this path policy.
     *
     * @param ploc  A {@link PortLocation} instance which represents the
     *              location of the switch port.
     * @return  The cost value associated with {@code ploc}.
     *          {@link PathPolicy#COST_UNDEF} is returned if not found.
     */
    public long getPathCost(PortLocation ploc) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            Long v = portCosts.get(ploc);
            return (v == null) ? PathPolicy.COST_UNDEF : v.longValue();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Associate the link cost value with the port location in this path
     * policy.
     *
     * @param ploc  A {@link PortLocation} instance which represents the
     *              location of the switch port.
     * @param cost  The link cost value.
     * @return  {@link UpdateType#ADDED} if the cost for {@code ploc} was
     *          newly added.
     *          {@link UpdateType#CHANGED} if the cost for {@code ploc} was
     *          changed.
     *          {@code null} if the cost was not changed.
     * @throws VTNException   An error occurred.
     */
    public UpdateType setPathCost(PortLocation ploc, long cost)
        throws VTNException {
        NodeUtils.checkPortLocation(ploc);
        checkCost(cost);
        Long c = Long.valueOf(cost);

        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            Long old = portCosts.put(ploc, c);
            UpdateType ret;
            if (old == null) {
                ret = UpdateType.ADDED;
            } else if (old.longValue() == cost) {
                ret = null;
            } else {
                ret = UpdateType.CHANGED;
            }

            return ret;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Remove the cost configuration for the specified port.
     *
     * @param ploc  A {@link PortLocation} instance which represents the
     *              location of the switch port.
     * @return  A {@link Long} instance which contains the cost value
     *          previously associated with the specified port is returned.
     *          {@code null} is returned if the specified port location was
     *          not found.
     * @throws VTNException   An error occurred.
     */
    public Long removePathCost(PortLocation ploc) throws VTNException {
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            return portCosts.remove(ploc);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Determine the cost of transmitting a packet from the specified port.
     *
     * @param mgr   VTN Manager service.
     * @param port  A {@link NodeConnector} instance corresponding to a
     *              physical switch port.
     * @return  A {@link Long} instance which represents the link cost.
     */
    public Long getCost(VTNManagerImpl mgr, NodeConnector port) {
        PortProperty prop = mgr.getPortProperty(port);
        if (prop == null) {
            // This should never happen.
            LOG.warn("{}.{}: Unknown port: {}",
                     mgr.getContainerName(), policyId, port);

            return Long.valueOf(Long.MAX_VALUE);
        }

        Node node = port.getNode();
        String portType = port.getType();
        String portId = port.getNodeConnectorIDString();
        String portName = prop.getName();
        SwitchPort[] swports;
        if (portName == null) {
            // Port name is unavailable. PortLocation instances which specify
            // the port name should not match.
            swports = new SwitchPort[] {
                new SwitchPort(portType, portId),

                null,
            };
        } else {
            swports = new SwitchPort[] {
                // Search for a PortLocation by port type, ID, and name.
                new SwitchPort(portName, portType, portId),

                // Search for a PortLocation by port type and ID.
                new SwitchPort(portType, portId),

                // Search for a PortLocation by port name.
                new SwitchPort(portName),

                // Seach for a PortLocation that does not contain a
                // SwitchPort instance.
                null,
            };
        }

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            for (SwitchPort swport: swports) {
                PortLocation ploc = new PortLocation(node, swport);
                Long cost = portCosts.get(ploc);
                if (cost != null) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}.{}: Path cost was found: port={}, " +
                                  "ploc={}, cost={}", mgr.getContainerName(),
                                  policyId, port, ploc, cost);
                    }
                    return cost;
                }
            }

            // Return the default.
            long c = defaultCost;
            if (c == PathPolicy.COST_UNDEF) {
                c = prop.getCost();
            }

            Long cost = Long.valueOf(c);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}.{}: Use default cost: port={}, cost={}",
                          mgr.getContainerName(), policyId, port, cost);
            }

            return cost;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Save the configuration of this path policy to the configuration file.
     *
     * @param mgr  VTN Manager service.
     * @return     "Success" or failure reason.
     */
    public Status saveConfig(VTNManagerImpl mgr) {
        Status status;
        String container = mgr.getContainerName();
        ContainerConfig cfg = new ContainerConfig(container);
        String name = Integer.toString(policyId);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            status = cfg.save(ContainerConfig.Type.PATHPOLICY, name, this);
            if (status.isSuccess()) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}.{}: Path policy was saved", container, name);
                }
                return status;
            }
        } finally {
            rdlock.unlock();
        }

        String msg = "Failed to save path policy configuration";
        LOG.error("{}.{}: {}: {}", container, name, msg, status);
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Destory this path policy.
     *
     * @param mgr  VTN Manager service.
     */
    public void destroy(VTNManagerImpl mgr) {
        String container = mgr.getContainerName();
        String name = Integer.toString(policyId);
        ContainerConfig cfg = new ContainerConfig(container);
        cfg.delete(ContainerConfig.Type.PATHPOLICY, name);
    }

    /**
     * Construct a flow port cost map from the specified list of
     * {@link PathCost} instances.
     *
     * @param list  A list of {@link PathCost} instances.
     * @return  A map which contains port costs.
     * @throws VTNException   An error occurred.
     */
    private Map<PortLocation, Long> createPortCosts(List<PathCost> list)
        throws VTNException {
        HashMap<PortLocation, Long> map = new HashMap<>();
        for (PathCost pc: list) {
            if (pc == null) {
                Status st = MiscUtils.argumentIsNull("PathCost");
                throw new VTNException(st);
            }

            PortLocation ploc = pc.getLocation();
            NodeUtils.checkPortLocation(ploc);

            long cost = pc.getCost();
            checkCost(cost);
            if (map.put(ploc, Long.valueOf(cost)) != null) {
                String msg = "Duplicate port location: " + ploc;
                throw new VTNException(StatusCode.BADREQUEST, msg);
            }
        }

        return map;
    }

    /**
     * Set the default cost of this path policy.
     *
     * <p>
     *   This method must be called with holding the writer lock.
     * </p>
     *
     * @param c  The cost value to be set.
     * @throws VTNException   An error occurred.
     */
    private void setDefaultCostImpl(long c) throws VTNException {
        if (c < PathPolicy.COST_UNDEF) {
            String msg = "Invalid default cost: " + c;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
        defaultCost = c;
    }

    /**
     * Ensure that the given link cost value is valid.
     *
     * @param cost  The link cost value to be tested.
     * @throws VTNException  The specified cost value is invalid.
     */
    private void checkCost(long cost) throws VTNException {
        if (cost <= PathPolicy.COST_UNDEF) {
            String msg = "Invalid cost value: " + cost;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }
    }

    /**
     * Read data from the given input stream and deserialize.
     *
     * @param in  An input stream.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @SuppressWarnings("unused")
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        // Reset the lock.
        rwLock = new ReentrantReadWriteLock();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            in.defaultReadObject();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Serialize this object and write it to the given output stream.
     *
     * @param out  An output stream.
     * @throws IOException
     *    An I/O error occurred.
     */
    @SuppressWarnings("unused")
    private void writeObject(ObjectOutputStream out)
        throws IOException {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            out.defaultWriteObject();
        } finally {
            rdlock.unlock();
        }
    }

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
        if (!(o instanceof PathPolicyImpl)) {
            return false;
        }

        PathPolicyImpl pp = (PathPolicyImpl)o;

        // Create a copy of portCosts in order to avoid deadlock.
        Lock rdlock = pp.rwLock.readLock();
        long otherDefault;
        Map<PortLocation, Long> otherPortCosts;
        rdlock.lock();
        try {
            otherDefault = pp.defaultCost;
            otherPortCosts = (Map<PortLocation, Long>)
                ((HashMap<PortLocation, Long>)portCosts).clone();
        } finally {
            rdlock.unlock();
        }

        rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return (policyId == pp.policyId && defaultCost == otherDefault &&
                    portCosts.equals(otherPortCosts));
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return MiscUtils.hashCode(defaultCost) + policyId * 31 +
                portCosts.hashCode() * 29;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PathPolicyImpl[id=");
        builder.append(policyId).append(",default=");

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            builder.append(defaultCost).append(",costs=").
                append(portCosts.toString());
        } finally {
            rdlock.unlock();
        }

        return builder.append(']').toString();
    }

    // Cloneable
    /**
     * Return a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public PathPolicyImpl clone() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            PathPolicyImpl pp = (PathPolicyImpl)super.clone();
            pp.rwLock = new ReentrantReadWriteLock();
            pp.portCosts = (Map<PortLocation, Long>)
                ((HashMap<PortLocation, Long>)portCosts).clone();
            return pp;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        } finally {
            rdlock.unlock();
        }
    }
}
