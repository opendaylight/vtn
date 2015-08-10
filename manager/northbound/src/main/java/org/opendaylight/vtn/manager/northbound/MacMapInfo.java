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

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;

/**
 * {@code MacMapInfo} class describes configuration information about the
 * MAC mapping configured in the vBridge.
 *
 * <p>
 *   This class is used to return information about the MAC mapping to
 *   REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"allow": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"machost": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"vlan": "0"},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"address": "00:11:22:33:44:55", "vlan": "10"}
 * &nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"deny": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"machost": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"address":"00:aa:bb:cc:dd:ee","vlan":0}
 * &nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;}
 * &nbsp;&nbsp;"mapped": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"macentry": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"address": "00:11:22:33:44:55",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"vlan": "10",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:01"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "2"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"inetAddresses": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"inetAddress": [{"address": "192.168.10.1"}]
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "macmap")
@XmlAccessorType(XmlAccessType.NONE)
public class MacMapInfo extends MacMapConfigInfo {
    /**
     * A list of <strong>macentry</strong> elements that shows the list of
     * hosts where mapping is actually acitve based on MAC mapping.
     *
     * <ul>
     *    <li>
     *      This element is omitted if no host is actually mapped.
     *    </li>
     * </ul>
     */
    @XmlElement(name = "mapped")
    private MacEntryList  mappedHosts;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private MacMapInfo() {
    }

    /**
     * Construct a new MAC mapping information.
     *
     * @param mcmap  MAC mapping information.
     * @throws NullPointerException
     *    {@code mcmap} is {@code null}.
     */
    public MacMapInfo(MacMap mcmap) {
        super(mcmap);

        List<MacAddressEntry> mapped = mcmap.getMappedHosts();
        if (!mapped.isEmpty()) {
            mappedHosts = new MacEntryList(mapped);
        }
    }

    /**
     * Return a {@link MacEntryList} instance which contains information about
     * hosts actually mapped to the vBridge by the MAC mapping.
     *
     * @return  A {@link MacEntryList} instance is returned if at least one
     *          host is actually mapped to the vBridge.
     *          {@code null} is returned if no host is mapped.
     */
    MacEntryList getMappedHosts() {
        return mappedHosts;
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
        if (!((o instanceof MacMapInfo) && super.equals(o))) {
            return false;
        }

        MacMapInfo mi = (MacMapInfo)o;
        if (mappedHosts == null) {
            return (mi.mappedHosts == null);
        }

        return mappedHosts.equals(mi.mappedHosts);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (mappedHosts != null) {
            h ^= mappedHosts.hashCode();
        }

        return h;
    }
}
