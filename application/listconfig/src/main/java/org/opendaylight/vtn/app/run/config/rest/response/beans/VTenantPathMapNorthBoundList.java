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
 * VTenantPathMapNorthBoundList - Bean Representaion for array of  VTenantPathMapNorthBound object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/pathmaps ")
@JsonObject
public class VTenantPathMapNorthBoundList implements Serializable, Cloneable {

    /**
     * serialization id for this class.
     */
    private static final long serialVersionUID = 5740034026547172179L;
   /**
     * Array of the pathmap Objects.
     */
    @JsonArray(name = "pathmap")
    private List<VTenantPathMapNorthBound> pathmap =  new ArrayList<VTenantPathMapNorthBound>();

    /**
     * getPathmap - function to get the list of pathmap value for this object.
     *
     * @return List of {@link VTenantPathMapNorthBound} objects
     */
    public List<VTenantPathMapNorthBound> getPathmap() {
        return pathmap;
    }

    /**
     * setPathmap - function to set  the list of pathmap value for this object.
     *
     * @param pathmap
     */
    public void setPathmap(List<VTenantPathMapNorthBound> pathmap) {
        this.pathmap = pathmap;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "pathmapoutput[pathmap:" + pathmap + "]";
    }
}
