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
 * IngressNode - Bean Representaion for IngressNode object from the JSON Response.
 *
 */
@JsonObject
public class IngressNode {

    /**
     * tenant name for this Node.
     */
    @JsonElement(name = "tenant")
    private String tenant  = "vtn_1";

    /**
     * bridge name for this Node.
     */
    @JsonElement(name = "bridge")
    private String bridge  = "bridge_1";

    public IngressNode() {}

    /**
     * getTenant - function to get the tenant value for this object.
     *
     * @return {@link String}
     */
    public String getTenant() {
        return tenant;
    }

    /**
     * setTenant - function to set the tenant value for this object.
     *
     * @param tenant {@link String}
     */
    public void setTenant(String tenant) {
        this.tenant = tenant;
    }

    /**
     * getBridge - function to get the bridge values for this object.
     *
     * @return {@link String}
     */
    public String getBridge() {
        return bridge;
    }

    /**
     * setBridge - function to set the bridge values for this object.
     *
     * @param bridge
     */
    public void setBridge(String bridge) {
        this.bridge = bridge;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "IngressNode [tenant = " + tenant + ", bridge = " + bridge + "]";
    }
}
