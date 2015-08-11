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
 * Destination - Bean Representaion for Destination object from the JSON Response.
 *
 */
@JsonObject
public class Destination {

    /**
     * The bridge value for this object.
     */
    @JsonElement(name = "bridge")
    private String bridge = "";

    /**
     * The interface value for this object.
     */
    @JsonElement(name = "interface")
    private String interfaces = "";

    public Destination() {}

    /**
     * getBridge - function to get the bridge for this object.
     *
     * @return {@link String}
     */
    public String getBridge() {
        return bridge;
    }

    /**
     * setBridge - function to set the bridge value for this object.
     *
     * @param bridge
     */
    public void setBridge(String bridge) {
        this.bridge = bridge;
    }

    /**
     * getInterfaces - function to get the interfaces for this object.
     *
     * @return {@link String}
     */
    public String getInterfaces() {
        return interfaces;
    }

    /**
     * setInterfaces - function to set the interfaces value for this object.
     *
     * @param interfaces
     */
    public void setInterfaces(String interfaces) {
        this.interfaces = interfaces;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "destination[bridge:" + bridge + ",interface:" + interfaces + "]";
    }
}
