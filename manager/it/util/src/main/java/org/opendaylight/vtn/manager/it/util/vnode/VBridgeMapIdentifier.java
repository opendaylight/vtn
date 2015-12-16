/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import org.opendaylight.yangtools.yang.binding.DataObject;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code VBridgeMapIdentifier} describes an identifier for a virtual mapping
 * configured in a vBridge.
 *
 * @param <T>  The type of the target data model.
 */
public abstract class VBridgeMapIdentifier<T extends DataObject>
    extends TenantNodeIdentifier<T, Vbridge> {
    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     */
    VBridgeMapIdentifier(VnodeName tname, VnodeName bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     */
    VBridgeMapIdentifier(String tname, String bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  A {@link BridgeIdentifier} instance that specifies the
     *               vBridge.
     */
    VBridgeMapIdentifier(BridgeIdentifier<Vbridge> vbrId) {
        super(vbrId.getTenantName(), vbrId.getBridgeName());
    }

    /**
     * Return a {@link BridgeMapInfo} instance that indicates the virtual
     * network mapping.
     *
     * @return  A {@link BridgeMapInfo} instance.
     */
    protected abstract BridgeMapInfo getBridgeMapInfo();

    // TenantNodeIdentifier

    /**
     * Return an identifier for the vBridge that contains the data object
     * specified by this instance.
     *
     * @return  A {@link VBridgeIdentifier} instance.
     */
    @Override
    public final VBridgeIdentifier getBridgeIdentifier() {
        return new VBridgeIdentifier(getTenantName(), getBridgeName());
    }

    /**
     * Return the vBridge name configured in the given {@link VnodePathFields}
     * instance.
     *
     * @param vpath  A {@link VnodePathFields} instance.
     *               {@code null} cannot be specified.
     * @return  The name of the vBridge configured in {@code vpath}.
     *          {@code null} if the vBridge name is not configured in
     *          {@code vpath}.
     */
    @Override
    protected final String getBridgeName(VnodePathFields vpath) {
        return vpath.getBridgeName();
    }

    /**
     * Set the vBridge name into the specified {@link VirtualNodePathBuilder}
     * instance.
     *
     * @param builder  A {@link VirtualNodePathBuilder} instance.
     * @return  {@code builder}.
     */
    @Override
    protected final VirtualNodePathBuilder setBridgeName(
        VirtualNodePathBuilder builder) {
        return builder.setBridgeName(getBridgeNameString());
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeIdentifier} that specifies a virtual network
     * element.
     *
     * <p>
     *   This method returns a {@link VBridgeIdentifier} instance that
     *   specifies a vBridge in which contains this network mapping.
     * </p>
     *
     * @return  A {@link VBridgeIdentifier} instance.
     */
    @Override
    public final VBridgeIdentifier getVNodeIdentifier() {
        return getBridgeIdentifier();
    }

    /**
     * Create a new {@link VirtualNodePathBuilder} instance that contains the
     * path components configured in this instance.
     *
     * @return  A {@link VirtualNodePathBuilder} instance.
     */
    @Override
    protected final VirtualNodePathBuilder getVirtualNodePathBuilder() {
        return super.getVirtualNodePathBuilder().
            addAugmentation(BridgeMapInfo.class, getBridgeMapInfo());
    }
}
