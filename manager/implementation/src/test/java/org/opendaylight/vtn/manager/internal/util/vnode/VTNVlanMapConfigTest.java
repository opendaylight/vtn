/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestVlanId;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link VTNVlanMapConfig}.
 */
public class VTNVlanMapConfigTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNVlanMapConfig} class.
     */
    private static final String  XML_ROOT = "vtn-vlan-map-config";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNVlanMapConfig} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("node", SalNode.class).add(name).prepend(parent),
            new XmlValueType("vlan-id",
                             VlanId.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for {@link VTNVlanMapConfig#createMapId(SalNode, VlanId)}.
     */
    @Test
    public void testCreateMapId1() {
        NodeId[] nodeIds = {
            null,
            new NodeId("openflow:1"),
            new NodeId("openflow:2"),
            new NodeId("openflow:123456789"),
            new NodeId("openflow:18446744073709551614"),
            new NodeId("openflow:18446744073709551615"),
        };
        int[] vids = {0, 1, 123, 456, 1024, 4094, 4095};

        for (NodeId nid: nodeIds) {
            String nodeId = (nid == null) ? "ANY" : nid.getValue();
            SalNode snode = SalNode.create(nid);
            for (int vid: vids) {
                VlanId vlanId = new VlanId(vid);
                String expected = nodeId + "." + vid;
                assertEquals(expected,
                             VTNVlanMapConfig.createMapId(snode, vlanId));
            }
        }
    }

    /**
     * Test case for {@link VTNVlanMapConfig#createMapId(VtnVlanMapConfig)}.
     */
    @Test
    public void testCreateMapId2() {
        NodeId[] nodeIds = {
            null,
            new NodeId("openflow:1"),
            new NodeId("openflow:2"),
            new NodeId("openflow:123456789"),
            new NodeId("openflow:18446744073709551614"),
            new NodeId("openflow:18446744073709551615"),
        };
        Integer[] vids = {-100, -1, 0, 1, 123, 456, 1024, 4094, 4095, 10000};
        VlanId[] empty = {null, new TestVlanId()};

        for (NodeId nid: nodeIds) {
            String nodeId = (nid == null) ? "ANY" : nid.getValue();
            SalNode snode = SalNode.create(nid);
            for (Integer vid: vids) {
                VlanId vlanId = new TestVlanId(vid);
                String expected = nodeId + "." + vid;
                VtnVlanMapConfig cfg = mock(VtnVlanMapConfig.class);
                when(cfg.getNode()).thenReturn(nid);
                when(cfg.getVlanId()).thenReturn(vlanId);
                assertEquals(expected, VTNVlanMapConfig.createMapId(cfg));
            }

            // No vlan-id is specified.
            String expected = nodeId + ".0";
            for (VlanId vlanId: empty) {
                VtnVlanMapConfig cfg = mock(VtnVlanMapConfig.class);
                when(cfg.getNode()).thenReturn(nid);
                when(cfg.getVlanId()).thenReturn(vlanId);
                assertEquals(expected, VTNVlanMapConfig.createMapId(cfg));
            }
        }

        // Invalid node-id.
        NodeId[] invalid = {
            new NodeId(""),
            new NodeId("unknown:1"),
            new NodeId("openflow:1:2"),
            new NodeId("openflow:18446744073709551616"),
        };
        VlanId vlanId = new VlanId(0);
        for (NodeId nid: invalid) {
            VtnVlanMapConfig cfg = mock(VtnVlanMapConfig.class);
            when(cfg.getNode()).thenReturn(nid);
            when(cfg.getVlanId()).thenReturn(vlanId);
            assertEquals(null, VTNVlanMapConfig.createMapId(cfg));
        }
    }

    /**
     * Test case for
     * {@link VTNVlanMapConfig#VTNVlanMapConfig(VtnVlanMapConfig)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        SalNode[] nodes = {
            null,
            new SalNode(1L),
            new SalNode(12345678L),
            new SalNode(-1L),
        };
        Integer[] vids = {
            null, 0, 1, 2, 100, 678, 1234, 3456, 4094, 4095,
        };

        VlanMapConfigBuilder builder = new VlanMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = (snode == null) ? null : snode.getNodeId();
            builder.setNode(node);
            for (Integer vid: vids) {
                VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                VtnVlanMapConfig vvmc = builder.setVlanId(vlanId).build();
                VTNVlanMapConfig vmc = new VTNVlanMapConfig(vvmc);
                assertEquals(snode, vmc.getTargetNode());
                int exVid = (vid == null) ? 0 : vid;
                assertEquals(exVid, vmc.getVlanId());
                assertEquals(new NodeVlan(snode, exVid), vmc.getNodeVlan());
            }
        }

        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;

        // VLAN ID is missing.
        String msg = "vlan-id cannot be null";
        VlanId vlanId = new TestVlanId();
        VtnVlanMapConfig vvmc = mock(VtnVlanMapConfig.class);
        when(vvmc.getNode()).thenReturn((NodeId)null);
        when(vvmc.getVlanId()).thenReturn(vlanId);

        try {
            new VTNVlanMapConfig(vvmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verify(vvmc).getNode();
        verify(vvmc).getVlanId();
        verifyNoMoreInteractions(vvmc);

        // Invalid VLAN ID.
        etag = RpcErrorTag.BAD_ELEMENT;
        Integer[] badVids = {
            Integer.MIN_VALUE, -2, -1, 4096, 4097, 100000, Integer.MAX_VALUE,
        };
        for (Integer vid: badVids) {
            msg = "Invalid VLAN ID: " + vid;
            vlanId = new TestVlanId(vid);
            vvmc = mock(VtnVlanMapConfig.class);
            when(vvmc.getNode()).thenReturn((NodeId)null);
            when(vvmc.getVlanId()).thenReturn(vlanId);

            try {
                new VTNVlanMapConfig(vvmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
            verify(vvmc).getNode();
            verify(vvmc).getVlanId();
            verifyNoMoreInteractions(vvmc);
        }

        // Invalid node ID.
        String[] badNodes = {
            "",
            "openflow",
            "openflow:",
            "openflow:DPID",
            "unknown:1",
        };
        for (String bad: badNodes) {
            NodeId node = new NodeId(bad);
            vvmc = new VlanMapConfigBuilder().setNode(node).build();
            try {
                new VTNVlanMapConfig(vvmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals("Invalid node ID: " + bad, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link VTNVlanMapConfig#toVlanMapConfig()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToVlanMapConfig() throws Exception {
        SalNode[] nodes = {
            null,
            new SalNode(1L),
            new SalNode(12345678L),
            new SalNode(-1L),
        };
        Integer[] vids = {
            null, 0, 1, 2, 100, 678, 1234, 3456, 4094, 4095,
        };

        VlanMapConfigBuilder builder = new VlanMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = (snode == null) ? null : snode.getNodeId();
            builder.setNode(node);
            for (Integer vid: vids) {
                VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                VtnVlanMapConfig vvmc = builder.setVlanId(vlanId).build();
                VTNVlanMapConfig vmc = new VTNVlanMapConfig(vvmc);
                assertEquals(snode, vmc.getTargetNode());
                Integer exVid = (vid == null) ? 0 : vid;
                VlanMapConfig expected = new VlanMapConfigBuilder(vvmc).
                    setVlanId(new VlanId(exVid)).build();
                assertEquals(expected, vmc.toVlanMapConfig());
            }
        }
    }

    /**
     * Test case for {@link VTNVlanMapConfig#equals(Object)} and
     * {@link VTNVlanMapConfig#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        SalNode[] nodes = {
            null,
            new SalNode(1L),
            new SalNode(123456789L),
            new SalNode(-1L),
        };
        Integer[] vids = {
            null, 1, 2, 100, 678, 1234, 3456, 4094, 4095,
        };

        VlanMapConfigBuilder builder = new VlanMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = (snode == null) ? null : snode.getNodeId();
            builder.setNode(node);
            for (Integer vid: vids) {
                VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                VtnVlanMapConfig vvmc = builder.setVlanId(vlanId).build();
                VTNVlanMapConfig vmc1 = new VTNVlanMapConfig(vvmc);
                VTNVlanMapConfig vmc2 = new VTNVlanMapConfig(vvmc);
                testEquals(set, vmc1, vmc2);
            }
        }

        assertEquals(nodes.length * vids.length, set.size());
    }

    /**
     * Ensure that {@link VTNVlanMapConfig} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<VTNVlanMapConfig> type = VTNVlanMapConfig.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);

        SalNode[] nodes = {
            null,
            new SalNode(1L),
            new SalNode(123456789L),
            new SalNode(-1L),
        };
        Integer[] vids = {
            null, 0, 1, 2, 100, 678, 1234, 3456, 4094, 4095,
        };

        VlanMapConfigBuilder builder = new VlanMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = (snode == null) ? null : snode.getNodeId();
            builder.setNode(node);
            for (Integer vid: vids) {
                VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                VtnVlanMapConfig vvmc = builder.setVlanId(vlanId).build();
                VTNVlanMapConfig vmc = new VTNVlanMapConfig(vvmc);
                VTNVlanMapConfig vmc1 = jaxbTest(vmc, type, m, um, XML_ROOT);
                vmc1.verify();
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // VLAN ID is missing.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vlan-id cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("node", "openflow:1")).
            toString();
        VTNVlanMapConfig vmc = unmarshal(um, xml, type);
        try {
            vmc.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }
}
