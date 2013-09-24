/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VBridgeIfEvent} describes an cluster event object which notifies that
 * a virtual bridge interface was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VBridgeIfEvent extends VNodeEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5162654527309471540L;

    /**
     * Construct a new virtual bridge interface event.
     *
     * @param path    Path to the virtual bridge interface.
     * @param viface  Information about the virtual bridge interface.
     * @param type    Update type.
     */
    public VBridgeIfEvent(VBridgeIfPath path, VInterface viface,
                          UpdateType type) {
        super(path, viface, type);
    }

    /**
     * Return information about the virtual bridge interface.
     *
     * @return  Information about the virtual bridge interface.
     */
    public VInterface getVInterface() {
        return (VInterface)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of this virtual node.
     */
    @Override
    public String getTypeName() {
        return "virtual bridge interface";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VBridgeIfPath path = (VBridgeIfPath)getPath();
        VInterface viface = getVInterface();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, viface, type);
    }
}
