/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;
/**
 *VlanNorthBoundList - bean which helps to map the response as java object.
 */
@JsonObject
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/vlanmaps")
public class VlanNorthBoundList implements Serializable, Cloneable {
    /**
     * Serialize ID constant fort this class.
     */
    private static final long serialVersionUID = 836750948728896411L;

    /**
     * Array of vlanmap objects.
     */
    @JsonArray(name = "vlanmap")
    private List<VlanNorthBound> vlanmap = new ArrayList<VlanNorthBound>();
    /**
     * Default Constructor.
     */
    public VlanNorthBoundList() {}


    /**
     * getVlanmap- function to get the list of VlanNorthBound  values for this object.
     *
     * @return List of {@link VlanNorthBound} objects
     */
    public List<VlanNorthBound> getVlanmap() {
        return vlanmap;
    }


    /**
     * setVbridge - function to set the list of vlanmapvalue for this object.
     *
     * @param vlanmap
     */
    public void setVlanmap(List<VlanNorthBound> vlanmap) {
        this.vlanmap = vlanmap;
    }
    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "vlanmap:" + vlanmap;
    }
}
