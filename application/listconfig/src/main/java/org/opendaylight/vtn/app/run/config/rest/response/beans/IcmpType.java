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
public class IcmpType {

    /**
     * priority for Port
     */
    @JsonElement(name = "type")
    private int type = 0;
    /**
     * Default Constructor
     */
    public IcmpType() {}

    /**
     * getDscp - function to get the type value for this object.
     *
     * @return The ICMP type.
     */
    public int getType() {
        return type;
    }

    /**
     * setType - function to set the type value for this object.
     *
     * @param type
     */
    public void setType(int type) {
        this.type = type;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "type:" + type;
    }
}
