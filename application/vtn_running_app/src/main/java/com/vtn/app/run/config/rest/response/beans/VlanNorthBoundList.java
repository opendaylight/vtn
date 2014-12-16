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

@JsonObject
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/vlanmaps")
public class VlanNorthBoundList implements Serializable, Cloneable {

    private static final long serialVersionUID = 836750948728896411L;

    @JsonArray(name = "vlanmap")
    private List<VlanNorthBound> vlanmap = new ArrayList<VlanNorthBound>();

    public VlanNorthBoundList() {}

    public List<VlanNorthBound> getVlanmap() {
        return vlanmap;
    }

    public void setVlanmap(List<VlanNorthBound> vlanmap) {
        this.vlanmap = vlanmap;
    }

    @Override
    public String toString() {
        return "vlanmap:" + vlanmap;
    }

}
