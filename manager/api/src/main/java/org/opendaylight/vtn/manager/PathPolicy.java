/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElementWrapper;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

/**
 * {@link PathPolicy} class describes the configuration of path policy.
 *
 * <p>
 *   The default policy of packet routing in the OpenDaylight controller is
 *   "shortest-path". When the VTN Manager configures a unicast flow with
 *   default path policy, it chooses the packet route which minimizes the
 *   number of hops. The cost of each link between switches is not considered.
 * </p>
 * <p>
 *   "Path Policy" feature implements cost-based packet routing.
 *   A path policy is a set of user-defined cost of using the link for
 *   transmission. It can specify the cost of using specific switch link.
 *   If a path policy is applied for packet routing, the VTN Manager chooses
 *   the packet route which minimizes the total cost of switch links.
 *   {@link org.opendaylight.vtn.manager.flow.cond.FlowCondition Flow condition}
 *   is used to determine path policy to be applied for packet routing.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"id": 1,
 * &nbsp;&nbsp;"default": 100000,
 * &nbsp;&nbsp;"cost": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"location": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:01"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s1-eth1"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"cost": 1000
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"location": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:02"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s2-eth3"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"cost": 300000
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "pathpolicy")
@XmlAccessorType(XmlAccessType.NONE)
public final class PathPolicy
    implements Serializable, VTNIdentifiable<Integer> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5554438805658814600L;

    /**
     * The pseudo cost value which represents the cost is not defined.
     */
    public static final long  COST_UNDEF = 0L;

    /**
     * The identifier of this path policy.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>3</strong>.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "id")
    private Integer  policyId;

    /**
     * The default cost for unspecified link.
     * <p>
     *   If the cost of outgoing switch port is not defined in
     *   <strong>costs</strong> element, the value configured in this
     *   attribute is used as the cost.
     * </p>
     * <ul>
     *   <li>
     *     The value must be zero or greater than zero.
     *   </li>
     *   <li>
     *     Specifying zero means that the default link cost should be
     *     determined individually by the link speed configured to a switch
     *     port. If zero is specified, the default link cost of a switch port
     *     is calculated by the following formula.
     *     <blockquote>
     *       MAX(10,000,000,000,000 / (<i>link speed (bps)</i>), 1)
     *     </blockquote>
     *   </li>
     *   <li>
     *     If omitted, it will be treated as if zero is specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "default")
    private long  defaultCost;

    /**
     * A list of {@link PathCost} instances which specifies the cost of
     * switch link for transmitting.
     *
     * <ul>
     *   <li>
     *     Each <strong>cost</strong> element can not omit
     *     <strong>location</strong> element which specifies the location of
     *     the switch port, and it must be unique in this element.
     *   </li>
     *   <li>
     *     The cost of using the specific switch port is determined by
     *     searching for a <strong>cost</strong> element which matches the
     *     switch port location specified by <strong>location</strong> element.
     *     If multiple <strong>cost</strong> elements match the target switch
     *     port, the VTN Manager gives higher priority to one which specifies
     *     more exact port location.
     *     The lookup of <strong>cost</strong> element list is implemented
     *     as follows.
     *     <ol>
     *       <li>
     *         Search for a <strong>cost</strong> element that meets all the
     *         following conditions. The link cost configured in that
     *         <strong>cost</strong> element is used if found.
     *         <ul>
     *           <li>
     *             The physical switch specified by <strong>node</strong>
     *             element in the <strong>location</strong> element contains
     *             the target switch port.
     *           </li>
     *           <li>
     *             Both the node connector type and ID specified by
     *             <strong>port</strong> element in the
     *             <strong>location</strong> element match the target switch
     *             port.
     *           </li>
     *           <li>
     *             The port name specified by <strong>port</strong> element in
     *             the <strong>location</strong> element matches the target
     *             switch port.
     *           </li>
     *         </ul>
     *       </li>
     *       <li>
     *         Search for a <strong>cost</strong> element that meets all the
     *         following conditions. The link cost configured in that
     *         <strong>cost</strong> element is used if found.
     *         <ul>
     *           <li>
     *             The physical switch specified by <strong>node</strong>
     *             element in the <strong>location</strong> element contains
     *             the target switch port.
     *           </li>
     *           <li>
     *             The node connector type and ID specified by
     *             <strong>port</strong> element in the
     *             <strong>location</strong> element matches the target switch
     *             port.
     *           </li>
     *           <li>
     *             The port name is not configured in <strong>port</strong>
     *             element in the <strong>location</strong> element.
     *           </li>
     *         </ul>
     *       </li>
     *       <li>
     *         Search for a <strong>cost</strong> element that meets all the
     *         following conditions. The link cost configured in that
     *         <strong>cost</strong> element is used if found.
     *         <ul>
     *           <li>
     *             The physical switch specified by <strong>node</strong>
     *             element in the <strong>location</strong> element contains
     *             the target switch port.
     *           </li>
     *           <li>
     *             The port name specified by <strong>port</strong> element in
     *             the <strong>location</strong> element matches the target
     *             switch port.
     *           </li>
     *           <li>
     *             Both the node connector type and ID are not configured in
     *             <strong>port</strong> element in the
     *             <strong>location</strong> element.
     *           </li>
     *         </ul>
     *       </li>
     *       <li>
     *         Search for a <strong>cost</strong> element that meets all the
     *         following conditions. The link cost configured in that
     *         <strong>cost</strong> element is used if found.
     *         <ul>
     *           <li>
     *             The physical switch specified by <strong>node</strong>
     *             element in the <strong>location</strong> element contains
     *             the target switch port.
     *           </li>
     *           <li>
     *             The <strong>location</strong> element does not contain
     *             <strong>port</strong> element.
     *           </li>
     *         </ul>
     *       </li>
     *       <li>
     *         The default cost specified in <strong>default</strong> attribute
     *         is used.
     *       </li>
     *     </ol>
     *   </li>
     *   <li>
     *     In JSON notation, this element is represented as an array property
     *     named <strong>cost</strong>.
     *     <strong>costs</strong> property will never appear in JSON object.
     *   </li>
     * </ul>
     */
    @XmlElementWrapper(name = "costs")
    @XmlElement(name = "cost")
    private List<PathCost>  pathCosts;

    /**
     * Private constructor only for JAXB.
     */
    private PathPolicy() {
    }

    /**
     * Construct a new instance without specifying policy identifier.
     *
     * @param id       The identifier of the path policy.
     *                 {@code null} means that the identifier is not specified.
     * @param defcost  The default cost for unspecified link.
     * @param costs    A list of {@link PathCost} instances whcih describes
     *                 the cost of using the switch link.
     * @see #PathPolicy(int, long, List)
     */
    public PathPolicy(Integer id, long defcost, List<PathCost> costs) {
        policyId = id;
        defaultCost = defcost;
        if (costs != null && !costs.isEmpty()) {
            pathCosts = new ArrayList<PathCost>(costs);
        }
    }

    /**
     * Construct a new instance without specifying policy identifier.
     *
     * @param defcost  The default cost for unspecified link.
     * @param costs    A list of {@link PathCost} instances whcih describes
     *                 the cost of using the switch link.
     * @see #PathPolicy(int, long, List)
     */
    public PathPolicy(long defcost, List<PathCost> costs) {
        this(null, defcost, costs);
    }

    /**
     * Construct a new instance.
     *
     * @param id
     *   The identifier of the path policy.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>1</strong> to <strong>3</strong>.
     *     </li>
     *   </ul>
     * @param defcost
     *   The default cost for unspecified link.
     *   <p>
     *     If the cost of outgoing switch port is not configured in
     *     {@code costs} parameter, the value passed to this parameter is used
     *     as the cost.
     *   </p>
     *   <ul>
     *     <li>
     *       The value must be {@link #COST_UNDEF} or a positive value.
     *     </li>
     *     <li>
     *       Specifying {@link #COST_UNDEF} means that the default link cost
     *       should be determined individually by the link speed configured to
     *       a switch port. If {@link #COST_UNDEF} is specified, the default
     *       link cost of a switch port is calculated by the following formula.
     *       <blockquote>
     *         MAX(10,000,000,000,000 / (<i>link speed (bps)</i>), 1)
     *       </blockquote>
     *     </li>
     *   </ul>
     * @param costs
     *   A list of {@link PathCost} instances which describes the cost of using
     *   the switch link.
     *   <ul>
     *     <li>
     *       Each {@link PathCost} element in the list must contain a
     *       {@link PortLocation} instance, and it must be unique in the list.
     *     </li>
     *     <li>
     *       The cost of using the specific switch port is determined by
     *       searching for a {@link PathCost} instance which matches the
     *       switch port location specified by {@link PortLocation} instance.
     *       If multiple {@link PathCost} instances match the target switch
     *       port, the VTN Manager gives higher priority to one which specifies
     *       more exact port location.
     *       The lookup of {@link PathCost} list is implemented as follows.
     *       <ol>
     *         <li>
     *           Search for a {@link PathCost} instance that meets all the
     *           following conditions. The link cost configured in that
     *           {@link PathCost} instance is used if found.
     *           <ul>
     *             <li>
     *               The physical switch specified by a
     *               {@link PortLocation#getNode() Node} instance contains the
     *               target switch port.
     *             </li>
     *             <li>
     *               Both the node connector {@link SwitchPort#getType() type}
     *               and {@link SwitchPort#getId() ID} configured in a
     *               {@link SwitchPort} instance match the target switch port.
     *             </li>
     *             <li>
     *               The {@link SwitchPort#getName() port name} configured in
     *               a {@link SwitchPort} instance matches the target switch
     *               port.
     *             </li>
     *           </ul>
     *         </li>
     *         <li>
     *           Search for a {@link PathCost} instance that meets all the
     *           following conditions. The link cost configured in that
     *           {@link PathCost} instance is used if found.
     *           <ul>
     *             <li>
     *               The physical switch specified by a
     *               {@link PortLocation#getNode() Node} instance contains the
     *               target switch port.
     *             </li>
     *             <li>
     *               Both the node connector {@link SwitchPort#getType() type}
     *               and {@link SwitchPort#getId() ID} configured in a
     *               {@link SwitchPort} instance match the target switch port.
     *             </li>
     *             <li>
     *               The {@link SwitchPort#getName() port name} is not
     *               specified in a {@link SwitchPort} instance.
     *             </li>
     *           </ul>
     *         </li>
     *         <li>
     *           Search for a {@link PathCost} instance that meets all the
     *           following conditions. The link cost configured in that
     *           {@link PathCost} instance is used if found.
     *           <ul>
     *             <li>
     *               The physical switch specified by a
     *               {@link PortLocation#getNode() Node} instance contains the
     *               target switch port.
     *             </li>
     *             <li>
     *               The {@link SwitchPort#getName() port name} configured in
     *               a {@link SwitchPort} instance matches the target switch
     *               port.
     *             </li>
     *             <li>
     *               Both the node connector {@link SwitchPort#getType() type}
     *               and {@link SwitchPort#getId() ID} are not specified in a
     *               {@link SwitchPort} instance.
     *             </li>
     *           </ul>
     *         </li>
     *         <li>
     *           Search for a {@link PathCost} instance that meets all the
     *           following conditions. The link cost configured in that
     *           {@link PathCost} instance is used if found.
     *           <ul>
     *             <li>
     *               The physical switch specified by a
     *               {@link PortLocation#getNode() Node} instance contains the
     *               target switch port.
     *             </li>
     *             <li>
     *               A {@link PortLocation} does not contain a
     *               {@link SwitchPort} instance.
     *             </li>
     *           </ul>
     *         </li>
     *         <li>
     *           The default cost specified by {@code defcost} parameter
     *           is used.
     *         </li>
     *       </ol>
     *     </li>
     *   </ul>
     */
    public PathPolicy(int id, long defcost, List<PathCost> costs) {
        this(Integer.valueOf(id), defcost, costs);
    }

    /**
     * Return the identifier of the path policy.
     *
     * @return  An {@link Integer} instance which represents the identifier of
     *          the path policy. {@code null} is returned if not configured.
     */
    public Integer getPolicyId() {
        return policyId;
    }

    /**
     * Return the default cost for unspecified link.
     *
     * @return  The default cost for unspecified link.
     *          {@link #COST_UNDEF} means that the default cost should be
     *          determined by link speed.
     */
    public long getDefaultCost() {
        return defaultCost;
    }

    /**
     * Return a list of {@link PathCost} instances configured in this instance.
     *
     * @return  A list of {@link PathCost} instances.
     */
    public List<PathCost> getPathCosts() {
        return (pathCosts == null)
            ? new ArrayList<PathCost>(0)
            : new ArrayList<PathCost>(pathCosts);
    }

    // Object

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
        if (!(o instanceof PathPolicy)) {
            return false;
        }

        PathPolicy pp = (PathPolicy)o;
        return (defaultCost == pp.defaultCost &&
                Objects.equals(policyId, pp.policyId) &&
                Objects.equals(pathCosts, pp.pathCosts));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(policyId, pathCosts) +
            (int)(defaultCost ^ (defaultCost >>> Integer.SIZE)) * 31;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PathPolicy[");
        String sep = "";
        if (policyId != null) {
            builder.append("id=").append(policyId.toString());
            sep = ",";
        }

        builder.append(sep).append("default=").append(defaultCost);

        if (pathCosts != null) {
            builder.append(",costs=").append(pathCosts.toString());
        }
        builder.append(']');

        return builder.toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This method returns the identifier of the path policy.
     * </p>
     *
     * @return  The identifier of the path policy.
     * @since   Lithium
     */
    @Override
    public Integer getIdentifier() {
        return policyId;
    }
}
