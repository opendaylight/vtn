/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.inventory.InventoryUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.vtn.port.mappable.PortMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VTNVlanMapConfig} describes the configuration of a port mapping.
 */
public final class VTNPortMapConfig {
    /**
     * The default value of the VLAN ID.
     */
    public static final Integer  DEFAULT_VLAN_ID = 0;

    /**
     * The value of node.
     */
    private String  nodeId;

    /**
     * The value of port-id.
     */
    private String  portId;

    /**
     * The value of port-name.
     */
    private String  portName;

    /**
     * The value of vlan-id.
     */
    private Integer  vlanId;

    /**
     * The identifier for the switch port expected to be mapped by the
     * port mapping.
     */
    private String  mappedPort;

    /**
     * Remove the port mapping in the specified virtual interface.
     *
     * @param service  The vtn-port-map RPC service.
     * @param ident    The identifier for the target virtual interface.
     * @return  A {@link VtnUpdateType} instance.
     */
    public static VtnUpdateType removePortMap(
        VtnPortMapService service, VInterfaceIdentifier<?> ident) {
        RemovePortMapInputBuilder builder = new RemovePortMapInputBuilder();
        builder.fieldsFrom(ident.getVirtualNodePath());

        RemovePortMapInput input = builder.build();
        return getRpcResult(service.removePortMap(input));
    }

    /**
     * Construct a new instance.
     *
     * @param node  The value of node.
     * @param id    The value of port-id.
     * @param name  The value of port-name.
     * @param vid   The value of vlan-id.
     */
    public VTNPortMapConfig(String node, String id, String name, Integer vid) {
        nodeId = node;
        portId = id;
        portName = name;
        vlanId = vid;
    }

    /**
     * Construct a new instance.
     *
     * @param pid  The identifier for the switch port to be mapped by the
     *             port mapping.
     * @param vid  The value of vlan-id.
     */
    public VTNPortMapConfig(String pid, Integer vid) {
        nodeId = OfMockUtils.getNodeIdentifier(pid);
        portId = OfMockUtils.getPortId(pid);
        vlanId = vid;
    }

    /**
     * Return the value of node.
     *
     * @return  The value of node.
     */
    public String getNode() {
        return nodeId;
    }

    /**
     * Set the value of node.
     *
     * @param node  The value of node.
     * @return  This instance.
     */
    public VTNPortMapConfig setNode(String node) {
        nodeId = node;
        return this;
    }

    /**
     * Return the value of port-id.
     *
     * @return  The value of port-id.
     */
    public String getPortId() {
        return portId;
    }

    /**
     * Set the value of port-id.
     *
     * @param id  The value of port-id.
     * @return  This instance.
     */
    public VTNPortMapConfig setPortId(String id) {
        portId = id;
        return this;
    }

    /**
     * Return the value of port-name.
     *
     * @return  The value of port-name.
     */
    public String getPortName() {
        return portName;
    }

    /**
     * Set the value of port-name.
     *
     * @param name  The value of port-name.
     * @return  This instance.
     */
    public VTNPortMapConfig setPortName(String name) {
        portName = name;
        return this;
    }

    /**
     * Return the value of vlan-id.
     *
     * @return  The value of vlan-id..
     */
    public Integer getVlanId() {
        return vlanId;
    }

    /**
     * Set the value of vlan-id.
     *
     * @param vid  The value of vlan-id.
     * @return  This instance.
     */
    public VTNPortMapConfig setVlanId(Integer vid) {
        vlanId = vid;
        return this;
    }

    /**
     * Return the identifier for the switch port expected to be mapped by the
     * port mapping.
     *
     * @return  The identifier of the switch port.
     *          {@code null} if no switch port should be mapped by the
     *          port mapping.
     */
    public String getMappedPort() {
        return mappedPort;
    }

    /**
     * Set the identifier for the switch port expected to be mapped by the
     * port mapping.
     *
     * @param mapped  The identifier for the switch port to be mapped.
     *                {@code null} indicates that no switch port should be
     *                mapped.
     * @return  This instance.
     */
    public VTNPortMapConfig setMappedPort(String mapped) {
        mappedPort = mapped;
        return this;
    }

    /**
     * Create a new input builder for set-port-map RPC.
     *
     * @return  An {@link SetPortMapInputBuilder} instance.
     */
    public SetPortMapInputBuilder newInputBuilder() {
        VlanId vid = (vlanId == null) ? null : new VlanId(vlanId);
        return new SetPortMapInputBuilder().
            setNode(new NodeId(nodeId)).
            setPortId(portId).
            setPortName(portName).
            setVlanId(vid);
    }

    /**
     * Create a new input builder for set-port-map RPC.
     *
     * @param ident  The identifier for the virtual interface.
     * @return  An {@link SetPortMapInputBuilder} instance.
     */
    public SetPortMapInputBuilder newInputBuilder(
        VInterfaceIdentifier<?> ident) {
        SetPortMapInputBuilder builder = newInputBuilder();
        builder.fieldsFrom(ident.getVirtualNodePath());
        return builder;
    }

    /**
     * Update the specified port mapping.
     *
     * @param service  The vtn-port-map service.
     * @param ident    The identifier for the virtual interface.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnPortMapService service,
                                VInterfaceIdentifier<?> ident) {
        SetPortMapInputBuilder builder = newInputBuilder(ident);
        return getRpcResult(service.setPortMap(builder.build()));
    }

    /**
     * Verify the given port mapping configuration.
     *
     * @param pmc     The port mapping configuration to be verified.
     * @param mapped  The identifier for the port mapped by the port mapping.
     */
    public void verify(PortMapConfig pmc, NodeConnectorId mapped) {
        NodeId node = new NodeId(nodeId);
        Integer v = (vlanId == null) ? DEFAULT_VLAN_ID : vlanId;
        VlanId vid = new VlanId(v);
        assertEquals(node, pmc.getNode());
        assertEquals(portId, pmc.getPortId());
        assertEquals(portName, pmc.getPortName());
        assertEquals(vid, pmc.getVlanId());

        String pid = (mapped == null) ? null : mapped.getValue();
        assertEquals(pid, mappedPort);
    }

    /**
     * Find a VTN port to be mapped by this port mapping.
     *
     * @param rtx   A read-only MD-SAL datastore transaction.
     * @return  A {@link VtnPort} instance if found.
     *          {@code null} otherwise.
     */
    public VtnPort findPort(ReadTransaction rtx) {
        return InventoryUtils.findPort(rtx, nodeId, portId, portName);
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the virtual interface.
     */
    public void apply(VTNServices service, VInterfaceIdentifier<?> ident) {
        SetPortMapInputBuilder builder = newInputBuilder();
        builder.fieldsFrom(ident.getVirtualNodePath());
        getRpcOutput(service.getPortMapService().setPortMap(builder.build()));
    }
}
