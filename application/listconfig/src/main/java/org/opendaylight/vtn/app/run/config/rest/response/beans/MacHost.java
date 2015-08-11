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
 * MacHost - Bean Representaion for MacHost object from the JSON Response.
 *
 */
@JsonObject
public class MacHost {

    /**
     * address attribute for MacHost
     */
    @JsonElement(name = "address")
    private String address = "";

    /**
     * vlan attribute for MacHost
     */
    @JsonElement(name = "vlan")
    private int vlan;

    public MacHost() {}

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "address:" + address + ",vlan:" + vlan;
    }
}
