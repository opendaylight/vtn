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

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code MacMapIdentifier} describes an identifier for a MAC mapping
 * configured in a vBridge.
 */
public final class MacMapIdentifier extends VBridgeMapIdentifier<MacMap> {
    /**
     * A long value which indicates that no mapped host is specified.
     */
    private static final long  HOST_UNDEFINED = -1L;

    /**
     * Cache for instance identifier that specifies the MAC mapping status.
     */
    private InstanceIdentifier<MacMapStatus>  statusPath;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     */
    public MacMapIdentifier(VnodeName tname, VnodeName bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  A {@link BridgeIdentifier} instance that specifies the
     *               vBridge.
     */
    public MacMapIdentifier(BridgeIdentifier<Vbridge> vbrId) {
        super(vbrId);
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid MAC mapping path components.
     */
    MacMapIdentifier(String[] comps) {
        super(comps[0], comps[1]);
    }

    /**
     * Return the path to the MAC mapping status.
     *
     * @return  An {@link InstanceIdentifier} instance for the MAC mapping
     *          status.
     */
    public InstanceIdentifier<MacMapStatus> getStatusPath() {
        InstanceIdentifier<MacMapStatus> path = statusPath;
        if (path == null) {
            path = getIdentifierBuilder().child(MacMapStatus.class).build();
            statusPath = path;
        }

        return path;
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#MACMAP}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.MACMAP;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<MacMap> getIdentifierBuilder() {
        return getVBridgeIdentifierBuilder().child(MacMap.class);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> newComponents() {
        return Arrays.asList(getTenantNameString(), getBridgeNameString());
    }

    /**
     * Determine whether the MAC mapping specified by this instance contains
     * the virtual network mapping specified by {@link VirtualNodePath}
     * instance.
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node. {@code null} cannot be specified.
     * @return  {@code true} if the MAC mapping specified by this instance
     *          contains the virtual network mapping specified by
     *          {@code vpath}. {@code false} otherwise.
     */
    @Override
    public boolean contains(VirtualNodePath vpath) {
        BridgeMapInfo bmi = vpath.getAugmentation(BridgeMapInfo.class);
        boolean ret;
        if (bmi == null) {
            ret = false;
        } else {
            ret = (bmi.getMacMappedHost() == null)
                ? false : super.contains(vpath);
        }

        return ret;
    }

    // VBridgeMapIdentifier

    /**
     * Return a {@link BridgeMapInfo} instance that indicates the MAC mapping
     * specified by this instance.
     *
     * @return  A {@link BridgeMapInfo} instance.
     */
    @Override
    protected BridgeMapInfo getBridgeMapInfo() {
        return new BridgeMapInfoBuilder().
            setMacMappedHost(HOST_UNDEFINED).build();
    }
}
