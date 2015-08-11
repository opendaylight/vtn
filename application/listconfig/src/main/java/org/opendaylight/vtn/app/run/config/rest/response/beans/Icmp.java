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
 * Icmp - Bean Representaion for Icmp object from the JSON Response.
 *
 */
@JsonObject
public class Icmp {

    /**
     * Type attribute for Icmp.
     */
    @JsonElement(name = "type")
    private int type = 0;

    /**
     * code attribute for Icmp.
     */
    @JsonElement(name = "code")
    private int code = 0;

    public Icmp() {
    }
    /**
     * Icmp - Constructor with arguments.
     * @param type - Type attribute for Icmp.
     * @param code - code attribute for Icmp.
     */
    public Icmp(int type, int code) {
        this.type = type;
        this.code = code;
    }

    /**
     * getIndex - function to get the type value for this object.
     *
     * @return The ICMP type.
     */
    public int getType() {
        return type;
    }

    /**
     * setType - function to set the type values for this object.
     *
     * @param type
     */
    public void setType(int type) {
        this.type = type;
    }

    /**
     * getCode - function to get the code values for this object.
     *
     * @return The ICMP code.
     */
    public int getCode() {
        return code;
    }

    /**
     * setCode - function to set the code value for this object.
     *
     * @param code
     */
    public void setCode(int code) {
        this.code = code;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Icmp [type = " + type + ", code = " + code + "]";
    }
}
