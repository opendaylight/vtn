/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VNodeRoute;

import org.opendaylight.vtn.manager.internal.PacketContext;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * {@code VirtualMapNode} determines interfaces to be implemented by virtual
 * node classes which map the physical network element.
 *
 * <p>
 *   Although this interface is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public interface VirtualMapNode extends Serializable {
    /**
     * Return path to this node.
     *
     * @return  Path to the node.
     */
    VNodePath getPath();

    /**
     * Return path to the virtual mapping which maps the given host.
     *
     * @param mac   A long value which represents the MAC address of the mapped
     *              host. {@link MacVlan#UNDEFINED} should be treated as if
     *              the mapped host is not specified.
     * @param vlan  VLAN ID of the mapped host.
     * @return  Path to the virtual network mapping.
     */
    VNodePath getPath(long mac, short vlan);

    /**
     * Determine whether this node is enabled or not.
     *
     * @return  {@code true} is returned only if this node is enabled.
     */
    boolean isEnabled();

    /**
     * Return a {@link VNodeRoute} instance which represents the ingress
     * virtual node.
     *
     * @param mac   A long value which represents the MAC address of the mapped
     *              host. {@link MacVlan#UNDEFINED} should be treated as if
     *              the mapped host is not specified.
     * @param vlan  VLAN ID of the mapped host.
     * @return  A {@link VNodeRoute} instance.
     */
    VNodeRoute getIngressRoute(long mac, short vlan);

    /**
     * Install a flow entry which drops every incoming packet.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     */
    void disableInput(VTNManagerImpl mgr, PacketContext pctx);

    /**
     * Evaluate flow filters for incoming packet mapped by this virtual
     * mapping.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     * @param vid   A VLAN ID to be used for packet matching.
     *              A VLAN ID configured in the given packet is used if a
     *              negative value is specified.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    void filterPacket(VTNManagerImpl mgr, PacketContext pctx, short vid)
        throws DropFlowException, RedirectFlowException;

    /**
     * Evaluate flow filters for outgoing packet to be transmitted by this
     * virtual mapping.
     *
     * @param mgr     VTN Manager service.
     * @param pctx    The context of the received packet.
     * @param vid     A VLAN ID to be used for packet matching.
     *                A VLAN ID configured in the given packet is used if a
     *                negative value is specified.
     * @param bridge  A {@link PortBridge} instance associated with this
     *                virtual mapping.
     * @return  A {@link PacketContext} to be used for transmitting packet.
     * @throws DropFlowException
     *    The given packet was discarded by a flow filter.
     * @throws RedirectFlowException
     *    The given packet was redirected by a flow filter.
     */
    PacketContext filterPacket(VTNManagerImpl mgr, PacketContext pctx,
                               short vid, PortBridge<?> bridge)
        throws DropFlowException, RedirectFlowException;
}
