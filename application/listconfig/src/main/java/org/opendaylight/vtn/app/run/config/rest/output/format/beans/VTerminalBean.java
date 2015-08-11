/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.output.format.beans;

import org.opendaylight.vtn.app.run.config.rest.response.beans.VTerminalIfFlowFilterNorthboundList;
import java.util.List;

/**
 * VTerminalBean - Bean which helps to map the responses as java object
 *
 */
public class VTerminalBean {

    /**
     * name for the VTermainal.
     */
    private String name;

    /**
     * description for the VTermainal.
     */
    private String description;

    /**
     * faults for the VTermainal.
     */
    private int faults = 0;

    /**
     * state for the VTermainal.
     */
    private int state = 0;

    /**
     * VTerminal interface attached to the VTerminal.
     */
    private List<VTerminalInterfaceBean> vInterface = null;

    /**
     * VTerminalIfFlowFilterNorthboundList attached to the VTerminal.
     */
    private VTerminalIfFlowFilterNorthboundList vtFlowfilters = null;

    /**
     * Default Constructor.
     */
    public VTerminalBean() {
    }

    /**
     * get the name of the VTermainal.
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * Set the VTermainal Name.
     * @param name {@link String}
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Get the fault details of VTerminal.
     * @return {@link Integer}
     */
    public int getFaults() {
        return faults;
    }

    /**
     * set the Fault info of the VTerminal.
     * @param faults
     */
    public void setFaults(int faults) {
        this.faults = faults;
    }

    /**
     * get the state details of the VTerminal.
     * @return {@link Integer}
     */
    public int getState() {
        return state;
    }

    /**
     * set the state of the VTermainal.
     * @param state
     */
    public void setState(int state) {
        this.state = state;
    }

    /**
     * getDescription about the VTermainal.
     * @return {@link String}
     */
    public String getDescription() {
        return description;
    }

    /**
     * setDescription about the VTermainal.
     * @param description
     */
    public void setDescription(String description) {
        this.description = description;
    }

    /**
     * getVInterface function to get the list of VTerminalInterfaceBean value for this object.
     * @return List of {@link VTerminalInterfaceBean} objects
     */
    public List<VTerminalInterfaceBean> getVInterface() {
        return vInterface;
    }

    /**
     * setVInterface function to set VTerminalInterfaceBean value for this object.
     * @param vInterface
     */
    public void setVInterface(List<VTerminalInterfaceBean> vInterface) {
        this.vInterface = vInterface;
    }

    /**
     * getVtFlowfilter function to get VTerminalIfFlowFilterNorthboundList value for this object.
     * @return {@link VTerminalIfFlowFilterNorthboundList}
     */
    public VTerminalIfFlowFilterNorthboundList getVflowfilter() {
        return vtFlowfilters;
    }

    /**
     * setVflowfilter function to set VTerminalIfFlowFilterNorthboundList value for this object.
     * @param vtFlowfilters
     */
    public void setVflowfilter(VTerminalIfFlowFilterNorthboundList vtFlowfilters) {
        this.vtFlowfilters = vtFlowfilters;
    }
}

