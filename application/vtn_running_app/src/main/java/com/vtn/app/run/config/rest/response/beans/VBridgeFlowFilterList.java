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

@RestURL(vtnMgrUrl = "controller/nb/v2/vtn {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/flowfilters/ {listType}")
@JsonObject
public class VBridgeFlowFilterList implements Cloneable, Serializable {

    /**
     * Serialized id for this class.
     */
    private static final long serialVersionUID = -5489169108073387630L;

    /**
     * Array of flowfilter objects.
     */
    @JsonArray(name = "flowfilter")
    private List<FlowFilter> flowfilter = new ArrayList<FlowFilter>();
    /**
     * Constructor.
     */
    public VBridgeFlowFilterList() {
    }

    /**
     * getFlowfilter - function to get the list of flowfilter value for this object.
     *
     * @return {@link List<FlowFilter>}
     */
    public List<FlowFilter> getFlowfilter() {
        return flowfilter;
    }

    /**
     * setFlowfilter - function to set the list of flowfilter value for this object.
     *
     * @param flowfilter
     */
    public void setFlowfilter(List<FlowFilter> flowfilter) {
        this.flowfilter = flowfilter;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "[flowfilter:" + flowfilter + "]";
    }
}
