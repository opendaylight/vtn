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
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * Icmp - Bean Representaion for Icmp object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/pathpolicies/ {policyId}")
@JsonObject
public class PathPolicy implements Serializable, Cloneable {

    /**
     * Serialization id for this object.
     */
    private static final long serialVersionUID = 7281946062087957084L;

    @JsonElement(name = "id")
    private int id;

    /**
     * Default pathpolicy for this object.
     */
    @JsonElement(name = "default")
    private int defaultInt;

    /**
     * Cost pathpolicy for this object.
     */
    @JsonArray(name = "cost")
    private List<CostDetails> costList = new ArrayList<CostDetails>();

    public PathPolicy() {
    }

    public PathPolicy(int id, int defaultInt, List<CostDetails> costList) {
        this.id = id;
        this.defaultInt = defaultInt;
        this.costList = costList;
    }

    /**
     * getId - Id value can be obtained from this method
     *
     * @return The identifier for the path policy.
     */
    public int getId() {
        return id;
    }

    /**
     * setId - Set Id value for this object
     *
     * @param id
     */
    public void setId(int id) {
        this.id = id;
    }

    /**
     * getDefaultInt - default value can be obtained from this method
     *
     * @return The default cost.
     */
    public int getDefaultInt() {
        return defaultInt;
    }

    /**
     * set_default - default value can be set in this method
     *
     * @param defaultInt
     */
    public void setDefaultInt(int defaultInt) {
        this.defaultInt = defaultInt;
    }

    /**
     * getCostList - costList can be obtained from this method
     *
     * @return List of CostDetails
     */
    public List<CostDetails> getCostList() {
        return costList;
    }

    /**
     * set_default - costList value can be set in this method
     *
     * @param costList
     */
    public void setCostList(List<CostDetails> costList) {
        this.costList = costList;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "PathPolicy [id:" + id + ", default:" + defaultInt + ", costList:"
                + costList + "]";
    }
}
