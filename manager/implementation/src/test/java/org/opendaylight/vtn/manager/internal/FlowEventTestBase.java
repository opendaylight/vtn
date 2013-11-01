/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.internal.VTNManagerImplTestCommon.VTNManagerAwareData;

/**
 * Base class for Flow event test case.
 */
public class FlowEventTestBase extends FlowModTaskTestBase {

    /**
     * Add IVTNManagerAware service to VTNManager service.
     * @param listener  IVTNManagerAware service.
     */
    public void addVTNManagerAware(IVTNManagerAware listener) {
        vtnMgr.addVTNManagerAware(listener);
    }

    /**
     * utility class used in VTNManagerAwareStub.
     */
    class VTNManagerAwareData<T, S> {
        T path = null;
        S obj = null;
        UpdateType type = null;
        int count = 0;

        VTNManagerAwareData(T p, S o, UpdateType t, int c) {
            path = p;
            obj = o;
            type = t;
            count = c;
        }
    };

    /**
     * Mock-up of IVTNManagerAware.
     */
    public class VTNManagerAwareStub implements IVTNManagerAware {
        private final long sleepMilliTime = 10L;

        private int vtnChangedCalled = 0;
        private int vbrChangedCalled = 0;
        private int vIfChangedCalled = 0;
        private int vlanMapChangedCalled = 0;
        private int portMapChangedCalled = 0;
        VTNManagerAwareData<VTenantPath, VTenant> vtnChangedInfo = null;
        VTNManagerAwareData<VBridgePath, VBridge> vbrChangedInfo = null;
        VTNManagerAwareData<VBridgeIfPath, VInterface> vIfChangedInfo = null;
        VTNManagerAwareData<VBridgePath, VlanMap> vlanMapChangedInfo = null;
        VTNManagerAwareData<VBridgeIfPath, PortMap> portMapChangedInfo = null;

        @Override
        public synchronized void vtnChanged(VTenantPath path, VTenant vtenant, UpdateType type) {
            vtnChangedCalled++;
            vtnChangedInfo = new VTNManagerAwareData<VTenantPath, VTenant>(path, vtenant,
                    type, vtnChangedCalled);
        }

        @Override
        public synchronized void vBridgeChanged(VBridgePath path, VBridge vbridge, UpdateType type) {
            vbrChangedCalled++;
            vbrChangedInfo = new VTNManagerAwareData<VBridgePath, VBridge>(path, vbridge, type,
                    vbrChangedCalled);
        }

        @Override
        public synchronized void vBridgeInterfaceChanged(VBridgeIfPath path, VInterface viface, UpdateType type) {
            vIfChangedCalled++;
            vIfChangedInfo = new VTNManagerAwareData<VBridgeIfPath, VInterface>(path, viface, type,
                    vIfChangedCalled);
        }

        @Override
        public synchronized void vlanMapChanged(VBridgePath path, VlanMap vlmap, UpdateType type) {
            vlanMapChangedCalled++;
            vlanMapChangedInfo = new VTNManagerAwareData<VBridgePath, VlanMap>(path, vlmap, type,
                    vlanMapChangedCalled);
        }

        @Override
        public synchronized void portMapChanged(VBridgeIfPath path, PortMap pmap, UpdateType type) {
            portMapChangedCalled++;
            portMapChangedInfo = new VTNManagerAwareData<VBridgeIfPath, PortMap>(path, pmap, type,
                    portMapChangedCalled);
        }

        /**
         * check information notified by vtnChanged().
         *
         * @param count     A expected number of times vtnChanged() was called.
         * @param path      A VTenantPath expect to be notified.
         * @param name      A name expect to be notified.
         * @param type      A type expect to be notified.
         */
        public synchronized void checkVtnInfo(int count, VTenantPath path,
                                              UpdateType type) {
            if (vtnChangedCalled < count) {
                long milli = 1000;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vtnChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vtnChangedCalled);

            if (path != null) {
                assertEquals(path, vtnChangedInfo.path);
                assertEquals(path.getTenantName(),
                             vtnChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vtnChangedInfo.type);
            }
            vtnChangedCalled = 0;
            vtnChangedInfo = null;
        }

        /**
         * check information notified by vBridgeChanged().
         *
         * @param count     A expected number of times vBridgeChanged() was called.
         * @param path      A VBridgePath expect to be notified.
         * @param name      A name expect to be notified.
         * @param type      A type expect to be notified.
         */
        public synchronized void checkVbrInfo(int count, VBridgePath path,
                                       String name, UpdateType type) {
            if (vbrChangedCalled < count) {
                long milli = 1000;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vbrChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vbrChangedCalled);

            if (path != null) {
                assertEquals(path, vbrChangedInfo.path);
            }
            if (name != null) {
                assertEquals(name, vbrChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vbrChangedInfo.type);
            }
            vbrChangedCalled = 0;
            vbrChangedInfo = null;
        }

        /**
         * check information notified by vBridgeInterfaceChanged().
         *
         * @param count     A expected number of times
         *                  vBridgeInterfaceChanged() was called.
         * @param path      A VBridgeIfPath expect to be notified.
         * @param name      A name expect to be notified.
         * @param type      A type expect to be notified.
         */
        public synchronized void checkVIfInfo(int count, VBridgeIfPath path,
                                       String name, UpdateType type) {
            if (vIfChangedCalled < count) {
                long milli = 1000;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vIfChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vIfChangedCalled);

            if (path != null) {
                assertEquals(path, vIfChangedInfo.path);
            }
            if (name != null) {
                assertEquals(name, vIfChangedInfo.obj.getName());
            }
            if (type != null) {
                assertEquals(type, vIfChangedInfo.type);
            }
            vIfChangedCalled = 0;
            vIfChangedInfo = null;
        }

        /**
         * check information notified by vlanMapChanged().
         *
         * @param count     A expected number of times vlanMapChanged() was called.
         * @param path      A VBridgePath expect to be notified.
         * @param name      A map-id expect to be notified.
         * @param type      A type expect to be notified.
         */
        public synchronized void checkVlmapInfo(int count, VBridgePath path,
                                         String id, UpdateType type) {
            if (vlanMapChangedCalled < count) {
                long milli = 1000;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (vlanMapChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, vlanMapChangedCalled);

            if (path != null) {
                assertEquals(path, vlanMapChangedInfo.path);
            }
            if (id != null) {
                assertEquals(id, vlanMapChangedInfo.obj.getId());
            }
            if (type != null) {
                assertEquals(type, vlanMapChangedInfo.type);
            }
            vlanMapChangedCalled = 0;
            vlanMapChangedInfo = null;
        }

        /**
         * check information notified by portMapChanged().
         *
         * @param count     A expected number of times portMapChanged() was called.
         * @param path      A VBridgeIfPath expect to be notified.
         * @param name      A PortMapConfig expect to be notified.
         * @param type      A type expect to be notified.
         */
        public synchronized void checkPmapInfo(int count, VBridgeIfPath path,
                                        PortMapConfig pconf, UpdateType type) {
            if (portMapChangedCalled < count) {
                long milli = 1000;
                long limit = System.currentTimeMillis() + milli;
                do {
                    try {
                        wait(milli);
                    } catch (InterruptedException e) {
                        break;
                    }
                    if (portMapChangedCalled >= count) {
                        break;
                    }
                    milli = limit - System.currentTimeMillis();
                } while (milli > 0);
            }
            assertEquals(count, portMapChangedCalled);

            if (path != null) {
                assertEquals(path, portMapChangedInfo.path);
            }
            if (pconf != null) {
                assertEquals(pconf, portMapChangedInfo.obj.getConfig());
            }
            if (type != null) {
                assertEquals(type, portMapChangedInfo.type);
            }
            portMapChangedCalled = 0;
            portMapChangedInfo = null;
        }

        /**
         * check all methods not called.
         */
        public void checkAllNull() {
            sleep(sleepMilliTime);
            assertEquals(0, vtnChangedCalled);
            assertNull(vtnChangedInfo);
            assertEquals(0, vbrChangedCalled);
            assertNull(vbrChangedInfo);
            assertEquals(0, vIfChangedCalled);
            assertNull(vIfChangedInfo);
            assertEquals(0, vlanMapChangedCalled);
            assertNull(vlanMapChangedInfo);
            assertEquals(0, portMapChangedCalled);
            assertNull(portMapChangedInfo);
        }
    };
}
