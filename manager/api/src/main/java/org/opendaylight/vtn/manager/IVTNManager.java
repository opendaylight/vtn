/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.net.InetAddress;
import java.util.List;
import java.util.Set;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.utils.Status;

/**
 * {@code IVTNManager} provides methods for application to access
 * Virtual Tenant Network (VTN) resources in a container.
 */
public interface IVTNManager {
    /**
     * Determine whether the Virtual Tenant Network is active in the container.
     *
     * @return  {@code true} is returned if the VTN is active in the container.
     *          Otherwise {@code false} is returned.
     */
    public boolean isActive();

    /**
     * Return a list of virtual tenant information.
     *
     * @return  A list of virtual tenant information.
     * @throws VTNException  An error occurred.
     */
    public List<VTenant> getTenants() throws VTNException;

    /**
     * Return the tenant information specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  Information about the specified tenant.
     * @throws VTNException  An error occurred.
     */
    public VTenant getTenant(VTenantPath path) throws VTNException;

    /**
     * Add a new virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @return  "Success" or failure reason.
     */
    public Status addTenant(VTenantPath path, VTenantConfig tconf);

    /**
     * Modify configuration of existing virtual tenant.
     *
     * @param path   Path to the virtual tenant.
     * @param tconf  Tenant configuration
     * @param all    If {@code true} is specified, all attributes of the
     *               tenant are modified. In this case, {@code null} in
     *               {@code tconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code tconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    public Status modifyTenant(VTenantPath path, VTenantConfig tconf,
                               boolean all);

    /**
     * Remove the virtual tenant specified by the given name.
     *
     * @param path  Path to the virtual tenant.
     * @return  "Success" or failure reason.
     */
    public Status removeTenant(VTenantPath path);

    /**
     * Return a list of virtual L2 bridges in the specified tenant.
     *
     * @param path  Path to the virtual tenant.
     * @return  A list of virtual L2 bridges.
     * @throws VTNException  An error occurred.
     */
    public List<VBridge> getBridges(VTenantPath path) throws VTNException;

    /**
     * Return information about the specified virtual L2 bridge.
     *
     * @param path  Path to the virtual bridge.
     * @return  Information about the specified L2 bridge.
     * @throws VTNException  An error occurred.
     */
    public VBridge getBridge(VBridgePath path) throws VTNException;

    /**
     * Add a new virtual L2 bridge.
     *
     * @param path   Path to the virtual bridge to be added.
     * @param bconf  Bridge configuration.
     * @return  "Success" or failure reason.
     */
    public Status addBridge(VBridgePath path, VBridgeConfig bconf);

    /**
     * Modify configuration of existing virtual L2 bridge.
     *
     * @param path   Path to the virtual bridge.
     * @param bconf  Bridge configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               bridge are modified. In this case, {@code null} in
     *               {@code bconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code bconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    public Status modifyBridge(VBridgePath path, VBridgeConfig bconf,
                               boolean all);

    /**
     * Remove the virtual L2 bridge specified by the given name.
     *
     * @param path  Path to the virtual bridge.
     * @return  "Success" or failure reason.
     */
    public Status removeBridge(VBridgePath path);

    /**
     * Return a list of virtual interfaces attached to the specified virtual
     * L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of virtual interfaces.
     * @throws VTNException  An error occurred.
     */
    public List<VInterface> getBridgeInterfaces(VBridgePath path)
        throws VTNException;

    /**
     * Return information about the specified virtual interface attached to
     * the virtual L2 bridge.
     *
     * @param path  Path to the interface.
     * @return  Information about the specified interface.
     * @throws VTNException  An error occurred.
     */
    public VInterface getBridgeInterface(VBridgeIfPath path)
        throws VTNException;

    /**
     * Add a new virtual interface to the virtual L2 bridge.
     *
     * @param path   Path to the interface to be added.
     * @param iconf  Interface configuration.
     * @return  "Success" or failure reason.
     */
    public Status addBridgeInterface(VBridgeIfPath path,
                                     VInterfaceConfig iconf);

    /**
     * Modify configuration of existing virtual interface attached to the
     * virtual L2 bridge.
     *
     * @param path   Path to the interface.
     * @param iconf  Interface configuration.
     * @param all    If {@code true} is specified, all attributes of the
     *               interface are modified. In this case, {@code null} in
     *               {@code iconf} is interpreted as default value.
     *               If {@code false} is specified, an attribute is not
     *               modified if its value in {@code iconf} is {@code null}.
     * @return  "Success" or failure reason.
     */
    public Status modifyBridgeInterface(VBridgeIfPath path,
                                        VInterfaceConfig iconf, boolean all);

    /**
     * Remove the virtual interface from the virtual L2 bridge.
     *
     * @param path  Path to the interface.
     * @return  "Success" or failure reason.
     */
    public Status removeBridgeInterface(VBridgeIfPath path);

    /**
     * Return a list of VLAN mappings in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of VLAN mappings.
     * @throws VTNException  An error occurred.
     */
    public List<VlanMap> getVlanMaps(VBridgePath path) throws VTNException;

    /**
     * Return information about the specified VLAN mapping in the virtual
     * L2 bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  Information about the specified VLAN mapping.
     * @throws VTNException  An error occurred.
     */
    public VlanMap getVlanMap(VBridgePath path, String mapId)
        throws VTNException;

    /**
     * Add a new VLAN mapping to the virtual L2 bridge.
     *
     * @param path    Path to the bridge.
     * @param vlconf  VLAN mapping configuration.
     * @return  Information about added VLAN mapping, which includes
     *          VLAN map identifier.
     * @throws VTNException  An error occurred.
     */
    public VlanMap addVlanMap(VBridgePath path, VlanMapConfig vlconf)
        throws VTNException;

    /**
     * Remove the VLAN mapping from the virtual L2 bridge.
     *
     * @param path   Path to the bridge.
     * @param mapId  The identifier of the VLAN mapping.
     * @return  "Success" or failure reason.
     */
    public Status removeVlanMap(VBridgePath path, String mapId);

    /**
     * Return the port mapping configuration applied to the specified virtual
     * bridge interface.
     *
     * @param path  Path to the bridge interface.
     * @return  Port mapping information. {@code null} is returned if the
     *          port mapping is not configured on the specified interface.
     * @throws VTNException  An error occurred.
     */
    public PortMap getPortMap(VBridgeIfPath path) throws VTNException;

    /**
     * Create or destroy mapping between the physical switch port and the
     * virtual bridge interface.
     *
     * @param path    Path to the bridge interface.
     * @param pmconf  Port mapping configuration to be set.
     *                If {@code null} is specified, port mapping on the
     *                specified interface is destroyed.
     * @return  "Success" or failure reason.
     */
    public Status setPortMap(VBridgeIfPath path, PortMapConfig pmconf);

    /**
     * Initiate the discovery of a host base on its IP address.
     *
     * <p>
     *   If the given IP address is an IPv4 address, this method sends
     *   a broadcast ARP request to the specified virtual L2 bridges.
     *   If a host is found, it is reported to {@code HostTracker} via
     *   {@code IfHostListener}.
     * </p>
     *
     * @param addr     IP address.
     * @param pathSet  A set of destination paths of virtual L2 bridges.
     *                 If {@code null} is specified, a ARP request is sent
     *                 to all existing bridges.
     */
    public void findHost(InetAddress addr, Set<VBridgePath> pathSet);

    /**
     * Send a unicast ARP request to the specified host.
     *
     * <p>
     *   If the specified host sends a ARP reply, it is reported to
     *   {@code HostTracker} via {@code IfHostListener}.
     * </p>
     *
     * @param host  A host to be probed.
     * @return  {@code true} is returned if an ARP request was sent.
     *          Otherwise {@code false} is returned.
     */
    public boolean probeHost(HostNodeConnector host);

    /**
     * Return a list of MAC address entries learned by the specified virtual
     * L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  A list of MAC address entries.
     * @throws VTNException  An error occurred.
     */
    public List<MacAddressEntry> getMacEntries(VBridgePath path)
        throws VTNException;

    /**
     * Search for a MAC address entry from the MAC address table in the
     * specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @param addr  MAC address.
     * @return  A MAC address entry associated with the specified MAC address.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry getMacEntry(VBridgePath path, DataLinkAddress addr)
        throws VTNException;

    /**
     * Remove a MAC address entry from the MAC address table in the virtual L2
     * bridge.
     *
     * @param path  Path to the bridge.
     * @param addr  Ethernet address.
     * @return  A MAC address entry actually removed is returned.
     *          {@code null} is returned if not found.
     * @throws VTNException  An error occurred.
     */
    public MacAddressEntry removeMacEntry(VBridgePath path,
                                          DataLinkAddress addr)
        throws VTNException;

    /**
     * Flush all MAC address table entries in the specified virtual L2 bridge.
     *
     * @param path  Path to the bridge.
     * @return  "Success" or failure reason.
     */
    public Status flushMacEntries(VBridgePath path);
}
