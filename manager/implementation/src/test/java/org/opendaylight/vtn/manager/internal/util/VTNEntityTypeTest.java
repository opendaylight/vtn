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

        etype = VTNEntityType.FLOW_STATS;
        ent = VTNEntityType.getGlobalEntity(etype);
        assertEquals(etype.getType(), ent.getType());
        assertSame(ent, VTNEntityType.getGlobalEntity(etype));

        assertEquals(null, VTNEntityType.getGlobalEntity(null));
    }

    /**
     * Test case for {@link VTNEntityType#getGlobalEntities()}.
     */
    @Test
    public void testGetGlobalEntities() {
        Collection<Entity> entities = VTNEntityType.getGlobalEntities();
        assertEquals(2, entities.size());

        Set<Entity> set = new HashSet<>(entities);
        assertEquals(2, set.size());

        VTNEntityType etype = VTNEntityType.CONFIG;
        Entity ent = VTNEntityType.getGlobalEntity(etype);
        assertTrue(set.remove(ent));

        etype = VTNEntityType.FLOW_STATS;
        ent = VTNEntityType.getGlobalEntity(etype);
        assertTrue(set.remove(ent));

        assertTrue(set.isEmpty());
    }
}
