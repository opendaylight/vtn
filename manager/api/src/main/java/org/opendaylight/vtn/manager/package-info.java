/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * The VTN Manager provides a network virtualization support with
 * multi-tenancy.
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
 *     VTN is a virtual network environment inside the network manager by the
 *     OpenDaylight controller. The VTN Manager sets up virtual network
 *     environment inside VTN by configuring virtual network elements
 *     (virtual node) like <a href="#vBridge">vBridge</a> in VTN.
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
 *       If the node identifier (node-id) corresponding to physical switch
 *       is specified during VLAN mapping, then only the input and output of
 *       that physical switch is mapped to vBridge. If a physical switch is
 *       not specified, then it will be used for all the physical switches
 *       recognized by the OpenDaylight controller.
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
 *       While configuring port mapping, specify a pair of physical switch port
 *       and VLAN ID that is to be mapped.
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
 * <h3 id="flow.cond" style="border-bottom: 2px solid #aaaaaa;">
 *   Flow Condition
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     A flow condition is a named list of flow match conditions, and it is
 *     used to select packets. Each flow match condition must have a match
 *     index, which is an unique index in a flow condition. When a flow
 *     condition tests a packet, flow match conditions in a flow condition are
 *     evaluated in ascending order of match indices. A packet is selected
 *     if at least one flow match condition matches the packet.
 *   </p>
 *   <p>
 *     Flow conditions are shared with all the VTNs.
 *   </p>
 * </div>
 *
 * <h3 id="flow.filter" style="border-bottom: 2px solid #aaaaaa;">
 *   Flow Filter
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     Flow filter provides packet filtering feature for packets forwarded
 *     in <a href="#VTN">VTN</a>.
 *     Flow filter can not only filter out the specified packets but also
 *     modify the specified packets.
 *   </p>
 *   <p>
 *     Each flow filter must specify a <a href="#flow.cond">flow condition</a>
 *     by name. If a packet matches the condition described by the flow
 *     condition in a flow filter, then actions configured in the same flow
 *     filter is applied to that packet.
 *   </p>
 *
 *   <h4 id="flow.filter.type" style="border-bottom: 1px dashed #aaaaaa;">
 *     Type of flow filter
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       There are three types of flow filter as follows.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="flow.filter.type.PASS" style="font-weight: bold; margin-top: 0.5em;">PASS
 *       <dd style="margin-left: 2em;">
 *         Let the packet through the virtual node if the packet matches the
 *         flow condition configured in a flow filter.
 *         This type of flow filter can be used to modify the specified
 *         packets.
 *
 *       <dt id="flow.filter.type.DROP" style="font-weight: bold; margin-top: 0.5em;">DROP
 *       <dd style="margin-left: 2em;">
 *         Discard the packet if the packet matches the flow condition
 *         configured in a flow filter.
 *
 *       <dt id="flow.filter.type.REDIRECT" style="font-weight: bold; margin-top: 0.5em;">REDIRECT
 *       <dd style="margin-left: 2em;">
 *         Forward the packet to another virtual interface in the same VTN
 *         if the packet matches the flow condition configured in a flow
 *         filter. This type of flow filter also can modify the matched packet.
 *         See description about
 *         <a href="#flow.filter.redirect">packet redirection</a> for
 *         more details.
 *     </dl>
 *   </div>
 *
 *   <h4 id="flow.filter.actions" style="border-bottom: 1px dashed #aaaaaa;">
 *     Flow action list
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       <strong>Flow action list</strong> is a list of rules to modify packet.
 *     </p>
 *     <ul>
 *       <li>
 *         When a <a href="#flow.filter.type.PASS">PASS</a> or a
 *         <a href="#flow.filter.type.REDIRECT">REDIRECT</a> flow filter is
 *         applied to a packet, flow actions configured in the same flow
 *         filter are applied to the packet in order.
 *       </li>
 *       <li>
 *         Although a <a href="#flow.filter.type.DROP">DROP</a> flow filter can
 *         have flow actions, they will be always ignored.
 *       </li>
 *     </ul>
 *
 *     <p>
 *       Note that modification done by flow actions in a flow filter is
 *        visible to succeeding evaluation of flow filters.
 *     </p>
 *   </div>
 *
 *   <h4 id="flow.filter.place" style="border-bottom: 1px dashed #aaaaaa;">
 *     Place to configure flow filter
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       One or more flow filters can be configured in virtual node in VTN as
 *       a list, and it is evaluated when a packet is forwarded to the
 *       virtual node. Each flow filter has a unique index in the list, and
 *       they are evaluated in ascending order of indices, and only the first
 *       matched flow filter is applied to the packet.
 *       If none of flow filter in the list matches the packet, then the
 *       VTN Manager lets the packet through the virtual node without modifying
 *       the packet.
 *     </p>
 *     <p>
 *       Flow filter can be configured in the following places.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="flow.filter.place.VTN" style="font-weight: bold; margin-top: 0.5em;">VTN
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when an incoming packet is
 *           mapped to the VTN. Note that the VTN flow filter list is evaluated
 *           only once before other flow filter lists are evaluated.
 *         </p>
 *
 *       <dt id="flow.filter.place.vBridge.in" style="font-weight: bold; margin-top: 0.5em;">vBridge (input)
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when a packet is forwarded
 *           to the specified <a href="#vBridge">vBridge</a>.
 *           This list is evaluated at the following instances.
 *         </p>
 *         <ul>
 *           <li>
 *             When a packet is forwarded from the virtual interface to the
 *             vBridge.
 *           </li>
 *           <li>
 *             When an incoming packet is mapped to the vBridge by
 *             <a href="#VLAN-map">VLAN mapping</a> or
 *             <a href="#MAC-map">MAC mapping</a>.
 *           </li>
 *         </ul>
 *
 *       <dt id="flow.filter.place.vBridge.out" style="font-weight: bold; margin-top: 0.5em;">vBridge (output)
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when a packet is going to
 *           be transmitted to the physical network mapped to the
 *           <a href="#vBridge">vBridge</a> by <a href="#VLAN-map">VLAN
 *           mapping</a> or <a href="#MAC-map">MAC mapping</a>.
 *           Note that this list is not evaluated when a packet is forwarded to
 *           the virtual interface in the same vBridge.
 *         </p>
 *
 *       <dt id="flow.filter.place.vBridge.vif.in" style="font-weight: bold; margin-top: 0.5em;">vBridge interface (input)
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when a packet is forwarded
 *           to the specified
 *           <a href="#vInterface">virtual interface</a>   in the
 *           <a href="#vBridge">vBridge</a>.
 *           This list is evaluated at the following instances.
 *         </p>
 *         <ul>
 *           <li>
 *             When an incoming packet is mapped to the vBridge interface by
 *             <a href="#port-map">port mapping</a>.
 *           </li>
 *           <li>
 *             When a packet is redirected by another flow filter to the
 *             vBridge interface as an incoming packet.
 *           </li>
 *         </ul>
 *
 *       <dt id="flow.filter.place.vBridge.vif.out" style="font-weight: bold; margin-top: 0.5em;">vBridge interface (output)
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when a packet is going to
 *           be transmitted to the physical network mapped to the
 *           <a href="#vInterface">virtual interface</a>
 *           in the <a href="#vBridge">vBridge</a>.
 *           This list is evaluated at the following instances.
 *         </p>
 *         <ul>
 *           <li>
 *             When a packet is forwarded from the vBridge to the virtual
 *             interface.
 *           </li>
 *           <li>
 *             When a packet is redirected by another flow filter to the
 *             vBridge interface as an outgoing packet.
 *           </li>
 *         </ul>
 *
 *       <dt id="flow.filter.place.vTerminal.vif.in" style="font-weight: bold; margin-top: 0.5em;">vTerminal interface (input)
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when a packet is forwarded
 *           to the specified
 *           <a href="#vInterface">virtual interface</a> in the
 *           <a href="#vTerminal">vTerminal</a>.
 *           This list is evaluated at the following instances.
 *         </p>
 *         <ul>
 *           <li>
 *             When an incoming packet is mapped to the vTerminal interface by
 *             <a href="#port-map">port mapping</a>.
 *           </li>
 *           <li>
 *             When a packet is redirected by another flow filter to the
 *             vTerminal interface as an incoming packet.
 *           </li>
 *         </ul>
 *         <p>
 *           vTerminal is an isolated input/output terminal.
 *           So an incoming packet is always discarded unless it is redirected
 *           to another virtual interface by the flow filter.
 *         </p>
 *
 *       <dt id="flow.filter.place.vTerminal.vif.out" style="font-weight: bold; margin-top: 0.5em;">vTerminal interface (output)
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Flow filters in this list are evaluated when a packet is going to
 *           be transmitted to the physical network mapped to the
 *           <a href="#vInterface">virtual interface</a> in the
 *           <a href="#vTerminal">vTerminal</a>.
 *         </p>
 *         <p>
 *           This list is evaluated only when a packet is redirected by another
 *           flow filter to the vTerminal interface as an outgoing packet.
 *         </p>
 *     </dl>
 *   </div>
 *
 *   <h4 id="flow.filter.redirect" style="border-bottom: 1px dashed #aaaaaa;">
 *     Packet redirection
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       A <a href="#flow.filter.type.REDIRECT">REDIRECT</a> flow filter
 *       forwards the packet to another <a href="#vInterface">virtual
 *       interface</a> in the same <a href="#VTN">VTN</a>.
 *       A REDIRECT flow filter has the following configurations.
 *     </p>
 *     <dl style="margin-left: 1em;">
 *       <dt id="flow.filter.redirect.destination" style="font-weight: bold; margin-top: 0.5em;">Destination virtual interface
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           The location of the destination virtual interface must be
 *           configured in every REDIRECT flow filter.
 *         </p>
 *         <ul>
 *           <li>
 *             Self-redirection (specifying the virtual interface that contains
 *             the REDIRECT flow filter itself as the destination) is always
 *             forbidden.
 *           </li>
 *           <li>
 *             If the specified destination node does not exist, every packets
 *             matched to that REDIRECT flow filter will be discarded.
 *           </li>
 *         </ul>
 *
 *       <dt id="flow.filter.redirect.destination" style="font-weight: bold; margin-top: 0.5em;">Direction
 *       <dd style="margin-left: 2em;">
 *         <p>
 *           Every REDIRECT flow filter must choose the direction of the packet
 *           redirection, <strong>input</strong> or <strong>output</strong>.
 *         </p>
 *         <ul>
 *           <li>
 *             <p>
 *               <strong>input</strong> means that a redirected packet should
 *               be treated as an incoming packet as if it is forwarded or
 *               mapped to the specified virtual interface.
 *             </p>
 *             <p>
 *               A list of flow filters for incoming packets configured in the
 *               destination virtual interface is evaluated against the
 *               redirected packet. If the flow filter passes the packet,
 *               the packet is forwarded to the virtual node which contains the
 *               destination virtual interface.
 *               <ul>
 *                 <li>
 *                   If the destination virtual interface is attached to the
 *                   <a href="#vBridge">vBridge</a>, then the packet is routed
 *                   according to the vBridge configuration.
 *                   Note that the source MAC address of the redirected packet
 *                   is never learned into the
 *                   <a href="#macTable">MAC address table</a> in the vBridge.
 *                 </li>
 *                 <li>
 *                   If the destination virtual interface is attached to the
 *                   <a href="#vTerminal">vTerminal</a>, then the packet is
 *                   always discarded. In other words, the packet is always
 *                   discarded unless the packet is redirected to another
 *                   interface by the flow filter configured in the destination
 *                   virtual interface.
 *                 </li>
 *               </ul>
 *             </p>
 *           </li>
 *           <li>
 *             <p>
 *               <strong>output</strong> means that a redirected packet should
 *               be treated as an outgoing packet as if it is going to be
 *               transmitted to the physical network mapped to the specified
 *               virtual interface.
 *             </p>
 *             <p>
 *               A list of flow filters for outgoing packets configured in the
 *               destination virtual interface is evaluated against the
 *               redirected packet. If the flow filter passes the packet,
 *               the packet is transmitted to the physical network mapped to
 *               the virtual interface by
 *               <a href="#port-map">port mapping</a>.
 *               Note that the packet is discarded if the port mapping is not
 *               configured in the virtual interface.
 *             </p>
 *           </li>
 *         </ul>
 *     </dl>
 *
 *     <h5 id="flow.filter.redirect.loop" style="border-bottom: 1px dotted #aaaaaa;">
 *       Packet loop detection
 *     </h5>
 *     <div style="margin-left: 1em;">
 *       <p>
 *         <a href="#flow.filter.type.REDIRECT">REDIRECT</a> flow filter should
 *         be configured not to cause the packet loop. The number of virtual
 *         node hops per a flow (the number of packet redirections per a flow)
 *         is limited to <strong>100</strong>. If the number of virtual node
 *         hops exceeds the limit, it is treated as the packet loop and then
 *         the packet is discarded.
 *       </p>
 *     </div>
 *   </div>
 * </div>
 *
 * <h2 id="limitations" style="border-bottom: 4px double #aaaaaa; padding-top: 0.5em;">
 *   Limitations
 * </h2>
 *
 * <h3 id="limit.appflow" style="border-bottom: 2px solid #aaaaaa;">
 *   Applications for setting flow entry
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     The VTN manager cannot successfully build an isolated virtual network
 *     environment if an application that sets flow entry to switch is
 *     running because this might be inconsistent with flow entry set by
 *     the VTN manager.
 *   </p>
 * </div>
 *
 * <h3 id="limit.filter" style="border-bottom: 2px solid #aaaaaa;">
 *   Limitations on flow filter
 * </h3>
 * <div style="margin-left: 1em;">
 *   <h4 id="limit.filter.multicast" style="border-bottom: 1px dashed #aaaaaa;">
 *     Broadcast/Multicast packet
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Basically, flow filter can be applied to unicast packets.
 *       So flow filter ignores broadcast and multicast packets except for
 *       <a href="#flow.filter.type.DROP">DROP</a> flow filter.
 *     </p>
 *     <p>
 *       For example, a broadcast packet is mapped to the VTN, flow filters
 *       in the <a href="#flow.filter.place.VTN">VTN flow filter</a> are
 *       evaluated as follows.
 *     </p>
 *     <ul>
 *       <li>
 *         A flow filter is ignored if its type is
 *         <a href="#flow.filter.type.PASS">PASS</a> or
 *         <a href="#flow.filter.type.REDIRECT">REDIRECT</a>.
 *       </li>
 *       <li>
 *         A flow filter is evaluated if its type is
 *         <a href="#flow.filter.type.DROP">DROP</a>. It the broadcast packet
 *         matches that flow filter, then the packet is discarded.
 *       </li>
 *     </ul>
 *     <p>
 *       If an unicast packet is mapped to the vBridge, and its destination
 *       MAC address is not learned in the vBridge, the packet is broadcasted
 *       to all physical network elements mapped to the vBridge.
 *       In that case all <a href="#flow.filter.type.REDIRECT">REDIRECT</a>
 *       flow filters configured in the vBridge and vBridge interface for
 *       outgoing packets are ignored.
 *     </p>
 *   </div>
 *
 *   <h4 id="limit.filter.self" style="border-bottom: 1px dashed #aaaaaa;">
 *     Self-originated packet
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Flow filters never affect packets originated by the VTN Manager.
 *     </p>
 *   </div>
 *
 *   <h4 id="limit.filter.controller" style="border-bottom: 1px dashed #aaaaaa;">
 *     Packet sent to the controller
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Flow filters never affect packets sent to the controller.
 *       If the destination MAC address of the packet is equal to the
 *       controller's MAC address, the VTN Manager ignores all flow filters.
 *     </p>
 *   </div>
 *
 *   <h4 id="limit.filter.fragment" style="border-bottom: 1px dashed #aaaaaa;">
 *     Fragmented layer 4 packet
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       A flow condtition which specifies layer 4 protocol header fields
 *       (e.g. TCP/UDP port, ICMP type and code) never matches fragments of
 *       layer 4 packet except the first fragment because layer protocol
 *       header is present only in the first fragment.
 *       If a flow filter is configured with such a flow condition,
 *       it will never be applied to fragments of layer 4 packet except the
 *       first fragment.
 *     </p>
 *   </div>
 * </div>
 */
package org.opendaylight.vtn.manager;
