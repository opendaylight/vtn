/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.PathPolicy;

import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;

/**
 * {@code PathPolicyChange} describes changes to the path policy configuration.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class PathPolicyChange {
    /**
     * A map that keeps updated path policies.
     */
    private final Map<Integer, VtnPathPolicy> updatedPolicies =
        new HashMap<>();

    /**
     * A set of identifiers for removed path policies.
     */
    private final Set<Integer> removedPolicies = new HashSet<>();

    /**
     * Add the updated path policy.
     *
     * @param id  An {@link Integer} that represents the path policy ID.
     * @param vpp  A {@link VtnPathPolicy} instance.
     */
    public void addUpdated(Integer id, VtnPathPolicy vpp) {
        if (!removedPolicies.contains(id)) {
            updatedPolicies.put(id, vpp);
        }
    }

    /**
     * Add the removed path policy.
     *
     * @param id  An {@link Integer} that represents the path policy ID.
     */
    public void addRemoved(Integer id) {
        if (removedPolicies.add(id)) {
            updatedPolicies.remove(id);
        }
    }

    /**
     * Apply changes to the path policy configuration.
     *
     * @param topo    A {@link TopologyGraph} instance.
     * @param logger  A {@link Logger} instance.
     */
    public void apply(TopologyGraph topo, Logger logger) {
        XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHPOLICY;
        for (Map.Entry<Integer, VtnPathPolicy> entry:
                 updatedPolicies.entrySet()) {
            Integer id = entry.getKey();
            VtnPathPolicy vpp = entry.getValue();

            // Update the route resolver for the path policy.
            boolean created = topo.updateResolver(id);

            // Save configuration into file.
            PathPolicy pp = PathPolicyUtils.toPathPolicy(vpp);
            XmlConfigFile.save(ftype, id.toString(), pp);
            logger.info("{}: Path policy has been {}.", id,
                        (created) ? "created" : "updated");
        }
        for (Integer id: removedPolicies) {
            // Remove route resolver for the path policy.
            topo.removeResolver(id);

            // Remove configuration file.
            XmlConfigFile.delete(ftype, id.toString());
            logger.info("{}: Path policy has been removed.", id);
        }
    }
}
