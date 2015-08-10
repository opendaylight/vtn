/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;


import java.net.InetAddress;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

/**
 * Junit test for {@link IpAddress}.
 */
public class IpAddressTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<Set<InetAddress>> ips = createInetAddresses();
        for (Set<InetAddress> ipset: ips) {
            if (ipset != null && ipset.size() != 0) {
                for (InetAddress iaddr: ipset) {
                    IpAddress ia = new IpAddress(iaddr);
                    assertEquals(iaddr.getHostAddress(), ia.getAddress());
                }
            } else {
                if (ipset == null) {
                    assertNull(ipset);
                } else {
                    assertEquals(0, ipset.size());
                }
            }
        }
    }

    /**
     * Test case for {@link IpAddress#equals(Object)} and {@link IpAddress#hashCode()}.
     */
    @Test
    public void testEquals() {
        List<Set<InetAddress>> ips = createInetAddresses();
        HashSet<Object> set = new HashSet<Object>();

        for (Set<InetAddress> ipset: ips) {
            set = new HashSet<Object>();
            if (ipset != null && ipset.size() != 0) {
                for (InetAddress iaddr: ipset) {
                    IpAddress ia1 = new IpAddress(iaddr);
                    IpAddress ia2 = new IpAddress(iaddr);

                    testEquals(set, ia1, ia2);
                }
                int required = ipset.size();
                assertEquals(required, set.size());
            } else {
                if (ipset == null) {
                    assertNull(ipset);
                } else {
                    assertEquals(0, ipset.size());
                }
            }
        }
    }

    /**
     * Ensure that {@link IpAddress} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        List<Set<InetAddress>> ips = createInetAddresses();
        for (Set<InetAddress> ipset: ips) {
            if (ipset != null && ipset.size() != 0) {
                for (InetAddress iaddr: ipset) {
                    IpAddress ia = new IpAddress(iaddr);
                    jaxbTest(ia, IpAddress.class, "inetAddress");
                    jsonTest(ia, IpAddress.class);
                }
            }
        }
    }
}
