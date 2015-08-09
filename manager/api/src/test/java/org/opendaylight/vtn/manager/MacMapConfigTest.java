/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.Collection;
import java.util.Set;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

/**
 * JUnit test for {@link MacMapConfig}.
 */
public class MacMapConfigTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Set<DataLinkHost> notEmpty = null;
        for (Set<DataLinkHost> allow: createDataLinkHostSet(3)) {
            assertTrue(allow == null || !allow.isEmpty());
            for (Set<DataLinkHost> deny: createDataLinkHostSet(3)) {
                assertTrue(deny == null || !deny.isEmpty());

                MacMapConfig mcconf = new MacMapConfig(allow, deny);
                Set<DataLinkHost> a = mcconf.getAllowedHosts();
                Set<DataLinkHost> d = mcconf.getDeniedHosts();
                if (allow == null) {
                    assertTrue(a.isEmpty());
                } else {
                    assertEquals(allow, a);
                    assertNotSame(allow, a);
                }

                if (deny == null) {
                    assertTrue(d.isEmpty());
                } else {
                    assertEquals(deny, d);
                    assertNotSame(deny, d);
                }

                if (notEmpty == null && a != null && !a.isEmpty()) {
                    notEmpty = a;
                }
            }
        }

        // Test that specifies empty set.
        assertNotNull(notEmpty);
        assertTrue(!notEmpty.isEmpty());
        Set<DataLinkHost> empty = new HashSet<DataLinkHost>();
        MacMapConfig mcconf = new MacMapConfig(empty, notEmpty);
        assertTrue(mcconf.getAllowedHosts().isEmpty());
        Set<DataLinkHost> set = mcconf.getDeniedHosts();
        assertNotSame(notEmpty, set);
        assertEquals(notEmpty, set);

        mcconf = new MacMapConfig(notEmpty, empty);
        assertTrue(mcconf.getDeniedHosts().isEmpty());
        set = mcconf.getAllowedHosts();
        assertNotSame(notEmpty, set);
        assertEquals(notEmpty, set);
    }

    /**
     * Test case for {@link MacMapConfig#equals(Object)} and
     * {@link MacMapConfig#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Set<DataLinkHost>> allowed = createDataLinkHostSet(3);
        List<Set<DataLinkHost>> denied = createDataLinkHostSet(3);
        for (Set<DataLinkHost> allow: allowed) {
            for (Set<DataLinkHost> deny: denied) {
                MacMapConfig mc1 = new MacMapConfig(allow, deny);
                MacMapConfig mc2 = new MacMapConfig(
                    copy(allow, DataLinkHost.class),
                    copy(deny, DataLinkHost.class));
                testEquals(set, mc1, mc2);
            }
        }

        assertEquals(allowed.size() * denied.size(), set.size());
    }

    /**
     * Test case for {@link MacMapConfig#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "MacMapConfig[";
        String suffix = "]";
        for (Set<DataLinkHost> allow: createDataLinkHostSet(3)) {
            for (Set<DataLinkHost> deny: createDataLinkHostSet(3)) {
                MacMapConfig mcconf = new MacMapConfig(allow, deny);

                // Data link host addresses are kept by unordered set.
                // So a string representation of the object may be changed.
                String str = mcconf.toString();
                HashSet<String> aset = new HashSet<String>();
                HashSet<String> dset = new HashSet<String>();
                parseString(str, aset, dset);

                Collection<String> aexp = (allow == null)
                    ? new HashSet<String>()
                    : toStringCollection(allow);
                Collection<String> dexp = (deny == null)
                    ? new HashSet<String>()
                    : toStringCollection(deny);
                assertEquals(aexp, aset);
                assertEquals(dexp, dset);
            }
        }
    }

    /**
     * Ensure that {@link MacMapConfig} is serializable.
     */
    @Test
    public void testSerialize() {
        for (Set<DataLinkHost> allow: createDataLinkHostSet(3)) {
            for (Set<DataLinkHost> deny: createDataLinkHostSet(3)) {
                MacMapConfig mcconf = new MacMapConfig(allow, deny);
                serializeTest(mcconf);
            }
        }
    }

    /**
     * Parse a string returned by {@link MacMapConfig#toString()}.
     *
     * @param str    A string returned by {@link MacMapConfig#toString()}.
     * @param allow  A set of strings which represents allowed hosts.
     * @param deny   A set of strings which represents denied hosts.
     */
    private void parseString(String str, Set<String> allow, Set<String> deny) {
        String prefix = "MacMapConfig[";
        assertEquals(0, str.indexOf(prefix));
        int start = prefix.length();

        String name = "EthernetHost";
        String label = "allow=";
        assertTrue(str.startsWith(label, start));
        start = parseString(name, str, start + label.length(), allow);

        label = ",deny=";
        assertTrue(str.startsWith(label, start));
        start = parseString(name, str, start + label.length(), deny);

        assertEquals(']', str.charAt(start));
        start++;
        assertEquals(start, str.length());
    }
}
