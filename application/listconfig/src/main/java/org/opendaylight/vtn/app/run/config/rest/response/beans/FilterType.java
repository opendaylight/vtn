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
 * FilterType - Bean Representaion for FilterType object from the JSON Response.
 *
 */
@JsonObject
public class FilterType {

    /**
     * The redirect value for this object.
     */
    @JsonObjectRef(name = "redirect")
    private Redirect redirect = new Redirect();

    /**
     * The Pass value for this object.
     */
    @JsonObjectRef(name = "Pass")
    private Pass pass = new Pass();

    /**
     * The Drop value for this object.
     */
    @JsonObjectRef(name = "Drop")
    private Drop drop = new Drop();

    /**
     * getRedirect - function to get the redirect for this object.
     *
     * @return {@link String}
     */

    public Redirect getRedirect() {
        return redirect;
    }

    /**
     * setRedirect - function to set the redirect for this object.
     *
     * @param redirect
     */
    public void setRedirect(Redirect redirect) {
        this.redirect = redirect;
    }

    public FilterType() {
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "filterType[redirect:" + redirect + "]";
    }
}
