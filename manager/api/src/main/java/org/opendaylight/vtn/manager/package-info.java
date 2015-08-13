/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * This package contains public APIs provided by the VTN Manager.
 *
 * <h2 id="osgi-bundle" style="border-bottom: 4px double #aaaaaa; padding-top: 0.5em;">
 *   OSGi bundles
 * </h2>
 * <div style="margin-left: 1em;">
 *   <p>
 *     The VTN Manager consists of the following OSGi bundles.
 *   </p>
 *   <dl style="margin-left: 1em;">
 *     <dt style="font-weight: bold; margin-top: 0.5em;">manager
 *     <dd style="margin-left: 2em;">
 *       This provides public APIs. Only this bundle exposes the VTN Manager
 *       classes and interfaces to external applications.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">manager.implementation
 *     <dd style="margin-left: 2em;">
 *       This implements the VTN Manager service.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">manager.northbound
 *     <dd style="margin-left: 2em;">
 *       This implements the REST API provided by the VTN Manager.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">manager.neutron
 *     <dd style="margin-left: 2em;">
 *       This provides OpenStack Neutron API support for the VTN Manager.
 *   </dl>
 * </div>
 *
 * <h2 id="function-overview" style="border-bottom: 4px double #aaaaaa; padding-top: 0.5em;">
 *   Function overview
 * </h2>
 *
 * <h3 id="VTN" style="border-bottom: 2px solid #aaaaaa;">VTN</h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     The VTN Manager manages the virtual network at <strong>VTN</strong>
 *     (Virtual Tenant Network) level. At least one VTN should be created for
 *     setting up virtual network using the VTN Manager.
 *   </p>
 *   <p>
 *     VTN is a virtual network environment that gets created inside the
 *     container of the OpenDaylight controller. The VTN Manager sets up
 *     virtual network environment inside VTN by configuring virtual network
 *     elements (virtual node) like <a href="#vBridge">vBridge</a> in VTN.
 *     If multiple VTNs are created, then networks inside different VTNs are
 *     managed as different individual networks.
 *   </p>
 * </div>
 *
 * <h3 id="vBridge" style="border-bottom: 2px solid #aaaaaa;">vBridge</h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     <strong>vBridge</strong> is a virtual layer 2 switch inside
 *     <a href="#VTN">VTN</a>, and it represents one virtual broadcast domain.
 *     Virtual network environment in VTN is enabled when one or more vBridges
 *     are created inside VTN, and vBridge and physical network are mapped.
 *   </p>
 *   <p>
 *     Following functionalities are provided at this point in time for
 *     mapping vBridge and physical network.
 *   </p>
 *   <ul>
 *     <li><a href="#VLAN-map">VLAN mapping</a></li>
 *     <li><a href="#port-map">Port mapping</a></li>
 *     <li><a href="#MAC-map">MAC mapping</a> (Helium onwards)</li>
 *   </ul>
 *
 *   <h4 id="vBridge.status" style="border-bottom: 1px dashed #aaaaaa;">
 *     vBridge status
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       vBridge can have the following internal status.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="vBridge.status.UNKNOWN" style="font-weight: bold; margin-top: 0.5em;">UNKNOWN
 *       <dd style="margin-left: 2em;">
 *         This shows the state wherein physical network is not mapped to
 *         vBridge. vBridge status will be UNKNOWN if all the following
 *         conditions are met.
 *         <ul>
 *           <li>
 *             No <a href="#VLAN-map">VLAN mapping</a> is configured in
 *             vBridge.
 *           </li>
 *           <li>
 *             No <a href="#MAC-map">MAC mapping</a> is configured in
 *             vBridge.
 *           </li>
 *           <li>
 *             <a href="#port-map">Port mapping</a> is not set for all the
 *             enabled <a href="#vInterface">virtual interfaces</a> inside
 *             vBridge.
 *           </li>
 *         </ul>
 *
 *       <dt id="vBridge.status.DOWN" style="font-weight: bold; margin-top: 0.5em;">DOWN
 *       <dd style="margin-left: 2em;">
 *         This shows the state wherein the physical network mapped to vBridge
 *         is not operating correctly. If any of the following conditions are
 *         met, then vBridge status will be DOWN.
 *         <ul>
 *           <li>
 *             Physical switch is configured in the
 *             <a href="#VLAN-map">VLAN mapping</a> done on vBridge and that
 *             physical switch is meeting any one of the following conditions.
 *             <ul>
 *               <li>Physical switch does not exist.</li>
 *               <li>
 *                 All the ports of the physical switch are connected to some
 *                 other physical switch.
 *               </li>
 *               <li>
 *                 Except for the ports that connect two physical switches,
 *                 no other ports are operational.
 *               </li>
 *             </ul>
 *           </li>
 *           <li>
 *             <a href="#MAC-map">MAC mapping</a> is set on vBridge and
 *             the target host for MAC mapping is not yet detected.
 *           </li>
 *           <li>
 *             One or more than one interface out of the enabled
 *             <a href="#vInterface">virtual interfaces</a> inside vBridge is
 *             in <a href="#vInterface.status.DOWN">DOWN</a> state.
 *           </li>
 *           <li>
 *             There is no reachable route available while configuring
 *             networks routes mapped to vBridge.
 *           </li>
 *         </ul>
 *
 *       <dt id="vBridge.status.UP" style="font-weight: bold; margin-top: 0.5em;">UP
 *       <dd style="margin-left: 2em;">
 *             This shows the state wherein the physical network mapped to the
 *             vBridge is operating correctly.
 *     </dl>
 *   </div>
 *
 *   <h4 id="VLAN-map" style="border-bottom: 1px dashed #aaaaaa;">
 *     VLAN mapping
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       <strong>VLAN mapping</strong> functionality is used for mapping VLAN
 *       network to any specified vBridge. It is possible to set one or more
 *       than one VLAN mapping to one vBridge.
 *     </p>
 *     <p>
 *       While configuring VLAN mapping, specify the VLAN ID that is to be
 *       mapped to vBridge.
 *     </p>
 *     <ul>
 *       <li>
 *         If <strong>0</strong> is set as VLAN ID, then the ethernet frames
 *         that have no VLAN tag get mapped to vBridge.
 *         <ul>
 *           <li>
 *             If the physical switch receives an ethernet frame that does not
 *             have VLAN tag, then it is treated as an input against this
 *             vBridge.
 *           </li>
 *           <li>
 *             VLAN tag is removed if an ethernet frame is output to this
 *             vBridge.
 *           </li>
 *         </ul>
 *       </li>
 *       <li>
 *         If a value greater than <strong>0</strong> and less than
 *         <strong>4096</strong> is specified as VLAN ID, then ethernet frames
 *         that have the same VLAN ID inside the VLAN tag get mapped to this
 *         vBridge.
 *         <ul>
 *           <li>
 *             If the physical switch receives an ethernet frame that has this
 *             VLAN ID set inside the VLAN tag, then it is treated as an input
 *             against this vBridge.
 *           </li>
 *           <li>
 *             VLAN tag is added and the corresponding VLAN ID is configured
 *             when ethernet frame is output to this vBridge.
 *           </li>
 *         </ul>
 *       </li>
 *     </ul>
 *     <p>
 *       Also, it is possible to specify one physical switch for VLAN mapping.
 *       If the {@link org.opendaylight.controller.sal.core.Node}
 *       corresponding to physical switch is specified during VLAN mapping,
 *       then only the input and output of that physical switch is mapped to
 *       vBridge. If a physical switch is not specified, then it will be used
 *       for all the physical switches recognized by the OpenDaylight
 *       controller.
 *     </p>
 *     <p>
 *       Further, if the same VLAN ID specified with and without the physical
 *       switch are set to different vBridges, higher priority will be given
 *       to the VLAN mapping specified with the physical switch.
 *       E.g., let us assume that vBridge is configured as shown below.
 *     </p>
 *     <ul>
 *       <li>
 *         Create a vBridge with the name <strong>bridge_1</strong> and
 *         configure VLAN mapping with the VLAN ID <strong>1</strong> and
 *         the physical switch <strong>switch-1</strong>.
 *       </li>
 *       <li>
 *         Create a vBridge with the name <strong>bridge_2</strong> and
 *         configure VLAN mapping with the VLAN ID <strong>1</strong>.
 *       </li>
 *     </ul>
 *     <p>
 *       Following will be the behavior in this case.
 *     </p>
 *     <ul>
 *       <li>
 *         Ethernet frames with VLAN ID <strong>1</strong> will be mapped to
 *         <strong>bridge_1</strong> if they are detected at physical switch
 *         <strong>switch-1</strong>.
 *       </li>
 *       <li>
 *         Other Ethernet frames with VLAN ID <strong>1</strong>, which are
 *         not detected at <strong>switch-1</strong>, will be mapped to
 *         <strong>bridge_2</strong>.
 *       </li>
 *     </ul>
 *     <p>
 *       Please note the following caution while configuring VLAN mapping.
 *     </p>
 *     <ul>
 *       <li>
 *         VLAN, mapped by VLAN mapping, will be exclusive for that vBridge.
 *         It is not possible to configure VLAN mapping with the same settings
 *         to another vBridge.
 *       </li>
 *       <li>
 *         Input and output against the internal ports (ports that are
 *         connected to other switches) of the switch will not be managed
 *         by VLAN mapping.
 *         <ul>
 *           <li>
 *             Even if the internal port of the switch receives an ethernet
 *             frame that has the VLAN ID specified in the VLAN mapping,
 *             that ethernet frame is not treated as an input against this
 *             vBridge.
 *           </li>
 *           <li>
 *             Ethernet frame is not output to the internal port when it is
 *             output to this vBridge.
 *           </li>
 *         </ul>
 *       </li>
 *       <li>
 *         VLAN mapping is not used on the VLAN network over a switch port
 *         which has been mapped using
 *         <a href="#port-map.conflict.VLAN">Port mapping</a>.
 *       </li>
 *       <li>
 *         VLAN mapping is not used on the VLAN network over a switch port
 *         which has detected a host mapped with
 *         <a href="#MAC-map.conflict.VLAN">MAC mapping</a>.
 *       </li>
 *     </ul>
 *   </div>
 *
 *   <h4 id="MAC-map" style="border-bottom: 1px dashed #aaaaaa;">
 *     MAC mapping
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       <strong>MAC mapping</strong> functionality is used to associate
 *       <a href="#vBridge">vBridge</a> with any specified host and it is
 *       supported from Helium onwards. Only one MAC mapping can be configured
 *       on one vBridge.
 *       However, it is possible to associate multiple hosts to vBridge by
 *       using one MAC mapping.
 *     </p>
 *     <p>
 *       A combination of MAC address of host and VLAN ID is specified to
 *       map hosts in MAC mapping.
 *     </p>
 *     <ul>
 *       <li>
 *         If 0 is specified as VLAN ID, MAC addresses detected over untagged
 *         network will be mapped to vBridge.
 *       </li>
 *       <li>
 *         If VLAN ID equal to or more than 1 and equal to or less than 4095
 *         is specified, MAC addresses detected over the specified VLAN will
 *         be mapped to vBridge.
 *       </li>
 *     </ul>
 *     <p>
 *       MAC mapping has the following two access control lists.
 *       Host is specified in MAC mapping by setting host information to
 *       these access control lists.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="MAC-map.allow" style="font-weight: bold; margin-top: 0.5em;">Map Allow list
 *       <dd style="margin-left: 2em;">
 *         Set the list of hosts which are to be mapped with MAC mapping.
 *         <ul>
 *           <li>
 *             Following MAC addresses cannot be set.
 *             <ul>
 *               <li>0</li>
 *               <li>Broadcast address</li>
 *               <li>Multi-cast address</li>
 *             </ul>
 *           </li>
 *           <li>
 *             If MAC address is not specified in the host information,
 *              all the hosts detected over the specified VLAN will be mapped.
 *           </li>
 *           <li>
 *             Host information set in Map Allow list will become exclusive
 *             to that vBridge. It is not possible to set the same host
 *             information to a different MAC mapping.
 *           </li>
 *           <li>
 *             It is not possible to set multiple host information with the
 *             same MAC address inside the same Map Allow list.
 *             E.g., if host information specified with MAC address
 *             <strong>A</strong> and VLAN ID <strong>1</strong> is set in
 *             the Map Allow list, it will not be possible to add host
 *             information having MAC address <strong>A</strong> with a
 *             VLAN ID different from <strong>1</strong> to that Map Allow
 *             list.
 *           </li>
 *         </ul>
 *
 *       <dt id="MAC-map.deny" style="font-weight: bold; margin-top: 0.5em;">Map Deny list
 *       <dd style="margin-left: 2em;">
 *         Set the list of hosts which are not mapped with MAC mapping.
 *         This is used to exclude specific hosts from mapping when host
 *         information set in Map Allow list does not specify any MAC address.
 *         <ul>
 *           <li>
 *             Following MAC addresses cannot be set.
 *             <ul>
 *               <li>0</li>
 *               <li>Broadcast address</li>
 *               <li>Multi-cast address</li>
 *             </ul>
 *           </li>
 *           <li>
 *             As against Map Allow list, it is mandatory to specify MAC
 *             address.
 *           </li>
 *           <li>
 *             Map Deny list is evaluated before Map Allow list.
 *             If the same host information is specified in both Map Allow
 *             list and Map Deny list of the same MAC mapping, that host will
 *             not be mapped using the MAC mapping.
 *           </li>
 *         </ul>
 *     </dl>
 *     <p>
 *       If host information with and without MAC address are configured in
 *       different MAC mapping respectively, settings with MAC address
 *       specified are given higher priority.
 *       E.g., let us assume that vBridge is configured as shown below.
 *     </p>
 *     <ul>
 *       <li>
 *         Create vBridge with the name <strong>bridge_1</strong> and
 *         configure MAC address <strong>A</strong> and VLAN ID
 *         <strong>1</strong> in the allow access list of MAC mapping.
 *       </li>
 *       <li>
 *         Create vBridge with the name <strong>bridge_2</strong> and
 *         configure VLAN ID <strong>1</strong> in the allow access list of
 *         MAC mapping.
 *       </li>
 *     </ul>
 *     <p>
 *       Following behavior will be seen in this case.
 *     </p>
 *     <ul>
 *       <li>
 *         Host with MAC address <strong>A</strong>, on the VLAN with VLAN ID
 *         <strong>1</strong>, will be mapped to <strong>bridge_1</strong>.
 *       </li>
 *       <li>
 *         All hosts with MAC addresses other than <strong>A</strong>, on the
 *         VLAN with VLAN ID <strong>1</strong>, will be mapped to
 *         <strong>bridge_2</strong>.
 *       </li>
 *     </ul>
 *     <h5 id="MAC-map.activate" style="border-bottom: 1px dotted #aaaaaa;">
 *       Activation of MAC mapping
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         As against <a href="#VLAN-map">VLAN mapping</a> etc., MAC mapping
 *         will not be activated just by configuring it. The mapping with
 *         a host is activated when a packet sent from the host specified in
 *         MAC mapping is detected for the first time. Also, when broadcast
 *         packets are sent towards MAC mapping, the packet will be sent only
 *         to the VLAN over the switch ports that have detected a host with
 *         which mapping is activated.
 *       </p>
 *       <p>
 *         E.g., let us assume that a network isconfigured as shown below.
 *       </p>
 *       <ul>
 *         <li>
 *           Following hosts are connected to one physical switch.
 *           <ul>
 *             <li>
 *               Host with MAC address <strong>A</strong> is connected to the
 *               VLAN over <strong>port-1</strong> which has the VLAN ID
 *               <strong>10</strong>.
 *             </li>
 *             <li>
 *               Host with MAC address <strong>B</strong> is connected to the
 *               VLAN over <strong>port-2</strong> which has the VLAN ID
 *               <strong>20</strong>.
 *             </li>
 *           </ul>
 *         </li>
 *         <li>
 *           MAC mapping is configured on a <a href="#vBridge">vBridge</a>
 *           with the name <strong>bridge_1</strong>, and following host
 *           information is configured in
 *           <a href="#MAC-map.allow">Map Allow list</a>.
 *           <ul>
 *             <li>
 *               MAC address <strong>A</strong>, VLAN ID <strong>10</strong>
 *             </li>
 *             <li>
 *               MAC address <strong>B</strong>, VLAN ID <strong>20</strong>
 *             </li>
 *           </ul>
 *         </li>
 *       </ul>
 *       <p>
 *         Here, following will be the behavior if the host with MAC address
 *         <strong>A</strong> sends a broadcast packet.
 *       </p>
 *       <ol>
 *         <li>
 *           Broadcast packets with source MAC address <strong>A</strong> and
 *           VLAN ID <strong>10</strong> is detected at the switch port
 *           <strong>port-1</strong> and it is notified to VTN Manager.
 *         </li>
 *         <li>
 *           VTN Manager maps the received packet to <strong>bridge_1</strong>
 *           and mapping is activated between the host having MAC address
 *          <strong>A</strong> and VLAN ID <strong>10</strong>.
 *         </li>
 *         <li>
 *           VTN Manager will try to send the received packet to switch ports
 *           that are connected to hosts with which MAC mapping is activated.
 *           However, since there are no hosts with which the mapping is
 *           activated, except for the host which sent the broadcast packet,
 *           the packet is discarded.
 *         </li>
 *       </ol>
 *       <p>
 *         After that, following will be the behavior if host with MAC address
 *         <strong>B</strong> sends a broadcast packet.
 *       </p>
 *       <ol>
 *         <li>
 *           Broadcast packets with source MAC address <strong>B</strong> and
 *           VLAN ID <strong>20</strong> is detected at the switch port
 *           <strong>port-2</strong> and it is notified to VTN Manager.
 *         </li>
 *         <li>
 *           VTN Manager will map the received packet to
 *           <strong>bridge_1</strong> and mapping is activated between the
 *           host having MAC address <strong>B</strong> and VLAN ID
 *           <strong>20</strong>.
 *         </li>
 *         <li>
 *           VTN Manager will try to send the received packet to switch ports
 *           that are connected to hosts with which MAC mapping is activated.
 *           If we leave out the port which received the packet, only
 *           <strong>port-1</strong> is connected to a host with which
 *           MAC mapping is activated. Therefore, broadcast packet is sent
 *           only to <strong>port-1</strong>.
 *           <ul>
 *             <li>VLAN ID is overwritten to <strong>10</strong>.
 *           </ul>
 *         </li>
 *       </ol>
 *       <p>
 *         Once the MAC mapping with the host is activated, the combination
 *         of physical switch port, which detected that host, and VLAN ID
 *         will be exclusive for that vBridge. In the above mentioned
 *         example, following switch port and VLAN ID combinations will
 *         be exclusive to <strong>bridge_1</strong>.
 *       </p>
 *       <ul>
 *         <li>Port <strong>port-1</strong>, VLAN ID <strong>10</strong>
 *         <li>Port <strong>port-2</strong>, VLAN ID <strong>20</strong>
 *       </ul>
 *       <p>
 *         E.g., in the above case, let us assume that there is a host with
 *         MAC address <strong>C</strong> over the VLAN of
 *         <strong>port-1</strong> and VLAN ID <strong>10</strong>.
 *         Here, configure MAC address <strong>C</strong> and VLAN ID
 *         <strong>10</strong> on the Map Allow list of
 *         <strong>bridge_1</strong>, and after that when a packet sent from
 *         MAC address <strong>C</strong> is detected, mapping is activated
 *         between the host with MAC address <strong>C</strong> and
 *         <strong>bridge_1</strong>.
 *       </p>
 *       <p>
 *         However, if a different vBridge <strong>bridge_2</strong> is
 *         created, and MAC address <strong>C</strong> and VLAN ID
 *         <strong>10</strong> are configured in the Map Allow list of
 *         <strong>bridge_2</strong>, the mapping with
 *         <strong>bridge_2</strong> will not be activated even if the packet
 *         sent from the host with MAC address <strong>C</strong> is detected.
 *         This is because port <strong>port-1</strong> and VLAN ID
 *         <strong>10</strong> is exclusive to <strong>bridge_1</strong>.
 *         Therefore, packet sent from MAC address <strong>C</strong> is
 *         discarded.
 *       </p>
 *       <p>
 *         Further, MAC mapping will not be used for hosts detected over a
 *         VLAN which is mapped using <a href="#port-map">Port mapping</a>.
 *       </p>
 *       <p>
 *         Mapping, activated by MAC mapping, between vBridge and host will
 *         be removed at the following instances.
 *       </p>
 *       <ul>
 *         <li>When MAC mapping is deleted.</li>
 *         <li>
 *           When the vBridge on which the MAC mapping is set is deleted.
 *         </li>
 *         <li>
 *           When the <a href="#VTN">VTN</a> that contains the vBridge where
 *           MAC mapping is set is deleted.
 *         </li>
 *         <li>
 *           When MAC mapping settings are changed, and the mapped host is
 *           removed from the mapping target of MAC mapping.
 *         </li>
 *         <li>
 *           When the switch to which the mapped host is connected is deleted.
 *         </li>
 *         <li>
 *           When switch port to which the mapped host is connected is deleted.
 *         </li>
 *         <li>
 *           When link down is detected at the switch port to which the mapped
 *           host is connected.
 *         </li>
 *         <li>
 *           When the switch port to which the mapped host is connected is
 *           mapped to <a href="#vInterface">virtual interface</a> with
 *           <a href="#port-map">Port mapping</a>.
 *         </li>
 *       </ul>
 *     </div>
 *
 *     <h5 id="MAC-map.conflict.VLAN" style="border-bottom: 1px dotted #aaaaaa;">
 *       Duplicate settings of VLAN mapping
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         If a host on which MAC mapping is applied is detected over a VLAN,
 *         mapped with <a href="#VLAN-map">VLAN mapping</a>, MAC mapping will
 *         be given higher priority.
 *       </p>
 *       <p>
 *         E.g., let us assume that a network is configured as shown below.
 *       </p>
 *       <ul>
 *         <li>
 *           Host with MAC address <strong>A</strong> is connected to the
 *           untagged network over physical port <strong>port-1</strong> of
 *           switch <strong>switch-1</strong>.
 *         </li>
 *         <li>
 *           Create vBridge with the name <strong>bridge_1</strong> and
 *           configure MAC mapping.
 *           <ul>
 *             <li>
 *               Configure MAC address <strong>A</strong> and VLAN ID
 *               <strong>0</strong> to
 *               <a href="#MAC-map.allow">Map Allow list</a>.
 *             </li>
 *           </ul>
 *         </li>
 *         <li>
 *           Create vBridge with the name <strong>bridge_2</strong> and
 *           configure VLAN mapping.
 *           <ul>
 *             <li>Specify <strong>0</strong> in VLAN ID.</li>
 *           </ul>
 *         </li>
 *       </ul>
 *       <p>
 *         Following behavior will be seen in such a case.
 *       </p>
 *       <ul>
 *         <li>
 *           Untagged packet sent by the host with MAC address
 *           <strong>A</strong> will be mapped using the MAC mapping of
 *           <strong>bridge_1</strong>.
 *         </li>
 *         <li>
 *           Untagged packet sent by hosts with MAC addresses other than
 *           <strong>A</strong> will be mapped using the VLAN mapping of
 *           <strong>bridge_2</strong>.
 *         </li>
 *       </ul>
 *     </div>
 *
 *     <h5 style="border-bottom: 1px dotted #aaaaaa;">
 *       Limitations
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         MAC mapping maps physical network with vBridge at host level but,
 *         when broadcast packet is sent towards MAC mapping,
 *         it is transmitted in the VLAN over switch port. Therefore,
 *         broadcast packet may be sent to hosts that are not mapped by
 *         MAC mapping.
 *       </p>
 *       <p>
 *         E.g., let us assume that a network is configured as shown below.
 *       </p>
 *       <ul>
 *         <li>
 *           Following hosts are connected to one physical switch.
 *           <ul>
 *             <li>
 *               Two hosts with MAC addresses <strong>A</strong> and
 *               <strong>B</strong> are connected to the untagged network over
 *               the port <strong>port-1</strong>.
 *             </li>
 *             <li>
 *               Host with MAC address <strong>C</strong> is connected to the
 *               untagged network over the port <strong>port-2</strong>.
 *             </li>
 *           </ul>
 *         </li>
 *         <li>
 *           Configure MAC mapping to vBridge with the name
 *           <strong>bridge_1</strong>.
 *           <ul>
 *             <li>
 *               Configure following host information in
 *               <a href="#MAC-map.allow">Map Allow list</a>.
 *               <ul>
 *                 <li>VLAN ID <strong>0</strong></li>
 *               </ul>
 *             </li>
 *             <li>
 *               Configure following host information in
 *               <a href="#MAC-map.deny">Map Deny list</a>.
 *               <ul>
 *                 <li>
 *                   MAC address <strong>B</strong>, VLAN ID <strong>0</strong>
 *                 </li>
 *               </ul>
 *             </li>
 *           </ul>
 *         </li>
 *       </ul>
 *       <p>
 *         In this case, packets sent by host with MAC address
 *         <strong>A</strong> or <strong>C</strong> will be mapped to
 *         <strong>bridge_1</strong> but packets sent by host with MAC address
 *         <strong>B</strong> will not be mapped to <strong>bridge_1</strong>.
 *         Also, if untagged unicast packets with the destination MAC address
 *         <strong>B</strong> are detected, they will be discarded.
 *       </p>
 *       <p>
 *         However, if the host with MAC address <strong>C</strong> sends
 *         broadcast packet, it will be forwarded to the untagged network
 *         over <strong>port-1</strong> and broadcast packet will be delivered
 *         to host with MAC address <strong>B</strong> as well.
 *       </p>
 *     </div>
 *   </div>
 *
 *   <h4 id="macTable" style="border-bottom: 1px dashed #aaaaaa;">
 *     MAC address table
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Each vBridge owns a <strong>MAC address table</strong> to learn MAC
 *       addresses detected in the vBridge. The following information is
 *       stored in the MAC address table when an input is detected from the
 *       physical network mapped to the vBridge.
 *     </p>
 *     <ul>
 *       <li>Source MAC address of ethernet frame.</li>
 *       <li>Port of the physical switch that received ethernet frame.</li>
 *       <li>VLAN ID inside the ethernet frame.</li>
 *     </ul>
 *     <p>
 *       Further, if source MAC address information of ethernet frame is
 *       registered in the MAC address table, and the physical switch port
 *       that received the ethernet frame and VLAN ID is different from the
 *       information in MAC address table, then the information inside MAC
 *       address table is updated to the information of the received ethernet
 *       frame.
 *     </p>
 *     <p>
 *       If a unicast packet is sent to vBridge and that packet is notified to
 *       OpenFlow controller, then the VTN Manager checks whether the
 *       destination MAC address is registered in the MAC address table.
 *       Following flow entries are configured on the related physical switches
 *       only when it is found in the MAC address table.
 *     </p>
 *     <ul>
 *       <li>
 *         VLAN ID inside ethernet frame is overwritten with the VLAN ID
 *         stored inside the MAC address table.
 *       </li>
 *       <li>
 *         Route to port of the physical switch, which is stored inside
 *         MAC address table, is configured on each switch.
 *       </li>
 *     </ul>
 *     <p>
 *       If the destination MAC address is not found in the MAC address table,
 *       then this ethernet frame is sent to all the physical networks mapped
 *       to vBridge.
 *     </p>
 *     <ul>
 *       <li>
 *         VLAN ID can be overwritten according to the setting of
 *         <a href="#port-map">port mapping</a> and
 *         <a href="#VLAN-map">VLAN mapping</a>.
 *       </li>
 *     </ul>
 *     <p>
 *       Only destination MAC address is used as the search key while
 *       searching inside MAC address table. Thus, it is necessary to
 *       configure vBridge in such a manner that ethernet frames that have
 *       the same MAC address as the source address and different VLAN IDs
 *       configured do not flow to the same vBridge.
 *     </p>
 *     <p>
 *       For example, let us assume that the following vBridge and network
 *       devices are configured.
 *     </p>
 *     <ul>
 *       <li>
 *         Create a vBridge with the name <strong>bridge_1</strong> and map
 *         VLAN ID <strong>1</strong> and <strong>2</strong> by using
 *         <a href="#VLAN-map">VLAN mapping</a>.
 *       </li>
 *       <li>
 *         Configure <strong>A:1</strong> and <strong>A:2</strong> alias for
 *         network interface of host having the MAC address <strong>A</strong>,
 *         and allocate VLAN ID <strong>1</strong> and <strong>2</strong> to
 *         each of the aliases.
 *       </li>
 *     </ul>
 *     <p>
 *       If the above configurations are done, then ethernet frames that have
 *       the source MAC address <strong>A</strong> and different VLAN IDs will
 *       be treated as an input of <strong>bridge_1</strong>.
 *     </p>
 *     <ul>
 *       <li>
 *         When a packet is sent from alias <strong>A:1</strong>, then an
 *         ethernet frame that has source MAC address as <strong>A</strong> and
 *         VLAN ID <strong>1</strong> will be input to bridge_1.
 *       </li>
 *       <li>
 *         When a packet is sent from alias <strong>A:2</strong>, then an
 *         ethernet frame that has source MAC address as <strong>A</strong> and
 *         VLAN ID <strong>2</strong> will be input to bridge_1.
 *       </li>
 *     </ul>
 *     <p>
 *       Configuring vBridges in the above manner results in undefined
 *       behavior.
 *     </p>
 *
 *     <h5 id="macTable.aging" style="border-bottom: 1px dotted #aaaaaa;">
 *       MAC address aging
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         The VTN Manager carries out the MAC address aging process
 *         periodically against MAC addresses stored in the MAC address
 *         table. If the MAC address inside the MAC address table is not
 *         referred since the last aging process, then that MAC address
 *         information is deleted from the MAC address table.
 *       </p>
 *       <p>
 *         Interval of the MAC address aging is configurable per vBridge.
 *         <strong>600 seconds</strong> is the default value for the aging
 *         interval. That is, when we use default settings, if the MAC address
 *         information inside MAC address table is not referred for some time,
 *         then it will be deleted earliest by <strong>600 seconds</strong> and
 *         latest by <strong>1200 seconds</strong>.
 *       </p>
 *     </div>
 *   </div>
 * </div>
 *
 * <h3 id="vTerminal" style="border-bottom: 2px solid #aaaaaa;">
 *   vTerminal
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     <strong>vTerminal</strong> is isolated input and output terminal inside
 *     <a href="#VTN">VTN</a>. vTerminal can have only one
 *     <a href="#vInterface">virtual interface</a>, and it can map a physical
 *     switch port by <a href="#port-map">port mapping</a>.
 *   </p>
 *   <p>
 *     vTerminal is always used in conjunction with redirection by flow filter.
 *   </p>
 *   <ul>
 *     <li>
 *       An incoming packet from the virtual interface inside the vTerminal is
 *       always dropped unless it is redirected to other virtual node by
 *       flow filter.
 *     </li>
 *     <li>
 *       A packet is never routed to the virtual interface inside the vTerminal
 *       unless flow filter redirects the packet to that interface.
 *     </li>
 *   </ul>
 *
 *   <h4 id="vTerminal.status" style="border-bottom: 1px dashed #aaaaaa;">
 *     vTerminal status
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       vTerminal can have the following internal status.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="vTerminal.status.UNKNOWN" style="font-weight: bold; margin-top: 0.5em;">UNKNOWN
 *       <dd style="margin-left: 2em;">
 *         This shows the state wherein physical network is not mapped to
 *         vTerminal. vTerminal status will be UNKNOWN if any of the following
 *         conditions are met.
 *         <ul>
 *           <li>
 *             No <a href="#vInterface">virtual interface</a> is configured in
 *             vTerminal.
 *           </li>
 *           <li>
 *             A <a href="#vInterface">virtual interface</a> inside vTerminal
 *             is disabled.
 *           </li>
 *           <li>
 *             <a href="#port-map">Port mapping</a> is not configured in a
 *             <a href="#vInterface">virtual interface</a> inside vTerminal.
 *           </li>
 *         </ul>
 *
 *       <dt id="vTerminal.status.DOWN" style="font-weight: bold; margin-top: 0.5em;">DOWN
 *       <dd style="margin-left: 2em;">
 *         This shows the state wherein the physical network mapped to
 *         vTerminal is not operating correctly. If an enabled
 *         <a href="#vInterface">virtual interface</a> inside vTerminal is in
 *         <a href="#vInterface.status.DOWN">DOWN</a> state, then vTerminal
 *         status will be DOWN.
 *
 *       <dt id="vTerminal.status.UP" style="font-weight: bold; margin-top: 0.5em;">UP
 *       <dd style="margin-left: 2em;">
 *             This shows the state wherein the physical network mapped to the
 *             vTerminal is operating correctly.
 *     </dl>
 *   </div>
 * </div>
 *
 * <h3 id="vInterface" style="border-bottom: 2px solid #aaaaaa;">
 *   Virtual interface
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     <strong>Virtual interface</strong> shows the input and output interface
 *     that could be set in virtual node of <a href="#VTN">VTN</a>.
 *     At this point in time, it is possible to set virtual interface to
 *     <a href="#vBridge">vBridge</a> and <a href="#vTerminal">vTerminal</a>.
 *   </p>
 *   <p>
 *     It is possible to dynamically enable or disable virtual interface.
 *     If a virtual interface is disabled, then all the inputs from that
 *     virtual interface will be ignored. Also, there will be no output
 *     against that virtual interface.
 *   </p>
 *
 *   <h4 id="vInterface.status" style="border-bottom: 1px dashed #aaaaaa;">
 *     Virtual interface status
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Virtual interface can have the following internal status.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="vInterface.status.UNKNOWN" style="font-weight: bold; margin-top: 0.5em;">UNKNOWN
 *       <dd style="margin-left: 2em;">
 *         This is a state wherein other network elements are not mapped to
 *         virtual interface.
 *
 *       <dt id="vInterface.status.DOWN" style="font-weight: bold; margin-top: 0.5em;">DOWN
 *       <dd style="margin-left: 2em;">
 *         This is a state wherein the network elements mapped to virtual
 *         interface are not operating. It will be in <strong>DOWN</strong>
 *         state even when the virtual interface is disabled.
 *
 *       <dt id="vInterface.status.UP" style="font-weight: bold; margin-top: 0.5em;">UP
 *       <dd style="margin-left: 2em;">
 *         This is a state wherein the network elements mapped to virtual
 *         interface are operating.
 *     </dl>
 *   </div>
 *
 *   <h4 id="vInterface.status.vBridge"style="border-bottom: 1px dashed #aaaaaa;">
 *     vBridge interface status
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Status of the virtual interface configured in
 *       <a href="#vBridge">vBridge</a> is decided as follows.
 *     </p>
 *     <ul>
 *       <li>
 *         If the virtual interface is disabled, then interface state will
 *         be <a href="#vInterface.status.DOWN">DOWN</a> unconditionally.
 *         <ul>
 *           <li>
 *             However, state of the disabled virtual interface will not
 *             affect the <a href="#vBridge.status">status of vBridge</a> to
 *             which that interface belongs.
 *           </li>
 *         </ul>
 *       </li>
 *       <li>
 *         Status of the enabled virtual interface is decided on the following
 *         basis.
 *         <ul>
 *           <li>
 *             It will be <a href="#vInterface.status.UNKNOWN">UNKNOWN</a>
 *             state when <a href="#port-map">port mapping</a> is not
 *             configured in the virtual interface.
 *           </li>
 *           <li>
 *             It will be <a href="#vInterface.status.DOWN">DOWN</a> when port
 *             of the physical switch mapped by
 *             <a href="#port-map">port mapping</a> meets any of the following
 *             conditions.
 *             <ul>
 *               <li>Port of the physical switch is not operating.</li>
 *               <li>
 *                 Port of the physical switch is connected to another
 *                 physical switch.
 *               </li>
 *             </ul>
 *           </li>
 *           <li>
 *             It will be in <a href="#vInterface.status.UP">UP</a> state when
 *             port of physical switch mapped by
 *             <a href="#port-map">port mapping</a> is operating.
 *           </li>
 *         </ul>
 *       </li>
 *     </ul>
 *   </div>
 *
 *   <h4 id="vInterface.status.vTerminal" style="border-bottom: 1px dashed #aaaaaa;">
 *     vTerminal interface status
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Status of the virtual interface configured in vTerminal is decided as
 *       <a href="#vInterface.status.vBridge">vBridge interface status</a>
 *       is decided.
 *     </p>
 *   </div>
 *
 *   <h4 id="port-map" style="border-bottom: 1px dashed #aaaaaa;">
 *     Port mapping
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       <strong>Port mapping</strong> functionality is used to map port of
 *       physical switch to <a href="#vInterface">virtual interface</a> of
 *       <a href="#vBridge">vBridge</a>. Only one port mapping can be set for
 *       one virtual interface. If you want to map ports of multiple physical
 *       switches to one vBridge, then it is necessary to create multiple
 *       virtual interfaces in the vBridge and configure port mapping against
 *       each virtual interface.
 *     </p>
 *     <p>
 *       While configuring port mapping, specify
 *       {@link org.opendaylight.controller.sal.core.NodeConnector}
 *       corresponding to the port of physical switch and VLAN ID that is to
 *       be mapped.
 *     </p>
 *     <ul>
 *       <li>
 *         If <strong>0</strong> is set as VLAN ID, then the ethernet frames
 *         that have no VLAN tag get mapped to vBridge interface.
 *         <ul>
 *           <li>
 *             If the specified port of the physical switch receives an
 *             ethernet frame that does not have VLAN tag, then it will be
 *             treated as an input against this vBridge interface.
 *           </li>
 *           <li>
 *             VLAN tag is removed when ethernet frame is output to this
 *             vBridge interface.
 *           </li>
 *         </ul>
 *       </li>
 *       <li>
 *         If a value greater than <strong>0</strong> and less than
 *         <strong>4096</strong> is specified as VLAN ID, then ethernet frames
 *         that have the same VLAN ID inside the VLAN tag get mapped to this
 *         vBridge interface.
 *         <ul>
 *           <li>
 *             If the specified port of the physical switch receives an
 *             ethernet frame that has this VLAN ID set inside the VLAN tag,
 *             then it is treated as an input against this vBridge interface.
 *           </li>
 *           <li>
 *             VLAN tag is added and the corresponding VLAN ID is configured
 *             when ethernet frame is output to this vBridge interface.
 *           </li>
 *         </ul>
 *       </li>
 *     </ul>
 *
 *     <p>
 *       Please note the following caution while configuring port mapping.
 *     </p>
 *     <ul>
 *       <li>
 *         Combination of the physical switch port and VLAN ID (including 0)
 *         that was specified during port mapping, will be exclusive to that
 *         vBridge interface. It is not possible to map the combination of
 *         physical switch port and VLAN ID to another vBridge interface if it
 *         has been already mapped to a vBridge interface.
 *       </li>
 *       <li>
 *         Input and output against the internal ports (ports that are
 *         connected to another switch) of a switch will not be managed by
 *         port mapping.
 *         <ul>
 *           <li>
 *             If the internal port of switch is mapped to
 *             <a href="#vInterface">virtual interface</a> with port mapping,
 *             then that interface will be in
 *             <a href="#vInterface.status.DOWN">DOWN</a> state and there will
 *             be no input and output of ethernet frame.
 *           </li>
 *         </ul>
 *       </li>
 *     </ul>
 *
 *     <h5 id="port-map.conflict.VLAN" style="border-bottom: 1px dotted #aaaaaa;">
 *       Duplicate setting of VLAN mapping
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         Port mapping settings will be given higher priority if the same
 *         VLAN network, which was set in port mapping, is mapped to another
 *         <a href="#vBridge">vBridge</a> using
 *         <a href="#VLAN-map">VLAN mapping</a>.
 *       </p>
 *       <p>
 *         For example, let us assume that two vBridges with the names
 *         <strong>bridge_1</strong> and <strong>bridge_2</strong> are
 *         configured like shown below.
 *       </p>
 *       <ul>
 *         <li>
 *           Configure port mapping in <strong>bridge_1</strong>
 *           <ul>
 *             <li>
 *               Specify physical port <strong>port-1</strong> of switch
 *               <strong>switch-1</strong>.
 *             </li>
 *             <li>Specify <strong>10</strong> in VLAN ID.</li>
 *           </ul>
 *         </li>
 *         <li>
 *           Configure VLAN mapping in <strong>bridge_2</strong>
 *           <ul>
 *             <li>Do not specify physica switch.</li>
 *             <li>Specify <strong>10</strong> in VLAN ID.</li>
 *           </ul>
 *         </li>
 *       </ul>
 *       <p>
 *         In this case, it will operate like below.
 *       </p>
 *       <ul>
 *         <li>
 *           If the physical port <strong>port-1</strong> of switch
 *           <strong>switch-1</strong> receives an ethernet frame with
 *           VLAN ID <strong>10</strong>, then that ethernet frame is treated
 *           as an input against <strong>bridge_1</strong>.
 *           <ul>
 *             <li>
 *               It is never treated as input against
 *               <strong>bridge_2</strong>.
 *             </li>
 *           </ul>
 *         </li>
 *         <li>
 *           If any port, except for the physical port <strong>port-1</strong>
 *           of switch <strong>switch-1</strong>, receives an ethernet frame
 *           with VLAN ID <strong>10</strong>, then that ethernet frame is
 *           treated as an input against <strong>bridge_2</strong>.
 *         </li>
 *       </ul>
 *     </div>
 *
 *     <h5 id="port-map.conflict.MAC" style="border-bottom: 1px dotted #aaaaaa;">
 *       Duplicate setting of MAC mapping
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         If a host set in <a href="#MAC-map">MAC mapping</a> is detected over
 *         VLAN network set in Port mapping, Port mapping settings will be
 *         prioritized. If MAC mapping is already activated with the host
 *         connected to this VLAN network, all MAC mappings will get
 *         deactivated.
 *       </p>
 *       <p>
 *         E.g., let us assume that a network is configured as shown below.
 *       </p>
 *       <ul>
 *         <li>
 *           Host with MAC address <strong>A</strong> is connected to untagged
 *           network over physical port <strong>port-1</strong> of switch
 *           <strong>switch-1</strong>.
 *         </li>
 *         <li>
 *           Create <a href="#vBridge">vBridge</a> with the name
 *           <strong>bridge_1</strong> and configure MAC mapping.
 *           <ul>
 *             <li>
 *               Configure MAC address <strong>A</strong> and VLAN ID
 *               <strong>0</strong> to
 *               <a href="#MAC-map.allow">Map Allow list</a>.
 *             </li>
 *           </ul>
 *         </li>
 *       </ul>
 *       <p>
 *         Further, newly create the following vBridge.
 *       </p>
 *       <ul>
 *         <li>
 *           Create vBridge with the name <strong>bridge_2</strong> and
 *           configure Port mapping.
 *           <ul>
 *             <li>
 *               Specify physical port <strong>port-1</strong> of switch
 *               <strong>switch-1</strong>
 *             </li>
 *             <li>Specify <strong>0</strong> in VLAN ID. </li>
 *           </ul>
 *         </li>
 *       </ul>
 *       <p>
 *         Following behavior will be there in this case.
 *       </p>
 *       <ul>
 *         <li>
 *           When the Port mapping of <strong>bridge_2</strong> is set, all
 *           the MAC mappings activated on the untagged network over physical
 *           port <strong>port-1</strong> of switch <strong>switch-1</strong>
 *           will get deactivated.
 *         </li>
 *         <li>
 *           Packets sent by host with MAC address <strong>A</strong> will be
 *           mapped according to the Port mapping on <strong>bridge_2</strong>.
 *         </li>
 *       </ul>
 *     </div>
 *   </div>
 * </div>
 *
 * <h3 id="dependencies" style="border-bottom: 2px solid #aaaaaa;">
 *   Dependencies
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     The VTN Manager uses the following OSGi services provided by the
 *     OpenDaylight controller.
 *   </p>
 *
 *   <dl style="margin-left: 1em;">
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IClusterGlobalServices
 *     <dd style="margin-left: 2em;">
 *       This is used to create cache for storing information used in all
 *       containers and share management information with controllers in the
 *       cluster.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IClusterContainerServices
 *     <dd style="margin-left: 2em;">
 *       This is used to create cache for storing information used in each
 *       container and share management information with controllers in the
 *       cluster.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       ISwitchManager
 *     <dd style="margin-left: 2em;">
 *       This is used to acquire information about switches managed by the
 *       OpenDaylight controller.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       ITopologyManager
 *     <dd style="margin-left: 2em;">
 *       This is used to understand the connection state of switches managed
 *       by the OpenDaylight controller.
 *       This is mainly used to distinguish the internal port
 *       (port connecting two switches) of the switch.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IDataPacketService
 *     <dd style="margin-left: 2em;">
 *       This is used to instruct packet transmission to switch.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IForwardingRulesManager
 *     <dd style="margin-left: 2em;">
 *       This is used to set flow entry against switch.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IConnectionManager
 *     <dd style="margin-left: 2em;">
 *       This is used to determine whether physical switch is connected to
         the OpenDaylight controller or not.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IRouting
 *     <dd style="margin-left: 2em;">
 *       This is used to determine the route between switches.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IfHostListener
 *     <dd style="margin-left: 2em;">
 *       This is used to notify the host information detected inside
 *       <a href="#vBridge">vBridge</a> to hosttracker.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IfIptoHost
 *     <dd style="margin-left: 2em;">
 *       This is used to refer the host information maintained in hosttracker.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IStatisticsManager
 *     <dd style="margin-left: 2em;">
 *       This is used to get flow statistics information. (Helium onwards)
 *   </dl>
 *
 *   <p>
 *     In addition, the VTN Manager registers the following OSGi listener
 *     services.
 *   </p>
 *
 *   <dl style="margin-left: 1em;">
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       ICacheUpdateAware
 *     <dd style="margin-left: 2em;">
 *       This is used to receive instruction issued by another controller in
 *       the cluster.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IConfigurationContainerAware
 *     <dd style="margin-left: 2em;">
 *       This is used to receive instruction to store configuration
 *       information of the OpenDaylight controller.
 *       The VTN Manager will store the VTN configuration information in
 *       file when it receives the instruction.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IListenDataPacket
 *     <dd style="margin-left: 2em;">
 *       This is used to acquire the packets received by switch.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IInventoryListener
 *     <dd style="margin-left: 2em;">
 *       This is used to detect status change of switches managed by
 *       the OpenDaylight controller.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       ITopologyManagerAware
 *     <dd style="margin-left: 2em;">
 *       This is used to detect connection status change of switches managed
 *       by the OpenDaylight controller.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IContainerListener
 *     <dd style="margin-left: 2em;">
 *       This is used to detect creation and deletion of container.
 *       This OSGi listener service is registered only by the VTN Manager in
 *       the default container. The VTN Manager in the default container will
 *       get disabled if container other than the default container is created.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IListenRoutingUpdates
 *     <dd style="margin-left: 2em;">
 *       This is used to detect completion of route calculation between
 *       switches.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IFlowProgrammerListener
 *     <dd style="margin-left: 2em;">
 *       This is used to detect flow entries removed from physical switch.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       IHostFinder
 *     <dd style="margin-left: 2em;">
 *       This provides <a href="#IHostFinder">IHostFinder</a> service instead
 *       of <a href="#arphandler">arphandler</a>.
 *   </dl>
 * </div>
 *
 * <h2 id="limitations" style="border-bottom: 4px double #aaaaaa; padding-top: 0.5em;">
 *   Limitations
 * </h2>
 *
 * <h3 id="proactive" style="border-bottom: 2px solid #aaaaaa;">
 *   Proactive Mode
 * </h3>
 * <p style="margin-left: 1em;">
 *   When building virtual environment with the VTN Manager, all the switches
 *   to be used in the VTN environment have to be set to reactive mode.
 *   If you set the switch to proactive mode, the virtual network environment
 *   built by the VTN Manager will not operate properly.
 * </p>
 *
 * <h3 id="subnet" style="border-bottom: 2px solid #aaaaaa;">
 *   Subnet
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     If <a href="#VTN">VTN</a> is present inside the container, then the
 *     VTN Manager functions without using any of the subnet information set
 *     in the OpenDaylight controller.
 *     While sending broadcast packets like ARP request, the broadcast domain
 *     is decided by the settings of <a href="#vBridge">vBridge</a>.
 *   </p>
 *   <p>
 *     If VTN is not present inside the container, it refers the subnet
 *     information for providing exactly the same functionalities as
 *     <a href="#arphandler">arphandler</a>.
 *   </p>
 * </div>
 *
 * <h3 id="arphandler" style="border-bottom: 2px solid #aaaaaa;">
 *   arphandler
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     Following functionalities implemented in arphandler are inconsistent
 *     with the functionalities of the VTN Manager. Thus arphandler and
 *     the VTN Manager cannot co-exist.
 *   </p>
 *
 *   <dl style="margin-left: 1em;">
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       New host detection
 *     <dd style="margin-left: 2em;">
 *       If arphandler receives ARP or IP packets, it will notify host
 *       information to hosttracker only when the host IP address is included
 *       in the <a href="#subnet">subnet</a> information set in the
 *       OpenDaylight controller.
 *       Subnet settings by the OpenDaylight controller is inconsistent with
 *       broadcast domain settings by <a href="#vBridge">vBridge</a>.
 *
 *     <dt style="font-weight: bold; margin-top: 0.5em;">
 *       ARP proxy
 *     <dd style="margin-left: 2em;">
 *       If arphandler detects ARP request, irrespective of whether packets
 *       can actually reach the hosts, it will instead send the ARP response
 *       if the target is a known host. This operation is inconsistent with
 *       broadcast domain settings by <a href="#vBridge">vBridge</a>.
 *   </dl>
 *   <p>
 *     Thus arphandler must be uninstalled for installing the VTN Manager in
 *     OpenDaylight controller. Functionalities that are being provided by
 *     arphandler will be provided by the VTN Manager.
 *   </p>
 *   <ul>
 *     <li>
 *       New host detection notification to hosttracker will be done by the
 *       VTN Manager.
 *     </li>
 *     <li>
 *       The VTN Manager will provide the
 *       <a href="#IHostFinder">IHostFinder</a> service.
 *     </li>
 *     <li>
 *       ARP packet forwarding will be done by the VTN Manager.
 *     </li>
 *   </ul>
 *
 *   <h4 id="IHostFinder" style="border-bottom: 1px dashed #aaaaaa;">
 *     IHostFinder
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       {@code IHostFinder} interface provides the functionality for sending
 *       ARP request to search host information.
 *       Normally this OSGi service is registered by
 *       <a href="#arphandler">arphandler</a>. However, since it is necessary
 *       to uninstall arphandler for the VTN Manager, instead this service is
 *       provided by the VTN Manager.
 *     </p>
 *
 *     <h5 id="IHostFinder.find" style="border-bottom: 1px dotted #aaaaaa;">
 *       find(InetAddress)
 *     </h5>
 *     <pre style="font-family: courier, monospace; border: 1px dashed #2f6fab; background-color: #f9f9f9; padding: 1em 0 1em 0">
 *  public void find(InetAddress networkAddress)</pre>
 *     <p>
 *       Initiate the discoverty of a host based on its IP address.
 *     </p>
 *     <p>
 *       <span style="text-decoration: underline">networkAddress</span> is
 *       an {@link java.net.InetAddress} object that indicates the IP address
 *       that you want to search. Currently this method sends a broadcast ARP
 *       request to search for the specified host. So this method will return
 *       without doing anything if {@code null} or an IPv6 address is passed to
 *       <span style="text-decoration: underline">networkAddress</span>.
 *     </p>
 *     <p>
 *       The behavior of this method will vary depending upon whether
 *       <a href="#VTN">VTN</a> is present in the container or not.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt style="font-weight: bold; margin-top: 0.5em;">
 *         If VTN is present inside the container
 *       <dd style="margin-left: 2em;">
 *         A broadcast ARP request is sent to all the
 *         <a href="#vBridge">vBridge</a> present inside the container.
 *
 *       <dt style="font-weight: bold; margin-top: 0.5em;">
 *         If VTN is not present inside the container
 *       <dd style="margin-left: 2em;">
 *         This method will behave exactly as the same method provided by
 *         <a href="#arphandler">arphandler</a>.
 *         <ul>
 *           <li>
 *             Only when the <a href="#subnet">subnet</a> information that
 *             includes
 *             <span style="text-decoration: underline">networkAddress</span>
 *             is registered, a broadcast ARP request is sent to the port of
 *             the switch defined by that subnet information.
 *           </li>
 *         </ul>
 *     </dl>
 *
 *     <h5 id="IHostFinder.probe" style="border-bottom: 1px dotted #aaaaaa;">
 *       probe(HostNodeConnector)
 *     </h5>
 *     <pre style="font-family: courier, monospace; border: 1px dashed #2f6fab; background-color: #f9f9f9; padding: 1em 0 1em 0">
 *  public void probe(HostNodeConnector host)</pre>
 *     <p>
 *       Check to see if a learned host is still in the network.
 *     </p>
 *     <p>
 *       <span style="text-decoration: underline">host</span> is a
 *       {@code HostNodeConnector} object corresponding to the target host.
 *       If a non-{@code null} value is specified, this method sends a unicast
 *       ARP request to the host specified by
 *       <span style="text-decoration: underline">host</span>.
 *       This method will return without doing anything if {@code null} is
 *       passed to <span style="text-decoration: underline">host</span>.
 *     </p>
 *     <p>
 *       The behavior of this method will vary depending upon whether
 *       <a href="#VTN">VTN</a> is present in the container or not.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt style="font-weight: bold; margin-top: 0.5em;">
 *         If VTN is present inside the container
 *       <dd style="margin-left: 2em;">
 *         This method carries out the same process as
 *         {@link org.opendaylight.vtn.manager.IVTNManager#probeHost(org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector) IVTNManager.probeHost(HostNodeConnector)}.
 *
 *       <dt style="font-weight: bold; margin-top: 0.5em;">
 *         If VTN is not present inside the container
 *       <dd style="margin-left: 2em;">
 *         This method will behave exactly as the same method provided by
 *         <a href="#arphandler">arphandler</a>.
 *         <ul>
 *           <li>
 *             Only when the <a href="#subnet">subnet</a> information that
 *             includes IP address of
 *             <span style="text-decoration: underline">host</span> is
 *             registered, a unicast ARP request is sent to the port of
 *             the switch corresponding to {@code NodeConnector} configured
 *             in <span style="text-decoration: underline">host</span>.
 *           </li>
 *         </ul>
 *     </dl>
 *   </div>
 * </div>
 *
 * <h3 id="appflow" style="border-bottom: 2px solid #aaaaaa;">
 *   Applications for setting flow entry
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     The VTN manager cannot successfully build an isolated virtual network
 *     environment if an application that sets flow entry to switch is
 *     running because this might be inconsistent with flow entry set by
 *     the VTN manager.
 *   </p>
 *   <p>
 *     The following applications in the OpenDaylight controller set flow
 *     entry for switch.
 *   </p>
 *   <ul>
 *     <li>
 *       <strong>simpleforwarding</strong>
 *       (opendaylight/samples/simpleforwarding)
 *     </li>
 *     <li>
 *       <strong>loadbalancer</strong>
 *       (opendaylight/samples/loadbalancer)
 *     </li>
 *   </ul>
 *   <p>
 *     Consequently, you cannot use the VTN Manager simultaneously with
 *     the above applications. The VTN Manager will not run successfully
 *     especially if you do not stop or uninstall
 *     <strong>simpleforwarding</strong> because it sets flow entry
 *     automatically after detecting host information.
 *   </p>
 * </div>
 */
@XmlJavaTypeAdapters({
    @XmlJavaTypeAdapter(value = ByteAdapter.class, type = Byte.class),
    @XmlJavaTypeAdapter(value = ByteAdapter.class, type = byte.class),
    @XmlJavaTypeAdapter(value = ShortAdapter.class, type = Short.class),
    @XmlJavaTypeAdapter(value = ShortAdapter.class, type = short.class),
    @XmlJavaTypeAdapter(value = IntegerAdapter.class, type = Integer.class),
    @XmlJavaTypeAdapter(value = IntegerAdapter.class, type = int.class),
    @XmlJavaTypeAdapter(value = LongAdapter.class, type = Long.class),
    @XmlJavaTypeAdapter(value = LongAdapter.class, type = long.class),
    @XmlJavaTypeAdapter(value = DoubleAdapter.class, type = double.class),
    @XmlJavaTypeAdapter(value = DoubleAdapter.class, type = Double.class),
})
package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapters;

import org.opendaylight.vtn.manager.util.xml.adapters.ByteAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.DoubleAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.IntegerAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.LongAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.ShortAdapter;
