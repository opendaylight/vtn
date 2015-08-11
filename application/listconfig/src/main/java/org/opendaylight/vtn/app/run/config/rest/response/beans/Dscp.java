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
 * Dscp - Bean Representaion for dscp object from the JSON Response.
 *
 */
@JsonObject
public class Dscp {

    /**
     * priority for dscp
     */
    @JsonElement(name = "dscp")
    private int dscp = 0;
    /**
     * Default Constructor
     */
    public Dscp() {}

    /**
     * getDscp - function to get the dscp value for this object.
     *
     * @return The DSCP field value.
     */
    public int getDscp() {
        return dscp;
    }

    /**
     * setDscp - function to set the dscp value for this object.
     *
     * @param dscp
     */
    public void setDscp(int dscp) {
        this.dscp = dscp;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "dscp:" + dscp;
    }
}
