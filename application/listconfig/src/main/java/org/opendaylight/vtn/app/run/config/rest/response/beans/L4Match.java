/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * L4Match - Bean Representaion for L4Match object from the JSON Response.
 *
 */
@JsonObject
public class L4Match {

    /**
     * Reference attribute for Icmp
     */
    @JsonObjectRef(name = "icmp")
    Icmp icmp = new Icmp();

    public L4Match() {
    }

    public L4Match(Icmp icmp) {
        this.icmp = icmp;
    }

    /**
     * getIcmp - function to get the icmp reference for this object.
     *
     * @return {@link Icmp}
     */
    public Icmp getIcmp() {
        return icmp;
    }

    /**
     * setIcmp - function to set the icmp reference for this object.
     *
     * @param icmp
     */
    public void setIcmp(Icmp icmp) {
        this.icmp = icmp;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "L4Match [icmp = " + icmp + "]";
    }
}
