/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import java.util.ArrayList;
import java.util.List;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code InventoryEvents} keeps a set of VTN inventory events to be delivered
 * to inventory listeners.
 */
final class InventoryEvents {
    /**
     * A list of events that notify created VTN nodes.
     */
    private List<VtnNodeEvent>  createdNodes;

    /**
     * A list of events that notify changed VTN nodes.
     */
    private List<VtnNodeEvent>  changedNodes;

    /**
     * A list of events that notify removed VTN nodes.
     */
    private List<VtnNodeEvent>  removedNodes;

    /**
     * A list of events that notify created VTN ports.
     */
    private List<VtnPortEvent>  createdPorts;

    /**
     * A list of events that notify changed VTN ports.
     */
    private List<VtnPortEvent>  changedPorts;

    /**
     * A list of events that notify removed VTN ports.
     */
    private List<VtnPortEvent>  removedPorts;

    /**
     * Add the specified VTN node event into this instance.
     *
     * @param ev  A {@link VtnNodeEvent} instance.
     */
    void add(VtnNodeEvent ev) {
        VtnUpdateType type = ev.getUpdateType();
        if (type == VtnUpdateType.CREATED) {
            createdNodes = add(createdNodes, ev);
        } else if (type == VtnUpdateType.REMOVED) {
            removedNodes = add(removedNodes, ev);
        } else {
            changedNodes = add(changedNodes, ev);
        }
    }

    /**
     * Add the specified VTN port event into this instance.
     *
     * @param ev  A {@link VtnPortEvent} instance.
     */
    void add(VtnPortEvent ev) {
        VtnUpdateType type = ev.getUpdateType();
        if (type == VtnUpdateType.CREATED) {
            createdPorts = add(createdPorts, ev);
        } else if (type == VtnUpdateType.REMOVED) {
            removedPorts = add(removedPorts, ev);
        } else {
            changedPorts = add(changedPorts, ev);
        }
    }

    /**
     * Publish all the events in this instance to the specified listeners.
     *
     * @param provider   VTN Manager provider service.
     * @param listeners  A list of event listeners.
     */
    void post(VTNManagerProvider provider,
              List<VTNInventoryListener> listeners) {
        // Deliver node creation events first.
        postNodeEvents(provider, createdNodes, listeners);

        // Deliver node change events.
        // Node change events should be delivered before port creation events
        // because it notifies the OpenFlow protocol version that may affect
        // port event listeners.
        postNodeEvents(provider, changedNodes, listeners);

        // Deliver port creation events.
        postPortEvents(provider, createdPorts, listeners);

        // Deliver port removal events.
        postPortEvents(provider, removedPorts, listeners);

        // Deliver node removal events.
        postNodeEvents(provider, removedNodes, listeners);

        // Deliver port change events.
        postPortEvents(provider, changedPorts, listeners);
    }

    /**
     * Add an inventory event to the event list.
     *
     * @param list  A list of events to store the specified event.
     * @param ev    An event to be added.
     * @param <T>   The type of event.
     * @return  {@code list} if the specified event list is not {@code null}.
     *          A new event list if the specified event list is {@code null}.
     */
    private <T extends TxEvent> List<T> add(List<T> list, T ev) {
        List<T> events = (list == null)
            ? new ArrayList<T>()
            : list;
        events.add(ev);
        return events;
    }

    /**
     * Publish all the specified VTN node events to the event listeners.
     *
     * @param provider   VTN Manager provider service.
     * @param events     A list of events to be delivered.
     * @param listeners  A list of event listeners.
     */
    private void postNodeEvents(VTNManagerProvider provider,
                                List<VtnNodeEvent> events,
                                List<VTNInventoryListener> listeners) {
        if (events != null) {
            for (VtnNodeEvent ev: events) {
                for (VTNInventoryListener l: listeners) {
                    provider.post(new VtnNodeEvent(l, ev));
                }
            }
        }
    }

    /**
     * Publish all the specified VTN port events to the event listeners.
     *
     * @param provider   VTN Manager provider service.
     * @param events     A list of events to be delivered.
     * @param listeners  A list of event listeners.
     */
    private void postPortEvents(VTNManagerProvider provider,
                                List<VtnPortEvent> events,
                                List<VTNInventoryListener> listeners) {
        if (events != null) {
            for (VtnPortEvent ev: events) {
                for (VTNInventoryListener l: listeners) {
                    provider.post(new VtnPortEvent(l, ev));
                }
            }
        }
    }
}
