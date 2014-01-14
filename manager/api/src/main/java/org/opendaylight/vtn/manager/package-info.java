/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
 *       Please note the following caution while configuring VLAN mapping.
 *     </p>
 *     <ul>
 *       <li>
 *         VLAN ID (including 0), which was mapped in VLAN mapping, will be
 *         exclusive for that vBridge. It is not possible to map a VLAN ID
 *         to another vBridge if it has been already mapped to a vBridge.
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
 *     </ul>
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
 *         Create a vBridge with the name <strong>bridge-1</strong> and map
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
 *       be treated as an input of <strong>bridge-1</strong>.
 *     </p>
 *     <ul>
 *       <li>
 *         When a packet is sent from alias <strong>A:1</strong>, then an
 *         ethernet frame that has source MAC address as <strong>A</strong> and
 *         VLAN ID <strong>1</strong> will be input to bridge-1.
 *       </li>
 *       <li>
 *         When a packet is sent from alias <strong>A:2</strong>, then an
 *         ethernet frame that has source MAC address as <strong>A</strong> and
 *         VLAN ID <strong>2</strong> will be input to bridge-1.
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
 * <h3 id="vInterface" style="border-bottom: 2px solid #aaaaaa;">
 *   Virtual interface
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     <strong>Virtual interface</strong> shows the input and output interface
 *     that could be set in virtual node of <a href="#VTN">VTN</a>.
 *     At this point in time, it is possible to set virtual interface to
 *     <a href="#vBridge">vBridge</a> alone.
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
 *   <h4 style="border-bottom: 1px dashed #aaaaaa;">
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
 *     <h5 style="border-bottom: 1px dotted #aaaaaa;">
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
 *         <strong>bridge-1</strong> and <strong>bridge-2</strong> are
 *         configured like shown below.
 *       </p>
 *       <ul>
 *         <li>
 *           Configure port mapping in <strong>bridge-1</strong>
 *           <ul>
 *             <li>
 *               Specify physical port <strong>port-1</strong> of switch
 *               <strong>switch-1</strong>.
 *             </li>
 *             <li>Specify <strong>10</strong> in VLAN ID.</li>
 *           </ul>
 *         </li>
 *         <li>
 *           Configure VLAN mapping in <strong>bridge-2</strong>
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
 *           as an input against <strong>bridge-1</strong>.
 *           <ul>
 *             <li>
 *               It is never treated as input against
 *               <strong>bridge-2</strong>.
 *             </li>
 *           </ul>
 *         </li>
 *         <li>
 *           If any port, except for the physical port <strong>port-1</strong>
 *           of switch <strong>switch-1</strong>, receives an ethernet frame
 *           with VLAN ID <strong>10</strong>, then that ethernet frame is
 *           treated as an input against <strong>bridge-2</strong>.
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
package org.opendaylight.vtn.manager;
