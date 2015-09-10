/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathPolicy;
import org.opendaylight.vtn.manager.internal.util.AbstractConfigFileUpdater;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;

/**
 * {@code PathPolicyChange} describes changes to the path policy to be applied
 * to the configuration file.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class PathPolicyChange
    extends AbstractConfigFileUpdater<Integer, XmlPathPolicy> {
    /**
     * The network topology graph.
     */
    private final TopologyGraph  topology;

    /**
     * Construct a new instance.
     *
     * @param topo  The network topology graph.
     */
    PathPolicyChange(TopologyGraph topo) {
        super(XmlConfigFile.Type.PATHPOLICY, "Path policy");
        topology = topo;
    }

    // AbstractConfigFileUpdater

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean onUpdated(Integer key, XmlPathPolicy value) {
        // Update the route resolver for the given path policy.
        return topology.updateResolver(key);
    }

    /**
     * {@inheritDoc}
     */
    protected void onRemoved(Integer key) {
        // Remove route resolver for the given path policy.
        topology.removeResolver(key);
    }
}
