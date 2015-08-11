/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * VlanNorthBound - Bean Representaion for VlanNorthBound object from the JSON Response.
 *
 */
@JsonObject
public class VlanNorthBound {

    /**
     *unique id for this object.
     */
    @JsonElement(name = "id")
    private String id = null;

    /**
     * vlan for this object.
     */
    @JsonElement(name = "vlan")
    private int vlan = 0;

    /**
     * Reference of NodeDetails for this object.
     */
    @JsonObjectRef(name = "node")
    NodeDetails node = new NodeDetails();
    /**
     * Default Constructor.
     */
    public VlanNorthBound() {}

    /**
     * getId - function to get the id for this object.
     *
     * @return  The identifier for the VLAN map.
     */
    public String getId() {
        return id;
    }

    /**
     * setId - function to set the id for this object.
     *
     * @param id
     */
    public void setId(String id) {
        this.id = id;
    }

    /**
     * getVlan - function to get the vlan for this object.
     *
     * @return  The VLAN ID.
     */
    public int getVlan() {
        return vlan;
    }

    /**
     * setVlan - function to set the vlan for this object.
     *
     * @param vlan
     */
    public void setVlan(int vlan) {
        this.vlan = vlan;
    }

    /**
     * getNode - function to get the Node obj for this object.
     *
     * @return {@link NodeDetails}
     */
    public NodeDetails getNode() {
        return node;
    }

    /**
     * setNode - function to set the node for this object.
     *
     * @param node
     */
    public void setNode(NodeDetails node) {
        this.node = node;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "";
    }
}
