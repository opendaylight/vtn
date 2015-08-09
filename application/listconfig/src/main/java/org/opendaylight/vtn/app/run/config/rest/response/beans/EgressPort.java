/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * EgressPort - Bean Representaion for EgressPort object from the JSON Response.
 *
 */

@JsonObject
public class EgressPort {

    /**
     * The reference of NodeDetails for this object.
     */

    @JsonObjectRef(name = "node")
    private NodeDetails nodeobj = new NodeDetails();

    /**
     * The reference of PortDetails for this object.
     */
    @JsonObjectRef(name = "port")
    private PortDetails portobj = new PortDetails();

    public EgressPort() {
    }

    /**
     * nodeDetails - function to get the NodeDetails for this object.
     *
     * @return {@link NodeDetails}
     */
    public NodeDetails nodeDetails() {
        return nodeobj;
    }

    /**
     * setNodeobj - function to set the Node details for this object.
     *
     * @param nodeobj
     */
    public void setNodeobj(NodeDetails nodeobj) {
        this.nodeobj = nodeobj;
    }

    /**
     * getPortobj - function to get the Portobj for this object.
     *
     * @return {@link PortDetails}
     */
    public PortDetails getPortobj() {
        return portobj;
    }

    /**
     * setPortobj - function to set the Portobj for this object.
     *
     * @param portobj
     */
    public void setPortobj(PortDetails portobj) {
        this.portobj = portobj;
    }

    /**
     * String representation of the object.
     *
     */
    public String toString() {
        return "VTN RUN CONFIG [node:" + nodeobj + ",port:" + portobj + "]";
    }
}
