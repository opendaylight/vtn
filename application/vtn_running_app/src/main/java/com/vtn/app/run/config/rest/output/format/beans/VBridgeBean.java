/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package com.vtn.app.run.config.rest.output.format.beans;

import java.util.List;

import com.vtn.app.run.config.rest.response.beans.MacEntryList;
import com.vtn.app.run.config.rest.response.beans.MacMapNorthBound;
import com.vtn.app.run.config.rest.response.beans.VlanNorthBoundList;

/**
 *VBridgeBean  - bean which helps to map the response as java object.
 */
public class VBridgeBean {

    /**
     * Name of the vBridge.
     */
    private String name = null;

    /**
     * Faults value of the VBridge.
     */
    private int faults = 0;

    /**
     * State of the VBridge.
     */
    private int state = 0;

    /**
     * AgeInterval of the VBridge.
     */
    private int ageInterval = 0;

    /**
     * Description about the VBridge.
     */
    private String description = null;

    /**
     * MacEntryList attached to the VBridge.
     */
    private MacEntryList macentry = null;

    /**
     * Vlan List attached to the VBridge.
     */
    private VlanNorthBoundList brvlan = null;

    /**
     * VBridge interface attached to the Vbridge.
     */
    private List<VBridgeInterfaceBean> vbrInterface = null;

    /**
     * Mac Address details attached to the VBridge.
     */
    private MacMapNorthBound macMap = null;

    /**
     * to get the MAcAddress attached to the VBridge.
     * @return {@link MacMapNorthBound}
     */
    public MacMapNorthBound getMacMap() {
        return macMap;
    }

    /**
     * To set the MAc Address details of the VBridge.
     * @param macMap
     * @return
     */
    public void setMacMap(MacMapNorthBound macMap) {
        this.macMap = macMap;
    }

    /**
     * Default constructor.
     */
    public VBridgeBean() {}

    /**
     * get the name of the VBridge.
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * Set the Vbridge Name.
     * @param name {@link String}
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * Get the fault details of VBridge.
     * @return {@link Integer}
     */
    public int getFaults() {
        return faults;
    }

    /**
     * set the Fault info of the VBridge.
     * @param faults
     */
    public void setFaults(int faults) {
        this.faults = faults;
    }

    /**
     * get the state details of the VBridge.
     * @return {@link Integer}
     */
    public int getState() {
        return state;
    }

    /**
     * set the state of the VBridge.
     * @param state
     */
    public void setState(int state) {
        this.state = state;
    }

    /**
     * get the Age interval of the VBridge.
     * @return {@link Integer}
     */
    public int getAgeInterval() {
        return ageInterval;
    }

    /**
     * Set the age interval of the VBridge.
     * @param ageInterval
     */
    public void setAgeInterval(int ageInterval) {
        this.ageInterval = ageInterval;
    }

    /**
     * getDescription about the VBridge.
     * @return {@link String}
     */
    public String getDescription() {
        return description;
    }

    /**
     * setDescription about the VBridge.
     * @param description
     */
    public void setDescription(String description) {
        this.description = description;
    }


    public VlanNorthBoundList getBrvlan() {
        return brvlan;
    }


    public void setBrvlan(VlanNorthBoundList brvlan) {
        this.brvlan = brvlan;
    }


    public MacEntryList getMacentry() {
        return macentry;
    }


    public void setMacentry(MacEntryList macentry) {
        this.macentry = macentry;
    }


    public List<VBridgeInterfaceBean> getVbrInterface() {
        return vbrInterface;
    }


    public void setVbrInterface(List<VBridgeInterfaceBean> vbrInterface) {
        this.vbrInterface = vbrInterface;
    }


    /*public VBridgePortMap getPortmap() {
        return portmap;
    }


    public void setPortmap(VBridgePortMap portmap) {
        this.portmap = portmap;
    }*/

    @Override
    public String toString() {
        return "macentry:" + macentry + "vbrInterface:" + vbrInterface;
    }
}
