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
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.locks.Lock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNThreadData;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * Implementation of vTerminal.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VTerminalImpl extends PortBridge<VTerminalIfImpl> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 518889407387134507L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTerminalImpl.class);

    /**
     * Configuration for the vTerminal.
     */
    private VTerminalConfig  terminalConfig;

    /**
     * Construct a vTerminal instance.
     *
     * @param vtn     The virtual tenant to which a new vTerminal belongs.
     * @param name    The name of the vTerminal.
     * @param vtconf  Configuration for the vTerminal.
     * @throws VTNException  An error occurred.
     */
    VTerminalImpl(VTenantImpl vtn, String name, VTerminalConfig vtconf)
        throws VTNException {
        super(vtn, name);
        terminalConfig = vtconf;
    }

    /**
     * Return information about the vTerminal.
     *
     * @param mgr  VTN Manager service.
     * @return  Information about the vTerminal.
     */
    VTerminal getVTerminal(VTNManagerImpl mgr) {
        Lock rdlock = readLock();
        try {
            return getVTerminal(mgr, getName(), terminalConfig);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Set vTerminal configuration.
     *
     * @param mgr     VTN Manager service.
     * @param vtconf  vTerminal configuration.
     * @param all     If {@code true} is specified, all attributes of the
     *                vTerminal are modified. In this case, {@code null} in
     *                {@code vtconf} is interpreted as default value.
     *                If {@code false} is specified, an attribute is not
     *                modified if its value in {@code vtconf} is {@code null}.
     * @return  {@code true} if the configuration is actually changed.
     *          Otherwise {@code false}.
     * @throws VTNException  An error occurred.
     */
    boolean setVTerminalConfig(VTNManagerImpl mgr, VTerminalConfig vtconf,
                               boolean all)
        throws VTNException {
        Lock wrlock = writeLock();
        try {
            VTerminalConfig cf = (all) ? vtconf : merge(vtconf);
            if (cf.equals(terminalConfig)) {
                return false;
            }

            terminalConfig = cf;
            VTerminalPath path = getPath();
            String name = path.getTerminalName();
            VTerminal vterm = getVTerminal(mgr, name, cf);
            VTerminalEvent.changed(mgr, path, vterm, true);
            return true;
        } finally {
            wrlock.unlock();
        }
    }

    /**
     * Merge the given vTerminal configuration to the current configuration.
     *
     * <p>
     *   If at least one field in {@code vtconf} keeps a valid value, this
     *   method creates a shallow copy of the current configuration, and set
     *   valid values in {@code vtconf} to the copy.
     * </p>
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param vtconf  Configuration to be merged.
     * @return  A merged {@code VTerminalConfig} object.
     */
    private VTerminalConfig merge(VTerminalConfig vtconf) {
        String desc = vtconf.getDescription();
        return (desc == null) ? terminalConfig : vtconf;
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
     * Return information about the vTerminal.
     *
     * <p>
     *   Note that this method must be called with holding the bridge write
     *   lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param name   The name of the vTerminal.
     * @param vtconf  vTerminal configuration.
     * @return  Information about the vTerminal.
     */
    private VTerminal getVTerminal(VTNManagerImpl mgr, String name,
                                   VTerminalConfig vtconf) {
        BridgeState bst = getBridgeState(mgr);
        int faulted = bst.getFaultedPathSize();
        return new VTerminal(name, bst.getState(), faulted, vtconf);
    }

    /**
     * Notify information about the host that sent a packet.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param pctx   The context of the received packet.
     */
    private void notifyHost(VTNManagerImpl mgr, PacketContext pctx) {
        byte[] src = pctx.getSourceAddress();
        if (!NetUtils.isUnicastMACAddr(src)) {
            return;
        }

        long mac = NetUtils.byteArray6ToLong(src);
        if (mac == 0L) {
            // Zero address should be ignored.
            LOG.warn("{}:{}: Ignore zero MAC address: {}",
                     getContainerName(), getNodePath(), pctx.getFrame());
            return;
        }

        byte[] sip = pctx.getSourceIpAddress();
        if (sip == null) {
            return;
        }

        InetAddress iaddr;
        try {
            iaddr = InetAddress.getByAddress(sip);
        } catch (UnknownHostException e) {
            // This should never happen.
            LOG.error("{}:{}: Invalid IP address: {}, ipaddr={}",
                      getContainerName(), getNodePath(),
                      pctx.getDescription(),
                      HexEncode.bytesToHexStringFormat(sip));
            return;
        }

        NodeConnector port = pctx.getIncomingNodeConnector();
        short vlan = pctx.getVlan();

        try {
            HostNodeConnector host =
                new HostNodeConnector(src, iaddr, port, vlan);
            if (LOG.isTraceEnabled()) {
                LOG.trace("{}:{}: Notify new host: ipaddr={}, host={}",
                          getContainerName(), getNodePath(),
                          iaddr.getHostAddress(), host);
            }
            mgr.notifyHost(host);
        } catch (Exception e) {
            if (LOG.isErrorEnabled()) {
                StringBuilder builder = new StringBuilder(getContainerName());
                builder.append(':').append(getNodePath()).
                    append(": Unable to create host: src=").
                    append(HexEncode.bytesToHexStringFormat(src)).
                    append(", ipaddr=").append(iaddr).
                    append(", port=").append(port.toString()).
                    append(", vlan=").append((int)vlan);
                LOG.error(builder.toString(), e);
            }
        }
    }

    // AbstractBridge

    /**
     * Return path to this vTerminal.
     *
     * @return  Path to this vTerminal.
     */
    @Override
    VTerminalPath getPath() {
        return (VTerminalPath)getNodePath();
    }

    /**
     * Notify the listener of current configuration.
     *
     * @param mgr       VTN Manager service.
     * @param listener  VTN manager listener service.
     */
    @Override
    void notifyConfiguration(VTNManagerImpl mgr, IVTNManagerAware listener) {
        VTerminalPath path = getPath();
        UpdateType type = UpdateType.ADDED;
        Lock rdlock = readLock();
        try {
            VTerminal vterm = getVTerminal(mgr);
            mgr.notifyChange(listener, path, vterm, type);
            notifyIfConfig(mgr, listener);
        } finally {
            rdlock.unlock();
        }
    }

    /**
     * Destroy the vTerminal.
     *
     * @param mgr     VTN manager service.
     * @param retain  {@code true} means that the parent tenant will be
     *                retained. {@code false} means that the parent tenant
     *                is being destroyed.
     */
    @Override
    void destroy(VTNManagerImpl mgr, boolean retain) {
        VTerminalPath path = getPath();
        VTerminal vterm;
        Lock wrlock = writeLock();
        try {
            vterm = getVTerminal(mgr);

            // Destroy all interfaces.
            destroyInterfaces(mgr);

            if (retain) {
                // Purge all VTN flows related to this vTerminal.
                VTNThreadData.removeFlows(mgr, path);
            }

            destroy();
        } finally {
            wrlock.unlock();
        }

        ConcurrentMap<VTenantPath, Object> db = mgr.getStateDB();
        db.remove(path);
        VTerminalEvent.removed(mgr, path, vterm, retain);
    }

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * Create path to this node.
     *
     * @param vtn   A virtual tenant that contains this node.
     * @param name  The name of this node.
     * @return  A path to this node.
     */
    @Override
    protected VTerminalPath createPath(VTenantImpl vtn, String name) {
        return new VTerminalPath(vtn.getName(), name);
    }

    /**
     * Create path to the virtual interface.
     *
     * @param name  The name of the virtual interface.
     * @return  Path to the specified virtual interface.
     */
    @Override
    protected VInterfacePath createIfPath(String name) {
        return new VTerminalIfPath(getPath(), name);
    }

    /**
     * Create a new virtual interface instance.
     *
     * @param name   The name of the virtual interface.
     * @param iconf  Interface configuration.
     * @return  An instance of virtual interface implementation.
     */
    @Override
    protected VTerminalIfImpl createInterface(String name,
                                              VInterfaceConfig iconf) {
        return new VTerminalIfImpl(this, name, iconf);
    }

    /**
     * Invoked when this node is going to be resumed from the configuration
     * file.
     *
     * @param mgr    VTN Manager service.
     * @param state  Current state of this node.
     * @return  New state of this node.
     */
    @Override
    protected VNodeState resuming(VTNManagerImpl mgr, VNodeState state) {
        // Nothing to do.
        return state;
    }

    /**
     * Invoked when this node is resumed from the configuration file.
     *
     * @param mgr    VTN Manager service.
     */
    @Override
    protected void resumed(VTNManagerImpl mgr) {
        // Nothing to do.
    }

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
    @Override
    protected void stateChanged(VTNManagerImpl mgr, BridgeState bst,
                                VNodeState state) {
        VTerminalPath path = getPath();
        int faulted = bst.getFaultedPathSize();
        VTerminal vterm = new VTerminal(path.getTerminalName(), state, faulted,
                                        terminalConfig);
        VTerminalEvent.changed(mgr, path, vterm, false);
    }

    // PortBridge

    /**
     * Evaluate flow filters configured in this vTerminal against the given
     * outgoing packet.
     *
     * <p>
     *   This method does nothing because vTerminal can not have flow filters.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param pctx    The context of the received packet.
     * @param vid     A VLAN ID for the outgoing packet.
     * @return  A value passed to {@code pctx} is always returned.
     */
    @Override
    PacketContext filterOutgoingPacket(VTNManagerImpl mgr, PacketContext pctx,
                                       short vid) {
        return pctx;
    }

    /**
     * Handle the received packet.
     *
     * <p>
     *   Note that this method must be called with holding the bridge lock.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param pctx   The context of the received packet.
     * @param vnode  A {@link VirtualMapNode} instance that maps the received
     *               packet.
     */
    @Override
    protected void handlePacket(VTNManagerImpl mgr, PacketContext pctx,
                                VirtualMapNode vnode) {
        // Notify source host of the packet.
        notifyHost(mgr, pctx);

        if (LOG.isDebugEnabled()) {
            LOG.debug("{}:{}: Disable input from vTerminal interface.",
                      getContainerName(), vnode.getPath());
        }

        vnode.disableInput(mgr, pctx);
    }
}
