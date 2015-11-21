/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestVlanId;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortLocation;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link VTNPortMapConfig}.
 */
public class VTNPortMapConfigTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNPortMapConfig} class.
     */
    private static final String  XML_ROOT = "vtn-port-map-config";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNPortMapConfig} instance.
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
     * Test case for
     * {@link VTNPortMapConfig#VTNPortMapConfig(VtnPortMapConfig)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(1234567890L),
            new SalNode(-1L),
        };
        String[] portIds = {
            null, "1", "2", "12345", "4294967040",
        };
        String[] portNames = {
            null, "port-1", "port-2", "port-123", "a,b,c",
        };
        Integer[] vids = {
            null, 0, 1, 2, 100, 789, 1234, 3344, 4094, 4095,
        };

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "Target port name or ID must be specified";
        PortMapConfigBuilder builder = new PortMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = snode.getNodeId();
            builder.setNode(node);
            for (String id: portIds) {
                builder.setPortId(id);
                for (String name: portNames) {
                    builder.setPortName(name);
                    for (Integer vid: vids) {
                        VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                        VtnPortMapConfig vpmc =
                            builder.setVlanId(vlanId).build();
                        if (id == null && name == null) {
                            try {
                                new VTNPortMapConfig(vpmc);
                                unexpected();
                            } catch (RpcException e) {
                                assertEquals(etag, e.getErrorTag());
                                assertEquals(vtag, e.getVtnErrorTag());
                                assertEquals(msg, e.getMessage());
                            }
                        } else {
                            VTNPortMapConfig pmc = new VTNPortMapConfig(vpmc);
                            assertEquals(snode, pmc.getTargetNode());
                            assertEquals(id, pmc.getPortId());
                            assertEquals(name, pmc.getPortName());
                            int exVid = (vid == null) ? 0 : vid;
                            assertEquals(exVid, pmc.getVlanId());
                            assertEquals(VtnSwitchPort.class,
                                         pmc.getImplementedInterface());
                        }
                    }
                }
            }
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
            msg = "Invalid node ID: " + bad;
            NodeId node = new NodeId(bad);
            VtnPortMapConfig vpmc = new PortMapConfigBuilder().
                setNode(node).setPortName("port-1").build();
            try {
                new VTNPortMapConfig(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid VLAN ID.
        Integer[] badVids = {
            Integer.MIN_VALUE, -2, -1, 4096, 4097, 100000, Integer.MAX_VALUE,
        };
        for (Integer vid: badVids) {
            msg = "Invalid VLAN ID: " + vid;
            VlanId vlanId = new TestVlanId(vid);
            VtnPortMapConfig vpmc = mock(VtnPortMapConfig.class);
            when(vpmc.getNode()).thenReturn(new NodeId("openflow:1"));
            when(vpmc.getPortId()).thenReturn("1");
            when(vpmc.getPortName()).thenReturn("port-1");
            when(vpmc.getVlanId()).thenReturn(vlanId);

            try {
                new VTNPortMapConfig(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
            verify(vpmc).getNode();
            verify(vpmc).getPortId();
            verify(vpmc).getPortName();
            verify(vpmc).getVlanId();
            verifyNoMoreInteractions(vpmc);
        }

        // Invalid port ID.
        String[] badPortIds = {
            "", "bad port ID", "99999999999999999999999", "4294967041",
        };
        for (String portId: badPortIds) {
            msg = "Invalid port ID: " + portId;
            VtnPortMapConfig vpmc = new PortMapConfigBuilder().
                setNode(new NodeId("openflow:1")).
                setPortId(portId).
                setPortName("port-1").
                build();

            try {
                new VTNPortMapConfig(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Empty port name.
        msg = "Port name cannot be empty";
        VtnPortMapConfig vpmc = mock(VtnPortMapConfig.class);
        when(vpmc.getNode()).thenReturn(new NodeId("openflow:1"));
        when(vpmc.getPortId()).thenReturn("1");
        when(vpmc.getPortName()).thenReturn("");
        when(vpmc.getVlanId()).thenReturn(new VlanId(0));
        try {
            new VTNPortMapConfig(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verify(vpmc).getNode();
        verify(vpmc).getPortId();
        verify(vpmc).getPortName();
        verify(vpmc).getVlanId();
        verifyNoMoreInteractions(vpmc);

        // Node ID is missing.
        etag = RpcErrorTag.MISSING_ELEMENT;
        msg = "Node cannot be null";
        vpmc = new PortMapConfigBuilder().setPortName("port-1").build();
        try {
            new VTNPortMapConfig(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // VLAN ID is missing.
        msg = "vlan-id cannot be null";
        VlanId vlanId = new TestVlanId();
        vpmc = mock(VtnPortMapConfig.class);
        when(vpmc.getNode()).thenReturn(new NodeId("openflow:1"));
        when(vpmc.getPortId()).thenReturn("1");
        when(vpmc.getPortName()).thenReturn("port-1");
        when(vpmc.getVlanId()).thenReturn(vlanId);
        try {
            new VTNPortMapConfig(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verify(vpmc).getNode();
        verify(vpmc).getPortId();
        verify(vpmc).getPortName();
        verify(vpmc).getVlanId();
        verifyNoMoreInteractions(vpmc);
    }

    /**
     * Test case for {@link VTNPortMapConfig#toPortMapConfig()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToPortMapConfig() throws Exception {
        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(1234567890L),
            new SalNode(-1L),
        };
        String[] portIds = {
            null, "1", "2", "12345", "4294967040",
        };
        String[] portNames = {
            null, "port-1", "port-2", "port-123", "a,b,c",
        };
        Integer[] vids = {
            null, 0, 1, 2, 100, 789, 1234, 3344, 4094, 4095,
        };

        PortMapConfigBuilder builder = new PortMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = snode.getNodeId();
            builder.setNode(node);
            for (String id: portIds) {
                builder.setPortId(id);
                for (String name: portNames) {
                    if (id == null && name == null) {
                        continue;
                    }
                    builder.setPortName(name);
                    for (Integer vid: vids) {
                        VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                        VtnPortMapConfig vpmc =
                            builder.setVlanId(vlanId).build();
                        VTNPortMapConfig pmc = new VTNPortMapConfig(vpmc);
                        Integer exVid = (vid == null) ? 0 : vid;
                        VtnPortLocation vpl = vpmc;
                        PortMapConfig expected = new PortMapConfigBuilder(vpl).
                            setVlanId(new VlanId(exVid)).build();
                        assertEquals(expected, pmc.toPortMapConfig());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VTNPortMapConfig#match(SalPort, VtnPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        SalNode targetNode = new SalNode(123L);
        SalPort targetPort = new SalPort(123L, 456L);
        String portName = "port-456";

        // In case where the port name is not specified.
        String portId = "456";
        PortMapConfig pmc = new PortMapConfigBuilder().
            setNode(targetNode.getNodeId()).
            setPortId(portId).
            build();
        VTNPortMapConfig vpmc = new VTNPortMapConfig(pmc);

        long[] nodeNums = {1L, 122L, 123L, 124L, -1L};
        long[] portNums = {1L, 2L, 345L, 455L, 456L, 457L, 1234567L};
        String[] portNames = {
            null, "port-1", "port-455", "port-456", "port-4567",
        };
        for (long dpid: nodeNums) {
            for (long pnum: portNums) {
                for (String pname: portNames) {
                    SalPort sport = new SalPort(dpid, pnum);
                    VtnPort vport = mock(VtnPort.class);
                    when(vport.getName()).thenReturn(pname);
                    assertEquals(sport.equals(targetPort),
                                 vpmc.match(sport, vport));
                }
            }
        }

        // In case where the port ID is not specified.
        pmc = new PortMapConfigBuilder().
            setNode(targetNode.getNodeId()).
            setPortName(portName).
            build();
        vpmc = new VTNPortMapConfig(pmc);
        for (long dpid: nodeNums) {
            for (long pnum: portNums) {
                for (String pname: portNames) {
                    SalPort sport = new SalPort(dpid, pnum);
                    VtnPort vport = mock(VtnPort.class);
                    when(vport.getName()).thenReturn(pname);
                    boolean expected = (portName.equals(pname) &&
                                        dpid == targetNode.getNodeNumber());
                    assertEquals(expected, vpmc.match(sport, vport));
                }
            }
        }

        // In case where both the port ID and name is specified.
        pmc = new PortMapConfigBuilder().
            setNode(targetNode.getNodeId()).
            setPortId(portId).
            setPortName(portName).
            build();
        vpmc = new VTNPortMapConfig(pmc);
        for (long dpid: nodeNums) {
            for (long pnum: portNums) {
                for (String pname: portNames) {
                    SalPort sport = new SalPort(dpid, pnum);
                    VtnPort vport = mock(VtnPort.class);
                    when(vport.getName()).thenReturn(pname);
                    boolean expected = (portName.equals(pname) &&
                                        sport.equals(targetPort));
                    assertEquals(expected, vpmc.match(sport, vport));
                }
            }
        }
    }

    /**
     * Test case for {@link VTNPortMapConfig#getMappedVlan(SalPort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetMappedVlan() throws Exception {
        long dpid = 10L;
        SalNode snode = new SalNode(dpid);
        String portName = "port-name";

        int[] vids = {
            0, 1, 2, 100, 789, 1234, 3344, 4094, 4095,
        };
        long[] portNumbers = {1L, 2L, 10L, 123456L};

        for (int vid: vids) {
            PortMapConfig pmc = new PortMapConfigBuilder().
                setNode(snode.getNodeId()).
                setPortName(portName).
                setVlanId(new VlanId(vid)).
                build();
            VTNPortMapConfig vpmc = new VTNPortMapConfig(pmc);
            assertEquals(null, vpmc.getMappedVlan(null));
            for (long pnum: portNumbers) {
                SalPort sport = new SalPort(dpid, pnum);
                PortVlan pv = vpmc.getMappedVlan(sport);
                assertEquals(sport, pv.getPort());
                assertEquals(vid, pv.getVlanId());
            }
        }
    }

    /**
     * Test case for {@link VTNPortMapConfig#equals(Object)} and
     * {@link VTNPortMapConfig#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(1234567890L),
            new SalNode(-1L),
        };
        String[] portIds = {
            null, "1", "2", "12345", "4294967040",
        };
        String[] portNames = {
            null, "port-1", "port-2", "port-123", "a,b,c",
        };
        Integer[] vids = {
            0, 1, 2, 100, 789, 1234, 3344, 4094, 4095,
        };

        int count = 0;
        PortMapConfigBuilder builder = new PortMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = snode.getNodeId();
            builder.setNode(node);
            for (String id: portIds) {
                builder.setPortId(id);
                for (String name: portNames) {
                    if (id == null && name == null) {
                        continue;
                    }
                    builder.setPortName(name);
                    for (Integer vid: vids) {
                        VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                        VtnPortMapConfig vpmc =
                            builder.setVlanId(vlanId).build();
                        VTNPortMapConfig pmc1 = new VTNPortMapConfig(vpmc);
                        VTNPortMapConfig pmc2 = new VTNPortMapConfig(vpmc);
                        testEquals(set, pmc1, pmc2);
                        count++;
                    }
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Ensure that {@link VTNPortMapConfig} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<VTNPortMapConfig> type = VTNPortMapConfig.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);

        SalNode[] nodes = {
            new SalNode(1L),
            new SalNode(1234567890L),
            new SalNode(-1L),
        };
        String[] portIds = {
            null, "1", "2", "12345", "4294967040",
        };
        String[] portNames = {
            null, "port-1", "port-2", "port-123", "a,b,c",
        };
        Integer[] vids = {
            0, 1, 2, 100, 789, 1234, 3344, 4094, 4095,
        };

        PortMapConfigBuilder builder = new PortMapConfigBuilder();
        for (SalNode snode: nodes) {
            NodeId node = snode.getNodeId();
            builder.setNode(node);
            for (String id: portIds) {
                builder.setPortId(id);
                for (String name: portNames) {
                    if (id == null && name == null) {
                        continue;
                    }
                    builder.setPortName(name);
                    for (Integer vid: vids) {
                        VlanId vlanId = (vid == null) ? null : new VlanId(vid);
                        VtnPortMapConfig vpmc =
                            builder.setVlanId(vlanId).build();
                        VTNPortMapConfig pmc = new VTNPortMapConfig(vpmc);
                        VTNPortMapConfig pmc1 =
                            jaxbTest(pmc, type, m, um, XML_ROOT);
                        pmc1.verify();
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // Node ID is missing.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "Node cannot be null";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("vlan-id", 0)).
            add(new XmlNode("port-name", "port-1")).
            toString();
        VTNPortMapConfig pmc = unmarshal(um, xml, type);
        try {
            pmc.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // VLAN ID is missing.
        msg = "vlan-id cannot be null";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("node", "openflow:1")).
            add(new XmlNode("port-name", "port-1")).
            toString();
        pmc = unmarshal(um, xml, type);
        try {
            pmc.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Empty port name.
        etag = RpcErrorTag.BAD_ELEMENT;
        msg = "Port name cannot be empty";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("node", "openflow:1")).
            add(new XmlNode("port-name", "")).
            add(new XmlNode("vlan-id", 0)).
            toString();
        pmc = unmarshal(um, xml, type);
        try {
            pmc.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid port ID.
        String[] badPortIds = {
            "", "bad port ID", "99999999999999999999999", "4294967041",
        };
        for (String portId: badPortIds) {
            msg = "Invalid port ID: " + portId;
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("node", "openflow:1")).
                add(new XmlNode("port-id", portId)).
                add(new XmlNode("port-name", "port-1")).
                add(new XmlNode("vlan-id", 0)).
                toString();
            pmc = unmarshal(um, xml, type);
            try {
                pmc.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Both port ID and name are missing.
        msg = "Target port name or ID must be specified";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("node", "openflow:1")).
            add(new XmlNode("vlan-id", 0)).
            toString();
        pmc = unmarshal(um, xml, type);
        try {
            pmc.verify();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }
}
