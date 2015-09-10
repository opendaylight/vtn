/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag.MISSING_ELEMENT;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.lang.reflect.Field;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.UnmarshalException;
import javax.xml.bind.Unmarshaller;
import javax.xml.bind.annotation.XmlRootElement;

import org.junit.After;
import org.junit.Assert;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.PassFilter;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.vtn.manager.internal.cluster.MacMapPath;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.MapType;
import org.opendaylight.vtn.manager.internal.cluster.VTenantImpl;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;
import org.opendaylight.vtn.manager.internal.packet.PacketInEvent;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;

import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.core.UpdateType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketReceivedBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * Abstract base class for JUnit tests.
 */
public abstract class TestBase extends Assert {
    /**
     * XML declaration.
     */
    public static final String  XML_DECLARATION =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";

    /**
     * String Declaration for Mac Unavilable.
     */
    protected static final String MAC_UNAVAILABLE =
        "MAC mapping is unavailable: ";

    /**
     * String Declaration for Mac Used .
     */
    protected static final String MAC_USED =
        "Same MAC address is already mapped: host={";

    /**
     * String Declaration for Mac Inactivated .
     */
    protected static final String MAC_INACTIVATED =
        "MAC mapping has been inactivated: host={";

    /**
     * String Declaration for Switch Reserved .
     */

    protected static final String SWITCH_RESERVED =
        "switch port is reserved: host={";
     /**
     * An array of index values with positive and negative values.
     */

    public static final int[] INDEX_ARRAY = {3, 0, 65536};

     /**
     * An array of Tenant values with valid and invalid values.
     */
    public static final String[] TENANT_NAME = {"tenant", "", null};

    /**
     * An array of Cost values with valid and invalid values.
     */
    public static final long[] COST = {0L, 333L, Long.MAX_VALUE};

    /**
     * An array of FlowCondition names with valid and invalid values.
     */
    public static final String[] FLOW_CONDITION_NAME = {"flowCondition", "", null};

    /**
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    protected static void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
    }

    /**
     * Throw an error which indicates the test code should never reach here.
     */
    protected static void unexpected() {
        fail("Should never reach here.");
    }

    /**
     * Create a copy of the specified string.
     *
     * @param str  A string to be copied.
     * @return     A copied string.
     */
    protected static String copy(String str) {
        if (str != null) {
            str = new String(str);
        }
        return str;
    }

    /**
     * Create a copy of the specified integer.
     *
     * @param i   An integer to be copied.
     * @return  A copied integer.
     */
    protected static Integer copy(Integer i) {
        if (i != null) {
            i = new Integer(i.intValue());
        }
        return i;
    }

    /**
     * Create a copy of the specified {@link Long} object.
     *
     * @param num  An {@link Long} object to be copied.
     * @return     A copied boolean object.
     */
    protected static Long copy(Long num) {
        if (num != null) {
            num = new Long(num.longValue());
        }
        return num;
    }

    /**
     * Create a copy of the specified {@link Node}.
     *
     * @param node  A {@link Node} object to be copied.
     * @return      A copied {@link Node} object.
     */
    protected static Node copy(Node node) {
        if (node != null) {
            try {
                node = new Node(node);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return node;
    }

    /**
     * Create a copy of the specified {@link NodeConnector}.
     *
     * @param nc  A {@link NodeConnector} object to be copied.
     * @return    A copied {@link NodeConnector} object.
     */
    protected static NodeConnector copy(NodeConnector nc) {
        if (nc != null) {
            try {
                nc = new NodeConnector(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return nc;
    }

    /**
     * Create a copy of the specified {@link EthernetAddress}.
     *
     * @param ea    A {@link EthernetAddress} object to be copied.
     * @return      A copied {@link EthernetAddress} object.
     */
    protected static EthernetAddress copy(EthernetAddress ea) {
        if (ea != null) {
            try {
                ea = new EthernetAddress(ea.getValue());
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return ea;
    }

    /**
     * Create a copy of the specified {@link InetAddress}.
     *
     * @param ia    A {@link InetAddress} object to be copied.
     * @return      A copied {@link InetAddress} object.
     */
    protected static InetAddress copy(InetAddress ia) {
        if (ia != null) {
            try {
                ia = InetAddress.getByAddress(ia.getAddress());
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return ia;
    }

    /**
     * Create a deep copy of the specified {@link InetAddress} set.
     *
     * @param ia    A {@link InetAddress} set to be copied.
     * @return      A copied {@link InetAddress} set.
     */
    protected static Set<InetAddress> copy(Set<InetAddress> ia) {
        if (ia != null) {
            Set<InetAddress> newset = new HashSet<InetAddress>();
            try {
                for (InetAddress iaddr : ia) {
                    newset.add(InetAddress.getByAddress(iaddr.getAddress()));
                }
            } catch (Exception e) {
                unexpected(e);
            }
            ia = newset;
        }
        return ia;
    }

    /**
     * Create a deep copy of the specified {@link VTenantPath} object.
     *
     * @param path  A {@link VTenantPath} object to be copied.
     * @return  A copied {@link VTenantPath} object.
     */
    protected static VTenantPath copy(VTenantPath path) {
        if (path != null) {
            String tname = copy(path.getTenantName());
            if (path instanceof VBridgePath) {
                VBridgePath bpath = (VBridgePath)path;
                String bname = copy(bpath.getBridgeName());
                bpath = new VBridgePath(tname, bname);

                if (path instanceof VBridgeIfPath) {
                    VBridgeIfPath ipath = (VBridgeIfPath)path;
                    String iname = copy(ipath.getInterfaceName());
                    path = new VBridgeIfPath(tname, bname, iname);
                } else if (path instanceof VlanMapPath) {
                    VlanMapPath vpath = (VlanMapPath)path;
                    String mapId = copy(vpath.getMapId());
                    path = new VlanMapPath(bpath, mapId);
                } else if (path instanceof MacMapPath) {
                    path = new MacMapPath(bpath);
                } else {
                    path = bpath;
                }
            } else if (path instanceof VTerminalPath) {
                VTerminalPath vtpath = (VTerminalPath)path;
                String vtname = copy(vtpath.getTerminalName());
                vtpath = new VTerminalPath(tname, vtname);

                if (path instanceof VTerminalIfPath) {
                    VTerminalIfPath ipath = (VTerminalIfPath)path;
                    String iname = copy(ipath.getInterfaceName());
                    path = new VTerminalIfPath(vtpath, iname);
                } else {
                    path = vtpath;
                }
            }
        }
        return path;
    }

    /**
     * Create a deep copy of the specified {@link MapReference} object.
     *
     * @param ref  A {@link MapReference} object to be copied.
     * @return  A copied {@link MapReference} object.
     */
    protected static MapReference copy(MapReference ref) {
        if (ref != null) {
            MapType type = ref.getMapType();
            String cname = copy(ref.getContainerName());
            VNodePath path = (VNodePath)copy(ref.getPath());
            ref = new MapReference(type, cname, path);
        }

        return ref;
    }

    /**
     * Create a deep copy of the specified {@link Packet} instance.
     *
     * @param src  The source {@link Packet} instance.
     * @param dst  The destination {@link Packet} instance.
     * @param <T>  Type of the packet.
     * @return  {@code dst}.
     */
    protected static <T extends Packet> T copy(T src, T dst) {
        if (src != null) {
            try {
                byte[] raw = src.serialize();
                int nbits = raw.length * Byte.SIZE;
                dst.deserialize(raw, 0, nbits);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return dst;
    }

    /**
     * Create a list of boolean values and a {@code null}.
     *
     * @return A list of boolean values.
     */
    protected static List<Boolean> createBooleans() {
        return createBooleans(true);
    }

    /**
     * Create a list of boolean values.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of boolean values.
     */
    protected static List<Boolean> createBooleans(boolean setNull) {
        ArrayList<Boolean> list = new ArrayList<Boolean>();
        if (setNull) {
            list.add(null);
        }

        list.add(Boolean.TRUE);
        list.add(Boolean.FALSE);
        return list;
    }

    /**
     * Create a list of strings and a {@code null}.
     *
     * @param base A base string.
     * @return A list of strings.
     */
    protected static List<String> createStrings(String base) {
        return createStrings(base, true);
    }

    /**
     * Create a list of strings.
     *
     * @param base     A base string.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of strings.
     */
    protected static List<String> createStrings(String base, boolean setNull) {
        ArrayList<String> list = new ArrayList<String>();
        if (setNull) {
            list.add(null);
        }

        for (int i = 0; i <= base.length(); i++) {
            list.add(base.substring(0, i));
        }

        return list;
    }

    /**
     * Create a list of Short objects and a {@code null}.
     *
     * @param start    The first value.
     * @param count    The number of Shorts to be created.
     * @return A list of {@link Short}.
     */
    protected static List<Short> createShorts(short start, short count) {
        return createShorts(start, count, true);
    }

    /**
     * Create a list of Short objects.
     *
     * @param start    The first value.
     * @param count    The number of Shorts to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link Short}.
     */
    protected static List<Short> createShorts(short start, short count,
                                              boolean setNull) {
        List<Short> list = new ArrayList<Short>();
        if (count > 0) {
            if (setNull) {
                list.add(null);
                count--;
            }

            for (short i = 0; i < count; i++, start++) {
                list.add(Short.valueOf(start));
            }
        }

        return list;
    }


    /**
     * Create a list of integer objects and a {@code null}.
     *
     * @param start    The first value.
     * @param count    The number of integers to be created.
     * @return A list of {@link Integer}.
     */
    protected static List<Integer> createIntegers(int start, int count) {
        return createIntegers(start, count, true);
    }

    /**
     * Create a list of integer objects.
     *
     * @param start    The first value.
     * @param count    The number of integers to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link Integer}.
     */
    protected static List<Integer> createIntegers(int start, int count,
                                                  boolean setNull) {
        ArrayList<Integer> list = new ArrayList<Integer>();
        if (count > 0) {
            if (setNull) {
                list.add(null);
                count--;
            }

            for (int i = 0; i < count; i++, start++) {
                list.add(new Integer(start));
            }
        }

        return list;
    }

    /**
     * Create a list of {@link Node} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link Node}.
     */
    protected static List<Node> createNodes(int num) {
        return createNodes(num, true);
    }

    /**
     * Create a list of {@link Node}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link Node}.
     */
    protected static List<Node> createNodes(int num, boolean setNull) {
        ArrayList<Node> list = new ArrayList<Node>();
        if (setNull) {
            list.add(null);
            num--;
        }

        String[] types = {
            Node.NodeIDType.OPENFLOW,
            Node.NodeIDType.ONEPK,
            Node.NodeIDType.PRODUCTION,
        };

        for (int i = 0; i < num; i++) {
            int tidx = i % types.length;
            String type = types[tidx];
            Object id;
            if (type.equals(Node.NodeIDType.OPENFLOW)) {
                id = new Long((long)i);
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
     * Create a list of {@link NodeConnector} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link NodeConnector}.
     */
    protected static List<NodeConnector> createNodeConnectors(int num) {
        return createNodeConnectors(num, true);
    }

    /**
     * Create a list of {@link NodeConnector}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link NodeConnector}.
     */
    protected static List<NodeConnector> createNodeConnectors(
        int num, boolean setNull) {
        ArrayList<NodeConnector> list = new ArrayList<NodeConnector>();
        if (setNull) {
            list.add(null);
            num--;
        }

        String[] nodeTypes = {
            Node.NodeIDType.OPENFLOW,
            Node.NodeIDType.ONEPK,
            Node.NodeIDType.PRODUCTION,
        };
        String[] connTypes = {
            NodeConnector.NodeConnectorIDType.OPENFLOW,
            NodeConnector.NodeConnectorIDType.ONEPK,
            NodeConnector.NodeConnectorIDType.PRODUCTION,
        };

        for (int i = 0; i < num; i++) {
            int tidx = i % nodeTypes.length;
            String nodeType = nodeTypes[tidx];
            String connType = connTypes[tidx];
            Object nodeId, connId;
            if (nodeType.equals(Node.NodeIDType.OPENFLOW)) {
                nodeId = new Long((long)i);
                connId = new Short((short)(i + 10));
            } else {
                nodeId = "Node ID: " + i;
                connId = "Node Connector ID: " + i;
            }

            try {
                Node node = new Node(nodeType, nodeId);
                assertNotNull(node.getType());
                assertNotNull(node.getNodeIDString());
                NodeConnector nc = new NodeConnector(connType, connId, node);
                assertNotNull(nc.getType());
                assertNotNull(nc.getNodeConnectorIDString());
                list.add(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a list of unique edges.
     *
     * @param num  The number of edges to be created.
     * @return  A list of unique edges.
     */
    protected static List<Edge> createEdges(int num) {
        List<Edge> list = new ArrayList<Edge>();
        NodeConnector src = null;
        for (NodeConnector nc: createNodeConnectors(num << 1, false)) {
            if (src == null) {
                src = nc;
            } else {
                try {
                    Edge edge = new Edge(src, nc);
                    list.add(edge);
                    src = null;
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }

        return list;
    }

    /**
     * Create a list of {@link SwitchPort} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link SwitchPort}.
     */
    protected static List<SwitchPort> createSwitchPorts(int num) {
        return createSwitchPorts(num, true);
    }

    /**
     * Create a list of {@link SwitchPort}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link SwitchPort}.
     */
    protected static List<SwitchPort> createSwitchPorts(int num,
                                                        boolean setNull) {
        ArrayList<SwitchPort> list = new ArrayList<SwitchPort>();
        if (setNull) {
            list.add(null);
            num--;
            if (num == 0) {
                return list;
            }
        }

        list.add(new SwitchPort(null, null, null));
        num--;
        if (num == 0) {
            return list;
        }

        list.add(new SwitchPort("name", "type", "id"));
        num--;

        for (; num > 0; num--) {
            String name = ((num % 2) == 0) ? null : "name:" + num;
            String type = ((num % 7) == 0) ? null : "type:" + num;
            String id = ((num % 9) == 0) ? null : "id:" + num;
            if (name == null && type == null && id == null) {
                name = "name:" + num;
            }
            list.add(new SwitchPort(name, type, id));
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
    protected static VTenantConfig createVTenantConfig(String desc,
            Integer idle, Integer hard) {
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
     * @param age   {@code age} value for aging interval for MAC address table.
     * @return  A {@link VBridgeConfig} object.
     */
    protected static VBridgeConfig createVBridgeConfig(String desc, Integer age) {
        if (age == null) {
            return new VBridgeConfig(desc);
        } else {
            return new VBridgeConfig(desc, age);
        }
    }

    /**
     * Create a list of {@link EthernetAddress} and a {@code null}.
     *
     * @return A list of {@link EthernetAddress}.
     */
    protected static List<EthernetAddress> createEthernetAddresses() {
        return createEthernetAddresses(true);
    }

    /**
     * Create a list of {@link EthernetAddress}.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link EthernetAddress}.
     */
    protected static List<EthernetAddress> createEthernetAddresses(
        boolean setNull) {
        List<EthernetAddress> list = new ArrayList<EthernetAddress>();
        byte [][] addrbytes = {
            new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                        (byte)0x00, (byte)0x00, (byte)0x01},
            new byte[] {(byte)0x12, (byte)0x34, (byte)0x56,
                        (byte)0x78, (byte)0x9a, (byte)0xbc},
            new byte[] {(byte)0xfe, (byte)0xdc, (byte)0xba,
                        (byte)0x98, (byte)0x76, (byte)0x54}
        };

        if (setNull) {
            list.add(null);
        }

        for (byte[] addr : addrbytes) {
            try {
                EthernetAddress ea;
                ea = new EthernetAddress(addr);
                list.add(ea);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a {@link EthernetAddress} instance which represents the
     * MAC address specified by a long integer value.
     *
     * @param mac  A long integer value which represents the MAC address.
     * @return  A {@link EthernetAddress} instance.
     *          {@code null} is returned if -1 is specified to {@code mac}.
     */
    protected static EthernetAddress createEthernetAddress(long mac) {
        if (mac == -1L) {
            return null;
        }

        byte[] addr = EtherAddress.toBytes(mac);
        EthernetAddress eaddr = null;
        try {
            eaddr = new EthernetAddress(addr);
        } catch (Exception e) {
            unexpected(e);
        }

        return eaddr;
    }

    /**
     * Create a {@link EthernetAddress} instance which represents the
     * MAC address specified by a byte array.
     *
     * @param addr  A byte array which represents the MAC address.
     * @return  A {@link EthernetAddress} instance.
     *          {@code null} is returned if {@code null} is specified to
     *          {@code mac}.
     */
    protected static EthernetAddress createEthernetAddress(byte[] addr) {
        if (addr == null) {
            return null;
        }

        EthernetAddress eaddr = null;
        try {
            eaddr = new EthernetAddress(addr);
        } catch (Exception e) {
            unexpected(e);
        }

        return eaddr;
    }

    /**
     * Create a list of {@link InetAddress} set and a {@code null}.
     *
     * @return A list of {@link InetAddress} set.
     */
    protected static List<Set<InetAddress>> createInetAddresses() {
        return createInetAddresses(true);
    }

    /**
     * Create a list of {@link InetAddress} set.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link InetAddress} set.
     */
    protected static List<Set<InetAddress>> createInetAddresses(boolean setNull) {
        List<Set<InetAddress>> list = new ArrayList<Set<InetAddress>>();
        String[][] arrays = {
            {"0.0.0.0"},
            {"255.255.255.255", "10.1.2.3"},
            {"0.0.0.0", "10.0.0.1", "192.255.255.1",
             "2001:420:281:1004:e123:e688:d655:a1b0"},
            {},
        };

        if (setNull) {
            list.add(null);
        }

        for (String[] array : arrays) {
            Set<InetAddress> iset = new HashSet<InetAddress>();
            try {
                for (String addr : array) {
                    assertTrue(iset.add(InetAddress.getByName(addr)));
                }
                list.add(iset);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return list;
    }

    /**
     * Create a list of IPv4 addresses and {@code null}.
     *
     * @return  A list of {@link InetAddress} instances.
     */
    protected static List<InetAddress> createInet4Addresses() {
        return createInet4Addresses(true);
    }

    /**
     * Create a list of IPv4 addresses.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return  A list of {@link InetAddress} instances.
     */
    protected static List<InetAddress> createInet4Addresses(boolean setNull) {
        List<InetAddress> list = new ArrayList<InetAddress>();
        if (setNull) {
            list.add(null);
        }

        String[] addrs = {
            "0.0.0.0",
            "255.255.255.255",
            "127.0.0.1",
            "10.20.30.40",
            "192.168.100.200",
        };
        for (String addr: addrs) {
            try {
                list.add(InetAddress.getByName(addr));
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a {@link InetAddress} instance which represents the specified
     * IP address.
     *
     * @param addr  A raw IP address.
     * @return  A {@link InetAddress} instance.
     */
    protected static InetAddress createInetAddress(byte[] addr) {
        InetAddress iaddr = null;
        try {
            iaddr = InetAddress.getByAddress(addr);
        } catch (Exception e) {
            unexpected(e);
        }

        return iaddr;
    }

    /**
     * Create a list of {@link VBridgePath} objects.
     *
     * @return  A list of {@link VBridgePath} objects.
     */
    protected static List<VBridgePath> createVBridgePaths() {
        List<VBridgePath> list = new ArrayList<VBridgePath>();
        for (int tidx = 0; tidx < 2; tidx++) {
            String tname = "tenant" + tidx;
            for (int bidx = 0; bidx < 2; bidx++) {
                VBridgePath path = new VBridgePath(tname, "bridge" + bidx);
                list.add(path);
            }
        }

        return list;
    }

    /**
     * Create a list of {@link VBridgePath} objects.
     *
     * @param subclass  If {@code true} is specified, instances of classes
     *                  which extend {@link VBridgePath} are added to the
     *                  returned list.
     * @return  A list of {@link VBridgePath} objects.
     */
    protected static List<VBridgePath> createVBridgePaths(boolean subclass) {
        if (!subclass) {
            return createVBridgePaths();
        }

        List<VBridgePath> list = new ArrayList<VBridgePath>();
        String[] bnames = {"bridge1", "bridge2"};
        String[] inames = {"if_1", "if_2"};
        for (String tname: new String[]{"tenant1", "tenant2"}) {
            for (String bname: bnames) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                list.add(bpath);

                for (String iname: inames) {
                    list.add(new VBridgeIfPath(tname, bname, iname));
                    list.add(new VlanMapPath(bpath, iname));
                }
            }
        }

        return list;
    }

    /**
     * create a {@link PacketContext} object.
     *
     * @param eth   A {@link Ethernet} object.
     * @param nc    A incoming node connector.
     * @return A {@link PacketContext} object.
     */
    protected PacketContext createPacketContext(Ethernet eth, NodeConnector nc) {
        PacketReceivedBuilder builder = new PacketReceivedBuilder();
        try {
            builder.setPayload(eth.serialize());
            SalPort ingress = SalPort.create(nc);
            assertNotNull(ingress);
            PacketInEvent pin =
                new PacketInEvent(null, builder.build(), ingress);
            return new PacketContext(pin);
        } catch (Exception e) {
            unexpected(e);
        }

        return null;
    }

    /**
     * Create a {@link PacketContext} object for unicast packet transmission.
     *
     * @param eth   A {@link Ethernet} object.
     * @param nc    A incoming node connector.
     * @return A {@link PacketContext} object.
     */
    protected PacketContext createUnicastPacketContext(Ethernet eth,
                                                       NodeConnector nc) {
        PacketContext pctx = createPacketContext(eth, nc);
        pctx.addUnicastMatchFields();
        return pctx;
    }

    /**
     * Create an Ethernet frame.
     *
     * @param src      Source MAC address.
     * @param dst      Destination MAC address.
     * @param type     Ethernet type.
     * @param vid      A VLAN ID.
     * @param pcp      VLAN priority.
     * @param payload  A {@link Packet} instance to be set as payload.
     * @return  An {@link Ethernet} instance.
     */
    protected static Ethernet createEthernet(byte[] src, byte[] dst, int type,
                                             short vid, byte pcp,
                                             Packet payload) {
        Ethernet pkt = new Ethernet();
        pkt.setSourceMACAddress(src).setDestinationMACAddress(dst);
        if (vid == MatchType.DL_VLAN_NONE) {
            pkt.setEtherType((short)type).setPayload(payload);
        } else {
            IEEE8021Q tag = new IEEE8021Q();
            tag.setCfi((byte)0).setPcp(pcp).setVid(vid).
                setEtherType((short)type).setPayload(payload);
            pkt.setEtherType(EtherTypes.VLANTAGGED.shortValue()).
                setPayload(tag);
        }

        return pkt;
    }

    /**
     * Create an Ethernet frame.
     *
     * @param src   Source MAC address.
     * @param dst   Destination MAC address.
     * @param type  Ethernet type.
     * @param vid   A VLAN ID.
     * @param pcp   VLAN priority
     * @param raw   A byte array to be set as payload.
     * @return  An {@link Ethernet} instance.
     */
    protected static Ethernet createEthernet(byte[] src, byte[] dst, int type,
                                             short vid, byte pcp, byte[] raw) {
        Ethernet pkt = new Ethernet();
        pkt.setSourceMACAddress(src).setDestinationMACAddress(dst);
        if (vid == MatchType.DL_VLAN_NONE) {
            pkt.setEtherType((short)type).setRawPayload(raw);
        } else {
            IEEE8021Q tag = new IEEE8021Q();
            tag.setCfi((byte)0).setPcp(pcp).setVid(vid).
                setEtherType((short)type).setRawPayload(raw);
            pkt.setEtherType(EtherTypes.VLANTAGGED.shortValue()).
                setPayload(tag);
        }

        return pkt;
    }

    /**
     * Create an untagged Ethernet frame that contains the given IPv4 packet.
     *
     * <p>
     *   Fixed values are used for the source and destination MAC address.
     * </p>
     *
     * @param ipv4  An {@link IPv4} to be configured as payload.
     * @return  An {@link Ethernet} instance.
     */
    protected static Ethernet createEthernet(IPv4 ipv4) {
        byte[] src = {
            (byte)0x00, (byte)0x11, (byte)0x22,
            (byte)0x33, (byte)0x44, (byte)0x55,
        };
        byte[] dst = {
            (byte)0xa0, (byte)0xb0, (byte)0xc0,
            (byte)0xdd, (byte)0xee, (byte)0xff,
        };

        return createEthernet(src, dst, EtherTypes.IPv4.intValue(),
                              MatchType.DL_VLAN_NONE, (byte)0, ipv4);
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * @param src    Source IP address.
     * @param dst    Destination IP address.
     * @param proto  IP protocol number.
     * @param dscp   DSCP field value.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(InetAddress src, InetAddress dst,
                                     short proto, byte dscp) {
        IPv4 pkt = new IPv4();
        return pkt.setSourceAddress(src).setDestinationAddress(dst).
            setProtocol((byte)proto).setDiffServ(dscp).
            setTtl((byte)64);
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * @param src    A byte array which represents the source IP address.
     * @param dst    A byte array which represents the destination IP address.
     * @param proto  IP protocol number.
     * @param dscp   DSCP field value.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(byte[] src, byte[] dst, short proto,
                                     byte dscp) {
        InetAddress srcInet = createInetAddress(src);
        InetAddress dstInet = createInetAddress(dst);
        return createIPv4(srcInet, dstInet, proto, dscp);
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * @param src    An integer value which represents the source IP address.
     * @param dst    An integer value which represents the destination IP
     *               address.
     * @param proto  IP protocol number.
     * @param dscp   DSCP field value.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(int src, int dst, short proto,
                                     byte dscp) {
        byte[] srcAddr = NumberUtils.toBytes(src);
        byte[] dstAddr = NumberUtils.toBytes(dst);
        return createIPv4(srcAddr, dstAddr, proto, dscp);
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * @param src      A byte array which represents the source IP address.
     * @param dst      A byte array which represents the destination IP address.
     * @param proto    IP protocol number.
     * @param dscp     DSCP field value.
     * @param payload  A {@link Packet} instance to be configured as payload.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(byte[] src, byte[] dst, short proto,
                                     byte dscp, Packet payload) {
        IPv4 pkt = createIPv4(src, dst, proto, dscp);
        pkt.setPayload(payload);

        return pkt;
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * <p>
     *   Fixed values are used for the source and destination IP address,
     *   and DSCP field.
     * </p>
     *
     * @param proto    IP protocol number.
     * @param payload  A {@link Packet} instance to be configured as payload.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(short proto, Packet payload) {
        byte[] src = {(byte)10, (byte)1, (byte)2, (byte)30};
        byte[] dst = {(byte)127, (byte)0, (byte)0, (byte)1};
        return createIPv4(src, dst, proto, (byte)0, payload);
    }

    /**
     * create a {@link RawPacket} object.
     *
     * @param eth   A {@link Ethernet} object.
     * @param nc    A incoming node connector.
     * @return A {@link RawPacket} object.
     */
    protected RawPacket createRawPacket(Ethernet eth, NodeConnector nc) {
        RawPacket raw = null;
        try {
            raw = new RawPacket(eth.serialize());
        } catch (Exception e) {
            unexpected(e);
        }
        raw.setIncomingNodeConnector(copy(nc));

        return raw;
    }

    /**
     * create a {@link Ethernet} object of IPv4 Packet.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify vlan ID. if vlan < 0, vlan tag is not added.
     * @return  A {@link Ethernet} object.
     */
    protected Ethernet createIPv4Packet(byte[] src, byte[] dst, byte[] sender,
            byte[] target, short vlan) {

        IPv4 ip = new IPv4();
        ip.setVersion((byte)4).
            setIdentification((short)5).
            setDiffServ((byte)0).
            setECN((byte)0).
            setTotalLength((short)84).
            setFlags((byte)2).
            setFragmentOffset((short)0).
            setTtl((byte)64);

        ip.setDestinationAddress(createInetAddress(target));
        ip.setSourceAddress(createInetAddress(sender));

        Ethernet eth = new Ethernet();
        eth.setSourceMACAddress(src).setDestinationMACAddress(dst);

        if (vlan >= 0) {
            eth.setEtherType(EtherTypes.VLANTAGGED.shortValue());

            IEEE8021Q vlantag = new IEEE8021Q();
            vlantag.setCfi((byte)0x0).setPcp((byte)0x0).setVid((short)vlan).
                setEtherType(EtherTypes.IPv4.shortValue()).setParent(eth);
            eth.setPayload(vlantag);

            vlantag.setPayload(ip);

        } else {
            eth.setEtherType(EtherTypes.IPv4.shortValue()).setPayload(ip);
        }
        return eth;
    }

    /**
     * create a {@link Ethernet} object of ARP Packet.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify vlan ID. if vlan < 0, vlan tag is not added.
     * @param arptype   ARP.REQUEST or ARP.REPLY. (ARP Reply is not implemented yet )
     * @return  A {@link Ethernet object.}
     */
    protected Ethernet createARPPacket(byte[] src, byte[] dst, byte[] sender,
            byte[] target, short vlan, short arptype) {
        ARP arp = new ARP();
        arp.setHardwareType(ARP.HW_TYPE_ETHERNET).
            setProtocolType(EtherTypes.IPv4.shortValue()).
            setHardwareAddressLength((byte)EthernetAddress.SIZE).
            setProtocolAddressLength((byte)target.length).
            setOpCode(arptype).
            setSenderHardwareAddress(src).setSenderProtocolAddress(sender).
            setTargetHardwareAddress(dst).setTargetProtocolAddress(target);

        Ethernet eth = new Ethernet();
        eth.setSourceMACAddress(src).setDestinationMACAddress(dst);

        if (vlan >= 0) {
            eth.setEtherType(EtherTypes.VLANTAGGED.shortValue());

            IEEE8021Q vlantag = new IEEE8021Q();
            vlantag.setCfi((byte)0x0).setPcp((byte)0x0).setVid(vlan)
                    .setEtherType(EtherTypes.ARP.shortValue()).setParent(eth);
            eth.setPayload(vlantag);

            vlantag.setPayload(arp);

        } else {
            eth.setEtherType(EtherTypes.ARP.shortValue()).setPayload(arp);
        }
        return eth;
    }

    /**
     * create a {@link PacketContext} object of ARP Request.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify vlan ID. if vlan < 0, vlan tag is not added.
     * @param nc        A node connector
     * @param arptype   ARP.REQUEST or ARP.REPLY. (ARP Reply is not implemented yet )
     * @return  A {@link PacketContext} object.
     */
    protected PacketContext createARPPacketContext(byte[] src, byte[] dst,
            byte[] sender, byte[] target, short vlan, NodeConnector nc,
            short arptype) {
        return createUnicastPacketContext(
                createARPPacket(src, dst, sender, target, vlan, arptype), nc);
    }

    /**
     * create a {@link PacketContext} object of IPv4 packet.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify vlan ID. if vlan < 0, vlan tag is not added.
     * @param nc        A node connector
     * @return  A {@link PacketContext} object.
     */
    protected PacketContext createIPv4PacketContext(byte[] src, byte[] dst,
            byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        return createUnicastPacketContext(
                createIPv4Packet(src, dst, sender, target, vlan), nc);
    }

    /**
     * create a {@link RawPacket} object of ARP Request.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify val ID. if vlan < 0, vlan tag is not added.
     * @param nc        A node connector
     * @param arptype   ARP.REQUEST or ARP.REPLY. (ARP Reply is not implemented yet )
     * @return  A {@link PacketContext} object.
     */
    protected RawPacket createARPRawPacket(byte[] src, byte[] dst,
            byte[] sender, byte[] target, short vlan, NodeConnector nc,
            short arptype) {
        return createRawPacket(
                createARPPacket(src, dst, sender, target, vlan, arptype), nc);
    }

    /**
     * create a {@link RawPacket} object of IPv4 packet.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vlan      specify vlan ID. if vlan < 0, vlan tag is not added.
     * @param nc        A node connector
     * @return  A {@link PacketContext} object.
     */
    protected RawPacket createIPv4RawPacket(byte[] src, byte[] dst,
            byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        return createRawPacket(
                createIPv4Packet(src, dst, sender, target, vlan), nc);
    }

    /**
     * Create a {@link EthernetHost} instance which represents the
     * specified MAC address and VLAN ID.
     *
     * @param mac   A long integer value which represents the MAC address.
     * @param vlan  A VLAN ID.
     * @return  A {@link EthernetHost} instance.
     */
    protected static EthernetHost createEthernetHost(long mac, short vlan) {
        EthernetAddress eaddr = createEthernetAddress(mac);
        return new EthernetHost(eaddr, vlan);
    }

    /**
     * Create a list of set of {@link DataLinkHost} instances.
     *
     * @param limit  The number of ethernet addresses to be created.
     * @return  A list of set of {@link DataLinkHost} instances.
     */
    protected static List<Set<DataLinkHost>> createDataLinkHostSet(int limit) {
        return createDataLinkHostSet(limit, true);
    }

    /**
     * Create a list of set of {@link DataLinkHost} instances.
     *
     * @param limit  The number of ethernet addresses to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return  A list of set of {@link DataLinkHost} instances.
     */
    protected static List<Set<DataLinkHost>> createDataLinkHostSet(
        int limit, boolean setNull) {
        List<Set<DataLinkHost>> list = new ArrayList<Set<DataLinkHost>>();
        if (setNull) {
            list.add(null);
        }

        HashSet<DataLinkHost> set = new HashSet<DataLinkHost>();
        short[] vlans = {0, 4095};
        for (short vlan: vlans) {
            for (EthernetAddress ether: createEthernetAddresses()) {
                set.add(new EthernetHost(ether, vlan));
                list.add(new HashSet<DataLinkHost>(set));
                if (list.size() >= limit) {
                    return list;
                }
            }
        }

        for (int i = 0; i < 2; i++) {
            TestDataLink dladdr = new TestDataLink("addr" + i);
            set.add(new TestDataLinkHost(dladdr));
            list.add(new HashSet<DataLinkHost>(set));
            if (list.size() >= limit) {
                break;
            }
        }

        return list;
    }

    /**
     * Determine whether the specified two strings are identical or not.
     *
     * <p>
     *   This method returns {@code true} if both strings are {@code null}.
     * </p>
     *
     * @param str1  A string to be tested.
     * @param str2  A string to be tested.
     * @return  {@code true} only if thw two strings are identical.
     */
    protected static boolean equals(String str1, String str2) {
        if (str1 == null) {
            return (str2 == null);
        }

        return str1.equals(str2);
    }

    /**
     * Join the separated strings with inserting a separator.
     *
     * @param prefix     A string to be prepended to the string.
     * @param suffix     A string to be appended to the string.
     * @param separator  A separator string.
     * @param args       An array of objects. Note that {@code null} in this
     *                   array is always ignored.
     */
    protected static String joinStrings(String prefix, String suffix,
            String separator, Object... args) {
        StringBuilder builder = new StringBuilder();
        if (prefix != null) {
            builder.append(prefix);
        }

        boolean first = true;
        for (Object o : args) {
            if (o != null) {
                if (first) {
                    first = false;
                } else {
                    builder.append(separator);
                }
                builder.append(o.toString());
            }
        }

        if (suffix != null) {
            builder.append(suffix);
        }

        return builder.toString();
    }

    /**
     * Ensure that the given two objects are identical.
     *
     * @param set  A set of tested objects.
     * @param o1   An object to be tested.
     * @param o2   An object to be tested.
     */
    protected static void testEquals(Set<Object> set, Object o1, Object o2) {
        assertEquals(o1, o2);
        assertEquals(o2, o1);
        assertEquals(o1, o1);
        assertEquals(o2, o2);
        assertEquals(o1.hashCode(), o2.hashCode());
        assertFalse(o1.equals(null));
        assertFalse(o1.equals(new Object()));
        assertFalse(o1.equals("string"));
        assertFalse(o1.equals(set));

        for (Object o : set) {
            assertFalse("o1=" + o1 + ", o=" + o, o1.equals(o));
            assertFalse(o.equals(o1));
        }

        assertTrue(set.add(o1));
        assertFalse(set.add(o1));
        assertFalse(set.add(o2));
    }

    /**
     * Ensure that an instance of {@link VTenantPath} is comparable.
     *
     * @param set  A set of {@link VTenantPath} variants.
     */
    protected static void comparableTest(Set<VTenantPath> set) {
        TreeSet<VTenantPath> treeSet = new TreeSet<VTenantPath>(set);

        VTenantPath empty = new VTenantPath(null);
        boolean added = set.add(empty);
        assertEquals(added, treeSet.add(empty));
        assertEquals(set, treeSet);

        // The first element in the set must be a VTenantPath instance with
        // null name.
        Iterator<VTenantPath> it = treeSet.iterator();
        assertTrue(it.hasNext());
        VTenantPath prev = it.next();
        assertEquals(VTenantPath.class, prev.getClass());
        assertNull(prev.getTenantName());

        Class<?> prevClass = VTenantPath.class;
        HashSet<Class<?>> classSet = new HashSet<Class<?>>();
        ArrayList<String> prevComponens = new ArrayList<String>();
        prevComponens.add(null);

        while (it.hasNext()) {
            VTenantPath path = it.next();
            assertTrue(prev.compareTo(path) < 0);
            assertFalse(prev.equals(path));

            ArrayList<String> components = new ArrayList<String>();
            components.add(path.getTenantName());
            if (path instanceof VNodePath) {
                components.add(((VNodePath)path).getTenantNodeName());
            }
            if (path instanceof VBridgeIfPath) {
                components.add(((VBridgeIfPath)path).getInterfaceName());
            } else if (path instanceof VlanMapPath) {
                components.add(((VlanMapPath)path).getMapId());
            }

            int prevSize = prevComponens.size();
            int compSize = components.size();
            Class<?> cls = path.getClass();
            boolean classChanged = false;
            if (prevSize == compSize) {
                if (cls.equals(prevClass)) {
                    checkPathOrder(prevComponens, components);
                } else {
                    String name = cls.getName();
                    String prevName = prevClass.getName();
                    assertTrue("name=" + name + ", prevName=" + prevName,
                               prevName.compareTo(name) < 0);
                    classChanged = true;
                }
            } else {
                assertTrue(prevSize < compSize);
                classChanged = true;
            }

            if (classChanged) {
                assertTrue(classSet.add(cls));
                prevClass = cls;
            }

            prevComponens = components;
            prev = path;
        }
    }

    /**
     * Verify the order of the path components.
     *
     * @param lesser    A path components that should be less than
     *                  {@code greater}.
     * @param greater   A path components to be compared.
     */
    private static void checkPathOrder(List<String> lesser,
                                       List<String> greater) {
        for (int i = 0; i < lesser.size(); i++) {
            String l = lesser.get(i);
            String g = greater.get(i);
            if (l == null) {
                return;
            }
            assertNotNull(g);
            int ret = l.compareTo(g);
            if (ret != 0) {
                assertTrue(ret < 0);
                return;
            }
        }

        fail("Identical: lesser=" + lesser + ", greater=" + greater);
    }

    /**
     * Ensure that the given object is serializable.
     *
     * @param o  An object to be tested.
     * @return  A deserialized object is returned.
     */
    protected static Object serializeTest(Object o) {
        Object newobj = serializeAndDeserialize(o);

        if (o instanceof Enum) {
            assertSame(o, newobj);
        } else {
            assertNotSame(o, newobj);
            assertEquals(o, newobj);
        }

        return newobj;
    }

    /**
     * Ensure that the given object is mapped to XML root element.
     *
     * @param o         An object to be tested.
     * @param cls       The type of the given object.
     * @param rootName  The name of expected root element.
     * @param <T>       The type of the given object.
     * @return  Deserialized object.
     */
    protected static <T> T jaxbTest(T o, Class<T> cls, String rootName) {
        // Ensure that the class of the given class has XmlRootElement
        // annotation.
        XmlRootElement xmlRoot = cls.getAnnotation(XmlRootElement.class);
        assertNotNull(xmlRoot);
        assertEquals(rootName, xmlRoot.name());

        // Marshal the given object into XML.
        Marshaller m = createMarshaller(cls);
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        try {
            m.marshal(o, out);
        } catch (Exception e) {
            unexpected(e);
        }

        byte[] bytes = out.toByteArray();
        assertTrue(bytes.length != 0);

        // Construct a new Java object from XML.
        ByteArrayInputStream in = new ByteArrayInputStream(bytes);
        Unmarshaller um = createUnmarshaller(cls);
        Object newobj = null;
        try {
            newobj = um.unmarshal(in);
        } catch (Exception e) {
            unexpected(e);
        }

        assertNotSame(o, newobj);
        assertEquals(o, newobj);

        assertTrue(cls.isInstance(newobj));
        return cls.cast(newobj);
    }

    /**
     * Create JAXB marshaller for the given JAXB class.
     *
     * @param cls  A class mapped to XML root element.
     * @return  An JAXB marshaller.
     */
    protected static Marshaller createMarshaller(Class<?> cls) {
        try {
            JAXBContext jc = JAXBContext.newInstance(cls);
            Marshaller m = jc.createMarshaller();
            m.setEventHandler(new TestXmlEventHandler());
            return m;
        } catch (Exception e) {
            unexpected(e);
            return null;
        }
    }

    /**
     * Create JAXB unmarshaller for the given JAXB class.
     *
     * @param cls  A class mapped to XML root element.
     * @return  An JAXB unmarshaller.
     */
    protected static Unmarshaller createUnmarshaller(Class<?> cls) {
        try {
            JAXBContext jc = JAXBContext.newInstance(cls);
            Unmarshaller um = jc.createUnmarshaller();
            um.setEventHandler(new TestXmlEventHandler());
            return um;
        } catch (Exception e) {
            unexpected(e);
            return null;
        }
    }

    /**
     * Unmarshal the given XML using the given unmarshaller.
     *
     * @param um   An {@link Unmarshaller} instance.
     * @param xml  A XML text.
     * @param cls  A class which indicates the type of object.
     * @param <T>  The type of the object to be deserialized.
     * @return  The deserialized object.
     * @throws JAXBException  Failed to unmarshal.
     */
    protected static <T> T unmarshal(Unmarshaller um, String xml,
                                     Class<T> cls) throws JAXBException {
        ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
        Object o = um.unmarshal(in);
        assertTrue(cls.isInstance(o));
        return cls.cast(o);
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param type    A class which indicates the type of object.
     * @param dtypes  An array of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Class<T> type,
                                            XmlDataType ... dtypes) {
        jaxbErrorTest(createUnmarshaller(type), type, dtypes);
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param type    A class which indicates the type of object.
     * @param dtypes  A list of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Class<T> type,
                                            List<XmlDataType> dtypes) {
        jaxbErrorTest(createUnmarshaller(type), type, dtypes);
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param um      An {@link Unmarshaller} instance.
     * @param type    A class which indicates the type of object.
     * @param dtypes  An array of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Unmarshaller um, Class<T> type,
                                            XmlDataType ... dtypes) {
        for (XmlDataType dtype: dtypes) {
            jaxbErrorTest(um, type, dtype);
        }
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param um      An {@link Unmarshaller} instance.
     * @param type    A class which indicates the type of object.
     * @param dtypes  A list of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Unmarshaller um, Class<T> type,
                                            List<XmlDataType> dtypes) {
        for (XmlDataType dtype: dtypes) {
            jaxbErrorTest(um, type, dtype);
        }
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param um     An {@link Unmarshaller} instance.
     * @param type   A class which indicates the type of object.
     * @param dtype  A {@link XmlDataType} instance that creates invalid XML
     *               text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Unmarshaller um, Class<T> type,
                                            XmlDataType dtype) {
        for (XmlNode xn: dtype.createInvalidNodes()) {
            try {
                unmarshal(um, xn.toString(), type);
                fail("Broken XML has been unmarshalled: " + xn);
            } catch (UnmarshalException e) {
                Throwable rootCause = null;
                Throwable cause = e.getCause();
                if (cause != null) {
                    while (true) {
                        if (cause instanceof IllegalArgumentException) {
                            return;
                        }
                        Throwable c = cause.getCause();
                        if (c == null) {
                            rootCause = cause;
                            break;
                        }
                        cause = c;
                    }
                }
                if (!(rootCause instanceof IllegalArgumentException)) {
                    fail("Unexpected exception: " + e + ", cause=" +
                         rootCause + ", xml=" + xn);
                }
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Ensure that the given object is serializable.
     *
     * <p>
     *   Note: This method doesn't check if deserialized object equals
     *   input object.
     * </p>
     *
     * @param o  An object to be tested.
     * @return  A deserialized object is returned.
     */
    protected static Object eventSerializeTest(Object o) {
        Object newobj = serializeAndDeserialize(o);
        assertNotSame(o, newobj);

        return newobj;
    }

    /**
     * Serialize and deserialize object.
     *
     * @param o An {@link Object} serialized and deserialized.
     * @return A deserialized object.
     */
    private static Object serializeAndDeserialize(Object o) {
        // Serialize the given object.
        byte[] bytes = null;
        try {
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            ObjectOutputStream out = new ObjectOutputStream(bout);

            out.writeObject(o);
            out.close();
            bytes = bout.toByteArray();
        } catch (Exception e) {
            unexpected(e);
        }
        assertTrue(bytes.length != 0);

        // Deserialize the object.
        Object newobj = null;
        try {
            ByteArrayInputStream bin = new ByteArrayInputStream(bytes);
            ObjectInputStream in = new ObjectInputStream(bin);
            newobj = in.readObject();
            in.close();
        } catch (Exception e) {
            unexpected(e);
        }
        return newobj;
    }

    /**
     * setup a startup directory
     */
    protected static void setupStartupDir() {
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        boolean result = confdir.exists();
        if (!result) {
            result = confdir.mkdirs();
        } else {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }
        }
    }

    /**
     * cleanup a startup directory
     */
    protected static void cleanupStartupDir() {
        String currdir = new File(".").getAbsoluteFile().getParent();
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());

        if (confdir.exists()) {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }

            while (confdir != null && confdir.getAbsolutePath() != currdir) {
                confdir.delete();
                String pname = confdir.getParent();
                if (pname == null) {
                    break;
                }
                confdir = new File(pname);
            }
        }
    }

    /**
     * Delete the specified directory recursively.
     *
     * @param file  A {@link File} instance which represents a file or
     *              directory to be removed.
     */
    protected static void delete(File file) {
        if (!file.exists()) {
            return;
        }

        File[] files = file.listFiles();
        if (files == null) {
            // Delete the specified file.
            file.delete();
            return;
        }

        // Make the specified directory empty.
        for (File f: files) {
            delete(f);
        }

        // Delete the specified directory.
        file.delete();
    }

    /**
     * Return a path to the default container configuration directory.
     *
     * @return  A {@link File} instance which represents the default container
     *          configuration directory.
     */
    protected static File getConfigDir() {
        return getConfigDir(GlobalConstants.DEFAULT.toString());
    }

    /**
     * Return a path to the container configuration directory.
     *
     * @param container  The name of the container.
     * @return  A {@link File} instance which represents the container
     *          configuration directory.
     */
    protected static File getConfigDir(String container) {
        File dir = new File(GlobalConstants.STARTUPHOME.toString(), container);
        return new File(dir, "vtn");
    }

    /**
     * Return a path to the tenant configuration directory.
     *
     * @param container  The name of the container.
     * @return  A {@link File} instance which represents the tenant
     *          configuration directory.
     */
    protected static File getTenantConfigDir(String container) {
        File dir = getConfigDir(container);
        return new File(dir, ContainerConfig.Type.TENANT.toString());
    }

    /**
     * Delete configuration directory after test.
     */
    @After
    public void deleteStartUpHome() {
        String dir = GlobalConstants.STARTUPHOME.toString();
        File file = new File(dir).getParentFile();
        delete(file);
    }

    /**
     * check a Ethernet packet whether expected parameters are set.
     *
     * @param msg       if check is failed, report error with a this string.
     * @param eth       An input ethernet frame data.
     * @param ethType   expected ethernet type.
     * @param destMac   expected destination mac address.
     * @param srcMac    expected source mac address.
     * @param vlan      expected vlan id. (if expected untagged, specify 0 or less than 0)
     * @param protoType expected protocol type.
     * @param opCode    expected opCode. if this is not ARP, opCode is not checked.
     * @param senderMac expected sender HW address.
     * @param targetMac expected target HW address.
     * @param senderAddr    expected sender protocol address.
     * @param targetAddr    expected target protocol address.
     *
     */
    protected void checkOutEthernetPacket(String msg, Ethernet eth,
                                          EtherTypes ethType, byte[] srcMac,
                                          byte[] destMac, short vlan,
                                          EtherTypes protoType, short opCode,
                                          byte[] senderMac, byte[] targetMac,
                                          byte[] senderAddr,
                                          byte[] targetAddr) {
        ARP arp = null;
        if (vlan > 0) {
            assertEquals(msg, EtherTypes.VLANTAGGED.shortValue(),
                         eth.getEtherType());
            IEEE8021Q vlantag = (IEEE8021Q)eth.getPayload();
            assertEquals(msg, vlan, vlantag.getVid());
            assertEquals(msg, ethType.shortValue(), vlantag.getEtherType());
            if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
                arp = (ARP)vlantag.getPayload();
            }
        } else {
            assertEquals(msg, ethType.shortValue(), eth.getEtherType());
            if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
                arp = (ARP)eth.getPayload();
            }
        }

        if (srcMac != null) {
            assertArrayEquals(msg, srcMac, eth.getSourceMACAddress());
        }
        if (destMac != null) {
            assertArrayEquals(msg, destMac, eth.getDestinationMACAddress());
        }

        if (ethType != null
                && ethType.shortValue() == EtherTypes.ARP.shortValue()) {
            assertNotNull(msg, arp);
            if (protoType != null) {
                assertEquals(msg, protoType.shortValue(), arp.getProtocolType());
            }
            if (opCode >= 0) {
                assertEquals(msg, opCode, arp.getOpCode());
            }
            if (senderMac != null) {
                assertArrayEquals(msg, senderMac,
                        arp.getSenderHardwareAddress());
            }
            if (targetMac != null) {
                assertArrayEquals(msg, targetMac,
                        arp.getTargetHardwareAddress());
            }
            if (senderAddr != null) {
                assertArrayEquals(msg, senderAddr,
                        arp.getSenderProtocolAddress());
            }
            if (targetAddr != null) {
                assertArrayEquals(msg, targetAddr,
                        arp.getTargetProtocolAddress());
            }
        }
    }

    /**
     * VtnmanagerImpl  object creation
     * @return VtnmanagerImplList
     */
    public List<VTNManagerImpl> createVtnManagerImplobj() {
        List<VTNManagerImpl> vtnMangerList = new ArrayList<VTNManagerImpl>();
        try {
            vtnMangerList.add(null);
            VTNManagerImpl vtnmangerimpl = new VTNManagerImpl();
            vtnMangerList.add(vtnmangerimpl);
            return vtnMangerList;
        } catch (Exception ex) {
            return vtnMangerList;
        }
    }

    /**
     * Common Method to create UpdateTye.
     *  @return UpdateType list.
     */
    public List<UpdateType> updateTypeList() {
        List<UpdateType> updatetypeList = new ArrayList<UpdateType>();
        updatetypeList.add(null);

        try {
            updatetypeList.add(UpdateType.ADDED);
            updatetypeList.add(UpdateType.REMOVED);
            updatetypeList.add(UpdateType.CHANGED);
            return updatetypeList;
        } catch (Exception ex) {
            return updatetypeList;
        }
    }

    /**
     * Common Method to create FlowFilter.
     *  @return flowfilter list object.
     */
    public List<FlowFilter> createFlowFilter() {
        List<FlowFilter> flowList = new ArrayList<FlowFilter>();
        flowList.add(null);

        try {
            FlowFilter flowfilter = createFlowFilterobj();
            flowList.add(flowfilter);
            flowfilter = null;
            flowList.add(flowfilter);
            return flowList;
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Common Method to create FlowFilter.
     *  @return flowfilter object.
     */
    public FlowFilter createFlowFilterobj() {
        int idx = 3;
        int port = 0;
        String cond = "Flow";
        try {
            List<FlowAction> actionList = new ArrayList<FlowAction>();
            actionList.add(new SetTpSrcAction(port));
            FlowFilter flowfilterobj = new FlowFilter(cond, new PassFilter(), actionList);
            return flowfilterobj;
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Common Method to create VtenantPaths.
     *  @return VTenantPath list object.
     */
    public List<VTenantPath> creatVtenantPaths() {
        List<VTenantPath> pathList = new ArrayList<VTenantPath>();
        try {
            VTenantPath vpath = null;
            pathList.add(null);
            pathList.add(vpath);

            for (String tenantName : TENANT_NAME) {
                vpath = new  VTenantPath(tenantName);
                pathList.add(vpath);
            }
            return pathList;
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Common Method to create VTenantImpl.
     *
     *  @param description      The description of the VTN.
     *  @param vContainerName   The name of the container to which the tenant belongs.
     *  @param tenantName       The name of the tenant.
     *  @return VTenantImpl list object.
     */
    public List<VTenantImpl> createVTenantImpl(String description, String vContainerName, String tenantName) {
        List<VTenantImpl> vTenantImplList = new ArrayList<VTenantImpl>();

        try {
            VTenantImpl vTenantImpl = null;
            vTenantImplList.add(null);
            vTenantImplList.add(vTenantImpl);


            // VTenantConfig.idleTimeout List
            for (Integer idleTimeout: createIntegers(-1, 4, false)) {
                // VTenantConfig.hardTimeout List
                for (Integer hardTimeout: createIntegers(-1, 4, false)) {
                    VTenantConfig tconf = createVTenantConfig(description, idleTimeout, hardTimeout);
                    VTenantImpl vtenantImpl = new VTenantImpl(vContainerName, tenantName, tconf);

                    vTenantImplList.add(vTenantImpl);
                }
            }
            return vTenantImplList;
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * Create lists of {@link PortLocation} instances for test.
     *
     * @return A list of {@link PortLocation} lists.
     */

    public List<PortLocation> createPortLocation() {
        try {

            List<PortLocation> portLocation = new ArrayList<PortLocation>();
            for (NodeConnector nodeConnector : createNodeConnectors(5)) {
                for (String name : createStrings("portName")) {
                    PortLocation locationobj = new PortLocation(nodeConnector, name);
                    portLocation.add(locationobj);
                }
            }
            return portLocation;
        } catch (Exception ex) {
            //ex.printStackTrace();
            return null;
        }
    }

    /**
     * Return the value of the field configured in the given object.
     *
     * @param obj   The target object.
     * @param type  A class which specifies the type of the field value.
     * @param name  The name of the field.
     * @param <T>   The type of the field value.
     * @return  The value of the field configured in the given object.
     * @throws Exception  An error occurred.
     */
    public static <T> T getFieldValue(Object obj, Class<T> type, String name)
        throws Exception {
        return getFieldValue(obj, obj.getClass(), type, name);
    }

    /**
     * Return the value of the field configured in the given object.
     *
     * @param obj      The target object.
     * @param objType  A class which contains the declaration of the specified
     *                 field.
     * @param type     A class which specifies the type of the field value.
     * @param name     The name of the field.
     * @param <T>      The type of the field value.
     * @return  The value of the field configured in the given object.
     * @throws Exception  An error occurred.
     */
    public static <T> T getFieldValue(Object obj, Class<?> objType,
                                      Class<T> type, String name)
        throws Exception {
        Field field = objType.getDeclaredField(name);
        field.setAccessible(true);
        Object value = field.get(obj);

        assertTrue(value == null || type.isInstance(value));
        return type.cast(value);
    }

    /**
     * Ensure that the given future associated with RPC contains an error state
     * caused by a null RPC input.
     *
     * @param future  A future associated with RPC.
     * @param <T>     The type of RPC output.
     * @throws Exception  An error occurred.
     */
    public static <T> void verifyRpcInputNull(Future<RpcResult<T>> future)
        throws Exception {
        verifyRpcFailure(future, MISSING_ELEMENT, "BADREQUEST",
                         "RPC input cannot be null");
    }

    /**
     * Ensure that the given future associated with RPC contains an error
     * state.
     *
     * @param future  A future associated with RPC.
     * @param tag     The expected error tag.
     * @param apTag   The expected application error tag.
     * @param msg     The expected error message.
     * @param <T>     The type of RPC output.
     * @throws Exception  An error occurred.
     */
    public static <T> void verifyRpcFailure(Future<RpcResult<T>> future,
                                            RpcErrorTag tag, String apTag,
                                            String msg) throws Exception {
        RpcResult<T> result = future.get(1L, TimeUnit.SECONDS);
        assertEquals(false, result.isSuccessful());

        Collection<RpcError> errors = result.getErrors();
        assertEquals(1, errors.size());

        RpcError error = errors.iterator().next();
        assertEquals(ErrorType.APPLICATION, error.getErrorType());
        assertEquals(tag.toString(), error.getTag());
        assertEquals(apTag, error.getApplicationTag());
        assertEquals(msg, error.getMessage());
    }

    /**
     * Ensure that the given two {@link StaticSwitchLinks} instances are
     * identical.
     *
     * @param expected  A {@link StaticSwitchLinks} instance which contains
     *                  expected values.
     * @param swlinks   A {@link StaticSwitchLinks} instance to be tested.
     */
    public static void assertEqualsStaticSwitchLinks(
        StaticSwitchLinks expected, StaticSwitchLinks swlinks) {
        if (expected == null) {
            assertEquals(null, swlinks);
            return;
        }

        List<StaticSwitchLink> elinks = expected.getStaticSwitchLink();
        List<StaticSwitchLink> links = swlinks.getStaticSwitchLink();
        Set<StaticSwitchLink> eset = new HashSet<>();
        Set<StaticSwitchLink> lset = new HashSet<>();
        if (elinks != null) {
            eset.addAll(elinks);
        }
        if (links != null) {
            lset.addAll(links);
        }
        assertEquals(eset, lset);
    }

    /**
     * Ensure that the given two {@link StaticEdgePorts} instances are
     * identical.
     *
     * @param expected  A {@link StaticEdgePorts} instance which contains
     *                  expected values.
     * @param edges     A {@link StaticEdgePorts} instance to be tested.
     */
    public static void assertEqualsStaticEdgePorts(StaticEdgePorts expected,
                                                   StaticEdgePorts edges) {
        if (expected == null) {
            assertEquals(null, edges);
            return;
        }

        List<StaticEdgePort> elist = expected.getStaticEdgePort();
        List<StaticEdgePort> list = edges.getStaticEdgePort();
        Set<StaticEdgePort> eset = new HashSet<>();
        Set<StaticEdgePort> lset = new HashSet<>();
        if (elist != null) {
            eset.addAll(elist);
        }
        if (list != null) {
            lset.addAll(list);
        }
        assertEquals(eset, lset);
    }

    /**
     * check a Ethernet packet whether expected parameters are set.
     * (for IPv4 packet)
     *
     * @param msg       if check is failed, report error with a this string.
     * @param eth       input ethernet frame data.
     * @param ethType   expected ethernet type.
     * @param destMac   expected destination mac address.
     * @param srcMac    expected source mac address.
     * @param vlan      expected vlan id. (if expected untagged, specify 0 or less than 0)
     */
    protected void checkOutEthernetPacketIPv4(String msg, Ethernet eth,
                                              EtherTypes ethType,
                                              byte[] srcMac, byte[] destMac,
                                              short vlan) {
        checkOutEthernetPacket(msg, eth, ethType, srcMac, destMac, vlan, null,
                               (short)-1, null, null, null, null);
    }

    /**
     * Create a response of read request on a MD-SAL transaction.
     *
     * @param obj  An object to be read.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the data object.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadResult(
        T obj) {
        Optional<T> opt = Optional.fromNullable(obj);
        return Futures.<Optional<T>, ReadFailedException>
            immediateCheckedFuture(opt);
    }

    /**
     * Create an error response of read request on a MD-SAL transaction.
     *
     * @param type   A class which indicates the type of the return value.
     * @param cause  A throwable which indicates the cause of error.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the return value.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadFailure(
        Class<T> type, Throwable cause) {
        String msg = "DS read failed";
        RpcError err = RpcResultBuilder.newError(
            ErrorType.APPLICATION, "failed", msg, null, null, cause);
        ReadFailedException rfe = new ReadFailedException(msg, cause, err);
        return Futures.<Optional<T>, ReadFailedException>
            immediateFailedCheckedFuture(rfe);
    }

    /**
     * Create a future associated with successful completion of the MD-SAL
     * datastore submit procedure.
     *
     * @return  A {@link CheckedFuture} instance.
     */
    protected static CheckedFuture<Void, TransactionCommitFailedException> getSubmitFuture() {
        ListenableFuture<Void> f = Futures.immediateFuture(null);
        DataStoreExceptionMapper mapper =
            DataStoreExceptionMapper.getInstance();
        return Futures.makeChecked(f, mapper);
    }

    /**
     * Create a future associated with the MD-SAL datastore submit procedure
     * with failure.
     *
     * @param cause  A throwable which indicates the cause of error.
     * @return  A {@link CheckedFuture} instance.
     */
    protected static CheckedFuture<Void, TransactionCommitFailedException> getSubmitFuture(
        Throwable cause) {
        ListenableFuture<Void> f = Futures.immediateFailedFuture(cause);
        DataStoreExceptionMapper mapper =
            DataStoreExceptionMapper.getInstance();
        return Futures.makeChecked(f, mapper);
    }

    /**
     * Create a {@link VtnPortBuilder} instance corresponding to an enabled
     * edge port.
     *
     * @param sport    A {@link SalPort} instance.
     */
    protected static VtnPortBuilder createVtnPortBuilder(SalPort sport) {
        return createVtnPortBuilder(sport, Boolean.TRUE, true);
    }

    /**
     * Create a {@link VtnPortBuilder} instance.
     *
     * @param sport    A {@link SalPort} instance.
     * @param enabled  The state of the port.
     * @param edge     Determine edge port or not.
     */
    protected static VtnPortBuilder createVtnPortBuilder(
        SalPort sport, Boolean enabled, boolean edge) {
        String name = "sw" + sport.getNodeNumber() + "-eth" +
            sport.getPortNumber();
        VtnPortBuilder builder = new VtnPortBuilder();
        builder.setCost(Long.valueOf(1000)).
            setId(sport.getNodeConnectorId()).setName(name).
            setEnabled(enabled);

        if (!edge) {
            PortLinkBuilder pbuilder = new PortLinkBuilder();
            long n = sport.getNodeNumber() + 100L;
            NodeConnectorId peer = new SalPort(n, sport.getPortNumber()).
                getNodeConnectorId();
            pbuilder.setLinkId(new LinkId("isl:" + name)).
                setPeer(peer);
            List<PortLink> list = new ArrayList<>();
            list.add(pbuilder.build());
            builder.setPortLink(list);
        }

        return builder;
    }

    /**
     * Generate tri-state boolean value.
     *
     * @param b  A {@link Boolean} instance.
     * @return
     *   {@link Boolean#TRUE} if {@code b} is {@code null}.
     *   {@link Boolean#FALSE} if {@code b} is {@link Boolean#TRUE}.
     *   {@code null} if {@code b} is {@link Boolean#FALSE}.
     */
    protected static Boolean triState(Boolean b) {
        if (b == null) {
            return Boolean.TRUE;
        }
        if (Boolean.TRUE.equals(b)) {
            return Boolean.FALSE;
        }

        return null;
    }

    /**
     * let a thread sleep. used for dispatch other thread.
     *
     * @param millis    the length of time in millisecond.
     */
    protected void sleep(long millis) {
        Thread.yield();
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            unexpected(e);
        }
    }

    /**
     * Flush all pending tasks on the VTN task thread.
     */
    protected void flushTasks(VTNManagerImpl vtnMgr) {
        NopTask task = new NopTask();
        vtnMgr.postTask(task);
        assertTrue(task.await(10, TimeUnit.SECONDS));
    }

    /**
     * A dummy task to flush tasks on the VTN task thread.
     */
    private class NopTask implements Runnable {
        /**
         * A latch to wait for completion.
         */
        private final CountDownLatch  latch = new CountDownLatch(1);

        /**
         * Wake up all threads waiting for this task.
         */
        @Override
        public void run() {
            latch.countDown();
        }

        /**
         * Wait for completion of this task.
         *
         * @param timeout  The maximum time to wait.
         * @param unit     The time unit of the {@code timeout} argument.
         * @return  {@code true} is returned if this task completed.
         *          Otherwise {@code false} is returned.
         */
        private boolean await(long timeout, TimeUnit unit) {
            try {
                return latch.await(timeout, unit);
            } catch (InterruptedException e) {
                return false;
            }
        }
    }
}

