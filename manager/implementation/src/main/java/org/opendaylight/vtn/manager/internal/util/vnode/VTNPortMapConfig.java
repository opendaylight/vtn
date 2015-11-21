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
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VTNPortMapConfig} describes configuration information about a
 * port mapping configured in a virtual interface.
 */
@XmlRootElement(name = "vtn-port-map-config")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNPortMapConfig implements VtnSwitchPort {
    /**
     * A pseudo port number that indicates an undefined value.
     */
    private static final long  PORT_UNDEF = -1L;

    /**
     * Identifier for a physical switch that contains the target switch port.
     */
    @XmlElement(name = "node", required = true)
    private SalNode  targetNode;

    /**
     * The name of the target switch port.
     */
    @XmlElement(name = "port-name")
    private String  portName;

    /**
     * Identifier for a physical switch port.
     */
    @XmlElement(name = "port-id")
    private String  portId;

    /**
     * A VLAN ID to be mapped.
     */
    @XmlElement(name = "vlan-id", required = true)
    private VlanId  vlanId;

    /**
     * A long value that indicates the switch port identifier.
     */
    private long  portNumber = PORT_UNDEF;

    /**
     * Private constructor only for JAXB.
     */
    private VTNPortMapConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param vpmc  A {@link VtnPortMapConfig} instance.
     * @throws RpcException
     *    {@code vpmc} contains invalid configuration.
     */
    public VTNPortMapConfig(VtnPortMapConfig vpmc) throws RpcException {
        targetNode = SalNode.checkedCreate(vpmc.getNode());
        portName = vpmc.getPortName();
        portId = vpmc.getPortId();
        vlanId = MiscUtils.getVlanId(vpmc);
        verify();
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
     * Return the VLAN ID to be mapped.
     *
     * @return  A VLAN ID.
     */
    public int getVlanId() {
        return vlanId.getValue();
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verification failed.
     */
    public void verify() throws RpcException {
        if (targetNode == null) {
            throw RpcException.getNullArgumentException("Node");
        }

        SalPort sport = NodeUtils.checkPortLocation(
            targetNode, portId, portName);
        if (sport != null) {
            // Preserve the port number as a long value.
            portNumber = sport.getPortNumber();
        }

        if (vlanId == null) {
            throw RpcException.getNullArgumentException("vlan-id");
        }
    }

    /**
     * Convert this instance into a {@link PortMapConfig} instance.
     *
     * @return  A {@link PortMapConfig} instance.
     */
    public PortMapConfig toPortMapConfig() {
        return new PortMapConfigBuilder().
            setNode(targetNode.getNodeId()).
            setPortName(portName).
            setPortId(portId).
            setVlanId(vlanId).
            build();
    }

    /**
     * Determine whether the given switch port satisfies the condition
     * specified by this instance.
     *
     * @param sport  A {@link SalPort} instance that specifies the physical
     *               switch port.
     * @param vport  A {@link VtnPort} instance that represents information
     *               about the switch port specified by {@code sport}.
     * @return  {@code true} only if the given switch port satisfies the
     *          condition.
     */
    public boolean match(SalPort sport, VtnPort vport) {
        boolean ret = (targetNode.getNodeNumber() == sport.getNodeNumber());
        if (ret) {
            ret = (portId == null || portNumber == sport.getPortNumber());
            if (ret) {
                ret = (portName == null || portName.equals(vport.getName()));
            }
        }

        return ret;
    }

    /**
     * Return a {@link PortVlan} instance that indicates the VLAN mapped by
     * the port mapping.
     *
     * @param mapped  A {@link SalPort} instance that specifies the switch port
     *                mapped by the port mapping.
     * @return  A {@link PortVlan} instance if {@code mapped} is not
     *          {@code null}. {@code null} otherwise.
     */
    public PortVlan getMappedVlan(SalPort mapped) {
        return (mapped == null)
            ? null : new PortVlan(mapped, vlanId.getValue());
    }

    // DataContainer

    /**
     * Return a class which indicates the data model type implemented by this
     * instance.
     *
     * @return  A class instance of {@link VtnSwitchPort}.
     */
    @Override
    public Class<VtnSwitchPort> getImplementedInterface() {
        return VtnSwitchPort.class;
    }

    // VtnSwitchPort

    /**
     * Return the name of the target switch port.
     *
     * @return  A port name if configured.
     *          {@code null} if not configured.
     */
    @Override
    public String getPortName() {
        return portName;
    }

    /**
     * Return the target switch port identifier.
     *
     * @return  A port identifier if configured.
     *          {@code null} if not configured.
     */
    @Override
    public String getPortId() {
        return portId;
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
            VTNPortMapConfig pmc = (VTNPortMapConfig)o;
            ret = (Objects.equals(targetNode, pmc.targetNode) &&
                   Objects.equals(portName, pmc.portName) &&
                   Objects.equals(portId, pmc.portId) &&
                   Objects.equals(vlanId, pmc.vlanId));
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
        return Objects.hash(getClass(), targetNode, portName, portId, vlanId);
    }
}
