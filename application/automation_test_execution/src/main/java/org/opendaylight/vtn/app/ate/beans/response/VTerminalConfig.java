/**
 * Copyright (c) 2015 NEC Corporation
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
 * VTerminalNorthBound - Bean Representaion for VTerminal object from the JSON Response.
 *
 */
@JsonObject
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vterminals/ {vtermName}")
public class VTerminalConfig {

    /**
     * name for the terminal.
     */
    @JsonElement(name = "name")
    private String name = null;

    /**
     * description for the terminal.
     */
    @JsonElement(name = "description")
    private String description = null;

    /**
     * faults for the terminal.
     */
    @JsonElement(name = "faults")
    private int faults = 0;

    /**
     * state for the terminal.
     */
    @JsonElement(name = "state")
    private int state = 0;
    /**
     * Default Constructor.
     */
    public VTerminalConfig() {
    }
    /**
     * Parameterized Constructor.
     * @param name,description,faults
     */
    public VTerminalConfig(String name, String description, int faults,
        int state) {
        this.name = name;
        this.description = description;
        this.faults = faults;
        this.state = state;
    }

    /**
     * getDescription - function to get the description for this object.
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * setName - function to set the name for this object.
     *
     * @param name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * getDescription - function to get the description for this object.
     *
     * @return {@link String}
     */
    public String getDescription() {
        return description;
    }

    /**
     * setDescription - function to set the description for this object.
     *
     * @param description
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * getFaults - function to get the faults for this object.
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
     * @return {@link int}
     */
    public int getState() {
        return state;
    }

    /**
     * setState - function to set the state for this object.
     *
     * @param state
     */
    public void setState(int state) {
        this.state = state;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTERMINAL [name:" + name + ",description:" + description
              + ",faults:" + faults + ",state:" + state + "]";
    }
}
