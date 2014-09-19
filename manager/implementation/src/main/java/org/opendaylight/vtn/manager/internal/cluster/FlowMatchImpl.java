/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.internal.MiscUtils;
import org.opendaylight.vtn.manager.internal.PacketContext;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * Implementation of flow match.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class FlowMatchImpl implements PacketMatch {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2201310174894943217L;

    /**
     * The minimum value of match index.
     */
    private static final int  INDEX_MIN = 1;

    /**
     * The maximum value of match index.
     */
    private static final int  INDEX_MAX = 65535;

    /**
     * An index value assigned to this condition.
     */
    private final int  index;

    /**
     * Condition to test Ethernet header.
     */
    private final EthernetMatchImpl  ethernetMatch;

    /**
     * Condition to test IP header.
     */
    private final InetMatchImpl  inetMatch;

    /**
     * Condition to layer 4 protocol header in an IP packet.
     */
    private final L4MatchImpl  l4Match;

    /**
     * Construct a new instance.
     *
     * @param match  A {@link FlowMatch} instance.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    public FlowMatchImpl(FlowMatch match) throws VTNException {
        if (match == null) {
            Status st = MiscUtils.argumentIsNull("Flow match");
            throw new VTNException(st);
        }

        Integer idx = match.getIndex();
        if (idx == null) {
            Status st = MiscUtils.argumentIsNull("Match index");
            throw new VTNException(st);
        }

        index = idx.intValue();
        if (index < INDEX_MIN || index > INDEX_MAX) {
            String msg = "Invalid match index: " + idx;
            throw new VTNException(StatusCode.BADREQUEST, msg);
        }

        EthernetMatch eth = match.getEthernetMatch();
        InetMatch inet = match.getInetMatch();
        L4Match l4 = match.getLayer4Match();

        EthernetMatchImpl ematch = (eth == null)
            ? null : new EthernetMatchImpl(eth);
        InetMatchImpl imatch = (inet == null)
            ? null : InetMatchImpl.create(inet);

        if (l4 == null) {
            l4Match = null;
        } else {
            l4Match = L4MatchImpl.create(l4);

            // IP protocol number must be specified.
            short proto = l4Match.getInetProtocol();
            if (imatch == null) {
                imatch = new Inet4MatchImpl(proto);
            } else {
                imatch.setProtocol(proto);
            }
        }

        if (imatch != null) {
            // Ethernet type must be specified.
            int etype = imatch.getEtherType();
            if (ematch == null) {
                ematch = new EthernetMatchImpl(etype);
            } else {
                ematch.setEtherType(etype);
            }
        }

        ethernetMatch = ematch;
        inetMatch = imatch;
    }

    /**
     * Return the match index assigned to this instance.
     *
     * @return  The match index.
     */
    public int getIndex() {
        return index;
    }

    /**
     * Return a {@link FlowMatch} instance which represents this condition.
     *
     * @return  A {@link FlowMatch} instance.
     */
    public FlowMatch getMatch() {
        EthernetMatch eth = (ethernetMatch == null)
            ? null : ethernetMatch.getMatch();
        InetMatch inet = (inetMatch == null) ? null : inetMatch.getMatch();
        L4Match l4 = (l4Match == null) ? null : l4Match.getMatch();

        return new FlowMatch(index, eth, inet, l4);
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
        if (!(o instanceof FlowMatchImpl)) {
            return false;
        }

        FlowMatchImpl match = (FlowMatchImpl)o;
        return (index == match.index &&
                Objects.equals(ethernetMatch, match.ethernetMatch) &&
                Objects.equals(inetMatch, match.inetMatch) &&
                Objects.equals(l4Match, match.l4Match));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(ethernetMatch, inetMatch, l4Match) ^ index;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("FlowMatchImpl[index=");
        builder.append(index);
        if (ethernetMatch != null) {
            builder.append(",ether=").append(ethernetMatch.toString());
        }
        if (inetMatch != null) {
            builder.append(",inet=").append(inetMatch.toString());
        }
        if (l4Match != null) {
            builder.append(",L4=").append(l4Match.toString());
        }
        builder.append(']');

        return builder.toString();
    }

    // PacketMatch

    /**
     * Determine whether the specified packet matches the condition defined
     * by this instance.
     *
     * @param pctx  The context of the packet to be tested.
     * @return  {@code true} if the specified packet matches the condition.
     *          Otherwise {@code false}.
     */
    @Override
    public boolean match(PacketContext pctx) {
        // Test Ethernet header.
        if (ethernetMatch != null && !ethernetMatch.match(pctx)) {
            return false;
        }

        // Test IP header.
        if (inetMatch != null && !inetMatch.match(pctx)) {
            return false;
        }

        // Test layer 4 protocol header.
        return (l4Match == null || l4Match.match(pctx));
    }
}
