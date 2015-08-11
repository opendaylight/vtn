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
 * Redirect - Bean Representaion for Redirect object from the JSON Response.
 *
 */
@JsonObject
public class Redirect {

    /**
     * output value in redirect object.
     */
    @JsonElement(name = "output")
    private boolean output = false;

    /**
     * destination reference for this object.
     */
    @JsonObjectRef(name = "destination")
    private Destination destination = new Destination();

    /**
     * isOutput - get the status of Output.
     *
     * @return  A boolean value which indicates the direction.
     */
    public boolean isOutput() {
        return output;
    }

    /**
     * setOutput - set the status of Output.
     *
     * @param output
     */
    public void setOutput(boolean output) {
        this.output = output;
    }

    /**
     * getDestination - get the destination value of Output.
     *
     * @return {@link Destination}
     */
    public Destination getDestination() {
        return destination;
    }

    /**
     * setDestination - set the destination value of Output.
     *
     * @param destination
     */
    public void setDestination(Destination destination) {
        this.destination = destination;
    }

    public Redirect() {
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "redirect:[ouput:" + output + ",destionation:" + destination
                + "]";
    }
}
