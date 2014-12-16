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

import com.vtn.app.run.config.json.annotations.JsonObject;
import com.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * DataFlowList - Bean Representaion for DataFlow objects from the JSON
 * Response.
 *
 */

@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/flows/detail")
@JsonObject
public class DataFlowList implements Serializable, Cloneable {

    /**
     * Serialization ID for thi object
     */

    private static final long serialVersionUID = 43423307945540270L;

    /**
     * Serialization ID for thi object
     */
    private List<DataFlow> dataFlowlist = new ArrayList<DataFlow>();

    public DataFlowList() {
    }

    /**
     * getDataFlowlist - function to get the dataFlows.
     *
     * @return {@link List<dataFlowlist>}
     */
    public List<DataFlow> getDataFlowlist() {
        return dataFlowlist;
    }

    /**
     * setDataFlowlist - function to set the dataFlowlist.
     *
     * @params dataFlowlist
     */
    public void setDataFlowlist(List<DataFlow> dataFlowlist) {
        this.dataFlowlist = dataFlowlist;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "" + dataFlowlist;
    }

}
