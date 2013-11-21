/*
 * Copyright (c) 2013 NEC Corporation
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
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

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
     * Register VLAN ID for VLAN mapping.
     *
     * @param containerName  The name of the container.
     * @param path           Path to the virtual bridge.
     * @param vlan           VLAN ID to map.
     * @return  {@code null} is returned on success.
     *          On failure, fully-qualified name of the bridge which maps
     *          the specified VLAN is returned.
     */
    String registerVlanMap(String containerName, VBridgePath path, short vlan);

    /**
     * Unregister VLAN mapping.
     *
     * @param vlan  VLAN ID.
     */
    void unregisterVlanMap(short vlan);

    /**
     * Register mapping between physical switch port and virtual bridge
     * interface.
     *
     * @param containerName  The name of the container.
     * @param path           Path to the virtual bridge interface.
     * @param pvlan          Identifier of the mapped switch port.
     * @return  {@code null} is returned on success.
     *          On failure, fully-qualified name of the bridge interface
     *          which maps the specified physical switch port is returned.
     */
    String registerPortMap(String containerName, VBridgeIfPath path,
                           PortVlan pvlan);

    /**
     * Unregister port mapping.
     *
     * @param pvlan  Identifier of the mapped switch port.
     */
    void unregisterPortMap(PortVlan pvlan);

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
