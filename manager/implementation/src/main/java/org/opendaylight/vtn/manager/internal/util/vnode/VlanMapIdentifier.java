/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.Arrays;
import java.util.List;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code VlanMapIdentifier} describes an identifier for a VLAN mapping
 * configured in a vBridge.
 */
public final class VlanMapIdentifier extends VBridgeMapIdentifier<VlanMap> {
    /**
     * A VLAN mapping ID assigned to the VLAN mapping.
     */
    private final String  mapId;

    /**
     * Cache for instance identifier that specifies the VLAN mapping status.
     */
    private InstanceIdentifier<VlanMapStatus>  statusPath;

    /**
     * Create a new identifier.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param id     The identifier assigned to the VLAN mapping.
     * @return  A {@link VBridgeIdentifier} instance.
     * @throws RpcException  Invalid name is specified.
     */
    public static VlanMapIdentifier create(
        String tname, String bname, String id) throws RpcException {
        VnodeName vtnName = VNodeType.VTN.checkName(tname, true);
        VnodeName vbrName = VNodeType.VBRIDGE.checkName(bname, true);
        if (id == null) {
            throw RpcException.getNullArgumentException("VLAN mapping ID");
        }
        return new VlanMapIdentifier(vtnName, vbrName, id);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param id     The identifier assigned to the VLAN mapping.
     */
    public VlanMapIdentifier(VnodeName tname, VnodeName bname, String id) {
        super(tname, bname);
        mapId = id;
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  A {@link BridgeIdentifier} instance that specifies the
     *               vBridge.
     * @param id     The identifier assigned to the VLAN mapping.
     */
    public VlanMapIdentifier(BridgeIdentifier<Vbridge> vbrId, String id) {
        super(vbrId);
        mapId = id;
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid VLAN mapping path components.
     */
    VlanMapIdentifier(String[] comps) {
        super(comps[0], comps[1]);
        mapId = comps[2];
        if (COMP_NULL.equals(mapId)) {
            throw new IllegalArgumentException(
                "Invalid VLAN mapping ID: " + mapId);
        }
    }

    /**
     * Return an identifier assigned to the VLAN mapping.
     *
     * @return  A VLAN mapping identifier.
     */
    public String getMapId() {
        return mapId;
    }

    /**
     * Return the path to the VLAN mapping status.
     *
     * @return  An {@link InstanceIdentifier} instance for the VLAN mapping
     *          status.
     */
    public InstanceIdentifier<VlanMapStatus> getStatusPath() {
        InstanceIdentifier<VlanMapStatus> path = statusPath;
        if (path == null) {
            path = getVBridgeIdentifierBuilder().
                child(VlanMap.class, new VlanMapKey(mapId)).
                child(VlanMapStatus.class).
                build();
            statusPath = path;
        }

        return path;
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#VLANMAP}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.VLANMAP;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<VlanMap> getIdentifierBuilder() {
        return getVBridgeIdentifierBuilder().
            child(VlanMap.class, new VlanMapKey(mapId));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> newComponents() {
        return Arrays.asList(getTenantNameString(), getBridgeNameString(),
                             mapId);
    }

    /**
     * Determine whether the VLAN mapping specified by this instance is equal
     * to the VLAN mapping specified by {@link VirtualNodePath} instance.
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node. {@code null} cannot be specified.
     * @return  {@code true} if the VLAN mapping specified by this instance is
     *          equal to the VLAN mapping specified by {@code vpath}.
     *          {@code false} otherwise.
     */
    @Override
    public boolean contains(VirtualNodePath vpath) {
        BridgeMapInfo bmi = vpath.getAugmentation(BridgeMapInfo.class);
        boolean ret;
        if (bmi == null) {
            ret = false;
        } else {
            String mid = bmi.getVlanMapId();
            ret = (mid == null)
                ? false
                : (super.contains(vpath) && mid.equals(mapId));
        }

        return ret;
    }

    // VBridgeMapIdentifier

    /**
     * Return a {@link BridgeMapInfo} instance that indicates the VLAN mapping
     * specified by this instance.
     *
     * @return  A {@link BridgeMapInfo} instance.
     */
    @Override
    protected BridgeMapInfo getBridgeMapInfo() {
        return new BridgeMapInfoBuilder().setVlanMapId(mapId).build();
    }
}
