/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VInterfaceConfig;

/**
 * Implementation of virtual interface attached to the vTerminal.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VTerminalIfImpl extends PortInterface {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7131192176450504387L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTerminalIfImpl.class);

    /**
     * Construct a vTerminal interface instance.
     *
     * @param vtm   The vTerminal to which a new interface is attached.
     * @param name  The name of the interface.
     * @param iconf Configuration for the interface.
     */
    VTerminalIfImpl(VTerminalImpl vtm, String name, VInterfaceConfig iconf) {
        super(vtm, name, iconf);
    }

    // AbstractInterface

    /**
     * Return a logger instance.
     *
     * @return  A logger instance.
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
