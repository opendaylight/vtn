/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

/**
 * Dldst - Bean Representaion for Dldst object from the JSON Response.
 *
 */
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;

@JsonObject
public class Dldst {

    /**
     * The address value for this object.
     */
    @JsonElement(name = "address")
    private String address = "";

    public Dldst() {
    }

    /**
     * getAddress - function to get the address for this object.
     *
     * @return {@link String}
     */
    public String getAddress() {
        return address;
    }

    /**
     * setAddress - function to set the address value for this object.
     *
     * @param address
     */
    public void setAddress(String address) {
        this.address = address;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "address:" + address;
    }
}
