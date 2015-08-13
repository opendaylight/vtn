/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.adapters.XmlJavaTypeAdapter;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.NodeRoute;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodePathAdapter;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.flow.action.DropAction;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.PopVlanAction;
import org.opendaylight.vtn.manager.flow.action.PushVlanAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanIdAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.util.NumberUtils;

/**
 * This class describes an end-to-end data flow configured by the VTN Manager.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"id": 123,
 * &nbsp;&nbsp;"creationTime": 1402399374808,
 * &nbsp;&nbsp;"idleTimeout": 300,
 * &nbsp;&nbsp;"hardTimeout": 0,
 * &nbsp;&nbsp;"ingressNode": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"tenant": "vtn_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"bridge": "bridge_1"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"ingressPort": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:03"
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s3-eth1"
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"egressNode": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"tenant": "vtn_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"bridge": "bridge_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"interface": "if_h1"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"egressPort": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:04"
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s4-eth2"
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"match": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"ethernet": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"src": "00:11:22:33:44:55",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"dst": "00:aa:bb:cc:dd:ee",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"vlan": 10
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"actions": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"popvlan": {}
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"dlsrc": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"address": "f0:f1:f2:f3:f4:f5"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;],
 * &nbsp;&nbsp;"vnoderoute": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"reason": "VLANMAPPED",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"path": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"tenant": "vtn_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"bridge": "bridge_1"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"reason": "FORWARDED",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"path": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"tenant": "vtn_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"bridge": "bridge_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"interface": "if_h1"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;],
 * &nbsp;&nbsp;"noderoute": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:03"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"input": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s3-eth1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "1"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"output": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "3",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s3-eth3"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:04"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"input": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "3",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s4-eth3"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"output": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s4-eth2"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;],
 * &nbsp;&nbsp;"statistics": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"bytes": 638,
 * &nbsp;&nbsp;&nbsp;&nbsp;"packets": 7,
 * &nbsp;&nbsp;&nbsp;&nbsp;"duration": 36510
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"averagedStats": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"bytes": 345.32,
 * &nbsp;&nbsp;&nbsp;&nbsp;"packets": 3.8,
 * &nbsp;&nbsp;&nbsp;&nbsp;"start": 1428516341931,
 * &nbsp;&nbsp;&nbsp;&nbsp;"end": 1428516352034
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "dataflow")
@XmlAccessorType(XmlAccessType.NONE)
public final class DataFlow implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -9014546462665468598L;

    /**
     * An identifier of this data flow assigned by the VTN Manager.
     */
    @XmlAttribute(name = "id", required = true)
    private long  flowId;

    /**
     * The creation time of this data flow.
     *
     * <p>
     *   The creation time is represented by the number of milliseconds
     *   between the creation time and 1970-1-1 00:00:00 UTC.
     * </p>
     */
    @XmlAttribute
    private long  creationTime;

    /**
     * The idle timeout of this data flow.
     *
     * <p>
     *   <strong>0</strong> means infinite time.
     * </p>
     */
    @XmlAttribute
    private short  idleTimeout;

    /**
     * The hard timeout of this data flow.
     *
     * <p>
     *   <strong>0</strong> means infinite time.
     * </p>
     */
    @XmlAttribute
    private short  hardTimeout;

    /**
     * The location of the virtual node which maps the incoming packet.
     */
    @XmlJavaTypeAdapter(VNodePathAdapter.class)
    @XmlElement(name = "ingressNode")
    private VNodePath  ingressNodePath;

    /**
     * The location of the physical switch port where the incoming packet is
     * received.
     */
    @XmlElement
    private PortLocation  ingressPort;

    /**
     * The location of the virtual node which sends the outgoing packet to the
     * physical network.
     *
     * <p>
     *   This element is omitted if this data flow discards the packet.
     * </p>
     */
    @XmlJavaTypeAdapter(VNodePathAdapter.class)
    @XmlElement(name = "egressNode")
    private VNodePath  egressNodePath;

    /**
     * The location of the physical switch port to which the outgoing packet
     * is sent.
     *
     * <p>
     *   This element is omitted if this data flow discards the packet.
     * </p>
     */
    @XmlElement
    private PortLocation  egressPort;

    /**
     * The flow condition configured in the ingress flow entry.
     *
     * <p>
     *   This element is omitted if the flow condition is not configured.
     * </p>
     */
    @XmlElement
    private FlowMatch  match;

    /**
     * A list of {@link FlowAction} instances to be applied to the packet
     * by the egress flow entry.
     *
     * <ul>
     *   <li>
     *     Elements are sorted in order of actions in the egress flow entry.
     *   </li>
     *   <li>
     *     Type of action is determined by the name of element in the list.
     *     One or more of the following elements can be used.
     *     <dl style="margin-left: 1em;">
     *       <dt>drop
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that discards packet.
     *         The type of this element must be {@link DropAction}.
     *
     *       <dt>dlsrc
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the source MAC address in
     *         Ethernet header.
     *         The type of this element must be {@link SetDlSrcAction}.
     *
     *       <dt>dldst
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the destination MAC address in
     *         Ethernet header.
     *         The type of this element must be {@link SetDlDstAction}.
     *
     *       <dt>popvlan
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that strips the outmost VLAN tag in
     *         Ethernet frame.
     *         The type of this element must be {@link PopVlanAction}.
     *
     *       <dt>pushvlan
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that adds a VLAN tag to Ethernet frame.
     *         The type of this element must be {@link PushVlanAction}.
     *
     *       <dt>vlanid
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the VLAN ID in Ethernet header.
     *         The type of this element must be {@link SetVlanIdAction}.
     *
     *       <dt>vlanpcp
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the VLAN priority in Ethernet
     *         header.
     *         The type of this element must be {@link SetVlanPcpAction}.
     *
     *       <dt>inet4src
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the source IP address in IPv4
     *         header.
     *         The type of this element must be {@link SetInet4SrcAction}.
     *
     *       <dt>inet4dst
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the destination IP address in
     *         IPv4 header.
     *         The type of this element must be {@link SetInet4DstAction}.
     *
     *       <dt>dscp
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the DSCP value in IP header.
     *         The type of this element must be {@link SetDscpAction}.
     *
     *       <dt>tpsrc
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the source port number in
     *         TCP or UDP header.
     *         The type of this element must be {@link SetTpSrcAction}.
     *
     *       <dt>tpdst
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the destination port number in
     *         TCP or UDP header.
     *         The type of this element must be {@link SetTpDstAction}.
     *
     *       <dt>icmptype
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the ICMP type in ICMPv4 header.
     *         The type of this element must be {@link SetIcmpTypeAction}.
     *
     *       <dt>icmpcode
     *       <dd style="margin-left: 1.5em;">
     *         Specifies the action that sets the ICMP code in ICMPv4 header.
     *         The type of this element must be {@link SetIcmpCodeAction}.
     *     </dl>
     *   </li>
     *   <li>
     *     This element is omitted if a list of flow actions is not configured.
     *     An empty action list is configured if this data flow forwards
     *     packets without any modification.
     *   </li>
     * </ul>
     */
    @XmlElementWrapper
    @XmlElements({
        @XmlElement(name = "drop", type = DropAction.class),
        @XmlElement(name = "dlsrc", type = SetDlSrcAction.class),
        @XmlElement(name = "dldst", type = SetDlDstAction.class),
        @XmlElement(name = "popvlan", type = PopVlanAction.class),
        @XmlElement(name = "pushvlan", type = PushVlanAction.class),
        @XmlElement(name = "vlanid", type = SetVlanIdAction.class),
        @XmlElement(name = "vlanpcp", type = SetVlanPcpAction.class),
        @XmlElement(name = "inet4src", type = SetInet4SrcAction.class),
        @XmlElement(name = "inet4dst", type = SetInet4DstAction.class),
        @XmlElement(name = "dscp", type = SetDscpAction.class),
        @XmlElement(name = "tpsrc", type = SetTpSrcAction.class),
        @XmlElement(name = "tpdst", type = SetTpDstAction.class),
        @XmlElement(name = "icmptype", type = SetIcmpTypeAction.class),
        @XmlElement(name = "icmpcode", type = SetIcmpCodeAction.class)})
    private List<FlowAction>  actions;

    /**
     * A sequence of {@link VNodeRoute} instances which represents the packet
     * route in the virtual network.
     *
     * <ul>
     *   <li>
     *     <strong>vnoderoute</strong> elements in the
     *     <strong>vnoderoutes</strong> are sorted in order of virtual packet
     *     forwarding.
     *     <ul>
     *       <li>
     *         The first element represents the virtual node which maps the
     *         incoming packet.
     *       </li>
     *       <li>
     *         The last element represents the virtual node which sends the
     *         outgoing packet to the physical network.
     *         If this data flow discards the packet, an empty
     *         <strong>vnoderoute</strong> element is set as the last element.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     This element is omitted if the virtual packet route is not
     *     configured.
     *   </li>
     *   <li>
     *     In JSON notation, this element is represented as an array property
     *     named <strong>vnoderoute</strong>.
     *     <strong>vnoderoutes</strong> property will never appear in JSON
     *     object.
     *   </li>
     * </ul>
     */
    @XmlElementWrapper(name = "vnoderoutes")
    @XmlElement(name = "vnoderoute")
    private List<VNodeRoute>  virtualRoute;

    /**
     * A sequence of {@link NodeRoute} instances which represents the physical
     * route of the data flow.
     *
     * <ul>
     *   <li>
     *     <strong>noderoute</strong> elements in the
     *     <strong>noderoutes</strong> are sorted in order of physical packet
     *     forwarding.
     *     <ul>
     *       <li>
     *         The first element represents the physical switch where the
     *         incoming packet is received.
     *       </li>
     *       <li>
     *         The last element represents the physical switch to which the
     *         outgoing packet is sent.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     This element is omitted if this data flow discards the packet,
     *     or if the physical packet route is not configured.
     *   </li>
     *   <li>
     *     In JSON notation, this element is represented as an array property
     *     named <strong>noderoute</strong>.
     *     <strong>noderoutes</strong> property will never appear in JSON
     *     object.
     *   </li>
     * </ul>
     */
    @XmlElementWrapper(name = "noderoutes")
    @XmlElement(name = "noderoute")
    private List<NodeRoute>  physicalRoute;

    /**
     * Statistics information about this data flow.
     *
     * <ul>
     *   <li>
     *     This element is omitted if statistics information is not configured
     *     or unavailable.
     *   </li>
     * </ul>
     */
    @XmlElement
    private FlowStats  statistics;

    /**
     * Averaged flow statistics per second.
     *
     * <ul>
     *   <li>
     *     This element is omitted if statistics information is not configured
     *     or unavailable.
     *   </li>
     * </ul>
     *
     * @since  Lithium
     */
    @XmlElement
    private AveragedFlowStats  averagedStats;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private DataFlow() {
    }

    /**
     * Construct a new instance.
     *
     * <p>
     *   Note that this class is not synchronized.
     *   Concurrent access to an instance of this class by multiple threads
     *   must be synchronized externally.
     * </p>
     *
     *
     * @param id       An identifier of the data flow.
     * @param created  A long integer value which represents the creation
     *                 time of the data flow.
     * @param idle     The idle timeout value of the data flow.
     * @param hard     The hard timeout value of the data flow.
     * @param inpath   The location of the virtual node which maps the
     *                 ingress flow.
     * @param inport   A {@link PortLocation} instance which represents the
     *                 location of the ingress switch port.
     * @param outpath  The location of the virtual node which maps the
     *                 egress flow. {@code null} means that the data flow
     *                 discards the packet.
     * @param outport  A {@link PortLocation} instance which represents the
     *                 location of the egress switch port. {@code null} means
     *                 that the data flow discards the packet.
     */
    public DataFlow(long id, long created, short idle, short hard,
                    VNodePath inpath, PortLocation inport,
                    VNodePath outpath, PortLocation outport) {
        flowId = id;
        creationTime = created;
        idleTimeout = idle;
        hardTimeout = hard;
        ingressNodePath = inpath;
        ingressPort = inport;
        egressNodePath = outpath;
        egressPort = outport;
    }

    /**
     * Return an identifier of the data flow.
     *
     * @return  An identifier of the data flow.
     */
    public long getFlowId() {
        return flowId;
    }

    /**
     * Return the creation time of the data flow.
     *
     * @return  The number of milliseconds between the creation time of this
     *          data flow and 1970-1-1 00:00:00 UTC.
     */
    public long getCreationTime() {
        return creationTime;
    }

    /**
     * Return the idle timeout value of the data flow.
     *
     * @return  The number of seconds idle before expiration.
     *          <strong>0</strong> means an infinite time.
     */
    public short getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Return the hard timeout value of the data flow.
     *
     * @return  The number of seconds before expiration.
     *          <strong>0</strong> means an infinite time.
     */
    public short getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Return the location of the virtual node path which maps the ingress
     * flow.
     *
     * @return  A {@link VNodePath} instance which represents the
     *          location of the ingress virtual node.
     */
    public VNodePath getIngressNodePath() {
        return ingressNodePath;
    }

    /**
     * Return a {@link PortLocation} instance which represents the location
     * of the ingress switch port.
     *
     * @return  A {@link PortLocation} instance which represents the location
     *          of the ingress port.
     */
    public PortLocation getIngressPort() {
        return ingressPort;
    }

    /**
     * Return the location of the virtual node path which maps the egress
     * flow.
     *
     * @return  A {@link VNodePath} instance which represents the
     *          location of the egress virtual node.
     *          {@code null} is returned if the data flow discards the packet.
     */
    public VNodePath getEgressNodePath() {
        return egressNodePath;
    }

    /**
     * Return a {@link PortLocation} instance which represents the location
     * of the egress switch port.
     *
     * @return  A {@link PortLocation} instance which represents the location
     *          of the egress port.
     *          {@code null} is returned if the data flow discards the packet.
     */
    public PortLocation getEgressPort() {
        return egressPort;
    }

    /**
     * Return a {@link FlowMatch} instance which represents the flow condition
     * in the ingress flow entry.
     *
     * @return  A {@link FlowMatch} instance.
     *          {@code null} is returned if it is not configured in this
     *          instance.
     */
    public FlowMatch getMatch() {
        return match;
    }

    /**
     * Return a list of {@link FlowAction} instances to be applied to the
     * packet by the egress flow entry.
     *
     * @return  A list of {@link FlowAction} instances.
     *          {@code null} is returned if it is not configured in this
     *          instance.
     */
    public List<FlowAction> getActions() {
        return (actions == null)
            ? null
            : new ArrayList<FlowAction>(actions);
    }

    /**
     * Return a list of {@link VNodeRoute} instances which represents the
     * virtual route of the data flow.
     *
     * @return  A list of {@link VNodeRoute}.
     *          {@code null} is returned if it is not configured in this
     *          instance.
     */
    public List<VNodeRoute> getVirtualRoute() {
        return (virtualRoute == null)
            ? null
            : new ArrayList<VNodeRoute>(virtualRoute);
    }

    /**
     * Return a list of {@link NodeRoute} instances which represents the
     * physical route of the data flow.
     *
     * @return  A list of {@link NodeRoute}.
     *          {@code null} is returned if it is not configured in this
     *          instance.
     */
    public List<NodeRoute> getPhysicalRoute() {
        return (physicalRoute == null)
            ? null
            : new ArrayList<NodeRoute>(physicalRoute);
    }

    /**
     * Return a {@link FlowStats} instance which contains the statistics
     * information about the data flow.
     *
     * @return  A {@link FlowStats} instance.
     *          {@code null} is returned if it is not configured in this
     *          instance.
     */
    public FlowStats getStatistics() {
        return statistics;
    }

    /**
     * Set statistics information of the data flow.
     *
     * @param stats  A {@link FlowStats} instance which contains statistics
     *               information.
     */
    public void setStatistics(FlowStats stats) {
        statistics = stats;
    }

    /**
     * Return a {@link AveragedFlowStats} instance which contains the averaged
     * flow statistics per second.
     *
     * @return  An {@link AveragedFlowStats} instance.
     *          {@code null} is returned if it is not configured in this
     *          instance.
     * @since  Lithium
     */
    public AveragedFlowStats getAveragedStatistics() {
        return averagedStats;
    }

    /**
     * Set averaged statistics information of the data flow.
     *
     * @param stats  An {@link AveragedFlowStats} instance which contains the
     *               averaged flow statistics per second.
     * @since  Lithium
     */
    public void setAveragedStatistics(AveragedFlowStats stats) {
        averagedStats = stats;
    }

    /**
     * Set the flow match for the ingress flow entry.
     *
     * @param fmatch  A {@link FlowMatch} instance which represents the flow
     *                match configured in the ingress flow entry.
     * @since  Lithium.
     */
    public void setMatch(FlowMatch fmatch) {
        match = fmatch;
    }

    /**
     * Set the list of egress flow actions.
     *
     * @param list  A list of {@link FlowAction} instances.
     *              Note that the given list itself is configured into this
     *              instance.
     * @since  Lithium
     */
    public void setActions(List<FlowAction> list) {
        actions = list;
    }

    /**
     * Set the virtual packet routing path of the data flow.
     *
     * @param vroutes  A list of {@link VNodeRoute} instances which represents
     *                 the virtual packet routing path.
     *                 Note that the given list itself is configured into this
     *                 instance.
     */
    public void setVirtualRoute(List<VNodeRoute> vroutes) {
        virtualRoute = vroutes;
    }

    /**
     * Set the physical packet routing path of the data flow.
     *
     * @param nroutes  A list of {@link NodeRoute} instances which represents
     *                 the physical packet routing path.
     *                 Note that the given list itself is configured into this
     *                 instance.
     * @since  Lithium
     */
    public void setPhysicalRoute(List<NodeRoute> nroutes) {
        physicalRoute = nroutes;
    }

    /**
     * Determine whether the given data flow contains the same flow
     * attributes.
     *
     * @param df  The data flow to be compared.
     * @return  {@code true} only if the given data flow contains the same
     *          flow attributes.
     */
    private boolean equalsFlowAttributes(DataFlow df) {
        if (flowId != df.flowId || creationTime != df.creationTime ||
            idleTimeout != df.idleTimeout ||
            hardTimeout != df.hardTimeout) {
            return false;
        }

        return (Objects.equals(match, df.match) &&
                Objects.equals(actions, df.actions));
    }

    /**
     * Determine whether the given data flow contains the same flow statistics.
     *
     * @param df  The data flow to be compared.
     * @return  {@code true} only if the given data flow contains the same
     *          flow statistics.
     */
    private boolean equalsFlowStats(DataFlow df) {
        return (Objects.equals(statistics, df.statistics) &&
                Objects.equals(averagedStats, df.averagedStats));
    }

    /**
     * Determine whether the given data flow contains the same virtual route.
     *
     * @param df  The data flow to be compared.
     * @return  {@code true} only if the given data flow contains the same
     *          virtual route.
     */
    private boolean equalsVirtualRoute(DataFlow df) {
        return (Objects.equals(ingressNodePath, df.ingressNodePath) &&
                Objects.equals(egressNodePath, df.egressNodePath) &&
                Objects.equals(virtualRoute, df.virtualRoute));
    }

    /**
     * Determine whether the given data flow contains the same physical route.
     *
     * @param df  The data flow to be compared.
     * @return  {@code true} only if the given data flow contains the same
     *          physical route.
     */
    private boolean equalsPhysicalRoute(DataFlow df) {
        return (Objects.equals(ingressPort, df.ingressPort) &&
                Objects.equals(egressPort, df.egressPort) &&
                Objects.equals(physicalRoute, df.physicalRoute));
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof DataFlow)) {
            return false;
        }

        DataFlow flow = (DataFlow)o;
        return (equalsFlowAttributes(flow) && equalsFlowStats(flow) &&
                equalsVirtualRoute(flow) && equalsPhysicalRoute(flow));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = Objects.hash(ingressNodePath, ingressPort,
                             egressNodePath, egressPort, match, actions,
                             virtualRoute, physicalRoute, statistics,
                             averagedStats);
        return h + idleTimeout + hardTimeout + NumberUtils.hashCode(flowId) +
            NumberUtils.hashCode(creationTime);
    }
}
