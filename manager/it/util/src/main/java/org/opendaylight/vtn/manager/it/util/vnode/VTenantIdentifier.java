/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import java.util.Collections;
import java.util.List;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code VTenantIdentifier} describes an identifier for a VTN.
 */
public final class VTenantIdentifier extends VNodeIdentifier<Vtn> {
    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     */
    public VTenantIdentifier(VnodeName tname) {
        super(tname, null, null);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     */
    public VTenantIdentifier(String tname) {
        super(tname, null, null);
    }

    /**
     * Construct a new instance from the given path components.
     *
     * @param comps  An array of strings which represents the path components
     *               of identifier. Note that the caller must guarantee that
     *               {@code comps} contains valid VTN path components.
     */
    VTenantIdentifier(String[] comps) {
        super(comps[0], null, null);
    }

    // VNodeIdentifier

    /**
     * Return a {@link VNodeType} instance which indicates the type of the
     * virtual node.
     *
     * @return  {@link VNodeType#VTN}.
     */
    @Override
    public VNodeType getType() {
        return VNodeType.VTN;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public InstanceIdentifierBuilder<Vtn> getIdentifierBuilder() {
        return getVtnIdentifierBuilder();
    }

    /**
     * Return a {@link VNodeIdentifier} that specifies a virtual network
     * element.
     *
     * <p>
     *   This method always returns this instance because VTN is virtual
     *   network element.
     * </p>
     *
     * @return  This instance.
     */
    @Override
    public VTenantIdentifier getVNodeIdentifier() {
        return this;
    }

    /**
     * Return an instance identifier builder rooted at the specified container
     * for the flow filter list.
     *
     * @param output  Always ignored because VTN has a flow filter list only
     *                for incoming packets.
     * @return  An instance identifier builder rooted at the specified
     *          container for the flow filter list.
     */
    @Override
    protected InstanceIdentifierBuilder<VtnInputFilter> getFlowFilterListIdentifierBuilder(
        boolean output) {
        return getVtnIdentifierBuilder().child(VtnInputFilter.class);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> newComponents() {
        return Collections.singletonList(getTenantNameString());
    }
}
