/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.math.BigInteger;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.flow.FlowUtils;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.Table;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.tables.TableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

/**
 * JUnit test for {@link SalNode}.
 */
public class SalNodeTest extends TestBase {
    /**
     * Describes an empty node-ref.
     */
    private static final class EmptyRef extends NodeRef {
        /**
         * Construct a new instance.
         */
        private EmptyRef() {
            super(InstanceIdentifier.create(Nodes.class));
        }

        // NodeRef

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
     *   <li>{@link SalNode#create(String)}</li>
     *   <li>{@link SalNode#create(NodeId)}</li>
     *   <li>{@link SalNode#create(NodeRef)}</li>
     *   <li>{@link SalNode#checkedCreate(String)}</li>
     *   <li>{@link SalNode#checkedCreate(NodeId)}</li>
     *   <li>{@link SalNode#getNodeNumber()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateString() throws Exception {
        assertEquals(null, SalNode.create((NodeId)null));
        assertEquals(null, SalNode.create((NodeRef)null));
        assertEquals(null, SalNode.create(new EmptyRef()));

        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String[] bad = {
            // Invalid node type.
            null,
            "",
            "proto1:1",
            "of:2",
            "openflow1:1",

            // Too large DPID.
            "openflow:18446744073709551616",
            "openflow:18446744073709551617",
            "openflow:99999999999999999999999",

            // Negative DPID.
            "openflow:-1",
            "openflow:-12345",

            // Invalid DPID.
            "openflow:0x12345678",
            "openflow:1234abcd",
            "openflow:Bad DPID",
        };
        for (String id: bad) {
            assertEquals(null, SalNode.create(id));
            if (id == null) {
                String msg = "Node cannot be null";
                try {
                    SalNode.checkedCreate(id);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }

                try {
                    SalNode.checkedCreate((NodeId)null);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            } else {
                NodeId nid = new NodeId(id);
                assertEquals(null, SalNode.create(nid));

                String msg = "Invalid node ID: " + id;
                try {
                    SalNode.checkedCreate(nid);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }

                try {
                    SalNode.checkedCreate(id);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }

                InstanceIdentifier<Node> path = InstanceIdentifier.
                    builder(Nodes.class).
                    child(Node.class, new NodeKey(nid)).build();
                NodeRef ref = new NodeRef(path);
                assertEquals(null, SalNode.create(ref));
            }
        }

        // Invalid node path.
        InstanceIdentifier<?> invalidPath = FlowUtils.getIdentifier("vtn1");
        assertEquals(null, SalPort.create(new NodeRef(invalidPath)));
        invalidPath = PathPolicyUtils.getIdentifier(1);
        assertEquals(null, SalPort.create(new NodeRef(invalidPath)));

        BigInteger[] ids = {
            BigInteger.ZERO,
            BigInteger.ONE,
            new BigInteger("9223372036854775807"),
            new BigInteger("9223372036854775808"),
            new BigInteger("18446744073709551614"),
            new BigInteger("18446744073709551615"),
        };
        for (BigInteger dpid: ids) {
            String id = "openflow:" + dpid;
            SalNode snode = SalNode.create(id);
            assertEquals(dpid.longValue(), snode.getNodeNumber());

            NodeId nid = new NodeId(id);
            snode = SalNode.create(nid);
            assertEquals(dpid.longValue(), snode.getNodeNumber());

            InstanceIdentifier<Node> path = InstanceIdentifier.
                builder(Nodes.class).
                child(Node.class, new NodeKey(nid)).build();
            NodeRef ref = new NodeRef(path);
        }
    }

    /**
     * Test case for constructors.
     *
     * <ul>
     *   <li>{@link SalNode#SalNode(long)}</li>
     *   <li>{@link SalNode#SalNode(long, String)}</li>
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

        for (long dpid: ids) {
            SalNode snode = new SalNode(dpid);
            assertEquals(dpid, snode.getNodeNumber());

            snode = new SalNode(dpid, (String)null);
            assertEquals(dpid, snode.getNodeNumber());
        }
    }

    /**
     * Test for getter methods.
     *
     * <ul>
     *   <li>{@link SalNode#getNodeId()}</li>
     *   <li>{@link SalNode#getNodeKey()}</li>
     *   <li>{@link SalNode#getNodeIdentifier()}</li>
     *   <li>{@link SalNode#getNodeRef()}</li>
     *   <li>{@link SalNode#getFlowNodeIdentifier()}</li>
     *   <li>{@link SalNode#getFlowTableIdentifier(Short)}</li>
     *   <li>{@link SalNode#getVtnNodeKey()}</li>
     *   <li>{@link SalNode#getVtnNodeIdentifier()}</li>
     *   <li>{@link SalNode#toStringBuilder()}</li>
     *   <li>{@link SalNode#toNodeString()}</li>
     *   <li>{@link SalNode#toNodeStringBuilder()}</li>
     *   <li>{@link SalNode#getNodeIdentifierBuilder()}</li>
     *   <li>{@link SalNode#getVtnNodeIdentifierBuilder()}</li>
     *   <li>{@link SalNode#toString()}</li>
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

        for (BigInteger dpid: ids) {
            long id = dpid.longValue();
            SalNode snode = new SalNode(id);
            String idstr = "openflow:" + dpid;
            NodeId nid = new NodeId(idstr);
            assertEquals(nid, snode.getNodeId());

            NodeKey key = new NodeKey(nid);
            assertEquals(key, snode.getNodeKey());

            InstanceIdentifier<Node> path = InstanceIdentifier.
                builder(Nodes.class).
                child(Node.class, key).build();
            assertEquals(path, snode.getNodeIdentifier());
            assertEquals(path, snode.getNodeIdentifierBuilder().build());

            NodeRef ref = new NodeRef(path);
            assertEquals(ref, snode.getNodeRef());

            InstanceIdentifier<FlowCapableNode> fnodePath = path.builder().
                augmentation(FlowCapableNode.class).build();
            assertEquals(fnodePath, snode.getFlowNodeIdentifier());

            for (short table = 0; table < 10; table++) {
                InstanceIdentifier<Table> tablePath = fnodePath.builder().
                    child(Table.class, new TableKey(table)).build();
                assertEquals(tablePath, snode.getFlowTableIdentifier(table));
            }

            VtnNodeKey vkey = new VtnNodeKey(nid);
            assertEquals(vkey, snode.getVtnNodeKey());

            InstanceIdentifier<VtnNode> vpath = InstanceIdentifier.
                builder(VtnNodes.class).
                child(VtnNode.class, vkey).build();
            assertEquals(vpath, snode.getVtnNodeIdentifier());
            assertEquals(vpath, snode.getVtnNodeIdentifierBuilder().build());

            assertEquals(idstr, snode.toStringBuilder().toString());
            assertEquals(idstr, snode.toNodeStringBuilder().toString());
            String s = snode.toString();
            assertEquals(idstr, s);

            // Node identifier string should be cached.
            assertSame(s, snode.toString());
            assertSame(s, snode.toNodeString());
            assertEquals(idstr, snode.toStringBuilder().toString());
            assertEquals(idstr, snode.toNodeStringBuilder().toString());
        }
    }

    /**
     * Test case for {@link SalNode#equalsNode(SalNode)}.
     */
    @Test
    public void testEqualsNode() {
        long nodeNumber = 3L;
        SalNode targetNode = new SalNode(nodeNumber);
        SalPort targetPort = new SalPort(nodeNumber, 123L);
        assertEquals(false, targetNode.equalsNode(null));
        assertEquals(false, targetPort.equalsNode(null));

        for (long dpid = 1L; dpid <= 10L; dpid++) {
            boolean expected = (dpid == nodeNumber);
            SalNode snode = new SalNode(dpid);
            assertEquals(expected, targetNode.equalsNode(snode));
            assertEquals(expected, targetPort.equalsNode(snode));

            for (long port = 1L; port <= 5L; port++) {
                SalPort sport = new SalPort(dpid, port);
                assertEquals(expected, targetNode.equalsNode(sport));
                assertEquals(expected, targetPort.equalsNode(sport));
            }
        }
    }

    /**
     * Test for {@link SalNode#equals(Object)} and {@link SalNode#hashCode()}.
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

        for (BigInteger dpid: ids) {
            long id = dpid.longValue();
            SalNode snode1 = new SalNode(id);
            SalNode snode2 = new SalNode(id);
            testEquals(set, snode1, snode2);
        }

        assertEquals(ids.length, set.size());
    }
}
