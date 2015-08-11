/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.output.format.beans;

import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalIfFlowFilterNorthboundList;
import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalPortMap;

/**
 * VTerminalInterfaceBean - Bean which helps to map the responses as java object
 *
 */
public class VTerminalInterfaceBean {

    /**
     * Name of the VTerminal.
     */
    private String name = null;

    /**
     * State of the VTerminal.
     */
    private int state = 0;

    /**
     * EntityState of the VTerminal.
     */
    private int entityState = 0;

    /**
     * Description of the VTerminal.
     */
    private String description = null;

    /**
     * ENable or disable VTerminal.
     */
    private boolean enabled = false;

    /**
     * Map for VTerminal ports created.
     */
    private VTerminalPortMap portmap = null;

    /**
     *  Flowfilter of the VTerminal
     */
    private VTerminalIfFlowFilterNorthboundList flowfilters = null;

    /**
     * get the name of the VTermainal.
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * set the name of the VTermainal.
     *
     * @param name
     *        {@link String}
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * get the state of the VTermainal.
     *
     * @return The state of the vTerminal.
     */
    public int getState() {
        return state;
    }

    /**
     * set the state of the VTermainal.
     *
     * @param state  The state of the vTerminal.
     */
    public void setState(int state) {
        this.state = state;
    }

    /**
     * get the Entitystate of the VTermainal.
     *
     * @return The entity state.
     */
    public int getEntityState() {
        return entityState;
    }

    /**
     * set the Entitystate of the VTermainal.
     *
     * @param entityState  The entity state.
     */
    public void setEntityState(int entityState) {
        this.entityState = entityState;
    }

    /**
     * get the Description of the VTermainal.
     *
     * @return description {@link String}
     */
    public String getDescription() {
        return description;
    }

    /**
     * set the Description of the VTermainal.
     *
     * @param description
     *        {@link String}
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * Check if VTermainal isEnabled
     *
     * @return A boolean value which indicates the enabled state.
     */
    public boolean isEnabled() {
        return enabled;
    }

    /**
     * Enable or Disable VTermainal
     *
     * @param enabled  A boolean value which indicates the enabled state.
     */
    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    /**
     * get the ports of the VTerminal.
     *
     * @return {@link VTerminalPortMap}
     */
    public VTerminalPortMap getPortmap() {
        return portmap;
    }

    /**
     * set the ports of the VTerminal.
     *
     * @param portmap
     *        {@link VTerminalPortMap}
     */

    public void setPortmap(VTerminalPortMap portmap) {
        this.portmap = portmap;
    }

    /**
     * get the VTermainal flowfilter
     *
     * @return {@link VTerminalIfFlowFilterNorthboundList}
     */
    public VTerminalIfFlowFilterNorthboundList getVtflowfilter() {
        return flowfilters;
    }

    /**
     * set the flowfilter of the VTermainal.
     *
     * @param flowfilters
     *        {@link VTerminalIfFlowFilterNorthboundList}
     */
    public void setVtFlowfilter(VTerminalIfFlowFilterNorthboundList flowfilters) {
        this.flowfilters = flowfilters;
    }

}
