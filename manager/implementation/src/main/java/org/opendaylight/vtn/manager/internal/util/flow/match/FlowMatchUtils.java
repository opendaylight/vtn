/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.internal.L2Host;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.MacAddressFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.EthernetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.match.VlanMatch;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code FlowMatchUtils} class is a collection of utility class methods for
 * flow match handling.
 */
public final class FlowMatchUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private FlowMatchUtils() {}

    /**
     * Return the source L2 host matched by the given MD-SAL match.
     *
     * @param match  A {@link Match} instance.
     * @return  A {@link L2Host} instance if found.
     *          {@code null} if not found.
     */
    public static L2Host getSourceHost(Match match) {
        SalPort sport = SalPort.create(getIngressPort(match));
        if (sport == null) {
            return null;
        }

        VlanId vid = getVlanId(match);
        if (vid == null) {
            return null;
        }

        MacAddress mac = getSourceMacAddress(match);
        return new L2Host(mac, vid.getValue().intValue(), sport);
    }

    /**
     * Return the source MAC address specified by the given match.
     *
     * @param match  A {@link Match} instance.
     * @return  A {@link MacAddress} instance which specifies the source MAC
     *          address. {@code null} if the given match does not specify
     *          the source MAC address.
     */
    public static MacAddress getSourceMacAddress(Match match) {
        EthernetMatch ether = match.getEthernetMatch();
        return (ether == null)
            ? null
            : getMacAddress(ether.getEthernetSource());
    }

    /**
     * Return the destination MAC address specified by the given match.
     *
     * @param match  A {@link Match} instance.
     * @return  A {@link MacAddress} instance which specifies the destination
     *          MAC address. {@code null} if the given match does not specify
     *          the destination MAC address.
     */
    public static MacAddress getDestinationMacAddress(Match match) {
        EthernetMatch ether = match.getEthernetMatch();
        return (ether == null)
            ? null
            : getMacAddress(ether.getEthernetDestination());
    }

    /**
     * Return VLAN ID specified by the given match.
     *
     * @param match  A {@link Match} instance.
     * @return  A {@link VlanId} instance if the given match specifies the
     *          VLAN ID. Note that {@link EtherHeader#VLAN_NONE} indicates
     *          the given match specifies untagged frame.
     *          {@code null} if the given match does not specify the VLAN ID.
     */
    public static VlanId getVlanId(Match match) {
        VlanMatch vlan = match.getVlanMatch();
        if (vlan == null) {
            return null;
        }

        org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.
            rev131026.vlan.match.fields.VlanId vid = vlan.getVlanId();
        if (vid == null) {
            return null;
        }

        return (Boolean.TRUE.equals(vid.isVlanIdPresent()))
            ? vid.getVlanId()
            : new VlanId(EtherHeader.VLAN_NONE);
    }

    /**
     * Return the ingress switch port configured in the given MD-SAL match.
     *
     * @param match  A {@link Match} instance.
     * @return  A {@link NodeConnectorId} instance if found.
     *          {@code null} if not found.
     */
    public static NodeConnectorId getIngressPort(Match match) {
        return (match == null) ? null : match.getInPort();
    }

    /**
     * Return the MAC address configured in the given instance.
     *
     * @param mf  A {@link MacAddressFilter} instance.
     * @return  A {@link MacAddress} instance if found.
     *          {@code null} if not found.
     */
    private static MacAddress getMacAddress(MacAddressFilter mf) {
        if (mf == null) {
            return null;
        }

        // MAC address mask is not supported.
        return (mf.getMask() == null) ? mf.getAddress() : null;
    }
}
