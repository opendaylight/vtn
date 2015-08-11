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
 *VBridgeInterfaceList - bean which helps to map the response as java object.
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vbridges/ {bridgeName}/interfaces")
@JsonObject
public class VBridgeInterfaceList implements Serializable, Cloneable {

    /**
     * Searialized id for this class.
     */
    private static final long serialVersionUID = -7308643277891453481L;

    /**
     * Arry of interference references.
     */
    @JsonArray(name = "interface")
    List<VBridgeInterface> interfaces = new ArrayList<VBridgeInterface>();

    /**
     * getInterfaces - function to get the interfaces values for this object.
     *
     * @return List of {@link VBridgeInterface}
     */
    public List<VBridgeInterface> getInterfaces() {
        return interfaces;
    }

    /**
     * setInterfaces - function to set the interfaces values for this object.
     *      * @param interfaces
     */
    public void setInterfaces(List<VBridgeInterface> interfaces) {
        this.interfaces = interfaces;
    }


    /**
     * Default Constructor.
     */
    public VBridgeInterfaceList() {}

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "" + interfaces;
    }
}
