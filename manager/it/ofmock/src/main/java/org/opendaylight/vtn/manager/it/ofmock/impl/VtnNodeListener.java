/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.HashSet;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VtnNodeListener} maintains physical switch information detected by
 * the VTN Manager.
 */
public final class VtnNodeListener extends DataStoreListener<VtnNode, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VtnNodeListener.class);

    /**
     * A set of node identifiers detected by the VTN Manager.
     */
    private final Set<String>  switches = new HashSet<>();

    /**
     * {@code NodeState} describes the condition to be met.
     */
    private interface NodeState {
        /**
         * Check the current state of the target node.
         *
         * @param present  {@code true} if the target node is present.
         * @return  {@code true} only if the current state of the target node
         *          meets the condition.
         */
        boolean check(boolean present);
    }

    /**
     * {@code NodeCreated} checks whether the target node has been created
     * or not.
     */
    private static final class NodeCreated implements NodeState {
        /**
         * {@inheritDoc}
         */
        @Override
        public boolean check(boolean present) {
            return present;
        }
    }

    /**
     * {@code NodeRemoved} checks whether the target node has been removed
     * or not.
     */
    private static final class NodeRemoved implements NodeState {
        /**
         * {@inheritDoc}
         */
        @Override
        public boolean check(boolean present) {
            return !present;
        }
    }

    /**
     * Construct a new instance.
     *
     * @param broker  Data broker service.
     */
    public VtnNodeListener(DataBroker broker) {
        super(VtnNode.class);
        registerListener(broker, LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.ONE);
    }

    /**
     * Wait for the given node to be created.
     *
     * @param nid      The node identifier.
     * @param timeout  The number of milliseconds to wait.
     * @return  {@code true} if the given node has been created.
     *          Otherwise {@code false}.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public boolean awaitCreated(String nid, long timeout)
        throws InterruptedException {
        return await(nid, new NodeCreated(), timeout);
    }

    /**
     * Wait for the given node to be created.
     *
     * @param nid  The node identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void awaitCreated(String nid) throws InterruptedException {
        if (!await(nid, new NodeCreated(), OfMockProvider.TASK_TIMEOUT)) {
            String msg = "The node was not created: " + nid;
            LOG.error(msg);
            throw new IllegalStateException(msg);
        }
    }

    /**
     * Wait for the given node to be removed.
     *
     * @param nid  The node identifier.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void awaitRemoved(String nid) throws InterruptedException {
        if (!await(nid, new NodeRemoved(), OfMockProvider.TASK_TIMEOUT)) {
            String msg = "The node was not removed: " + nid;
            LOG.error(msg);
            throw new IllegalStateException(msg);
        }
    }

    /**
     * Return the node identifier in the given instance identifier.
     *
     * @param path  A {@link InstanceIdentifier} instance.
     * @return  The node identifier.
     */
    private String getIdentifier(InstanceIdentifier<?> path) {
        VtnNodeKey key = path.firstKeyOf(VtnNode.class);
        if (key == null) {
            String msg = "Unexpected VtnNode path: " + path;
            LOG.error(msg);
            throw new IllegalArgumentException(msg);
        }

        return key.getId().getValue();
    }

    /**
     * Wait for the given node to meet the given condition.
     *
     * @param nid      The target node identifier.
     * @param cond     A {@link NodeState} instance.
     * @param timeout  The number of milliseconds to wait.
     * @return  {@code true} only if the given node has satisfied the given
     *          condition.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    private synchronized boolean await(String nid, NodeState cond,
                                       long timeout)
        throws InterruptedException {
        if (cond.check(switches.contains(nid))) {
            return true;
        }

        long tm = timeout;
        long deadline = System.currentTimeMillis() + timeout;
        do {
            wait(tm);
            if (cond.check(switches.contains(nid))) {
                return true;
            }
            tm = deadline - System.currentTimeMillis();
        } while (tm > 0);

        return cond.check(switches.contains(nid));
    }

    /**
     * Update the node information.
     *
     * @param path   Path to the target {@link VtnNode} instance.
     * @param vnode  The target {@link VtnNode} instance.
     */
    private synchronized void update(InstanceIdentifier<?> path,
                                     VtnNode vnode) {
        String nid = getIdentifier(path);
        if (vnode == null) {
            switches.remove(nid);
        } else {
            switches.add(nid);
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
    protected void onCreated(Void ectx, InstanceIdentifier<VtnNode> path,
                             VtnNode data) {
        update(path, data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx, InstanceIdentifier<VtnNode> path,
                             VtnNode oldData, VtnNode newData) {
        update(path, newData);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx, InstanceIdentifier<VtnNode> path,
                             VtnNode data) {
        update(path, null);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnNode> getWildcardPath() {
        return InstanceIdentifier.builder(VtnNodes.class).child(VtnNode.class).
            build();
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
