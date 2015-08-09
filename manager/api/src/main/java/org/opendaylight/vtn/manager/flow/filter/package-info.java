/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/**
 * This package provides public APIs related to flow filter.
 *
 * <h2 id="function-overview" style="border-bottom: 4px double #aaaaaa; padding-top: 0.5em;">
 *   Function overview
 * </h2>
 *
 * <div style="margin-left: 1em;">
 *   <p>
 *     Flow filter provides packet filtering feature for packets forwarded
 *     in <a href="../../package-summary.html#VTN">VTN</a>.
 *     Flow filter can not only filter out the specified packets but also
 *     modify the specified packets.
 *   </p>
 *   <p>
 *     Each flow filter must specify a
 *     <a href="../cond/package-summary.html">flow condition</a> by name.
 *     If a packet matches the condition described by the flow condition in a
 *     flow filter, then actions configured in the same flow filter is applied
 *     to that packet.
 *   </p>
 * </div>
 *
 * <h3 id="type" style="border-bottom: 2px solid #aaaaaa;">
 *   Type of flow filter
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     There are three types of flow filter as follows.
 *   </p>
 *   <dl style="margin-left: 1em;">
 *     <dt id="type.PASS" style="font-weight: bold; margin-top: 0.5em;">PASS
 *     <dd style="margin-left: 2em;">
 *       Let the packet through the virtual node if the packet matches the
 *       flow condition configured in a flow filter.
 *       This type of flow filter can be used to modify the specified packets.
 *
 *     <dt id="type.DROP" style="font-weight: bold; margin-top: 0.5em;">DROP
 *     <dd style="margin-left: 2em;">
 *       Discard the packet if the packet matches the flow condition configured
 *       in a flow filter.
 *
 *     <dt id="type.REDIRECT" style="font-weight: bold; margin-top: 0.5em;">REDIRECT
 *     <dd style="margin-left: 2em;">
 *       Forward the packet to another virtual interface in the same VTN if the
 *       packet matches the flow condition configured in a flow filter.
 *       This type of flow filter also can modify the matched packet.
 *       See description about <a href="#redirect">packet redirection</a> for
 *       more details.
 *   </dl>
 * </div>
 *
 * <h3 id="actions" style="border-bottom: 2px solid #aaaaaa;">
 *   Flow action list
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     <strong>Flow action list</strong> is a list of rules to modify packet.
 *   </p>
 *   <ul>
 *     <li>
 *       When a <a href="#type.PASS">PASS</a> or a
 *       <a href="#type.REDIRECT">REDIRECT</a> flow filter is applied to
 *       a packet, flow actions configured in the same flow filter are applied
 *       to the packet in order.
 *     </li>
 *     <li>
 *       Although a <a href="#type.DROP">DROP</a> flow filter can have
 *       flow actions, they will be always ignored.
 *     </li>
 *   </ul>
 *
 *   <p>
 *     Note that modification done by flow actions in a flow filter is visible
 *     to succeeding evaluation of flow filters.
 *   </p>
 * </div>
 *
 * <h3 id="place" style="border-bottom: 2px solid #aaaaaa;">
 *   Place to configure flow filter
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     One or more flow filters can be configured in virtual node in VTN as a
 *     list, and it is evaluated when a packet is forwarded to the
 *     virtual node. Each flow filter has a unique index in the list, and
 *     they are evaluated in ascending order of indices, and only the first
 *     matched flow filter is applied to the packet.
 *     If none of flow filter in the list matches the packet, then the
 *     VTN Manager lets the packet through the virtual node without modifying
 *     the packet.
 *   </p>
 *   <p>
 *     Flow filter can be configured in the following places.
 *   </p>
 *   <dl style="margin-left: 1em;">
 *     <dt id="place.VTN" style="font-weight: bold; margin-top: 0.5em;">VTN
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when an incoming packet is
 *         mapped to the VTN. Note that the VTN flow filter list is evaluated
 *         only once before other flow filter lists are evaluated.
 *       </p>
 *
 *     <dt id="place.vBridge.in" style="font-weight: bold; margin-top: 0.5em;">vBridge (input)
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when a packet is forwarded
 *         to the specified
 *         <a href="../../package-summary.html#vBridge">vBridge</a>.
 *         This list is evaluated at the following instances.
 *       </p>
 *       <ul>
 *         <li>
 *           When a packet is forwarded from the virtual interface to the
 *           vBridge.
 *         </li>
 *         <li>
 *           When an incoming packet is mapped to the vBridge by
 *           <a href="../../package-summary.html#VLAN-map">VLAN mapping</a> or
 *           <a href="../../package-summary.html#MAC-map">MAC mapping</a>.
 *         </li>
 *       </ul>
 *
 *     <dt id="place.vBridge.out" style="font-weight: bold; margin-top: 0.5em;">vBridge (output)
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when a packet is going to be
 *         transmitted to the physical network mapped to the
 *         <a href="../../package-summary.html#vBridge">vBridge</a> by
 *         <a href="../../package-summary.html#VLAN-map">VLAN mapping</a> or
 *         <a href="../../package-summary.html#MAC-map">MAC mapping</a>.
 *         Note that this list is not evaluated when a packet is forwarded to
 *         the virtual interface in the same vBridge.
 *       </p>
 *
 *     <dt id="place.vBridge.vif.in" style="font-weight: bold; margin-top: 0.5em;">vBridge interface (input)
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when a packet is forwarded to
 *         the specified
 *         <a href="../../package-summary.html#vInterface">virtual interface</a>
 *         in the <a href="../../package-summary.html#vBridge">vBridge</a>.
 *         This list is evaluated at the following instances.
 *       </p>
 *       <ul>
 *         <li>
 *           When an incoming packet is mapped to the vBridge interface by
 *           <a href="../../package-summary.html#port-map">port mapping</a>.
 *         </li>
 *         <li>
 *           When a packet is redirected by another flow filter to the
 *           vBridge interface as an incoming packet.
 *         </li>
 *       </ul>
 *
 *     <dt id="place.vBridge.vif.out" style="font-weight: bold; margin-top: 0.5em;">vBridge interface (output)
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when a packet is going to be
 *         transmitted to the physical network mapped to the
 *         <a href="../../package-summary.html#vInterface">virtual interface</a>
 *         in the <a href="../../package-summary.html#vBridge">vBridge</a>.
 *         This list is evaluated at the following instances.
 *       </p>
 *       <ul>
 *         <li>
 *           When a packet is forwarded from the vBridge to the virtual
 *           interface.
 *         </li>
 *         <li>
 *           When a packet is redirected by another flow filter to the
 *           vBridge interface as an outgoing packet.
 *         </li>
 *       </ul>
 *
 *     <dt id="place.vTerminal.vif.in" style="font-weight: bold; margin-top: 0.5em;">vTerminal interface (input)
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when a packet is forwarded
 *         to the specified
 *         <a href="../../package-summary.html#vInterface">virtual interface</a>
 *         in the <a href="../../package-summary.html#vTerminal">vTerminal</a>.
 *         This list is evaluated at the following instances.
 *       </p>
 *       <ul>
 *         <li>
 *           When an incoming packet is mapped to the vTerminal interface by
 *           <a href="../../package-summary.html#port-map">port mapping</a>.
 *         </li>
 *         <li>
 *           When a packet is redirected by another flow filter to the
 *           vTerminal interface as an incoming packet.
 *         </li>
 *       </ul>
 *       <p>
 *         vTerminal is an isolated input/output terminal.
 *         So an incoming packet is always discarded unless it is redirected
 *         to another virtual interface by the flow filter.
 *       </p>
 *
 *     <dt id="place.vTerminal.vif.out" style="font-weight: bold; margin-top: 0.5em;">vTerminal interface (output)
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Flow filters in this list are evaluated when a packet is going to
 *         be transmitted to the physical network mapped to the
 *         <a href="../../package-summary.html#vInterface">virtual interface</a>
 *         in the <a href="../../package-summary.html#vTerminal">vTerminal</a>.
 *       </p>
 *       <p>
 *         This list is evaluated only when a packet is redirected by another
 *         flow filter to the vTerminal interface as an outgoing packet.
 *       </p>
 *   </dl>
 * </div>
 *
 * <h3 id="redirect" style="border-bottom: 2px solid #aaaaaa;">
 *   Packet redirection
 * </h3>
 * <div style="margin-left: 1em;">
 *   <p>
 *     A <a href="#type.REDIRECT">REDIRECT</a> flow filter forwards the packet
 *     to another
 *     <a href="../../package-summary.html#vInterface">virtual interface</a>
 *     in the same <a href="../../package-summary.html#VTN">VTN</a>.
 *     A REDIRECT flow filter has the following configurations.
 *   </p>
 *   <dl style="margin-left: 1em;">
 *     <dt id="redirect.destination" style="font-weight: bold; margin-top: 0.5em;">Destination virtual interface
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         The location of the destination virtual interface must be configured
 *         in every REDIRECT flow filter.
 *       </p>
 *       <ul>
 *         <li>
 *           Self-redirection (specifying the virtual interface that contains
 *           the REDIRECT flow filter itself as the destination) is always
 *           forbidden.
 *         </li>
 *         <li>
 *           If the specified destination node does not exist, every packets
 *           matched to that REDIRECT flow filter will be discarded.
 *         </li>
 *       </ul>
 *
 *     <dt id="redirect.destination" style="font-weight: bold; margin-top: 0.5em;">Direction
 *     <dd style="margin-left: 2em;">
 *       <p>
 *         Every REDIRECT flow filter must choose the direction of the packet
 *         redirection, <strong>input</strong> or <strong>output</strong>.
 *       </p>
 *       <ul>
 *         <li>
 *           <p>
 *             <strong>input</strong> means that a redirected packet should be
 *             treated as an incoming packet as if it is forwarded or mapped
 *             to the specified virtual interface.
 *           </p>
 *           <p>
 *             A list of flow filters for incoming packets configured in the
 *             destination virtual interface is evaluated against the
 *             redirected packet. If the flow filter passes the packet,
 *             the packet is forwarded to the virtual node which contains the
 *             destination virtual interface.
 *             <ul>
 *               <li>
 *                 If the destination virtual interface is attached to the
 *                 <a href="../../package-summary.html#vBridge">vBridge</a>,
 *                 then the packet is routed according to the vBridge
 *                 configuration.
 *                 Note that the source MAC address of the redirected packet
 *                 is never learned into the
 *                 <a href="../../package-summary.html#macTable">MAC address table</a>
 *                 in the vBridge.
 *               </li>
 *               <li>
 *                 If the destination virtual interface is attached to the
 *                 <a href="../../package-summary.html#vTerminal">vTerminal</a>,
 *                 then the packet is always discarded. In other words, the
 *                 packet is always discarded unless the packet is redirected
 *                 to another interface by the flow filter configured in the
 *                 destination virtual interface.
 *               </li>
 *             </ul>
 *           <p>
 *         </li>
 *         <li>
 *           <p>
 *             <strong>output</strong> means that a redirected packet should be
 *             treated as an outgoing packet as if it is going to be
 *             transmitted to the physical network mapped to the specified
 *             virtual interface.
 *           </p>
 *           <p>
 *             A list of flow filters for outgoing packets configured in the
 *             destination virtual interface is evaluated against the
 *             redirected packet. If the flow filter passes the packet,
 *             the packet is transmitted to the physical network mapped to the
 *             virtual interface by
 *             <a href="../../package-summary.html#port-map">port mapping</a>.
 *             Note that the packet is discarded if the port mapping is not
 *             configured in the virtual interface.
 *           </p>
 *         </li>
 *       </ul>
 *   </dl>
 *
 *   <h4 id="redirect.loop" style="border-bottom: 1px dashed #aaaaaa;">
 *     Packet loop detection
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       <a href="#type.REDIRECT">REDIRECT</a> flow filter should be configured
 *       not to cause the packet loop. The number of virtual node hops per a
 *       flow (the number of packet redirections per a flow) is limited to
 *       <strong>100</strong>. If the number of virtual node hops exceeds the
 *       limit, it is treated as the packet loop and then the packet is
 *       discarded.
 *     </p>
 *   </div>
 * </div>
 *
 * <h3 style="border-bottom: 2px solid #aaaaaa;">
 *   Limitations
 * </h3>
 * <div style="margin-left: 1em;">
 *   <h4 id="multicast" style="border-bottom: 1px dashed #aaaaaa;">
 *     Broadcast/Multicast packet
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Basically, flow filter can be applied to unicast packets.
 *       So flow filter ignores broadcast and multicast packets except for
 *       <a href="#type.DROP">DROP</a> flow filter.
 *     </p>
 *     <p>
 *       For example, a broadcast packet is mapped to the VTN, flow filters
 *       in the <a href="#place.VTN">VTN flow filter</a> are evaluated as
 *       follows.
 *     </p>
 *     <ul>
 *       <li>
 *         A flow filter is ignored if its type is <a href="#type.PASS">PASS</a>
 *         or <a href="#type.REDIRECT">REDIRECT</a>.
 *       </li>
 *       <li>
 *         A flow filter is evaluated if its type is
 *         <a href="#type.DROP">DROP</a>. It the broadcast packet matches that
 *         flow filter, then the packet is discarded.
 *       </li>
 *     </ul>
 *     <p>
 *       If an unicast packet is mapped to the vBridge, and its destination
 *       MAC address is not learned in the vBridge, the packet is broadcasted
 *       to all physical network elements mapped to the vBridge.
 *       In that case all <a href="#type.REDIRECT">REDIRECT</a> flow filters
 *       configured in the vBridge and vBridge interface for outgoing packets
 *       are ignored.
 *     </p>
 *   </div>
 *
 *   <h4 id="multicast" style="border-bottom: 1px dashed #aaaaaa;">
 *     Self-originated packet
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       Flow filters never affect packets originated by the VTN Manager.
 *       The following APIs may transmit self-originated ARP packet.
 *     </p>
 *     <ul>
 *       <li>
 *         {@link org.opendaylight.vtn.manager.IVTNManager#findHost(java.net.InetAddress, java.util.Set)}
 *       </li>
 *       <li>
 *         {@link org.opendaylight.vtn.manager.IVTNManager#probeHost(org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector)}
 *       </li>
 *       <li>
 *         {@link org.opendaylight.controller.hosttracker.hostAware.IHostFinder#find(java.net.InetAddress)}
 *       </li>
 *       <li>
 *         {@link org.opendaylight.controller.hosttracker.hostAware.IHostFinder#probe(org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector)}
 *       </li>
 *     </ul>
 *   </div>
 *
 *   <h4 id="multicast" style="border-bottom: 1px dashed #aaaaaa;">
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
 *   <h4 id="multicast" style="border-bottom: 1px dashed #aaaaaa;">
 *     Fragmented layer 4 packet
 *   </h4>
 *   <div style="margin-left: 1em;">
 *     <p>
 *       A flow condtition which specifies layer 4 protocol header fields
 *       (e.g. TCP/UDP port, ICMP type and code) never matches fragments of
 *       layer 4 packet except the first fragment because layer protocol header
 *       is present only in the first fragment.
 *       If a flow filter is configured with such a flow condition,
 *       it will never be applied to fragments of layer 4 packet except the
 *       first fragment.
 *     </p>
 *   </div>
 * </div>
 *
 * @since  Helium
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
package org.opendaylight.vtn.manager.flow.filter;

import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapters;

import org.opendaylight.vtn.manager.util.xml.adapters.ByteAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.DoubleAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.IntegerAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.LongAdapter;
import org.opendaylight.vtn.manager.util.xml.adapters.ShortAdapter;
