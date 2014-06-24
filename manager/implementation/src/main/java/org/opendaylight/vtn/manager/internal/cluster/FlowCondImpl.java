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
import java.util.List;
import java.util.NavigableMap;
import java.util.TreeMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.internal.ContainerConfig;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of flow condition.
 *
 * <p>
 *   Although this interface is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class FlowCondImpl implements Serializable, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -156762956849289435L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(FlowCondImpl.class);

    /**
     * The name of the flow condition.
     */
    private final String  name;

    /**
     * A list of {@link FlowMatchImpl} instances sorted by match index.
     */
    private NavigableMap<Integer, FlowMatchImpl>  matches;

    /**
     * Read write lock to synchronize {@link #matches}.
     */
    private transient ReentrantReadWriteLock  rwLock =
        new ReentrantReadWriteLock();

    /**
     * Construct a new instance.
     *
     * @param name   The name of the flow condition.
     * @param fcond  A {@link FlowCondition} instance.
     * @throws VTNException   An error occurred.
     */
    public FlowCondImpl(String name, FlowCondition fcond) throws VTNException {
        this.name = name;

        List<FlowMatch> list = (fcond == null) ? null : fcond.getMatches();
        this.matches = createMatches(list);
    }

    /**
     * Return the name of the flow condition.
     *
     * @return  The name of the flow condition.
     */
    public String getName() {
        return name;
    }

    /**
     * Return a {@link FlowCondition} instance which represents this
     * flow condition.
     *
     * @return  A {@link FlowCondition} instance.
     */
    public FlowCondition getFlowCondition() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            List<FlowMatch> list = new ArrayList<FlowMatch>(matches.size());
            for (FlowMatchImpl fc: matches.values()) {
                list.add(fc.getMatch());
            }

            return new FlowCondition(name, list);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Change the list of {@link FlowMatch} instances as specified.
     *
     * @param list  A list of {@link FlowMatch} instances.
     * @return  {@code true} if the list was actually changed.
     *          {@code false} if the list was not changed.
     * @throws VTNException   An error occurred.
     */
    public boolean setMatches(List<FlowMatch> list) throws VTNException {
        NavigableMap<Integer, FlowMatchImpl> map = createMatches(list);
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            boolean changed = !map.equals(matches);
            matches = map;

            return changed;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return the flow match at the specified match index.
     *
     * @param index  A match index.
     * @return  A {@link FlowMatch} instance if found.
     *          {@code null} if not found.
     */
    public FlowMatch getMatch(int index) {
        Integer idx = Integer.valueOf(index);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            FlowMatchImpl match = matches.get(idx);
            return (match == null) ? null: match.getMatch();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Update the condition at the specified index.
     *
     * @param match  A {@link FlowMatch} instance.
     * @return  {@link UpdateType#ADDED} if the specified match condition was
     *          newly added.
     *          {@link UpdateType#CHANGED} if the flow match at the index
     *          configured in {@code match} was changed.
     *          {@code null} if the flow match was not changed.
     * @throws VTNException   An error occurred.
     */
    public UpdateType setMatch(FlowMatch match) throws VTNException {
        FlowMatchImpl fc = new FlowMatchImpl(match);
        Integer idx = fc.getIndex();
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            FlowMatchImpl old = matches.put(idx, fc);
            UpdateType ret;
            if (old == null) {
                ret = UpdateType.ADDED;
            } else if (old.equals(fc)) {
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
     * Remove the flow match at the specified index.
     *
     * @param index  The match index which specifies the flow match to be
     *               removed.
     * @return  A {@link FlowMatchImpl} instance removed from this instance.
     *          {@code null} if no flow match was found at the specified
     *          index.
     */
    public FlowMatchImpl removeMatch(int index) {
        Integer idx = Integer.valueOf(index);
        Lock wrlock = rwLock.writeLock();
        wrlock.lock();
        try {
            return matches.remove(idx);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Determine whether this flow condition matches the specified packet
     * or not.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  A packet context which contains the packet.
     * @return  {@code true} if this flow condition matches the specified
     *          packet. Otherwise {@code false}.
     */
    public boolean match(VTNManagerImpl mgr, PacketContext pctx) {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            if (matches.isEmpty()) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}: {}: Matched an empty condition: packet={}",
                              mgr.getContainerName(), name,
                              pctx.getDescription());
                }
                return true;
            }

            for (FlowMatchImpl match: matches.values()) {
                if (match.match(pctx)) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("{}: {}: Matched the condition: match={}, " +
                                  "packet={}", mgr.getContainerName(), name,
                                  match, pctx.getDescription());
                    }
                    return true;
                }
            }

            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Unmatched: packet={}", name,
                          pctx.getDescription());
            }
            return false;
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Save the configuration of this flow condition to the configuration file.
     *
     * @param mgr  VTN Manager service.
     * @return     "Success" or failure reason.
     */
    public Status saveConfig(VTNManagerImpl mgr) {
        Status status;
        String container = mgr.getContainerName();
        ContainerConfig cfg = new ContainerConfig(container);
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            status = cfg.save(ContainerConfig.Type.FLOWCOND, name, this);
            if (status.isSuccess()) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("{}:{}: Flow condition saved", container, name);
                }
                return status;
            }
        } finally {
            rdlock.unlock();
        }

        String msg = "Failed to save flow condition configuration";
        LOG.error("{}:{}: {}: {}", container, name, msg, status);
        return new Status(StatusCode.INTERNALERROR, msg);
    }

    /**
     * Destory this flow condition.
     *
     * @param mgr  VTN Manager service.
     */
    public void destroy(VTNManagerImpl mgr) {
        String container = mgr.getContainerName();
        ContainerConfig cfg = new ContainerConfig(container);
        cfg.delete(ContainerConfig.Type.FLOWCOND, name);
    }

    /**
     * Construct a flow condition map from the specified list of
     * {@link FlowMatch} instances.
     *
     * @param list  A list of {@link FlowMatch} instances.
     * @return  A map which contains flow conditions.
     * @throws VTNException   An error occurred.
     */
    private NavigableMap<Integer, FlowMatchImpl>
        createMatches(List<FlowMatch> list) throws VTNException {
        TreeMap<Integer, FlowMatchImpl> map =
            new TreeMap<Integer, FlowMatchImpl>();
        if (list != null) {
            for (FlowMatch match: list) {
                FlowMatchImpl fc = new FlowMatchImpl(match);
                int index = fc.getIndex();
                Integer idx = Integer.valueOf(index);
                if (map.put(idx, fc) != null) {
                    String msg = "Duplicate match index: " + idx;
                    throw new VTNException(StatusCode.BADREQUEST, msg);
                }
            }
        }

        return map;
    }

    /**
     * Create a copy of {@link #matches}.
     *
     * @return  A copy of {@link #matches}.
     */
    private NavigableMap<Integer, FlowMatchImpl> copyMatches() {
        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return (NavigableMap<Integer, FlowMatchImpl>)
                ((TreeMap<Integer, FlowMatchImpl>)matches).clone();
        } finally {
            rdlock.unlock();
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
        if (!(o instanceof FlowCondImpl)) {
            return false;
        }

        FlowCondImpl fc = (FlowCondImpl)o;

        // Create a copy of matches in order to avoid deadlock.
        NavigableMap<Integer, FlowMatchImpl> otherMatches = fc.copyMatches();

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            return (name.equals(fc.name) && matches.equals(otherMatches));
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
            return name.hashCode() + matches.hashCode();
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
        StringBuilder builder = new StringBuilder("FlowCondImpl[name=");
        builder.append(name).append(",matches=");

        Lock rdlock = rwLock.readLock();
        rdlock.lock();
        try {
            builder.append(matches.values());
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
    public FlowCondImpl clone() {
        try {
            FlowCondImpl fc = (FlowCondImpl)super.clone();
            fc.matches = new TreeMap<Integer, FlowMatchImpl>(matches);
            return fc;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
