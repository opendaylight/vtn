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
 * Inet - Bean Representaion for Inet object from the JSON Response.
 *
 */
@JsonObject
public class Inet {
    /**
     * Source values for the Inet.
     */
    @JsonElement(name = "src")
    private String src = null;

    /**
     * destination values for the Inet.
     */
    @JsonElement(name = "dst")
    private String dst = null;

    /**
     * protocol values for the Inet.
     */
    @JsonElement(name = "protocol")
    private int protocol;
    /**
     * Inet - Constructor with arguments.
     * @param src - Source values for the Inet.
     * @param dst - destination values for the Inet.
     * @param protocol - protocol values for the Inet.
     */

    public Inet(String src, String dst, int protocol) {
        this.src = src;
        this.dst = dst;
        this.protocol = protocol;
    }

    public Inet() {
    }

    /**
     * getSrc - function to get the Src values for this object.
     *
     * @return {@link String}
     */
    public String getSrc() {
        return src;
    }

    /**
     * setSrc - function to set the Src values for this object.
     *
     * @param src
     */
    public void setSrc(String src) {
        this.src = src;
    }

    /**
     * getDst - function to get the Destination values for this object.
     */
    public String getDst() {
        return dst;
    }

    /**
     * setDst - function to set the Destination values for this object.
     *
     * @param dst
     */
    public void setDst(String dst) {
        this.dst = dst;
    }

    /**
     * getProtocol - function to get the protocol values for this object.
     *
     * @return The protocol number.
     */
    public int getProtocol() {
        return protocol;
    }

    /**
     * setProtocol - function to set the protocol values for this object.
     *
     * @param protocol
     */
    public void setProtocol(int protocol) {
        this.protocol = protocol;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Inet [src = " + src + ", dst = " + dst + ", protocol = " + protocol
                + "]";
    }
}
