/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code MacMapEvent} describes an cluster event object which notifies that
 * a MAC mapping was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class MacMapEvent extends VNodeEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1601379292210958855L;

    /**
     * Generate a MAC mapping event which indicates the MAC mapping has
     * been added.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual bridge associated with the MAC
     *                mapping.
     * @param mcconf  Configuration information about the MAC mapping.
     */
    public static void added(VTNManagerImpl mgr, VBridgePath path,
                             MacMapConfig mcconf) {
        mgr.enqueueEvent(new MacMapEvent(path, mcconf, UpdateType.ADDED,
                                         true));
    }

    /**
     * Generate a MAC mapping event which indicates the MAC mapping has
     * been changed.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual bridge associated with the MAC
     *                mapping.
     * @param mcconf  Configuration information about the MAC mapping.
     */
    public static void changed(VTNManagerImpl mgr, VBridgePath path,
                               MacMapConfig mcconf) {
        mgr.enqueueEvent(new MacMapEvent(path, mcconf, UpdateType.CHANGED,
                                         true));
    }

    /**
     * Generate a MAC mapping event which indicates the MAC mapping has
     * been removed.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual bridge associated with the MAC
     *                mapping.
     * @param mcconf  Configuration information about the MAC mapping.
     * @param save    {@code true} means that the tenant configuration should
     *                be saved.
     */
    public static void removed(VTNManagerImpl mgr, VBridgePath path,
                               MacMapConfig mcconf, boolean save) {
        mgr.enqueueEvent(new MacMapEvent(path, mcconf, UpdateType.REMOVED,
                                         save));
    }

    /**
     * Construct a new MAC mapping event.
     *
     * @param path    Path to the virtual bridge associated with the MAC
     *                mapping.
     * @param mcconf  Configuration information about the MAC mapping.
     * @param type    Update type.
     * @param save    {@code true} means that the tenant configuration should
     *                be saved.
     */
    public MacMapEvent(VBridgePath path, MacMapConfig mcconf, UpdateType type,
                       boolean save) {
        super(path, mcconf, type, save);
    }

    /**
     * Return information about the MAC mapping configuration.
     *
     * @return  Information about the MAC mapping configuration.
     */
    public MacMapConfig getMacMapConfig() {
        return (MacMapConfig)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of this virtual node.
     */
    @Override
    public String getTypeName() {
        return "MAC mapping";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VBridgePath path = (VBridgePath)getPath();
        MacMapConfig mcconf = getMacMapConfig();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, mcconf, type);
    }
}
