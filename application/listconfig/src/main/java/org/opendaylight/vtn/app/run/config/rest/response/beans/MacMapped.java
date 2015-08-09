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
 * MacMapped - Bean Representaion for MacMapped object from the JSON Response.
 *
 */
@JsonObject
public class MacMapped {

    /**
     * macentry attribute for MacMapped
     */
    @JsonArray(name = "macentry")
    private List<MacEntry> macentry = new ArrayList<MacEntry>();

    public MacMapped() {}

    /**
     * getMacentry - function to get the macentry value for this object.
     *
     * @return List of MacEntry objects
     */
    public List<MacEntry> getMacentry() {
        return macentry;
    }

    /**
     * setMacentry - function to set the macentry value for this object.
     *
     * @param macentry
     */
    public void setMacentry(List<MacEntry> macentry) {
        this.macentry = macentry;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "macentry:" + macentry;
    }
}
