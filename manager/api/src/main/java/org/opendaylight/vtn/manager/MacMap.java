/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
 * {@code MacMap} class describes information about the MAC mapping, which
 * maps hosts to a vBridge.
 *
 * <p>
 *   {@code MacMap} class inherits {@link MacMapConfig} class, and
 *   {@code MacMap} object contains the configuration information about the
 *   MAC mapping. The VTN Manager passes the MAC mapping information to
 *   other components by passing {@code MacMap} object.
 * </p>
 *
 * @see    <a href="package-summary.html#MAC-map">MAC mapping</a>
 * @since  Helium
 */
public class MacMap extends MacMapConfig {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5408809363858835693L;

    /**
     * A list of {@link MacAddressEntry} instances which represents
     * MAC addresses actually mapped to the vBridge.
     */
    private final List<MacAddressEntry>  mappedHosts =
        new ArrayList<MacAddressEntry>();

    /**
     * Construct a new object which represents information about the
     * MAC mapping.
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
     * @param mapped
     *    A set of {@link MacAddressEntry} instances which shows the list of
     *    {@linkplain <a href="package-summary.html#MAC-map.activate">hosts where mapping is actually active</a>}
     *    based on MAC mapping.
     *    <ul>
     *      <li>
     *        Mapped hosts will not be configured if {@code null} is specified.
     *      </li>
     *    </ul>
     */
    public MacMap(Set<? extends DataLinkHost> allow,
                  Set<? extends DataLinkHost> deny,
                  List<MacAddressEntry> mapped) {
        super(allow, deny);
        if (mapped != null) {
            mappedHosts.addAll(mapped);
        }
    }

    /**
     * Return a list of {@link MacAddressEntry} instances that shows the list
     * of
     * {@linkplain <a href="package-summary.html#MAC-map.activate">hosts where mapping is actually active</a>}
     * based on MAC mapping.
     *
     * @return  A list of {@link MacAddressEntry} instances.
     */
    public List<MacAddressEntry> getMappedHosts() {
        return new ArrayList<MacAddressEntry>(mappedHosts);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code MacMap} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         A set of {@link DataLinkHost} instances which represents
     *         MAC addresses to be mapped.
     *       </li>
     *       <li>
     *         A set of {@link DataLinkHost} instances which represents
     *         MAC addresses not to be mapped.
     *       </li>
     *       <li>
     *         A list of {@link MacAddressEntry} instances which represents
     *         MAC addresses actually mapped by the MAC mapping.
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
        if (!((o instanceof MacMap) && super.equals(o))) {
            return false;
        }

        MacMap macmap = (MacMap)o;
        return mappedHosts.equals(macmap.mappedHosts);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() ^ mappedHosts.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("MacMap[");
        appendContents(builder).
            append(",mapped=").append(mappedHosts.toString()).append(']');
        return builder.toString();
    }
}
