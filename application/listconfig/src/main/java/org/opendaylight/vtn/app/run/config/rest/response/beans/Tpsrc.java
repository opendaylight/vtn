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
 * Tpsrc - Bean Representaion for Tpsrc object from the JSON Response.
 *
 */
@JsonObject
public class Tpsrc {

    /**
     * priority for Port
     */
    @JsonElement(name = "port")
    private int port = 0;
    /**
     * Default Constructor
     */
    public Tpsrc() {}

    /**
     * getDscp - function to get the dscp value for this object.
     *
     * @return The port value.
     */
    public int getPort() {
        return port;
    }

    /**
     * setPort - function to set the port value for this object.
     *
     * @param port
     */
    public void setPort(int port) {
        this.port = port;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "port:" + port;
    }
}
