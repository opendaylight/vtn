/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.redirect.filter.config.RedirectDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VTerminalIfIdentifier} describes an identifier for a virtual
 * interface attached to a vTerminal.
 */
@XmlRootElement(name = "vterminal-if-path")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTerminalIfIdentifier
    extends VInterfaceIdentifier<Vterminal> {
    /**
     * Create a new identifier.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vTerminal.
     * @param iname  The name of the virtual interface.
     * @param find   {@code true} indicates the given name is used for
     *               finding existing virtual node.
     * @return  A {@link VTerminalIfIdentifier} instance.
     * @throws RpcException  Invalid name is specified.
     */
    public static VTerminalIfIdentifier create(
        String tname, String bname, String iname, boolean find)
        throws RpcException {
        VnodeName vtnName = VNodeType.VTN.checkName(tname, true);
        VnodeName vtermName = VNodeType.VTERMINAL.checkName(bname, true);
        VnodeName ifName = VNodeType.VTERMINAL_IF.checkName(iname, find);
        return new VTerminalIfIdentifier(vtnName, vtermName, ifName);
    }

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private VTerminalIfIdentifier() {
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vTerminal.
     * @param iname  The name of the virtual interface.
     */
    public VTerminalIfIdentifier(VnodeName tname, VnodeName bname,
                                 VnodeName iname) {
        super(tname, bname, iname);
    }

    /**
     * Construct a new instance.
     *
     * @param vtmId  A {@link VTerminalIdentifier} instance that specifies the
     *               vTerminal.
     * @param iname  The name of the virtual interface.
     */
    public VTerminalIfIdentifier(VTerminalIdentifier vtmId, VnodeName iname) {
        super(vtmId.getTenantName(), vtmId.getBridgeName(), iname);
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid vTerminal interface path
     *               components.
     */
    VTerminalIfIdentifier(String[] comps) {
        super(comps[0], comps[1], comps[2]);
    }

    // VInterfaceIdentifier

    /**
     * {@inheritDoc}
     */
    @Override
    public VTerminalIfIdentifier replaceTenantName(VnodeName tname) {
        return new VTerminalIfIdentifier(tname, getBridgeName(),
                                         getInterfaceName());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RedirectDestination toRedirectDestination() {
        // Ignore VTN name.
        return new RedirectDestinationBuilder().
            setTerminalName(getBridgeNameString()).
            setInterfaceName(getInterfaceNameString()).
            build();
    }

    // TenantNodeIdentifier

    /**
     * Return an identifier for the vTerminal that contains the virtual
     * interface specified by this instance.
     *
     * @return  A {@link VTerminalIdentifier} instance.
     */
    @Override
    public VTerminalIdentifier getBridgeIdentifier() {
        return new VTerminalIdentifier(getTenantName(), getBridgeName());
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
     * @return  {@link VNodeType#VTERMINAL_IF}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.VTERMINAL_IF;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<Vinterface> getIdentifierBuilder() {
        return getVTerminalIdentifierBuilder().
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
    public VTerminalIfIdentifier getVNodeIdentifier() {
        return this;
    }
}
