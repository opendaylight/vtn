/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import java.io.File;

import org.junit.Test;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.VNodeEventTestBase;

/**
 * JUnit test for {@link VTenantEvent}.
 */
public class VTenantEventTest extends VNodeEventTestBase {

    /**
     * Test case for getter methods and
     * {@link VNodeEvent#isSingleThreaded(boolean)} and
     * {@link VNodeEvent#isSaveConfig()}.
     */
    @Test
    public void testGetter() {
        for (String tname : createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String desc: createStrings("desc")) {
                for (Integer iv: createIntegers(-2, 5)) {
                    for (Integer hv: createIntegers(-2, 5)) {
                        VTenantConfig tconf
                            = createVTenantConfig(desc, iv, hv);
                        VTenant vtenant = new VTenant(tname, tconf);
                        for (UpdateType utype : UpdateType.values()) {
                            String emsg = "(VTenantPath)" + tpath.toString() +
                                ",(desc)" + desc  + ",(iv)" + iv +
                                ",(hv)" + hv;

                            VTenantEvent tevent =
                                new VTenantEvent(tpath, vtenant, utype);
                            checkVTenantEvent(tevent, tpath, tconf, utype, emsg);
                        }
                    }
                }
            }
        }
    }

    /**
     * Test method for
     * {@link VTenantEvent#eventReceived(VTNManagerImpl, boolean)},
     * {@link VTenantEvent#notifyEvent(VTNManagerImpl)}.
     */
    @Test
    public void testEventReceived() {
        File dir = getTenantConfigDir("default");
        String tenantName = "tenant";
        String configFileName = tenantName + ".conf";
        File configFile = new File(dir, configFileName);
        configFile.delete();

        // register stub.
        VTNManagerAwareStub stub = new VTNManagerAwareStub();
        addVTNManagerAware(stub);
        flushTasks();
        stub.checkAllNull();

        // create tenant.
        VTenantPath tpath = new VTenantPath(tenantName);
        Status st = vtnMgr.addTenant(tpath, new VTenantConfig("desc"));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        VTenant vtenant = null;
        try {
            vtenant = vtnMgr.getTenant(tpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        flushTasks();
        stub.checkVtnInfo(1, tpath, UpdateType.ADDED);

        // test evenReceved().
        for (Boolean local : createBooleans(false)) {
            VTenantEvent tevent = new VTenantEvent(tpath, vtenant, UpdateType.REMOVED);
            tevent.eventReceived(vtnMgr, local.booleanValue());
            flushTasks();

            stub.checkVtnInfo(1, tpath, UpdateType.REMOVED);
            if (local == Boolean.FALSE) {
                assertNull(vtnMgr.getTenantFlowDB(tpath.getTenantName()));
                assertFalse(configFile.exists());
            } else {
                // if local, files are not modified.
                assertTrue(configFile.exists());
            }

            tevent = new VTenantEvent(tpath, vtenant, UpdateType.ADDED);
            tevent.eventReceived(vtnMgr, local.booleanValue());
            flushTasks();

            stub.checkVtnInfo(1, tpath, UpdateType.ADDED);
            if (local == Boolean.FALSE) {
                assertNotNull(vtnMgr.getTenantFlowDB(tpath.getTenantName()));
                assertTrue(configFile.exists());
            } else {
                // if local, files are not modified.
                assertTrue(configFile.exists());
            }
            configFile.delete();

            tevent = new VTenantEvent(tpath, vtenant, UpdateType.CHANGED);
            tevent.eventReceived(vtnMgr, local.booleanValue());
            flushTasks();

            stub.checkVtnInfo(1, tpath, UpdateType.CHANGED);
            if (local == Boolean.FALSE) {
                assertNotNull(vtnMgr.getTenantFlowDB(tpath.getTenantName()));
                assertTrue(configFile.exists());
            } else {
                // if local, files are not modified.
                assertFalse(configFile.exists());
            }
        }
    }

    /**
     * Ensure that {@link VTenantEvent} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String tname : createStrings("tenant")) {
            VTenantPath tpath = new VTenantPath(tname);
            for (String desc: createStrings("desc")) {
                for (Integer iv: createIntegers(-2, 5)) {
                    for (Integer hv: createIntegers(-2, 5)) {
                        VTenantConfig tconf
                            = createVTenantConfig(desc, iv, hv);
                        VTenant vtenant = new VTenant(tname, tconf);
                        for (UpdateType utype : UpdateType.values()) {
                            String emsg = "(VTenantPath)" + tpath.toString()
                                    + ",(desc)" + desc  + ",(iv)" + iv
                                    + ",(hv)" + hv;

                            VTenantEvent tevent =
                                new VTenantEvent(tpath, vtenant, utype);
                            VTenantEvent newobj =
                                (VTenantEvent)eventSerializeTest(tevent);

                            checkVTenantEvent(newobj, tpath, tconf, utype, emsg);
                        }
                    }
                }
            }
        }
    }

    /**
     * Check {@link VTenantEvent} object.
     *
     * @param tev       A {@link VTenantEvent}.
     * @param tpath     A {@link VTenantPath}.
     * @param tconf     A {@link VTenantConfig}.
     * @param utype     An {@link UpdateType}.
     * @param emsg      Output error message.
     */
    private void checkVTenantEvent(VTenantEvent tev, VTenantPath tpath,
                                   VTenantConfig tconf, UpdateType utype,
                                   String emsg) {
        assertEquals(emsg, tpath, tev.getPath());
        VTenant tenant = new VTenant(tpath.getTenantName(), tconf);
        assertEquals(emsg, tenant, tev.getVTenant());
        assertEquals(emsg, tenant, tev.getObject());
        assertEquals(emsg, utype, tev.getUpdateType());
        assertEquals(emsg, "virtual tenant", tev.getTypeName());
        assertTrue(emsg, tev.isSaveConfig());

        for (Boolean local : createBooleans(false)) {
            assertTrue(emsg, tev.isSingleThreaded(local.booleanValue()));
        }
    }
}
