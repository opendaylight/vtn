/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.internal.util.MiscUtils.DEFAULT_VLAN_ID;

import static org.opendaylight.vtn.manager.internal.TestBase.createVlanId;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfigBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code XmlPortMapConfig} describes configuration information about a
 * port mapping configured in a virtual interface.
 */
public final class XmlPortMapConfig {
    /**
     * Identifier for a physical switch that contains the target switch port.
     */
    private final SalNode  targetNode;

    /**
     * The name of the target switch port.
     */
    private String  portName;

    /**
     * Identifier for a physical switch port.
     */
    private String  portId;

    /**
     * A VLAN ID to be mapped.
     */
    private Integer  vlanId;

    /**
     * Construct a new instance.
     *
     * @param snode  A {@link SalNode} that specifies the target switch.
     */
    public XmlPortMapConfig(SalNode snode) {
        targetNode = snode;
    }

    /**
     * Construct a new instance using the given pseudo random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public XmlPortMapConfig(Random rand) {
        targetNode = new SalNode(rand.nextLong());

        // Determine the port ID.
        int pid = rand.nextInt(1000) + 1;
        int n = rand.nextInt(3);
        if (n <= 1) {
            // Specify the port identifier as string.
            portId = Integer.toString(pid);
        }
        if (n >= 1) {
            // Specify the port name.
            portName = "port-" + pid;
        }

        if (rand.nextInt(4) > 0) {
            // Specify the VLAN ID.
            vlanId = createVlanId(rand);
        }
    }

    /**
     * Return the target switch identifier.
     *
     * @return  A {@link SalNode} instance.
     */
    public SalNode getTargetNode() {
        return targetNode;
    }

    /**
     * Return the name of the target switch port.
     *
     * @return  A port name if configured.
     *          {@code null} if not configured.
     */
    public String getPortName() {
        return portName;
    }

    /**
     * Set the name of the target switch port.
     *
     * @param pname  The name of the target switch port.
     * @return  This instance.
     */
    public XmlPortMapConfig setPortName(String pname) {
        portName = pname;
        return this;
    }

    /**
     * Return the target switch port identifier.
     *
     * @return  A port identifier if configured.
     *          {@code null} if not configured.
     */
    public String getPortId() {
        return portId;
    }

    /**
     * Set the target switch identifier.
     *
     * @param pid  The target switch identifier.
     * @return  This instance.
     */
    public XmlPortMapConfig setPortId(String pid) {
        portId = pid;
        return this;
    }

    /**
     * Return the VLAN ID to be mapped.
     *
     * @return  A VLAN ID if configured.
     *          {@code null} if not configured.
     */
    public Integer getVlanId() {
        return vlanId;
    }

    /**
     * Set the VLAN ID to be mapped.
     *
     * @param vid  The VLAN ID to be mapped.
     * @return  This instance.
     */
    public XmlPortMapConfig setVlanId(Integer vid) {
        vlanId = vid;
        return this;
    }

    /**
     * Convert this instance into a port-map-config instance.
     *
     * @return  A {@link PortMapConfig} instance.
     */
    public PortMapConfig toPortMapConfig() {
        VlanId vid = (vlanId == null) ? null : new VlanId(vlanId);
        return new PortMapConfigBuilder().
            setNode(targetNode.getNodeId()).
            setPortName(portName).
            setPortId(portId).
            setVlanId(vid).
            build();
    }

    /**
     * Set port mapping configuration in this instance into the specified
     * XML node.
     *
     * @param xnode   A {@link XmlNode} instance.
     */
    public void setXml(XmlNode xnode) {
        XmlNode xpmc = new XmlNode("port-map").
            add(new XmlNode("node", targetNode));
        if (portId != null) {
            xpmc.add(new XmlNode("port-id", portId));
        }
        if (portName != null) {
            xpmc.add(new XmlNode("port-name", portName));
        }

        Integer vid = vlanId;
        if (vid == null) {
            vid = DEFAULT_VLAN_ID;
        }
        xpmc.add(new XmlNode("vlan-id", vid));

        xnode.add(xpmc);
    }

    /**
     * Ensure that the given {@link VTNPortMapConfig} instance is identical
     * to this instance.
     *
     * @param vpmc  A {@link VTNPortMapConfig} instance.
     */
    public void verify(VTNPortMapConfig vpmc) {
        assertNotNull(vpmc);
        assertEquals(targetNode, vpmc.getTargetNode());
        assertEquals(portId, vpmc.getPortId());
        assertEquals(portName, vpmc.getPortName());

        int vid = (vlanId == null) ? DEFAULT_VLAN_ID : vlanId.intValue();
        assertEquals(vid, vpmc.getVlanId());
    }

    /**
     * Ensure that the given port-map-config instance is identical
     * to this instance.
     *
     * @param pmc  A {@link PortMapConfig} instance.
     */
    public void verify(PortMapConfig pmc) {
        assertNotNull(pmc);
        assertEquals(targetNode.getNodeId(), pmc.getNode());
        assertEquals(portId, pmc.getPortId());
        assertEquals(portName, pmc.getPortName());

        Integer vid = vlanId;
        if (vid == null) {
            vid = DEFAULT_VLAN_ID;
        }
        assertEquals(vid, pmc.getVlanId().getValue());
    }
}
