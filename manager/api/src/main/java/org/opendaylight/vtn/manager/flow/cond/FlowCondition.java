/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

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
 * This class describes the flow condition, which specifies a list of
 * flow conditions to select packets.
 *
 * <p>
 *   A flow condition can contain a list of flow match conditions represented
 *   by a list of {@link FlowMatch} instances. A list of flow match conditions
 *   is used to select packets. Each flow match condition must have a match
 *   index, which is an unique index in a flow condition. When a flow condition
 *   tests a packet, flow match conditions in a flow condition are evaluated
 *   in ascending order of match index. A packet is selected if at least one
 *   flow match condition matches the packet.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"name": "cond_1",
 * &nbsp;&nbsp;"match": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"index": 1,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"ethernet": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"src": "00:11:22:33:44:55",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"dst": "00:aa:bb:cc:dd:ee",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": 2048
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"inetMatch": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"inet4": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"src": "192.168.10.1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"dst": "192.168.20.2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"protocol": 1
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"l4Match": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"icmp": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": 3,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"code": 0
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"index": 2,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"ethernet": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"src": "00:12:34:56:78:9a",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"vlan": 10
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowcondition")
@XmlAccessorType(XmlAccessType.NONE)
public final class FlowCondition
    implements Serializable, VTNIdentifiable<String> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6250511924260021815L;

    /**
     * The name of the flow condition.
     */
    @XmlAttribute
    private String  name;

    /**
     * A list of {@link FlowMatch} instances which specifies flow match
     * conditions used to select packets.
     * <ul>
     *   <li>
     *     Unique index value must be assigned to each <strong>match</strong>
     *     element in this element.
     *   </li>
     *   <li>
     *     If more than one <strong>match</strong> elements are configured,
     *     they are evaluated against packets in ascending order of
     *     match index configured in each <strong>match</strong> element.
     *     Packets which matches the condition described by at least one
     *     <strong>match</strong> element are selected by this flow condition.
     *   </li>
     *   <li>
     *     Every packet is selected if this element is omitted or empty.
     *   </li>
     *   <li>
     *     In JSON notation, this element is represented as an array property
     *     named <strong>match</strong>.
     *     <strong>matches</strong> property will never appear in JSON object.
     *   </li>
     * </ul>
     */
    @XmlElementWrapper
    @XmlElement(name = "match")
    private List<FlowMatch>  matches;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private FlowCondition() {
    }

    /**
     * Construct a new instance.
     *
     * @param name     The name of the flow condition.
     * @param matches  A list of {@link FlowMatch} instances which describes
     *                 the condition to select packets.
     *                 Every packets are selected if {@code null} or an empty
     *                 list is specified.
     */
    public FlowCondition(String name, List<FlowMatch> matches) {
        this.name = name;
        if (matches != null && !matches.isEmpty()) {
            this.matches = new ArrayList<FlowMatch>(matches);
        }
    }

    /**
     * Return the name of the flow condition.
     *
     * @return  The name of the flow condition configured in this instance.
     */
    public String getName() {
        return name;
    }

    /**
     * Return a list of {@link FlowMatch} instances configured in this
     * instance.
     *
     * @return  A list of {@link FlowMatch} instances.
     */
    public List<FlowMatch> getMatches() {
        return (matches == null)
            ? new ArrayList<FlowMatch>(0)
            : new ArrayList<FlowMatch>(matches);
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
        if (!(o instanceof FlowCondition)) {
            return false;
        }

        FlowCondition fc = (FlowCondition)o;
        return (Objects.equals(name, fc.name) &&
                Objects.equals(matches, fc.matches));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(name, matches);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("FlowCondition[");
        String sep = "";
        if (name != null) {
            builder.append("name=").append(name);
            sep = ",";
        }
        if (matches != null) {
            builder.append(sep).append("matches=").append(matches.toString());
        }
        builder.append(']');

        return builder.toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This class uses the anme of the flow condition as the identifier.
     * </p>
     *
     * @return  The name of the flow condition.
     * @since   Lithium
     */
    @Override
    public String getIdentifier() {
        return name;
    }
}
