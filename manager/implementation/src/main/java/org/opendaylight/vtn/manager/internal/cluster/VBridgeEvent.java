/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VBridgeEvent} describes an cluster event object which notifies that
 * a virtual bridge was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VBridgeEvent extends VNodeEvent {
    private static final long serialVersionUID = 4839916486356062624L;

    /**
     * Construct a new virtual bridge event.
     *
     * @param path     Path to the virtual bridge.
     * @param vbridge  Information about the virtual bridge.
     * @param type     Update type.
     */
    public VBridgeEvent(VBridgePath path, VBridge vbridge, UpdateType type) {
        super(path, vbridge, type);
    }

    /**
     * Return information about the virtual bridge.
     *
     * @return  Information about the virtual bridge.
     */
    public VBridge getVBridge() {
        return (VBridge)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of this virtual node.
     */
    @Override
    public String getTypeName() {
        return "virtual bridge";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VBridgePath path = (VBridgePath)getPath();
        VBridge vbridge = getVBridge();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, vbridge, type);
    }
}
