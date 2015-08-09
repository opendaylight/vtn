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
 * VNodeRoute - Bean Representaion for VNodeRoute object from the JSON Response.
 *
 */
@JsonObject
public class VNodeRoute {

    /**
     * Reason attribute for VNodeRoute
     */
    @JsonElement(name = "reason")
    private String reason =  "FORWARDED";

    /**
     * Path for node route
     */
    @JsonObjectRef(name = "Path")
    private EgressNode path = new EgressNode();

    /**
     * getReason - function to get the reason for this object.
     *
     * @return {@link String }
     */
    public String getReason() {
        return reason;
    }

    /**
     * setReason - function to set the reason for this object.
     *
     * @param reason
     */
    public void setReason(String reason) {
        this.reason = reason;
    }

    /**
     * getPath - function to get the path for this object.
     *
     * @return {@link EgressNode }
     */
    public EgressNode getPath() {
        return path;
    }

    /**
     * setPath - function to set the path for this object.
     *
     * @param path
     */
    public void setPath(EgressNode path) {
        this.path = path;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VNodeRoute [reason = " + reason + ", path = " + path + "]";
    }

    public VNodeRoute() {}
}
