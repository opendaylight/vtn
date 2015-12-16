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

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.inventory.InventoryUtils;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.remove.vlan.map.output.RemoveVlanMapResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * {@code VTNVlanMapConfig} describes the configuration of a VLAN mapping.
 */
public final class VTNVlanMapConfig {
    /**
     * A pseudo node-id that indicates no physical switch is specified.
     */
    public static final String  NODE_ID_ANY = "ANY";

    /**
     * The default value of the VLAN ID.
     */
    public static final Integer  DEFAULT_VLAN_ID = 0;

    /**
     * The identifier for the VLAN mapping.
     */
    private final String  mapId;

    /**
     * The value of node.
     */
    private final String  nodeId;

    /**
     * The value of vlan-id.
     */
    private final Integer  vlanId;

    /**
     * A boolean value that specifies the expected state of the VLAN mapping.
     */
    private Boolean  active;

    /**
     * Remove all the VLAN mappings configured in the specified vBridge.
     *
     * @param service  The vtn-vlan-map RPC service.
     * @param ident    The identifier for the target vBridge.
     * @return  A map that specifies the removed VLAN mappings.
     */
    public static Map<String, VtnUpdateType> removeVlanMap(
        VtnVlanMapService service, VBridgeIdentifier ident) {
        return removeVlanMap(service, ident, null);
    }

    /**
     * Remove the specified VLAN mapping.
     *
     * @param service  The vtn-vlan-map RPC service.
     * @param ident    The identifier for the target vBridge.
     * @param mapIds   A list of VLAN mappings IDs to be removed.
     * @return  A map that specifies the removed VLAN mappings.
     */
    public static Map<String, VtnUpdateType> removeVlanMap(
        VtnVlanMapService service, VBridgeIdentifier ident,
        List<String> mapIds) {
        RemoveVlanMapInput input = new RemoveVlanMapInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            setMapIds(mapIds).
            build();

        RemoveVlanMapOutput output =
            getRpcOutput(service.removeVlanMap(input));
        List<RemoveVlanMapResult> removed = output.getRemoveVlanMapResult();
        Map<String, VtnUpdateType> result = new HashMap<>();
        if (removed != null) {
            for (RemoveVlanMapResult rvres: removed) {
                String mapId = rvres.getMapId();
                VtnUpdateType status = rvres.getStatus();
                assertEquals(null, result.put(mapId, status));
            }
        }

        return result;
    }

    /**
     * Construct an identifier for a VLAN mapping.
     *
     * @param nid  A node identifier.
     * @param vid  A VLAN ID.
     * @return  A VLAN mapping identifier.
     */
    public static String createMapId(String nid, Integer vid) {
        String nodeId = (nid == null) ? NODE_ID_ANY : nid;
        Integer vlanId = (vid == null) ? DEFAULT_VLAN_ID : vid;
        return nodeId + "." + vlanId;
    }

    /**
     * Construct a new instance with default values.
     */
    public VTNVlanMapConfig() {
        this(null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param vid   The value of vlan-id.
     */
    public VTNVlanMapConfig(Integer vid) {
        this(null, vid);
    }

    /**
     * Construct a new instance.
     *
     * @param node  The value of node.
     * @param vid   The value of vlan-id.
     */
    public VTNVlanMapConfig(String node, Integer vid) {
        nodeId = node;
        vlanId = vid;
        mapId = createMapId(node, vid);
        if (node == null) {
            active = Boolean.TRUE;
        }
    }

    /**
     * Create a copy of this instance, and complete missing parameters.
     *
     * @return  A copied {@link VTNVlanMapConfig} instance.
     */
    public VTNVlanMapConfig complete() {
        Integer vid = vlanId;
        if (vid == null) {
            vid = DEFAULT_VLAN_ID;
        }

        return new VTNVlanMapConfig(nodeId, vid);
    }

    /**
     * Return the VLAN mapping identifier.
     *
     * @return  The identifier for the VLAN mapping.
     */
    public String getMapId() {
        return mapId;
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
     * Return the value of vlan-id.
     *
     * @return  The value of vlan-id..
     */
    public Integer getVlanId() {
        return vlanId;
    }

    /**
     * Return the expected state of the VLAN mapping.
     *
     * @return  The expected state of the VLAN mapping.
     */
    public Boolean isActive() {
        return active;
    }

    /**
     * Set the expected state of the VLAN mapping.
     *
     * @param act  A boolean value which specifies the expected state of the
     *             VLAN mapping. If {@code null} is specified, the expected
     *             state of the VLAN mapping is determined automatically.
     * @return  This instance.
     */
    public VTNVlanMapConfig setActive(Boolean act) {
        active = act;
        return this;
    }

    /**
     * Create a new input builder for add-vlan-map RPC.
     *
     * @return  An {@link AddVlanMapInputBuilder} instance.
     */
    public AddVlanMapInputBuilder newInputBuilder() {
        NodeId nid = (nodeId == null) ? null : new NodeId(nodeId);
        VlanId vid = (vlanId == null) ? null : new VlanId(vlanId);
        return new AddVlanMapInputBuilder().
            setNode(nid).
            setVlanId(vid);
    }

    /**
     * Create a new input builder for add-vlan-map RPC.
     *
     * @param ident  The identifier for the target vBridge.
     * @return  An {@link AddVlanMapInputBuilder} instance.
     */
    public AddVlanMapInputBuilder newInputBuilder(VBridgeIdentifier ident) {
        return newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString());
    }

    /**
     * Create a new input builder for get-vlan-map RPC.
     *
     * @return  A {@link GetVlanMapInputBuilder} instance.
     */
    public GetVlanMapInputBuilder newGetInputBuilder() {
        NodeId nid = (nodeId == null) ? null : new NodeId(nodeId);
        VlanId vid = (vlanId == null) ? null : new VlanId(vlanId);
        return new GetVlanMapInputBuilder().
            setNode(nid).
            setVlanId(vid);
    }

    /**
     * Create a new input builder for get-vlan-map RPC.
     *
     * @param ident  The identifier for the target vBridge.
     * @return  A {@link GetVlanMapInputBuilder} instance.
     */
    public GetVlanMapInputBuilder newGetInputBuilder(VBridgeIdentifier ident) {
        return newGetInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString());
    }

    /**
     * Verify the given VLAN mapping.
     *
     * @param rtx    A read-only MD-SAL datastore transaction.
     * @param vlmap  The VLAN mapping to be verified.
     * @return  A {@link VnodeState} instance that indicates the stauts of the
     *          VLAN mapping.
     */
    public VnodeState verify(ReadTransaction rtx, VlanMap vlmap) {
        assertEquals(mapId, vlmap.getMapId());

        NodeId node = (nodeId == null)
            ? null
            : new NodeId(nodeId);
        Boolean act = active;

        if (act == null) {
            // Determine the expected state of the VLAN mapping.
            act = (nodeId == null)
                ? Boolean.TRUE
                : InventoryUtils.hasEdgePort(rtx, nodeId);
        }

        Integer v = (vlanId == null) ? DEFAULT_VLAN_ID : vlanId;
        VlanId vid = new VlanId(v);

        // Verify the configuration.
        VlanMapConfig vmc = vlmap.getVlanMapConfig();
        assertEquals(node, vmc.getNode());
        assertEquals(vid, vmc.getVlanId());

        // Verify the status.
        VlanMapStatus vst = vlmap.getVlanMapStatus();
        assertEquals(act, vst.isActive());

        return (act.booleanValue()) ? VnodeState.UP : VnodeState.DOWN;
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the vBridge.
     */
    public void apply(VTNServices service, VBridgeIdentifier ident) {
        AddVlanMapInput input = newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            build();
        getRpcOutput(service.getVlanMapService().addVlanMap(input));
    }
}
