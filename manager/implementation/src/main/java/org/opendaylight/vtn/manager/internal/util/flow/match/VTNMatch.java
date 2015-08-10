/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlElements;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnMatchFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.Match;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpVersion;

/**
 * {@code VTNMatch} describes the condition to match against packtes.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-match")
@XmlAccessorType(XmlAccessType.NONE)
public class VTNMatch {
    /**
     * A separator used to construct flow condition key.
     */
    public static final String  COND_KEY_SEPARATOR = ",";

    /**
     * A brief description about the source address.
     */
    static final String  DESC_SRC = "source";

    /**
     * A brief description about the destination address.
     */
    static final String  DESC_DST = "destination";

    /**
     * Conditions against Ethernet header.
     */
    @XmlElement(name = "vtn-ether-match")
    private VTNEtherMatch  etherMatch;

    /**
     * Conditions against IP header.
     */
    @XmlElements({
        @XmlElement(name = "vtn-inet4-match", type = VTNInet4Match.class)})
    private VTNInetMatch  inetMatch;

    /**
     * Conditions against layer 4 protocol header.
     */
    @XmlElements({
        @XmlElement(name = "vtn-tcp-match", type = VTNTcpMatch.class),
        @XmlElement(name = "vtn-udp-match", type = VTNUdpMatch.class),
        @XmlElement(name = "vtn-icmp-match", type = VTNIcmpMatch.class)})
    private VTNLayer4Match  layer4Match;

    /**
     * Create a new flow match that matches every packet.
     */
    public VTNMatch() {
    }

    /**
     * Construct a new flow match.
     *
     * @param ematch   A {@link VTNEtherMatch} instance which specifies the
     *                 condition for Ethernet header.
     * @param imatch   A {@link VTNInetMatch} instance which specifies the
     *                 condition for IP header.
     * @param l4match  A {@link VTNLayer4Match} instance which specifies the
     *                 condition for layer 4 protocol header.
     * @throws RpcException
     *    The specified condition is invalid.
     */
    public VTNMatch(VTNEtherMatch ematch, VTNInetMatch imatch,
                    VTNLayer4Match l4match) throws RpcException {
        etherMatch = ematch;
        inetMatch = imatch;
        layer4Match = l4match;
        complete();
    }

    /**
     * Construct a new flow match from the given {@link Match} instance.
     *
     * <p>
     *   Note that this constructor does not complete the condition specified
     *   by the given {@link Match} instance.
     * </p>
     *
     * @param match  A {@link Match} instance.
     * @throws RpcException
     *    {@code match} contains invalid value.
     */
    public VTNMatch(Match match) throws RpcException {
        if (match != null) {
            etherMatch = VTNEtherMatch.create(match);
            inetMatch = VTNInetMatch.create(match);
            layer4Match = VTNLayer4Match.create(match);
        }
    }

    /**
     * Set flow conditions configured in the given {@link FlowMatch} instance.
     *
     * <p>
     *   This method completes the condition specified by the given
     *   {@link FlowMatch} instance. For instance, ethernet type is added
     *   if IP condition is configured in the given {@link FlowMatch} instance.
     * </p>
     *
     * @param fmatch  A {@link FlowMatch} instance.
     * @throws NullPointerException
     *    {@code fmatch} is {@code null}.
     * @throws RpcException
     *    {@code fmatch} contains invalid value.
     */
    public final void set(FlowMatch fmatch) throws RpcException {
        EthernetMatch eth = fmatch.getEthernetMatch();
        InetMatch inet = fmatch.getInetMatch();
        L4Match l4 = fmatch.getLayer4Match();

        etherMatch = (eth == null) ? null : new VTNEtherMatch(eth);
        inetMatch = VTNInetMatch.create(inet);
        layer4Match = VTNLayer4Match.create(l4);
        complete();
    }

    /**
     * Set flow conditions configured in the given {@link VtnMatchFields}
     * instance.
     *
     * <p>
     *   This method completes the condition specified by the given
     *   {@link VtnMatchFields} instance. For instance, ethernet type is added
     *   if IP condition is configured in the given {@link VtnMatchFields}
     *   instance.
     * </p>
     *
     * @param vmatch  A {@link VtnMatchFields} instance.
     * @throws NullPointerException
     *    {@code vmatch} is {@code null}.
     * @throws RpcException
     *    {@code fmatch} contains invalid value.
     */
    public final void set(VtnMatchFields vmatch) throws RpcException {
        VtnEtherMatch eth = vmatch.getVtnEtherMatch();
        VtnInetMatch ip = vmatch.getVtnInetMatch();
        VtnLayer4Match l4 = vmatch.getVtnLayer4Match();

        etherMatch = (eth == null) ? null : new VTNEtherMatch(eth);
        inetMatch = VTNInetMatch.create(etherMatch, ip);
        layer4Match = VTNLayer4Match.create(l4);
        complete();
    }

    /**
     * Return the condition against Ethernet header.
     *
     * @return  A {@link VTNEtherMatch} instance.
     *          {@code null} if no condition is specified.
     */
    public final VTNEtherMatch getEtherMatch() {
        return etherMatch;
    }

    /**
     * Return the condition against IP header.
     *
     * @return  A {@link VTNInetMatch} instance.
     *          {@code null} if no condition is specified.
     */
    public final VTNInetMatch getInetMatch() {
        return inetMatch;
    }

    /**
     * Return the condition against layer 4 protocol header.
     *
     * @return  A {@link VTNLayer4Match} instance.
     *          {@code null} if no condition is specified.
     */
    public final VTNLayer4Match getLayer4Match() {
        return layer4Match;
    }

    /**
     * Return a {@link FlowMatch} instance which represents this condition.
     *
     * @return  A {@link FlowMatch} instance.
     */
    public FlowMatch toFlowMatch() {
        return toFlowMatch(null);
    }

    /**
     * Return a {@link FlowMatch} instance which represents this condition.
     *
     * @param index  A index to be assigned to {@code FlowMatch}.
     * @return  A {@link FlowMatch} instance.
     */
    public final FlowMatch toFlowMatch(Integer index) {
        EthernetMatch eth = null;
        InetMatch inet = null;
        L4Match l4 = null;
        if (etherMatch != null) {
            eth = etherMatch.toEthernetMatch();
            if (inetMatch != null) {
                inet = inetMatch.toInetMatch();
                if (layer4Match != null) {
                    l4 = layer4Match.toL4Match();
                }
            }
        }

        return (index == null)
            ? new FlowMatch(eth, inet, l4)
            : new FlowMatch(index.intValue(), eth, inet, l4);
    }

    /**
     * Return a {@link DataFlowMatchBuilder} instance which contains the
     * flow conditions configured in this instance.
     *
     * @return  A {@link DataFlowMatchBuilder} instance.
     */
    public DataFlowMatchBuilder toDataFlowMatchBuilder() {
        DataFlowMatchBuilder builder = new DataFlowMatchBuilder();
        if (etherMatch != null) {
            builder.setVtnEtherMatch(etherMatch.toVtnEtherMatchBuilder().
                                     build());
        }
        if (inetMatch != null) {
            builder.setVtnInetMatch(inetMatch.toVtnInetMatchBuilder().build());
        }
        if (layer4Match != null) {
            builder.setVtnLayer4Match(layer4Match.toVtnLayer4Match());
        }

        return builder;
    }

    /**
     * Return a {@link MatchBuilder} instance which contains the flow
     * conditions configured in this instance.
     *
     * @return  A {@link MatchBuilder} instance.
     */
    public final MatchBuilder toMatchBuilder() {
        MatchBuilder builder = new MatchBuilder();
        if (etherMatch != null) {
            etherMatch.setMatch(builder);
            if (inetMatch != null) {
                inetMatch.setMatch(builder);
                if (layer4Match != null) {
                    layer4Match.setMatch(builder, inetMatch.getIpVersion());
                }
            }
        }

        return builder;
    }

    /**
     * Create a flow condition key that identifies the flow entry.
     *
     * @param snode    A {@link SalNode} corresponding to the target switch.
     * @param pri      Flow priority value.
     * @param ingress  A {@link SalPort} instance corresponding to the ingress
     *                 switch port.
     * @return  A string which can be used to identify the flow entry.
     */
    public final String getFlowKey(SalNode snode, int pri, SalPort ingress) {
        StringBuilder builder = new StringBuilder("node=").
            append(snode).append(COND_KEY_SEPARATOR).
            append("pri=").append(pri);
        if (ingress != null) {
            builder.append(COND_KEY_SEPARATOR).
                append(FlowMatchType.IN_PORT).append('=').append(ingress);
        }

        return createConditionKey(builder);
    }

    /**
     * Create a condition key that identifies this flow match.
     *
     * @return  A string which can be used to identify this flow match.
     */
    public final String getConditionKey() {
        return createConditionKey(new StringBuilder());
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verifycation failed.
     */
    public void verify() throws RpcException {
        complete();

        if (etherMatch != null) {
            etherMatch.verify();
            if (inetMatch != null) {
                inetMatch.verify();
                if (layer4Match != null) {
                    layer4Match.verify();
                }
            }
        }
    }

    /**
     * Complete the flow conditions.
     *
     * @throws RpcException
     *    This flow match contains inconsistent conditions.
     */
    public final void complete() throws RpcException {
        VTNEtherMatch ematch = etherMatch;
        VTNInetMatch imatch = inetMatch;

        if (layer4Match != null) {
            // IP protocol number must be specified.
            // Defaults to IPv4 if IP match is not specified.
            IpVersion ipver = (imatch == null)
                ? IpVersion.Ipv4
                : imatch.getIpVersion();
            short p = layer4Match.getInetProtocol(ipver);
            Short proto = Short.valueOf(p);
            if (imatch == null) {
                imatch = new VTNInet4Match(proto);
            } else {
                imatch.setProtocol(proto);
            }

            if (layer4Match.isEmpty()) {
                layer4Match = null;
            }
        }

        if (imatch != null) {
            // Ethernet type must be specified.
            Integer etype = Integer.valueOf(imatch.getEtherType());
            if (ematch == null) {
                ematch = new VTNEtherMatch(etype);
            } else {
                ematch.setEtherType(etype);
            }

            if (imatch.isEmpty()) {
                imatch = null;
            }
        }

        if (ematch != null && ematch.isEmpty()) {
            ematch = null;
        }

        etherMatch = ematch;
        inetMatch = imatch;
    }

    /**
     * Determine whether the given packet header matches the condition
     * specified by this instance.
     *
     * @param ctx   A {@link FlowMatchContext} instance.
     * @return  {@code true} only if the given packet header matches all
     *          the conditions configured in this instance.
     */
    public final boolean match(FlowMatchContext ctx) {
        // Test Ethernet header.
        if (etherMatch == null) {
            return true;
        }
        if (!etherMatch.match(ctx)) {
            return false;
        }

            // Test IP header.
        if (inetMatch == null) {
            return true;
        }
        if (!inetMatch.match(ctx)) {
            return false;
        }

        // Test layer 4 protocol header.
        return (layer4Match == null || layer4Match.match(ctx));
    }

    /**
     * Return the IP protocol specified by this match.
     *
     * @return  A {@link Short} instance which represents the IP protocol
     *          number if this match specifies the IP protocol.
     *          {@code null} if this match does not specify the IP protocol.
     */
    public final Short getInetProtocol() {
        return (inetMatch == null) ? null : inetMatch.getProtocol();
    }

    /**
     * Create a string which identifies the flow condition.
     *
     * @param builder  A {@link StringBuilder} instance which contains strings
     *                 used to construct flow condition key.
     * @return  A string which identifies the flow condition.
     */
    private String createConditionKey(StringBuilder builder) {
        if (etherMatch != null) {
            etherMatch.setConditionKey(builder);
            if (inetMatch != null) {
                inetMatch.setConditionKey(builder);
                if (layer4Match != null) {
                    layer4Match.setConditionKey(builder);
                }
            }
        }

        return builder.toString();
    }

    // Objects

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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        VTNMatch vm = (VTNMatch)o;
        return (Objects.equals(etherMatch, vm.etherMatch) &&
                Objects.equals(inetMatch, vm.inetMatch) &&
                Objects.equals(layer4Match, vm.layer4Match));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass().getName(), etherMatch, inetMatch,
                            layer4Match);
    }
}
