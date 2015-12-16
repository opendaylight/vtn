/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import java.util.Arrays;
import java.util.List;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

/**
 * {@code VTerminalIdentifier} describes an identifier for a vTerminal.
 */
public final class VTerminalIdentifier extends BridgeIdentifier<Vterminal> {
    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vTerminal.
     */
    public VTerminalIdentifier(VnodeName tname, VnodeName bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vTerminal.
     */
    public VTerminalIdentifier(String tname, String bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               VTN.
     * @param bname  The name of the vTerminal.
     */
    public VTerminalIdentifier(VNodeIdentifier<?> ident, VnodeName bname) {
        super(ident.getTenantName(), bname);
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid vTerminal path components.
     */
    VTerminalIdentifier(String[] comps) {
        super(comps[0], comps[1]);
    }

    // BridgeIdentifier

    /**
     * Construct a new identifier for a virtual interface inside the vBridge
     * specified by this instance.
     *
     * @param iname  The name of the virtual interface.
     * @return  A {@link VTerminalIfIdentifier} instance.
     */
    @Override
    public VTerminalIfIdentifier childInterface(String iname) {
        return childInterface(new VnodeName(iname));
    }

    /**
     * Construct a new identifier for a virtual interface inside the vTerminal
     * specified by this instance.
     *
     * @param iname  The name of the virtual interface.
     * @return  A {@link VTerminalIfIdentifier} instance.
     */
    @Override
    public VTerminalIfIdentifier childInterface(VnodeName iname) {
        return new VTerminalIfIdentifier(this, iname);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<BridgeStatus> newStatusPath() {
        return getVTerminalIdentifierBuilder().
            child(BridgeStatus.class).build();
    }

    // TenantNodeIdentifier

    /**
     * Return an identifier for the vTerminal.
     *
     * @return  This instance.
     */
    @Override
    public VTerminalIdentifier getBridgeIdentifier() {
        return this;
    }

    /**
     * Return the vTerminal name configured in the given
     * {@link VnodePathFields} instance.
     *
     * @param vpath  A {@link VnodePathFields} instance.
     *               {@code null} cannot be specified.
     * @return  The name of the vTerminal configured in {@code vpath}.
     *          {@code null} if the vTerminal name is not configured in
     *          {@code vpath}.
     */
    @Override
    protected String getBridgeName(VnodePathFields vpath) {
        return vpath.getTerminalName();
    }

    /**
     * Set the vTerminal name into the specified {@link VirtualNodePathBuilder}
     * instance.
     *
     * @param builder  A {@link VirtualNodePathBuilder} instance.
     * @return  {@code builder}.
     */
    @Override
    protected VirtualNodePathBuilder setBridgeName(
        VirtualNodePathBuilder builder) {
        return builder.setTerminalName(getBridgeNameString());
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#VTERMINAL}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.VTERMINAL;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<Vterminal> getIdentifierBuilder() {
        return getVTerminalIdentifierBuilder();
    }

    /**
     * Return a {@link VNodeIdentifier} that specifies a virtual network
     * element.
     *
     * <p>
     *   This method always returns this instance because vTerminal is virtual
     *   network element.
     * </p>
     *
     * @return  This instance.
     */
    @Override
    public VTerminalIdentifier getVNodeIdentifier() {
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> newComponents() {
        return Arrays.asList(getTenantNameString(), getBridgeNameString());
    }
}
