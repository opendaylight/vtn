/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;

/**
 * {@code VBridgeIfIdentifier} describes an identifier for a virtual interface
 * attached to a vBridge.
 */
public final class VBridgeIfIdentifier extends VInterfaceIdentifier<Vbridge> {
    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param iname  The name of the virtual interface.
     */
    public VBridgeIfIdentifier(VnodeName tname, VnodeName bname,
                               VnodeName iname) {
        super(tname, bname, iname);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param iname  The name of the virtual interface.
     */
    public VBridgeIfIdentifier(String tname, String bname, String iname) {
        super(tname, bname, iname);
    }

    /**
     * Construct a new instance.
     *
     * @param vbrId  A {@link VBridgeIdentifier} instance that specifies the
     *               vBridge.
     * @param iname  The name of the virtual interface.
     */
    public VBridgeIfIdentifier(VBridgeIdentifier vbrId, VnodeName iname) {
        super(vbrId.getTenantName(), vbrId.getBridgeName(), iname);
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid vBridge interface path
     *               components.
     */
    VBridgeIfIdentifier(String[] comps) {
        super(comps[0], comps[1], comps[2]);
    }

    // VInterfaceIdentifier

    /**
     * {@inheritDoc}
     */
    @Override
    public VBridgeIfIdentifier replaceTenantName(VnodeName tname) {
        return new VBridgeIfIdentifier(tname, getBridgeName(),
                                       getInterfaceName());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RedirectDestination toRedirectDestination() {
        return new RedirectDestinationBuilder().
            setTenantName(getTenantNameString()).
            setBridgeName(getBridgeNameString()).
            setInterfaceName(getInterfaceNameString()).
            build();
    }

    // TenantNodeIdentifier

    /**
     * Return an identifier for the vBridge that contains the virtual interface
     * specified by this instance.
     *
     * @return  A {@link VBridgeIdentifier} instance.
     */
    @Override
    public VBridgeIdentifier getBridgeIdentifier() {
        return new VBridgeIdentifier(getTenantName(), getBridgeName());
    }

    /**
     * Set the vBridge name into the specified {@link VirtualNodePathBuilder}
     * instance.
     *
     * @param builder  A {@link VirtualNodePathBuilder} instance.
     * @return  {@code builder}.
     */
    @Override
    protected VirtualNodePathBuilder setBridgeName(
        VirtualNodePathBuilder builder) {
        return builder.setBridgeName(getBridgeNameString());
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
    protected String getBridgeName(VnodePathFields vpath) {
        return vpath.getBridgeName();
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#VBRIDGE_IF}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.VBRIDGE_IF;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<Vinterface> getIdentifierBuilder() {
        return getVBridgeIdentifierBuilder().
            child(Vinterface.class, new VinterfaceKey(getInterfaceName()));
    }

    /**
     * Return a {@link VNodeIdentifier} that specifies a virtual network
     * element.
     *
     * <p>
     *   This method always returns this instance because virtual interface is
     *   virtual network element.
     * </p>
     *
     * @return  This instance.
     */
    @Override
    public VBridgeIfIdentifier getVNodeIdentifier() {
        return this;
    }
}
