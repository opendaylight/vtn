/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package com.vtn.app.run.config.rest.response.beans;

import com.vtn.app.run.config.json.annotations.JsonElement;
import com.vtn.app.run.config.json.annotations.JsonObject;

@JsonObject
public class VTenantPathMapNorthBound {

    /**
     * unique pathmap value for vtenant.
     */
    @JsonElement(name = "index")
    int index = 0;

    /**
     * Condition value for pathmap in vtenant.
     */
    @JsonElement(name = "condition")
    String condition = null;

    /**
     * Policy for pathmap in vtenant.
     */
    @JsonElement(name = "policy")
    int policy = 0;

    /**
     * Maximum idle time .
     */
    @JsonElement(name = "idleTimeout")
    int idleTimeout = 0;

    /**
     * Life time the vtenant.
     */
    @JsonElement(name = "hardTimeout")
    int hardTimeout = 0;

    public VTenantPathMapNorthBound() {
    }

    public VTenantPathMapNorthBound(int index, String condition,
            int idleTimeout, int hardTimeout) {
        this.index = index;
        this.condition = condition;
        this.idleTimeout = idleTimeout;
        this.hardTimeout = hardTimeout;
    }

    /**
     * getPolicy - function to get the policy for this object.
     *
     * @return {@link int }
     */
    public int getIndex() {
        return index;
    }

    /**
     * setIndex - function to set the index for this object.
     *
     * @param idleTimeout
     */
    public void setIndex(int index) {
        this.index = index;
    }

    /**
     * getCondition - function to get the condition for this object.
     *
     * @return {@link int }
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
     * @return {@link int }
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
     * @return {@link int }
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
     * @return {@link int }
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

    @Override
    public String toString() {
        return "pathmap [index :" + index + ",condition:" + condition
                + ",policy:" + policy + ",idleTimeout:" + idleTimeout
                + ",hardTimeout:" + hardTimeout + "]";
    }
}
