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
 * VTerminalInterfaceNorthboundList - Bean Representaion for VTerminal Interface object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vterminals/ {terminalName}/interfaces")
@JsonObject
public class VTerminalInterfaceNorthboundList implements Serializable, Cloneable {

    /**
     * Serialization id for this class
     */
    private static final long serialVersionUID = 856456312241473164L;

    /**
     * Arry of interference references.
     */
    @JsonArray(name = "interface")
    List<VTerminalInterfaceNorthbound> interfaces = new ArrayList<VTerminalInterfaceNorthbound>();

    /**
     * getInterfaces - function to get the interfaces values for this object.
     * @return List of {@link VTerminalInterfaceNorthbound} objects
     */
    public List<VTerminalInterfaceNorthbound> getInterfaces() {
        return interfaces;
    }

    /**
     * setInterfaces - function to set the interfaces values for this object.
     * @param interfaces
     */
    public void setInterfaces(List<VTerminalInterfaceNorthbound> interfaces) {
        this.interfaces = interfaces;
    }


    /**
     * Default Constructor.
     */
    public VTerminalInterfaceNorthboundList() {}

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "" + interfaces;
    }
}

