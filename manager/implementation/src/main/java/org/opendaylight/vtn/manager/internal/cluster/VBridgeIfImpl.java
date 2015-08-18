/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;


import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.internal.MacAddressTable;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * Implementation of virtual interface attached to the vBridge.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VBridgeIfImpl extends PortInterface implements VBridgeNode {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 3130089879301868613L;

    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VBridgeIfImpl.class);

    /**
     * Construct a virtual interface instance.
     *
     * @param vbr   The virtual bridge to which a new interface is attached.
     * @param name  The name of the interface.
     * @param iconf Configuration for the interface.
     */
    VBridgeIfImpl(VBridgeImpl vbr, String name, VInterfaceConfig iconf) {
        super(vbr, name, iconf);
    }

    // PortInterface

    /**
     * Flush cached network data associated with the network specified by
     * a pair of switch port and VLAN ID.
     *
     * <p>
     *   This method is used to flush network data cached by obsolete port
     *   mapping.
     * </p>
     *
     * @param mgr    VTN Manager service.
     * @param port   A node connector associated with a switch port.
     * @param vlan   A VLAN ID.
     */
    @Override
    protected void flushCache(VTNManagerImpl mgr, NodeConnector port,
                              short vlan) {
        VBridgeImpl vbr = (VBridgeImpl)getParent();
        MacAddressTable table = mgr.getMacAddressTable(vbr.getPath());
        if (table != null) {
            // Flush MAC address table entries added by obsolete port mapping.
            table.flush(port, vlan);
        }

        super.flushCache(mgr, port, vlan);
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

    // VBridgeNode

    /**
     * Return path to this interface.
     *
     * @return  Path to the interface.
     */
    @Override
    public VBridgeIfPath getPath() {
        return (VBridgeIfPath)getInterfacePath();
    }
}
