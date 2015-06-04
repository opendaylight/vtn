/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.beans.response;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

@JsonObject
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}")
public class VTenantConfig {

    @JsonElement(name = "description")
    String description = null;

    @JsonElement(name = "idleTimeout")
    int idleTimeout = 0;

    @JsonElement(name = "hardTimeout")
    int hardTimeout = 0;

    public VTenantConfig() {
    }

    public VTenantConfig(String description, int idleTimeout, int hardTimeout) {
        this.description = description;
        this.idleTimeout = idleTimeout;
        this.hardTimeout = hardTimeout;
    }

    public String getVtnDesc() {
        return description;
    }

    public void setVtnDesc(String vtnDesc) {
        this.description = vtnDesc;
    }

    public int getIdleTimeout() {
        return idleTimeout;
    }

    public void setIdleTimeout(int idleTimeout) {
        this.idleTimeout = idleTimeout;
    }

    public int getHardTimeout() {
        return hardTimeout;
    }

    public void setHardTimeout(int hardTimeout) {
        this.hardTimeout = hardTimeout;
    }

    @Override
    public String toString() {
        return "VTNConfig [description:" + description + ", " + "idleTimeOut:" + idleTimeout + ",hardTimeOut:" + hardTimeout + "]";
    }
}
