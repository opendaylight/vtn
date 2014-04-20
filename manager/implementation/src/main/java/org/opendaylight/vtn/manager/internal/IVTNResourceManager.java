/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.Timer;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.NodeVlan;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * This interface defines an internal OSGi service which manages global
 * resources used by the VTN Manager.
 */
public interface IVTNResourceManager {
    /**
     * Add the VTN Manager service.
     *
     * @param mgr  VTN Manager service.
     */
    void addManager(VTNManagerImpl mgr);

    /**
     * Remove the VTN Manager service.
     *
     * @param mgr  VTN Manager service.
     */
    void removeManager(VTNManagerImpl mgr);

    /**
     * Register a new VLAN mapping.
     *
     * <ul>
     *   <li>
     *     If {@code nvlan} specifies a VLAN network on a specific node and
     *     {@code true} is specified to {@code purge}, this method purges
     *     network caches corresponding to the network superseded by a new
     *     VLAN mapping.
     *   </li>
     *   <li>
     *     Note that this method must be called with holding the read/write
     *     lock of {@code mgr} in writer mode.
     *   </li>
     * </ul>
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the VLAN mapping which maps the specified VLAN
     *               network.
     * @param nvlan  A {@link NodeVlan} object which specifies the VLAN network
     *               to be mapped.
     * @param purge  If {@code true} is specified, this method purges caches
     *               corresponding to the VLAN network superseded by a new
     *               VLAN mapping.
     * @return  {@code null} is returned on success.
     *          On failure, a reference to the VLAN mapping which maps the
     *          VLAN network specified by {@code nvlan} is returned.
     * @throws VTNException  A fatal error occurred.
     */
    MapReference registerVlanMap(VTNManagerImpl mgr, VlanMapPath path,
                                 NodeVlan nvlan, boolean purge)
        throws VTNException;

    /**
     * Unregister VLAN mapping.
     *
     * <p>
     *   Note that this method must be called with holding the read/write lock
     *   of {@code mgr} in writer mode.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the VLAN mapping to be unregistered.
     * @param nvlan  A {@link NodeVlan} object which specifies the VLAN network
     *               to be unmapped.
     * @param purge  If {@code true} is specified, this method purges caches
     *               corresponding to the unmapped VLAN network.
     * @throws VTNException  A fatal error occurred.
     */
    void unregisterVlanMap(VTNManagerImpl mgr, VlanMapPath path,
                           NodeVlan nvlan, boolean purge)
        throws VTNException;

    /**
     * Register mapping between physical switch port and virtual bridge
     * interface.
     *
     * <ul>
     *   <li>
     *     If a non-{@code null} value is specified to {@code rmlan},
     *     the port mapping which maps the VLAN network specified by
     *     {@code rmlan} is removed in one transaction.
     *     Note that the VLAN network specified by {@code rmlan} must be
     *     currently mapped to the virtual interface specified by {@code path}.
     *   </li>
     *   <li>
     *     If {@code true} is specified to {@code purge}, this method purges
     *     network caches corresponding to obsolete mapping and the
     *     VLAN network superseded by a new port mapping.
     *   </li>
     *   <li>
     *     Note that this method must be called with holding the read/write
     *     lock of {@code mgr} in writer mode.
     *   </li>
     * </ul>
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual bridge interface which maps the
     *               specified VLAN network.
     * @param pvlan  A {@link PortVlan} object which specifies the VLAN network
     *               to be mapped. No port mapping is added if {@code null} is
     *               specified.
     * @param rmlan  A {@link PortVlan} object which specifies the VLAN network
     *               to be unmapped. No port mapping is removed if {@code null}
     *               is specified.
     * @param purge  If {@code true} is specified, this method purges network
     *               caches as appropriate.
     * @return  {@code null} is returned on success.
     *          On failure, a reference to the port mapping which maps the
     *          VLAN network specified by {@code pvlan} is returned.
     * @throws VTNException  A fatal error occurred.
     */
    MapReference registerPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                                 PortVlan pvlan, PortVlan rmlan, boolean purge)
        throws VTNException;

    /**
     * Unregister port mapping.
     *
     * <p>
     *   Note that this method must be called with holding the read/write lock
     *   of {@code mgr} in writer mode.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual bridge interface.
     * @param pvlan  A {@link PortVlan} object which specifies the VLAN network
     *               to be unmapped.
     * @param purge  If {@code true} is specified, this method purges caches
     *               corresponding to the unmapped VLAN network.
     * @throws VTNException  A fatal error occurred.
     */
    void unregisterPortMap(VTNManagerImpl mgr, VBridgeIfPath path,
                           PortVlan pvlan, boolean purge) throws VTNException;

    /**
     * Determine whether the specified VLAN network is mapped by VLAN mapping
     * or not.
     *
     * @param nvlan  A pair of the switch and the VLAN ID.
     * @return  {@code true} is returned only if the given VLAN network is
     *          mapped by VLAN mapping.
     *          Otherwise {@code false} is returned.
     */
    boolean isVlanMapped(NodeVlan nvlan);

    /**
     * Determine whether the given switch port is mapped to the virtual
     * interface or not.
     *
     * @param pvlan  A pair of the switch port and the VLAN ID.
     * @return  {@code true} is returned only if the given switch port is
     *          mapped to the virtual interface.
     *          Otherwise {@code false} is returned.
     */
    boolean isPortMapped(PortVlan pvlan);

    /**
     * Return a reference to virtual network mapping which maps the VLAN
     * network specified by the switch port and the VLAN ID.
     *
     * @param nc    A node connector corresponding to the switch port.
     *              Specifying {@code null} results in undefined behavior.
     * @param vlan  A VLAN ID.
     * @return      A {@link MapReference} object is returned if found.
     *              {@code null} is returned if not found.
     */
    MapReference getMapReference(NodeConnector nc, short vlan);

    /**
     * Return the global timer.
     *
     * @return  The global timer.
     */
    Timer getTimer();

    /**
     * Run the given command asynchronously.
     *
     * <p>
     *   Note that this method exucutes the given command using a thread pool
     *   which has multiple threads. So the order of the command execution is
     *   unspecified.
     * </p>
     *
     * @param command  A task to be executed asynchronously.
     * @return  {@code true} is returned if the specified task was submitted.
     *          {@code false} is returned if the specified tas was rejected.
     */
    boolean executeAsync(Runnable command);

    /**
     * Return the IP address of the controller in the cluster.
     *
     * @return  The IP address of the controller.
     */
    InetAddress getControllerAddress();

    /**
     * Return the number of remote cluster nodes.
     *
     * <p>
     *   Zero is returned if no remote node was found in the cluster.
     * </p>
     *
     * @return  The number of remote cluster nodes.
     */
    int getRemoteClusterSize();

    /**
     * Determine whether the given IP address is one of remote cluster node
     * addresses or not.
     *
     * @param addr  IP address to be tested.
     * @return  {@code true} is returned if the given IP address is one of
     *          remote cluster node address.
     *          Otherwise {@code false} is returned.
     */
    boolean isRemoteClusterAddress(InetAddress addr);

    /**
     * Clean up resources associated with the given container.
     *
     * @param containerName  The name of the container.
     */
    void cleanUp(String containerName);
}
