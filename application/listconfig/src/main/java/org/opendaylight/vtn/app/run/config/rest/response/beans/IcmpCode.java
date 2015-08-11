/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;

/**
 * IcmpType - Bean Representaion for IcmpType object from the JSON Response.
 *
 */
@JsonObject
public class IcmpCode {

    /**
     * priority for Port
     */
    @JsonElement(name = "code")
    private int code = 0;
    /**
     * Default Constructor
     */
    public IcmpCode() {}

    /**
     * getCode - function to get the code value for this object.
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
        return "code:" + code;
    }
}
