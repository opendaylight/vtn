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
/**
 * NodeDetails - Bean Representaion for NodeDetails object from the JSON Response.
 *
 */
@JsonObject
public class NodeDetails {

    /**
     * Node type
     */
    @JsonElement(name = "type")
    private String type = null;

    /**
     * Unique Id for Node.
     */
    @JsonElement(name = "id")
    private String id = null;

    public NodeDetails() { }

    /**
     * getType - function to get the type for this object.
     *
     * @return  The type of the node.
     */
    public String getType() {
        return type;
    }

    /**
     * setCode - function to set the type for this object.
     *
     * @param type
     */
    public void setType(String type) {
        this.type = type;
    }

    /**
     * getType - function to get the value of id for this object.
     *
     * @return The identifier of the node.
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
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "NodeDetails[type:" + type + ",id:" + id + "]";
    }
}
