/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.util.ArrayList;
import java.util.List;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;

/**
 * Deny - Bean Representaion for Deny object from the JSON Response.
 *
 */
@JsonObject
public class Deny {

    /**
     * The machost value.
     */
    @JsonArray(name = "machost")
    private List<MacHost> machost = new ArrayList<MacHost>();

    /**
     * getMachost - function to get the MacHosts.
     *
     * @return List of MacHost objects
     */
    public List<MacHost> getMachost() {
        return machost;
    }

    /**
     * setMachost - function to set the MacHosts.
     *
     * @param machost
     * */
    public void setMachost(List<MacHost> machost) {
        this.machost = machost;
    }

    public Deny() {
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "machost:" + machost;
    }
}
