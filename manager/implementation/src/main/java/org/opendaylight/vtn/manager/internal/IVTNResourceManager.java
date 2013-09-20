/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

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
     * Register VLAN ID for VLAN mapping.
     *
     * @param containerName  The name of the container.
     * @param path           Path to the virtual bridge.
     * @param vlan           VLAN ID to map.
     * @return  {@code null} is returned on success.
     *          On failure, fully-qualified name of the bridge which maps
     *          the specified VLAN is returned.
     */
    public String registerVlanMap(String containerName, VBridgePath path,
                                  short vlan);

    /**
     * Unregister VLAN mapping.
     *
     * @param vlan  VLAN ID.
     */
    public void unregisterVlanMap(short vlan);

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
    public String registerPortMap(String containerName, VBridgeIfPath path,
                                  PortVlan pvlan);

    /**
     * Unregister port mapping.
     *
     * @param pvlan  Identifier of the mapped switch port.
     */
    public void unregisterPortMap(PortVlan pvlan);

    /**
     * Determine whether the given switch port is mapped to the virtual
     * interface or not.
     *
     * @param pvlan  A pair of the switch port and the VLAN ID.
     * @return  {@code true} is returned only if the given switch port is
     *          mapped to the virtual interface.
     *          Otherwise {@code false} is returned.
     */
    public boolean isPortMapped(PortVlan pvlan);

    /**
     * Return the global timer.
     *
     * @return  The global timer.
     */
    public Timer getTimer();

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
    public boolean executeAsync(Runnable command);

    /**
     * Return the number of remote cluster nodes.
     *
     * <p>
     *   Zero is returned if no remote node was found in the cluster.
     * </p>
     *
     * @return  The number of remote cluster nodes.
     */
    public int getRemoteClusterSize();

    /**
     * Clean up resources associated with the given container.
     *
     * @param containerName  The name of the container.
     */
    public void cleanUp(String containerName);
}
