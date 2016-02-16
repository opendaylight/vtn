/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnPortMappableBridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;

/**
 * {@code BridgeIdentifier} describes an identifier for virtual bridge,
 * such ask vBridge and vTerminal.
 *
 * @param <T>  The type of the target data model.
 */
public abstract class BridgeIdentifier<T extends VtnPortMappableBridge>
    extends TenantNodeIdentifier<T, T> {
    /**
     * Cache for instance identifier that specifies the virtual bridge status.
     */
    private InstanceIdentifier<BridgeStatus>  statusPath;

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     */
    BridgeIdentifier(VnodeName tname, VnodeName bname) {
        super(tname, bname);
    }

    /**
     * Construct a new instance.
     *
     * @param tname  The name of the VTN.
     * @param bname  The name of the virtual bridge.
     */
    BridgeIdentifier(String tname, String bname) {
        super(tname, bname);
    }

    /**
     * Return the path to the virtual bridge status.
     *
     * @return  An {@link InstanceIdentifier} instance for the virtual bridge
     *          status.
     */
    public final InstanceIdentifier<BridgeStatus> getStatusPath() {
        InstanceIdentifier<BridgeStatus> path = statusPath;
        if (path == null) {
            path = newStatusPath();
            statusPath = path;
        }

        return path;
    }

    /**
     * Construct a new identifier for a virtual interface inside the virtual
     * bridge specified by this instance.
     *
     * @param iname  The name of the virtual interface.
     * @return  An identifier for a virtual interface.
     */
    public abstract VInterfaceIdentifier<T> childInterface(VnodeName iname);

    /**
     * Construct a new instance identifier for the virtual bridge status.
     *
     * @return  An {@link InstanceIdentifier} instance for the virtual bridge
     *          status.
     */
    protected abstract InstanceIdentifier<BridgeStatus> newStatusPath();
}
