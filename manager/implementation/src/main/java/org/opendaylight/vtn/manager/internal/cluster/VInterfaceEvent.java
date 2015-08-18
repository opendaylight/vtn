/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VInterfaceEvent} describes an cluster event object which notifies
 * that a virtual interface was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class VInterfaceEvent extends VNodeEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5543358873095038613L;

    /**
     * Generate a virtual interface event which indicates the virtual interface
     * has been added.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual interface.
     * @param viface  Information about the virtual interface.
     */
    public static void added(VTNManagerImpl mgr, VInterfacePath path,
                             VInterface viface) {
        mgr.enqueueEvent(new VInterfaceEvent(path, viface, UpdateType.ADDED,
                                             true));
    }

    /**
     * Generate a virtual interface event which indicates the virtualinterface
     * has been changed.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual interface.
     * @param viface  Information about the virtual interface.
     * @param save    {@code true} means that the tenant configuration should
     *                be saved.
     */
    public static void changed(VTNManagerImpl mgr, VInterfacePath path,
                               VInterface viface, boolean save) {
        mgr.enqueueEvent(new VInterfaceEvent(path, viface, UpdateType.CHANGED,
                                             save));
    }

    /**
     * Generate a virtual interface event which indicates the virtual interface
     * has been removed.
     *
     * @param mgr     VTN Manager service.
     * @param path    Path to the virtual interface.
     * @param viface  Information about the virtual interface.
     * @param save    {@code true} means that the tenant configuration should
     *                be saved.
     */
    public static void removed(VTNManagerImpl mgr, VInterfacePath path,
                               VInterface viface, boolean save) {
        mgr.enqueueEvent(new VInterfaceEvent(path, viface, UpdateType.REMOVED,
                                             save));
    }

    /**
     * Construct a new virtual interface event.
     *
     * @param path    Path to the virtual interface.
     * @param viface  Information about the virtual interface.
     * @param type    Update type.
     * @param save    {@code true} means that the tenant configuration should
     *                be saved.
     */
    private VInterfaceEvent(VInterfacePath path, VInterface viface,
                            UpdateType type, boolean save) {
        super((VNodePath)path, viface, type, save);
    }

    /**
     * Return information about the virtual interface.
     *
     * @return  Information about the virtual interface.
     */
    public VInterface getVInterface() {
        return (VInterface)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of the virtual node.
     */
    @Override
    public String getTypeName() {
        return "virtual interface";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VInterfacePath path = (VInterfacePath)getPath();
        VInterface viface = getVInterface();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, viface, type);
    }
}
