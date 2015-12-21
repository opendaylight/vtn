/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.controller.md.sal.common.api.clustering.Entity;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link VTNEntityType}.
 */
public class VTNEntityTypeTest extends TestBase {
    /**
     * Test case for {@link VTNEntityType#getGlobalEntity(VTNEntityType)}.
     */
    @Test
    public void testGetGlobalEntity() {
        VTNEntityType etype = VTNEntityType.CONFIG;
        Entity ent = VTNEntityType.getGlobalEntity(etype);
        assertEquals(etype.getType(), ent.getType());
        assertSame(ent, VTNEntityType.getGlobalEntity(etype));

        etype = VTNEntityType.INVENTORY;
        ent = VTNEntityType.getGlobalEntity(etype);
        assertEquals(etype.getType(), ent.getType());
        assertSame(ent, VTNEntityType.getGlobalEntity(etype));

        etype = VTNEntityType.FLOW_STATS;
        ent = VTNEntityType.getGlobalEntity(etype);
        assertEquals(etype.getType(), ent.getType());
        assertSame(ent, VTNEntityType.getGlobalEntity(etype));

        VTNEntityType[] invalid = {null, VTNEntityType.VBRIDGE};
        for (VTNEntityType et: invalid) {
            assertEquals(null, VTNEntityType.getGlobalEntity(et));
        }
    }

    /**
     * Test case for {@link VTNEntityType#getGlobalEntities()}.
     */
    @Test
    public void testGetGlobalEntities() {
        VTNEntityType[] globals = {
            VTNEntityType.CONFIG,
            VTNEntityType.INVENTORY,
            VTNEntityType.FLOW_STATS,
        };

        Collection<Entity> entities = VTNEntityType.getGlobalEntities();
        assertEquals(globals.length, entities.size());

        Set<Entity> set = new HashSet<>(entities);
        assertEquals(globals.length, set.size());

        for (VTNEntityType etype: globals) {
            Entity ent = VTNEntityType.getGlobalEntity(etype);
            assertTrue(set.remove(ent));
        }

        assertTrue(set.isEmpty());
    }

    /**
     * Test case for {@link VTNEntityType#getType()}.
     */
    @Test
    public void testGetType() {
        Set<String> set = new HashSet<>();
        for (VTNEntityType etype: VTNEntityType.values()) {
            // Every entity type must have "vtn:" prefix.
            String type = etype.getType();
            assertTrue(type.startsWith("vtn:"));

            // Entity type string must be unique.
            assertTrue(set.add(type));
        }
    }
}
