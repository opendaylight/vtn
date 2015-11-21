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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VbridgePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code VBridgeIdentifier} describes an identifier for a vBridge.
 */
public final class VBridgeIdentifier extends BridgeIdentifier<Vbridge> {
    /**
     * Create a new identifier.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     * @param find   {@code true} indicates the given name is used for
     *               finding existing virtual node.
     * @return  A {@link VBridgeIdentifier} instance.
     * @throws RpcException  Invalid name is specified.
     */
    public static VBridgeIdentifier create(
        String tname, String bname, boolean find) throws RpcException {
        VnodeName vtnName = VNodeType.VTN.checkName(tname, true);
        VnodeName vbrName = VNodeType.VBRIDGE.checkName(bname, find);
        return new VBridgeIdentifier(vtnName, vbrName);
    }

    /**
     * Create a new identifier.
     *
     * @param path  A {@link VbridgePathFields} instance.
     * @param find  {@code true} indicates the given name is used for
     *              finding existing virtual node.
     * @return  A {@link VBridgeIdentifier} instance.
     * @throws RpcException  Invalid name is specified.
     */
    public static VBridgeIdentifier create(
        VbridgePathFields path, boolean find) throws RpcException {
        return create(path.getTenantName(), path.getBridgeName(), find);
    }


    /**
     * Create an instance identifier that specifies the MAC address table list
     * the specified VTN.
     *
     * @param ident  A virtual node identifier.
     * @return  An instance identifier that specifies the MAC address table
     *          list for the specified vBridge.
     */
    public static InstanceIdentifier<TenantMacTable> getTenantMacTablePath(
        VNodeIdentifier<?> ident) {
        String tname = ident.getTenantNameString();
        TenantMacTableKey tkey = new TenantMacTableKey(tname);
        return InstanceIdentifier.builder(MacTables.class).
            child(TenantMacTable.class, tkey).build();
    }

    /**
     * Create an instance identifier that specifies the MAC address table for
     * the specified vBridge.
     *
     * @param ident  An identifier for the vBridge.
     * @return  An instance identifier that specifies the MAC address table
     *          for the specified vBridge.
     */
    public static InstanceIdentifier<MacAddressTable> getMacTablePath(
        BridgeIdentifier<Vbridge> ident) {
        return getMacTablePathBuilder(ident).build();
    }

    /**
     * Create an instance identifier that specifies the MAC address table
     * entry for the specified MAC address learned by the specified vBridge.
     *
     * @param ident  An identifier for the vBridge.
     * @param mac    A MAC adderss.
     * @return  An instance identifier that specifies the MAC address table
     *          entry.
     */
    public static InstanceIdentifier<MacTableEntry> getMacEntryPath(
        BridgeIdentifier<Vbridge> ident, MacAddress mac) {
        return getMacTablePathBuilder(ident).
            child(MacTableEntry.class, new MacTableEntryKey(mac)).
            build();
    }

    /**
     * Create an instance identifier builder rooted at the MAC address table
     * for the specified vBridge.
     *
     * @param ident  An identifier for the vBridge.
     * @return  An instance identifier builder rooted at the MAc address table
     *          for the specified vBridge.
     */
    private static InstanceIdentifierBuilder<MacAddressTable> getMacTablePathBuilder(
        BridgeIdentifier<Vbridge> ident) {
        String tname = ident.getTenantNameString();
        String bname = ident.getBridgeNameString();
        TenantMacTableKey tkey = new TenantMacTableKey(tname);
        MacAddressTableKey bkey = new MacAddressTableKey(bname);
        return InstanceIdentifier.builder(MacTables.class).
            child(TenantMacTable.class, tkey).
            child(MacAddressTable.class, bkey);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the vBridge.
     */
    public VBridgeIdentifier(VnodeName tname, VnodeName bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  A {@link VNodeIdentifier} instance that specifies the
     *               VTN.
     * @param bname  The name of the vBridge.
     */
    public VBridgeIdentifier(VNodeIdentifier<?> ident, VnodeName bname) {
        super(ident.getTenantName(), bname);
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid vBridge path components.
     */
    VBridgeIdentifier(String[] comps) {
        super(comps[0], comps[1]);
    }

    // BridgeIdentifier

    /**
     * Construct a new identifier for a virtual interface inside the vBridge
     * specified by this instance.
     *
     * @param iname  The name of the virtual interface.
     * @return  A {@link VBridgeIfIdentifier} instance.
     */
    @Override
    public VBridgeIfIdentifier childInterface(VnodeName iname) {
        return new VBridgeIfIdentifier(this, iname);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<BridgeStatus> newStatusPath() {
        return getVBridgeIdentifierBuilder().child(BridgeStatus.class).build();
    }

    // TenantNodeIdentifier

    /**
     * Return an identifier for the vBridge.
     *
     * @return  This instance.
     */
    @Override
    public VBridgeIdentifier getBridgeIdentifier() {
        return this;
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

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#VBRIDGE}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.VBRIDGE;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<Vbridge> getIdentifierBuilder() {
        return getVBridgeIdentifierBuilder();
    }

    /**
     * Return a {@link VNodeIdentifier} that specifies a virtual network
     * element.
     *
     * <p>
     *   This method always returns this instance because vBridge is virtual
     *   network element.
     * </p>
     *
     * @return  This instance.
     */
    @Override
    public VBridgeIdentifier getVNodeIdentifier() {
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifierBuilder<? extends VtnFlowFilterList> getFlowFilterListIdentifierBuilder(
        boolean output) {
        InstanceIdentifierBuilder<Vbridge> builder =
            getVBridgeIdentifierBuilder();
        return (output)
            ? builder.child(VbridgeOutputFilter.class)
            : builder.child(VbridgeInputFilter.class);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> newComponents() {
        return Arrays.asList(getTenantNameString(), getBridgeNameString());
    }
}
