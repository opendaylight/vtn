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
 * VTenantNorthBoundList - Bean Representaion for array of VTenantNorthBound
 * object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns")
@JsonObject
public class VTenantNorthBoundList implements Serializable, Cloneable {

    /**
     * Serialized id for this class.
     */
    private static final long serialVersionUID = 5047440197949513794L;
    /**
     * Array of the vtn Objects.
     */
    @JsonArray(name = "vtn")
    private List<VTenantNorthBound> vtn = new ArrayList<VTenantNorthBound>();
    /**
     * Default Constructor.
     */
    public VTenantNorthBoundList() {
    }

    /**
     * Parameterized Constructor.
     * @param vtns
     */
    public VTenantNorthBoundList(List<VTenantNorthBound> vtns) {
        this.vtn = vtns;
    }

    /**
     * getVtns - function to get the vtn value for this object.
     *
     * @return A list of VTNs.
     */
    public List<VTenantNorthBound> getVtns() {
        return vtn;
    }

    /**
     * setVtns - function to set the vtn value for this object.
     *
     * @param vtns
     */
    public void setVtns(List<VTenantNorthBound> vtns) {
        this.vtn = vtns;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTNS [VTN:" + vtn + "]";
    }
}
