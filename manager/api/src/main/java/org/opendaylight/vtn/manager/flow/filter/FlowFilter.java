/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

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

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;
import org.opendaylight.vtn.manager.util.VTNIdentifiable;

/**
 * {@code FlowFilter} class describes configuration information about
 * flow filter.
 *
 * <p>
 *   An instance of this class represents a rule of a packet filter applied
 *   to packets forwarded in the VTN. If a flow condition configured in
 *   a flow filter matches a packet, actions configured in that flow filter
 *   are applied to the packet.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"index": 10,
 * &nbsp;&nbsp;"condition": "flowcond_1",
 * &nbsp;&nbsp;"filterType": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"redirect": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"destination": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"bridge": "vbridge_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"interface": "if_2"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"output": false
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"actions": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{"dlsrc": {"address": "f0:f1:f2:f3:f4:f5"}},
 * &nbsp;&nbsp;&nbsp;&nbsp;{"vlanpcp": {"priority": 7}}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowfilter")
@XmlAccessorType(XmlAccessType.NONE)
public final class FlowFilter
    implements Serializable, VTNIdentifiable<Integer> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 3615448052561767816L;

    /**
     * An index value assigned to this flow filter.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     This value is used to determine order of evaluation.
     *     Flow filters in the same list are evaluated against packet
     *     in ascending order of indices assigned to
     *     <strong>flowfilter</strong> elements, and only the first matched
     *     flow filter is applied to the packet.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "index")
    private Integer  index;

    /**
     * The name of the flow condition that selects packets.
     *
     * <ul>
     *   <li>
     *     This flow filter will be applied to the packet if the packet is
     *     selected by the flow condition specified by this attribute.
     *   </li>
     *   <li>
     *     This flow filter is invalidated if the flow condition specified by
     *     this attribute does not exist.
     *   </li>
     * </ul>
     */
    @XmlAttribute(required = true)
    private String  condition;

    /**
     * The type of this flow filter.
     *
     * <p>
     *   The flow filter type determines action to be applied to the packet
     *   when it matches the condition specified by the
     *   <strong>condition</strong> attribute.
     *   One of the following elements can be specified.
     * </p>
     * <dl style="margin-left: 1em;">
     *   <dt>pass
     *   <dd style="margin-left: 1.5em;">
     *     Let the matched packet through the virtual node.
     *     The type of this element must be {@link PassFilter}.
     *
     *   <dt>drop
     *   <dd style="margin-left: 1.5em;">
     *     Discard the matched packet.
     *     The type of this element must be {@link DropFilter}.
     *
     *   <dt>redirect
     *   <dd style="margin-left: 1.5em;">
     *     Forward the matched packet to another virtual interface in the
     *     same VTN.
     *     The type of this element must be {@link RedirectFilter}.
     * </dl>
     *
     * <ul>
     *   <li>
     *     If omitted, it will be treated as if <strong>pass</strong> is
     *     specified.
     *   </li>
     *   <li>
     *     In JSON notation, this element must be wrapped by a
     *     <strong>filterType</strong> element.
     *   </li>
     * </ul>
     */
    @XmlElements({
        @XmlElement(name = "pass", type = PassFilter.class),
        @XmlElement(name = "drop", type = DropFilter.class),
        @XmlElement(name = "redirect", type = RedirectFilter.class)})
    private FilterType  filterType;

    /**
     * A list of {@link FlowAction} instances that modifies the packet
     * when this flow filter is applied to the packet.
     *
     * <ul>
     *   <li>
     *     If omitted or an empty list is specified, this flow filter is
     *     applied to the packet without modifying the packet.
     *   </li>
     *   <li>
     *     Note that the VLAN ID of the packet cannot be specified
     *     because it is always determined by the virtual mapping configuration
     *     such as port mapping.
     *   </li>
     *   <li>
     *     This element is ignored if a <strong>drop</strong> element is
     *     configured in this instance.
     *   </li>
     * </ul>
     * <p>
     *   One or more of the following elements can be specified.
     * </p>
     * <dl style="margin-left: 1em;">
     *   <dt>dlsrc
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the source MAC address in
     *     Ethernet header.
     *     <ul>
     *       <li>The type of this element must be {@link SetDlSrcAction}.</li>
     *       <li>
     *         The source MAC address cannot be changed to the following
     *         addresses.
     *         <ul>
     *           <li>Zero <code>00:00:00:00:00:00</code></li>
     *           <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *           <li>Multicast address</li>
     *         </ul>
     *       </li>
     *     </ul>
     *
     *   <dt>dldst
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the destination MAC address in
     *     Ethernet header.
     *     <ul>
     *       <li>The type of this element must be {@link SetDlDstAction}.</li>
     *       <li>
     *         The destination MAC address cannot be changed to the following
     *         addresses.
     *         <ul>
     *           <li>Zero <code>00:00:00:00:00:00</code></li>
     *           <li>Broadcast address <code>ff:ff:ff:ff:ff:ff</code></li>
     *           <li>Multicast address</li>
     *         </ul>
     *       </li>
     *     </ul>
     *
     *   <dt>vlanpcp
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the VLAN priority in Ethernet
     *     header.
     *     <ul>
     *       <li>The type of this element must be {@link SetVlanPcpAction}.</li>
     *       <li>This element does not affect packets without VLAN tag.</li>
     *     </ul>
     *
     *   <dt>inet4src
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the source IP address in IPv4
     *     header.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetInet4SrcAction}.
     *       </li>
     *       <li>This element does not affect packets without IPv4 header.</li>
     *     </ul>
     *
     *   <dt>inet4dst
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the destination IP address in IPv4
     *     header.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetInet4DstAction}.
     *       </li>
     *       <li>This element does not affect packets without IPv4 header.</li>
     *     </ul>
     *
     *   <dt>dscp
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the DSCP value in IP header.
     *     The type of this element must be {@link SetDscpAction}.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetDscpAction}.
     *       </li>
     *       <li>This element does not affect packets without IP header.</li>
     *     </ul>
     *
     *   <dt>tpsrc
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the source port number in TCP or
     *     UDP header.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetTpSrcAction}.
     *       </li>
     *       <li>
     *         This element does not affect packets without TCP or UDP header.
     *       </li>
     *     </ul>
     *
     *   <dt>tpdst
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the destination port number in TCP or
     *     UDP header.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetTpDstAction}.
     *       </li>
     *       <li>
     *         This element does not affect packets without TCP or UDP header.
     *       </li>
     *     </ul>
     *
     *   <dt>icmptype
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the ICMP type in ICMPv4 header.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetIcmpTypeAction}.
     *       </li>
     *       <li>
     *         This element does not affect packets without ICMPv4 header.
     *       </li>
     *     </ul>
     *
     *   <dt>icmpcode
     *   <dd style="margin-left: 1.5em;">
     *     Specifies the action that sets the ICMP code in ICMPv4 header.
     *     <ul>
     *       <li>
     *         The type of this element must be {@link SetIcmpCodeAction}.
     *       </li>
     *       <li>
     *         This element does not affect packets without ICMPv4 header.
     *       </li>
     *     </ul>
     * </dl>
     */
    @XmlElementWrapper
    @XmlElements({
        @XmlElement(name = "dlsrc", type = SetDlSrcAction.class),
        @XmlElement(name = "dldst", type = SetDlDstAction.class),
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
     * Private constructor only for JAXB.
     */
    private FlowFilter() {
    }

    /**
     * Construct a new flow filter configuration.
     *
     * @param idx   An {@link Integer} instance which represents the filter
     *              index.
     * @param cond  The name of the flow condition.
     * @param type  The type of the flow filter.
     *              {@code null} cannot be specified.
     * @param acts  A list of {@link FlowAction} instances to be applied to
     *              packets.
     */
    private FlowFilter(Integer idx, String cond, FilterType type,
                       List<FlowAction> acts) {
        index = idx;
        condition = cond;
        filterType = type;
        if (acts != null && !acts.isEmpty()) {
            actions = new ArrayList<FlowAction>(acts);
        }
    }

    /**
     * Construct a new flow filter configuration without specifying
     * filter index.
     *
     * @param cond  The name of the flow condition.
     *              {@code null} cannot be specified.
     * @param type  The type of the flow filter.
     *              {@code null} cannot be specified.
     * @param acts  A list of {@link FlowAction} instances to be applied to
     *              packets.
     */
    public FlowFilter(String cond, FilterType type, List<FlowAction> acts) {
        this(null, cond, type, acts);
    }

    /**
     * Construct a new flow filter configuration.
     *
     * @param idx   The index value to be assigned to the flow filter.
     *              The range of value that can be specified is from
     *              <strong>1</strong> to <strong>65535</strong>.
     * @param cond  The name of the flow condition.
     *              {@code null} cannot be specified.
     * @param type  The type of the flow filter.
     *              {@code null} cannot be specified.
     * @param acts  A list of {@link FlowAction} instances to be applied to
     *              packets.
     */
    public FlowFilter(int idx, String cond, FilterType type,
                      List<FlowAction> acts) {
        this(Integer.valueOf(idx), cond, type, acts);
    }

    /**
     * Return an index value assigned to this instance.
     *
     * @return  An {@link Integer} instance which represents an index value
     *          assigned to this instance.
     *          {@code null} is returned if no index is assigned.
     */
    public Integer getIndex() {
        return index;
    }

    /**
     * Return the name of the flow condition configured in this instance.
     *
     * @return  The name of the flow condition.
     */
    public String getFlowConditionName() {
        return condition;
    }

    /**
     * Return a {@link FilterType} instance which represents the type of
     * this flow filter.
     *
     * @return  A {@link FilterType} instance.
     *          {@code null} is returned if no filter type is configured.
     */
    public FilterType getFilterType() {
        return filterType;
    }

    /**
     * Return a list of {@link FlowAction} instances to be applied to the
     * packet when this flow filter is applied.
     *
     * @return  A list of {@link FlowAction} instances.
     *          {@code null} is returned if no flow action is configured.
     */
    public List<FlowAction> getActions() {
        return (actions == null)
            ? null
            : new ArrayList<FlowAction>(actions);
    }

    // Objects

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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        FlowFilter ff = (FlowFilter)o;
        return (Objects.equals(index, ff.index) &&
                Objects.equals(condition, ff.condition) &&
                Objects.equals(filterType, ff.filterType) &&
                Objects.equals(actions, ff.actions));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(index, condition, filterType, actions);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("FlowFilter[");
        String sep = "";
        if (index != null) {
            builder.append("index=").append(index.toString());
            sep = ",";
        }
        if (condition != null) {
            builder.append(sep).append("cond=").append(condition);
            sep = ",";
        }
        if (filterType != null) {
            builder.append(sep).append("type=").append(filterType);
            sep = ",";
        }
        if (actions != null) {
            builder.append(sep).append("actions=").append(actions);
        }

        return builder.append(']').toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This method returns the index of the flow filter.
     * </p>
     *
     * @return  The index of the flow filter.
     * @since   Lithium
     */
    @Override
    public Integer getIdentifier() {
        return index;
    }
}
