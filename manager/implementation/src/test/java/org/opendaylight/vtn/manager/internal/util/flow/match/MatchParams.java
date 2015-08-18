/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.HashMap;
import java.util.Map;

import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.InetMatch;
import org.opendaylight.vtn.manager.flow.cond.L4Match;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.packet.EtherHeader;
import org.opendaylight.vtn.manager.internal.util.packet.InetHeader;
import org.opendaylight.vtn.manager.internal.util.packet.Layer4Header;
import org.opendaylight.vtn.manager.internal.util.packet.PacketHeader;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.IPProtocols;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnEtherMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnInetMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.match.fields.VtnLayer4Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.info.DataFlowMatchBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.MatchBuilder;

/**
 * {@code MatchParams} describes parameters for conditions to match against
 * packets.
 */
public class MatchParams extends TestBase implements PacketHeader, Cloneable {
    /**
     * An {@link EtherMatchParams} instance.
     */
    private EtherMatchParams  etherParams;

    /**
     * An {@link Inet4MatchParams} instance.
     */
    private Inet4MatchParams  inet4Params;

    /**
     * An {@link Layer4MatchParams} instance.
     */
    private Layer4MatchParams  layer4Params;

    /**
     * Create a map that contains {@link MatchParams} instances for test.
     *
     * <p>
     *   An instance of {@link MatchParams} to be used to construct
     *   {@link VTNMatch} instance is set as a key, and an instance of
     *   {@link MatchParams} instances that contains conditions expected to
     *   be configured in {@link VTNMatch} is set as a value.
     * </p>
     *
     * @return  A map that contains {@link MatchParams} instances.
     */
    public static Map<MatchParams, MatchParams> createMatches() {
        Map<MatchParams, MatchParams> map = new HashMap<>();
        MatchParams params = new MatchParams();
        map.put(params, params);

        // Match to Ethernet header.
        EtherMatchParams eparams = new EtherMatchParams().
            setSourceAddress(0x000102030405L).
            setDestinationAddress(0xf0f1f2f3f4f5L).
            setVlanId(Integer.valueOf(EtherHeader.VLAN_NONE));
        params = new MatchParams().setEtherParams(eparams);
        map.put(params, params);

        eparams = new EtherMatchParams().
            setSourceAddress(0xa83401bf34ceL).
            setDestinationAddress(0x00abcdef1234L).
            setEtherType(Integer.valueOf(0x800)).
            setVlanId(Integer.valueOf(4095)).
            setVlanPriority(Short.valueOf((short)7));
        params = new MatchParams().setEtherParams(eparams);
        map.put(params, params);

        // Match to IPv4 header.
        Inet4MatchParams ip4params = new Inet4MatchParams();
        params = new MatchParams().setInet4Params(ip4params);

        // Ethernet type will be configured.
        Integer ethIpv4 = Integer.valueOf(EtherTypes.IPv4.intValue());
        eparams = new EtherMatchParams().setEtherType(ethIpv4);
        MatchParams expected = new MatchParams().setEtherParams(eparams);
        map.put(params, expected);

        ip4params = new Inet4MatchParams().setDscp(Short.valueOf((short)41));
        params = new MatchParams().setInet4Params(ip4params);
        expected = new MatchParams().
            setEtherParams(eparams).setInet4Params(ip4params);
        map.put(params, expected);

        eparams = new EtherMatchParams().
            setSourceAddress(0x3873cad23847L).
            setVlanId(Integer.valueOf(3));
        ip4params = new Inet4MatchParams().
            setSourceNetwork(IpNetwork.create("192.168.100.255")).
            setSourcePrefix(Short.valueOf((short)25)).
            setDestinationNetwork(IpNetwork.create("203.198.39.255")).
            setDestinationPrefix(Short.valueOf((short)23)).
            setProtocol(Short.valueOf((short)123)).
            setDscp(Short.valueOf((short)45));
        params = new MatchParams().
            setEtherParams(eparams).setInet4Params(ip4params);
        eparams = new EtherMatchParams().
            setSourceAddress(0x3873cad23847L).
            setEtherType(ethIpv4).
            setVlanId(Integer.valueOf(3));
        ip4params.setSourceNetwork(IpNetwork.create("192.168.100.255/25")).
            setDestinationNetwork(IpNetwork.create("203.198.39.255/23"));
        expected = new MatchParams().
            setEtherParams(eparams).setInet4Params(ip4params);
        map.put(params, expected);

        // Match to TCP header.
        TcpMatchParams tcpParams = new TcpMatchParams();
        params = new MatchParams().setLayer4Params(tcpParams);

        // Ethernet type and IP protocol will be configured.
        Short tcpProto = Short.valueOf(IPProtocols.TCP.shortValue());
        eparams = new EtherMatchParams().setEtherType(ethIpv4);
        ip4params = new Inet4MatchParams().setProtocol(tcpProto);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params);
        map.put(params, expected);

        tcpParams = new TcpMatchParams().
            setSourcePortFrom(35).setDestinationPortFrom(987);
        params = new MatchParams().setLayer4Params(tcpParams);
        tcpParams = new TcpMatchParams().
            setSourcePortFrom(35).setSourcePortTo(35).
            setDestinationPortFrom(987).setDestinationPortTo(987);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(tcpParams);
        map.put(params, expected);

        eparams = new EtherMatchParams().
            setSourceAddress(0x043e1eb0e32cL).
            setDestinationAddress(0xc8ff32f9e7feL).
            setVlanId(Integer.valueOf(123)).
            setVlanPriority(Short.valueOf((short)4));
        ip4params = new Inet4MatchParams().
            setSourceNetwork(IpNetwork.create("10.20.30.45")).
            setDestinationNetwork(IpNetwork.create("192.168.90.100")).
            setDscp(Short.valueOf((short)0));
        tcpParams = new TcpMatchParams().
            setSourcePortFrom(100).setSourcePortTo(101).
            setDestinationPortFrom(30000).setDestinationPortTo(40000);
        params = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(tcpParams);
        eparams = new EtherMatchParams().
            setSourceAddress(0x043e1eb0e32cL).
            setDestinationAddress(0xc8ff32f9e7feL).
            setEtherType(ethIpv4).
            setVlanId(Integer.valueOf(123)).
            setVlanPriority(Short.valueOf((short)4));
        ip4params = new Inet4MatchParams().
            setSourceNetwork(IpNetwork.create("10.20.30.45")).
            setDestinationNetwork(IpNetwork.create("192.168.90.100")).
            setProtocol(tcpProto).
            setDscp(Short.valueOf((short)0));
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(tcpParams);
        map.put(params, expected);

        // Match to UDP header.
        UdpMatchParams udpParams = new UdpMatchParams();
        params = new MatchParams().setLayer4Params(udpParams);

        // Ethernet type and IP protocol will be configured.
        Short udpProto = Short.valueOf(IPProtocols.UDP.shortValue());
        eparams = new EtherMatchParams().setEtherType(ethIpv4);
        ip4params = new Inet4MatchParams().setProtocol(udpProto);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params);
        map.put(params, expected);

        udpParams = new UdpMatchParams().
            setSourcePortFrom(999).setDestinationPortFrom(64321);
        params = new MatchParams().setLayer4Params(udpParams);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(udpParams);
        map.put(params, expected);

        eparams = new EtherMatchParams().
            setSourceAddress(0xfc086b487935L).
            setVlanId(Integer.valueOf(19));
        ip4params = new Inet4MatchParams().
            setDestinationNetwork(IpNetwork.create("123.234.56.78"));
        udpParams = new UdpMatchParams().
            setSourcePortFrom(12345).setSourcePortTo(20000).
            setDestinationPortFrom(50).setDestinationPortTo(60);
        params = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(udpParams);
        eparams = new EtherMatchParams().
            setSourceAddress(0xfc086b487935L).
            setEtherType(ethIpv4).
            setVlanId(Integer.valueOf(19));
        ip4params = new Inet4MatchParams().
            setDestinationNetwork(IpNetwork.create("123.234.56.78")).
            setProtocol(udpProto);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(udpParams);
        map.put(params, expected);

        // Match to ICMP header.
        IcmpMatchParams icmpParams = new IcmpMatchParams();
        params = new MatchParams().setLayer4Params(icmpParams);

        // Ethernet type and IP protocol will be configured.
        Short icmpProto = Short.valueOf(IPProtocols.ICMP.shortValue());
        eparams = new EtherMatchParams().setEtherType(ethIpv4);
        ip4params = new Inet4MatchParams().setProtocol(icmpProto);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params);
        map.put(params, expected);

        icmpParams = new IcmpMatchParams().
            setType(Short.valueOf((short)0)).
            setCode(Short.valueOf((short)0));
        params = new MatchParams().setLayer4Params(icmpParams);
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(icmpParams);
        map.put(params, expected);

        eparams = new EtherMatchParams().
            setVlanId(Integer.valueOf(159)).
            setVlanPriority(Short.valueOf((short)6));
        ip4params = new Inet4MatchParams().
            setSourceNetwork(IpNetwork.create("10.245.32.189")).
            setSourcePrefix(Short.valueOf((short)28)).
            setDestinationNetwork(IpNetwork.create("192.168.195.209")).
            setDestinationPrefix(Short.valueOf((short)31)).
            setDscp(Short.valueOf((short)49));
        icmpParams = new IcmpMatchParams().
            setType(Short.valueOf((short)123)).
            setCode(Short.valueOf((short)91));
        params = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(icmpParams);
        eparams = new EtherMatchParams().
            setEtherType(ethIpv4).
            setVlanId(Integer.valueOf(159)).
            setVlanPriority(Short.valueOf((short)6));
        ip4params = new Inet4MatchParams().
            setSourceNetwork(IpNetwork.create("10.245.32.189/28")).
            setDestinationNetwork(IpNetwork.create("192.168.195.209/31")).
            setProtocol(icmpProto).
            setDscp(Short.valueOf((short)49));
        expected = new MatchParams().setEtherParams(eparams).
            setInet4Params(ip4params).setLayer4Params(icmpParams);
        map.put(params, expected);

        return map;
    }

    /**
     * Construct an empty instance.
     */
    public MatchParams() {
    }

    /**
     * Copy constructor.
     *
     * @param params  A {@link MatchParams} instance to be copied.
     */
    public MatchParams(MatchParams params) {
        setEtherParams(params.getEtherParams());
        setInet4Params(params.getInet4Params());
        setLayer4Params(params.getLayer4Params());
    }

    /**
     * Return the parameters for Ethernet header.
     *
     * @return  An {@link EtherMatchParams} instance.
     */
    public final EtherMatchParams getEtherParams() {
        return etherParams;
    }

    /**
     * Set the parameters for Ethernet header.
     *
     * @param params  An {@link EtherMatchParams} instance.
     * @return  This instance.
     */
    public final MatchParams setEtherParams(EtherMatchParams params) {
        etherParams = (params == null) ? null : params.clone();
        return this;
    }

    /**
     * Return the parameters for IPv4 header.
     *
     * @return  An {@link Inet4MatchParams} instance.
     */
    public final Inet4MatchParams getInet4Params() {
        return inet4Params;
    }

    /**
     * Set the parameters for IPv4 header.
     *
     * @param params  An {@link Inet4MatchParams} instance.
     * @return  This instance.
     */
    public final MatchParams setInet4Params(Inet4MatchParams params) {
        inet4Params = (params == null) ? null : params.clone();
        return this;
    }

    /**
     * Return the parameters for layer 4 header.
     *
     * @return  A {@link Layer4MatchParams} instance.
     */
    public final Layer4MatchParams getLayer4Params() {
        return layer4Params;
    }

    /**
     * Set the parameters for layer 4 header.
     *
     * @param params  A {@link Layer4MatchParams} instance.
     * @return  This instance.
     */
    public final MatchParams setLayer4Params(Layer4MatchParams params) {
        layer4Params = (params == null) ? null : params.clone();
        return this;
    }

    /**
     * Return an {@link EthernetMatch} instance.
     *
     * @return  An {@link EthernetMatch} instance.
     */
    public final EthernetMatch getEthernetMatch() {
        return (etherParams == null) ? null : etherParams.toEthernetMatch();
    }

    /**
     * Return an {@link InetMatch} instance.
     *
     * @return  An {@link InetMatch} instance.
     */
    public final InetMatch getInetMatch() {
        return (inet4Params == null) ? null : inet4Params.toInet4Match();
    }

    /**
     * Return an {@link L4Match} instance.
     *
     * @return  An {@link L4Match} instance.
     */
    public final L4Match getL4Match() {
        return (layer4Params == null) ? null : layer4Params.toL4Match();
    }

    /**
     * Return a {@link VtnEtherMatch} instance.
     *
     * @return  A {@link VtnEtherMatch} instance.
     */
    public final VtnEtherMatch getVtnEtherMatch() {
        return (etherParams == null) ? null : etherParams.toVtnEtherMatch();
    }

    /**
     * Return a {@link VtnInetMatch} instance.
     *
     * @return  A {@link VtnInetMatch} instance.
     */
    public final VtnInetMatch getVtnInetMatch() {
        return getVtnInetMatch(false);
    }

    /**
     * Return a {@link VtnInetMatch} instance.
     *
     * @param noZero  Avoid zero prefix if {@code true}.
     * @return  A {@link VtnInetMatch} instance.
     */
    public final VtnInetMatch getVtnInetMatch(boolean noZero) {
        return (inet4Params == null)
            ? null : inet4Params.toVtnInetMatch(noZero);
    }

    /**
     * Return a {@link VtnLayer4Match} instance.
     *
     * @return  A {@link VtnLayer4Match} instance.
     */
    public final VtnLayer4Match getVtnLayer4Match() {
        return getVtnLayer4Match(false);
    }

    /**
     * Return a {@link VtnLayer4Match} instance.
     *
     * @param comp  Complete the settings if {@code true}.
     * @return  A {@link VtnLayer4Match} instance.
     */
    public final VtnLayer4Match getVtnLayer4Match(boolean comp) {
        return (layer4Params == null)
            ? null : layer4Params.toVtnLayer4Match(comp);
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public MatchParams reset() {
        etherParams = null;
        inet4Params = null;
        layer4Params = null;
        return this;
    }

    /**
     * Construct a new {@link FlowMatch} instance.
     *
     * @return  A {@link FlowMatch} instance.
     */
    public FlowMatch toFlowMatch() {
        return new FlowMatch(getEthernetMatch(), getInetMatch(),
                             getL4Match());
    }

    /**
     * Create a {@link VtnFlowMatchBuilder} instance that contains the settings
     * configured in this instance.
     *
     * @return  A {@link VtnFlowMatchBuilder} instance.
     */
    public VtnFlowMatchBuilder toVtnFlowMatchBuilder() {
        return new VtnFlowMatchBuilder().
            setVtnEtherMatch(getVtnEtherMatch()).
            setVtnInetMatch(getVtnInetMatch()).
            setVtnLayer4Match(getVtnLayer4Match());
    }

    /**
     * Create a {@link DataFlowMatch} instance that contains the settings
     * configured in this instance.
     *
     * @return  A {@link DataFlowMatch} instance.
     */
    public DataFlowMatch toDataFlowMatch() {
        return new DataFlowMatchBuilder().
            setVtnEtherMatch(getVtnEtherMatch()).
            setVtnInetMatch(getVtnInetMatch(true)).
            setVtnLayer4Match(getVtnLayer4Match(true)).
            build();
    }

    /**
     * Construct a new {@link VTNMatch} instance.
     *
     * @return  A {@link VTNMatch} instance.
     * @throws Exception  An error occurred.
     */
    public final VTNMatch toVTNMatch() throws Exception {
        VTNMatch vmatch = new VTNMatch();
        vmatch.set(toFlowMatch());
        return vmatch;
    }

    /**
     * Return a {@link XmlNode} instance which represents this instance.
     *
     * @param name  The name of the root node.
     * @return  A {@link XmlNode} instance.
     */
    public XmlNode toXmlNode(String name) {
        XmlNode root = new XmlNode(name);
        if (etherParams != null) {
            root.add(etherParams.toXmlNode("vtn-ether-match"));
        }

        if (inet4Params != null) {
            root.add(inet4Params.toXmlNode("vtn-inet4-match"));
        }

        if (layer4Params != null) {
            root.add(layer4Params.toXmlNode());
        }

        return root;
    }

    /**
     * Ensure that the given {@link VTNMatch} instance contains the same
     * conditions as this instance.
     *
     * @param vmatch  A {@link VTNMatch} instance.
     * @throws Exception  An error occurred.
     */
    public void verify(VTNMatch vmatch) throws Exception {
        FlowMatch fm = vmatch.toFlowMatch();
        assertEquals(toFlowMatch(), fm);
        assertEquals(toDataFlowMatch(),
                     vmatch.toDataFlowMatchBuilder().build());

        VTNEtherMatch vether = vmatch.getEtherMatch();
        MatchBuilder mb = vmatch.toMatchBuilder();
        if (vether == null) {
            if (etherParams != null) {
                assertEquals(true, etherParams.isEmpty());
            }
            assertEquals(null, mb.getEthernetMatch());
            assertEquals(null, mb.getVlanMatch());
        } else {
            etherParams.verifyValues(vether);
            etherParams.verify(mb);
        }

        VTNInetMatch vinet = vmatch.getInetMatch();
        if (vinet == null) {
            if (inet4Params != null) {
                assertEquals(true, inet4Params.isEmpty());
            }
            assertEquals(null, mb.getIpMatch());
            assertEquals(null, mb.getLayer3Match());
        } else {
            assertTrue(vinet instanceof VTNInet4Match);
            inet4Params.verifyValues((VTNInet4Match)vinet);
            inet4Params.verify(mb);
        }

        VTNLayer4Match vl4 = vmatch.getLayer4Match();
        if (vl4 == null) {
            if (layer4Params != null) {
                assertEquals(true, layer4Params.isEmpty());
            }
            assertEquals(null, mb.getIcmpv4Match());
            assertEquals(null, mb.getLayer4Match());
        } else {
            layer4Params.verifyValues(vl4);
            layer4Params.verify(mb);
        }

        if (VTNMatch.class.equals(vmatch.getClass())) {
            assertEquals(vmatch, new VTNMatch(vether, vinet, vl4));
        }

        assertEquals(null, mb.getInPort());
        assertEquals(null, mb.getInPhyPort());
        assertEquals(null, mb.getMetadata());
        assertEquals(null, mb.getTunnel());
        assertEquals(null, mb.getIcmpv6Match());
        assertEquals(null, mb.getProtocolMatchFields());
        assertEquals(null, mb.getTcpFlagMatch());

        VTNMatch vmatch1 = new VTNMatch(mb.build());
        vether = vmatch1.getEtherMatch();
        vinet = vmatch1.getInetMatch();
        vl4 = vmatch1.getLayer4Match();

        if (vether == null) {
            assertEquals(null, etherParams);
        } else {
            etherParams.verifyValues(vether);
        }

        if (vinet != null) {
            assertTrue(vinet instanceof VTNInet4Match);
            inet4Params.verifyValues((VTNInet4Match)vinet);
        } else if (inet4Params != null) {
            assertEquals(true, inet4Params.isEmpty());
        }

        if (vl4 != null) {
            layer4Params.verifyMd(vl4);
        } else if (layer4Params != null) {
            assertEquals(true, layer4Params.isEmpty());
        }
    }

    // PacketHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public final EtherHeader getEtherHeader() {
        return etherParams;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final InetHeader getInetHeader() {
        return inet4Params;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final Layer4Header getLayer4Header() {
        return layer4Params;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final String getHeaderDescription() {
        return "";
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public MatchParams clone() {
        try {
            MatchParams params = (MatchParams)super.clone();
            if (etherParams != null) {
                params.etherParams = etherParams.clone();
            }
            if (inet4Params != null) {
                params.inet4Params = inet4Params.clone();
            }
            if (layer4Params != null) {
                params.layer4Params = layer4Params.clone();
            }

            return params;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
