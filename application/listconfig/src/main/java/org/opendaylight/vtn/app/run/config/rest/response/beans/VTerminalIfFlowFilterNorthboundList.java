/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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
 * VTerminalIfFlowFilterNorthboundList - Bean Representaion for VTerminal flowfilter object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vterminals/ {terminalName}/interfaces/ {ifName}/flowfilters/ {listType}")
@JsonObject
public class VTerminalIfFlowFilterNorthboundList implements Cloneable, Serializable {

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
    public VTerminalIfFlowFilterNorthboundList() {
    }

    /**
     * getFlowfilter - function to get the list of flowfilter value for this object.
     *
     * @return List of {@link FlowFilter} objects
     */
    public List<FlowFilter> getFlowfilters() {
        return flowfilter;
    }

    /**
     *  setFlowfilter - function to set the list of flowfilter value for this object.
     *
     *  @param flowfilter
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

