/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.pathpolicy;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * {@code PathCost} describes the cost of using specific switch link for
 * packet transmission.
 */
public final class PathCost {
    /**
     * The default value of the path cost.
     */
    private static final Long  DEFAULT_COST = 1L;

    /**
     * Field separator of vtn-port-desc.
     */
    private static final char  PORT_DESC_SEPARATOR = ',';

    /**
     * The MD-SAL node identifier.
     */
    private final String  nodeId;

    /**
     * The identifier for the physical switch port.
     */
    private final String  portId;

    /**
     * The name of the physical switch port.
     */
    private final String  portName;

    /**
     * The cost of using physical switch port.
     */
    private Long  pathCost;

    /**
     * The physical switch port descriptor.
     */
    private VtnPortDesc  portDesc;

    /**
     * Convert the given path costs into a list of {@link PathCostList}
     * instance.
     *
     * @param costs  A collection of {@link PathCost} instances.
     * @return  A list of {@link PathCostList} instances or {@code null}.
     */
    public static List<PathCostList> toPathCostList(
        Collection<PathCost> costs) {
        List<PathCostList> pcl;
        if (costs == null) {
            pcl = null;
        } else {
            pcl = new ArrayList<>(costs.size());
            for (PathCost pc: costs) {
                pcl.add((pc == null) ? null : pc.toPathCostList());
            }
        }

        return pcl;
    }

    /**
     * Convert the given path costs into a list of {@link PathCostList}
     * instance.
     *
     * @param costs  An array of {@link PathCost} instances.
     * @return  A list of {@link PathCostList} instances or {@code null}.
     */
    public static List<PathCostList> toPathCostList(PathCost ... costs) {
        List<PathCostList> pcl;
        if (costs == null) {
            pcl = null;
        } else {
            pcl = new ArrayList<>(costs.length);
            for (PathCost pc: costs) {
                pcl.add((pc == null) ? null : pc.toPathCostList());
            }
        }

        return pcl;
    }

    /**
     * Construct an empty instance.
     */
    public PathCost() {
        this(null, null, null, null);
    }

    /**
     * Construct a new instance without specifying port descriptor.
     *
     * @param cost  The cost of using physical switch port.
     */
    public PathCost(Long cost) {
        this(null, null, null, cost);
    }

    /**
     * Construct a new instance with the default path cost.
     *
     * @param node  The MD-SAL node identifier.
     * @param id    The identifier for the physical switch port.
     * @param name  The name of the physical switch port.
     */
    public PathCost(String node, String id, String name) {
        this(node, id, name, null);
    }

    /**
     * Construct a new instance.
     *
     * @param node  The MD-SAL node identifier.
     * @param id    The identifier for the physical switch port.
     * @param name  The name of the physical switch port.
     * @param cost  The cost of using physical switch port.
     */
    public PathCost(String node, String id, String name, Long cost) {
        nodeId = node;
        portId = id;
        portName = name;
        pathCost = cost;
    }

    /**
     * Return the node identifier.
     *
     * @return  The MD-SAL node identifier.
     */
    public String getNodeId() {
        return nodeId;
    }

    /**
     * Return the identifier for the physical switch port.
     *
     * @return  The identifier for the physical switch port.
     */
    public String getPortId() {
        return portId;
    }

    /**
     * Return the name of the physical switch port.
     *
     * @return  The name of the physical switch port.
     */
    public String getPortName() {
        return portName;
    }

    /**
     * Return the cost of using physical switch port.
     *
     * @return  The cost of using physical switch port.
     */
    public Long getCost() {
        return pathCost;
    }

    /**
     * Set the cost of using physical switch port.
     *
     * @param cost  The cost of using physical switch port.
     * @return  This instance.
     */
    public PathCost setCost(Long cost) {
        pathCost = cost;
        return this;
    }

    /**
     * Return the switch port descriptor.
     *
     * @return  The switch port descriptor.
     */
    public VtnPortDesc getPortDesc() {
        VtnPortDesc pdesc = portDesc;
        if (pdesc == null && nodeId != null) {
            StringBuilder builder = new StringBuilder(nodeId).
                append(PORT_DESC_SEPARATOR);
            if (portId != null) {
                builder.append(portId);
            }
            builder.append(PORT_DESC_SEPARATOR);
            if (portName != null) {
                builder.append(portName);
            }

            pdesc = new VtnPortDesc(builder.toString());
            portDesc = pdesc;
        }

        return pdesc;
    }

    /**
     * Verify the given path cost.
     *
     * @param vpc  A {@link VtnPathCost} instance.
     */
    public void verify(VtnPathCost vpc) {
        assertEquals(getPortDesc(), vpc.getPortDesc());

        Long cost = pathCost;
        if (cost == null) {
            cost = DEFAULT_COST;
        }
        assertEquals(cost, vpc.getCost());
    }

    /**
     * Convert this instance into a {@link VtnPathCost} instance.
     *
     * @return  A {@link VtnPathCost} instance.
     */
    public VtnPathCost toVtnPathCost() {
        return new VtnPathCostBuilder().
            setPortDesc(getPortDesc()).
            setCost(pathCost).
            build();
    }

    /**
     * Convert this instance into a {@link PathCostList} instance.
     *
     * @return  A {@link PathCostList} instance.
     */
    public PathCostList toPathCostList() {
        return new PathCostListBuilder().
            setPortDesc(getPortDesc()).
            setCost(pathCost).
            build();
    }
}
