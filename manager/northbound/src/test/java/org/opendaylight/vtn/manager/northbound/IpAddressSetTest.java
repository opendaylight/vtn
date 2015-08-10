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
 * Junit test for {@link IpAddressSet}.
 */
public class IpAddressSetTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<Set<InetAddress>> ips = createInetAddresses();
        Set<IpAddress> addrSet;

        for (Set<InetAddress> ipset: ips) {
            addrSet = new HashSet<IpAddress>();
            if (ipset != null && ipset.size() != 0) {
                for (InetAddress ip: ipset) {
                    addrSet.add(new IpAddress(ip));
                }
                IpAddressSet iaddrs = new IpAddressSet(ipset);
                assertEquals(addrSet, iaddrs.getAddresses());
                assertSame(ipset.size(), iaddrs.getLength());
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
     * Test case for {@link IpAddressSet#equals(Object)} and {@link IpAddressSet#hashCode()}.
     */
    @Test
    public void testEquals() {
        List<Set<InetAddress>> ips = createInetAddresses(false);
        HashSet<Object> set = new HashSet<Object>();
        int required = 0;

        for (Set<InetAddress> ipset: ips) {
            IpAddressSet iaddrs1 = new IpAddressSet(ipset);
            IpAddressSet iaddrs2 = new IpAddressSet(ipset);

            testEquals(set, iaddrs1, iaddrs2);
            required++;
        }

        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link IpAddressSet} is mapped to both XML root element.
     */
    @Test
    public void testJAXB() {
        List<Set<InetAddress>> ips = createInetAddresses(false);
        for (Set<InetAddress> ipset: ips) {
            IpAddressSet iaddrs = new IpAddressSet(ipset);
            jaxbTest(iaddrs, IpAddressSet.class, "inetAddresses");
            jsonTest(iaddrs, IpAddressSet.class);
        }
    }
}
