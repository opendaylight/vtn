/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNThreadData;

import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * A map that keeps flow filters.
 *
 * <p>
 *   Note that this class is not synchronized. Synchronization should be done
 *   by the virtual node that contains the flow filter.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class FlowFilterMap implements Serializable, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8039322793324993017L;

    /**
     * A pseudo VLAN ID which represents the VLAN ID is not specified.
     */
    public static final short  VLAN_UNSPEC = -1;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(FlowFilterMap.class);

    /**
     * Pseudo flow filter index that indicates all flow filters in this map.
     */
    private static final int  INDEX_ALL = 0;

    /**
     * A string that indicates this flow filter is applied to outgoing flow.
     */
    private static final String  DIRECTION_IN = "IN";

    /**
     * A string that indicates this flow filter is applied to outgoing flow.
     */
    private static final String  DIRECTION_OUT = "OUT";

    /**
     * Pairs of filter index and flow filter implementation.
     */
    private Map<Integer, FlowFilterImpl>  flowFilters =
        new TreeMap<Integer, FlowFilterImpl>();

    /**
     * The virtual node that contains this flow filter map.
     *
     * <p>
     *   This field never affects object identify.
     * </p>
     */
    private transient FlowFilterNode  parent;

    /**
     * Create a new flow filter map for incoming packets.
     *
     * @param fnode  Virtual node that contains this flow filter.
     * @return  A {@link FlowFilterMap} instance.
     */
    public static FlowFilterMap createIncoming(FlowFilterNode fnode) {
        return new Incoming(fnode);
    }

    /**
     * Create a new flow filter map for outgoing packets.
     *
     * @param fnode  Virtual node that contains this flow filter.
     * @return  A {@link FlowFilterMap} instance.
     */
    public static FlowFilterMap createOutgoing(FlowFilterNode fnode) {
        return new Outgoing(fnode);
    }

    /**
     * Invoked when a flow filter event is received from another controller.
     *
     * @param mgr  VTN Manager service.
     * @param ev   A {@link FlowFilterEvent} instance.
     */
    public static void eventReceived(VTNManagerImpl mgr, FlowFilterEvent ev) {
        // Save VTN configuration.
        VTenantPath path = ev.getPath();
        mgr.saveTenantConfig(path.getTenantName());

        String container = mgr.getContainerName();
        boolean out = ev.isOutput();
        String direction = getFlowDirectionName(out);
        int index = ev.getIndex();
        if (index == INDEX_ALL) {
            assert ev.getUpdateType() == UpdateType.REMOVED;
            logCleared(container, path, direction);
        } else {
            UpdateType type = ev.getUpdateType();
            logUpdated(container, path, direction, index, type, null);
        }
    }

    /**
     * Return a string which describes the flow direction to be evaluated.
     *
     * @param out  A boolean value which specifies the flow direction.
     * @return  A string which represents the flow direction.
     */
    public static String getFlowDirectionName(boolean out) {
        return (out) ? DIRECTION_OUT : DIRECTION_IN;
    }

    /**
     * Record an informational log that indicates a flow filter was updated.
     *
     * @param container  The name of the container.
     * @param path       Pat to the virtual node.
     * @param direction  A string which represents the flow direction.
     * @param index      Index of the flow filter.
     * @param type       Update type.
     * @param fi         A {@link FlowFilterImpl} for trace logging.
     */
    private static void logUpdated(String container, VTenantPath path,
                                   String direction, int index,
                                   UpdateType type, FlowFilterImpl fi) {
        if (fi != null && LOG.isTraceEnabled()) {
            LOG.trace("{}:{}:{}.{}: Flow filter was {}: {}",
                      container, path, direction, index, type.getName(),
                      fi.getFlowFilter());
        } else {
            LOG.info("{}:{}:{}.{}: Flow filter was {}.",
                     container, path, direction, index, type.getName());
        }
    }

    /**
     * Record an informational log that indicates all flow filters was updated.
     *
     * @param container  The name of the container.
     * @param path       Pat to the virtual node.
     * @param direction  A string which represents the flow direction.
     */
    private static void logCleared(String container, VTenantPath path,
                                   String direction) {
        LOG.info("{}:{}:{}: All flow filters were removed.",
                 container, path, direction);
    }

    /**
     * Construct a new instance.
     *
     * @param fnode  Virtual node that contains this flow filter.
     */
    private FlowFilterMap(FlowFilterNode fnode) {
        setParent(fnode);
    }

    /**
     * Return a list of flow filters configured in this instance.
     *
     * @return  A list of {@link FlowFilter} instances.
     */
    public final List<FlowFilter> getAll() {
        List<FlowFilter> list = new ArrayList<FlowFilter>(flowFilters.size());
        for (FlowFilterImpl fi: flowFilters.values()) {
            list.add(fi.getFlowFilter());
        }

        return list;
    }

    /**
     * Return the flow filter associated with the specified index number
     * in this instance.
     *
     * @param index  The index number of the flow filter.
     * @return  A {@link FlowFilter} instance if found.
     *          {@code null} if not found.
     */
    public final FlowFilter get(int index) {
        FlowFilterImpl fi = flowFilters.get(index);
        return (fi == null) ? null : fi.getFlowFilter();
    }

    /**
     * Create or modify the flow filter specified by the index number.
     *
     * @param mgr     VTN Manager service.
     * @param index   The index number of the flow filter.
     * @param filter  A {@link FlowFilter} instance which specifies the
     *                configuration of the flow filter.
     * @return  A {@link UpdateType} object which represents the result of the
     *          operation is returned. {@code null} is returned if no change
     *          was made.
     * @throws VTNException  An error occurred.
     */
    public final UpdateType set(VTNManagerImpl mgr, int index,
                                FlowFilter filter) throws VTNException {
        FlowFilterImpl fi = FlowFilterImpl.create(parent, index, filter);
        Integer key = Integer.valueOf(index);

        UpdateType result;
        FlowFilterImpl old = flowFilters.put(key, fi);
        if (old == null) {
            result = UpdateType.ADDED;
        } else if (old.equals(fi)) {
            // No change was made to the flow filter.
            return null;
        } else {
            result = UpdateType.CHANGED;
        }

        // REVISIT: Select flow entries affected by the change.
        VTenantPath path = parent.getPath();
        VTNThreadData.removeFlows(mgr, path.getTenantName());

        logUpdated(parent.getContainerName(), path, getFlowDirectionName(),
                   index, result, fi);
        FlowFilterEvent.raise(mgr, path, isOutput(), index, result);
        return result;
    }

    /**
     * Remove the flow filter specified by the index number.
     *
     * @param mgr    VTN Manager service.
     * @param index  The index number of the flow filter.
     * @return  A {@link Status} instance which indicates the result of the
     *          operation. {@code null} is returned if the specified
     *          flow filter does not exist.
     */
    public final Status remove(VTNManagerImpl mgr, int index) {
        Integer key = Integer.valueOf(index);
        FlowFilterImpl fi = flowFilters.remove(key);
        if (fi == null) {
            return null;
        }

        // REVISIT: Select flow entries affected by the change.
        VTenantPath path = parent.getPath();
        VTNThreadData.removeFlows(mgr, path.getTenantName());

        UpdateType type = UpdateType.REMOVED;
        logUpdated(parent.getContainerName(), path, getFlowDirectionName(),
                   index, type, fi);
        FlowFilterEvent.raise(mgr, path, isOutput(), index, type);
        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Remove all flow filters in this instance.
     *
     * @param mgr    VTN Manager service.
     * @return  A {@link Status} instance which indicates the result of the
     *          operation. {@code null} is returned if this instance contains
     *          no flow filter.
     */
    public final Status clear(VTNManagerImpl mgr) {
        if (flowFilters.isEmpty()) {
            return null;
        }

        HashMap<Integer, FlowFilterImpl> removed = (LOG.isTraceEnabled())
            ? new HashMap<Integer, FlowFilterImpl>(flowFilters)
            : null;
        flowFilters.clear();

        // REVISIT: Select flow entries affected by the change.
        VTenantPath path = parent.getPath();
        VTNThreadData.removeFlows(mgr, path.getTenantName());

        String container = parent.getContainerName();
        UpdateType type = UpdateType.REMOVED;
        String direction = getFlowDirectionName();
        if (removed == null) {
            logCleared(container, path, direction);
        } else {
            for (FlowFilterImpl fi: removed.values()) {
                logUpdated(container, path, direction, fi.getIndex(), type,
                           fi);
            }
        }

        FlowFilterEvent.raise(mgr, path, isOutput(), INDEX_ALL, type);
        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Determine whether this flow filter map is empty or not.
     *
     * @return  {@code true} only if this flow filter map is empty.
     */
    public final boolean isEmpty() {
        return flowFilters.isEmpty();
    }

    /**
     * Evaluate flow filters configured in this instance.
     *
     * <p>
     *   This method must be called with holding the lock for the parent node.
     * </p>
     *
     * @param mgr   VTN Manager service.
     * @param pctx  A packet context which contains the packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @return  A {@link PacketContext} to be used for succeeding packet
     *          processing.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter configured in
     *    this instance.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter configured in
     *    this instance.
     */
    public final PacketContext evaluate(VTNManagerImpl mgr, PacketContext pctx,
                                        short vid)
        throws DropFlowException, RedirectFlowException {
        PacketContext pc = pctx;
        if (!flowFilters.isEmpty()) {
            pctx.setFiltered(true);
            if (pctx.isFilterDisabled()) {
                logDisabled(pctx);
            } else {
                pc = getPacketContext(pctx);
                evaluateImpl(mgr, pc, vid);
            }
        }

        return pc;
    }

    /**
     * Create a prefix string for a log record.
     *
     * @param index  Index of the flow filter.
     * @return  A prefix for a log record.
     */
    final String getLogPrefix(int index) {
        StringBuilder builder = new StringBuilder(parent.getContainerName());
        return builder.append(':').append(parent.getPath()).
            append(':').append(getFlowDirectionName()).
            append('.').append(index).toString();
    }

    /**
     * Return the virtual node that contains this flow filter map.
     *
     * @return  A {@link FlowFilterNode} instance that contains this
     *          flow filter map.
     */
    final FlowFilterNode getParent() {
        return parent;
    }

    /**
     * Set the virtual node that contains this flow filter.
     *
     * @param fnode  Virtual node that contains this flow filter.
     */
    final void setParent(FlowFilterNode fnode) {
        parent = fnode;
    }

    /**
     * Determine the flow direction to be evaluated.
     *
     * @return  {@code true} is returned if the flow filter is applied to
     *          outgoing flow. Otherwise {@code false} is returned.
     */
    protected abstract boolean isOutput();

    /**
     * Return a string which describes the flow direction to be evaluated.
     *
     * @return  A string which represents the flow direction.
     */
    protected abstract String getFlowDirectionName();

    /**
     * Return a {@link PacketContext} instance to be used for filtering.
     *
     * @param pctx  A packet context which contains the packet.
     * @return  A {@link PacketContext} instance.
     */
    protected abstract PacketContext getPacketContext(PacketContext pctx);

    /**
     * Evaluate flow filters configured in this instance.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  A packet context which contains the packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter configured in
     *    this instance.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter configured in
     *    this instance.
     */
    private void evaluateImpl(VTNManagerImpl mgr, PacketContext pctx,
                              short vid)
        throws DropFlowException, RedirectFlowException {
        if (vid >= 0) {
            // Use the given VLAN ID for packet matching.
            pctx.setVlan(vid);
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}:{}: Evaluating flow filter map: {}",
                      parent.getContainerName(), parent.getPath(),
                      getFlowDirectionName(), pctx.getDescription());
        }

        for (FlowFilterImpl fi: flowFilters.values()) {
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}: Evaluating flow filter: {}",
                          getLogPrefix(fi.getIndex()), fi);
            }
            if (fi.evaluate(mgr, pctx, this)) {
                return;
            }
        }

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}:{}: No flow filter was matched",
                      parent.getContainerName(), parent.getPath(),
                      getFlowDirectionName());
        }
    }

    /**
     * Record a log message that indicates the given packet disables the
     * flow filter.
     *
     * @param pctx  A packet context which contains the packet.
     */
    private void logDisabled(PacketContext pctx) {
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}:{}:{}: Flow filter is disabled: {}",
                      parent.getContainerName(), parent.getPath(),
                      getFlowDirectionName(), pctx.getDescription());
        }
    }

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

        FlowFilterMap fmap = (FlowFilterMap)o;
        return flowFilters.equals(fmap.flowFilters);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return getClass().getName().hashCode() +
            (flowFilters.hashCode() * 37);
    }

    // Cloneable

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public final FlowFilterMap clone() {
        try {
            FlowFilterMap fmap = (FlowFilterMap)super.clone();
            fmap.flowFilters = (Map<Integer, FlowFilterImpl>)
                ((TreeMap<Integer, FlowFilterImpl>)flowFilters).clone();

            return fmap;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }

    /**
     * A map that keeps flow filters for incoming packets.
     *
     * <p>
     *   Although this class is public to other packages, this class does not
     *   provide any API. Applications other than VTN Manager must not use this
     *   class.
     * </p>
     */
    public static final class Incoming extends FlowFilterMap {
        /**
         * Version number for serialization.
         */
        private static final long serialVersionUID = -7266877348382534521L;

        /**
         * Construct a new instance.
         *
         * @param fnode  Virtual node that contains this flow filter.
         */
        private Incoming(FlowFilterNode fnode) {
            super(fnode);
        }

        /**
         * Determine the flow direction to be evaluated.
         *
         * @return  {@code false}.
         */
        @Override
        protected boolean isOutput() {
            return false;
        }

        /**
         * Return a string which describes the flow direction to be evaluated.
         *
         * @return  {@code "IN"}.
         */
        @Override
        protected String getFlowDirectionName() {
            return DIRECTION_IN;
        }

        /**
         * Return a {@link PacketContext} instance to be used for filtering.
         *
         * @param pctx  A packet context which contains the packet.
         * @return  The given {@link PacketContext} instance is always
         *          returned.
         */
        @Override
        protected PacketContext getPacketContext(PacketContext pctx) {
            return pctx;
        }
    }

    /**
     * A map that keeps flow filters for outgoing packets.
     *
     * <p>
     *   Although this class is public to other packages, this class does not
     *   provide any API. Applications other than VTN Manager must not use this
     *   class.
     * </p>
     */
    public static final class Outgoing extends FlowFilterMap {
        /**
         * Version number for serialization.
         */
        private static final long serialVersionUID = 7451633827904306067L;

        /**
         * Construct a new instance.
         *
         * @param fnode  Virtual node that contains this flow filter.
         */
        private Outgoing(FlowFilterNode fnode) {
            super(fnode);
        }

        /**
         * Determine the flow direction to be evaluated.
         *
         * @return  {@code true}.
         */
        @Override
        protected boolean isOutput() {
            return true;
        }

        /**
         * Return a string which describes the flow direction to be evaluated.
         *
         * @return  {@code "OUT"}.
         */
        @Override
        protected String getFlowDirectionName() {
            return DIRECTION_OUT;
        }

        /**
         * Return a {@link PacketContext} instance to be used for filtering.
         *
         * @param pctx  A packet context which contains the packet.
         * @return  A {@link PacketContext} to be used for transmitting packet.
         */
        @Override
        protected PacketContext getPacketContext(PacketContext pctx) {
            // If the given packet is going to be broadcasted, we have to
            // preserve the original packet for succeeding transmission.
            return (pctx.isFlooding()) ? pctx.clone() : pctx;
        }
    }
}
