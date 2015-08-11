/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * MacEntry - Bean Representaion for MacEntry object from the JSON Response.
 *
 */
@JsonObject
public class MacEntry {

    /**
     * address attribute for MacEntry
     */
    @JsonElement(name = "address")
    private String address = null;

    /**
     * vlan attribute for MacEntry
     */
    @JsonElement(name = "vlan")
    private int vlan = 0;

    /**
     * node attribute for MacEntry
     */
    @JsonObjectRef(name = "node")
    private NodeDetails node = new NodeDetails();

    /**
     * port attribute for MacEntry
     */
    @JsonObjectRef(name = "port")
    private PortDetails port = new PortDetails();

    /**
     * address attribute for MacEntry
     */
    @JsonObjectRef(name = "inetAddresses")
    private InetAddressDetails inet = new InetAddressDetails();

    public MacEntry() {}

    /**
     * getAddress - function to get the address value for this object.
     *
     * @return {@link String}
     */
    public String getAddress() {
        return address;
    }

    /**
     * setAddress - function to set the address value for this object.
     *
     * @param address
     */
    public void setAddress(String address) {
        this.address = address;
    }

    /**
     * getVlan - function to set the vlan value for this object.
     *
     * @return {@link String}
     */
    public int getVlan() {
        return vlan;
    }

    /**
     * setVlan - function to set the vlan value for this object.
     *
     * @param vlan
     */
    public void setVlan(int vlan) {
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
     * setNode - function to set the node value for this object.
     *
     * @param node
     */
    public void setNode(NodeDetails node) {
        this.node = node;
    }

    /**
     * getPort - function to get the port value for this object.
     *
     *  @return {@link PortDetails}
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
     * getInet - function to get the inet value for this object.
     *
     *  @return {@link InetAddressDetails}
     */
    public InetAddressDetails getInet() {
        return inet;
    }

    /**
     * setInet - function to set the inet value for this object.
     *
     *  @param inet
     */
    public void setInet(InetAddressDetails inet) {
        this.inet = inet;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "macentry[address:" + address + ",vlan:" + vlan + ",node:" + node
                        + ",port:" + port + ",address:" + address + "]";
    }
}
