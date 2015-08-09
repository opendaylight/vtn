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

/**
 * VlanPcp - Bean Representaion for VlanPcp object from the JSON Response.
 *
 */
@JsonObject
public class VlanPcp {

    /**
     * priority for VlanPcp
     */
    @JsonElement(name = "priority")
    private int priority = 0;
    /**
     * Default Constructor
     */
    public VlanPcp() {}

    /**
     * getPriority - function to get the priority value for this object.
     *
     * @return The VLAN priority.
     */
    public int getPriority() {
        return priority;
    }

    /**
     * setPriority - function to set the priority value for this object.
     *
     * @param priority
     */
    public void setPriority(int priority) {
        this.priority = priority;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "priority:" + priority;
    }
}
