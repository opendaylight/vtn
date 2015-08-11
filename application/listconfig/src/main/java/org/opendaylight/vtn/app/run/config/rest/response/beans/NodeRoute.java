/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;
/**
 * NodeRoute - Bean Representaion for NodeRoute object from the JSON Response.
 *
 */
@JsonObject
public class NodeRoute {

    @JsonObjectRef(name = "Node")
    private NodeDetails node =  new NodeDetails();

    @JsonObjectRef(name = "input")
    private InputOutput input = new InputOutput();

    @JsonObjectRef(name = "outputt")
    private InputOutput output = new InputOutput();

    public NodeRoute() {}

    /**
     * getNode - function to get Nodedetails for this object.
     *
     * @return node details
     */
    public NodeDetails getNode() {
        return node;
    }

    /**
     * setNode - function to set Nodedetails for this object.
     *
     * @param node
     */
    public void setNode(NodeDetails node) {
        this.node = node;
    }

    /**
     * getInput - function to get Input value for this object.
     *
     * @return input
     */
    public InputOutput getInput() {
        return input;
    }

    /**
     * setInput - function to set Input value for this object.
     *
     * @param input
     */
    public void setInput(InputOutput input) {
        this.input = input;
    }

    /**
     * getOutput - function to get Output value for this object.
     *
     * @return output
     */
    public InputOutput getOutput() {
        return output;
    }

    /**
     * setOutput - function to set Output value for this object.
     *
     * @param output
     */
    public void setOutput(InputOutput output) {
        this.output = output;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "NodeRoute [node = " + node + ", input = " + input + ", output = "
                + output + "]";
    }
}
