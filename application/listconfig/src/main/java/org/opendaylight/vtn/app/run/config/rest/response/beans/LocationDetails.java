/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

@JsonObject
public class LocationDetails implements Serializable, Cloneable {

    /**
     * Serialized ID for this class.
     */
    private static final long serialVersionUID = -2310385436056964546L;

    /**
     * NodeDetails for the locationDetails object.
     */
    @JsonObjectRef(name = "node")
    private NodeDetails node = new NodeDetails();

    /**
     * PortDetails for the locationDetails object.
     */
    @JsonObjectRef(name = "port")
    private PortDetails port = new PortDetails();

    public LocationDetails() {
    }

    public LocationDetails(NodeDetails node, PortDetails port) {
        super();
        this.node = node;
        this.port = port;
    }

    /**
     * getNode - function to get the Node values for this object.
     *
     * @return {@link NodeDetails}
     */
    public NodeDetails getNode() {
        return node;
    }

    /**
     * setNode - function to set the Node values for this object.
     *
     * @param node
     */
    public void setNode(NodeDetails node) {
        this.node = node;
    }

    /**
     * getPort - function to get the port values for this object.
     *
     * @return {@link PortDetails}
     */
    public PortDetails getPort() {
        return port;
    }

    /**
     * setPort - function to set the port values for this object.
     *
     * @param port
     */
    public void setPort(PortDetails port) {
        this.port = port;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Location [node:" + node + "port:" + port + "]";
    }
}
