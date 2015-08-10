/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.PathMap;

/**
 * {@code PathMapList} class describes a list of path map information.
 *
 * <p>
 *   This class is used to return a list of path map information to
 *   REST client.
 * </p>
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"pathmap": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"index": 10,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"condition": "flowcond_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"policy": 1,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"idleTimeout": 300,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"hardTimeout": 0
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"index": 20,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"condition": "flowcond_2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"policy": 0
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "pathmaps")
@XmlAccessorType(XmlAccessType.NONE)
public class PathMapList {
    /**
     * A list of {@link PathMap} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more {@link PathMap} instances
     *     which represent information about path map.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "pathmap")
    private List<PathMap>  pathMaps;

    /**
     * Default constructor.
     */
    public PathMapList() {
    }

    /**
     * Construct a list of path maps.
     *
     * @param list  A list of path map information.
     */
    public PathMapList(List<PathMap> list) {
        pathMaps = list;
    }

    /**
     * Return a list of path map information.
     *
     * @return A list of path map information.
     */
    List<PathMap> getPathMaps() {
        return pathMaps;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof PathMapList)) {
            return false;
        }

        List<PathMap> list = ((PathMapList)o).pathMaps;
        if (pathMaps == null || pathMaps.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return pathMaps.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (pathMaps != null && !pathMaps.isEmpty()) {
            h ^= pathMaps.hashCode();
        }

        return h;
    }
}
