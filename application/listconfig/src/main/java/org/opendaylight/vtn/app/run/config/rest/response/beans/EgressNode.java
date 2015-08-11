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
 * DlSrc - Bean Representaion for DlSrc object from the JSON Response.
 *
 */
@JsonObject
public class EgressNode {

    /**
     * The tenant value to be created.
     */
    @JsonElement(name = "tenant")
    private String tenant = "vtn_1";

    /**
     * The bridge value to be created.
     */
    @JsonElement(name = "bridge")
    private String bridge = "bridge_1";

    /**
     * The interfacee value to be created.
     */
    @JsonElement(name = "interfacee")
    private String interfacee = "if_h1";

    public EgressNode() {
    }

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
     * @param tenant
     */
    public void setTenant(String tenant) {
        this.tenant = tenant;
    }

    /**
     * getBridge - function to get the bridge value for this object.
     *
     * @return {@link String}
     */
    public String getBridge() {
        return bridge;
    }

    /**
     * setBridge - function to set the bridge value for this object.
     *
     * @param bridge
     */
    public void setBridge(String bridge) {
        this.bridge = bridge;
    }

    /**
     * getInterfacee - function to get the interfacee value for this object.
     *
     * @return {@link String}
     */
    public String getInterfacee() {
        return interfacee;
    }

    /**
     * setInterfacee - function to set the interfacee value for this object.
     *
     * @param interfacee
     */
    public void setInterfacee(String interfacee) {
        this.interfacee = interfacee;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTN RUN CONFIG [VTenant:" + tenant + ",VTerminal:" + interfacee
                + ",bridge:" + bridge + "]";
    }
}
