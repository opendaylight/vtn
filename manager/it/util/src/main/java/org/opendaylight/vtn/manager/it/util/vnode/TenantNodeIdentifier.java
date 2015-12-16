/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * {@code TenantNodeIdentifier} describes an identifier for a virtual node
 * inside VTN, such as vBridge and virtual interface.
 *
 * @param <T>  The type of the target data model.
 * @param <B>  The type of the virtual bridge.
 */
public abstract class TenantNodeIdentifier
    <T extends DataObject, B extends VtnPortMappableBridge>
    extends VNodeIdentifier<T> {
    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     */
    TenantNodeIdentifier(VnodeName tname, VnodeName bname) {
        this(tname, bname, null);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     * @param iname  The name of the virtual interface.
     */
    TenantNodeIdentifier(VnodeName tname, VnodeName bname, VnodeName iname) {
        super(tname, bname, iname);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     */
    TenantNodeIdentifier(String tname, String bname) {
        super(tname, new VnodeName(bname), null);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     * @param iname  The name of the virtual interface.
     */
    TenantNodeIdentifier(String tname, String bname, String iname) {
        super(tname, new VnodeName(bname), new VnodeName(iname));
    }

    /**
     * Return an identifier for the virtual bridge that contains the data
     * object specified by this instance.
     *
     * @return  An identifier for the virtual bridge.
     */
    public abstract BridgeIdentifier<B> getBridgeIdentifier();

    /**
     * Return the virtual bridge name configured in the given
     * {@link VnodePathFields} instance.
     *
     * @param vpath  A {@link VnodePathFields} instance.
     *               {@code null} cannot be specified.
     * @return  The name of the virtual bridge configured in {@code vpath}.
     *          {@code null} if the virtual bridge name is not configured
     *          in {@code vpath}.
     */
    protected abstract String getBridgeName(VnodePathFields vpath);

    /**
     * Set the virtual bridge name into the specified
     * {@link VirtualNodePathBuilder} instance.
     *
     * @param builder  A {@link VirtualNodePathBuilder} instance.
     * @return  {@code builder}.
     */
    protected abstract VirtualNodePathBuilder setBridgeName(
        VirtualNodePathBuilder builder);

    /**
     * Return an instance identifier builder which contains the key of the
     * vBridge.
     *
     * @return  An instance identifier builder.
     */
    protected final InstanceIdentifierBuilder<Vbridge> getVBridgeIdentifierBuilder() {
        return getVtnIdentifierBuilder().
            child(Vbridge.class, new VbridgeKey(getBridgeName()));
    }

    /**
     * Return an instance identifier builder which contains the key of the
     * vTerminal.
     *
     * @return  An instance identifier builder.
     */
    protected final InstanceIdentifierBuilder<Vterminal> getVTerminalIdentifierBuilder() {
        return getVtnIdentifierBuilder().
            child(Vterminal.class, new VterminalKey(getBridgeName()));
    }

    // VNodeIdentifier

    /**
     * Determine whether the virtua node specified by this instance contains
     * the virtual node specified by {@link VirtualNodePath} instance.
     *
     * <p>
     *   This method in this class checks only the VTN and virtual bridge name.
     *   Subclass may override this method.
     * </p>
     *
     * @param vpath  A {@link VirtualNodePath} instance that specifies the
     *               virtual node. {@code null} cannot be specified.
     * @return  {@code true} if the virtual node specified by this instance
     *          contains the virtual node specified by {@code vpath}.
     *          {@code false} otherwise.
     */
    @Override
    public boolean contains(VirtualNodePath vpath) {
        String bname = getBridgeName(vpath);
        return (bname != null && super.contains(vpath) &&
                bname.equals(getBridgeNameString()));
    }

    /**
     * Create a new {@link VirtualNodePathBuilder} instance that contains the
     * path components configured in this instance.
     *
     * <p>
     *   This method in this class sets only the VTN and virtual bridge name.
     *   Subclass may override this method.
     * </p>
     *
     * @return  A {@link VirtualNodePathBuilder} instance.
     */
    @Override
    protected VirtualNodePathBuilder getVirtualNodePathBuilder() {
        return setBridgeName(super.getVirtualNodePathBuilder());
    }
}
