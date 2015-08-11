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
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * MacMapNorthBound - Bean Representaion for MacMapNorthBound object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/macmap")
@JsonObject
public class MacMapNorthBound {

    /**
     * Allow reference for this object
     */
    @JsonObjectRef(name = "allow")
    private Allow allow = new Allow();

    /**
     * Deny reference for this object
     */
    @JsonObjectRef(name = "deny")
    private Deny deny = new Deny();

    /**
     * MacMapped reference for this object
     */
    @JsonObjectRef(name = "mapped")
    private MacMapped mapped = new MacMapped();

    public MacMapNorthBound() {}

    /**
     * getAllow - function to get the allow values for this object.
     *
     * @return {@link String}
     */
    public Allow getAllow() {
        return allow;
    }

    /**
     * getAllow - function to set the allow values for this object.
     *
     * @param allow
     */
    public void setAllow(Allow allow) {
        this.allow = allow;
    }

    /**
     * getDeny - function to get the Deny values for this object.
     *
     *@return {@link String}
     */
    public Deny getDeny() {
        return deny;
    }

    /**
     * setDeny - function to set the deny values for this object.
     *
     * @param deny
     */
    public void setDeny(Deny deny) {
        this.deny = deny;
    }

    /**
     * getAllow - function to get the allow values for this object.
     *
     * @return {@link MacMapped}
     */
    public MacMapped getMapped() {
        return mapped;
    }

    /**
     * setMapped - function to set the mapped values for this object.
     *
     * @param mapped
     */
    public void setMapped(MacMapped mapped) {
        this.mapped = mapped;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "allow:" + allow + ",deny:" + deny + ",mapped:" + mapped;
    }
}
