/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.KeyedLeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkKey;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * Helper class for building port-link instance.
 */
public final class PortLinkBuildHelper
    extends KeyedLeafBuildHelper<PortLink, PortLinkBuilder, PortLinkKey> {
    /**
     * Construct a new instance.
     *
     * @param lid   A {@link LinkId} instance that specifies the link.
     * @param peer  A {@link SalPort} instance that specifies the peer port.
     */
    public PortLinkBuildHelper(LinkId lid, SalPort peer) {
        super(PortLink.class,
              new PortLinkBuilder().setLinkId(lid).setPeer(
                  peer.getNodeConnectorId()));
    }
}
