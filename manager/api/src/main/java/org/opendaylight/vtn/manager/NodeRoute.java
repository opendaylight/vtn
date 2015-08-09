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
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code NodeRoute} class describes a pair of input and output switch port
 * within the same physical switch.
 *
 * <p>
 *   An instance of {@code NodeRoute} represents two physical ports
 *   in the same physical switch. The route of the packet in the physical
 *   network from the source to the destination physical switch port is
 *   represented by a sequence of {@code NodeRoute} instances.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:01"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"input": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"name": "s1-eth1"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"output": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "4",
 * &nbsp;&nbsp;&nbsp;&nbsp;"name": "s1-eth4"
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "noderoute")
@XmlAccessorType(XmlAccessType.NONE)
public final class NodeRoute implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -80502914050326845L;

    /**
     * Node information corresponding to the physical switch.
     *
     * <ul>
     *   <li>This element is mandatory.</li>
     * </ul>
     */
    @XmlElement(required = true)
    private Node  node;

   /**
     * Condition for identifying the switch port where the packet is received.
     *
     * <ul>
     *   <li>This element is mandatory.</li>
     * </ul>
     */
    @XmlElement(required = true)
    private SwitchPort  input;

   /**
     * Condition for identifying the switch port to which the packet is sent.
     *
     * <ul>
     *   <li>This element is mandatory.</li>
     * </ul>
     */
    @XmlElement(required = true)
    private SwitchPort  output;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private NodeRoute() {
    }

    /**
     * Construct a new instance.
     *
     * @param node    A {@link Node} instance corresponding to the physical
     *                switch.
     * @param input   A {@link SwitchPort} instance corresponding to the
     *                switch port where the packet is received.
     * @param output  A {@link SwitchPort} instance corresponding to the
     *                switch port to which the packet is sent.
     */
    public NodeRoute(Node node, SwitchPort input, SwitchPort output) {
        this.node = node;
        this.input = input;
        this.output = output;
    }

    /**
     * Construct a new instance.
     *
     * @param inport   A {@link NodeConnector} instance corresponding to the
     *                 physical switch port from which the packet is received.
     *                 {@code null} cannot be specified.
     * @param inname   The name of the physical switch port specified by
     *                 {@code inport}. {@code null} can be specified if no
     *                 name is assigned.
     * @param outport  A {@link NodeConnector} instance corresponding to the
     *                 physical switch port to which the packet is forwarded.
     *                 {@code null} cannot be specified.
     * @param outname  The name of the physical switch port specified by
     *                 {@code outport}. {@code null} can be specified if no
     *                 name is assigned.
     * @throws NullPointerException
     *    {@code null} is passed to either {@code inport} or {@code outport}.
     * @throws IllegalArgumentException
     *    Physical switch ports in different switches are specified to
     *    {@code inport} and {@code outport}.
     */
    public NodeRoute(NodeConnector inport, String inname,
                     NodeConnector outport, String outname) {
        Node nd = inport.getNode();
        if (!nd.equals(outport.getNode())) {
            throw new IllegalArgumentException(
                "Input and output port must belong to the same switch: " +
                inport + ", " + outport);
        }

        node = nd;
        input = new SwitchPort(inport, inname);
        output = new SwitchPort(outport, outname);
    }

    /**
     * Return a {@link Node} instance corresponding to the physical switch.
     *
     * <p>
     *   Note that this method returns {@code null} if this instance is
     *   deserialized from XML data which does not specify the physical switch.
     * </p>
     *
     * @return  A {@link Node} instance.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Return a {@link SwitchPort} instance which represents the location of
     * the input switch port within the physical switch.
     *
     * <p>
     *   Note that {@code null} is returned if this instance is deserialized
     *   from XML data without input port information.
     * </p>
     *
     * @return  A {@link SwitchPort} instance which specifies the location
     *          of the input port.
     */
    public SwitchPort getInputPort() {
        return input;
    }

    /**
     * Return a {@link SwitchPort} instance which represents the location of
     * the output switch port within the physical switch.
     *
     * <p>
     *   Note that {@code null} is returned if this instance is deserialized
     *   from XML data without output port information.
     * </p>
     *
     * @return  A {@link SwitchPort} instance which specifies the location
     *          of the output port.
     */
    public SwitchPort getOutputPort() {
        return output;
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
        if (!(o instanceof NodeRoute)) {
            return false;
        }

        NodeRoute pr = (NodeRoute)o;
        return (Objects.equals(node, pr.node) &&
                Objects.equals(input, pr.input) &&
                Objects.equals(output, pr.output));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(node, input, output);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("NodeRoute[");
        String sep = "";
        if (node != null) {
            builder.append("node=").append(node.toString());
            sep = ",";
        }
        if (input != null) {
            builder.append(sep).append("input=").append(input.toString());
            sep = ",";
        }
        if (output != null) {
            builder.append(sep).append("output=").append(output.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
