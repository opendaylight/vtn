/**
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VTNException;


/**
 * JUnit test for {@link MacTableEntry}
 */
public class MacTableEntryTest extends TestBase {

    @Rule
    public ExpectedException thrown = ExpectedException.none();

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short vlans[] = {-10, 0, 1, 100, 4095};

        for (NodeConnector nc: createNodeConnectors(3, false)) {
            for (short vlan: vlans) {
                for (Set<InetAddress> ips: createInetAddresses(false)) {
                    MacTableEntry me = null;

                    if (ips.size() == 0) {
                        me = new MacTableEntry(copy(nc), vlan, null);
                        assertNotNull(me);
                    } else {
                        boolean first = true;
                        for (InetAddress ip: ips) {
                            if (first) {
                                me = new MacTableEntry(copy(nc), vlan, ip);
                                assertNotNull(me);
                                assertEquals(nc, me.getPort());
                                assertEquals(vlan, me.getVlan());

                                first = false;
                            } else {
                                assertTrue(me.addInetAddress(ip));
                            }
                        }
                    }

                    for (EthernetAddress ea: createEthernetAddresses(false)) {

                        // test for getEntry()
                        try {
                            long eval =  NetUtils.byteArray6ToLong(ea.getValue());
                            MacAddressEntry mae1 = me.getEntry(eval);
                            MacAddressEntry mae2 = new MacAddressEntry(copy(ea), vlan, copy(nc), copy(ips));
                            assertEquals(mae2, mae1);
                        } catch (VTNException e) {
                            unexpected(e);
                        }
                    }

                    for (short newvlan: vlans) {
                        me.setVlan(newvlan);
                        assertEquals(newvlan, me.getVlan());
                    }

                    for (NodeConnector newnc: createNodeConnectors(3, false)) {
                        me.setPort(newnc);
                        assertEquals(newnc, me.getPort());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link MacTableEntry#setUsed} and {@link MacTableEntry#clearUsed}
     */
    @Test
    public void testUsed() {
        short vlan = 0;
        List<NodeConnector> connectors = createNodeConnectors(1, false);
        NodeConnector nc = connectors.get(0);
        List<Set<InetAddress>> ips = createInetAddresses(false);

        Iterator<InetAddress> ip = ips.get(0).iterator();
        MacTableEntry me = new MacTableEntry(nc, vlan, ip.next());

        assertTrue(me.clearUsed());
        me.setUsed();
        assertTrue(me.clearUsed());
        assertFalse(me.clearUsed());
    }

    /**
     * Test case for {@link MacTableEntry#toString()}
     */
    @Test
    public void testToString() {
        String prefix = "MacTableEntry[";
        String suffix = "]";
        short vlans[] = {-10, 0, 1, 100, 4095};

        for (NodeConnector nc: createNodeConnectors(3, false)) {
            for (short vlan: vlans) {
                for (Set<InetAddress> ips: createInetAddresses(false)) {
                    StringBuffer i = new StringBuffer("ipaddr={");
                    MacTableEntry me = null;
                    ArrayList<String> list = new ArrayList<String>();
                    HashSet<InetAddress> hash = new HashSet<InetAddress>();

                    for (InetAddress ip: ips) {
                        hash.add(ip);
                    }

                    if (ips.size() == 0) {
                        me = new MacTableEntry(nc, vlan, null);
                        i.append("}");
                    } else {
                        boolean first = true;
                        for (InetAddress ip: hash) {
                            if (first) {
                                if (ip == null) {
                                    me = new MacTableEntry(nc, vlan, null);
                                } else {
                                    me = new MacTableEntry(nc, vlan, ip);
                                    i.append(ip.getHostAddress());
                                }
                                first = false;
                            } else {
                                me.addInetAddress(ip);
                                i.append(",");
                                i.append(ip.getHostAddress());
                            }
                        }
                        i.append("}");
                    }

                    String p ="port=" + nc.toString();
                    String v = "vlan=" + vlan;
                    String required = joinStrings(prefix, suffix, ",", p, v, i);

                    assertEquals(required, me.toString());
                }
            }
        }
    }
}