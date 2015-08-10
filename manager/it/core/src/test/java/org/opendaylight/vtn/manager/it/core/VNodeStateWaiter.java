/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.util.TestBase.unexpected;

import java.util.Dictionary;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;

import org.osgi.framework.ServiceRegistration;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.ServiceHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

/**
 * {@code VNodeStateWaiter} is a utility class used to wait for the state
 * of virtual node to be changed.
 */
public class VNodeStateWaiter extends VTNManagerAwareAdapter
    implements Runnable {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.
        getLogger(VNodeStateWaiter.class);

    /**
     * The number of milliseconds to wait the vBridge state to be changed.
     */
    private static final long  WAIT_TIMEOUT = 10000L;

    /**
     * VTN Manager service.
     */
    private final IVTNManager  vtnManager;

    /**
     * A list of virtual node state conditions.
     */
    private final Map<VNodePath, StateCond>  conditions = new HashMap<>();

    /**
     * Set {@code true} if one of the target virtual nodes has been changed
     * as expected.
     */
    private boolean  changed;

    /**
     * {@code VNodeStateCond} describes the condition for virtual node state.
     *
     * @param <P>  The type of the virtual node path.
     */
    private abstract static class StateCond<P extends VNodePath> {
        /**
         * Path to the target virtual node.
         */
        private final P  targetPath;

        /**
         * Construct a new instance.
         *
         * @param path  Path to the target virtual node.
         */
        private StateCond(P path) {
            targetPath = path;
        }

        /**
         * Return path to the target virtual node.
         *
         * @return  path to the target virtual node.
         */
        protected final P getTargetPath() {
            return targetPath;
        }

        /**
         * Fetch the current state of the virtual node, and determine whether
         * it is the expected state or not.
         *
         * @param mgr  VTN Manager service.
         * @return  {@code true} only if the current state of the virtual node
         *          is the expected state.
         * @throws VTNException  An error occurred.
         */
        protected final boolean checkState(IVTNManager mgr)
            throws VTNException {
            return isExpected(getState(mgr));
        }

        /**
         * Fetch the current state of the virtual node.
         *
         * @param mgr  VTN Manager service.
         * @return  An object which represents the current state of the
         *          target virtual node.
         * @throws VTNException  An error occurred.
         */
        protected abstract Object getState(IVTNManager mgr)
            throws VTNException;

        /**
         * Determine whether the given object represents the expected state
         * of the target virtual node or not.
         *
         * @param o  An object to be tested.
         * @return   {@code true} only if the given object represents the
         *           expected state of the target virtual node.
         */
        protected abstract boolean isExpected(Object o);
    }

    /**
     * {@code VBridgeStateCond} describes the condition for vBridge state.
     */
    private static class VBridgeStateCond extends StateCond<VBridgePath> {
        /**
         * Expected state of the vBridge.
         */
        private final VnodeState  expectedState;

        /**
         * Expected fault count.
         */
        private final int  expectedFaults;

        /**
         * Construct a new instance.
         *
         * @param path    A path to the target vBridge.
         * @param state   The expected state of the vBridge.
         * @param faults  The expected value of the path fault count.
         *                A negative value means that the path fault count
         *                should not be observed.
         */
        private VBridgeStateCond(VBridgePath path, VnodeState state,
                                 int faults) {
            super(path);
            expectedState = state;
            expectedFaults = faults;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected Object getState(IVTNManager mgr) throws VTNException {
            return mgr.getBridge(getTargetPath());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isExpected(Object o) {
            if (o instanceof VBridge) {
                VBridge vbr = (VBridge)o;
                return (expectedState.equals(vbr.getState()) &&
                        expectedFaults == vbr.getFaults());
            }

            return false;
        }
    }

    /**
     * {@code VTerminalStateCond} describes the condition for vTerminal state.
     */
    private static class VTerminalStateCond extends StateCond<VTerminalPath> {
        /**
         * Expected state of the vTerminal.
         */
        private final VnodeState  expectedState;

        /**
         * Expected fault count.
         */
        private final int  expectedFaults;

        /**
         * Construct a new instance.
         *
         * @param path    A path to the target vTerminal.
         * @param state   The expected state of the vTerminal.
         * @param faults  The expected value of the path fault count.
         *                A negative value means that the path fault count
         *                should not be observed.
         */
        private VTerminalStateCond(VTerminalPath path, VnodeState state,
                                   int faults) {
            super(path);
            expectedState = state;
            expectedFaults = faults;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected Object getState(IVTNManager mgr) throws VTNException {
            return mgr.getTerminal(getTargetPath());
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isExpected(Object o) {
            if (o instanceof VTerminal) {
                VTerminal vterm = (VTerminal)o;
                return (expectedState.equals(vterm.getState()) &&
                        expectedFaults == vterm.getFaults());
            }

            return false;
        }
    }

    /**
     * {@code VInterfaceStateCond} describes the condition for virtual
     * interface state.
     *
     * @param <P>  The type of the virtual interface path.
     */
    private abstract static class VInterfaceStateCond<P extends VNodePath>
        extends StateCond<P> {
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
         * @param path    Path to the target virtual interface.
         * @param state   The expected state of the virtual interface.
         * @param estate  The expected state of the network element mapped to
         *                the virtual interface.
         */
        private VInterfaceStateCond(P path, VnodeState state,
                                    VnodeState estate) {
            super(path);
            expectedState = state;
            expectedEntityState = estate;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected final boolean isExpected(Object o) {
            if (o instanceof VInterface) {
                VInterface vif = (VInterface)o;
                return (expectedState.equals(vif.getState()) &&
                        expectedEntityState.equals(vif.getEntityState()));
            }

            return false;
        }
    }

    /**
     * {@code VBridgeIfStateCond} describes the condition for the state of
     * the virtual interface attached to the vBridge.
     */
    private static class VBridgeIfStateCond
        extends VInterfaceStateCond<VBridgeIfPath> {
        /**
         * Construct a new instance.
         *
         * @param path    Path to the target virtual interface.
         * @param state   The expected state of the virtual interface.
         * @param estate  The expected state of the network element mapped to
         *                the virtual interface.
         */
        private VBridgeIfStateCond(VBridgeIfPath path, VnodeState state,
                                   VnodeState estate) {
            super(path, state, estate);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected Object getState(IVTNManager mgr) throws VTNException {
            return mgr.getInterface(getTargetPath());
        }
    }

    /**
     * {@code VTerminalIfStateCond} describes the condition for the state of
     * the virtual interface attached to the vTerminal.
     */
    private static class VTerminalIfStateCond
        extends VInterfaceStateCond<VTerminalIfPath> {
        /**
         * Construct a new instance.
         *
         * @param path    Path to the target virtual interface.
         * @param state   The expected state of the virtual interface.
         * @param estate  The expected state of the network element mapped to
         *                the virtual interface.
         */
        private VTerminalIfStateCond(VTerminalIfPath path, VnodeState state,
                                     VnodeState estate) {
            super(path, state, estate);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected Object getState(IVTNManager mgr) throws VTNException {
            return mgr.getInterface(getTargetPath());
        }
    }

    /**
     * Construct a new instance.
     *
     * @param mgr  VTN Manager service.
     */
    public VNodeStateWaiter(IVTNManager mgr) {
        vtnManager = mgr;
    }

    /**
     * Set condition for the vBridge.
     *
     * <p>
     *   This method specifies zero as the expected path fault count.
     * </p>
     *
     * @param path   Path to the target vBridge.
     * @param state  The expected state of the vBridge.
     * @return  This instance.
     */
    public VNodeStateWaiter set(VBridgePath path, VnodeState state) {
        return set(path, state, 0);
    }

    /**
     * Set condition for the vBridge.
     *
     * @param path    Path to the target vBridge.
     * @param state   The expected state of the vBridge.
     * @param faults  The expected value of the path fault count.
     * @return  This instance.
     */
    public VNodeStateWaiter set(VBridgePath path, VnodeState state,
                                int faults) {
        conditions.put(path, new VBridgeStateCond(path, state, faults));
        return this;
    }

    /**
     * Set condition for the vTerminal.
     *
     * <p>
     *   This method specifies zero as the expected path fault count.
     * </p>
     *
     * @param path   Path to the target vTerminal.
     * @param state  The expected state of the vTerminal
     * @return  This instance.
     */
    public VNodeStateWaiter set(VTerminalPath path, VnodeState state) {
        return set(path, state, 0);
    }

    /**
     * Set condition for the vTerminal.
     *
     * @param path    Path to the target vTerminal.
     * @param state   The expected state of the vTerminal.
     * @param faults  The expected value of the path fault count.
     * @return  This instance.
     */
    public VNodeStateWaiter set(VTerminalPath path, VnodeState state,
                                int faults) {
        conditions.put(path, new VTerminalStateCond(path, state, faults));
        return this;
    }

    /**
     * Set condition for the virtual interface attached to the
     * vBridge.
     *
     * @param path    Path to the target vBridge interface.
     * @param state   The expected state of the interface.
     * @param estate  The expected state of the network element mapped to the
     *                target interface.
     * @return  This instance.
     */
    public VNodeStateWaiter set(VBridgeIfPath path, VnodeState state,
                                VnodeState estate) {
        conditions.put(path, new VBridgeIfStateCond(path, state, estate));
        return this;
    }

    /**
     * Set condition for the virtual interface attached to the vTerminal.
     *
     * @param path    Path to the target vTerminal interface.
     * @param state   The expected state of the interface.
     * @param estate  The expected state of the network element mapped to the
     *                target interface.
     * @return  This instance.
     */
    public VNodeStateWaiter set(VTerminalIfPath path, VnodeState state,
                                VnodeState estate) {
        conditions.put(path, new VTerminalIfStateCond(path, state, estate));
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
     * @throws VTNException
     *    Failed to fetch the current state of the virtual node.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public void await() throws VTNException, InterruptedException {
        if (checkState()) {
            return;
        }

        // Register VTN listener.
        Dictionary<String, Object> props = new Hashtable<String, Object>();
        ServiceRegistration reg = ServiceHelper.
            registerServiceWReg(IVTNManagerAware.class,
                                GlobalConstants.DEFAULT.toString(), this,
                                props);
        if (reg == null) {
            throw new IllegalStateException("Failed to add VTN listener.");
        }

        try {
            long timeout = WAIT_TIMEOUT;
            long limit = System.currentTimeMillis() + timeout;
            synchronized (this) {
                if (checkState()) {
                    return;
                }

                while (!changed) {
                    if (timeout <= 0) {
                        throw new IllegalStateException("Timed out.");
                    }
                    wait(timeout);
                    timeout = limit - System.currentTimeMillis();
                }
            }
        } finally {
            reg.unregister();
        }
    }

    /**
     * Determine whether the current virtual node state meets all the
     * conditions configured in this instance.
     *
     * @throws VTNException
     *    Failed to fetch the current state of the virtual node.
     * @return  {@code true} only if all the conditions are satisfied.
     */
    private boolean checkState() throws VTNException {
        for (StateCond c: conditions.values()) {
            if (!c.checkState(vtnManager)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Evaluate the conditions.
     *
     * @param path  Path to the virtual node.
     * @param o     An object which represents the virtual node state.
     * @param type  Update type.
     */
    private void evaluate(VNodePath path, Object o, UpdateType type) {
        if (type.equals(UpdateType.REMOVED)) {
            return;
        }

        for (StateCond c: conditions.values()) {
            if (path.equals(c.getTargetPath()) && c.isExpected(o)) {
                synchronized (this) {
                    changed = true;
                    notifyAll();
                }
                break;
            }
        }
    }

    // IVTNManagerAware

    /**
     * {@inheritDoc}
     */
    @Override
    public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                               UpdateType type) {
        evaluate(path, vbridge, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                                 UpdateType type) {
        evaluate(path, vterm, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vInterfaceChanged(VBridgeIfPath path, VInterface viface,
                                  UpdateType type) {
        evaluate(path, viface, type);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void vInterfaceChanged(VTerminalIfPath path, VInterface viface,
                                  UpdateType type) {
        evaluate(path, viface, type);
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
