/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VTerminalEvent} describes an cluster event object which notifies that
 * a vTerminal was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VTerminalEvent extends VNodeEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1307047949724999264L;

    /**
     * Generate a vTerminal event which indicates the vTerminal has been added.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the vTerminal.
     * @param vterm  Information about the vTerminal.
     */
    public static void added(VTNManagerImpl mgr, VTerminalPath path,
                             VTerminal vterm) {
        mgr.enqueueEvent(new VTerminalEvent(path, vterm, UpdateType.ADDED,
                                            true));
    }

    /**
     * Generate a vTerminal event which indicates the vTerminal has been
     * changed.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the vTerminal.
     * @param vterm  Information about the vTerminal.
     * @param save   {@code true} means that the tenant configuration should
     *               be saved.
     */
    public static void changed(VTNManagerImpl mgr, VTerminalPath path,
                               VTerminal vterm, boolean save) {
        mgr.enqueueEvent(new VTerminalEvent(path, vterm, UpdateType.CHANGED,
                                            save));
    }

    /**
     * Generate a vTerminal event which indicates the vTerminal has been
     * removed.
     *
     * @param mgr    VTN Manager service.
     * @param path   Path to the vTerminal.
     * @param vterm  Information about the vTerminal.
     * @param save   {@code true} means that the tenant configuration should
     *               be saved.
     */
    public static void removed(VTNManagerImpl mgr, VTerminalPath path,
                               VTerminal vterm, boolean save) {
        mgr.enqueueEvent(new VTerminalEvent(path, vterm, UpdateType.REMOVED,
                                            save));
    }

    /**
     * Construct a new vTerminal event.
     *
     * @param path   Path to the vTerminal.
     * @param vterm  Information about the vTerminal.
     * @param type   Update type.
     * @param save   {@code true} means that the tenant configuration should
     *               be saved.
     */
    private VTerminalEvent(VTerminalPath path, VTerminal vterm,
                           UpdateType type, boolean save) {
        super(path, vterm, type, save);
    }

    /**
     * Return information about the vTerminal.
     *
     * @return  Information about the vTerminal.
     */
    public VTerminal getVTerminal() {
        return (VTerminal)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of this virtual node.
     */
    @Override
    public String getTypeName() {
        return "vTerminal";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VTerminalPath path = (VTerminalPath)getPath();
        VTerminal vterm = getVTerminal();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, vterm, type);
    }
}
