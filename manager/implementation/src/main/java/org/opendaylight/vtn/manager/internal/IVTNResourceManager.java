/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.Map;
import java.util.Set;

import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
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
     * @param path   Path to the virtual interface which maps the specified
     *               VLAN network.
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
    MapReference registerPortMap(VTNManagerImpl mgr, VInterfacePath path,
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
     * @param path   Path to the virtual interface.
     * @param pvlan  A {@link PortVlan} object which specifies the VLAN network
     *               to be unmapped.
     * @param purge  If {@code true} is specified, this method purges caches
     *               corresponding to the unmapped VLAN network.
     * @throws VTNException  A fatal error occurred.
     */
    void unregisterPortMap(VTNManagerImpl mgr, VInterfacePath path,
                           PortVlan pvlan, boolean purge) throws VTNException;

    /**
     * Register MAC mapping configuration.
     *
     * <p>
     *   Note that this method must be called with holding the read/write
     *   lock of {@code mgr} in writer mode.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the target MAC mapping.
     * @param change  A {@link MacMapChange} instance which keeps differences
     *                to be applied.
     * @throws MacMapConflictException
     *   At least one host configured in {@code allowAdded} is already mapped
     *   to vBridge by MAC mapping.
     * @throws VTNException  A fatal error occurred.
     */
    void registerMacMap(VTNManagerImpl mgr, MacMapPath path,
                        MacMapChange change)
        throws VTNException;

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
     * <p>
     *   Note that this method returns {@code true} if at least one MAC mapping
     *   is active on the specified VLAN network on the switch port.
     * </p>
     *
     * @param pvlan  A pair of the switch port and the VLAN ID.
     * @return  {@code true} is returned only if the given switch port is
     *          mapped to the virtual interface.
     *          Otherwise {@code false} is returned.
     */
    boolean isPortMapped(PortVlan pvlan);

    /**
     * Return a reference to virtual network mapping which maps the specified
     * host.
     *
     * <p>
     *   This method checks port mapping, MAC mapping, and VLAN mapping
     *   configurations in order, and returns a reference to the virtual
     *   mapping which maps the specified host.
     * </p>
     *
     * @param mac   A MAC address of the host.
     *              Specifying {@code null} results in undefined behavior.
     * @param nc    A node connector corresponding to the switch port where the
     *              specified host was detected.
     *              Specifying {@code null} results in undefined behavior.
     * @param vlan  A VLAN ID.
     * @return      A {@link MapReference} object is returned if found.
     *              {@code null} is returned if not found.
     */
    MapReference getMapReference(byte[] mac, NodeConnector nc, short vlan);

    /**
     * Return a reference to virtual network mapping which reserves the
     * specified VLAN network on a swtich port.
     *
     * <p>
     *   This method searches for a port or MAC mapping which reserves the
     *   specified VLAN network.
     * </p>
     *
     * @param pvlan  A {@link PortVlan} instance which represents a VLAN
     *               network on a specific switch port.
     * @return  A {@link MapReference} instance which reserves the specified
     *          VLAN network is returned.
     *          {@code null} is returned if not found.
     */
    MapReference getMapReference(PortVlan pvlan);

    /**
     * Return a reference to MAC mapping which maps the specified host.
     *
     * <p>
     *   Note that this method only sees the MAC mapping configuration.
     *   It never sees configurations for other mapping, such as port mapping.
     *   In addition, this method never checks whether the MAC mapping for
     *   the specified host is actually activated or not.
     * </p>
     *
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @return  A {@link MapReference} instance which maps the the specified
     *          host.
     *          {@code null} is returned if not found.
     */
    MapReference getMapReference(MacVlan mvlan);

    /**
     * Return a set of VLAN networks mapped by the specified MAC mapping.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the target MAC mapping.
     * @return  A set of {@link PortVlan} instances corresponding to
     *          VLAN networks on switch ports.
     */
    Set<PortVlan> getMacMappedNetworks(VTNManagerImpl mgr, MacMapPath path);


    /**
     * Return a {@link PortVlan} instance associated with the specified
     * MAC address in the MAC mapping.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the target MAC mapping.
     * @param mac   A MAC address of the host.
     * @return  A {@link PortVlan} instance if found.
     *          {@code null} if not found.
     */
    PortVlan getMacMappedNetwork(VTNManagerImpl mgr, MacMapPath path,
                                 long mac);

    /**
     * Return a map which contains information about hosts actually mapped by
     * the MAC mapping.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the target MAC mapping.
     * @return
     *    If at least one host is actually mapped by the MAC mapping. a map
     *    which contains information about hosts actually mapped by the
     *    MAC mapping is returned.
     *    The key is a {@link MacVlan} instance which indicates the L2 host,
     *    and the value is a {@link NodeConnector} instance corresponding to
     *    a switch port to which the host is connected.
     *    <p>
     *      {@code null} is returned if no host is mapped by the MAC mapping.
     *    </p>
     */
    Map<MacVlan, NodeConnector> getMacMappedHosts(VTNManagerImpl mgr,
                                                  MacMapPath path);

    /**
     * Return a {@link NodeConnector} instance associated with the specified
     * host in the MAC mapping.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the target MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which specifies the L2 host
     *               to be tested.
     * @return  A {@link NodeConnector} instance if found.
     *          {@code null} if not found.
     */
    NodeConnector getMacMappedPort(VTNManagerImpl mgr, MacMapPath path,
                                   MacVlan mvlan);

    /**
     * Determine whether at least one host is actually mapped by the specified
     * MAC mapping or not.
     *
     * @param mgr   VTN Manager service.
     * @param path  Path to the target MAC mapping.
     * @return  {@code true} is returned if at least one host is actually
     *          mapped by the MAC mapping. Otherwise {@code false} is returned.
     */
    boolean hasMacMappedHost(VTNManagerImpl mgr, MacMapPath path);

    /**
     * Activate the specified host in the MAC mapping.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the target MAC mapping.
     * @param mvlan  A {@link MacVlan} instance which represents L2 host
     *               information.
     * @param port   A {@link NodeConnector} instance corresponding to a
     *               switch port where the specified host is detected.
     * @return  {@code true} is returned if the specified MAC mapping was
     *          activated. In other words, {@code true} is returned if the
     *          specified host was activated, and it is the only active host
     *          in the MAC mapping. Otherwise {@code false} is returned.
     * @throws MacMapGoneException
     *    The specified host is no longer mapped by the target MAC mapping.
     * @throws MacMapPortBusyException
     *    The specified VLAN network on a switch port is reserved by
     *    another virtual mapping.
     * @throws MacMapDuplicateException
     *    The same MAC address as {@code mvlan} is already mapped to the
     *    same vBridge.
     * @throws VTNException  A fatal error occurred.
     */
    boolean activateMacMap(VTNManagerImpl mgr, MacMapPath path,
                           MacVlan mvlan, NodeConnector port)
        throws VTNException;

    /**
     * Inactivate all MAC mappings detected on switch ports accepted by the
     * specified port filter.
     *
     * <p>
     *   Note that this method never purges network caches corresponding to
     *   invalidated mappings, such as flow entry.
     * </p>
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the target MAC mapping.
     * @param filter  A {@link PortFilter} instance which selects switch ports.
     * @return  {@code true} is returned if at least one host is still mapped
     *          by the MAC mapping.
     *          {@code false} is returned the specified MAC mapping is
     *          no longer active.
     * @throws VTNException  A fatal error occurred.
     */
    boolean inactivateMacMap(VTNManagerImpl mgr, MacMapPath path,
                             PortFilter filter) throws VTNException;

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
     * Return a VTN Manager service in the specified container.
     *
     * @param containerName  The name of the container.
     * @return  A {@link VTNManagerImpl} instance if found.
     *          {@code null} if not found.
     */
    VTNManagerImpl getVTNManager(String containerName);
}
