/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * This interface provides methods to notify listeners about the Virtual
 * Tenant Network changes.
 */
public interface IVTNManagerAware {
    /**
     * Invoked when a virtual tenant is added, removed, or changed.
     *
     * @param path     Path to the tenant.
     * @param vtenant  Information about the virtual tenant.
     * @param type     {@code ADDED} if added.
     *                 {@code REMOVED} if removed.
     *                 {@code CHANGED} if changed.
     */
    public void vtnChanged(VTenantPath path, VTenant vtenant, UpdateType type);

    /**
     * Invoked when a virtual L2 bridge is added, removed, or changed.
     *
     * @param path     Path to the bridge.
     * @param vbridge  Information about the virtual L2 bridge.
     * @param type     {@code ADDED} if added.
     *                 {@code REMOVED} if removed.
     *                 {@code CHANGED} if changed.
     */
    public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                               UpdateType type);

    /**
     * Invoked when a virtual L2 bridge interface is added, removed, or
     * changed.
     *
     * @param path    Path to the interface.
     * @param viface  Information about the virtual interface.
     * @param type    {@code ADDED} if added.
     *                {@code REMOVED} if removed.
     *                {@code CHANGED} if changed.
     */
    public void vBridgeInterfaceChanged(VBridgeIfPath path, VInterface viface,
                                        UpdateType type);

    /**
     * Invoked when a VLAN is mapped to the virtual L2 bridge, or unmapped.
     *
     * @param path   Path to the bridge associated with the VLAN mapping.
     * @param vlmap  Information about the VLAN mapping.
     * @param type   {@code ADDED} if added.
     *               {@code REMOVED} if removed.
     */
    public void vlanMapChanged(VBridgePath path, VlanMap vlmap,
                               UpdateType type);

    /**
     * Invoked when a mapping between virtual bridge interface between
     * a physical switch port is established or destroyed.
     *
     * @param path  Path to the bridge interface associated with the port
     *              mapping.
     * @param pmap  Information about the port mapping.
     * @param type  {@code ADDED} if established.
     *              {@code REMOVED} if destroyed.
     *              {@code CHANGED} if changed.
     */
    public void portMapChanged(VBridgeIfPath path, PortMap pmap,
                               UpdateType type);
}
