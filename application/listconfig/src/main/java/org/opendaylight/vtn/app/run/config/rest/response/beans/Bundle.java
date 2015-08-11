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

/**
 * Bundle - Bean Representaion for Bundle object from the JSON Response.
 *
 */

@JsonObject
public class Bundle {

    /**
     * Attribute major in Bundle Object.
     */
    @JsonElement(name = "major")
    private int major =  0;

    /**
     * Attribute minor in Bundle Object.
     */
    @JsonElement(name = "minor")
    private int minor =  0;

    /**
     * Attribute micro in Bundle Object.
     */
    @JsonElement(name = "micro")
    private int micro =  0;

    /**
     * Attribute qualifier in Bundle Object.
     */
    @JsonElement(name = "qualifier")
    private String qualifier =  null;

    public Bundle() {}

    /**
     * getMajor - function to get the major values.
     * @return The major version of the bundle.
     */
    public int getMajor() {
        return major;
    }

    /**
     * setMajor - function to set a value for major.
     * @param major The major version of the bundle.
     */
    public void setMajor(int major) {
        this.major = major;
    }

    /**
     * getMinor - function to get the value for minor.
     * @return  The minor version of the bundle.
     */
    public int getMinor() {
        return minor;
    }

    /**
     * setMinor - function to set a value for minor.
     * @param minor  The minor version of the bundle.
     */
    public void setMinor(int minor) {
        this.minor = minor;
    }

    /**
     * getMicro - function to get the value for micro.
     * @return The micro version of the bundle.
     */
    public int getMicro() {
        return micro;
    }

    /**
     * setMicro - function to set a value for micro.
     * @param micro The micro version of the bundle.
     */
    public void setMicro(int micro) {
        this.micro = micro;
    }

    /**
     * getQualifier - function to get the qualifier.
     * @return {@link String}
     */
    public String getQualifier() {
        return qualifier;
    }

    /**
     * setQualifier - function to set the qualifier.
     * @param qualifier {@link String}
     */
    public void setQualifier(String qualifier) {
        this.qualifier = qualifier;
    }

    /**
     * String representation of the Bundle object.
     *
     */
    @Override
    public String toString() {
        return "[major:" + major + ",minor:" + minor + ",micro:" + micro
                + ",qualifier:" + qualifier + "]";
    }
}
