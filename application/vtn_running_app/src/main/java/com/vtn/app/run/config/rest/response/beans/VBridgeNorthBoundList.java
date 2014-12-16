/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package com.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import com.vtn.app.run.config.json.annotations.JsonArray;
import com.vtn.app.run.config.json.annotations.JsonObject;
import com.vtn.app.run.config.rest.enumgroups.RestURL;

@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges")
@JsonObject
public class VBridgeNorthBoundList implements Serializable, Cloneable {

    /**
     * Serialize ID constant fort this class.
     */
    private static final long serialVersionUID = 3891236058591482631L;

    @JsonArray(name = "vbridge")
    private List<VBridgeNorthBound> vBridge = new ArrayList<VBridgeNorthBound>();

    /**
     * getVbridge - function to get the list of vBridge values for this object.
     *
     * @return {@link List<VBridgeNorthBound>}
     */
    public List<VBridgeNorthBound> getVbridge() {
        return vBridge;
    }

    /**
     * setVbridge - function to set the lisr of vbridge value for this object.
     *
     * @param vbridge
     */
    public void setVbridge(List<VBridgeNorthBound> vbridge) {
        this.vBridge = vbridge;
    }

    /**
     * String representation of the object.
     *
     */
    public String toString() {
        return "vBridge[vbridge:" + vBridge + "]";
    }
}
