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

import com.vtn.app.run.config.json.annotations.JsonElement;
import com.vtn.app.run.config.json.annotations.JsonObject;

/**
 * ContainerPathMapNorthBound - Bean Representaion for
 * ContainerPathMapNorthBound object from the JSON Response.
 *
 */

@JsonObject
public class ContainerPathMapNorthBound implements Serializable, Cloneable {

    /**
     * Serialization ID for thi object
     */
    private static final long serialVersionUID = 5120501567903808595L;

    /**
     * The unique index value for this bean.
     */
    @JsonElement(name = "index")
    private int index;

    /**
     * The Condition for pathmap.
     */
    @JsonElement(name = "condition")
    private String condition = null;

    /**
     * The policy name for pathmap.
     */
    @JsonElement(name = "policy")
    private int policy;

    /**
     * The minimum idle time in pathmap.
     */
    @JsonElement(name = "idleTimeout")
    private int idleTimeout;
    /**
     * Timeout in pathmap.
     */
    @JsonElement(name = "hardTimeout")
    private int hardTimeout;

    public ContainerPathMapNorthBound() {
    }

    public ContainerPathMapNorthBound(int index, String condition, int policy,
            int idleTimeout, int hardTimeout) {
        this.index = index;
        this.condition = condition;
        this.policy = policy;
        this.idleTimeout = idleTimeout;
        this.hardTimeout = hardTimeout;
    }

    /**
     * getIndex - function to get the index values.
     *
     * @return {@link int}
     */
    public int getIndex() {
        return index;
    }

    /**
     * setIndex - function to set the index values.
     *
     * @param {@link int}
     */
    public void setIndex(int index) {
        this.index = index;
    }

    /**
     * getCondition - function to get the Condition set.
     *
     * @return {@link String}
     */

    public String getCondition() {
        return condition;
    }

    /**
     * setCondition - function to set the Condition value
     *
     * @param condition
     *            .
     */
    public void setCondition(String condition) {
        this.condition = condition;
    }

    /**
     * getPolicy - function to get the policy set.
     *
     * @return {@link int}
     */
    public int getPolicy() {
        return policy;
    }

    /**
     * setPolicy - function to set policy.
     *
     * @param policy
     */
    public void setPolicy(int policy) {
        this.policy = policy;
    }

    /**
     * getIdleTimeout - function to get the idleTimeout set.
     *
     * @return {@link int}
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * setIdleTimeout - function to set idleTimeout.
     *
     * @param idleTimeout
     */
    public void setIdleTimeout(int idleTimeout) {
        this.idleTimeout = idleTimeout;
    }

    /**
     * getHardTimeout - function to get the hardTimeout set.
     *
     * @return {@link int}
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * setHardTimeout - function to set the hardTimeout.
     *
     * @param {@link int}
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
        return "ContainerPathMapNorthBound [index:" + index + ", condition:"
                + condition + ", policy:" + policy + ", idleTimeOut:"
                + idleTimeout + ",hardTimeOut:" + hardTimeout + "]";
    }
}
