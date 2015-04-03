/**
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * VBridgeIfFlowFilterNorthboundList - Bean Representaion for VBridge flowfilter object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/interfaces/ {ifName}/flowfilters/ {listType}")
@JsonObject
public class VBridgeIfFlowFilterNorthboundList implements Cloneable, Serializable {

    /**
     * Serialized id for this class.
     */
    private static final long serialVersionUID = 847469108068987630L;

    /**
     * Array of flowfilter objects.
     */
    @JsonArray(name = "flowfilter")
    private List<FlowFilter> flowfilter = new ArrayList<FlowFilter>();

    /**
     * Default Constructor.
     */
    public VBridgeIfFlowFilterNorthboundList() {
    }

    /**
     * getFlowfilter - function to get the list of flowfilter value for this object.
     *
     * @return {@link List<FlowFilter>}
     */
    public List<FlowFilter> getFlowfilters() {
        return flowfilter;
    }

    /**
     *  setFlowfilter - function to set the list of flowfilter value for this object.
     *
     *  @param flowfilters
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
        return "[flowfilter" + flowfilter + "]";
    }
}

