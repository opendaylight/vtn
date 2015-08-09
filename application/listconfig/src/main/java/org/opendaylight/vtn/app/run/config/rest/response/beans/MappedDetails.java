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
 * MappedDetails - Bean Representaion for MappedDetails object from the JSON
 * Response.
 *
 */

@JsonObject
public class MappedDetails {

    /**
     * Type attribute for MappedDetails
     */
    @JsonElement(name = "type")
    private String type = null;

    /**
     * Identifier attribute for MappedDetails
     */
    @JsonElement(name = "id")
    private String id = null;

    public MappedDetails() {
    }

    /**
     * getType - function to get the type values for this object.
     *
     * @return {@link String}
     */
    public String getType() {
        return type;
    }

    /**
     * setType - function to set the type values for this object.
     *
     * @param type
     */
    public void setType(String type) {
        this.type = type;
    }

    /**
     * getId - function to get the Id values for this object.
     *
     * @return The identifier of the switch port.
     */
    public String getId() {
        return id;
    }

    /**
     * setId - function to set the type values for this object.
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
        return "[type:" + type + ",id:" + id + "]";
    }
}
