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
import org.opendaylight.vtn.app.run.config.rest.enumgroups.RestURL;

/**
 * VTNManagerVersion - Bean Representaion for array of VTNManagerVersion object
 * from the JSON Response.
 */
@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/version")
@JsonObject
public class VTNManagerVersion {
    /**
     * api for VTNManagerVersion.
     */
    @JsonElement(name = "api")
    private int api = 0;

    /**
     * Bundle reference to the object.
     */
    @JsonObjectRef(name = "bundle")
    private Bundle bundle = new Bundle();


    /**
     *  Default Constructor
     */
    public VTNManagerVersion() {
    }

    /**
     * getApi - function to get the api for this object.
     *
     * @return The API version.
     */
    public int getApi() {
        return api;
    }

    /**
     * setApi - function to set the api for this object.
     *
     * @param api
     */
    public void setApi(int api) {
        this.api = api;
    }

    /**
     * getBundle - function to get the bundle for this object.
     *
     * @return {@link Bundle }
     */
    public Bundle getBundle() {
        return bundle;
    }

    /**
     * setBundle - function to set the bundle for this object.
     *
     * @param bundle
     */
    public void setBundle(Bundle bundle) {
        this.bundle = bundle;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "[api:" + api + "]";
    }
}
