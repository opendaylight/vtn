/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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
 * PathPoliciesIndex - Bean Representaion for PathPoliciesIndex object from the JSON Response.
 *
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/pathpolicies")
@JsonObject
public class PathPoliciesIndex implements Serializable, Cloneable {

    /**
     * Serialization ID for this object.
     */
    private static final long serialVersionUID = 6166755047100988276L;

    /**
     * Array of Indexes.
     */
    @JsonArray(name = "integer")
    private List<Index> integers = new ArrayList<Index>();

    public PathPoliciesIndex() {}

    public PathPoliciesIndex(List<Index> integers) {
        this.integers = integers;
    }

    /**
     * getIntegers - Method returns the index valus for this object.
     *
     * @return A list of path policy indices.
     */
    public List<Index> getIntegers() {
        return integers;
    }

    /**
     * setIntegers - Method sets the index valus for this object.
     *
     * @param integers
     */
    public void setIntegers(List<Index> integers) {
        this.integers = integers;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Integers [integer:" + integers + "]";
    }
}
