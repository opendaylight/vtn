/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
/**
 * VTenantPathMapNorthBound - Bean Representaion for VTenantPathMapNorthBound object from the JSON Response.
 */
@JsonObject
public class VTenantPathMapNorthBound {

    /**
     * unique pathmap value for vtenant.
     */
    @JsonElement(name = "index")
    private int index = 0;

    /**
     * Condition value for pathmap in vtenant.
     */
    @JsonElement(name = "condition")
    private String condition = null;

    /**
     * Policy for pathmap in vtenant.
     */
    @JsonElement(name = "policy")
    private int policy = 0;

    /**
     * Maximum idle time .
     */
    @JsonElement(name = "idleTimeout")
    private int idleTimeout = 0;

    /**
     * Life time the vtenant.
     */
    @JsonElement(name = "hardTimeout")
    private int hardTimeout = 0;

    /**
     * Default Constructor.
     */
    public VTenantPathMapNorthBound() {
    }

    /**
     * Parameterized Constructor.
     * @param index
     * @param condition
     * @param idleTimeout
     * @param hardTimeout
     */
    public VTenantPathMapNorthBound(int index, String condition,
            int idleTimeout, int hardTimeout) {
        this.index = index;
        this.condition = condition;
        this.idleTimeout = idleTimeout;
        this.hardTimeout = hardTimeout;
    }

    /**
     * getPolicy - function to get the index for this object.
     *
     * @return  The index for this object.
     */
    public int getIndex() {
        return index;
    }

    /**
     * setIndex - function to set the index for this object.
     *
     * @param index
     */
    public void setIndex(int index) {
        this.index = index;
    }

    /**
     * getCondition - function to get the condition for this object.
     *
     * @return  The name of the flow condition.
     */
    public String getCondition() {
        return condition;
    }

    /**
     * setCondition - function to set the condition for this object.
     *
     * @param condition
     */
    public void setCondition(String condition) {
        this.condition = condition;
    }

    /**
     * getPolicy - function to get the policy for this object.
     *
     * @return  The path policy identifier.
     */
    public int getPolicy() {
        return policy;
    }

    /**
     * setPolicy - function to set the policy for this object.
     *
     * @param policy
     */
    public void setPolicy(int policy) {
        this.policy = policy;
    }

    /**
     * getIdleTimeout - function to get the idleTimeout for this object.
     *
     * @return The idle timeout value.
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * setIdleTimeout - function to set the idleTimeout for this object.
     *
     * @param idleTimeout
     */
    public void setIdleTimeout(int idleTimeout) {
        this.idleTimeout = idleTimeout;
    }

    /**
     * getVtnName - function to get the hardTimeout for this object.
     *
     * @return The hard timeout value.
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * setHardTimeout - function to set the hardTimeout for this object.
     *
     * @param hardTimeout
     */
    public void setHardTimeout(int hardTimeout) {
        this.hardTimeout = hardTimeout;
    }


    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "pathmap [index :" + index + ",condition:" + condition
                + ",policy:" + policy + ",idleTimeout:" + idleTimeout
                + ",hardTimeout:" + hardTimeout + "]";
    }
}
