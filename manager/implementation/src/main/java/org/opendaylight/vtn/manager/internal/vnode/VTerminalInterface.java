/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNPortMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VTerminalInterface} describes a configuration and runtime status for
 * a virtual interface inside vTerminal.
 */
public final class VTerminalInterface extends VInterface<Vterminal> {
    /**
     * A logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTerminalInterface.class);

    /**
     * Construct a new instance.
     *
     * <p>
     *   This constructor is used to resume runtime status of the virtual
     *   interface on bootstrap.
     * </p>
     *
     * @param ifId  The location of the virtual interface.
     * @param vifc  Configuration of the virtual interface.
     * @param pmap  Port mapping configuration.
     */
    public VTerminalInterface(VInterfaceIdentifier<Vterminal> ifId,
                              VtnVinterfaceConfig vifc, VTNPortMapConfig pmap) {
        super(ifId, vifc, pmap);
    }

    /**
     * Construct a new instance.
     *
     * @param ifId  The location of the virtual interface.
     * @param vif   A {@link Vinterface} instance read from the MD-SAL
     *              datastore.
     */
    public VTerminalInterface(VInterfaceIdentifier<Vterminal> ifId,
                              Vinterface vif) {
        super(ifId, vif);
    }

    // Vinterface

    /**
     * This method does nothing.
     *
     * @param ctx    MD-SAL datastore transaction context.
     * @param sport  A {@link SalPort} instance that specifies the physical
     *               switch port mapped by the port mapping.
     * @param vid    The VLAN ID specified by the port mapping configuration.
     */
    @Override
    protected void purgePortCache(TxContext ctx, SalPort sport, int vid) {
    }

    // VirtualElement

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
