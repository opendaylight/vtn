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
import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;

/**
 * Flowconditions - Bean Representaion for Flowconditions object from the JSON
 * Response.
 *
 */
@JsonObject
public class Flowconditions implements Serializable {

    /**
     * The Serializable Id for this object.
     */
    private static final long serialVersionUID = 5323714422160033795L;

    /**
     * The name value for this object.
     */
    @JsonElement(name = "name")
    private String name = null;

    /**
     * The match values for this object.
     */
    @JsonArray(name = "match")
    List<Match> match = new ArrayList<Match>();

    public Flowconditions() {
    }
    /**
     * Flowconditions - Constructor with arguments.
     * @param name - name of the Flow condition.
     * @param matchList - match values for this object.
     */
    public Flowconditions(String name, List<Match> matchList) {
        this.name = name;
        this.match = matchList;
    }

    /**
     * getName - function to get the name for this object.
     *
     * @return {@link String}
     */
    public String getName() {
        return name;
    }

    /**
     * setName - function to set the name for this object.
     *
     * @param name
     */
    public void setName(String name) {
        this.name = name;
    }

    /**
     * getMatchobj - function to get the Match objects for this object.
     *
     * @return List of Match objects
     */
    public List<Match> getMatchobj() {
        return match;
    }

    /**
     * setMatchobj - function to set the Match objects for this object.
     *
     * @param matchList
     */
    public void setMatchobj(List<Match> matchList) {
        this.match = matchList;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Flowconditions [name = " + name + ", match = " + match + "]";
    }
}
