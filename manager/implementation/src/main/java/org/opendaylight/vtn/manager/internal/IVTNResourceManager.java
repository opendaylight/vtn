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
     * Return the global timer.
     *
     * @return  The global timer.
     */
    public Timer getTimer();

    /**
     * Clean up resources associated with the given container.
     *
     * @param containerName  The name of the container.
     */
    public void cleanUp(String containerName);
}
