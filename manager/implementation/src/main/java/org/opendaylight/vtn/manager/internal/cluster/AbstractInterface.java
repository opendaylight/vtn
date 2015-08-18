/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.concurrent.ConcurrentMap;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * Abstract implementation of virtual interface.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class AbstractInterface implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8164690341869791248L;

    /**
     * The parent node which contains this virtual interface.
     */
    private transient AbstractBridge  parent;

    /**
     * Path to this virtual interface.
     */
    private transient VInterfacePath  ifPath;

    /**
     * Configuration for this virtual interface.
     */
    private VInterfaceConfig  ifConfig;

    /**
     * Construct a virtual interface instance.
     *
     * @param parent  The parent node that contains this interface.
     * @param name    The name of this interface.
     * @param iconf   Configuration for the interface.
     */
    protected AbstractInterface(AbstractBridge parent, String name,
                                VInterfaceConfig iconf) {
        ifConfig = resolve(iconf);
        initPath(parent, name);
    }

    /**
     * Initialize virtual interface path for this instance.
     *
     * @param parent  The parent node that contains this interface.
     * @param name    The name of this interface.
     */
    void setPath(AbstractBridge parent, String name) {
        initPath(parent, name);
    }

    /**
     * Return path to this interface.
     *
     * @return  Path to this interface.
     */
    public final VInterfacePath getInterfacePath() {
        return ifPath;
    }

    /**
     * Determine whether this interface is administravely enabled or not.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @return {@code true} is returned only if this interface is enabled.
     */
    public final boolean isEnabled() {
        return ifConfig.getEnabled().booleanValue();
    }

    /**
     * Return the name of the container to which this interface belongs.
     *
     * @return  The name of the container.
     */
    public final String getContainerName() {
        return parent.getContainerName();
    }

    /**
     * Return the parent node.
     *
     * @return  The parent node.
     */
    final AbstractBridge getParent() {
        return parent;
    }

    /**
     * Return the name of this node.
     *
     * @return  The name of this node.
     */
    final String getName() {
        return ifPath.getInterfaceName();
    }

    /**
     * Return the name of the tenant to which this interface belongs.
     *
     * @return  The name of the virtual tenant.
     */
    final String getTenantName() {
        return ifPath.getTenantName();
    }

    /**
     * Return configuration of the virtual interface.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @return  A {@link VInterfaceConfig} instance.
     */
    final VInterfaceConfig getVInterfaceConfig() {
        return ifConfig;
    }

    /**
     * Return information about this node.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @return  A {@link VInterface} instance which represents this node.
     */
    final VInterface getVInterface(VTNManagerImpl mgr) {
        VInterfaceState ist = getIfState(mgr);
        return new VInterface(ifPath.getInterfaceName(), ist.getState(),
                              ist.getPortState(), ifConfig);
    }

    /**
     * Set interface configuration.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param ctx    MD-SAL datastore transaction context.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  {@code true} if the configuration is actually changed.
     *          Otherwise {@code false}.
     */
    final boolean setVInterfaceConfig(VTNManagerImpl mgr, TxContext ctx,
                                      VInterfaceConfig iconf, boolean all) {
        VInterfaceConfig cf = (all) ? resolve(iconf) : merge(iconf);
        if (cf.equals(ifConfig)) {
            return false;
        }

        Boolean olden = ifConfig.getEnabled();
        Boolean newen = cf.getEnabled();
        boolean changed = !olden.equals(newen);
        ifConfig = cf;

        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VInterfaceState ist = getIfState(db);
        VnodeState state;
        if (changed) {
            // Need to change interface state.
            state = changeEnableState(mgr, ctx, db, ist, newen.booleanValue());
        } else {
            state = ist.getState();
        }

        VnodeState pstate = ist.getPortState();
        VInterface viface = new VInterface(getName(), state, pstate, cf);
        VInterfaceEvent.changed(mgr, ifPath, viface, true);
        return true;
    }

    /**
     * Resume this virtual node.
     *
     * <p>
     *   This method is called just after this virtual node is instantiated
     *   from the configuration file.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param ctx      A runtime context for MD-SAL datastore transaction task.
     * @param prstate  Current state of the parent node.
     * @return  New state of the parent node.
     */
    final VnodeState resume(VTNManagerImpl mgr, TxContext ctx,
                            VnodeState prstate) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VInterfaceState ist = getIfState(db);
        VnodeState state = resuming(mgr, db, ctx, ist);
        if (!isEnabled()) {
            // State of disabled interface must be DOWN.
            state = VnodeState.DOWN;
        }

        ist.setState(state);
        if (ist.isDirty()) {
            db.put((VNodePath)ifPath, ist);
        }

        Logger logger = getLogger();
        if (logger.isDebugEnabled()) {
            logger.debug("{}:{}: Resume interface: state={}",
                         getContainerName(), ifPath, state);
        }

        return getParentState(state, prstate);
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    void notifyConfiguration(VTNManagerImpl mgr, IVTNManagerAware listener) {
        VInterface viface = getVInterface(mgr);
        mgr.notifyChange(listener, ifPath, viface, UpdateType.ADDED);
    }

    /**
     * Destroy this virtual interface.
     *
     * @param mgr     VTN manager service.
     * @param retain  {@code true} means that the parent node will be retained.
     *                {@code false} means that the parent node is being
     *                destroyed.
     */
    final void destroy(VTNManagerImpl mgr, boolean retain) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        VInterfaceState ist = getIfState(mgr);
        VnodeState state = ist.getState();
        VnodeState pstate = ist.getPortState();
        VInterface viface = new VInterface(getName(), state, pstate, ifConfig);
        destroying(mgr, db, ist, retain);
        db.remove(ifPath);
        VInterfaceEvent.removed(mgr, ifPath, viface, retain);

        // Unlink parent for GC.
        parent = null;
    }

    /**
     * Determine parent node state from current interface state.
     *
     * @param db       Virtual node state DB.
     * @param prstate  Current state of the parent node.
     * @return  New state of the parent node.
     */
    final VnodeState getParentState(ConcurrentMap<VTenantPath, Object> db,
                                    VnodeState prstate) {
        VInterfaceState ist = getIfState(db);
        VnodeState state = ist.getState();
        return getParentState(state, prstate);
    }

    /**
     * Determine state of the parent node.
     *
     * @param state    Current interface state.
     * @param prstate  Current state of the parent node.
     * @return  New state of the parent node.
     */
    protected final VnodeState getParentState(VnodeState state,
                                              VnodeState prstate) {
        if (state == VnodeState.DOWN) {
            return VnodeState.DOWN;
        }

        if (prstate == VnodeState.UNKNOWN && state == VnodeState.UP) {
            return VnodeState.UP;
        }

        return prstate;
    }

    /**
     * Return a runtime state object for this virtual interface.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @return  A runtume state of this interface.
     */
    protected final VInterfaceState getIfState(VTNManagerImpl mgr) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        return getIfState(db);
    }

    /**
     * Return a runtime state object for this virtual interface.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param db  Runtime state DB.
     * @return  A runtume state of this interface.
     */
    protected final VInterfaceState getIfState(
        ConcurrentMap<VTenantPath, Object> db) {
        VInterfaceState ist = (VInterfaceState)db.get(ifPath);
        if (ist == null) {
            VnodeState state = (isEnabled())
                ? VnodeState.UNKNOWN : VnodeState.DOWN;
            ist = new VInterfaceState(state);
        }

        return ist;
    }

    /**
     * Set state of this virtual interface.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param db     Virtual node state DB.
     * @param ist    Runtime state of this interface.
     * @param state  New virtual node state.
     * @return  {@code true} is returned if the state of this node was
     *          actually changed. {@code false} is returned if not changed.
     */
    protected boolean setState(VTNManagerImpl mgr,
                               ConcurrentMap<VTenantPath, Object> db,
                               VInterfaceState ist, VnodeState state) {
        VnodeState st;
        if (!isEnabled()) {
            // State of disabled interface must be DOWN.
            st = VnodeState.DOWN;
        } else {
            st = state;
        }

        ist.setState(st);
        boolean dirty = ist.isDirty();
        if (dirty) {
            db.put((VNodePath)ifPath, ist);
            VnodeState pstate = ist.getPortState();
            VInterface viface = new VInterface(getName(), st, pstate,
                                               ifConfig);
            VInterfaceEvent.changed(mgr, ifPath, viface, false);
        }

        return dirty;
    }

    /**
     * Initialize virtual interface path for this instance.
     *
     * @param parent  The parent node that contains this interface.
     * @param name    The name of this interface.
     */
    private void initPath(AbstractBridge parent, String name) {
        this.parent = parent;
        ifPath = parent.createIfPath(name);
    }

    /**
     * Merge the specified interface configuration to the current
     * configuration.
     *
     * <p>
     *   If at least one field in {@code iconf} keeps a valid value, this
     *   method creates a shallow copy of this object, and set valid values
     *   in {@code iconf} to the copy.
     * </p>
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param iconf  Configuration to be merged.
     * @return  A merged {@code VInterfaceConfig} object.
     */
    private VInterfaceConfig merge(VInterfaceConfig iconf) {
        String desc = iconf.getDescription();
        Boolean en = iconf.getEnabled();
        if (desc == null) {
            return (en == null)
                ? ifConfig
                : new VInterfaceConfig(ifConfig.getDescription(), en);
        } else if (en == null) {
            return new VInterfaceConfig(desc, ifConfig.getEnabled());
        }

        return iconf;
    }

    /**
     * Resolve undefined attributes in the specified interface configuration.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param iconf  The interface configuration.
     * @return       {@code VInterfaceConfig} to be applied.
     */
    private VInterfaceConfig resolve(VInterfaceConfig iconf) {
        Boolean enabled = iconf.getEnabled();
        if (enabled == null) {
            // Interface should be enabled by default.
            return new VInterfaceConfig(iconf.getDescription(), Boolean.TRUE);
        }

        return iconf;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        AbstractInterface ai = (AbstractInterface)o;
        return (ifPath.equals(ai.ifPath) && ifConfig.equals(ai.ifConfig));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().getName().hashCode() +
            (ifPath.hashCode() * 7) + (ifConfig.hashCode() * 11);
    }

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    protected abstract Logger getLogger();

    /**
     * Change enable/disable configuration of this interface.
     *
     * <p>
     *   This method assumes that the call of this method is synchronized
     *   by the parent node.
     * </p>
     *
     * @param mgr      VTN Manager service.
     * @param ctx      MD-SAL datastore transaction context.
     * @param db       Virtual node state DB.
     * @param ist      Current runtime state of this interface.
     * @param enabled  {@code true} is passed if this interface has been
     *                 enabled.
     *                 {@code false} is passed if this interface has been
     *                 disabled.
     * @return  New state of this interface.
     */
    protected abstract VnodeState changeEnableState(
        VTNManagerImpl mgr, TxContext ctx,
        ConcurrentMap<VTenantPath, Object> db, VInterfaceState ist,
        boolean enabled);

    /**
     * Invoked when this interface is going to be resuming from the
     * configuration file.
     *
     * @param mgr  VTN Manager service.
     * @param db   Virtual node state DB.
     * @param ctx  A runtime context for MD-SAL datastore transaction task.
     * @param ist  Current state of this interface.
     * @return  New state of this interface.
     */
    protected abstract VnodeState resuming(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db,
        TxContext ctx, VInterfaceState ist);

    /**
     * Invoked when this interface is going to be destroyed.
     *
     * @param mgr     VTN Manager service.
     * @param db      Virtual node state DB.
     * @param ist     Current state of this interface.
     * @param retain  {@code true} means that the parent node will be retained.
     *                {@code false} means that the parent node is being
     *                destroyed.
     */
    protected abstract void destroying(VTNManagerImpl mgr,
                                       ConcurrentMap<VTenantPath, Object> db,
                                       VInterfaceState ist, boolean retain);
}
