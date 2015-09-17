/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.IOException;
import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * {@code AbstractBridge} class describes virtual node that contains
 * virtual interfaces.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 *
 * @param <T>  Type of virtual interface.
 */
public abstract class AbstractBridge<T extends AbstractInterface>
    extends VTenantNode {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 4508356334331941156L;

    /**
     * Attached virtual interfaces.
     */
    private final Map<String, T> vInterfaces = new TreeMap<String, T>();

    /**
     * Construct an abstract bridge node.
     *
     * @param vtn   The virtual tenant to which a new node belongs.
     * @param name  The name of this node.
     */
    AbstractBridge(VTenantImpl vtn, String name) {
        initPath(vtn, name);
    }

    /**
     * Return the state of the node.
     *
     * @param mgr  VTN Manager service.
     * @return  The state of this node.
     */
    final VnodeState getState(VTNManagerImpl mgr) {
        BridgeState bst = getBridgeState(mgr);
        return bst.getState();
    }

    /**
     * Add a new virtual interface to this node.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual interface.
     * @param iconf  Interface configuration.
     * @throws VTNException  An error occurred.
     */
    final void addInterface(VTNManagerImpl mgr, VInterfacePath path,
                            VInterfaceConfig iconf) throws VTNException {
        // Ensure the given interface name is valid.
        String ifName = path.getInterfaceName();
        MiscUtils.checkName("Interface", ifName);

        if (iconf == null) {
            throw RpcException.getNullArgumentException(
                "Interface configuration");
        }

        Lock wrlock = writeLock();
        try {
            T vif = createInterface(ifName, iconf);
            T old = vInterfaces.put(ifName, vif);
            if (old != null) {
                vInterfaces.put(ifName, old);
                String msg = ifName + ": Interface name already exists";
                throw RpcException.getDataExistsException(msg);
            }

            VInterface viface = vif.getVInterface(mgr);
            VInterfaceEvent.added(mgr, path, viface);
            updateState(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Change configuration of existing virtual interface.
     *
     * @param mgr    VTN Manager service.
     * @param ctx    MD-SAL datastore transaction context.
     * @param path   Path to the virtual interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  {@code true} is returned only if the interface configuration is
     *          actually changed.
     * @throws VTNException  An error occurred.
     */
    final boolean modifyInterface(VTNManagerImpl mgr, TxContext ctx,
                                  VInterfacePath path, VInterfaceConfig iconf,
                                  boolean all)
        throws VTNException {
        if (iconf == null) {
            throw RpcException.getNullArgumentException(
                "Interface configuration");
        }

        // Write lock is needed because this code determines the state of
        // this node by scanning interfaces.
        Lock wrlock = writeLock();
        try {
            T vif = getInterfaceImpl(path);
            if (!vif.setVInterfaceConfig(mgr, ctx, iconf, all)) {
                return false;
            }
            updateState(mgr);
            return true;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Remove the specified virtual interface.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the virtual interface.
     * @throws VTNException  An error occurred.
     */
    final void removeInterface(VTNManagerImpl mgr, VInterfacePath path)
        throws VTNException {
        Lock wrlock = writeLock();
        try {
            String ifName = path.getInterfaceName();
            if (ifName == null) {
                throw RpcException.getNullArgumentException("Interface name");
            }

            T vif = vInterfaces.remove(ifName);
            if (vif == null) {
                throw getInterfaceNotFoundException(ifName);
            }

            vif.destroy(mgr, true);
            updateState(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return a list of all virtual interface information.
     *
     * @param mgr   VTN Manager service.
     * @return  A list of virtual interface information.
     */
    final List<VInterface> getInterfaces(VTNManagerImpl mgr) {
        List<VInterface> list;
        Lock rdlock = readLock();
        try {
            list = new ArrayList<VInterface>(vInterfaces.size());
            for (T vif: vInterfaces.values()) {
                list.add(vif.getVInterface(mgr));
            }
        } finally {
            rdlock.unlock();
        }

        return list;
    }

    /**
     * Return information about the virtual interface associated with
     * the given name.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the interface.
     * @return  The virtual interface information associated with the given
     *          name.
     * @throws VTNException  An error occurred.
     */
    final VInterface getInterface(VTNManagerImpl mgr, VInterfacePath path)
        throws VTNException {
        Lock rdlock = readLock();
        try {
            T vif = getInterfaceImpl(path);
            return vif.getVInterface(mgr);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Resume this bridge node.
     *
     * <p>
     *   This method is called just after this bridge is instantiated from
     *   the configuration file.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @param ctx  A runtime context for MD-SAL datastore transaction task.
     */
    final void resume(VTNManagerImpl mgr, TxContext ctx) {
        VnodeState state = VnodeState.UNKNOWN;
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        String containerName = getContainerName();

        Lock wrlock = writeLock();
        try {
            // Resume virtual interfaces.
            for (T vif: vInterfaces.values()) {
                VnodeState s = vif.resume(mgr, ctx, state);
                if (vif.isEnabled()) {
                    state = s;
                }
            }
            state = resuming(mgr, ctx, state);

            VNodePath path = getNodePath();
            BridgeState bst = getBridgeState(db);
            state = bst.setState(state);
            if (bst.isDirty()) {
                db.put(path, bst);
            }

            Logger logger = getLogger();
            if (logger.isDebugEnabled()) {
                logger.debug("{}:{}: Resumed {}: state={}",
                             containerName, path, path.getNodeType(), state);
            }

            resumed(mgr);
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Return the virtual interface instance associated with the given name.
     *
     * <p>
     *   This method must be called with the bridge lock.
     * </p>
     *
     * @param path  Path to the interface.
     * @return  Virtual interface instance is returned.
     * @throws VTNException  An error occurred.
     */
    protected final T getInterfaceImpl(VInterfacePath path)
        throws VTNException {
        String ifName = path.getInterfaceName();
        if (ifName == null) {
            throw RpcException.getNullArgumentException("Interface name");
        }

        T vif = vInterfaces.get(ifName);
        if (vif == null) {
            throw getInterfaceNotFoundException(ifName);
        }

        return vif;
    }

    /**
     * Set state of the bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param state  New bridge state.
     */
    protected final void setState(VTNManagerImpl mgr, VnodeState state) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        BridgeState bst = getBridgeState(db);
        setState(mgr, db, bst, state);
    }

    /**
     * Set state of the bridge.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param db     Virtual node state DB.
     * @param bst    Runtime state of the bridge.
     * @param state  New bridge state.
     */
    protected final void setState(VTNManagerImpl mgr,
                                  ConcurrentMap<VTenantPath, Object> db,
                                  BridgeState bst, VnodeState state) {
        bst.setState(state);
        if (bst.isDirty()) {
            db.put(getNodePath(), bst);
            stateChanged(mgr, bst, state);
        }
    }

    /**
     * Return a runtime state object for the virtual bridge.
     *
     * @param mgr  VTN Manager service.
     * @return  A runtume state object.
     */
    protected final BridgeState getBridgeState(VTNManagerImpl mgr) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        return getBridgeState(db);
    }

    /**
     * Return a runtime state object for the virtual bridge.
     *
     * @param db  Runtime state DB.
     * @return  A runtume state object.
     */
    protected final BridgeState getBridgeState(
        ConcurrentMap<VTenantPath, Object> db) {
        BridgeState bst = (BridgeState)db.get(getNodePath());
        if (bst == null) {
            bst = new BridgeState(VnodeState.UNKNOWN);
        }

        return bst;
    }

    /**
     * Add a node path to the set of faulted node paths.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param snode  The source node.
     * @param dnode  The destination node.
     * @return  {@code true} is returned if the specified node path is
     *          actually added to the faulted path set.
     *          {@code false} is returned if it already exists in the set.
     */
    protected final boolean addFaultedPath(VTNManagerImpl mgr, SalNode snode,
                                           SalNode dnode) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        BridgeState bst = getBridgeState(db);
        boolean ret = bst.addFaultedPath(snode.getAdNode(), dnode.getAdNode());
        setState(mgr, db, bst, VnodeState.DOWN);

        return ret;
    }

    /**
     * Scan virtual mappings configured in this node, and determine state
     * of this node.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr   VTN Manager service.
     */
    protected final void updateState(VTNManagerImpl mgr) {
        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        BridgeState bst = getBridgeState(db);
        updateState(mgr, db, bst);
    }

    /**
     * Scan virtual mappings configured in this node, and determine state
     * of this node.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr  VTN Manager service.
     * @param db   Runtime state DB.
     * @param bst  Runtime state of the bridge.
     */
    protected final void updateState(VTNManagerImpl mgr,
                                     ConcurrentMap<VTenantPath, Object> db,
                                     BridgeState bst) {
        VnodeState state = (bst.getFaultedPathSize() == 0)
            ? updateStateImpl(mgr, db) : VnodeState.DOWN;
        if (state != VnodeState.DOWN) {
            // Scan virtual interfaces.
            for (T vif: vInterfaces.values()) {
                if (vif.isEnabled()) {
                    state = vif.getParentState(db, state);
                    if (state == VnodeState.DOWN) {
                        break;
                    }
                }
            }
        }

        setState(mgr, db, bst, state);
    }

    /**
     * Scan virtual mappings except for virtual interfaces configured in this
     * node, and determine state of this node.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param db     Runtime state DB.
     * @return  New state of this bridge.
     */
    protected VnodeState updateStateImpl(
        VTNManagerImpl mgr, ConcurrentMap<VTenantPath, Object> db) {
        return VnodeState.UNKNOWN;
    }

    /**
     * Notify the listener of virtual interface configuration.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    protected final void notifyIfConfig(VTNManagerImpl mgr,
                                        IVTNManagerAware listener) {
        for (T vif: vInterfaces.values()) {
            vif.notifyConfiguration(mgr, listener);
        }
    }

    /**
     * Destroy all virtual interfaces in this node.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr  VTN Manager service.
     */
    protected final void destroyInterfaces(VTNManagerImpl mgr) {
        for (Iterator<T> it = vInterfaces.values().iterator(); it.hasNext();) {
            T vif = it.next();
            vif.destroy(mgr, false);
            it.remove();
        }
    }

    /**
     * Return a map that keeps all virtual interfaces.
     *
     * @return  A map that keeps all virtual interfaces.
     */
    protected final Map<String, T> getInterfaceMap() {
        return vInterfaces;
    }

    /**
     * Return a list of objects which represents the contents of this class.
     *
     * <ul>
     *   <li>
     *     This method is used to determine object identity.
     *   </li>
     *   <li>
     *     Note that this method must be called with holding the bridge lock.
     *   </li>
     * </ul>
     *
     * @param copy  If {@code true} is specified, a copies of objects in this
     *              instance are stored in the returned list.
     * @return  A list of objects.
     * @see #equals(Object)
     * @see #hashCode()
     */
    protected List<Object> getContentsList(boolean copy) {
        List<Object> list = new ArrayList<Object>();
        if (copy) {
            list.add(new TreeMap<String, T>(vInterfaces));
        } else {
            list.add(vInterfaces);
        }

        return list;
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
        // Read serialized fields.
        // Note that the lock does not need to be acquired here because this
        // instance is not yet visible.
        in.defaultReadObject();
    }

    /**
     * Serialize this object and write it to the given output stream.
     *
     * @param out  An output stream.
     * @throws IOException
     *    An I/O error occurred.
     */
    @SuppressWarnings("unused")
    private void writeObject(ObjectOutputStream out) throws IOException {
        Lock rdlock = readLock();
        try {
            out.defaultWriteObject();
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Initialize node path to this bridge.
     *
     * @param vtn   Virtual tenant which contains this node.
     * @param name  The name of this node.
     */
    private void initPath(VTenantImpl vtn, String name) {
        VNodePath path = createPath(vtn, name);
        setNodePath(vtn, path);
    }

    /**
     * Return an exception that indicates the specified interface does not
     * exist.
     *
     * @param ifName  The name of the interface.
     * @return  A {@link RpcException} instance.
     */
    private RpcException getInterfaceNotFoundException(String ifName) {
        String msg = ifName + ": Interface does not exist";
        return RpcException.getNotFoundException(msg);
    }

    /**
     * Return path to this bridge.
     *
     * @return  Path to this bridge.
     */
    abstract VNodePath getPath();

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    abstract void notifyConfiguration(VTNManagerImpl mgr,
                                      IVTNManagerAware listener);

    /**
     * Destroy the virtual bridge.
     *
     * @param mgr     VTN manager service.
     * @param retain  {@code true} means that the parent tenant will be
     *                retained. {@code false} means that the parent tenant
     *                is being destroyed.
     */
    abstract void destroy(VTNManagerImpl mgr, boolean retain);

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    protected abstract Logger getLogger();

    /**
     * Create path to this node.
     *
     * @param vtn   A virtual tenant that contains this node.
     * @param name  The name of this node.
     * @return  A path to this node.
     */
    protected abstract VNodePath createPath(VTenantImpl vtn, String name);

    /**
     * Create path to the virtual interface.
     *
     * @param name  The name of the virtual interface.
     * @return  Path to the specified virtual interface.
     */
    protected abstract VInterfacePath createIfPath(String name);

    /**
     * Create a new virtual interface instance.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param name   The name of the virtual interface.
     * @param iconf  Interface configuration.
     * @return  An instance of virtual interface implementation.
     * @throws VTNException  An error occurred.
     */
    protected abstract T createInterface(String name, VInterfaceConfig iconf)
        throws VTNException;

    /**
     * Invoked when this node is going to be resumed from the configuration
     * file.
     *
     * @param mgr    VTN Manager service.
     * @param ctx  A runtime context for MD-SAL datastore transaction task.
     * @param state  Current state of this node.
     * @return  New state of this node.
     */
    protected abstract VnodeState resuming(VTNManagerImpl mgr, TxContext ctx,
                                           VnodeState state);

    /**
     * Invoked when this node is resumed from the configuration file.
     *
     * @param mgr    VTN Manager service.
     */
    protected abstract void resumed(VTNManagerImpl mgr);

    /**
     * Invoked when the state of this node has been changed.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param bst    Runtime state of this node.
     * @param state  New state of this node.
     */
    protected abstract void stateChanged(VTNManagerImpl mgr, BridgeState bst,
                                         VnodeState state);

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (!super.equals(o)) {
            return false;
        }

        AbstractBridge abr = (AbstractBridge)o;

        // Copy contents of the specified bridge in order to avoid deadlock.
        List<Object> otherContents;
        Lock rdlock = abr.readLock();
        try {
            otherContents = abr.getContentsList(true);
        } finally {
            rdlock.unlock();
        }

        rdlock = readLock();
        try {
            List<Object> contents = getContentsList(false);
            return contents.equals(otherContents);
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
    public final int hashCode() {
        Lock rdlock = readLock();
        try {
            return super.hashCode() + getContentsList(false).hashCode();
        } finally {
            rdlock.unlock();
        }
    }

    // VTenantNode

    /**
     * Initialize path to this node.
     *
     * @param vtn   Virtual tenant which contains this node.
     * @param name  The name of this node.
     */
    @Override
    void setPath(VTenantImpl vtn, String name) {
        initPath(vtn, name);

        // Set this bridge as parent of interfaces.
        for (Map.Entry<String, T> entry: vInterfaces.entrySet()) {
            String iname = entry.getKey();
            T vif = entry.getValue();
            vif.setPath(this, iname);
        }
    }
}
