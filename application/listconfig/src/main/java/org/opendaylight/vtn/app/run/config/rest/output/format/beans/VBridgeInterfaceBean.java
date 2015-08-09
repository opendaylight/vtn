/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.output.format.beans;

import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgeIfFlowFilterNorthboundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VBridgePortMap;

/**
 * VBridgeInterfaceBean - Bean which helps to map the responses as java object
 *
 */
public class VBridgeInterfaceBean {

    /**
     * Name of the vBridge.
     */
    private String name = null;

    /**
     * State of the VBridge.
     */
    private int state = 0;

    /**
     * EntityState of the VBridge.
     */
    private int entityState = 0;

    /**
     * Description of the vbridge.
     */
    private String description = null;

    /**
     * ENable or disable vbridge.
     */
    private boolean enabled = false;

    /**
     * Map for Vbrigeports created.
     */
    private VBridgePortMap portmap = null;

    /**
     *  Flowfilter of the VBridge
     */
    private VBridgeIfFlowFilterNorthboundList flowfilters = null;

    /**
     * get the name of the VBridge.
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * get the ports of the VBridge.
     *
     * @return {@link VBridgePortMap}
     */
    public VBridgePortMap getPortmap() {
        return portmap;
    }

    /**
     * set the ports of the VBridge.
     *
     * @param portmap
     *        {@link VBridgePortMap}
     */

    public void setPortmap(VBridgePortMap portmap) {
        this.portmap = portmap;
    }

    /**
     * set the name of the VBridge.
     *
     * @param name
     *        {@link String}
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * get the state of the VBridge.
     *
     * @return The state of the vBridge.
     */
    public int getState() {
        return state;
    }

    /**
     * set the state of the VBridge.
     *
     * @param state  The state of the vBridge.
     */
    public void setState(int state) {
        this.state = state;
    }

    /**
     * get the Entitystate of the VBridge.
     *
     * @return The entity state.
     */
    public int getEntityState() {
        return entityState;
    }

    /**
     * set the Entitystate of the VBridge.
     *
     * @param entityState  The entity state.
     */
    public void setEntityState(int entityState) {
        this.entityState = entityState;
    }

    /**
     * get the Description of the VBridge.
     *
     * @return description {@link String}
     */
    public String getDescription() {
        return description;
    }

    /**
     * set the Description of the VBridge.
     *
     * @param description
     *        {@link String}
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * Check if VBridge isEnabled
     *
     * @return A boolean value which indicates the enabled state.
     */
    public boolean isEnabled() {
        return enabled;
    }

    /**
     * Enable or Disable VBridge
     *
     * @param enabled  A boolean value which indicates the enabled state.
     */
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    /**
     * @return the flowfilters
     */
    public VBridgeIfFlowFilterNorthboundList getFlowfilters() {
        return flowfilters;
    }
    /**
     * @param flowfilters the flowfilters to set
     */
    public void setFlowfilters(VBridgeIfFlowFilterNorthboundList flowfilters) {
        this.flowfilters = flowfilters;
    }
}
