/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.MacMapConfig;

/**
 * JUnit test for {@link MacMapConfigInfo}.
 */
public class MacMapConfigInfoTest extends TestBase {
    /**
     * Test case for {@link MacMapConfigInfo#MacMapConfigInfo(MacMapConfig)}
     * and getter methods.
     */
    @Test
    public void testGetter() {
        for (Set<EthernetHost> allow: createEthernetHostSet(10)) {
            assertTrue(allow == null || !allow.isEmpty());
            for (Set<EthernetHost> deny: createEthernetHostSet(10)) {
                assertTrue(deny == null || !deny.isEmpty());

                MacMapConfig mcconf = new MacMapConfig(allow, deny);
                MacMapConfigInfo mci = new MacMapConfigInfo(mcconf);
                MacHostSet a = mci.getAllowedHosts();
                MacHostSet d = mci.getDeniedHosts();
                if (allow == null) {
                    assertNull(a);
                } else {
                    assertEquals(new MacHostSet(mcconf.getAllowedHosts()), a);
                }

                if (deny == null) {
                    assertNull(d);
                } else {
                    assertEquals(new MacHostSet(mcconf.getDeniedHosts()), d);
                }
            }
        }
    }

    /**
     * test case for {@link MacMapConfigInfo#equals(Object)} and
     * {@link MacMapConfigInfo#hashCode()}
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<Set<EthernetHost>> allowed = createEthernetHostSet(10);
        List<Set<EthernetHost>> denied = createEthernetHostSet(10);
        for (Set<EthernetHost> allow: allowed) {
            for (Set<EthernetHost> deny: denied) {
                MacMapConfig mc1 = new MacMapConfig(allow, deny);
                MacMapConfig mc2 = new MacMapConfig(
                    copy(allow, EthernetHost.class),
                    copy(deny, EthernetHost.class));
                MacMapConfigInfo mci1 = new MacMapConfigInfo(mc1);
                MacMapConfigInfo mci2 = new MacMapConfigInfo(mc2);
                testEquals(set, mci1, mci2);
            }
        }

        assertEquals(allowed.size() * denied.size(), set.size());
    }

    /**
     * Ensure that {@link MacMapConfigInfo} is mapped to both XML root element
     * and JSON object.
     */
    @Test
    public void testJAXB() {
        for (Set<EthernetHost> allow: createEthernetHostSet(10)) {
            for (Set<EthernetHost> deny: createEthernetHostSet(10)) {
                MacMapConfig mcconf = new MacMapConfig(allow, deny);
                MacMapConfigInfo mci = new MacMapConfigInfo(mcconf);
                jaxbTest(mci, MacMapConfigInfo.class, "macmapconf");
                jsonTest(mci, MacMapConfigInfo.class);
            }
        }
    }
}
