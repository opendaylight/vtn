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
 * VTenantNorthBound - Bean Representaion for VTenant object from the JSON
 * Response.
 *
 */
@JsonObject
public class VTenantNorthBound {

    /**
     * name for the vtenant created.
     */
    @JsonElement(name = "name")
    String name = null;

    /**
     * description for the vtenant created.
     */
    @JsonElement(name = "description")
    String description = null;

    /**
     * Maximum idle time .
     */
    @JsonElement(name = "idleTimeout")
    int idleTimeout = 0;

    /**
     * Life time the vtenant.
     */
    @JsonElement(name = "hardTimeout")
    int hardTimeout = 0;
    /**
     * Default Constructor.
     */
    public VTenantNorthBound() {
    }

    /**
     * Parameterized Constructor.
     * @param vtnName
     * @param vtnDesc
     * @param idleTimeout
     * @param hardTimeout
     */
    public VTenantNorthBound(String vtnName, String vtnDesc, int idleTimeout,
            int hardTimeout) {
        this.name = vtnName;
        this.description = vtnDesc;
        this.idleTimeout = idleTimeout;
        this.hardTimeout = hardTimeout;
    }

    /**
     * getVtnName - function to get the name for this object.
     *
     * @return {@link String }
     */
    public String getVtnName() {
        return name;
    }

    /**
     * setVtnName - function to set the name for this object.
     *
     * @param vtnName
     */
    public void setVtnName(String vtnName) {
        this.name = vtnName;
    }

    /**
     * getVtnDesc - function to get the description for this object.
     *
     * @return {@link String }
     */
    public String getVtnDesc() {
        return description;
    }

    /**
     * description - function to set the description for this object.
     *
     * @param vtnDesc
     */
    public void setVtnDesc(String vtnDesc) {
        this.description = vtnDesc;
    }

    /**
     * getIdleTimeout - function to get the idleTimeout for this object.
     *
     * @return The idle timeout value.
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * setIdleTimeout - function to set the idleTimeout for this object.
     *
     * @param idleTimeout
     */
    public void setIdleTimeout(int idleTimeout) {
        this.idleTimeout = idleTimeout;
    }

    /**
     * getHardTimeout - function to get the hardTimeout for this object.
     *
     * @return The hard timeout value.
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * setHardTimeout - function to set the hardTimeout for this object.
     *
     * @param hardTimeout
     */
    public void setHardTimeout(int hardTimeout) {
        this.hardTimeout = hardTimeout;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTN [name:" + name + ", description:" + description + ", "
                + "idleTimeOut:" + idleTimeout + ",hardTimeOut:" + hardTimeout
                + "]";
    }
}
