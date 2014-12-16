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

@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/mac")
@JsonObject
public class MacEntryList implements Serializable, Cloneable {

    private static final long serialVersionUID = 2002901637718900671L;

    @JsonArray(name = "macentry")
    private List<MacEntry> macentry = new ArrayList<MacEntry>();

    public List<MacEntry> getMacentry() {
        return macentry;
    }

    public void setMacentry(List<MacEntry> macentry) {
        this.macentry = macentry;
    }

    @Override
    public String toString() {
        return "macentry:" + macentry;
    }
}
