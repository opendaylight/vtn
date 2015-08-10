/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.SwitchPort;

/**
 * JUnit test for {@link MacEntry}.
 */
public class MacEntryTest extends TestBase {
    /**
     * Root XML element name associated with {@link MacEntry} class.
     */
    private static final String  XML_ROOT = "macentry";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link MacEntry} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "vlan", short.class).
                  add(parent));
        return dlist;
    }

    /**
     * Test case for {@link MacEntry#MacEntry(MacAddressEntry)} and getter
     * method
     */
    @Test
    public void testGetter() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);

        // use a combination of NodeConnectors, InetAddresses,
        // EthernetAddresses, vlans as input parameter
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        String emsg = "(NodeConnector)" + nc.toString()
                                + "(ipset)" + ((ipset == null) ? "null" : ipset.toString())
                                + "(EthernetAddress)" + ea.toString();
                        MacAddressEntry mae = new MacAddressEntry(ea, vlan, nc, ipset);
                        MacEntry me = new MacEntry(mae);

                        String type = nc.getType();
                        String id = nc.getNodeConnectorIDString();

                        assertEquals(emsg, ea.getMacAddress(), me.getAddress());
                        assertEquals(emsg, vlan, me.getVlan());
                        assertEquals(emsg, nc.getNode(), me.getNode());
                        assertEquals(emsg, new SwitchPort(type, id), me.getPort());
                        IpAddressSet iset = me.getInetAddresses();
                        if (ipset != null && ipset.size() != 0) {
                            assertNotNull(emsg, iset);
                            assertEquals(emsg, ipset.size(), iset.getLength());
                            Set<IpAddress> is = iset.getAddresses();
                            for (InetAddress iaddr : ipset) {
                                IpAddress ipaddr = new IpAddress(iaddr);
                                assertTrue(emsg, is.contains(ipaddr));
                            }
                        } else {
                            assertNull(emsg, iset);
                        }
                    }
                }
            }
        }

        // test for DataLinkAddress class except EthrenetAddress.
        TestDataLink tdl = new TestDataLink("TestDataLink");
        for (NodeConnector nc : createNodeConnectors(3, false)) {
            String emsg = "(nc)" + nc.toString();
            short vlan = 0;
            Set<InetAddress> ipset = ips.get(1);
            MacAddressEntry mae = new MacAddressEntry(tdl, vlan, nc, ipset);
            MacEntry me = new MacEntry(mae);

            String type = nc.getType();
            String id = nc.getNodeConnectorIDString();
            SwitchPort port = new SwitchPort(type, id);

            assertEquals(emsg, tdl.toString(), me.getAddress());
            assertEquals(emsg, vlan, me.getVlan());
            assertEquals(emsg, nc.getNode(), me.getNode());
            assertEquals(emsg, port, me.getPort());

            IpAddressSet iset = me.getInetAddresses();
            assertNotNull(emsg, iset);
            assertEquals(emsg, ipset.size(), iset.getLength());
            Set<IpAddress> is = iset.getAddresses();
            for (InetAddress iaddr : ipset) {
                IpAddress ipaddr = new IpAddress(iaddr);
                assertTrue(emsg + ",(ipaddr)" + iaddr.toString(),
                        is.contains(ipaddr));
            }
        }
    }

    /**
     * test case for {@link MacEntry#equals(Object)} and
     * {@link MacEntry#hashCode()}
     */
    @Test
    public void testEquals() {
        int num = 3;
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses(false);
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<Node> nodes = createNodes(num, false);
        List<NodeConnector> connectors = null;

        HashSet<Object> set = new HashSet<Object>();

        for (Node node : nodes) {
            connectors = createNodeConnectors(num, node, false);
            for (NodeConnector nc : connectors) {
                for (Set<InetAddress> ipset : ips) {
                    for (EthernetAddress ea : ethaddrs) {
                        for (short vlan : vlans) {
                            MacAddressEntry mae1 = new MacAddressEntry(
                                ea, vlan, nc, ipset);
                            MacAddressEntry mae2 = new MacAddressEntry(
                                copy(ea), vlan, copy(nc),
                                copy(ipset, InetAddress.class));
                            MacEntry me1 = new MacEntry(mae1);
                            MacEntry me2 = new MacEntry(mae2);

                            testEquals(set, me1, me2);
                        }
                    }
                }
            }
        }

        assertEquals(nodes.size() * connectors.size() * ips.size() * ethaddrs.size() * vlans.length, set.size());
    }

    private List<NodeConnector> createNodeConnectors(int num, Node node, boolean setNull) {
        ArrayList<NodeConnector> list = new ArrayList<NodeConnector>();
        if (setNull) {
            list.add(null);
            num--;
        }

        for (int i = 0; i < num; i++) {
            String nodeType = node.getType();
            String connType = nodeType;
            Object connId;
            if (nodeType.equals(Node.NodeIDType.OPENFLOW)) {
                connId = new Short((short)(i + 10));
            } else {
                connId = "Node Connector ID: " + i;
            }

            try {
                NodeConnector nc = new NodeConnector(connType, connId, node);
                list.add(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return list;
    }

    /**
     * Ensure that {@link MacEntry} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        List<Set<InetAddress>> ips = createInetAddresses();
        List<EthernetAddress> ethaddrs = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);

        Set<InetAddress> ip = new HashSet<InetAddress>();
        ips.add(ip);
        for (NodeConnector nc : connectors) {
            for (Set<InetAddress> ipset : ips) {
                for (EthernetAddress ea : ethaddrs) {
                    for (short vlan : vlans) {
                        MacAddressEntry mae =
                            new MacAddressEntry(ea, vlan, nc, ipset);
                        MacEntry me = new MacEntry(mae);
                        jaxbTest(me, MacEntry.class, XML_ROOT);
                        jsonTest(me, MacEntry.class);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(MacEntry.class, getXmlDataTypes(XML_ROOT));
    }
}
