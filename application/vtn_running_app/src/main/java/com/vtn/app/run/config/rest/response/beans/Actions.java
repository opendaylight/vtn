/**
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package com.vtn.app.run.config.rest.response.beans;

import com.vtn.app.run.config.json.annotations.JsonObject;

import com.vtn.app.run.config.json.annotations.JsonObjectRef;
/**
 * Actions - Bean Representaion for Actions object from the JSON Response.
 *
 */
@JsonObject
public class Actions {

    /**
     * Atribute DlSrc in Action Object.
     */
    @JsonObjectRef(name = "dlsrc")
    private DlSrc dlsrc = new DlSrc();

    /**
     * Atribute vlanpcp in Action Object.
     */
    @JsonObjectRef(name = "vlanpcp")
    private VlanPcp vlanpcp = new VlanPcp();
}
