/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;

import org.opendaylight.controller.northbound.commons.exception.
    InternalServerErrorException;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * JUnit test for {@link MacHost}.
 */
public class MacHostTest extends TestBase {
    /**
     * Root XML element name associated with {@link MacHost} class.
     */
    private static final String  XML_ROOT = "machost";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link MacHost} instance.
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
     * Test case for {@link MacHost#MacHost(DataLinkHost)} and getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (DataLinkAddress dladdr: createDataLinkAddresses()) {
            if (dladdr == null || (dladdr instanceof EthernetAddress)) {
                EthernetAddress eaddr = (EthernetAddress)dladdr;
                String saddr = (dladdr == null)
                    ? null : eaddr.getMacAddress();
                for (short vlan : vlans) {
                    EthernetHost ehost = new EthernetHost(eaddr, vlan);
                    MacHost host = new MacHost(ehost);

                    assertEquals(saddr, host.getAddress());
                    assertEquals(vlan, host.getVlan());
                }
            } else {
                for (short vlan : vlans) {
                    TestDataLinkHost dlhost = new TestDataLinkHost(dladdr);
                    try {
                        MacHost host = new MacHost(dlhost);
                        fail("An exception must be thrown.");
                    } catch (InternalServerErrorException e) {
                    }
                }
            }
        }
    }

    /**
     * test case for {@link MacHost#equals(Object)} and
     * {@link MacHost#hashCode()}
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<EthernetAddress> addresses = createEthernetAddresses();
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: addresses) {
            for (short vlan : vlans) {
                DataLinkHost dh1 = new EthernetHost(eaddr, vlan);
                DataLinkHost dh2 =
                    new EthernetHost((EthernetAddress)copy(eaddr), vlan);
                MacHost h1 = new MacHost(dh1);
                MacHost h2 = new MacHost(dh2);
                testEquals(set, h1, h2);
            }
        }

        assertEquals(addresses.size() * vlans.length, set.size());
    }

    /**
     * Ensure that {@link MacHost} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (EthernetAddress eaddr: createEthernetAddresses()) {
            for (short vlan : vlans) {
                DataLinkHost dlhost = new EthernetHost(eaddr, vlan);
                MacHost host = new MacHost(dlhost);
                jaxbTest(host, MacHost.class, XML_ROOT);
                jsonTest(host, MacHost.class);
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(MacHost.class, getXmlDataTypes(XML_ROOT));
    }
}
