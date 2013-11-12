/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import static org.junit.Assert.*;

import java.io.File;

import org.junit.Test;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.FlowEventTestBase;

/**
 * JUnit test for {@link VTenantEvent}
 */
public class VTenantEventTest extends FlowEventTestBase {

    /**
     * Test case for getter methods and
     * {@link VNodeEvent#isSingleThreaded(boolean)} and
     * {@link VNodeEvent#isSaveConfig(boolean)}.
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
                            String emsg = "(VTenantPath)" + tpath.toString()
                                    + ",(desc)" + desc  + ",(iv)" + iv
                                    + ",(hv)" + hv;

                            VTenantEvent tevent
                                    = new VTenantEvent(tpath, vtenant, utype);

                            assertEquals(emsg, tpath, tevent.getPath());
                            assertEquals(emsg, "virtual tenant", tevent.getTypeName());
                            assertEquals(emsg, utype, tevent.getUpdateType());
                            assertEquals(emsg, vtenant, tevent.getObject());
                            assertEquals(emsg, vtenant, tevent.getVTenant());
                            assertTrue(emsg, tevent.isSaveConfig());

                            for (Boolean local : createBooleans(false)) {
                                assertTrue(emsg + "(local)" + local.toString(),
                                        tevent.isSingleThreaded(local.booleanValue()));
                            }
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
        String root = GlobalConstants.STARTUPHOME.toString();
        String tenantName = "tenant";
        String tenantListFileName = root + "vtn-default-tenant-names.conf";
        String configFileName =
            root + "vtn-" + "default" + "-" + tenantName + ".conf";
        File tenantList = new File(tenantListFileName);
        File configFile = new File(configFileName);

        tenantList.delete();
        configFile.delete();

        // register stub.
        VTNManagerAwareStub stub = new VTNManagerAwareStub();
        addVTNManagerAware(stub);
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
                assertTrue(tenantList.exists());
                assertFalse(configFile.exists());
            } else {
                // if local, files are not modified.
                assertTrue(tenantList.exists());
                assertTrue(configFile.exists());
            }
            tenantList.delete();

            tevent = new VTenantEvent(tpath, vtenant, UpdateType.ADDED);
            tevent.eventReceived(vtnMgr, local.booleanValue());
            flushTasks();

            stub.checkVtnInfo(1, tpath, UpdateType.ADDED);
            if (local == Boolean.FALSE) {
                assertNotNull(vtnMgr.getTenantFlowDB(tpath.getTenantName()));
                assertTrue(tenantList.exists());
                assertTrue(configFile.exists());
            } else {
                // if local, files are not modified.
                assertFalse(tenantList.exists());
                assertTrue(configFile.exists());
            }
            configFile.delete();

            tevent = new VTenantEvent(tpath, vtenant, UpdateType.CHANGED);
            tevent.eventReceived(vtnMgr, local.booleanValue());
            flushTasks();

            stub.checkVtnInfo(1, tpath, UpdateType.CHANGED);
            if (local == Boolean.FALSE) {
                assertNotNull(vtnMgr.getTenantFlowDB(tpath.getTenantName()));
                assertTrue(tenantList.exists());
                assertTrue(configFile.exists());
            } else {
                // if local, files are not modified.
                assertFalse(tenantList.exists());
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
                            VTenantEvent tevent = new VTenantEvent(tpath, vtenant, utype);

                            VTenantEvent newobj = (VTenantEvent) eventSerializeTest(tevent);

                            assertEquals(tpath, newobj.getPath());
                            assertEquals("virtual tenant", newobj.getTypeName());
                            assertEquals(utype, newobj.getUpdateType());
                            assertEquals(vtenant, newobj.getObject());
                            assertEquals(vtenant, newobj.getVTenant());
                            assertTrue(newobj.isSaveConfig());

                            for (Boolean local : createBooleans(false)) {
                                assertTrue(newobj.isSingleThreaded(local.booleanValue()));
                            }
                        }
                    }
                }
            }
        }
    }
}
