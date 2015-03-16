/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.cond;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.flow.cond.VTNFlowCondition;

/**
 * {@code FlowCondChange} describes changes to the flow condition
 * configuration.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class FlowCondChange {
    /**
     * A map that keeps updated flow conditions.
     */
    private final Map<String, VTNFlowCondition>  updatedConditions =
        new HashMap<>();

    /**
     * A map that keeps names for created flow conditions.
     */
    private final Set<String>  createdConditions = new HashSet<>();

    /**
     * A set of removed flow condition names.
     */
    private final Set<String>  removedConditions = new HashSet<>();

    /**
     * Add the updated flow condition.
     *
     * @param vfcond   A {@link VTNFlowCondition} instance.
     * @param created  {@code true} means that the given flow condition has
     *                 been newly created.
     */
    public void addUpdated(VTNFlowCondition vfcond, boolean created) {
        String name = vfcond.getIdentifier();
        if (!removedConditions.contains(name)) {
            updatedConditions.put(name, vfcond);
            if (created) {
                createdConditions.add(name);
            }
        }
    }

    /**
     * Add the removed flow condition.
     *
     * @param name  The name of the flow condition that has been removed.
     */
    public void addRemoved(String name) {
        if (removedConditions.add(name)) {
            updatedConditions.remove(name);
            createdConditions.remove(name);
        }
    }

    /**
     * Apply changes to the flow condition configuration.
     *
     * @param logger  A {@link Logger} instance.
     */
    public void apply(Logger logger) {
        XmlConfigFile.Type ftype = XmlConfigFile.Type.FLOWCOND;
        for (VTNFlowCondition vfcond: updatedConditions.values()) {
            // Save configuration into file.
            String name = vfcond.getIdentifier();
            XmlConfigFile.save(ftype, name, vfcond);
            logger.info("{}: Flow condition has been {}.", name,
                        (createdConditions.contains(name))
                        ? "created" : "updated");
        }
        for (String name: removedConditions) {
            // Remove configuration file.
            XmlConfigFile.delete(ftype, name);
            logger.info("{}: Path policy has been removed.", name);
        }
    }
}
