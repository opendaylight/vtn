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
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * FlowFilter - Bean Representaion for FlowFilter object from the JSON Response.
 *
 */
@JsonObject
public class FlowFilter implements Cloneable, Serializable {

    /**
     * Serialized unique constant for this class.
     */
    private static final long serialVersionUID = -8773782024003501769L;

    /**
     * uniqued id's for flow filters
     */
    @JsonElement(name = "index")
    private int index = 0;

    /**
     * condition for flow filters
     */
    @JsonElement(name = "condition")
    private String condition = "";

    /**
     * filterType associated for flow filters
     */
    @JsonObjectRef(name = "filterType")
    private FilterType filterType = new FilterType();

    /**
     * actions for flow filters
     */
    @JsonArray(name = "actions")
    private List<Actions> actions = new ArrayList<Actions>();

    public FlowFilter() {
    }

    /**
     * getIndex - function to get the index values for this object.
     *
     * @return The index for the flow filter.
     */
    public int getIndex() {
        return index;
    }

    /**
     * setIndex - function to set the index values for this object.
     *
     * @param index
     */
    public void setIndex(int index) {
        this.index = index;
    }

    /**
     * getCondition - function to get the condition values for this object.
     *
     * @return {@link String}
     */
    public String getCondition() {
        return condition;
    }

    /**
     * setCondition - function to set the condition values for this object.
     *
     * @param condition
     */
    public void setCondition(String condition) {
        this.condition = condition;
    }

    /**
     * getFilterType - function to get the filterType values for this object.
     *
     * @return {@link FilterType}
     */
    public FilterType getFilterType() {
        return filterType;
    }

    /**
     * setFilterType - function to set the filterType values for this object.
     *
     * @param filterType
     */
    public void setFilterType(FilterType filterType) {
        this.filterType = filterType;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "flowFilter:[index:" + index + ",condition:" + condition
                + ",filterType:" + filterType + "]";
    }
}
