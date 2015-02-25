/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VtnPortListener} maintains physical switch port information detected
 * by the VTN Manager.
 */
public final class VtnPortListener extends DataStoreListener<VtnPort, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VtnPortListener.class);

    /**
     * A map that keeps physical switch port information.
     */
    private final Map<String, VtnPort>  physicalPorts = new HashMap<>();

    /**
     * {@code PortState} describes the condition to be met.
     */
    private interface PortState {
        /**
         * Check the current state of the target port.
         *
         * @param vport  The current port information.
         * @return  {@code true} only if the current state of the target port
         *          meets the condition.
         */
        boolean check(VtnPort vport);
    }

    /**
     * {@code PortCreated} checks whether the target port has been created
     * or not.
     */
    private static final class PortCreated implements PortState {
        /**
         * {@inheritDoc}
         */
        @Override
        public boolean check(VtnPort vport) {
            return (vport != null);
        }
    }

    /**
     * {@code PortRemoved} checks whether the target port has been removed
     * or not.
     */
    private static final class PortRemoved implements PortState {
        /**
         * {@inheritDoc}
         */
        @Override
        public boolean check(VtnPort vport) {
            return (vport == null);
        }
    }

    /**
     * {@code LinkState} checks the link-up state of the target port.
     * or not.
     */
    private static final class LinkState implements PortState {
        /**
         * The expected link state.
         */
        private final boolean  linkUp;

        /**
         * Construct a new instance.
         *
         * @param up   The expected link state.
         */
        private LinkState(boolean up) {
            linkUp = up;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public boolean check(VtnPort vport) {
            if (vport == null) {
                return false;
            }

            return (Boolean.TRUE.equals(vport.isEnabled()) == linkUp);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param broker  Data broker service.
     */
    public VtnPortListener(DataBroker broker) {
        super(VtnPort.class);
        registerListener(broker, LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.BASE);
    }

    /**
     * Wait for the given port to be created.
     *
     * @param pid  The port identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void awaitCreated(String pid) throws InterruptedException {
        if (!await(pid, new PortCreated())) {
            throw new IllegalStateException(
                "The port was not created: " + pid);
        }
    }

    /**
     * Wait for the given port to be removed.
     *
     * @param pid  The port identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void awaitRemoved(String pid) throws InterruptedException {
        if (!await(pid, new PortRemoved())) {
            throw new IllegalStateException(
                "The port was not removed: " + pid);
        }
    }

    /**
     * Wait for the given port to change its link state.
     *
     * @param pid  The port identifier.
     * @param up   The expected link state.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void awaitLinkState(String pid, boolean up)
        throws InterruptedException {
        if (!await(pid, new LinkState(up))) {
            StringBuilder builder =
                new StringBuilder("The link state was not changed: ");
            builder.append(pid).append(", ").append(up);
            throw new IllegalStateException(builder.toString());
        }
    }

    /**
     * Return the port identifier in the given instance identifier.
     *
     * @param path  A {@link InstanceIdentifier} instance.
     * @return  The port identifier.
     */
    private String getIdentifier(InstanceIdentifier<?> path) {
        VtnPortKey key = path.firstKeyOf(VtnPort.class, VtnPortKey.class);
        if (key == null) {
            throw new IllegalArgumentException(
                "Unexpected VtnPort path: " + path);
        }

        return key.getId().getValue();
    }

    /**
     * Wait for the given port to meet the given condition.
     *
     * @param pid   The target port identifier.
     * @param cond  A {@link PortState} instance.
     * @return  {@code true} only if the given port has satisfied the given
     *          condition.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private synchronized boolean await(String pid, PortState cond)
        throws InterruptedException {
        if (cond.check(physicalPorts.get(pid))) {
            return true;
        }

        long timeout = OfMockProvider.TASK_TIMEOUT;
        long deadline = System.currentTimeMillis() + timeout;
        do {
            wait(timeout);
            if (cond.check(physicalPorts.get(pid))) {
                return true;
            }
            timeout = deadline - System.currentTimeMillis();
        } while (timeout > 0);

        return cond.check(physicalPorts.get(pid));
    }

    /**
     * Update the port information.
     *
     * @param path   Path to the target {@link VtnPort} instance.
     * @param vport  The target {@link VtnPort} instance.
     */
    private synchronized void update(InstanceIdentifier<?> path,
                                     VtnPort vport) {
        String pid = getIdentifier(path);
        if (vport == null) {
            physicalPorts.remove(pid);
        } else {
            physicalPorts.put(pid, vport);
        }

        notifyAll();
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected Void enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(Void ectx) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(Void ectx, InstanceIdentifier<VtnPort> path,
                             VtnPort data) {
        update(path, data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx, InstanceIdentifier<VtnPort> path,
                             VtnPort oldData, VtnPort newData) {
        update(path, newData);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx, InstanceIdentifier<VtnPort> path,
                             VtnPort data) {
        update(path, null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnPort> getWildcardPath() {
        return InstanceIdentifier.builder(VtnNodes.class).child(VtnNode.class).
            child(VtnPort.class).build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Set<VtnUpdateType> getRequiredEvents() {
        return null;
    }
}
