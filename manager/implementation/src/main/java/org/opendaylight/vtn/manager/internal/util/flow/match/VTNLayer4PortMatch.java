/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4PortHeader;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnPortRange;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNLayer4PortMatch} describes the condition for IP port numbers
 * to match against packets.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 *
 * @param <H>  The type of packet header.
 */
@XmlRootElement(name = "vtn-layer4-port-match")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({VTNTcpMatch.class, VTNUdpMatch.class})
public abstract class VTNLayer4PortMatch<H extends Layer4PortHeader>
    extends VTNLayer4Match {
    /**
     * The range of the source port number to match against packets.
     */
    @XmlElement(name = "source-port")
    private VTNPortRange  sourcePort;

    /**
     * The range of the destination port number to match against packets.
     */
    @XmlElement(name = "destination-port")
    private VTNPortRange  destinationPort;

    /**
     * Construct a new instance.
     */
    VTNLayer4PortMatch() {
    }

    /**
     * Construct a new instance.
     *
     * @param src  A {@link VTNPortRange} instance which specifies the range
     *             of the source port number. {@code null} matches every
     *             source port number.
     * @param dst  A {@link VTNPortRange} instance which specifies the range
     *             of the destination port number. {@code null} matches every
     *             destination port number.
     */
    VTNLayer4PortMatch(VTNPortRange src, VTNPortRange dst) {
        sourcePort = src;
        destinationPort = dst;
    }

    /**
     * Construct a new instance from the given pair of {@link VtnPortRange}
     * instances.
     *
     * @param src  A {@link VtnPortRange} instance which specifies the range
     *             of source port number.
     * @param dst  A {@link VtnPortRange} instance which specifies the range
     *             of destination port number.
     * @throws RpcException
     *    {@code src} or {@code dst} contains invalid value.
     */
    VTNLayer4PortMatch(VtnPortRange src, VtnPortRange dst)
        throws RpcException {
        sourcePort = VTNPortRange.create(src);
        destinationPort = VTNPortRange.create(dst);
    }

    /**
     * Construct a new instance from the given pair of {@link PortNumber}
     * instances.
     *
     * @param src  A {@link PortNumber} instance which specifies the source
     *             port number.
     * @param dst  A {@link VtnPortRange} instance which specifies the
     *             destination port number.
     * @throws RpcException
     *    {@code src} or {@code dst} contains invalid value.
     */
    VTNLayer4PortMatch(PortNumber src, PortNumber dst)
        throws RpcException {
        sourcePort = VTNPortRange.create(src);
        destinationPort = VTNPortRange.create(dst);
    }

    /**
     * Return the range of the source port number to match against packet.
     *
     * @return  A {@link VTNPortRange} instance if the range of the source
     *          port number is specified. {@code null} if not specified.
     */
    public final VTNPortRange getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the range of the destination port number to match against packet.
     *
     * @return  A {@link VTNPortRange} instance if the range of the destination
     *          port number is specified. {@code null} if not specified.
     */
    public final VTNPortRange getDestinationPort() {
        return destinationPort;
    }

    /**
     * Return a class which represents the packet header type.
     *
     * @return  A class associated with the packet header type.
     */
    public abstract Class<H> getHeaderType();

    /**
     * Return a flow match type corresponding to the source port.
     *
     * @return  A {@link FlowMatchType} instance.
     */
    public abstract FlowMatchType getSourceMatchType();

    /**
     * Return a flow match type corresponding to the destination port.
     *
     * @return  A {@link FlowMatchType} instance.
     */
    public abstract FlowMatchType getDestinationMatchType();

    // VTNLayer4Match

    /**
     * {@inheritDoc}
     */
    @Override
    public final void verify() throws RpcException {
        if (sourcePort != null) {
            sourcePort.verify();
        }
        if (destinationPort != null) {
            destinationPort.verify();
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean match(FlowMatchContext ctx) {
        Class<H> type = getHeaderType();
        Layer4Header l4 = ctx.getLayer4Header();
        if (!type.isInstance(l4)) {
            return false;
        }

        H l4head = type.cast(l4);
        if (sourcePort != null) {
            ctx.addMatchField(getSourceMatchType());
            int port = l4head.getSourcePort();
            if (!sourcePort.match(port)) {
                return false;
            }
        }

        if (destinationPort != null) {
            ctx.addMatchField(getDestinationMatchType());
            int port = l4head.getDestinationPort();
            if (!destinationPort.match(port)) {
                return false;
            }
        }

        return true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final void setConditionKey(StringBuilder builder) {
        String sep = (builder.length() == 0)
            ? "" : VTNMatch.COND_KEY_SEPARATOR;

        if (sourcePort != null) {
            builder.append(sep).append(getSourceMatchType()).append('=');
            sourcePort.setConditionKey(builder);
            sep = VTNMatch.COND_KEY_SEPARATOR;
        }

        if (destinationPort != null) {
            builder.append(sep).append(getDestinationMatchType()).append('=');
            destinationPort.setConditionKey(builder);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean isEmpty() {
        return (sourcePort == null && destinationPort == null);
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        VTNLayer4PortMatch l4 = (VTNLayer4PortMatch)o;
        return (Objects.equals(sourcePort, l4.sourcePort) &&
                Objects.equals(destinationPort, l4.destinationPort));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return Objects.hash(getClass().getName(), sourcePort, destinationPort);
    }
}
