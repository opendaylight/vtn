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
 * InetMatch - Bean Representaion for InetMatch object from the JSON Response.
 *
 */
@JsonObject
public class InetMatch {

    /**
     * inet reference.
     */
    @JsonObjectRef(name = "inet4")
    Inet inet4 = new Inet();

    public InetMatch(Inet inet) {
        this.inet4 = inet;
    }

    public InetMatch() {
    }

    /**
     * getInet - function to get the inet4 values for this object.
     *
     * @return {@link Inet}
     */
    public Inet getInet() {
        return inet4;
    }

    /**
     * setInet - function to set the inet4 values for this object.
     *
     * @param inet
     */
    public void setInet(Inet inet) {
        this.inet4 = inet;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "InetMatch [inet4 = " + inet4 + "]";
    }
}
