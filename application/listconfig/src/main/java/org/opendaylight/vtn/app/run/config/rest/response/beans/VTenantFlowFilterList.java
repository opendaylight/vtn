/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * VTenantFlowFilterList - Bean Representaion for array of VTenantFlowFilter object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/flowfilters")
@JsonObject
public class VTenantFlowFilterList implements Cloneable, Serializable {

    /**
     * serialization ID for this calss.
     */
    private static final long serialVersionUID = -4315590746123101315L;

    /**
     * List of flowfilter reference.
     */
    @JsonArray(name = "flowfilter")
    private List<FlowFilter> flowfilter = new ArrayList<FlowFilter>();
    /**
     * Default Constructor.
     */
    public VTenantFlowFilterList() {
    }

    /**
     * getPath - function to get the list of flowfilter objects.
     *
     * @return List of {@link FlowFilter} objects
     */
    public List<FlowFilter> getFlowfilter() {
        return flowfilter;
    }

    /**
     * setFlowfilter - function to set the list of flowfilters.
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
