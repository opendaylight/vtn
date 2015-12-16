/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import static org.opendaylight.vtn.manager.it.util.TestBase.unexpected;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.it.ofmock.DataChangeWaiter;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;

import org.opendaylight.vtn.manager.it.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VNodeIdentifier;

import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;

/**
 * {@code VNodeStateWaiter} is a utility class used to wait for the virtual
 * node status to be changed.
 */
public final class VNodeStateWaiter implements Runnable {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.
        getLogger(VNodeStateWaiter.class);

    /**
     * ofmock service.
     */
    private final OfMockService  ofMock;

    /**
     * A set of expected virtual node status.
     */
    private final Map<VNodeIdentifier<?>, VNodeCond<?>>  conditions =
        new HashMap<>();

    /**
     * Set {@code true} if one of the target virtual nodes has been changed
     * as expected.
     */
    private boolean  changed;

    /**
     * {@code VNodeCond} describes the condition for virtual node status.
     *
     * @param <T>  The type of the target data object.
     */
    private abstract static class VNodeCond<T extends DataObject> {
        /**
         * The identifier for the target virtual node.
         */
        private final VNodeIdentifier<?>  vnodeId;

        /**
         * Path to the target data object.
         */
        private final InstanceIdentifier<T>  targetPath;

        /**
         * Data change waiter.
         */
        private DataChangeWaiter<T>  waiter;

        /**
         * Construct a new instance.
         *
         * @param ident  The identifier for the target virtual node.
         * @param path   Path to the target virtual node.
         */
        private VNodeCond(VNodeIdentifier<?> ident,
                          InstanceIdentifier<T> path) {
            vnodeId = ident;
            targetPath = path;
        }

        /**
         * Return the identifier for the target virtual node.
         *
         * @return  A {@link VNodeIdentifier} instance.
         */
        protected final VNodeIdentifier<?> getIdentifier() {
            return vnodeId;
        }

        /**
         * Register a data change waiter.
         *
         * @param mock  ofmock service.
         */
        protected final void setUp(OfMockService mock) {
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            waiter = mock.newDataChangeWaiter(oper, targetPath);
        }

        /**
         * Close the data change waiter.
         */
        protected final void close() {
            if (waiter != null) {
                waiter.close();
                waiter = null;
            }
        }

        /**
         * Throw an error if this condition is not satisfied.
         */
        protected final void verify() {
            verify(waiter.getValue());
        }

        /**
         * Throw an error if this condition is not satisfied.
         *
         * @param limit  The system time in milliseconds which specifies
         *               the limit of the wait.
         * @throws InterruptedException
         *    The calling thread was interrupted.
         */
        protected final void verify(long limit) throws InterruptedException {
            if (!check(waiter.getValue())) {
                if (LOG.isTraceEnabled()) {
                    LOG.trace("Waiting: now={}, limit={}, cond={}, value={}",
                              System.currentTimeMillis(), limit,
                              this, toString(waiter.getValue()));
                }

                while (waiter.await(limit)) {
                    if (LOG.isTraceEnabled()) {
                        LOG.trace("Woke up: cond={}, value={}",
                                  this, toString(waiter.getValue()));
                    }
                    if (check(waiter.getValue())) {
                        return;
                    }
                }

                verify(waiter.getValue());
            }
        }

        /**
         * Determine whether the given object satisfies the condition or not.
         *
         * @param value  A data object to be tested.
         * @return   {@code true} only if the given object satisfies the
         *           condition.
         */
        protected abstract boolean check(T value);

        /**
         * Throw an error if the given object does not satisfy the condition.
         *
         * @param value  A data object to be tested.
         */
        protected abstract void verify(T value);

        /**
         * Return a string representation of the target data.
         *
         * @param value  A data object.
         * @return  A string representation of the given object.
         */
        protected abstract String toString(T value);
    }

    /**
     * {@code BridgeStatusCond} describes the condition for the status of
     * virtual bridge.
     */
    private static final class BridgeStatusCond
        extends VNodeCond<BridgeStatus> {
        /**
         * Expected state of the virtual bridge.
         */
        private final VnodeState  expectedState;

        /**
         * Expected set of faulted paths.
         */
        private final Set<NodePath>  faultedPaths;

        /**
         * Construct a new instance.
         *
         * @param ident   The identifier for the target virtual bridge.
         * @param state   The expected state of the virtual bridge.
         * @param faults  The expected set of faulted paths.
         *                A negative value means that the path fault count
         *                should not be observed.
         */
        private BridgeStatusCond(BridgeIdentifier<?> ident, VnodeState state,
                                 Set<NodePath> faults) {
            super(ident, ident.getStatusPath());
            expectedState = state;
            faultedPaths = (faults == null)
                ? Collections.<NodePath>emptySet()
                : new HashSet<NodePath>(faults);
        }

        /**
         * Convert the given list of {@link FaultedPaths} instances into a
         * set of {@link NodePath} instances.
         *
         * @param faults  A list of {@link FaultedPaths} instances.
         * @return  A set of {@link NodePath} instances.
         */
        private Set<NodePath> toNodePathSet(List<FaultedPaths> faults) {
            Set<NodePath> npaths;
            if (faults == null || faults.isEmpty()) {
                npaths = Collections.<NodePath>emptySet();
            } else {
                npaths = new HashSet<>();
                for (FaultedPaths fpath: faults) {
                    npaths.add(new NodePath(fpath));
                }
            }

            return npaths;
        }

        /**
         * Return a string that represents the virtual bridge status.
         *
         * @param state    The state of the virtual bridge.
         * @param nfaults  The number of faulted paths.
         * @param fpaths   A set of faulted paths.
         */
        private String getDescription(VnodeState state, int nfaults,
                                      Set<NodePath> fpaths) {
            return "state=" + state + ", nfaults=" + nfaults +
                ", faulted-paths=" + fpaths;
        }

        // VNodeCond

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean check(BridgeStatus value) {
            boolean result = (value != null);
            if (result) {
                Set<NodePath> npaths = toNodePathSet(value.getFaultedPaths());
                int nfaults = value.getPathFaults().intValue();
                result = (value.getState() == expectedState &&
                          npaths.equals(faultedPaths) &&
                          nfaults == faultedPaths.size());
            }

            return result;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void verify(BridgeStatus value) {
            String desc = toString();
            if (value == null) {
                fail("Virtual bridge is not present: " + desc);
            }

            Set<NodePath> npaths = toNodePathSet(value.getFaultedPaths());
            int nfaults = value.getPathFaults().intValue();
            assertEquals(desc, faultedPaths, npaths);
            assertEquals(desc, faultedPaths.size(), nfaults);
            assertEquals(desc, expectedState, value.getState());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected String toString(BridgeStatus value) {
            Set<NodePath> npaths = toNodePathSet(value.getFaultedPaths());
            int nfaults = value.getPathFaults().intValue();
            return getDescription(value.getState(), nfaults, npaths);
        }

        // Object

        /**
         * Return a string representation of this object.
         *
         * @return  A string representation of this object.
         */
        @Override
        public String toString() {
            return "path=" + getIdentifier() + ", " +
                getDescription(expectedState, faultedPaths.size(),
                               faultedPaths);
        }
    }

    /**
     * {@code VInterfaceStatusCond} describes the condition for the satus of
     * virtual interface.
     */
    private static final class VInterfaceStatusCond
        extends VNodeCond<VinterfaceStatus> {
        /**
         * Expected state of the virtual interface.
         */
        private final VnodeState  expectedState;

        /**
         * Expected state of the network element mapped to the virtual
         * interface.
         */
        private final VnodeState  expectedEntityState;

        /**
         * Construct a new instance.
         *
         * @param ident   The identifier for the target virtual interface.
         * @param state   The expected state of the virtual interface.
         * @param estate  The expected state of the network element mapped to
         *                the virtual interface.
         */
        private VInterfaceStatusCond(VInterfaceIdentifier<?> ident,
                                     VnodeState state, VnodeState estate) {
            super(ident, ident.getStatusPath());
            expectedState = state;
            expectedEntityState = estate;
        }

        /**
         * Return a string that represents the virtual interface status.
         *
         * @param state   The state of the virtual bridge.
         * @param estate  The state of the network element mapped to the
         *                virtual interface.
         */
        private String getDescription(VnodeState state, VnodeState estate) {
            return "state=" + state + ", entity-state=" + estate;
        }

        // VNodeCond

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean check(VinterfaceStatus value) {
            boolean result = (value != null);
            if (result) {
                result = (value.getState() == expectedState &&
                          value.getEntityState() == expectedEntityState);
            }

            return result;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void verify(VinterfaceStatus value) {
            String desc = toString();
            if (value == null) {
                fail("Virtual interface is not present: " + desc);
            }

            assertEquals(desc, expectedState, value.getState());
            assertEquals(desc, expectedEntityState, value.getEntityState());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected String toString(VinterfaceStatus value) {
            return getDescription(value.getState(), value.getEntityState());
        }

        // Object

        /**
         * Return a string representation of this object.
         *
         * @return  A string representation of this object.
         */
        @Override
        public String toString() {
            return "path=" + getIdentifier() + ", " +
                getDescription(expectedState, expectedEntityState);
        }
    }

    /**
     * Condition evaluator.
     */
    private static final class Evaluator implements AutoCloseable {
        /**
         * The number of milliseconds to wait the virtual node status to be
         * changed.
         */
        private static final long  WAIT_TIMEOUT = 10000L;

        /**
         * A set of data change waiters.
         */
        private final Set<VNodeCond<?>>  condSet = new HashSet<>();

        /**
         * Construct a new instance.
         *
         * @param mock   ofmock service.
         * @param conds  A collection of {@link VNodeCond} instances.
         */
        private Evaluator(OfMockService mock, Collection<VNodeCond<?>> conds) {
            for (VNodeCond<?> cond: conds) {
                cond.setUp(mock);
                condSet.add(cond);
            }
        }

        /**
         * Evaluate all the conditions in this instance.
         *
         * @throws InterruptedException
         *    The calling thread was interrupted.
         */
        private void evaluate() throws InterruptedException {
            long limit = System.currentTimeMillis() + WAIT_TIMEOUT;

            for (VNodeCond<?> cond: condSet) {
                cond.verify(limit);
            }

            // Ensure that all the conditions are still satisfied without
            // blocking the calling thread.
            for (VNodeCond<?> cond: condSet) {
                cond.verify();
            }
        }

        // AutoCloseable

        /**
         * Unregister all the data change waiters.
         */
        @Override
        public void close() {
            for (Iterator<VNodeCond<?>> it = condSet.iterator();
                 it.hasNext();) {
                VNodeCond<?> cond = it.next();
                it.remove();
                cond.close();
            }
        }
    }

    /**
     * Construct a new instance.
     *
     * @param mock  ofmock service.
     */
    public VNodeStateWaiter(OfMockService mock) {
        ofMock = mock;
    }

    /**
     * Set condition for the vBridge or vTerminal.
     *
     * <p>
     *   This method specifies zero as the expected path fault count.
     * </p>
     *
     * @param ident  The identifier for the target virtual bridge.
     * @param state  The expected state of the vBridge.
     * @return  This instance.
     */
    public VNodeStateWaiter set(BridgeIdentifier<?> ident, VnodeState state) {
        return set(ident, state, null);
    }

    /**
     * Set condition for the vBridge.
     *
     * @param ident   The identifier for the target virtual bridge.
     * @param state   The expected state of the vBridge.
     * @param faults  A set of expected faulted paths.
     * @return  This instance.
     */
    public VNodeStateWaiter set(BridgeIdentifier<?> ident, VnodeState state,
                                Set<NodePath> faults) {
        InstanceIdentifier<BridgeStatus> spath = ident.getStatusPath();
        conditions.put(ident, new BridgeStatusCond(ident, state, faults));
        return this;
    }

    /**
     * Set condition for the virtual interface attached to the virtual bridge.
     *
     * @param ident   The identifier for the target virtual interface.
     * @param state   The expected state of the interface.
     * @param estate  The expected state of the network element mapped to the
     *                target interface.
     * @return  This instance.
     */
    public VNodeStateWaiter set(VInterfaceIdentifier<?> ident,
                                VnodeState state, VnodeState estate) {
        conditions.put(ident, new VInterfaceStatusCond(ident, state, estate));
        return this;
    }

    /**
     * Clear all conditions.
     *
     * @return  This instance.
     */
    public VNodeStateWaiter clear() {
        conditions.clear();
        return this;
    }

    /**
     * Wait for the virtual node state to be changed as expected.
     *
     * <p>
     *   An error will be thrown if at least one condition is not satisfied.
     * </p>
     *
     * @return  This instance.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public VNodeStateWaiter await() throws InterruptedException {
        try (Evaluator eval = new Evaluator(ofMock, conditions.values())) {
            eval.evaluate();
        }
        return this;
    }

    // Runnable

    /**
     * Block the calling thread until the virtual node state is changed
     * as expected.
     */
    @Override
    public void run() {
        try {
            await();
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
