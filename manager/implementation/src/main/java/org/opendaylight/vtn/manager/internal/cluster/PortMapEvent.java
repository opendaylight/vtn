/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VlanMapEvent} describes an cluster event object which notifies that
 * a port mapping was added, modified, or removed.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class PortMapEvent extends VNodeEvent {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7275615707564769661L;

    /**
     * Construct a new port mapping event.
     *
     * @param path  Path to the virtual bridge interface associated with the
     *              port mapping.
     * @param pmap  Information about the port mapping.
     * @param type  Update type.
     */
    public PortMapEvent(VBridgeIfPath path, PortMap pmap, UpdateType type) {
        super(path, pmap, type);
    }

    /**
     * Return information about the port mapping.
     *
     * @return  Information about the port mapping.
     */
    public PortMap getPortMap() {
        return (PortMap)getObject();
    }

    /**
     * Return the name of node type.
     *
     * @return  A string which describes the type of this virtual node.
     */
    @Override
    public String getTypeName() {
        return "port mapping";
    }

    /**
     * Deliver this event to {@code IVTNManagerAware} listeners.
     *
     * @param mgr  VTN Manager service.
     */
    @Override
    public void notifyEvent(VTNManagerImpl mgr) {
        VBridgeIfPath path = (VBridgeIfPath)getPath();
        PortMap pmap = getPortMap();
        UpdateType type = getUpdateType();
        mgr.notifyListeners(path, pmap, type);
    }
}
