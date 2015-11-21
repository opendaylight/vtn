/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfigBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VTNVlanMapConfig} describes configuration information about a
 * VLAN mapping configured in a vBridge.
 */
@XmlRootElement(name = "vtn-vlan-map-config")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNVlanMapConfig {
    /**
     * A pseudo node-id that indicates no physical switch is specified.
     */
    public static final String  NODE_ID_ANY = "ANY";

    /**
     * Identifier for a physical switch to be mapped.
     */
    @XmlElement(name = "node")
    private SalNode  targetNode;

    /**
     * A VLAN ID to be mapped.
     */
    @XmlElement(name = "vlan-id", required = true)
    private VlanId  vlanId;

    /**
     * An identifier for this VLAN mapping.
     */
    private String  mapId;

    /**
     * Construct an identifier for a VLAN mapping.
     *
     * @param snode  A {@link SalNode} instance which specifies the physical
     *               switch. {@code null} indicates no switch is speciifed.
     * @param vid    A {@link VlanId} instance which indicates the VLAN ID.
     *               Specifying {@code null} results in undefined behavior.
     * @return  A VLAN mapping identifier.
     */
    public static String createMapId(SalNode snode, VlanId vid) {
        String nodeId = (snode == null) ? NODE_ID_ANY : snode.toString();
        return createMapId(nodeId, vid.getValue());
    }

    /**
     * Construct an identifier for a VLAN mapping.
     *
     * @param cfg  A {@link VtnVlanMapConfig} instance.
     * @return  A VLAN mapping identifier if the given configuration is valid.
     *          {@code null} otherwise.
     */
    public static String createMapId(VtnVlanMapConfig cfg) {
        // Determine node ID in the given configuration.
        NodeId nid = cfg.getNode();
        String nodeId = (nid == null) ? null : nid.getValue();
        SalNode snode;
        if (nodeId == null) {
            nodeId = NODE_ID_ANY;
        } else if (SalNode.create(nodeId) == null) {
            // Invalid node ID.
            return null;
        }

        VlanId vlanId = cfg.getVlanId();
        Integer vid = (vlanId == null) ? null : vlanId.getValue();
        return createMapId(nodeId, vid);
    }

    /**
     * Construct an identifier for a VLAN mapping.
     *
     * @param nodeId  A node identifier which specifies the physical switch.
     *                Specifying {@code null} results in undefined behavior.
     * @param vid     A VLAN ID.
     * @return  An identifier for a VLAN mapping.
     */
    private static String createMapId(String nodeId, Integer vid) {
        int v = (vid == null) ? MiscUtils.DEFAULT_VLAN_ID : vid.intValue();
        return nodeId + "." + v;
    }

    /**
     * Private constructor only for JAXB.
     */
    private VTNVlanMapConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param vvmc  A {@link VtnVlanMapConfig} instance.
     * @throws RpcException
     *    {@code vvmc} contains invalid configuration.
     */
    public VTNVlanMapConfig(VtnVlanMapConfig vvmc) throws RpcException {
        NodeId nid = vvmc.getNode();
        if (nid != null) {
            targetNode = SalNode.checkedCreate(nid);
        }
        vlanId = MiscUtils.getVlanId(vvmc);
    }

    /**
     * Return the target switch identifier.
     *
     * @return  A {@link SalNode} instance if the target switch is configured.
     *          {@code null} if not configured.
     */
    public SalNode getTargetNode() {
        return targetNode;
    }

    /**
     * Return the VLAN ID to be mapped.
     *
     * @return  A VLAN ID.
     */
    public int getVlanId() {
        return vlanId.getValue();
    }

    /**
     * Return a {@link NodeVlan} instance which specifies the target VLAN.
     *
     * @return  A {@link NodeVlan} instance.
     */
    public NodeVlan getNodeVlan() {
        return new NodeVlan(targetNode, vlanId.getValue());
    }

    /**
     * Return an identifier for this VLAN mapping.
     *
     * @return  An identifier for this VLAN mapping.
     */
    public String getMapId() {
        String id = mapId;
        if (id == null) {
            id = createMapId(targetNode, vlanId);
            mapId = id;
        }

        return id;
    }

    /**
     * Convert this instance into a {@link VlanMapConfig} instance.
     *
     * @return  A {@link VlanMapConfig} instance.
     */
    public VlanMapConfig toVlanMapConfig() {
        VlanMapConfigBuilder builder = new VlanMapConfigBuilder();
        if (targetNode != null) {
            builder.setNode(targetNode.getNodeId());
        }

        return builder.setVlanId(vlanId).build();
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    public void verify() throws RpcException {
        if (vlanId == null) {
            throw RpcException.getNullArgumentException("vlan-id");
        }
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            VTNVlanMapConfig vmc = (VTNVlanMapConfig)o;
            ret = (Objects.equals(targetNode, vmc.targetNode) &&
                   Objects.equals(vlanId, vmc.vlanId));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), targetNode, vlanId);
    }
}
