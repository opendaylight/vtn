/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.InventoryUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VtnPortEvent} describes an event which notifies a VTN port has been
 * added, changed, orremoved.
 */
public final class VtnPortEvent extends TxEvent {
    /**
     * The target inventory listener.
     */
    private final VTNInventoryListener  listener;

    /**
     * A {@link SalPort} instance.
     */
    private final SalPort  salPort;

    /**
     * A {@link VtnPort} instance.
     */
    private final VtnPort  vtnPort;

    /**
     * A {@link Boolean} value which describes the change of port running
     * state.
     */
    private final Boolean  stateChange;

    /**
     * A {@link Boolean} value which describes the change of inter-switch link
     * state.
     */
    private final Boolean  islChange;

    /**
     * A {@link VtnUpdateType} instance that specifies the type of notification.
     */
    private final VtnUpdateType  updateType;

    /**
     * Construct a new instance.
     *
     * @param l      A {@link VTNInventoryListener} instance.
     * @param vport  A {@link VtnPort} instance.
     * @param state  A {@link Boolean} instance which describes the change of
     *               running state of the given port.
     *               Note that this argument is ignored if {@code type} is
     *               {@link VtnUpdateType#REMOVED}.
     * @param isl    A {@link Boolean} instance which describes the change of
     *               inter-switch link state.
     *               Note that this argument is ignored if {@code type} is
     *               {@link VtnUpdateType#REMOVED}.
     * @param type   A {@link VtnUpdateType} instance.
     */
    VtnPortEvent(VTNInventoryListener l, VtnPort vport, Boolean state,
                 Boolean isl, VtnUpdateType type) {
        listener = l;
        vtnPort = vport;
        if (type == VtnUpdateType.REMOVED) {
            stateChange = null;
            islChange = null;
        } else {
            stateChange = state;
            islChange = isl;
        }
        updateType = type;
        salPort = SalPort.create(vport.getId());
    }

    /**
     * Create a copy of the given event for the given listener.
     *
     * @param l   A {@link VTNInventoryListener} instance.
     * @param ev  A {@link VtnPortEvent} instance.
     */
    VtnPortEvent(VTNInventoryListener l, VtnPortEvent ev) {
        listener = l;
        vtnPort = ev.vtnPort;
        stateChange = ev.stateChange;
        islChange = ev.islChange;
        updateType = ev.updateType;
        salPort = ev.salPort;
    }

    /**
     * Return a {@link SalPort} instance that represents the location of
     * a swtich port.
     *
     * @return  A {@link SalPort} instance.
     */
    public SalPort getSalPort() {
        return salPort;
    }

    /**
     * Return a {@link VtnPort} instance.
     *
     * @return  A {@link VtnPort} instance.
     */
    public VtnPort getVtnPort() {
        return vtnPort;
    }

    /**
     * Return a {@link Boolean} value which describes the change of
     * running state of the switch port.
     *
     * @return
     *   <ul>
     *     <li>
     *       {@link Boolean#TRUE} if the port status has been changed to UP
     *       state.
     *     </li>
     *     <li>
     *       {@link Boolean#FALSE} if the port status has been changed to DOWN
     *       state.
     *     </li>
     *     <li>
     *       {@code null} if the port status was not changed.
     *     </li>
     *   </ul>
     */
    public Boolean getStateChange() {
        return stateChange;
    }

    /**
     * Return a {@link Boolean} value which describes the change of
     * inter-switch link state.
     *
     * @return
     *   <ul>
     *     <li>
     *       {@link Boolean#TRUE} if the port has become an internal port.
     *     </li>
     *     <li>
     *       {@link Boolean#FALSE} if the port has become an edge port.
     *       switch port.
     *     </li>
     *     <li>
     *       {@code null} if the inter-switch link state was not changed.
     *     </li>
     *   </ul>
     */
    public Boolean getInterSwitchLinkChange() {
        return islChange;
    }

    /**
     * Return the type of notification.
     *
     * @return
     *   <ul>
     *     <li>
     *       {@link VtnUpdateType#CREATED} if a VTN port has been added.
     *     </li>
     *     <li>
     *       {@link VtnUpdateType#CHANGED} if a VTN port has been changed.
     *     </li>
     *     <li>
     *       {@link VtnUpdateType#REMOVED} if a VTN port has been removed.
     *     </li>
     *   </ul>
     */
    public VtnUpdateType getUpdateType() {
        return updateType;
    }

    /**
     * Determine whether the notified switch port is disabled or not.
     *
     * @return  {@code true} if the notified switch port is disabled.
     *          Otherwise {@code false}.
     */
    public boolean isDisabled() {
        return (updateType == VtnUpdateType.REMOVED)
            ? true
            : !InventoryUtils.isEnabled(vtnPort);
    }

    // TxEvent

    /**
     * {@inheritDoc}
     */
    @Override
    protected void notifyEvent() throws VTNException {
        listener.notifyVtnPort(this);
    }
}
