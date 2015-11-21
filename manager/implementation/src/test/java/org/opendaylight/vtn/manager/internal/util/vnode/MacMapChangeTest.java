/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.HashSet;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link MacMapChange}.
 */
public class MacMapChangeTest extends TestBase {
    /**
     * A mask which contains all valid flags for {@link MacMapChange}.
     */
    private static final int  ALL_FLAGS =
        MacMapChange.REMOVING | MacMapChange.DONT_PURGE;

    /**
     * Test case for {@link MacMapChange#MacMapChange(Set, Set, Set, Set, int)}.
     */
    @Test
    public void testConstructor1() {
        long mac = 1L;
        short vlan = 0;
        Set<MacVlan> allowAdded = new HashSet<MacVlan>();
        for (int i = 0; i < 5; i++) {
            allowAdded.add(new MacVlan(mac, vlan));
            mac++;
            vlan++;
        }

        Set<MacVlan> allowRemoved = new HashSet<MacVlan>();
        for (int i = 0; i < 5; i++) {
            allowRemoved.add(new MacVlan(mac, vlan));
            mac++;
            vlan++;
        }

        Set<MacVlan> denyAdded = new HashSet<MacVlan>();
        for (int i = 0; i < 5; i++) {
            denyAdded.add(new MacVlan(mac, vlan));
            mac++;
            vlan++;
        }

        Set<MacVlan> denyRemoved = new HashSet<MacVlan>();
        for (int i = 0; i < 5; i++) {
            denyRemoved.add(new MacVlan(mac, vlan));
            mac++;
            vlan++;
        }

        for (int flags = 0; flags <= ALL_FLAGS; flags++) {
            MacMapChange change = new MacMapChange(allowAdded, allowRemoved,
                                                   denyAdded, denyRemoved,
                                                   flags);
            assertEquals(allowAdded, change.getAllowAddedSet());
            assertEquals(allowRemoved, change.getAllowRemovedSet());
            assertEquals(denyAdded, change.getDenyAddedSet());
            assertEquals(denyRemoved, change.getDenyRemovedSet());
            assertEquals((flags & MacMapChange.REMOVING) != 0,
                         change.isRemoving());
            assertEquals((flags & MacMapChange.DONT_PURGE) != 0,
                         change.dontPurge());
        }
    }

    /**
     * Test case for {@link MacMapChange#MacMapChange(Set, Set, int)}.
     */
    @Test
    public void testConstructor2() {
        long mac = 1L;
        short vlan = 0;
        Set<MacVlan> allow = new HashSet<MacVlan>();
        for (int i = 0; i < 5; i++) {
            allow.add(new MacVlan(mac, vlan));
            mac++;
            vlan++;
        }

        Set<MacVlan> deny = new HashSet<MacVlan>();
        for (int i = 0; i < 5; i++) {
            deny.add(new MacVlan(mac, vlan));
            mac++;
            vlan++;
        }

        for (int flags = 0; flags <= ALL_FLAGS; flags++) {
            MacMapChange change = new MacMapChange(allow, deny, flags);
            boolean removing;
            if ((flags & MacMapChange.REMOVING) == 0) {
                assertEquals(allow, change.getAllowAddedSet());
                assertTrue(change.getAllowRemovedSet().isEmpty());
                assertEquals(deny, change.getDenyAddedSet());
                assertTrue(change.getDenyRemovedSet().isEmpty());
                removing = false;
            } else {
                assertTrue(change.getAllowAddedSet().isEmpty());
                assertEquals(allow, change.getAllowRemovedSet());
                assertTrue(change.getDenyAddedSet().isEmpty());
                assertEquals(deny, change.getDenyRemovedSet());
                removing = true;
            }
            assertEquals(removing, change.isRemoving());
            assertEquals((flags & MacMapChange.DONT_PURGE) != 0,
                         change.dontPurge());
        }
    }
}
