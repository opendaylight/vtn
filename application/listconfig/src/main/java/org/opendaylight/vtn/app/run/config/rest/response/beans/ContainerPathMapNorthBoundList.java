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

@RestURL(vtnMgrUrl = "controller/nb/v2/vtn/ {containerName}/pathmaps")
@JsonObject
public class ContainerPathMapNorthBoundList implements Serializable, Cloneable {

    /**
     * ContainerPathMapNorthBoundList - Bean Representaion for list of
     * ContainerPathMapNorthBound object from the JSON Response.
     *
     */
    private static final long serialVersionUID = 4704041441484005061L;

    /**
     * The path reference.
     */
    @JsonArray(name = "pathmap")
    private List<ContainerPathMapNorthBound> pathMap = new ArrayList<ContainerPathMapNorthBound>();

    public ContainerPathMapNorthBoundList() {
    }
    /**
     * ContainerPathMapNorthBoundList - Constructor with arguments.
     * @param pathMaps -List of path reference.
     */
    public ContainerPathMapNorthBoundList(
            List<ContainerPathMapNorthBound> pathMaps) {
        this.pathMap = pathMaps;
    }

    /**
     * getPathMaps - function to get the pathmps.
     *
     * @return List of ContainerPathMapNorthBound objects
     */
    public List<ContainerPathMapNorthBound> getPathMaps() {
        return pathMap;
    }

    /**
     * setPathMaps - function to set the pathmps.
     *
     * @param pathMaps
     */
    public void setPathMaps(List<ContainerPathMapNorthBound> pathMaps) {
        this.pathMap = pathMaps;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "pathMaps [pathMap:" + pathMap + "]";
    }
}
