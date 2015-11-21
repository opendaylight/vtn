/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import static org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListId.getFlowDirectionName;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.internal.util.flow.filter.FlowFilterListId;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yangtools.yang.binding.DataObject;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterList;

/**
 * {@code VirtualNode} describes an abstracted data for a virtual node.
 *
 * @param <T>  The type of the target data model.
 * @param <I>  The type of the virtual node identifier.
 */
public abstract class VirtualNode
    <T extends DataObject, I extends VNodeIdentifier<T>>
    extends VirtualElement<T, I> {
    /**
     * Description about the virtual node.
     */
    private String  description;

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the virtual node.
     */
    protected VirtualNode(I ident) {
        super(ident);
    }

    /**
     * Construct a new instance.
     *
     * @param ident  The identifier for the virtual node.
     * @param value  A data object read from the MD-SAL datastore.
     */
    protected VirtualNode(I ident, T value) {
        super(ident, value);
    }

    /**
     * Return the description about the virtual node.
     *
     * @return  The description about the virtual node.
     */
    public final String getDescription() {
        return description;
    }

    /**
     * Set the description about the virtual node.
     *
     * @param desc  The description about the virtual node.
     */
    public final void setDescription(String desc) {
        description = desc;
    }

    /**
     * Create a new {@link FlowFilterList} instance.
     *
     * @param ctx     MD-SAL datastore transaction context.
     * @param out     A boolean value that specifies the flow direction.
     *                {@code true} indicates the output filter list.
     *                {@code false} indicates the input filter list.
     * @param vflist  A {@link VtnFlowFilterList} instance.
     * @return  A {@link FlowFilterList} instance.
     */
    protected final FlowFilterList getFlowFilterList(
        TxContext ctx, boolean out, VtnFlowFilterList vflist) {
        I ident = getIdentifier();
        try {
            FlowFilterList filters = FlowFilterList.create(vflist, false);
            if (!filters.isEmpty()) {
                // Configure the identifier for the flow filter list.
                FlowFilterListId flid = new FlowFilterListId(ident, out);
                filters.setListId(flid);
            }

            return filters;
        } catch (RpcException | RuntimeException e) {
            // This should never happen.
            ctx.log(getLogger(), VTNLogLevel.ERROR, e,
                    "%s: %s: Unable to parse flow filter list: %s",
                    ident, getFlowDirectionName(out), e);
            return new FlowFilterList();
        }
    }
}
