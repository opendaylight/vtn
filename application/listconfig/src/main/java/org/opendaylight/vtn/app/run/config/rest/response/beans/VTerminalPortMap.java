/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;
/**
 * VTerminalPortMap - bean which helps to map the response as java object.
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/vtns/ {tenantName}/vterminals/ {terminalName}/interfaces/ {ifName}/portmap")
@JsonObject
public class VTerminalPortMap implements Cloneable, Serializable {

    /**
     * Seriallized id for this object.
     */
    private static final long serialVersionUID = 4443162428925065719L;

    /**
     * vlan for vTerminal.
     */
    private String vlan = null;

    /**
     * NodeDetails reference for this object.
     */
    @JsonObjectRef(name = "node")
    private NodeDetails node = new NodeDetails();

    /**
     * PortDetails reference for this object.
     */
    @JsonObjectRef(name = "port")
    private PortDetails port = new PortDetails();

    /**
     * MappedDetails reference for this object.
     */
    @JsonObjectRef(name = "mapped")
    private MappedDetails mapped = new MappedDetails();

    /**
     * getVlan - function to get the vlan value for this object.
     *
     * @return {@link String}
     */
    public String getVlan() {
        return vlan;
    }

    /**
     * setVlan - function to get the vlan value for this object.
     *
     * @param vlan
     */
    public void setVlan(String vlan) {
        this.vlan = vlan;
    }

    /**
     * getNode - function to get the node value for this object.
     *
     * @return {@link NodeDetails}
     */
    public NodeDetails getNode() {
        return node;
    }

    /**
     * setVlan - function to get the node value for this object.
     *
     * @param node
     */
    public void setNode(NodeDetails node) {
        this.node = node;
    }

    /**
     * getVlan - function to get the port value for this object.
     *
     * @return {@link String}
     */
    public PortDetails getPort() {
        return port;
    }

    /**
     * setPort - function to set the port value for this object.
     *
     * @param port
     */
    public void setPort(PortDetails port) {
        this.port = port;
    }

    /**
     * getMapped - function to get the map dtails for this object.
     *
     * @return {@link MappedDetails}
     */
    public MappedDetails getMapped() {
        return mapped;
    }

    /**
     * setMapped - function to set the map details for this object.
     *
     * @param mapped
     */
    public void setMapped(MappedDetails mapped) {
        this.mapped = mapped;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "vlan:" + vlan + "[node:" + node + ",port:" + port + ",mapped:"
              + mapped + "]";
    }
}
