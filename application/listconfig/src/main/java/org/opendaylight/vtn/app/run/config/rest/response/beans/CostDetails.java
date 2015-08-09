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
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * CostDetails - Bean Representaion for CostDetails object from the JSON
 * Response.
 *
 */
@JsonObject
public class CostDetails implements Serializable, Cloneable {

    /**
     * Serialization ID for thi object
     */
    private static final long serialVersionUID = 6390062057619231068L;

    /**
     * LocationDetails object refrence.
     */
    @JsonObjectRef(name = "location")
    private LocationDetails location = new LocationDetails();

    /**
     * cost object refrence.
     */
    @JsonElement(name = "cost")
    private int cost;

    public CostDetails() {
    }
    /**
     * CostDetails - Constructor with arguments.
     * @param location - LocationDetails object refrence.
     * @param cost - cost object refrence.
     */
    public CostDetails(LocationDetails location, int cost) {
        this.location = location;
        this.cost = cost;
    }

    /**
     * getLocation - function to get the location values.
     *
     * @return {@link LocationDetails}
     */
    public LocationDetails getLocation() {
        return location;
    }

    /**
     * setLocation - function to set the location values.
     *
     * @param location
     */
    public void setLocation(LocationDetails location) {
        this.location = location;
    }

    /**
     * getCost - function to get the cost values.
     *
     * @return The link cost value.
     */
    public int getCost() {
        return cost;
    }

    /**
     * getCost - function to get the cost values.
     *
     * @param cost
     */
    public void setCost(int cost) {
        this.cost = cost;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "[location:" + location + "cost:" + cost + "]";
    }
}
