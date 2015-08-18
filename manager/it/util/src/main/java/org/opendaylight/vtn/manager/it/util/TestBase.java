/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import java.math.BigInteger;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Assert;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.util.packet.ArpFactory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * Abstract base class for integration tests using JUnit.
 */
public abstract class TestBase extends Assert {
    /**
     * The number of milliseconds to wait for OSGi service.
     */
    public static final long  OSGI_TIMEOUT = 120000L;

    /**
     * The number of milliseconds to sleep for short delay.
     */
    public static final long  SHORT_DELAY = 100L;

    /**
     * The number of milliseconds to wait for background tasks to complete.
     */
    public static final long  BGTASK_DELAY = 3000L;

    /**
     * The number of seconds to wait for completion of asynchronous task.
     */
    public static final long  TASK_TIMEOUT = 10L;

    /**
     * Separator in a MD-SAL node or node connector identifier.
     */
    public static final String  ID_SEPARATOR = OfMockService.ID_SEPARATOR;

    /**
     * Protocol prefix for OpenFlow switch.
     */
    public static final String  ID_OPENFLOW = OfMockService.ID_OPENFLOW;

    /**
     * The symbolic name of the manager.implementation bundle.
     */
    public static final String  BUNDLE_VTN_MANAGER_IMPL =
        "org.opendaylight.vtn.manager.implementation";

    /**
     * Broadcast MAC address.
     */
    public static final byte[]  MAC_BROADCAST = {
        (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff, (byte)0xff,
    };

    /**
     * Zero MAC address.
     */
    public static final byte[]  MAC_ZERO = {0, 0, 0, 0, 0, 0};

    /**
     * Dummy MAC address.
     */
    public static final byte[]  MAC_DUMMY = {
        (byte)0x00, (byte)0xde, (byte)0xad, (byte)0xbe, (byte)0xef, (byte)0x11,
    };

    /**
     * Zero IPv4 address.
     */
    public static final byte[]  IPV4_ZERO = {0, 0, 0, 0};

    /**
     * IPv4 address used to send ARP request.
     */
    public static final byte[]  IPV4_DUMMY = {
        (byte)10, (byte)20, (byte)30, (byte)40,
    };

    /**
     * Conversion cache for {@link #toAdNode(String)}.
     */
    private static final Map<String, Node>  TO_AD_NODE_CACHE = new HashMap<>();

    /**
     * Conversion cache for {@link #toAdNodeConnector(String)}.
     */
    private static final Map<String, NodeConnector>  TO_AD_NC_CACHE =
        new HashMap<>();

    /**
     * Conversion cache for {@link #toPortIdentifier(NodeConnector)}.
     */
    private static final Map<NodeConnector, String>  TO_PORT_ID_CACHE =
        new HashMap<>();

    /**
     * Throw an error which indicates the test code should never reach here.
     */
    public static void unexpected() {
        fail("Should never reach here.");
    }

    /**
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    public static void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
    }

    /**
     * Block the calling thread for the specified number of milliseconds.
     *
     * @param millis  The number of milliseconds to sleep.
     */
    public static void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
        }
    }

    /**
     * Return the OSGi bundle instance specified by the symbolic name.
     *
     * @param bc    A {@link BundleContext} instance.
     * @param name  The symbolic name of the OSGi bundle.
     * @return  {@link Bundle} instance if found.
     *          {@code null} if not found.
     */
    public static Bundle getBundle(BundleContext bc, String name) {
        for (Bundle b: bc.getBundles()) {
            if (name.equals(b.getSymbolicName())) {
                return b;
            }
        }

        return null;
    }

    /**
     * Return the OSGi bundle instance associated with the
     * manager.implementation bundle.
     *
     * @param bc    A {@link BundleContext} instance.
     * @return  {@link Bundle} instance if found.
     *          {@code null} if not found.
     */
    public static Bundle getManagerBundle(BundleContext bc) {
        return getBundle(bc, BUNDLE_VTN_MANAGER_IMPL);
    }

    /**
     * Create a list of boolean values and a {@code null}.
     *
     * @return A list of boolean values.
     */
    public static List<Boolean> createBooleans() {
        return createBooleans(true);
    }

    /**
     * Create a list of boolean values.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of boolean values.
     */
    public static List<Boolean> createBooleans(boolean setNull) {
        ArrayList<Boolean> list = new ArrayList<Boolean>();
        if (setNull) {
            list.add(null);
        }

        list.add(Boolean.TRUE);
        list.add(Boolean.FALSE);
        return list;
    }

    /**
     * Create a list of {@link Node} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link Node}.
     */
    public static List<Node> createNodes(int num) {
        return createNodes(num, true);
    }

    /**
     * Create a list of {@link Node}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link Node}.
     */
    public static List<Node> createNodes(int num, boolean setNull) {
        int n = num;
        ArrayList<Node> list = new ArrayList<Node>();
        if (setNull) {
            list.add(null);
            n--;
        }

        String[] types = {
            Node.NodeIDType.OPENFLOW,
            Node.NodeIDType.ONEPK,
            Node.NodeIDType.PRODUCTION,
        };

        for (int i = 0; i < n; i++) {
            int tidx = i % types.length;
            String type = types[tidx];
            Object id;
            if (type.equals(Node.NodeIDType.OPENFLOW)) {
                id = Long.valueOf((long)i);
            } else {
                id = "Node ID: " + i;
            }

            try {
                Node node = new Node(type, id);
                assertNotNull(node.getType());
                assertNotNull(node.getNodeIDString());
                list.add(node);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a {@link VTenantConfig} object.
     *
     * @param desc  Description of the virtual tenant.
     * @param idle  {@code idle_timeout} value for flow entries.
     * @param hard  {@code hard_timeout} value for flow entries.
     * @return  A {@link VBridgeConfig} object.
     */
    public static VTenantConfig createVTenantConfig(String desc, Integer idle,
                                                    Integer hard) {
        if (idle == null) {
            if (hard == null) {
                return new VTenantConfig(desc);
            } else {
                return new VTenantConfig(desc, -1, hard.intValue());
            }
        } else if (hard == null) {
            return new VTenantConfig(desc, idle.intValue(), -1);
        }

        return new VTenantConfig(desc, idle.intValue(), hard.intValue());
    }

    /**
     * Create a {@link VBridgeConfig} object.
     *
     * @param desc  Description of the virtual bridge.
     * @param age  {@code age} value for aging interval for MAC address table.
     * @return  A {@link VBridgeConfig} object.
     */
    public static VBridgeConfig createVBridgeConfig(String desc, Integer age) {
        if (age == null) {
            return new VBridgeConfig(desc);
        } else {
            return new VBridgeConfig(desc, age);
        }
    }

    /**
     * Learn the given host mapped to the given bridge network.
     *
     * @param ofmock    openflowplugin mock-up service.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must
     *                  be associated with the key.
     * @param host      A test host to learn.
     * @throws Exception  An error occurred.
     */
    public static void learnHost(OfMockService ofmock,
                                 Map<String, Set<Short>> allPorts,
                                 TestHost host) throws Exception {
        String ingress = host.getPortIdentifier();
        byte[] sha = host.getMacAddress();
        byte[] spa = host.getRawInetAddress();
        short vlan = host.getVlan();
        EthernetFactory efc = new EthernetFactory(sha, MAC_BROADCAST).
            setVlanId(vlan);
        ArpFactory afc = ArpFactory.newInstance(efc);
        afc.setSenderHardwareAddress(sha).
            setTargetHardwareAddress(MAC_ZERO).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(IPV4_DUMMY);
        byte[] payload = efc.create();
        ofmock.sendPacketIn(ingress, payload);

        for (Map.Entry<String, Set<Short>> entry: allPorts.entrySet()) {
            Set<Short> vids = entry.getValue();
            if (vids == null) {
                continue;
            }

            String portId = entry.getKey();
            if (portId.equals(ingress)) {
                vids = new HashSet<Short>(vids);
                assertTrue(vids.remove(vlan));
                if (vids.isEmpty()) {
                    continue;
                }
            }

            int count = vids.size();
            for (int i = 0; i < count; i++) {
                byte[] transmitted = ofmock.awaitTransmittedPacket(portId);
                vids = efc.verify(ofmock, transmitted, vids);
            }
            assertTrue(vids.isEmpty());
        }

        for (String pid: allPorts.keySet()) {
            assertNull(ofmock.getTransmittedPacket(pid));
        }
    }


    /**
     * Send a broadcast ARP packet.
     *
     * @param ofmock   openflowplugin mock-up service.
     * @param mac      The source MAC address.
     * @param ip       The source IPv4 address.
     * @param vlan     VLAN ID for a ARP packet.
     *                 Zero or a negative value indicates untagged.
     * @param ingress  The MD-SAL node connector identifier which specifies
     *                 the ingress port.
     * @return  A raw byte image of the ARP packet.
     * @throws Exception  An error occurred.
     */
    public static byte[] sendBroadcast(OfMockService ofmock, byte[] mac,
                                       byte[] ip, short vlan, String ingress)
        throws Exception {
        EthernetFactory efc = new EthernetFactory(mac, MAC_BROADCAST).
            setVlanId(vlan);
        ArpFactory afc = ArpFactory.newInstance(efc);
        afc.setSenderHardwareAddress(mac).
            setTargetHardwareAddress(MAC_ZERO).
            setSenderProtocolAddress(ip).
            setTargetProtocolAddress(IPV4_DUMMY);
        byte[] payload = efc.create();
        ofmock.sendPacketIn(ingress, payload);
        return payload;
    }

    /**
     * Send a broadcast ARP packet.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param host    The source host of the ARP packet.
     * @return  A raw byte image ot the ARP packet.
     * @throws Exception  An error occurred.
     */
    public static byte[] sendBroadcast(OfMockService ofmock, TestHost host)
        throws Exception {
        return sendBroadcast(ofmock, host.getMacAddress(),
                             host.getRawInetAddress(), host.getVlan(),
                             host.getPortIdentifier());
    }

    /**
     * Send a broadcast ARP packet.
     *
     * @param ofmock  openflowplugin mock-up service.
     * @param ment    The source host of the ARP packet.
     * @return  A raw byte image ot the ARP packet.
     * @throws Exception  An error occurred.
     */
    public static byte[] sendBroadcast(OfMockService ofmock,
                                       MacAddressEntry ment) throws Exception {
        DataLinkAddress dladdr = ment.getAddress();
        assertTrue(dladdr instanceof EthernetAddress);
        EthernetAddress eaddr = (EthernetAddress)dladdr;

        Set<InetAddress> ipaddrs = ment.getInetAddresses();
        assertFalse(ipaddrs.isEmpty());
        InetAddress ipaddr = ipaddrs.iterator().next();
        assertTrue(ipaddr instanceof Inet4Address);

        String pid = toPortIdentifier(ment.getNodeConnector());

        return sendBroadcast(ofmock, eaddr.getValue(), ipaddr.getAddress(),
                             ment.getVlan(), pid);
    }

    /**
     * Convert a MD-SAL node or node connector identifier into a {@link Node}
     * instance.
     *
     * @param id  A MD-SAL node or node connector identifier.
     *            An OpenFlow node or node connector identifier must be
     *            specified.
     * @return    A {@link Node} instance.
     */
    public static Node toAdNode(String id) {
        Node node = TO_AD_NODE_CACHE.get(id);
        if (node != null) {
            return node;
        }

        if (!id.startsWith(ID_OPENFLOW)) {
            throw invalidMdNodeId(id, null);
        }

        try {
            int from = ID_OPENFLOW.length();
            int to = id.indexOf(ID_SEPARATOR.charAt(0), from);
            String idstr = (to < 0)
                ? id.substring(from)
                : id.substring(from, to);
            Long dpid = Long.valueOf(new BigInteger(idstr).longValue());

            node = new Node(Node.NodeIDType.OPENFLOW, dpid);
            TO_AD_NODE_CACHE.put(id, node);
            return node;
        } catch (Exception e) {
            throw invalidMdNodeId(id, e);
        }
    }

    /**
     * Convert a MD-SAL node connector identifier into a {@link NodeConnector}
     * instance.
     *
     * @param id  A MD-SAL node connector identifier.
     *            An OpenFlow node connector identifier must be specified.
     * @return    A {@link NodeConnector} instance.
     */
    public static NodeConnector toAdNodeConnector(String id) {
        NodeConnector nc = TO_AD_NC_CACHE.get(id);
        if (nc != null) {
            return nc;
        }

        if (!id.startsWith(ID_OPENFLOW)) {
            throw invalidMdNodeId(id, null);
        }

        try {
            int from = ID_OPENFLOW.length();
            int to = id.indexOf(ID_SEPARATOR.charAt(0), from);
            String idstr = id.substring(from, to);
            Long dpid = Long.valueOf(new BigInteger(idstr).longValue());

            Node node = new Node(Node.NodeIDType.OPENFLOW, dpid);
            Short portId = Short.valueOf(id.substring(to + 1));
            String type = NodeConnector.NodeConnectorIDType.OPENFLOW;
            nc = new NodeConnector(type, portId, node);
            TO_AD_NC_CACHE.put(id, nc);
            return nc;
        } catch (Exception e) {
            throw invalidMdNodeId(id, e);
        }
    }

    /**
     * Convert a AD-SAL node connector identifier into a MD-SAL node connector
     * identifier.
     *
     * @param nc  A {@link NodeConnector} instance.
     * @return    MD-SAL node connector identifier.
     */
    public static String toPortIdentifier(NodeConnector nc) {
        String pid = TO_PORT_ID_CACHE.get(nc);
        if (pid != null) {
            return pid;
        }

        if (!NodeConnector.NodeConnectorIDType.OPENFLOW.equals(nc.getType())) {
            throw new IllegalArgumentException(
                "Invalid AD-SAL node connector: " + nc);
        }

        Node node = nc.getNode();
        StringBuilder builder = new StringBuilder(ID_OPENFLOW).
            append(node.getID()).append(ID_SEPARATOR).
            append(nc.getNodeConnectorIDString());
        pid = builder.toString();
        TO_PORT_ID_CACHE.put(nc, pid);
        return pid;
    }

    /**
     * Eliminate MAC address table entries associated with the given port.
     *
     * @param hosts  A set of {@link MacAddressEntry}.
     *               This method never modifies this set.
     * @param pid    The MD-SAL port identifier.
     * @return  A new set of {@link MacAddressEntry}.
     */
    public static Set<MacAddressEntry> filterOut(Set<MacAddressEntry> hosts,
                                                 String pid) {
        Set<MacAddressEntry> newSet = new HashSet<>();
        for (MacAddressEntry ment: hosts) {
            NodeConnector nc = ment.getNodeConnector();
            if (!pid.equals(toPortIdentifier(nc))) {
                assertTrue(newSet.add(ment));
            }
        }

        return newSet;
    }

    /**
     * Convert the given long number into a hex string with ":" inserted.
     *
     * @param value  A long number.
     * @return  A hex string.
     */
    public static String toHexString(long value) {
        String sep = "";
        StringBuilder builder = new StringBuilder();
        for (int nshift = Long.SIZE - Byte.SIZE; nshift >= 0;
             nshift -= Byte.SIZE) {
            int octet = (int)(value >>> nshift) & NumberUtils.MASK_BYTE;
            builder.append(sep).append(String.format("%02x", octet));
            sep = ByteUtils.HEX_SEPARATOR;
        }

        return builder.toString();
    }

    /**
     * Convert a hex string with ":" inserted into a long number.
     *
     * @param hex  A hex string.
     * @return  A long number.
     */
    public static long toLong(String hex) {
        String str = hex.replaceAll(ByteUtils.HEX_SEPARATOR, "");
        BigInteger bi = new BigInteger(str, ByteUtils.HEX_RADIX);
        return bi.longValue();
    }

    /**
     * Return an exception that indicates an invalid MD-SAL node or
     * node connector is specified.
     *
     * @param id     A MD-SAL node or node connector identifier.
     * @param cause  The cause of error.
     * @return  An {@link IllegalArgumentException} instance.
     */
    private static IllegalArgumentException invalidMdNodeId(String id,
                                                            Throwable cause) {
        String msg = "Illegal MD-SAL node/node-connector identifier: " + id;
        throw new IllegalArgumentException(msg, cause);
    }
}
