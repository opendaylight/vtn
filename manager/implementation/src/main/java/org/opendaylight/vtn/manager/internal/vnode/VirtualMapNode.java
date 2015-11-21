/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.flow.VNodeHop;
import org.opendaylight.vtn.manager.internal.util.flow.filter.DropFlowException;
import org.opendaylight.vtn.manager.internal.util.flow.filter.RedirectFlowException;
import org.opendaylight.vtn.manager.internal.util.vnode.TenantNodeIdentifier;

/**
 * {@code VirtualMapNode} determines interfaces to be implemented by virtual
 * node classes which map the physical network element.
 */
public interface VirtualMapNode {
    /**
     * Return the identifier for this node.
     *
     * @return  The identifier for this node.
     */
    TenantNodeIdentifier<?, ?> getIdentifier();

    /**
     * Return the identifier for the virtual mapping which maps the given host.
     *
     * @param eaddr  An {@link EtherAddress} instance that specifies the MAC
     *               address of the mapped host.
     *               {@code null} should be treated as if the mapped host is
     *               not specified.
     * @param vid    The VLAN ID of the mapped host.
     * @return  The identifier for the virtual network mapping.
     */
    TenantNodeIdentifier<?, ?> getIdentifier(EtherAddress eaddr, int vid);

    /**
     * Determine whether this virtual mapping is administratively enabled
     * or not.
     *
     * @return  {@code true} only if this mapping is enabled.
     */
    boolean isEnabled();

    /**
     * Return a {@link VNodeHop} instance that represents the ingress virtual
     * node in the virtual packet route.
     *
     * @param eaddr  An {@link EtherAddress} instance that specifies the MAC
     *               address of the mapped host.
     *               {@code null} should be treated as if the mapped host is
     *               not specified.
     * @param vid    The VLAN ID of the mapped host.
     * @return  A {@link VNodeHop} instance.
     */
    VNodeHop getIngressHop(EtherAddress eaddr, int vid);

    /**
     * Install a flow entry that drops every incoming packet.
     *
     * @param pctx  A runtime context for a received packet.
     */
    void disableInput(PacketContext pctx);

    /**
     * Evaluate flow filters for incoming packet mapped by this virtual
     * mapping.
     *
     * @param pctx  A runtime context for a received packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    void filterPacket(PacketContext pctx, int vid)
        throws DropFlowException, RedirectFlowException;

    /**
     * Evaluate flow filters for outgoing packet to be transmitted by this
     * virtual mapping.
     *
     * @param pctx    A runtime context for a received packet.
     * @param vid     A VLAN ID to be used for packet matching.
     *                A VLAN ID configured in the given packet is used if a
     *                negative value is specified.
     * @param bridge  A {@link VirtualBridge} instance associated with this
     *                virtual mapping.
     * @return  A {@link PacketContext} to be used for transmitting packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    PacketContext filterPacket(PacketContext pctx, int vid,
                               VirtualBridge<?> bridge)
        throws DropFlowException, RedirectFlowException;
}
