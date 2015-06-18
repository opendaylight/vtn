/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.Objects;
import java.util.Set;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.MacMapConfig;

/**
 * {@code MacMapConfig} class describes configuration information about the
 * MAC mapping in the vBridge.
 *
 * <p>
 *   This class is used by REST client to get or set whole configuration of
 *   the MAC mapping.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"allow": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"machost": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"vlan": 0},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"address": "00:11:22:33:44:55", "vlan": "10"}
 * &nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"deny": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"machost": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"address":"00:aa:bb:cc:dd:ee","vlan":0}
 * &nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "macmapconf")
@XmlAccessorType(XmlAccessType.NONE)
public class MacMapConfigInfo {
    /**
     * A set of <strong>machost</strong> elements that the list of host
     * information in Map Allow list.
     *
     * <ul>
     *    <li>
     *      If omitted, it will be treated as if the host information is not
     *      specified in Map Allow list.
     *    </li>
     *    <li>
     *      It will be aggregated as one entry if there are multiple
     *      <strong>machost</strong> elements with the same MAC address and
     *      VLAN ID.
     *    </li>
     * </ul>
     */
    @XmlElement(name = "allow")
    private MacHostSet  allowedHosts;

    /**
     * A set of <strong>machost</strong> elements that the list of host
     * information in Map Deny list.
     *
     * <ul>
     *    <li>
     *      If omitted, it will be treated as if the host information is not
     *      specified in Map Deny list.
     *    </li>
     *    <li>
     *      It will be aggregated as one entry if there are multiple
     *      <strong>machost</strong> elements with the same MAC address and
     *      VLAN ID.
     *    </li>
     * </ul>
     */
    @XmlElement(name = "deny")
    private MacHostSet  deniedHosts;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    protected MacMapConfigInfo() {
    }

    /**
     * Construct a new MAC mapping configuration information.
     *
     * @param mcconf  MAC mapping configuration.
     * @throws NullPointerException
     *    {@code mcconf} is {@code null}.
     */
    public MacMapConfigInfo(MacMapConfig mcconf) {
        Set<DataLinkHost> allowed = mcconf.getAllowedHosts();
        if (!allowed.isEmpty()) {
            allowedHosts = new MacHostSet(allowed);
        }

        Set<DataLinkHost> denied = mcconf.getDeniedHosts();
        if (!denied.isEmpty()) {
            deniedHosts = new MacHostSet(denied);
        }
    }

    /**
     * Return a {@link MacHostSet} instance which contains host information
     * to be mapped by the MAC mapping.
     *
     * @return  A {@link MacHostSet} instance is returned if at least one
     *          host to be mapped is configured.
     *          {@code null} is returned if no host to be mapped is configured.
     */
    MacHostSet getAllowedHosts() {
        return allowedHosts;
    }

    /**
     * Return a {@link MacHostSet} instance which contains host information
     * not to be mapped by the MAC mapping.
     *
     * @return  A {@link MacHostSet} instance is returned if at least one
     *          host not to be mapped is configured.
     *          {@code null} is returned if no host not to be mapped is
     *          configured.
     */
    MacHostSet getDeniedHosts() {
        return deniedHosts;
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
        if (!(o instanceof MacMapConfigInfo)) {
            return false;
        }

        MacMapConfigInfo mci = (MacMapConfigInfo)o;
        return (Objects.equals(allowedHosts, mci.allowedHosts) &&
                Objects.equals(deniedHosts, mci.deniedHosts));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (allowedHosts != null) {
            h ^= allowedHosts.hashCode();
        }
        if (deniedHosts != null) {
            h ^= deniedHosts.hashCode();
        }

        return h;
    }
}
