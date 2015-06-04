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
 *VbrInterface- bean which helps to map the response as java object.
 */
@JsonObject
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {vbrName}/interfaces/ {vbrIf}")
public class VbrInterface{
    /**
     * Name of the interface
     */
    @JsonElement(name = "name")
    private String name = null;

    /**
     * state of the interface.
     */
    @JsonElement(name = "state")
    private int state = 0;

    /**
     * entity state of the interface.
     */
    @JsonElement(name = "entityState")
    private int entityState = 0;

    /**
     * description if the interface.
     */
    @JsonElement(name = "description")
    private String description = null;

    /**
     * enabled or disable the interface.
     */
    @JsonElement(name = "enabled")
    private boolean enabled = false;
    /**
     * Default Constructor.
     */
    public VbrInterface() {
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
     * setName - function to set the name for this object.
     *
     * @param name
     */
    public void setName(String name) {
        this.name = name;
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
     * getEntityState - function to get the entity state for this object.
     *
     * @return {@link int}
     */
    public int getEntityState() {
        return entityState;
    }

    /**
     * setEntityState - function to set the entity state for this object.
     *
     * @param entityState
     */
    public void setEntityState(int entityState) {
        this.entityState = entityState;
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
     * getEnabled - function to get the enabled state for this object.
     *
     * @return {@link boolean}
     */
    public boolean getEnabled() {
        return enabled;
    }

    /**
     * setEnabled - function to set the enabled state for this object.
     *
     * @param enabled
     */
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "[name:" + name + ",state:" + state + ",entityState:"
                + entityState + ",description:" + description + ",enabled:"
                + enabled + "]";
    }
}
