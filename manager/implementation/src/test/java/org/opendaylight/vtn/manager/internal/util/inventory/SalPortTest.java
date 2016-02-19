/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import java.math.BigInteger;
import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortLocation;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Destination;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.DestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Source;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.SourceBuilder;

/**
 * JUnit test for {@link SalPort}.
 */
public class SalPortTest extends TestBase {
    /**
     * Describes an empty node-connector-ref.
     */
    private static final class EmptyRef extends NodeConnectorRef {
        /**
         * Construct a new instance.
         */
        private EmptyRef() {
            super(InstanceIdentifier.create(Nodes.class));
        }

        // NodeConnectorRef

        /**
         * Return {@code null} as value.
         *
         * @return {@code null}.
         */
        @Override
        public InstanceIdentifier<?> getValue() {
            return null;
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link SalPort#create(String)}</li>
     *   <li>{@link SalPort#create(NodeConnectorId)}</li>
     *   <li>{@link SalPort#create(NodeConnectorRef)}</li>
     *   <li>{@link SalPort#create(Source)}</li>
     *   <li>{@link SalPort#create(Destination)}</li>
     *   <li>{@link SalPort#create(TpId)}</li>
     *   <li>{@link SalPort#create(long, String)}</li>
     *   <li>{@link SalPort#getNodeNumber()}</li>
     *   <li>{@link SalPort#getPortNumber()}</li>
     *   <li>{@link SalPort#isLogicalPort(NodeConnectorId)}</li>
     */
    @Test
    public void testCreateString() {
        assertEquals(null, SalPort.create((NodeConnectorId)null));
        assertEquals(null, SalPort.create((NodeConnectorRef)null));
        assertEquals(null, SalPort.create((Source)null));
        assertEquals(null, SalPort.create((Destination)null));
        assertEquals(null, SalPort.create((TpId)null));
        assertEquals(null, SalPort.create(new EmptyRef()));
        assertFalse(SalPort.isLogicalPort(null));

        // Invalid node connector path.
        InstanceIdentifier<?> path = InstanceIdentifier.
            builder(Nodes.class).
            child(Node.class, new NodeKey(new NodeId("test"))).build();
        assertEquals(null, SalPort.create(new NodeConnectorRef(path)));
        path = FlowUtils.getIdentifier("vtn1");
        assertEquals(null, SalPort.create(new NodeConnectorRef(path)));
        path = PathPolicyUtils.getIdentifier(1);
        assertEquals(null, SalPort.create(new NodeConnectorRef(path)));

        String[] bad = {
            // Invalid node type.
            null,
            "",
            "proto1:1:3",
            "of:2:1",
            "openflow1:1:4",

            // Too large DPID.
            "openflow:18446744073709551616:3",
            "openflow:18446744073709551617:2",
            "openflow:99999999999999999999999:2",
            "openflow:999999999999999999999999:LOCAL",

            // Negative DPID.
            "openflow:-1:1",
            "openflow:-12345678:2",
            "openflow:-3333333333333333333333333:2",

            // Invalid DPID.
            "openflow:0x12345678:1",
            "openflow:1234abcd:1",
            "openflow:Bad DPID:1",
        };
        for (String id: bad) {
            for (SalPort sport: createSalPorts(id, false)) {
                assertEquals(null, sport);
            }
        }

        BigInteger[] ids = {
            BigInteger.ZERO,
            BigInteger.ONE,
            new BigInteger("9223372036854775807"),
            new BigInteger("9223372036854775808"),
            new BigInteger("18446744073709551614"),
            new BigInteger("18446744073709551615"),
        };
        long[] portNums = {
            0L,
            1L,
            100L,
            0x7ffffffeL,
            0x7fffffffL,
            0x80000000L,
            0x80000001L,
            0xffffff00L,
        };
        long[] invPorts = {
            Long.MIN_VALUE,
            -2L,
            -1L,
            0xffffff01L,
            0xffffff02L,
            0xfffffff0L,
            0xfffffffeL,
            0xffffffffL,
        };
        String[] logicalPorts = {
            "MAX",
            "IN_PORT",
            "TABLE",
            "NORMAL",
            "FLOOD",
            "ALL",
            "CONTROLLER",
            "LOCAL",
        };
        for (BigInteger dpid: ids) {
            long nid = dpid.longValue();

            // Without specifying port number.
            String id = "openflow:" + dpid;
            for (SalPort sport: createSalPorts(id, false)) {
                assertEquals(null, sport);
            }
            assertEquals(null, SalPort.create(nid, (String)null));
            assertEquals(null, SalPort.create(nid, ""));

            // Specifying logical port name.
            for (String l: logicalPorts) {
                String portId = id + ":" + l;
                for (SalPort sport: createSalPorts(portId, true)) {
                    assertEquals(null, sport);
                }
                assertEquals(null, SalPort.create(nid, l));
            }

            // Port number is out of range.
            for (long p: invPorts) {
                String portId = id + ":" + p;
                for (SalPort sport: createSalPorts(portId, false)) {
                    assertEquals(null, sport);
                }
                assertEquals(null, SalPort.create(nid, String.valueOf(p)));
            }

            for (long p: portNums) {
                String portId = id + ":" + p;
                for (SalPort sport: createSalPorts(portId, false)) {
                    assertNotNull(sport);
                    assertEquals(nid, sport.getNodeNumber());
                    assertEquals(p, sport.getPortNumber());
                }

                SalPort sport = SalPort.create(nid, String.valueOf(p));
                assertNotNull(sport);
                assertEquals(nid, sport.getNodeNumber());
                assertEquals(p, sport.getPortNumber());

                sport = SalPort.create(nid, String.format("0x%x", p));
                assertNotNull(sport);
                assertEquals(nid, sport.getNodeNumber());
                assertEquals(p, sport.getPortNumber());
            }
        }
    }

    /**
     * Test case for {@link SalPort#create(VtnPortLocation)}.
     */
    @Test
    public void testCreateVtnPortLocation() {
        VtnPortLocation vpl = null;
        assertEquals(null, SalPort.create(vpl));

        SalPort[] sports = {
            new SalPort(1L, 1L),
            new SalPort(1L, 0xffffff00L),
            new SalPort(1L, 2L),
            new SalPort(123L, 456L),
            new SalPort(-1L, 1000L),
            new SalPort(-1L, 0xffffff00L),
        };

        for (SalPort sport: sports) {
            NodeId nodeId = sport.getNodeId();
            String portId = String.valueOf(sport.getPortNumber());
            vpl = mock(VtnPortLocation.class);
            when(vpl.getNode()).thenReturn(nodeId);
            when(vpl.getPortId()).thenReturn(portId);
            assertEquals(sport, SalPort.create(vpl));
        }

        // Invalid node ID.
        NodeId[] badNodeIds = {
            null,
            new NodeId(""),
            new NodeId("openflow"),
            new NodeId("openflow:"),
            new NodeId("openflow:18446744073709551616"),
            new NodeId("unknown:1"),
        };

        for (NodeId nodeId: badNodeIds) {
            vpl = mock(VtnPortLocation.class);
            when(vpl.getNode()).thenReturn(nodeId);
            when(vpl.getPortId()).thenReturn("1");
            assertEquals(null, SalPort.create(vpl));
        }

        // Invalid port ID.
        NodeId nodeId = new NodeId("openflow:1");
        String[] badPortIds = {
            null,
            "MAX",
            "IN_PORT",
            "TABLE",
            "NORMAL",
            "FLOOD",
            "ALL",
            "CONTROLLER",
            "LOCAL",
            "-1",
            "bad port ID",
            "4294967041",
        };

        for (String portId: badPortIds) {
            vpl = mock(VtnPortLocation.class);
            when(vpl.getNode()).thenReturn(nodeId);
            when(vpl.getPortId()).thenReturn(portId);
            assertEquals(null, SalPort.create(vpl));
        }
    }

    /**
     * Test case for constructors.
     *
     * <ul>
     *   <li>{@link SalPort#SalPort(long, long)}</li>
     *   <li>{@link SalPort#SalPort(long, long, String)}</li>
     * </ul>
     */
    @Test
    public void testConstructor() {
        long[] ids = {
            Long.MIN_VALUE,
            -1L,
            0L,
            1L,
            10000L,
            Long.MAX_VALUE,
        };
        long[] ports = {
            0L,
            1L,
            100L,
            0x7ffffffeL,
            0x7fffffffL,
            0x80000000L,
            0x80000001L,
            0xfffffeffL,
            0xffffff00L,
        };

        for (long dpid: ids) {
            for (long port: ports) {
                SalPort sport = new SalPort(dpid, port);
                assertEquals(dpid, sport.getNodeNumber());
                assertEquals(port, sport.getPortNumber());

                sport = new SalPort(dpid, port, (String)null);
                assertEquals(dpid, sport.getNodeNumber());
                assertEquals(port, sport.getPortNumber());
            }
        }
    }

    /**
     * Test for getter methods.
     *
     * <ul>
     *   <li>{@link SalPort#getSalNode()}</li>
     *   <li>{@link SalPort#getNodeConnectorId()}</li>
     *   <li>{@link SalPort#getNodeConnectorKey()}</li>
     *   <li>{@link SalPort#getNodeConnectorRef()}</li>
     *   <li>{@link SalPort#getNodeConnectorIdentifier()}</li>
     *   <li>{@link SalPort#getVtnPortKey()}</li>
     *   <li>{@link SalPort#getVtnPortIdentifier()}</li>
     *   <li>{@link SalPort#getPortLinkIdentifier(LinkId)}</li>
     *   <li>{@link SalPort#getVtnPortIdentifierBuilder()}</li>
     *   <li>{@link SalPort#getNodeId()}</li>
     *   <li>{@link SalPort#getNodeKey()}</li>
     *   <li>{@link SalPort#getNodeIdentifier()}</li>
     *   <li>{@link SalPort#getNodeRef()}</li>
     *   <li>{@link SalPort#getVtnNodeKey()}</li>
     *   <li>{@link SalPort#getVtnNodeIdentifier()}</li>
     *   <li>{@link SalPort#toStringBuilder()}</li>
     *   <li>{@link SalPort#toNodeString()}</li>
     *   <li>{@link SalPort#toNodeStringBuilder()}</li>
     *   <li>{@link SalPort#getNodeIdentifierBuilder()}</li>
     *   <li>{@link SalPort#getVtnNodeIdentifierBuilder()}</li>
     *   <li>{@link SalPort#toString()}</li>
     * </ul>
     */
    @Test
    public void testGetter() {
        BigInteger[] ids = {
            BigInteger.ZERO,
            BigInteger.ONE,
            BigInteger.valueOf(10L),
            BigInteger.valueOf(12345L),
            BigInteger.valueOf(0xaabbccddeeff01L),
            new BigInteger("9223372036854775806"),
            new BigInteger("9223372036854775807"),
            new BigInteger("9223372036854775808"),
            new BigInteger("9223372036854775809"),
            new BigInteger("18446744073709551613"),
            new BigInteger("18446744073709551614"),
            new BigInteger("18446744073709551615"),
        };

        long[] ports = {
            0L,
            1L,
            100L,
            0x7ffeL,
            0x7fffL,
            0x8000L,
            0x8001L,
            0xfeffL,
            0xff00L,
            0xff01L,
            0xfffeL,
            0xffffL,
            0x10000L,
            0x7ffffffeL,
            0x7fffffffL,
            0x80000000L,
            0x80000001L,
            0xfffffeffL,
            0xffffff00L,
        };

        for (BigInteger dpid: ids) {
            long id = dpid.longValue();
            SalNode exnode = new SalNode(id);
            for (long portNum: ports) {
                SalPort sport = new SalPort(id, portNum);
                SalNode prevNode = null;
                for (int i = 0; i < 4; i++) {
                    SalNode sn = sport.getSalNode();
                    assertEquals(false, sn.equals(sport));
                    assertEquals(false, sport.equals(sn));
                    assertEquals(id, sn.getNodeNumber());
                    assertEquals(exnode, sn);
                    assertNotSame(prevNode, sn);
                    prevNode = sn;
                }

                String nodeStr = "openflow:" + dpid;
                NodeId nid = new NodeId(nodeStr);
                assertEquals(nid, sport.getNodeId());
                String portStr = nodeStr + ":" + portNum;
                NodeConnectorId ncId = new NodeConnectorId(portStr);
                assertEquals(ncId, sport.getNodeConnectorId());

                NodeKey key = new NodeKey(nid);
                assertEquals(key, sport.getNodeKey());
                NodeConnectorKey ncKey = new NodeConnectorKey(ncId);
                assertEquals(ncKey, sport.getNodeConnectorKey());

                InstanceIdentifier<Node> path = InstanceIdentifier.
                    builder(Nodes.class).
                    child(Node.class, key).build();
                assertEquals(path, sport.getNodeIdentifier());
                assertEquals(path, sport.getNodeIdentifierBuilder().build());

                InstanceIdentifier<NodeConnector> ncPath = InstanceIdentifier.
                    builder(Nodes.class).
                    child(Node.class, key).
                    child(NodeConnector.class, ncKey).build();
                assertEquals(ncPath, sport.getNodeConnectorIdentifier());

                NodeRef ref = new NodeRef(path);
                assertEquals(ref, sport.getNodeRef());
                NodeConnectorRef ncRef = new NodeConnectorRef(ncPath);
                assertEquals(ncRef, sport.getNodeConnectorRef());

                VtnNodeKey vkey = new VtnNodeKey(nid);
                assertEquals(vkey, sport.getVtnNodeKey());
                VtnPortKey vpKey = new VtnPortKey(ncId);
                assertEquals(vpKey, sport.getVtnPortKey());

                InstanceIdentifier<VtnNode> vpath = InstanceIdentifier.
                    builder(VtnNodes.class).
                    child(VtnNode.class, vkey).build();
                assertEquals(vpath, sport.getVtnNodeIdentifier());
                assertEquals(vpath,
                             sport.getVtnNodeIdentifierBuilder().build());

                InstanceIdentifier<VtnPort> vpPath = InstanceIdentifier.
                    builder(VtnNodes.class).
                    child(VtnNode.class, vkey).
                    child(VtnPort.class, vpKey).build();
                assertEquals(vpPath, sport.getVtnPortIdentifier());

                LinkId linkId = new LinkId("link@" + portStr);
                InstanceIdentifier<PortLink> plPath = InstanceIdentifier.
                    builder(VtnNodes.class).
                    child(VtnNode.class, vkey).
                    child(VtnPort.class, vpKey).
                    child(PortLink.class, new PortLinkKey(linkId)).build();
                assertEquals(plPath, sport.getPortLinkIdentifier(linkId));

                assertEquals(portStr, sport.toStringBuilder().toString());
                assertEquals(nodeStr, sport.toNodeStringBuilder().toString());
                assertEquals(nodeStr, sport.toNodeString());
                String s = sport.toString();
                assertEquals(portStr, s);

                // Port identifier string should be cached.
                assertSame(s, sport.toString());
                assertEquals(portStr, sport.toStringBuilder().toString());
                assertEquals(nodeStr, sport.toNodeStringBuilder().toString());
                assertEquals(nodeStr, sport.toNodeString());
            }
        }
    }

    /**
     * Test case for {@link SalPort#equalsPort(SalPort)}.
     */
    @Test
    public void testEqualsPort() {
        SalPort target = new SalPort(6L, 2L);
        assertEquals(false, target.equalsPort(null));

        for (long dpid = 1L; dpid <= 10L; dpid++) {
            for (long port = 1L; port <= 5L; port++) {
                SalPort sport = new SalPort(dpid, port);
                assertEquals(target.equals(sport), target.equalsPort(sport));
            }
        }
    }

    /**
     * Test for {@link SalPort#equals(Object)} and {@link SalPort#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        BigInteger[] ids = {
            BigInteger.ZERO,
            BigInteger.ONE,
            BigInteger.valueOf(10L),
            BigInteger.valueOf(12345L),
            BigInteger.valueOf(0xaabbccddeeff01L),
            new BigInteger("9223372036854775806"),
            new BigInteger("9223372036854775807"),
            new BigInteger("9223372036854775808"),
            new BigInteger("9223372036854775809"),
            new BigInteger("18446744073709551613"),
            new BigInteger("18446744073709551614"),
            new BigInteger("18446744073709551615"),
        };

        long[] ports = {
            0L,
            1L,
            100L,
            0x7ffeL,
            0x7fffL,
            0x8000L,
            0x8001L,
            0xfeffL,
            0xff00L,
            0xff01L,
            0xfffeL,
            0xffffL,
            0x10000L,
            0x7ffffffeL,
            0x7fffffffL,
            0x80000000L,
            0x80000001L,
            0xfffffeffL,
            0xffffff00L,
        };

        for (BigInteger dpid: ids) {
            long id = dpid.longValue();
            boolean first = true;
            for (long portNum: ports) {
                SalPort sport1 = new SalPort(id, portNum);
                SalPort sport2 = new SalPort(id, portNum);
                testEquals(set, sport1, sport2);

                SalNode snode1 = sport1.getSalNode();
                SalNode snode2 = sport1.getSalNode();
                assertNotSame(snode1, snode2);
                assertEquals(snode1, snode2);
                if (first) {
                    testEquals(set, snode1, snode2);
                    first = false;
                } else {
                    assertEquals(false, set.add(snode1));
                    assertEquals(false, set.add(snode2));
                }
            }
        }

        int expected = ids.length * ports.length + ids.length;
        assertEquals(expected, set.size());
    }

    /**
     * Create a list of {@link SalPort} instances created by factory methods.
     *
     * @param id       A string representation of the node connector ID.
     * @param logical  {@code true} means that the given ID is a valid ID for
     *                 the logical port.
     * @return  A list of {@link SalPort} instances.
     */
    private List<SalPort> createSalPorts(String id, boolean logical) {
        List<SalPort> list = new ArrayList<>();
        list.add(SalPort.create(id));
        if (id != null) {
            NodeConnectorId ncid = new NodeConnectorId(id);
            list.add(SalPort.create(ncid));
            assertEquals(logical, SalPort.isLogicalPort(ncid));

            InstanceIdentifier<NodeConnector> path = InstanceIdentifier.
                builder(Nodes.class).
                child(Node.class, new NodeKey(new NodeId("test"))).
                child(NodeConnector.class, new NodeConnectorKey(ncid)).
                build();
            list.add(SalPort.create(new NodeConnectorRef(path)));

            TpId tpid = new TpId(id);
            list.add(SalPort.create(tpid));
            Source src = new SourceBuilder().setSourceTp(tpid).build();
            list.add(SalPort.create(src));
            Destination dst = new DestinationBuilder().setDestTp(tpid).build();
            list.add(SalPort.create(dst));
        }

        return list;
    }
}
