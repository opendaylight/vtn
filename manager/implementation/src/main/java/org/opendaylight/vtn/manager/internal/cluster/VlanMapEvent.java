/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VlanMapEvent} describes an cluster event object which notifies that
 * a VLAN mapping was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VlanMapEvent extends VNodeEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5983073727151900538L;

    /**
     * Generate a VLAN mapping event which indicates the VLAN mapping has
     * been added.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual bridge associated with the VLAN
     *               mapping.
     * @param vlmap  Information about the VLAN mapping.
     */
    public static void added(VTNManagerImpl mgr, VBridgePath path,
                             VlanMap vlmap) {
        mgr.enqueueEvent(new VlanMapEvent(path, vlmap, UpdateType.ADDED,
                                          true));
    }

    /**
     * Generate a VLAN mapping event which indicates the VLAN mapping has
     * been removed.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the virtual bridge associated with the VLAN
     *               mapping.
     * @param vlmap  Information about the VLAN mapping.
     * @param save   {@code true} means that the tenant configuration should
     *               be saved.
     */
    public static void removed(VTNManagerImpl mgr, VBridgePath path,
                               VlanMap vlmap, boolean save) {
        mgr.enqueueEvent(new VlanMapEvent(path, vlmap, UpdateType.REMOVED,
                                          save));
    }

    /**
     * Construct a new VLAN mapping event.
     *
     * @param path   Path to the virtual bridge associated with the VLAN
     *               mapping.
     * @param vlmap  Information about the VLAN mapping.
     * @param type   Update type.
     * @param save   {@code true} means that the tenant configuration should
     *               be saved.
     */
    public VlanMapEvent(VBridgePath path, VlanMap vlmap, UpdateType type,
                        boolean save) {
        super(path, vlmap, type, save);
    }

    /**
     * Return information about the VLAN mapping.
     *
     * @return  Information about the VLAN mapping.
     */
    public VlanMap getVlanMap() {
        return (VlanMap)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of this virtual node.
     */
    @Override
    public String getTypeName() {
        return "VLAN mapping";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VBridgePath path = (VBridgePath)getPath();
        VlanMap vlmap = getVlanMap();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, vlmap, type);
    }
}
