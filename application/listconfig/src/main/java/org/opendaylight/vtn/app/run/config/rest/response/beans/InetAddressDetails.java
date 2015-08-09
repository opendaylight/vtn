/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonArray;

/**
 * InetAddressDetails - Bean Representaion for InetAddressDetails object from
 * the JSON Response.
 *
 */
public class InetAddressDetails {

    @JsonArray(name = "inetAddress")
    private List<InetAddressDetailsList> inetAddress = new ArrayList<InetAddressDetailsList>();

    /**
     * getInetAddress - function to get the inetAddress values for this object.
     *
     * @return A list of {@link InetAddressDetailsList}.
     */
    public List<InetAddressDetailsList> getInetAddress() {
        return inetAddress;
    }

    /**
     * setInetAddress - function to set the inetAddress values for this object.
     *
     * @param inetAddress
     */
    public void setInetAddress(List<InetAddressDetailsList> inetAddress) {
        this.inetAddress = inetAddress;
    }

    public InetAddressDetails() {
    }

    /**
     * String representation of the object.
     *
     */
    public String toString() {
        return "[inetAddress:" + inetAddress + "]";
    }
}
