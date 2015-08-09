/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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
 * VTerminalNorthBoundList - Bean Representaion for VTerminalNorthBoundList object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vterminals")
@JsonObject
public class VTerminalNorthBoundList implements Serializable, Cloneable {

    /**
     * Serialization id for this class
     */
    private static final long serialVersionUID = 612456312241473164L;

    /**
     * Array of vterminals objects.
     */
    @JsonArray(name = "vterminal")
    private List<VTerminalNorthBound> vterminal = new ArrayList<VTerminalNorthBound>();
    /**
     * Default Constructor.
     */
    public VTerminalNorthBoundList() {}

    /**
     * getVterm - function to get the vterminal lists for this object.
     *
     * @return List of {@link VTerminalNorthBound} objects
     */
    public List<VTerminalNorthBound> getVterm() {
        return vterminal;
    }

    /**
     * setVterm - function to set the vterminal lists for this object.
     *
     * @param vterm
     */
    public void setVterm(List<VTerminalNorthBound> vterm) {
        this.vterminal = vterm;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "VTERMLIST [VTERMINAL :" + vterminal + "]";
    }
}
