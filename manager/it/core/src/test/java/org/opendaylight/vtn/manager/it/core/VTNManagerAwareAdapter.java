/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * An adapter class for {@link IVTNManagerAware}.
 */
public class VTNManagerAwareAdapter implements IVTNManagerAware {
    /**
     * Invoked when the information related to the VTN is changed.
     *
     * @param path     A {@link VTenantPath} object that specifies the
     *                 position of the VTN.
     * @param vtenant  A {@link VTenant} object which represents the VTN
     *                 information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void vtnChanged(VTenantPath path, VTenant vtenant,
                           UpdateType type) {
    }

    /**
     * Invoked when the information related to the vBridge is changed.
     *
     * @param path     A {@link VBridgePath} object that specifies the
     *                 position of the vBridge.
     * @param vbridge  A {@link VBridge} object which represents the
     *                 vBridge information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                               UpdateType type) {
    }

    /**
     * Invoked when the information related to vTerminal inside the
     * container is changed.
     *
     * @param path   A {@link VTerminalPath} object that specifies the
     *               position of the vTerminal.
     * @param vterm  A {@link VTerminal} object which represents the
     *               vTerminal information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                                 UpdateType type) {
    }

    /**
     * Invoked when the information related to the virtual interface
     * configured in the vBridge is changed.
     *
     * @param path    A {@link VBridgeIfPath} object that specifies the
     *                position of the vBridge interface.
     * @param viface  A {@link VInterface} object which represents the
     *                vBridge interface information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void vInterfaceChanged(VBridgeIfPath path, VInterface viface,
                                  UpdateType type) {
    }

    /**
     * Invoked when the information related to the virtual interface
     * configured in the vTerminal is changed.
     *
     * @param path    A {@link VTerminalIfPath} object that specifies the
     *                position of the vBridge interface.
     * @param viface  A {@link VInterface} object which represents the
     *                vTerminal interface information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void vInterfaceChanged(VTerminalIfPath path, VInterface viface,
                                  UpdateType type) {
    }

    /**
     * Invoked when the information related to the VLAN mapping
     * configured in the vBridge is changed.
     *
     * @param path   A {@link VBridgePath} object that specifies the
     *               position of the VBridge.
     * @param vlmap  A {@link VlanMap} object which represents the VLAN
     *               mapping information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void vlanMapChanged(VBridgePath path, VlanMap vlmap,
                               UpdateType type) {
    }

    /**
     * Invoked when the information related to the port mapping
     * configured in the vBridge interface is changed.
     *
     * @param path  A {@link VBridgeIfPath} object that specifies the
     *              position of the vBridge interface.
     * @param pmap  A {@link PortMap} object which represents the
     *              port mapping information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void portMapChanged(VBridgeIfPath path, PortMap pmap,
                               UpdateType type) {
    }

    /**
     * Invoked when the information related to the port mapping
     * configured in the vTerminal interface is changed.
     *
     * @param path  A {@link VTerminalIfPath} object that specifies the
     *              position of the vTerminal interface.
     * @param pmap  A {@link PortMap} object which represents the
     *              port mapping information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void portMapChanged(VTerminalIfPath path, PortMap pmap,
                               UpdateType type) {
    }

    /**
     * Invoked when the information related to the MAC mapping configured
     * in vBridge is changed.
     *
     * @param path    A {@link VBridgePath} object that specifies the
     *                position of the VBridge.
     * @param mcconf  A {@link MacMapConfig} object which represents
     *                the MAC mapping configuration information.
     * @param type
     *   An {@link UpdateType} object which indicates the type of
     *   modification is specified.
     */
    @Override
    public void macMapChanged(VBridgePath path, MacMapConfig mcconf,
                              UpdateType type) {
    }
}
