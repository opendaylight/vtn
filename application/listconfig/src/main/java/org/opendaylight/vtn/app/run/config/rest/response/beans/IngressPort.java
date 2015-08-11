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
 * IngressPort - Bean Representaion for IngressPort object from the JSON
 * Response.
 *
 */
@JsonObject
public class IngressPort {

    /**
     * Node details for IngressPort.
     */
    @JsonObjectRef(name = "IngressNode")
    private NodeDetails nodeobj = new NodeDetails();

    /**
     * PortDetails details for IngressPort.
     */
    @JsonObjectRef(name = "IngressNode")
    private PortDetails portobj = new PortDetails();

    public IngressPort() {
    }

    /**
     * getNodeobj - function to get the nodeobj for this object.
     *
     * @return {@link NodeDetails}
     */
    public NodeDetails getNodeobj() {
        return nodeobj;
    }

    /**
     * setNodeobj - function to get the type value for this object.
     *
     * @param nodeobj
     */
    public void setNodeobj(NodeDetails nodeobj) {
        this.nodeobj = nodeobj;
    }

    /**
     * getPortobj - function to get the portDetials for this object.
     *
     * @return {@link PortDetails}
     */
    public PortDetails getPortobj() {
        return portobj;
    }

    /**
     * setPortobj - function to set the portDetials for this object.
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
    @Override
    public String toString() {
        return "IngressPort [nodeobj = " + nodeobj + ", portobj = " + portobj + "]";
    }
}
