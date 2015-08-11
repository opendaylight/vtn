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
 * Ethernet - Bean Representaion for Ethernet object from the JSON Response.
 *
 */
@JsonObject
public class Ethernet {

    /**
     * The src value for this object.
     */
    @JsonElement(name = "src")
    private String src = null;

    /**
     * The dst value for this object.
     */
    @JsonElement(name = "dst")
    private String dst = null;

    /**
     * The Ethernet type value for this object.
     */
    @JsonElement(name = "type")
    private int type = 0;

    /**
     * The vlan value for this object.
     */
    @JsonElement(name = "vlan")
    private int vlan = 0;

    public Ethernet() {

    }
    /**
     * Ethernet - Constructor with arguments.
     * @param src -  src value for this object.
     * @param dst - dst value for this object.
     * @param type -Ethernet type.
     * @param vlan - vlan value.
     */
    public Ethernet(String src, String dst, int type, int vlan) {
        this.src = src;
        this.dst = dst;
        this.type = type;
        this.vlan = vlan;
    }

    /**
     * getSrc - function to get the src for this object.
     *
     * @return {@link String}
     */
    public String getSrc() {
        return src;
    }

    /**
     * setSrc - function to set the src for this object.
     *
     * @param src
     */
    public void setSrc(String src) {
        this.src = src;
    }

    /**
     * getSrc - function to get the source for this object.
     *
     * @return {@link String}
     */
    public String getDst() {
        return dst;
    }

    /**
     * setDst - function to set the destination for this object.
     *
     * @param dst
     */
    public void setDst(String dst) {
        this.dst = dst;
    }

    /**
     * getType - function to get the Ethernet type value for this object.
     *
     * @return {@link String}
     */
    public int getType() {
        return type;
    }

    /**
     * setType - function to set the type for this object.
     *
     * @param type
     */
    public void setType(int type) {
        this.type = type;
    }

    /**
     * getVlan - function to get the vlan value for this object.
     *
     * @return {@link String}
     */
    public int getVlan() {
        return vlan;
    }

    /**
     * setVlan - function to set the vlan value for this object.
     *
     * @param vlan
     */
    public void setVlan(int vlan) {
        this.vlan = vlan;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Ethernet [src = " + src + ", dst = " + dst + ", type = " + type
                + "vlan" + vlan + "]";
    }
}
