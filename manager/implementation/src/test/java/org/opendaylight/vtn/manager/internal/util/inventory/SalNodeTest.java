/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node.NodeIDType;
import org.opendaylight.controller.sal.utils.NodeCreator;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeKey;

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
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link SalNode#create(String)}</li>
     *   <li>{@link SalNode#create(NodeId)}</li>
     *   <li>{@link SalNode#create(NodeRef)}</li>
     *   <li>{@link SalNode#getNodeNumber()}</li>
     * </ul>
     */
    @Test
    public void testCreateString() {
        assertEquals(null, SalNode.create((NodeId)null));
        assertEquals(null, SalNode.create((NodeRef)null));
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
        };
        for (String id: bad) {
            assertEquals(null, SalNode.create(id));
            if (id != null) {
                NodeId nid = new NodeId(id);
                assertEquals(null, SalNode.create(id));

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
            BigInteger.valueOf(0L),
            BigInteger.valueOf(1L),
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
     * Test case for {@link SalNode#create(org.opendaylight.controller.sal.core.Node)} and
     * {@link SalNode#getNodeNumber()}.
     *
     * @throws Exception   An error occurred.
     */
    @Test
    public void testCreateAdNode() throws Exception {
        org.opendaylight.controller.sal.core.Node node = null;
        assertEquals(null, SalNode.create(node));

        // Unsupported nodes.
        node = new org.opendaylight.controller.sal.core.Node(
            NodeIDType.ONEPK, "node 1");
        assertEquals(null, SalNode.create(node));
        node = new org.opendaylight.controller.sal.core.Node(
            NodeIDType.PRODUCTION, "node 2");
        assertEquals(null, SalNode.create(node));

        long[] ids = {
            Long.MIN_VALUE,
            -1L,
            0L,
            1L,
            10000L,
            Long.MAX_VALUE,
        };
        for (long dpid: ids) {
            node = new org.opendaylight.controller.sal.core.Node(
                NodeIDType.OPENFLOW, Long.valueOf(dpid));
            SalNode snode = SalNode.create(node);
            assertNotNull(snode);
            assertEquals(dpid, snode.getNodeNumber());

            // AD-SAL node should be cached.
            assertSame(node, snode.getAdNode());
            assertSame(node, snode.getAdNode());
        }
    }

    /**
     * Test case for constructors.
     *
     * <ul>
     *   <li>{@link SalNode#SalNode(long)}</li>
     *   <li>{@link SalNode#SalNode(long, String)}</li>
     *   <li>{@link SalNode#SalNode(long, org.opendaylight.controller.sal.core.Node)}</li>
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

        org.opendaylight.controller.sal.core.Node node = null;
        for (long dpid: ids) {
            SalNode snode = new SalNode(dpid);
            assertEquals(dpid, snode.getNodeNumber());

            snode = new SalNode(dpid, (String)null);
            assertEquals(dpid, snode.getNodeNumber());

            snode = new SalNode(dpid, node);
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
     *   <li>{@link SalNode#getAdNode()}</li>
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
            BigInteger.valueOf(0L),
            BigInteger.valueOf(1L),
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

            org.opendaylight.controller.sal.core.Node adnode = NodeCreator.
                createOFNode(Long.valueOf(id));
            assertNotNull(adnode);
            org.opendaylight.controller.sal.core.Node ad = snode.getAdNode();
            assertEquals(adnode, ad);

            // AD-SAL node should be cached.
            assertSame(ad, snode.getAdNode());

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
     * Test for {@link SalNode#equals(Object)} and {@link SalNode#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        BigInteger[] ids = {
            BigInteger.valueOf(0L),
            BigInteger.valueOf(1L),
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
