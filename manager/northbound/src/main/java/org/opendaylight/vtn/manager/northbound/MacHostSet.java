/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.Collection;
import java.util.Set;
import java.util.HashSet;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.DataLinkHost;

/**
 * {@link MacHostSet} class describes a set of host information in data link
 * layer.
 *
 * <p>
 *   This class is used by REST client to get or set a set of host information
 *   in data link layer.
 *   Note that an instance of this class cannot contain duplicate elements.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"machost":[
 * &nbsp;&nbsp;&nbsp;&nbsp;{"vlan": "0"},
 * &nbsp;&nbsp;&nbsp;&nbsp;{"address": "00:11:22:33:44:55", "vlan": "10"}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "machosts")
@XmlAccessorType(XmlAccessType.NONE)
public class MacHostSet {
    /**
     * A set of {@link MacHost} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more <strong>machost</strong> elements
     *     which represent host information for data link layer.
     *   </li>
     *   <li>
     *     It will be aggregated to one entry if multiple
     *     <strong>machost</strong> attributes are specified with the same
     *     MAC address and VLAN ID.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "machost")
    private final Set<MacHost>  hosts = new HashSet<MacHost>();

    /**
     * Default constructor.
     */
    public MacHostSet() {
    }

    /**
     * Construct a set of host information in data link layer.
     *
     * @param dlhosts  A collection of {@link DataLinkHost} instances.
     *                 Duplicate elements in {@code dlhosts} are not added
     *                 to this instance.
     */
    public MacHostSet(Collection<? extends DataLinkHost> dlhosts) {
        if (dlhosts != null) {
            for (DataLinkHost dlhost: dlhosts) {
                hosts.add(new MacHost(dlhost));
            }
        }
    }

    /**
     * Return a set of host information in data link layer.
     *
     * @return  A set of host information in data link layer.
     *          {@code null} is returned if this instance is empty.
     */
    Set<MacHost> getHosts() {
        return hosts;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof MacHostSet)) {
            return false;
        }

        Set<MacHost> set = ((MacHostSet)o).hosts;
        return hosts.equals(set);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return hosts.hashCode();
    }
}
