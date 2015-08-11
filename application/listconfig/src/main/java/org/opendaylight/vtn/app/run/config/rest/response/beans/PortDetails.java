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
 * PortDetails - Bean Representaion for PortDetails object from the JSON
 * Response.
 *
 */
@JsonObject
public class PortDetails {
    /**
     * Type of port used.
     */
    @JsonElement(name = "type")
    private String type = null;

    /**
     * Id of port used.
     */
    @JsonElement(name = "id")
    private String id = null;

    /**
     * name of port used.
     */
    @JsonElement(name = "name")
    private String name = null;

    public PortDetails() {
    }

    /**
     * getName - function to get the name for this object
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * setName - function to set the name for this object.
     *
     * @param name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * getType - function to get the type for this object
     *
     * @return {@link String}
     */
    public String getType() {
        return type;
    }

    /**
     * setType - function to set the type for this object
     *
     * @param type {@link String}
     */
    public void setType(String type) {
        this.type = type;
    }

    /**
     * getId - function to get the ID for this object
     *
     * @return {@link String}
     */
    public String getId() {
        return id;
    }

    /**
     * setId - function to set the ID for this object
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
        return "PortDetails[type:" + type + ",id:" + id + "]";
    }
}
