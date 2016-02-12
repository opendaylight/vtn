/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.DEFAULT_VLAN_ID;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatusBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code XmlVlanMapConfig} describes configuration information about a
 * VLAN mapping configured in a vBridge.
 */
public final class XmlVlanMapConfig {
    /**
     * A pseudo node-id that indicates no physical switch is specified.
     */
    private static final String  NODE_ID_ANY = "ANY";

    /**
     * Identifier for a physical switch to be mapped.
     */
    private final SalNode  targetNode;

    /**
     * A VLAN ID to be mapped.
     */
    private final Integer  vlanId;

    /**
     * An identifier for this VLAN mapping.
     */
    private String  mapId;

    /**
     * Construct an empty instance.
     */
    public XmlVlanMapConfig() {
        this(null, null);
    }

    /**
     * Construct a new instance without specifying the target switch.
     *
     * @param vid  A VLAN ID to be mapped.
     */
    public XmlVlanMapConfig(Integer vid) {
        this(null, vid);
    }

    /**
     * Construct a new instance.
     *
     * @param snode  A {@link SalNode} instance that specifies the target node.
     * @param vid    A VLAN ID to be mapped.
     */
    public XmlVlanMapConfig(SalNode snode, Integer vid) {
        targetNode = snode;
        vlanId = vid;
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
    public Integer getVlanId() {
        return vlanId;
    }

    /**
     * Return an identifier for this VLAN mapping.
     *
     * @return  An identifier for this VLAN mapping.
     */
    public String getMapId() {
        String id = mapId;
        if (id == null) {
            String nid = (targetNode == null)
                ? NODE_ID_ANY
                : targetNode.toString();
            Integer vid = vlanId;
            if (vid == null) {
                vid = DEFAULT_VLAN_ID;
            }
            id = nid + "." + vid;
            mapId = id;
        }

        return id;
    }

    /**
     * Convert this instance into a vlan-map instance.
     *
     * @return  A {@link VlanMapConfig} instance.
     */
    public VlanMap toVlanMap() {
        // Create VLAN mapping status.
        // This should be always ignored.
        VlanMapStatus vmst = new VlanMapStatusBuilder().
            setActive(true).
            build();

        return new VlanMapBuilder().
            setMapId(getMapId()).
            setVlanMapConfig(toVlanMapConfig()).
            setVlanMapStatus(vmst).
            build();
    }

    /**
     * Convert this instance into a vlan-map-config instance.
     *
     * @return  A {@link VlanMapConfig} instance.
     */
    public VlanMapConfig toVlanMapConfig() {
        VlanMapConfigBuilder builder = new VlanMapConfigBuilder();
        if (targetNode != null) {
            builder.setNode(targetNode.getNodeId());
        }

        VlanId vid = (vlanId == null) ? null : new VlanId(vlanId);
        return builder.setVlanId(vid).build();
    }

    /**
     * Convert this instance into a XML node.
     *
     * @return  A {@link XmlNode} instance that indicates the VLAN mapping
     *          configuration in this instance.
     */
    public XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode("vlan-map");
        if (targetNode != null) {
            xnode.add(new XmlNode("node", targetNode));
        }

        Integer vid = vlanId;
        if (vid == null) {
            vid = DEFAULT_VLAN_ID;
        }
        xnode.add(new XmlNode("vlan-id", vid));
        return xnode;
    }

    /**
     * Ensure that the given {@link VTNVlanMapConfig} instance is identical
     * to this instance.
     *
     * @param vvmc  A {@link VTNVlanMapConfig} instance.
     */
    public void verify(VTNVlanMapConfig vvmc) {
        assertEquals(targetNode, vvmc.getTargetNode());

        int vid = (vlanId == null) ? DEFAULT_VLAN_ID : vlanId.intValue();
        assertEquals(vid, vvmc.getVlanId());
    }
}
