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

import org.opendaylight.vtn.manager.internal.VTNFlowDatabase;
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
public final class FlowFilterMap implements Serializable, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -7816922061009950427L;

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
     * Flow direction to be evaluated.
     *
     * {@code true} means that this flow filter should be applied to
     * outgoing flow.
     */
    private final boolean  output;

    /**
     * The virtual node that contains this flow filter map.
     *
     * <p>
     *   This field never affects object identify.
     * </p>
     */
    private transient FlowFilterNode  parent;

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
        int index = ev.getIndex();
        if (index == INDEX_ALL) {
            assert ev.getUpdateType() == UpdateType.REMOVED;
            logCleared(container, path, out);
        } else {
            UpdateType type = ev.getUpdateType();
            logUpdated(container, path, out, index, type, null);
        }
    }

    /**
     * Record an informational log that indicates a flow filter was updated.
     *
     * @param container  The name of the container.
     * @param path       Pat to the virtual node.
     * @param out        A boolean value which specifies the flow direction.
     * @param index      Index of the flow filter.
     * @param type       Update type.
     * @param fi         A {@link FlowFilterImpl} for trace logging.
     */
    private static void logUpdated(String container, VTenantPath path,
                                   boolean out, int index, UpdateType type,
                                   FlowFilterImpl fi) {
        String direction = getFlowDirectionName(out);
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
     * @param out        A boolean value which specifies the flow direction.
     */
    private static void logCleared(String container, VTenantPath path,
                                   boolean out) {
        LOG.info("{}:{}:{}: All flow filters were removed.",
                 container, path, getFlowDirectionName(out));
    }

    /**
     * Return a string which describes the flow direction to be evaluated.
     *
     * @param out  A boolean value which specifies the flow direction.
     * @return  A string which represents the flow direction.
     */
    private static String getFlowDirectionName(boolean out) {
        return (out) ? DIRECTION_OUT : DIRECTION_IN;
    }

    /**
     * Construct a new instance.
     *
     * @param fnode  Virtual node that contains this flow filter.
     * @param out    {@code true} means that this flow filter should be applied
     *               to outgoing flows.
     */
    public FlowFilterMap(FlowFilterNode fnode, boolean out) {
        output = out;
        setParent(fnode);
    }

    /**
     * Return a list of flow filters configured in this instance.
     *
     * @return  A list of {@link FlowFilter} instances.
     */
    public List<FlowFilter> getAll() {
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
    public FlowFilter get(int index) {
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
    public UpdateType set(VTNManagerImpl mgr, int index, FlowFilter filter)
        throws VTNException {
        FlowFilterImpl fi = FlowFilterImpl.create(index, filter);
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
        String tname = path.getTenantName();
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(tname);
        VTNThreadData.removeFlows(mgr, fdb);

        logUpdated(parent.getContainerName(), path, output, index, result, fi);
        FlowFilterEvent.raise(mgr, path, output, index, result);
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
    public Status remove(VTNManagerImpl mgr, int index) {
        Integer key = Integer.valueOf(index);
        FlowFilterImpl fi = flowFilters.remove(key);
        if (fi == null) {
            return null;
        }

        // REVISIT: Select flow entries affected by the change.
        VTenantPath path = parent.getPath();
        String tname = path.getTenantName();
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(tname);
        VTNThreadData.removeFlows(mgr, fdb);

        UpdateType type = UpdateType.REMOVED;
        logUpdated(parent.getContainerName(), path, output, index, type, fi);
        FlowFilterEvent.raise(mgr, path, output, index, type);
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
    public Status clear(VTNManagerImpl mgr) {
        if (flowFilters.isEmpty()) {
            return null;
        }

        HashMap<Integer, FlowFilterImpl> removed = (LOG.isTraceEnabled())
            ? new HashMap<Integer, FlowFilterImpl>(flowFilters)
            : null;
        flowFilters.clear();

        // REVISIT: Select flow entries affected by the change.
        VTenantPath path = parent.getPath();
        String tname = path.getTenantName();
        VTNFlowDatabase fdb = mgr.getTenantFlowDB(tname);
        VTNThreadData.removeFlows(mgr, fdb);

        String container = parent.getContainerName();
        UpdateType type = UpdateType.REMOVED;
        if (removed == null) {
            logCleared(container, path, output);
        } else {
            for (FlowFilterImpl fi: removed.values()) {
                logUpdated(container, path, output, fi.getIndex(), type, fi);
            }
        }

        FlowFilterEvent.raise(mgr, path, output, INDEX_ALL, type);
        return new Status(StatusCode.SUCCESS, null);
    }

    /**
     * Determine whether this flow filter map is empty or not.
     *
     * @return  {@code true} only if this flow filter map is empty.
     */
    public boolean isEmpty() {
        return flowFilters.isEmpty();
    }

    /**
     * Set the virtual node that contains this flow filter.
     *
     * @param fnode  Virtual node that contains this flow filter.
     */
    void setParent(FlowFilterNode fnode) {
        parent = fnode;
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        FlowFilterMap fmap = (FlowFilterMap)o;
        return (flowFilters.equals(fmap.flowFilters) && output == fmap.output);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = flowFilters.hashCode();
        if (output) {
            h += 37;
        }

        return h;
    }

    // Cloneable

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public FlowFilterMap clone() {
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
}
