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
 * InputOutput - Bean Representaion for InputOutput object from the JSON Response.
 *
 */
@JsonObject
public class InputOutput {

    /**
     * Identifiier for InputOutput
     */
    @JsonElement(name = "id")
    private String id =  "2";

    /**
     * Type of InputOutput
     */
    @JsonElement(name = "type")
    private String type  = "OF";

    /**
     * Name of InputOutput
     */
    @JsonElement(name = "name")
    private String name =  "s4-eth2";

    public InputOutput() {}

    /**
     * getId - function to get the id for this object.
     *
     * @return The identifier of the switch port.
     */
    public String getId() {
        return id;
    }

    /**
     * setId - function to set the idfor this object.
     *
     * @param id
     */
    public void setId(String id) {
        this.id = id;
    }

    /**
     * getType - function to get the type for this object.
     *
     * @return {@link String}
     */
    public String getType() {
        return type;
    }

    /**
     * setType - function to set the idfor this object.
     *
     * @param type
     */
    public void setType(String type) {
        this.type = type;
    }

    /**
     * getName - function to get the name for this object.
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * setName - function to set the name for this object.
     *
     * @param name {@link String}
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTN RUN CONFIG [type:" + type + ",id:" + id + "]";
    }
}
