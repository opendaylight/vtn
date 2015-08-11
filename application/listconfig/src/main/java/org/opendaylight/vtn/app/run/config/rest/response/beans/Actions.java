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

    /**
     * Atribute dscp in Action Object.
     */
    @JsonObjectRef(name = "dscp")
    private Dscp dscp = new Dscp();

    /**
     * Atribute Tpsrc in Action Object.
     */
    @JsonObjectRef(name = "Tpsrc")
    private Tpsrc tpsrc = new Tpsrc();

    /**
     * Atribute Tpdst in Action Object.
     */
    @JsonObjectRef(name = "Tpdst")
    private Tpdst tpdst = new Tpdst();

    /**
     * Atribute Dldst in Action Object.
     */
    @JsonObjectRef(name = "Dldst")
    private Dldst dldst = new Dldst();

    /**
     * Atribute Inet4Src in Action Object.
     */
    @JsonObjectRef(name = "inet4Src")
    private Inet4Src inet4Address = new Inet4Src();

    /**
     * Atribute Inet4Dst in Action Object.
     */
    @JsonObjectRef(name = "inet4Address")
    private Inet4Dst inet4dst = new Inet4Dst();

    /**
     * Atribute IcmpType in Action Object.
     */
    @JsonObjectRef(name = "icmpType")
    private IcmpType icmpType = new IcmpType();

    /**
     * Atribute Inet4Dst in Action Object.
     */
    @JsonObjectRef(name = "icmpCode")
    private IcmpCode icmpCode = new IcmpCode();


}
