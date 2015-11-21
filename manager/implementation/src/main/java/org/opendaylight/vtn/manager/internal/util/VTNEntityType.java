/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.Collection;
import java.util.EnumMap;
import java.util.Map;

import com.google.common.collect.ImmutableMap;

import org.opendaylight.controller.md.sal.common.api.clustering.Entity;

/**
 * {@code VTNEntityType} describes entities shared by VTN Managers in a
 * cluster.
 */
public enum VTNEntityType {
    /**
     * The global entity type that indicates the VTN configuration.
     */
    CONFIG("vtn:config"),

    /**
     * The global entity type that indicates the timer for updating flow
     * statistics.
     */
    FLOW_STATS("vtn:flow-stats"),

    /**
     * The entity type that indicates the vBridge.
     *
     * <p>
     *   One entity of this type is associated with a vBridge.
     * </p>
     */
    VBRIDGE("vtn:vbridge");

    /**
     * A map that keeps global entities.
     */
    private static final Map<VTNEntityType, Entity>  GLOBAL_ENTITIES;

    /**
     * Initialize global entities.
     */
    static {
        Map<VTNEntityType, Entity> map = new EnumMap<>(VTNEntityType.class);
        VTNEntityType etype = VTNEntityType.CONFIG;
        String type = etype.getType();
        map.put(etype, new Entity(type, type));

        etype = VTNEntityType.FLOW_STATS;
        type = etype.getType();
        map.put(etype, new Entity(type, type));

        GLOBAL_ENTITIES = ImmutableMap.copyOf(map);
    }

    /**
     * The type of the entity.
     */
    private final String  entityType;

    /**
     * Return a global entity associated with the given type.
     *
     * @param etype  A VTN global entity type.
     * @return  An {@link Entity} instance if found.
     *          {@code null} if not found.
     */
    public static Entity getGlobalEntity(VTNEntityType etype) {
        return GLOBAL_ENTITIES.get(etype);
    }

    /**
     * Return an unmodifiable collection of global entities.
     *
     * @return  An unmodifiable collection of global entities.
     */
    public static Collection<Entity> getGlobalEntities() {
        return GLOBAL_ENTITIES.values();
    }

    /**
     * Construct a new instance.
     *
     * @param type  The type of the entity.
     */
    private VTNEntityType(String type) {
        entityType = type;
    }

    /**
     * Return the type of entity.
     *
     * @return  The type of entity.
     */
    public String getType() {
        return entityType;
    }
}
