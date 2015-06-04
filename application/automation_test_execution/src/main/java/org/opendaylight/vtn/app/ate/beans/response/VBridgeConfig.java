/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.ate.beans.response;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * VBridgeNorthBound - Bean Representaion for VBridgeNorthBound object from the JSON Response.
 *
 */
@JsonObject
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {vbrName}")
public class VBridgeConfig {
    /**
     * Nmae of the vbridge
     */
    @JsonElement(name = "name")
    private String name = null;

    /**
     * number of faults in vbridge.
     */
    @JsonElement(name = "faults")
    private int faults = 0;

    /**
     * state of the vbridge
     */
    @JsonElement(name = "state")
    private int state = 0;

    /**
     * Interval duration for vbridge.
     */
    @JsonElement(name = "ageInterval")
    private int ageInterval = 0;

    /**
     * description for vbridge.
     */
    @JsonElement(name = "description")
    private String description = null;

    /**
     * Default Constructor
     */
    public VBridgeConfig() {}

    /**
     * getDescription - function to get the description  for this object.
     *
     * @return {@link String}
     */
    public String getDescription() {
        return description;
    }

    /**
     * setDescription - function to set the description  for this object.
     *
     * @param description
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * getName - function to get the name for this object.
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * setName - function to set the name  for this object.
     *
     * @param name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * getFaults - function to get the faults  for this object.
     *
     * @return {@link int}
     */
    public int getFaults() {
        return faults;
    }

    /**
     * setFaults - function to set the faults for this object.
     *
     * @param faults
     */
    public void setFaults(int faults) {
        this.faults = faults;
    }

    /**
     * getState - function to get the state for this object.
     *
     * @return {@link String}
     */
    public int getState() {
        return state;
    }

    /**
     * setState - function to set the state  for this object.
     *
     * @param state
     */
    public void setState(int state) {
        this.state = state;
    }

    /**
     * getAgeInterval - function to get the ageInterval  for this object.
     *
     * @return {@link String}
     */
    public int getAgeInterval() {
        return ageInterval;
    }

    /**
     * setAgeInterval - function to set the ageInterval  for this object.
     *
     * @param name
     */
    public void setAgeInterval(int ageInterval) {
        this.ageInterval = ageInterval;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "vBirdge:[name:" + name + ",faults:" + faults
                        + ",state:" + state + ",ageInterval:" + ageInterval + "]";
    }
}
