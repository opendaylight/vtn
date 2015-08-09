/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.io.Serializable;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;

/**
 * Index - Bean Representaion for Index object from the JSON Response.
 *
 */
@JsonObject
public class Index implements Serializable, Cloneable {

    /**
     * Serializied value for this class.
     */
    private static final long serialVersionUID = 6929656865685776912L;

    /**
     * Index values.
     */
    @JsonElement(name = "value")
    private int value;

    public Index() {
    }

    public Index(int value) {
        this.value = value;
    }

    /**
     * getValue - function to get the values for this object.
     *
     * @return The index value.
     */
    public int getValue() {
        return value;
    }

    /**
     * setValue - function to set the values for this object.
     *
     * @param value
     */
    public void setValue(int value) {
        this.value = value;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Value [value:" + value + "]";
    }
}
