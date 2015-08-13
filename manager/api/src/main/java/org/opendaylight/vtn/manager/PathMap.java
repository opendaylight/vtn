/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

/**
 * {@code PathMap} class describes the configuration of path map.
 *
 * <p>
 *   Path map defines the packet routing policy in the physical network.
 *   The configuration for a path map specifies the name of the
 *   {@link org.opendaylight.vtn.manager.flow.cond.FlowCondition flow condition}
 *   and the identifier of the
 *   {@link PathPolicy path policy}. If a packet matches the flow condition
 *   configured in a path map, the route of that packet will be determined
 *   by the path policy configured in the same path map.
 * </p>
 * <p>
 *   Multiple path maps can be configured as a list. Each path map in the list
 *   must have a unique index number which determines the evaluation order
 *   of path maps. When an incoming packet is mapped to a VTN, path maps in
 *   the path map list are evaluated against the packet in ascending order of
 *   index numbers. If a flow condition configured in a path map matches the
 *   packet, the path policy configured in the same path map is used to
 *   determine packet routing.
 * </p>
 * <p>
 *   There are two types of path map list.
 * </p>
 * <dl style="margin-left: 1em;">
 *   <dt>VTN path map list
 *   <dd style="margin-left: 1.5em;">
 *     Each VTN owns one path map list. Path maps in this list affect packets
 *     mapped to that VTN.
 *
 *   <dt>Container path map list
 *   <dd style="margin-left: 1.5em;">
 *     Each container owns one path map list which affects all VTNs
 *     in that container. Path maps in this list are evaluated when a packet
 *     does not match any path map in the VTN path map list.
 * </dl>
 * <p>
 *   Note that path maps are evaluated just after an incoming packet is
 *   received by the VTN Manager. Some features in the VTN Manager may modify
 *   packet, but those modifications never affect the path map evaluation.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"index": 10,
 * &nbsp;&nbsp;"condition": "flowcond_1",
 * &nbsp;&nbsp;"policy": 1,
 * &nbsp;&nbsp;"idleTimeout": 300,
 * &nbsp;&nbsp;"hardTimeout": 0
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "pathmap")
@XmlAccessorType(XmlAccessType.NONE)
public final class PathMap
    implements Serializable, Cloneable, VTNIdentifiable<Integer> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2728886905552705809L;

    /**
     * An index value assigned to this path map.
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     This value is used to determine order of evaluation.
     *     Path maps in the path map list are evaluated in ascending order
     *     of indices assigned to <strong>pathmap</strong> elements.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "index")
    private Integer  mapIndex;

    /**
     * The name of the flow condition that selects packets.
     * <ul>
     *   <li>
     *     This path map is invalidated if the flow condition specified by
     *     this attribute does not exist.
     *   </li>
     * </ul>
     */
    @XmlAttribute(required = true)
    private String  condition;

    /**
     * The identifier of the path policy which determines the route of packets.
     * <ul>
     *   <li>
     *     This path map is invalidated if the path policy specified by this
     *     attribute does not exist.
     *   </li>
     *   <li>
     *     <strong>0</strong> specifies the system default path policy which
     *     minimizes the number of hops in the packet route.
     *   </li>
     *   <li>
     *     If omitted, it will be treated as if <strong>0</strong> is
     *     specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute(name = "policy")
    private int  policyId;

    /**
     * The number of seconds to be configured in {@code idle_timeout} field
     * in flow entries.
     * <p>
     *   If the route of the packet is determined by this path map,
     *   the VTN Manager configures the specified value into
     *   {@code idle_timeout} field in flow entries which establish the packet
     *   route. So they will expire if they are not referred for the specified
     *   seconds.
     * </p>
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     <strong>0</strong> means infinite time.
     *   </li>
     *   <li>
     *     If omitted, the {@code idle_timeout} value is determined by the
     *     VTN configuration.
     *   </li>
     *   <li>
     *     If this attribute is configured, the <strong>hardTimeout</strong>
     *     attribute also needs to be configured.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Integer  idleTimeout;

    /**
     * The number of seconds to be configured in {@code hard_timeout} field
     * in flow entries.
     * <p>
     *   If the route of the packet is determined by this path map,
     *   the VTN Manager configures the specified value into
     *   {@code hard_timeout} field in flow entries which establish the packet
     *   route. So they will expire in the specified seconds.
     * </p>
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     <strong>0</strong> means infinite time.
     *   </li>
     *   <li>
     *     If omitted, the {@code hard_timeout} value is determined by the
     *     VTN configuration.
     *   </li>
     *   <li>
     *     If this attribute is configured, the <strong>idleTimeout</strong>
     *     attribute also needs to be configured.
     *   </li>
     *   <li>
     *     If both values configured in <strong>idleTimeout</strong> and
     *     <strong>hardTimeout</strong> are not zero, the value configured in
     *     <strong>hardTimeout</strong> must be greater than the value
     *     configured in <strong>idleTimeout</strong>.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Integer  hardTimeout;

    /**
     * Private constructor only for JAXB.
     */
    private PathMap() {
    }

    /**
     * Construct a new instance without specifying index and flow timeout.
     *
     * <p>
     *   If this constructor is used, timeout of flow entries which establish
     *   packet route is determined by the VTN configuration.
     * </p>
     *
     * @param cond     The name of the flow condition.
     * @param policy   The identifier of the path policy.
     */
    public PathMap(String cond, int policy) {
        condition = cond;
        policyId = policy;
    }

    /**
     * Construct a new instance without specifying flow timeout.
     *
     * <p>
     *   If this constructor is used, timeout of flow entries which establish
     *   packet route is determined by the VTN configuration.
     * </p>
     *
     * @param index   An index value assigned to a new instance.
     * @param cond    The name of the flow condition.
     * @param policy  The identifier of the path policy.
     */
    public PathMap(int index, String cond, int policy) {
        mapIndex = Integer.valueOf(index);
        condition = cond;
        policyId = policy;
    }

    /**
     * Construct a new instance without specifying index.
     *
     * @param cond    The name of the flow condition.
     * @param policy  The identifier of the path policy.
     * @param idle
     *   The number of seconds to be configured in {@code idle_timeout} field
     *   in flow entries.
     *   <p>
     *     If the route of the packet is determined by this path map,
     *     the VTN Manager configures the specified value into
     *     {@code idle_timeout} field in flow entries which establish the
     *     packet route. So they will expire if they are not referred for the
     *     specified seconds.
     *   </p>
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *   </ul>
     * @param hard
     *   The number of seconds to be configured in {@code hard_timeout} field
     *   in flow entries.
     *   <p>
     *     If the route of the packet is determined by this path map,
     *     the VTN Manager configures the specified value into
     *     {@code hard_timeout} field in flow entries which establish the
     *     packet route. So they will expire in the specified seconds.
     *   </p>
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *     <li>
     *       If both values passed to {@code idle} and {@code hard} are not
     *       zero, the value passed to {@code hard} must be greater than
     *       the value passed to {@code idle}.
     *     </li>
     *   </ul>
     */
    public PathMap(String cond, int policy, int idle, int hard) {
        condition = cond;
        policyId = policy;
        idleTimeout = Integer.valueOf(idle);
        hardTimeout = Integer.valueOf(hard);
    }

    /**
     * Construct a new instance.
     *
     * @param index   An index value assigned to a new instance.
     * @param cond    The name of the flow condition.
     * @param policy  The identifier of the path policy.
     * @param idle
     *   The number of seconds to be configured in {@code idle_timeout} field
     *   in flow entries.
     *   <p>
     *     If the route of the packet is determined by this path map,
     *     the VTN Manager configures the specified value into
     *     {@code idle_timeout} field in flow entries which establish the
     *     packet route. So they will expire if they are not referred for the
     *     specified seconds.
     *   </p>
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *   </ul>
     * @param hard
     *   The number of seconds to be configured in {@code hard_timeout} field
     *   in flow entries.
     *   <p>
     *     If the route of the packet is determined by this path map,
     *     the VTN Manager configures the specified value into
     *     {@code hard_timeout} field in flow entries which establish the
     *     packet route. So they will expire in the specified seconds.
     *   </p>
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *     <li>
     *       If both values passed to {@code idle} and {@code hard} are not
     *       zero, the value passed to {@code hard} must be greater than
     *       the value passed to {@code idle}.
     *     </li>
     *   </ul>
     */
    public PathMap(int index, String cond, int policy, int idle, int hard) {
        mapIndex = Integer.valueOf(index);
        condition = cond;
        policyId = policy;
        idleTimeout = Integer.valueOf(idle);
        hardTimeout = Integer.valueOf(hard);
    }

    /**
     * Return an index value assigned to this instance.
     *
     * @return  An {@link Integer} instance which represents an index value
     *          assigned to this instance.
     *          {@code null} is returned if no index is assigned.
     */
    public Integer getIndex() {
        return mapIndex;
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
     * Return the identifier of the path policy configured in this instance.
     *
     * @return  The identifier of the path policy.
     */
    public int getPathPolicyId() {
        return policyId;
    }

    /**
     * Return the number of seconds to be configured in {@code idle_timeout}
     * field in flow entries.
     *
     * @return  An {@link Integer} instance which represents the
     *          {@code idle_timeout} field value.
     *          {@code null} is returned if not configured.
     */
    public Integer getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Return the number of seconds to be configured in {@code hard_timeout}
     * field in flow entries.
     *
     * @return  An {@link Integer} instance which represents the
     *          {@code hard_timeout} field value.
     *          {@code null} is returned if not configured.
     */
    public Integer getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Determine whether the given object contains the same flow timeout.
     *
     * @param ptmap  The object to be compared.
     * @return  {@code true} only if the given object contains the same flow
     *          timeout.
     */
    private boolean equalsFlowTimeout(PathMap ptmap) {
        return (Objects.equals(idleTimeout, ptmap.idleTimeout) &&
                Objects.equals(hardTimeout, ptmap.hardTimeout));
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
        if (!(o instanceof PathMap)) {
            return false;
        }

        PathMap ptmap = (PathMap)o;
        return (Objects.equals(mapIndex, ptmap.mapIndex) &&
                policyId == ptmap.policyId &&
                Objects.equals(condition, ptmap.condition) &&
                equalsFlowTimeout(ptmap));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(mapIndex, condition, idleTimeout, hardTimeout) +
            (policyId * 31);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PathMap[");
        String sep = "";
        if (mapIndex != null) {
            builder.append("index=").append(mapIndex.toString());
            sep = ",";
        }
        if (condition != null) {
            builder.append(sep).append("cond=").append(condition);
            sep = ",";
        }
        builder.append(sep).append("policy=").append(policyId);

        if (idleTimeout != null) {
            builder.append(",idle=").append(idleTimeout.toString());
        }
        if (hardTimeout != null) {
            builder.append(",hard=").append(hardTimeout.toString());
        }

        return builder.append(']').toString();
    }

    // Cloneable

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A copy of this instance.
     */
    @Override
    public PathMap clone() {
        try {
            return (PathMap)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This method returns the identifier of the path map.
     * </p>
     *
     * @return  The identifier of the path map.
     * @since   Lithium
     */
    @Override
    public Integer getIdentifier() {
        return mapIndex;
    }
}
