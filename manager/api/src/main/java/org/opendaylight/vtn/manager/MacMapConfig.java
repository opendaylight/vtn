/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Set;
import java.util.HashSet;

/**
 * {@code MacMapConfig} class describes configuration information about the
 * MAC mapping.
 *
 * <p>
 *   This class is used to specify configuration information about the MAC
 *   mapping to the VTN Manager during configuration of MAC mapping.
 * </p>
 *
 * @see    <a href="package-summary.html#MAC-map">MAC mapping</a>
 * @since  Helium
 */
public class MacMapConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 420805623673965595L;

    /**
     * A set of {@link DataLinkHost} instances which represents MAC addresses
     * to be mapped by the MAC mapping.
     */
    private final Set<DataLinkHost>  allowedHosts =
        new HashSet<DataLinkHost>();

    /**
     * A set of {@link DataLinkHost} instances which represents MAC addresses
     * not to be mapped by the MAC mapping.
     */
    private final Set<DataLinkHost>  deniedHosts = new HashSet<DataLinkHost>();

    /**
     * Construct a new configuration information about the MAC mapping.
     *
     * <p>
     *   At present, VTN Manager can handle Ethernet frames only. Therefore,
     *   it is necessary to configure {@link EthernetHost} instances in
     *   {@code allow} and {@code deny}.
     * </p>
     *
     * @param allow
     *    A set of {@link DataLinkHost} instances which shows the list of
     *    host information in
     *    {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *    <ul>
     *      <li>
     *        Host information will not be configured in Map Allow list
     *        if {@code null} is specified.
     *      </li>
     *    </ul>
     * @param deny
     *    A set of {@link DataLinkHost} instances which shows the list of
     *    host information in
     *    {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}.
     *    <ul>
     *      <li>
     *        Host information will not be configured in Map Deny list
     *        if {@code null} is specified.
     *      </li>
     *    </ul>
     */
    public MacMapConfig(Set<? extends DataLinkHost> allow,
                        Set<? extends DataLinkHost> deny) {
        if (allow != null) {
            allowedHosts.addAll(allow);
        }
        if (deny != null) {
            deniedHosts.addAll(deny);
        }
    }

    /**
     * Return a set of {@link DataLinkHost} instances that shows the host
     * information configured in
     * {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *
     * @return  A set of {@link DataLinkHost} instances.
     */
    public Set<DataLinkHost> getAllowedHosts() {
        return new HashSet<DataLinkHost>(allowedHosts);
    }

    /**
     * Return a set of {@link DataLinkHost} instances that shows the host
     * information configured in
     * {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}.
     *
     * @return  A set of {@link DataLinkHost} instances.
     */
    public Set<DataLinkHost> getDeniedHosts() {
        return new HashSet<DataLinkHost>(deniedHosts);
    }

    /**
     * Append human readable strings which represents the contents of this
     * object to the specified {@link StringBuilder}.
     *
     * @param builder  A {@link StringBuilder} instance.
     * @return  A {@link StringBuilder} instance specified by {@code builder}.
     */
    StringBuilder appendContents(StringBuilder builder) {
        builder.append("allow=").append(allowedHosts.toString()).
            append(",deny=").append(deniedHosts.toString());
        return builder;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code MacMapConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         A set of {@link DataLinkHost} instances which represents
     *         host information in
     *         {@linkplain <a href="package-summary.html#MAC-map.allow">Map Allow list</a>}.
     *       </li>
     *       <li>
     *         A set of {@link DataLinkHost} instances which represents
     *         host information in
     *         {@linkplain <a href="package-summary.html#MAC-map.deny">Map Deny list</a>}.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof MacMapConfig)) {
            return false;
        }

        MacMapConfig mcconf = (MacMapConfig)o;
        return (allowedHosts.equals(mcconf.allowedHosts) &&
                deniedHosts.equals(mcconf.deniedHosts));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return allowedHosts.hashCode() ^ deniedHosts.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("MacMapConfig[");
        appendContents(builder).append(']');
        return builder.toString();
    }
}
