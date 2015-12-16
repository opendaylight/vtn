/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.inventory;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockUtils.getNodeIdentifier;
import static org.opendaylight.vtn.manager.it.ofmock.OfMockUtils.getPortIdentifier;

import java.util.List;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * {@code InventoryUtils} class is a collection of utility class methods
 * for MD-SAL based inventory management.
 */
public final class InventoryUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private InventoryUtils() {}

    /**
     * Return the instance identifier for the VTN node.
     *
     * @param nid  The node identifier.
     * @return  The instance identifier for the specified VTN node.
     */
    public static InstanceIdentifier<VtnNode> getVtnNodeIdentifier(String nid) {
        VtnNodeKey nodeKey = new VtnNodeKey(new NodeId(nid));

        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class, nodeKey).
            build();
    }

    /**
     * Return the instance identifier for the VTN port.
     *
     * @param pid  The port identifier.
     * @return  The instance identifier for the specified VTN port.
     */
    public static InstanceIdentifier<VtnPort> getVtnPortIdentifier(String pid) {
        VtnNodeKey nodeKey =
            new VtnNodeKey(new NodeId(getNodeIdentifier(pid)));
        VtnPortKey portKey = new VtnPortKey(new NodeConnectorId(pid));

        return InstanceIdentifier.builder(VtnNodes.class).
            child(VtnNode.class, nodeKey).
            child(VtnPort.class, portKey).
            build();
    }

    /**
     * Determine whether the given VTN port has at least one inter-switch link
     * or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port has at least one
     *          inter-switch link.
     */
    public static boolean hasPortLink(VtnPort vport) {
        List<PortLink> plinks = vport.getPortLink();
        return (plinks != null && !plinks.isEmpty());
    }

    /**
     * Determine whether the given VTN port is an edge port or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port is an edge port.
     */
    public static boolean isEdge(VtnPort vport) {
        return !hasPortLink(vport);
    }

    /**
     * Determine whether the given VTN port is enabled or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port is enabled.
     */
    public static boolean isEnabled(VtnPort vport) {
        return Boolean.TRUE.equals(vport.isEnabled());
    }

    /**
     * Determine whether the given VTN port is an enabled edge port or not.
     *
     * @param vport  A {@link VtnPort} instance.
     * @return  {@code true} only if the given port is an enabled edge port.
     */
    public static boolean isEnabledEdge(VtnPort vport) {
        return isEnabled(vport) && isEdge(vport);
    }

    /**
     * Determine whether the given node has at least one edge port in up state
     * or not.
     *
     * @param vnode  A {@link VtnNode} instance.
     * @return  {@code true} is returned if the given node has at least one
     *          edge port in up state. Otherwise {@code false} is returned.
     */
    public static boolean hasEdgePort(VtnNode vnode) {
        if (vnode != null) {
            List<VtnPort> ports = vnode.getVtnPort();
            if (ports != null) {
                for (VtnPort vport: ports) {
                    if (isEnabledEdge(vport)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    /**
     * Determine whether the given node has at least one edge port in up state
     * or not.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @param nid  The node identifier string.
     * @return  {@code true} is returned if the given node has at least one
     *          edge port in up state. Otherwise {@code false} is returned.
     */
    public static boolean hasEdgePort(ReadTransaction rtx, String nid) {
        Optional<VtnNode> opt = DataStoreUtils.read(
            rtx, getVtnNodeIdentifier(nid));
        return (opt.isPresent()) ? hasEdgePort(opt.get()) : false;
    }

    /**
     * Find a VTN port that meets the specified condition.
     *
     * @param rtx   A read-only MD-SAL datastore transaction.
     * @param nid   The node identifier string.
     * @param pid   A string representation of the port ID.
     * @param name  The name of the port.
     * @return  A {@link VtnPort} instance if found.
     *          {@code null} otherwise.
     */
    public static VtnPort findPort(ReadTransaction rtx, String nid, String pid,
                                   String name) {
        Optional<VtnNode> opt = DataStoreUtils.read(
            rtx, getVtnNodeIdentifier(nid));
        VtnPort found = null;
        if (opt.isPresent()) {
            List<VtnPort> ports = opt.get().getVtnPort();
            if (ports != null) {
                for (VtnPort vport: ports) {
                    if (match(vport, nid, pid, name)) {
                        found = vport;
                        break;
                    }
                }
            }
        }

        return found;
    }

    /**
     * Determine whether the given VTN port matches the given condition.
     *
     * @param vport  A VTN port to be tested.
     * @param nid    The node identifier string.
     * @param pid    A string representation of the port ID.
     * @param name   The name of the port.
     * @return  {@code true} if the given VTN port matches the condition.
     *          {@code false} otherwise.
     */
    private static boolean match(VtnPort vport, String nid, String pid,
                                 String name) {
        boolean ret = (name == null || name.equals(vport.getName()));
        if (ret && pid != null) {
            String portId = getPortIdentifier(nid, pid);
            ret = portId.equals(vport.getId().getValue());
        }

        return ret;
    }
}
