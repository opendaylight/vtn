/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * FlowconditionList - Bean Representaion for FlowconditionList object from the
 * JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/flowconditions")
@JsonObject
public class FlowconditionList {

    /**
     * The flowconditions value for this object.
     */
    @JsonArray(name = "flowcondition")
    List<Flowconditions> flowcondition = new ArrayList<Flowconditions>();

    public FlowconditionList() {
    }

    /**
     * getFlowconditionlist - function to get the flowcondition for this object.
     *
     * @return List of Flowconditions
     */
    public List<Flowconditions> getFlowconditionlist() {
        return flowcondition;
    }

    /**
     * setFlowconditionlist - function to set the Flowconditions for this
     * object.
     *
     * @param flowconditionlist
     */
    public void setFlowconditionlist(List<Flowconditions> flowconditionlist) {
        this.flowcondition = flowconditionlist;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Flowcondition [flowcondition = " + flowcondition + "]";
    }
}
