/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
     * Determine whether this node is enabled or not.
     *
     * @return  {@code true} is returned only if this node is enabled.
     */
    boolean isEnabled();

    /**
     * Return a {@link VNodeRoute} instance which represents the ingress
     * virtual node.
     *
     * @return  A {@link VNodeRoute} instance.
     */
    VNodeRoute getIngressRoute();

    /**
     * Install a flow entry which drops every incoming packet.
     *
     * @param mgr   VTN Manager service.
     * @param pctx  The context of the received packet.
     */
    void disableInput(VTNManagerImpl mgr, PacketContext pctx);
}
