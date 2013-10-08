/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.ops4j.pax.exam.CoreOptions.junitBundles;
import static org.ops4j.pax.exam.CoreOptions.mavenBundle;
import static org.ops4j.pax.exam.CoreOptions.options;
import static org.ops4j.pax.exam.CoreOptions.systemPackages;
import static org.ops4j.pax.exam.CoreOptions.systemProperty;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Dictionary;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import javax.inject.Inject;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeState;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;

import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.IObjectReader;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.IContainerListener;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.IListenDataPacket;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.routing.IListenRoutingUpdates;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManagerAware;

import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.junit.Configuration;
import org.ops4j.pax.exam.junit.PaxExam;
import org.ops4j.pax.exam.util.PathUtils;
import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceRegistration;
import org.osgi.framework.Version;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

@RunWith(PaxExam.class)
public class VTNManagerIT extends TestBase {
    private static final Logger log = LoggerFactory.
        getLogger(VTNManagerIT.class);
    private static final String  BUNDLE_VTN_MANAGER_IMPL =
        "org.opendaylight.vtn.manager.implementation";

    // get the OSGI bundle context
    @Inject
    private BundleContext bc;

    private IVTNManager vtnManager = null;
    private IVTNGlobal vtnGlobal = null;
    private IObjectReader objReader = null;
    private ICacheUpdateAware<ClusterEventId, Object> cacheUpdateAware = null;
    private IConfigurationContainerAware  configContainerAware = null;
    private IInventoryListener inventoryListener = null;
    private ITopologyManagerAware  topologyManagerAware = null;
    private IContainerListener containerListener = null;
    private IListenDataPacket listenDataPacket = null;
    private IListenRoutingUpdates listenRoutingUpdates = null;
    private IHostFinder hostFinder = null;

    private Bundle  implBundle;

    // Configure the OSGi container
    @Configuration
    public Option[] config() {
        return options(
                //
                systemProperty("logback.configurationFile").value(
                        "file:" + PathUtils.getBaseDir() + "/src/test/resources/logback.xml"),
                // To start OSGi console for inspection remotely
                systemProperty("osgi.console").value("2401"),
                // Set the systemPackages (used by clustering)
                systemPackages("sun.reflect", "sun.reflect.misc", "sun.misc"),

                mavenBundle("org.slf4j", "jcl-over-slf4j").versionAsInProject(),
                mavenBundle("org.slf4j", "slf4j-api").versionAsInProject(),
                mavenBundle("org.slf4j", "log4j-over-slf4j").versionAsInProject(),
                mavenBundle("ch.qos.logback", "logback-core").versionAsInProject(),
                mavenBundle("ch.qos.logback", "logback-classic").versionAsInProject(),
                mavenBundle("org.apache.commons", "commons-lang3").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager").versionAsInProject(),

                // the plugin stub to get data for the tests
                mavenBundle("org.opendaylight.controller", "protocol_plugins.stub").versionAsInProject(),

                // List needed opendaylight modules
                mavenBundle("org.opendaylight.controller", "configuration").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "configuration.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "containermanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "containermanager.it.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "clustering.services").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "clustering.services-implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.connection").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.connection.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "connectionmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "connectionmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "switchmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "switchmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwardingrulesmanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwardingrulesmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "hosttracker").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "hosttracker.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "routing.dijkstra_implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "topologymanager").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "clustering.stub").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "forwarding.staticrouting").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "statisticsmanager").versionAsInProject(),

                // VTN Manager bundels
                mavenBundle("org.opendaylight.vtn", "manager").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.implementation").versionAsInProject(),

                mavenBundle("equinoxSDK381", "org.eclipse.equinox.ds").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.util").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.osgi.services").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.command").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.runtime").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.shell").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.cm").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.console").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.launcher").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager.shell").versionAsInProject(),

                mavenBundle("org.jboss.spec.javax.transaction", "jboss-transaction-api_1.1_spec").versionAsInProject(),
                mavenBundle("eclipselink", "javax.resource").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.fileinstall").versionAsInProject(),

                mavenBundle("org.ops4j.pax.exam", "pax-exam-container-native"),
                mavenBundle("org.ops4j.pax.exam", "pax-exam-junit4"),
                mavenBundle("org.ops4j.pax.exam", "pax-exam-link-mvn"),
                mavenBundle("org.ops4j.pax.url", "pax-url-aether"),

                mavenBundle("org.opendaylight.controller.thirdparty", "net.sf.jung2").versionAsInProject(),

                junitBundles());
    }

    private String stateToString(int state) {
        switch (state) {
        case Bundle.ACTIVE:
            return "ACTIVE";
        case Bundle.INSTALLED:
            return "INSTALLED";
        case Bundle.RESOLVED:
            return "RESOLVED";
        case Bundle.UNINSTALLED:
            return "UNINSTALLED";
        default:
            return "Not CONVERTED";
        }
    }

    @Before
    public void areWeReady() {
        assertNotNull(bc);
        boolean debugit = false;
        for (Bundle b: bc.getBundles()) {
            String name = b.getSymbolicName();
            int state = b.getState();
            if (state != Bundle.ACTIVE && state != Bundle.RESOLVED) {
                log.debug("Bundle:" + name + " state:" + stateToString(state));
                debugit = true;
            } else if (BUNDLE_VTN_MANAGER_IMPL.equals(name)) {
                implBundle = b;
            }
        }
        if (debugit) {
            log.debug("Do some debugging because some bundle is "
                    + "unresolved");
        }

        // Assert if true, if false we are good to go!
        assertFalse(debugit);

        ServiceReference r = bc.getServiceReference(IVTNManager.class.getName());
        if (r != null) {
            this.vtnManager = (IVTNManager) bc.getService(r);
            this.objReader = (IObjectReader) this.vtnManager ;
            this.cacheUpdateAware =
                (ICacheUpdateAware<ClusterEventId, Object>) this.vtnManager;
            this.configContainerAware = (IConfigurationContainerAware) this.vtnManager;
            this.inventoryListener = (IInventoryListener) this.vtnManager;
            this.topologyManagerAware = (ITopologyManagerAware) this.vtnManager;
            this.containerListener = (IContainerListener) this.vtnManager;
            this.listenDataPacket = (IListenDataPacket) this.vtnManager;
            this.listenRoutingUpdates = (IListenRoutingUpdates) this.vtnManager;
            this.hostFinder = (IHostFinder) this.vtnManager;
        }

        r = bc.getServiceReference(IVTNGlobal.class.getName());
        if (r != null) {
            this.vtnGlobal = (IVTNGlobal)bc.getService(r);
        }

        // If either IVTNManager or IVTNGlobal is null, cannot run tests.
        assertNotNull(this.vtnManager);
        assertNotNull(this.vtnGlobal);
    }

    @BeforeClass
    public static void before() {
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        boolean result = confdir.exists();
        if (!result) {
            result = confdir.mkdirs();
        }
    }

    @AfterClass
    public static void after() {
        String currdir = new File(".").getAbsoluteFile().getParent();
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());

        if (confdir.exists()) {
            File[] list = confdir.listFiles();
            for (File f : list) {
                f.delete();
            }

            while (confdir != null && confdir.getAbsolutePath() != currdir) {
                confdir.delete();
                String pname = confdir.getParent();
                if (pname == null) {
                    break;
                }
                confdir = new File(pname);
            }
        }
    }

    /**
     * test method for {@link IVTNManager}
     */
    @Test
    public void testIVTNManager() {
        testIVTNGlobal();
        testAddGetRemoveTenant();
        testModifyTenant();
        testBridge();
        testBridgeInterface();
        testVlanMap();
        testPortMap();
    }

    /**
     * Test method for
     * {@link IVTNManager#addTenant(VTenantPath, VTenantConfig)},
     * {@link IVTNManager#removeTenant(VTenantPath)},
     * {@link IVTNManager#getTenants()},
     * {@link IVTNManager#getTenant(VTenantPath)},
     * {@link IVTNManager#isActive()}
     */
    private void testAddGetRemoveTenant() {
        IVTNManager mgr = this.vtnManager;

        List<String> strings = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();
        List<Integer> ivs = new ArrayList<Integer>();
        List<Integer> hvs = new ArrayList<Integer>();
        strings.add(new String("tenant"));
        strings.add(new String("123456789012345678901234567890_"));
        descs.add(null);
        descs.add(new String("description."));
        ivs.add(new Integer(0));
        ivs.add(new Integer(65535));
        hvs.add(new Integer(0));
        hvs.add(new Integer(65535));

        assertFalse(mgr.isActive());

        // test for add
        for (String tname : strings) {
            if (tname.isEmpty()) {
                // empty is invalid for tenant name.
                continue;
            }
            VTenantPath tpath = new VTenantPath(tname);

            for (String desc : descs) {
                for (Integer iv : ivs) {
                    for (Integer hv : hvs) {
                        VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                        Status st = mgr.addTenant(tpath, tconf);
                        String emsg = "(name)" + tname + "(desc)"+ desc
                                + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                                + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                        if (iv != null && hv != null && iv.intValue() > 0 && hv.intValue() > 0
                                && iv.intValue() >= hv.intValue()) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else if ((iv == null || iv.intValue() < 0) && hv != null && hv.intValue() > 0
                                && 300 >= hv.intValue()) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else {
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                            assertTrue(emsg, mgr.isActive());
                        }

                        // getTenant()
                        VTenant tenant = null;
                        try {
                            tenant = mgr.getTenant(tpath);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(tname, tenant.getName());
                        assertEquals(tconf.getDescription(), tenant.getDescription());
                        if (iv == null || iv.intValue() < 0) {
                            assertEquals(emsg, 300, tenant.getIdleTimeout());
                        } else {
                            assertEquals(emsg, iv.intValue(), tenant.getIdleTimeout());
                        }
                        if (hv == null || hv.intValue() < 0) {
                            assertEquals(emsg, 0, tenant.getHardTimeout());
                        } else {
                            assertEquals(emsg, hv.intValue(), tenant.getHardTimeout());
                        }

                        // removeTenant()
                        mgr.removeTenant(tpath);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                    }
                }
            }
        }

        try {
            List<VTenant> list = mgr.getTenants();
            assertEquals(0, list.size());
        } catch (Exception e) {
            unexpected(e);
        }

        assertFalse(mgr.isActive());
    }

    /**
     * Test method for
     * {@link IVTNManager#modifyTenant(VTenantPath, VTenantConfig, boolean)}
     */
    private void testModifyTenant() {
        IVTNManager mgr = this.vtnManager;
        List<String> tnames = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();
        List<Integer> ivs = new ArrayList<Integer>();
        List<Integer> hvs = new ArrayList<Integer>();

        tnames.add(new String("vtn"));
        tnames.add(new String("123456789012345678901234567890_"));
        descs.add(null);
        descs.add(new String("desc"));
        ivs.add(new Integer(0));
        ivs.add(new Integer(65535));
        hvs.add(new Integer(0));
        hvs.add(new Integer(65535));

        boolean first = true;
        for (String tname : tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            VTenantConfig tconf = createVTenantConfig(new String("orig"), 20, 30);
            Status st = mgr.addTenant(tpath, tconf);

            for (String desc : descs) {
                for (Integer orgiv : ivs) {
                    for (Integer orghv : hvs) {
                        // check parameters
                        if (orgiv != null && orghv != null && orgiv.intValue() > 0 && orghv.intValue() > 0
                                && orgiv.intValue() >= orghv.intValue()) {
                            continue;
                        } else if ((orgiv == null || orgiv.intValue() < 0) && orghv != null && orghv.intValue() > 0
                                && 300 >= orghv.intValue()) {
                            continue;
                        }

                        String olddesc;
                        Integer oldiv;
                        Integer oldhv;
                        for (Integer iv : ivs) {
                            for (Integer hv : hvs) {
                                VTenant tenant = null;

                                if (first) {
                                    for (String ndesc : descs) {
                                        // test for all == true. executed at
                                        // first time only.
                                        String emsg = "(name)" + tname + "(ndesc)"+ ndesc
                                                + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                                                + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                                        tconf = createVTenantConfig(ndesc, iv, hv);
                                        st = mgr.modifyTenant(tpath, tconf, true);

                                        if (iv != null && hv != null
                                                && iv.intValue() > 0 && hv.intValue() > 0
                                                && iv.intValue() >= hv.intValue()) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else if ((iv == null || iv < 0)
                                                && hv != null && hv > 0 && 300 >= hv) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }

                                        try {
                                            tenant = mgr.getTenant(tpath);
                                        } catch (Exception e) {
                                            unexpected(e);
                                        }

                                        if (st.isSuccess()) {
                                            assertEquals(emsg, tname, tenant.getName());
                                            assertEquals(emsg,
                                                    ndesc, tenant.getDescription());
                                            if (iv == null || iv.intValue() < 0) {
                                                assertEquals(emsg,
                                                        300, tenant.getIdleTimeout());
                                            } else {
                                                assertEquals(emsg,
                                                        iv.intValue(),
                                                        tenant.getIdleTimeout());
                                            }
                                            if (hv == null || hv.intValue() < 0) {
                                                assertEquals(emsg,
                                                        0, tenant.getHardTimeout());
                                            } else {
                                                assertEquals(emsg,
                                                        hv.intValue(),
                                                        tenant.getHardTimeout());
                                            }
                                        }
                                    }
                                }

                                VTenantConfig tconfOrg = createVTenantConfig(desc,
                                        orgiv, orghv);
                                st = mgr.modifyTenant(tpath, tconfOrg, true);

                                olddesc = (desc == null) ? null : new String(desc);
                                oldiv = (orgiv == null || orgiv.intValue() < 0) ? new Integer(300) : orgiv;
                                oldhv = (orghv == null || orghv.intValue() < 0) ? new Integer(0) : orghv;

                                // all == false
                                tconf = createVTenantConfig(desc, iv, hv);
                                st = mgr.modifyTenant(tpath, tconf, false);

                                String emsg = "(VTenangConfig(orig))"  + tconfOrg.toString()
                                        + "(name)" + tname + "(ndesc)"+ desc
                                        + ",(iv)" + ((iv == null) ? "null" : iv.intValue())
                                        + ",(hv)" + ((hv == null) ? "null" : hv.intValue());

                                if ((iv == null || iv.intValue() < 0) && (hv == null || hv.intValue() < 0)) {
                                    // both are notset
                                    // not changed.

                                } else if (iv == null || iv.intValue() < 0) {
                                    // idle_timeout is notset
                                    if (hv.intValue() > 0
                                            && (oldiv.intValue() != 0 && oldiv.intValue() >= hv.intValue())) {
                                        assertEquals(emsg,
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                } else if (hv == null || hv.intValue() < 0) {
                                    // hard_timeout is notset
                                    if (iv > 0 && oldhv > 0) {
                                        if (iv >= oldhv) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                } else {
                                    // both are set
                                    if (iv.intValue() > 0 && hv.intValue() > 0) {
                                        if (iv.intValue() >= hv.intValue()) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                }

                                tenant = null;
                                try {
                                    tenant = mgr.getTenant(tpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }

                                if (st.isSuccess()) {
                                    assertEquals(emsg, tname, tenant.getName());
                                    assertEquals(emsg, olddesc, tenant.getDescription());
                                    if (iv == null || iv.intValue() < 0) {
                                        assertEquals(emsg,
                                                oldiv.intValue(), tenant.getIdleTimeout());
                                    } else {
                                        assertEquals(emsg,
                                                iv.intValue(), tenant.getIdleTimeout());
                                    }
                                    if (hv == null || hv.intValue() < 0) {
                                        assertEquals(emsg,
                                                oldhv.intValue(), tenant.getHardTimeout());
                                    } else {
                                        assertEquals(emsg,
                                                hv.intValue(), tenant.getHardTimeout());
                                    }
                                }

                                olddesc = tenant.getDescription();
                                oldiv = tenant.getIdleTimeout();
                                oldhv = tenant.getHardTimeout();
                            }
                        }
                        first = false;
                    }
                }
            }
        }

        try {
            List<VTenant> list = mgr.getTenants();
            assertEquals(tnames.size(), list.size());
        } catch (Exception e) {
            unexpected(e);
        }

        for (String tname : tnames) {
            VTenantPath tpath = new VTenantPath(tname);
            mgr.removeTenant(tpath);
        }
    }

    /**
     * Test method for
     * {@link IVTNManager#addBridge(VBridgePath, VBridgeConfig)},
     * {@link IVTNManager#removeBridge(VBridgePath)},
     * {@link IVTNManager#modifyBridge(VBridgePath, VBridgeConfig, boolean)},
     * {@link IVTNManager#getBridges(VTenantPath)},
     * {@link IVTNManager#getBridge(VBridgePath)}.
     */
    private void testBridge() {
        IVTNManager mgr = this.vtnManager;
        List<Integer> ages = new ArrayList<Integer>();
        List<String> tlist = new ArrayList<String>();
        List<String> blist = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();

        tlist.add("vtn");
        tlist.add("123456789012345678901234567890_");
        blist.add("vbr");
        blist.add("012345678901234567890123456789_");
        ages.add(null);
        ages.add(10);
        ages.add(600);
        ages.add(1000000);
        descs.add(null);
        descs.add("description...");

        boolean first = true;
        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertEquals(StatusCode.SUCCESS, st.getCode());

            for (String bname : blist) {
                if (bname.isEmpty()) {
                    continue; // This is a invalid condition.
                }
                VBridgePath bpath = new VBridgePath(tname, bname);
                for (String desc : descs) {
                    for (Integer age : ages) {
                        VBridgeConfig bconf = createVBridgeConfig(desc, age);
                        String emsg = "(VBridgePath)" + bpath.toString()
                                + "(VBridgeConfig)" + bconf.toString()
                                + "(age)" + ((age == null) ? "null" : age.intValue());

                        st = mgr.addBridge(bpath, bconf);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                        VBridge brdg = null;
                        try {
                            brdg = mgr.getBridge(bpath);
                        } catch (Exception e) {
                            unexpected(e);
                        }
                        assertEquals(emsg, bname, brdg.getName());
                        assertEquals(emsg, desc, brdg.getDescription());
                        if (age == null) {
                            assertEquals(emsg, 600, brdg.getAgeInterval());
                        } else {
                            assertEquals(emsg, age.intValue(), brdg.getAgeInterval());
                        }
                        assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                        String olddesc = brdg.getDescription();
                        int oldage = brdg.getAgeInterval();

                        for (String newdesc : descs) {
                            for (Integer newage : ages) {
                                bconf = createVBridgeConfig(newdesc, newage);
                                st = mgr.modifyBridge(bpath, bconf, false);

                                String emsgMod = emsg
                                        + "(VBridgeConfig(new))" + bconf.toString()
                                        + "(age(new))"
                                        + ((newage == null) ? "null" : newage.intValue());

                                brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(emsgMod, bname, brdg.getName());
                                if (newdesc == null) {
                                    assertEquals(emsgMod,
                                            olddesc, brdg.getDescription());
                                } else {
                                    assertEquals(emsgMod,
                                            newdesc, brdg.getDescription());
                                }
                                if (newage == null) {
                                    assertEquals(emsgMod,
                                            oldage, brdg.getAgeInterval());
                                } else {
                                    assertEquals(emsgMod,
                                            newage.intValue(), brdg.getAgeInterval());
                                }
                                olddesc = brdg.getDescription();
                                oldage = brdg.getAgeInterval();

                                assertEquals(emsgMod,
                                        VNodeState.UNKNOWN, brdg.getState());
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                    }

                    if (first) {
                        VBridgeConfig bconf = new VBridgeConfig("desc", 10);
                        st = mgr.addBridge(bpath, bconf);
                        for (String newdesc : descs) {
                            for (Integer newage : ages) {
                                bconf = createVBridgeConfig(newdesc, newage);
                                st = mgr.modifyBridge(bpath, bconf, true);

                                String emsgMod =
                                        "(VBridgeConfig(new))" + bconf.toString()
                                        + "(age(new))"
                                        + ((newage == null) ? "null" : newage.intValue());

                                VBridge brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(emsgMod, bname, brdg.getName());
                                assertEquals(emsgMod, newdesc, brdg.getDescription());
                                if (newage == null) {
                                    assertEquals(emsgMod, 600, brdg.getAgeInterval());
                                } else {
                                    assertEquals(emsgMod,
                                            newage.intValue(), brdg.getAgeInterval());
                                }
                                assertEquals(emsgMod,
                                        VNodeState.UNKNOWN, brdg.getState());
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        assertEquals("(VBridgePath)" + bpath.toString(),
                                StatusCode.SUCCESS, st.getCode());

                        first = false;
                    }
                }
            }

            try {
                List<VBridge> list = mgr.getBridges(tpath);
                assertEquals(0, list.size());
            } catch (VTNException e) {
                unexpected(e);
            }
            st = mgr.removeTenant(tpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // add mulitple entry.
        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());

            for (String bname : blist) {
                if (bname.isEmpty()) {
                    continue; // This is a invalid condition.
                }
                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);

                st = mgr.addBridge(bpath, bconf);
                assertEquals("(VBridgePath)" + bpath.toString()
                        + "(VBridgeConfig)" + bconf.toString(),
                        StatusCode.SUCCESS, st.getCode());
            }
            try {
                List<VBridge> list = mgr.getBridges(tpath);
                assertEquals("(VTenantPath)" + tpath.toString(),
                        blist.size(), list.size());
            } catch (VTNException e) {
                unexpected(e);
            }
            st = mgr.removeTenant(tpath);
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * Test method for
     * {@link IVTNManager#addBridgeInterface(VBridgeIfPath, VInterfaceConfig)},
     * {@link IVTNManager#modifyBridgeInterface(VBridgeIfPath, VInterfaceConfig, boolean)},
     * {@link IVTNManager#removeBridgeInterface(VBridgeIfPath)},
     * {@link IVTNManager#getBridgeInterfaces(VBridgePath)},
     * {@link IVTNManager#getBridgeInterface(VBridgeIfPath)}.
     */
    private void testBridgeInterface() {
        IVTNManager mgr = this.vtnManager;
        List<String> tlist = new ArrayList<String>();
        List<String> blist = new ArrayList<String>();
        List<String> iflist = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();

        tlist.add("vtn");
        // tlist.add("123456789012345678901234567890_");
        blist.add("vbr");
        // blist.add("012345678901234567890123456789_");
        iflist.add("vinterface");
        iflist.add("abcdefghijklmnoopqrstuvwxyz1234");
        descs.add(null);
        descs.add("description");

        for (String tname : tlist) {
            VTenantPath tpath = new VTenantPath(tname);
            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            assertEquals("(VTenantPath)" + tpath.toString(),
                    StatusCode.SUCCESS, st.getCode());

            for (String bname : blist) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);
                st = mgr.addBridge(bpath, bconf);
                assertEquals("(VBridgePath)" + bpath.toString(),
                        StatusCode.SUCCESS, st.getCode());

                List<VInterface> list = null;
                try {
                    list = mgr.getBridgeInterfaces(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals("(VBridgePath)" + bpath.toString(),
                        0, list.size());

                for (String ifname : iflist) {
                    if (ifname.isEmpty()) {
                        continue; // This is a invalid condition.
                    }

                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + ",(VInterfaceConfig)" + ifconf.toString();

                            st = mgr.addBridgeInterface(ifp, ifconf);
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(emsg, ifname, vif.getName());
                            assertEquals(emsg, desc, vif.getDescription());
                            if (enabled == null) {
                                assertEquals(emsg, Boolean.TRUE, vif.getEnabled());
                            } else {
                                assertEquals(emsg, enabled, vif.getEnabled());
                            }
                            if (enabled == Boolean.FALSE) {
                                assertEquals(emsg, VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VNodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                            st = mgr.removeBridgeInterface(ifp);
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                        }
                    }

                    // for modify(false)
                    st = mgr.addBridgeInterface(ifp, new VInterfaceConfig("desc", Boolean.FALSE));
                    assertEquals(StatusCode.SUCCESS, st.getCode());

                    String olddesc = new String("desc");
                    Boolean oldenabled = Boolean.FALSE;
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + "(VInterfaceConfig)" + ifconf.toString();

                            st = mgr.modifyBridgeInterface(ifp, ifconf, false);
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(emsg, ifname, vif.getName());

                            if (desc == null) {
                                assertEquals(emsg, olddesc, vif.getDescription());
                            } else {
                                assertEquals(emsg, desc, vif.getDescription());
                            }

                            Boolean currenabled = enabled;
                            if (enabled == null) {
                                assertEquals(emsg, oldenabled, vif.getEnabled());
                                currenabled = oldenabled;
                            } else {
                                assertEquals(enabled, vif.getEnabled());
                            }

                            if (currenabled == Boolean.FALSE) {
                                assertEquals(emsg, VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VNodeState.UNKNOWN, vif.getState());
                            }
                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                            olddesc = vif.getDescription();
                            oldenabled = vif.getEnabled();
                        }
                    }
                    // for modify(true)
                    st = mgr.modifyBridgeInterface(ifp, new VInterfaceConfig("desc", Boolean.FALSE), true);
                    assertEquals(StatusCode.SUCCESS, st.getCode());
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + "(VInterfaceConfig)" + ifconf.toString();

                            st = mgr.modifyBridgeInterface(ifp, ifconf, true);
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                            VInterface vif = null;
                            try {
                                vif = mgr.getBridgeInterface(ifp);
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            assertEquals(emsg, ifname, vif.getName());
                            assertEquals(emsg, desc, vif.getDescription());
                            if (enabled == null) {
                                assertEquals(emsg, Boolean.TRUE, vif.getEnabled());
                            } else {
                                assertEquals(emsg, enabled, vif.getEnabled());
                            }

                            if (enabled == Boolean.FALSE) {
                                assertEquals(emsg, VNodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VNodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                               unexpected(e);
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());
                        }
                    }
                }

                list = null;
                try {
                    list = mgr.getBridgeInterfaces(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(iflist.size(), list.size());
            }

            st = mgr.removeTenant(tpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // TODO: NOTACCEPTABLE
    }

    /**
     * Test method for
     * {@link IVTNManager#addVlanMap(VBridgePath, VlanMapConfig)},
     * {@link IVTNManager#removeVlanMap(VBridgePath, java.lang.String)},
     * {@link IVTNManager#getVlanMap(VBridgePath, java.lang.String)},
     * {@link IVTNManager#getVlanMaps(VBridgePath)}.
     */
    private void testVlanMap() {
        IVTNManager mgr = this.vtnManager;
        short[] vlans = new short[] { 0, 10, 4095 };

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeConfig bconf = createVBridgeConfig(null, null);
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig(null, Boolean.TRUE);
        st = mgr.addBridgeInterface(ifp, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // add a vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String emsg = "(VlanMapConfig)" + vlconf.toString();

                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null && node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                        continue;
                    }
                }

                VlanMap getmap = null;
                try {
                    getmap = mgr.getVlanMap(bpath, map.getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, getmap.getId(), map.getId());
                assertEquals(emsg, getmap.getNode(), node);
                assertEquals(emsg, getmap.getVlan(), vlan);

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(emsg,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN, brdg.getState());

                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }

        // add multi vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null && node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                        continue;
                    }
                } catch (Exception e) {
                    unexpected(e);
                }
            }

            List<VlanMap> list = null;
            try {
                list = mgr.getVlanMaps(bpath);
            } catch (Exception e) {
                unexpected(e);
            }
            String emsg = "(Node)" + ((node == null) ? "null" : node.toString());
            if (node == null || node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                assertEquals(emsg,
                        vlans.length, list.size());
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(emsg,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN, brdg.getState());
            } else {
                assertEquals(0, list.size());
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());
            }

            for (VlanMap map : list) {
                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg + ",(VlanMap)"+ map.toString(),
                        StatusCode.SUCCESS, st.getCode());
            }
        }

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test method for {@link IVTNManager#getPortMap(VBridgeIfPath)} and
     * {@link IVTNManager#setPortMap(VBridgeIfPath, PortMapConfig)}.
     */
    private void testPortMap() {
        IVTNManager mgr = this.vtnManager;
        short[] vlans = new short[] { 0, 10, 4095 };

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        VBridgeConfig bconf = createVBridgeConfig(null, null);
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig(null, Boolean.TRUE);
        st = mgr.addBridgeInterface(ifp, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        PortMap pmap = null;
        try {
            pmap = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(pmap);

        Node node = NodeCreator.createOFNode(0L);
        SwitchPort[] ports = new SwitchPort[] {
                new SwitchPort("port-10", NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
                new SwitchPort(null, NodeConnector.NodeConnectorIDType.OPENFLOW, "11"),
                new SwitchPort("port-10", null, null),
                new SwitchPort("port-10"),
                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "13"),
        };

        for (SwitchPort port: ports) {
            for (short vlan : vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port, (short)vlan);
                String emsg = "(PortMapConfig)" + pmconf.toString();

                st = mgr.setPortMap(ifp, pmconf);
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                PortMap map = null;
                try {
                    map = mgr.getPortMap(ifp);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(emsg, pmconf, map.getConfig());
                assertNull(emsg, map.getNodeConnector());

                VInterface bif = null;
                try {
                    bif = mgr.getBridgeInterface(ifp);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, VNodeState.DOWN, bif.getState());

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                   unexpected(e);
                }
                assertEquals(emsg, VNodeState.DOWN, brdg.getState());
            }
        }

        st = mgr.setPortMap(ifp, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        PortMap map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(map);

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // assign mutli portmaps to a vbridge
        st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        String bname1 = "vbridge1";
        st = mgr.addBridge(new VBridgePath(tname, bname1), new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        String bname2 = "vbridge2";
        st = mgr.addBridge(new VBridgePath(tname, bname2), new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String ifname1 = "vinterface1";
        VBridgeIfPath ifp1 = new VBridgeIfPath(tname, bname1, ifname1);
        st = mgr.addBridgeInterface(ifp1, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String ifname2 = "vinterface2";
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname1, ifname2);
        st = mgr.addBridgeInterface(ifp2, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        Node node1 = NodeCreator.createOFNode(0L);
        SwitchPort port1 = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf1 = new PortMapConfig(node1, port1, (short) 0);
        st = mgr.setPortMap(ifp1, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        SwitchPort port2 = new SwitchPort( NodeConnector.NodeConnectorIDType.OPENFLOW, "11");
        PortMapConfig pmconf2 = new PortMapConfig(node1, port2, (short) 0);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // if specified port is not exist, duplicate portmap success.
        String ifname3 = "vinterface3";
        VBridgeIfPath ifp3 = new VBridgeIfPath(tname, bname2, ifname3);
        st = mgr.addBridgeInterface(ifp3, new VInterfaceConfig(null, Boolean.TRUE));
        st = mgr.setPortMap(ifp3, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test case for {@link IVTNGlobal}.
     */
    private void testIVTNGlobal() {
        int api = vtnGlobal.getApiVersion();
        assertTrue("API version = " + api, api > 0);

        BundleVersion bv = vtnGlobal.getBundleVersion();
        assertNotNull(bv);

        // Determine actual bundle version of manager.implementation.
        assertNotNull(implBundle);
        Version ver = implBundle.getVersion();
        assertNotNull(ver);

        assertEquals(ver.getMajor(), bv.getMajor());
        assertEquals(ver.getMinor(), bv.getMinor());
        assertEquals(ver.getMicro(), bv.getMicro());

        String qf = ver.getQualifier();
        if (qf == null || qf.isEmpty()) {
            assertNull(bv.getQualifier());
        } else {
            assertEquals(qf, bv.getQualifier());
        }
    }

    /**
     * stub class for test of {@link IVTNManagerAware}
     */
    private class VTNManagerAwareData<T, S> {
        T path = null;
        S obj = null;
        UpdateType type = null;

        VTNManagerAwareData(T p, S o, UpdateType t) {
            path = p;
            obj = o;
            type = t;
        }
    };

    private class Update {
        VTNManagerAwareData<VTenantPath, VTenant> vtnChangedInfo = null;
        VTNManagerAwareData<VBridgePath, VBridge> vbrChangedInfo = null;
        VTNManagerAwareData<VBridgeIfPath, VInterface> vIfChangedInfo = null;
        VTNManagerAwareData<VBridgePath, VlanMap> vlanMapChangedInfo = null;
        VTNManagerAwareData<VBridgeIfPath, PortMap> portMapChangedInfo = null;

        Update (UpdateType t, VTenantPath path, VTenant vtenant) {
            vtnChangedInfo = new VTNManagerAwareData<VTenantPath, VTenant>(path, vtenant, t);
        }

        Update (UpdateType t, VBridgePath path, VBridge vbridge) {
            vbrChangedInfo = new VTNManagerAwareData<VBridgePath, VBridge>(path, vbridge, t);
        }

        Update (UpdateType t, VBridgeIfPath path, VInterface iface) {
            vIfChangedInfo = new VTNManagerAwareData<VBridgeIfPath, VInterface>(path, iface, t);
        }

        Update (UpdateType t, VBridgePath path, VlanMap vlmap) {
            vlanMapChangedInfo = new VTNManagerAwareData<VBridgePath, VlanMap>(path, vlmap, t);
        }

        Update (UpdateType t, VBridgeIfPath path, PortMap pmap) {
            portMapChangedInfo = new VTNManagerAwareData<VBridgeIfPath, PortMap>(path, pmap, t);
        }
    }

    private class VTNManagerAware implements IVTNManagerAware {
        private CopyOnWriteArrayList<Update> gotUpdates;
        private CountDownLatch latch = null;

        VTNManagerAware() {
            this.gotUpdates = new CopyOnWriteArrayList<Update>();
        }

        /**
         * Restart the monitor of the updates on the VTNManagerAware object
         *
         * @param expectedOperations Number of expected updates
         *
         * @return a countdown latch wich will be used to wait till the updates are done
         */
        CountDownLatch restart(int expectedOperations) {
            this.gotUpdates.clear();
            this.latch = new CountDownLatch(expectedOperations);
            return this.latch;
        }

        List<Update> getUpdates() {
//            sleep(10);
            return this.gotUpdates;
        }

        @Override
        public void vtnChanged(VTenantPath path, VTenant vtenant, UpdateType type) {
            log.debug("VTNManager[{}] Got an vtn changed for path:{} object:{}", path, vtenant);
            Update u = new Update(type, path, vtenant);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vBridgeChanged(VBridgePath path, VBridge vbridge, UpdateType type) {
            log.debug("VTNManager[{}] Got an vbridge changed for path:{} object:{}", path, vbridge);
            Update u = new Update(type, path, vbridge);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vBridgeInterfaceChanged(VBridgeIfPath path, VInterface viface, UpdateType type) {
            log.debug("VTNManager[{}] Got an vbridge interface changed for path:{} object:{}", path, viface);
            Update u = new Update(type, path, viface);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vlanMapChanged(VBridgePath path, VlanMap vlmap, UpdateType type) {
            log.debug("VTNManager[{}] Got an vlan map changed for path:{} object:{}", path, vlmap);
            Update u = new Update(type, path, vlmap);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void portMapChanged(VBridgeIfPath path, PortMap pmap, UpdateType type) {
            log.debug("VTNManager[{}] Got an port map changed for path:{} object:{}", path, pmap);
            Update u = new Update(type, path, pmap);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }
    }

    /**
     * Test method for
     * {@link IVTNManagerAware#vtnChanged(VTenantPath, VTenant, UpdateType)}
     * {@link IVTNManagerAware#vBridgeChanged(VBridgePath, VBridge, UpdateType)}
     * {@link IVTNManagerAware#vBridgeInterfaceChanged(VBridgeIfPath, VInterface, UpdateType)}
     * {@link IVTNManagerAware#vlanMapChanged(VBridgePath, VlanMap, UpdateType)}
     * {@link IVTNManagerAware#portMapChanged(VBridgeIfPath, PortMap, UpdateType)}
     * @throws InterruptedException
     * @throws VTNException
     */
    @Test
    public void testIVTNManagerAware() throws InterruptedException, VTNException {
        IVTNManager mgr = this.vtnManager;

        String tname1 = "tenant1";
        String tname2 = "tenant2";
        VTenantPath tpath1 = new VTenantPath(tname1);
        VTenantPath tpath2 = new VTenantPath(tname2);
        Status st = mgr.addTenant(tpath1, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.addTenant(tpath2, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        Dictionary<String, Object> props = new Hashtable<String, Object>();
        Set<String> propSet = new HashSet<String>();
        propSet.add(tname1);
        propSet.add(tname2);
        props.put("vtnnames", propSet);
        VTNManagerAware listener = new VTNManagerAware();
        VTNManagerAware listenerRepeated  = new VTNManagerAware();

        // Start monitoring the updates
        CountDownLatch res = null;
        List<Update> ups = null;
        Update up = null;
        res = listener.restart(2);

        ServiceRegistration updateServiceReg =
                ServiceHelper.registerServiceWReg(IVTNManagerAware.class, "default",
                                                    listener, props);
        assertNotNull(updateServiceReg);

        ServiceRegistration updateServiceRegRepeated =
                ServiceHelper.registerServiceWReg(IVTNManagerAware.class, "default",
                                                    listenerRepeated, props);
        assertNotNull(updateServiceRegRepeated);
        res.await(10L, TimeUnit.SECONDS);

        res = listener.restart(1);

        // add a tenant
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vtnChangedInfo.type.equals(UpdateType.ADDED));
        assertTrue(up.vtnChangedInfo.path.equals(tpath));
        assertTrue(up.vtnChangedInfo.obj.getName().equals(tname));

        // add a vbridge
        res = listener.restart(1);
        String bname = "bname";
        VBridgePath bpath = new VBridgePath(tname1, bname);
        st = mgr.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.ADDED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // add a vInterface
        res = listener.restart(1);
        String ifname = "vif";
        VBridgeIfPath ifpath = new VBridgeIfPath(tname1, bname, ifname);
        VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
        st = mgr.addBridgeInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vIfChangedInfo.type.equals(UpdateType.ADDED));
        assertTrue(up.vIfChangedInfo.path.equals(ifpath));
        assertTrue(up.vIfChangedInfo.obj.getName().equals(ifname));

        // set a PortMap
        res = listener.restart(3);
        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
        String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(3, ups.size());

        up = ups.get(0);
        assertTrue(up.portMapChangedInfo.type.equals(UpdateType.ADDED));
        assertTrue(up.portMapChangedInfo.path.equals(ifpath));
        assertTrue(up.portMapChangedInfo.obj.getConfig().equals(pmconf));

        up = ups.get(1);
        assertTrue(up.vIfChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vIfChangedInfo.path.equals(ifpath));
        assertTrue(up.vIfChangedInfo.obj.getName().equals(ifname));

        up = ups.get(2);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // set a VLanMap
        res = listener.restart(1);
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)4095);
        VlanMap map = null;
        try {
            map = mgr.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vlanMapChangedInfo.type.equals(UpdateType.ADDED));
        assertTrue(up.vlanMapChangedInfo.path.equals(bpath));
        assertTrue(up.vlanMapChangedInfo.obj.equals(map));

        // modify a tenant setting
        res = listener.restart(1);
        st = mgr.modifyTenant(tpath, new VTenantConfig("desc"), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vtnChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vtnChangedInfo.path.equals(tpath));
        assertTrue(up.vtnChangedInfo.obj.getName().equals(tname));

        // modify a vbridge setting
        res = listener.restart(1);
        st = mgr.modifyBridge(bpath, new VBridgeConfig("desc"), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // modify a vbridge interface setting
        res = listener.restart(1);
        st = mgr.modifyBridgeInterface(ifpath,
                new VInterfaceConfig("interface", Boolean.TRUE), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vIfChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vIfChangedInfo.path.equals(ifpath));
        assertTrue(up.vIfChangedInfo.obj.getName().equals(ifname));

        // change a PortMap setting
        res = listener.restart(1);
        port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, String.valueOf(11));
        pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.portMapChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.portMapChangedInfo.path.equals(ifpath));
        assertTrue(up.portMapChangedInfo.obj.getConfig().equals(pmconf));

        //remove a VLanMap
        res = listener.restart(1);
        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vlanMapChangedInfo.type.equals(UpdateType.REMOVED));
        assertTrue(up.vlanMapChangedInfo.path.equals(bpath));
        assertTrue(up.vlanMapChangedInfo.obj.equals(map));

        // remove a portmap
        res = listener.restart(3);
        st = mgr.setPortMap(ifpath, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(3, ups.size());

        up = ups.get(0);
        assertTrue(up.portMapChangedInfo.type.equals(UpdateType.REMOVED));
        assertTrue(up.portMapChangedInfo.path.equals(ifpath));
        assertTrue(up.portMapChangedInfo.obj.getConfig().equals(pmconf));

        up = ups.get(1);
        assertTrue(up.vIfChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vIfChangedInfo.path.equals(ifpath));
        assertTrue(up.vIfChangedInfo.obj.getName().equals(ifname));

        up = ups.get(2);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // remove a vbridge interface
        res = listener.restart(1);
        st = mgr.removeBridgeInterface(ifpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vIfChangedInfo.type.equals(UpdateType.REMOVED));
        assertTrue(up.vIfChangedInfo.path.equals(ifpath));
        assertTrue(up.vIfChangedInfo.obj.getName().equals(ifname));

        // remove a vbridge
        res = listener.restart(1);
        st = mgr.removeBridge(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.REMOVED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // remove a tenant
        res = listener.restart(1);
        st =mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(10L, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vtnChangedInfo.type.equals(UpdateType.REMOVED));
        assertTrue(up.vtnChangedInfo.path.equals(tpath));
        assertTrue(up.vtnChangedInfo.obj.getName().equals(tname));

        // ensure no updates comes
        updateServiceReg.unregister();

        res = listener.restart(1);

        mgr.addTenant(tpath, new VTenantConfig(null));
        mgr.addBridge(bpath, null);
        mgr.modifyBridge(bpath, new VBridgeConfig(null), false);
        mgr.removeTenant(tpath);

        res.await(1L, TimeUnit.SECONDS);
        ups = listener.getUpdates();
        assertEquals(0, ups.size());
    }


    /**
     * Test method for
     * {@link VTNManager#getMacEntries(VBridgePath)},
     * {@link VTNManager#getMacEntries(VBridgePath)},
     * {@link VTNManager#getMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManager#removeMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManager#flushMacEntries(VBridgePath)}
     */
    @Test
    public void testMacEntry() {
        IVTNManager mgr = this.vtnManager;
        VNodeState[] stateValues = VNodeState.values();
        VNodeState[] states = new VNodeState[stateValues.length + 1];
        System.arraycopy(stateValues, 0, states, 1, stateValues.length);
        states[0] = null;

        int flt1 = 1;
        int flt2 = 2;

        VBridgeConfig bconf = new VBridgeConfig(null);
        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String bname = "vbridge";
        VBridge bridge1 = new VBridge(bname, states[1], flt1, bconf);
        VBridgePath bpath = new VBridgePath(tname, bname);
        st = mgr.addBridge(bpath, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String bname2 = "vbridge2";
        VBridge bridge2 = new VBridge(bname2, states[2], flt2, bconf);
        VBridgePath bpath2 = new VBridgePath(tname, bname2);
        st = mgr.addBridge(bpath2, bconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VlanMapConfig vlconf = new VlanMapConfig(null, (short)0);
        VlanMap map = null;
        try {
            map = vtnManager.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }

        List<EthernetAddress> ethers = createEthernetAddresses(false);
        List<NodeConnector> connectors = createNodeConnectors(3, false);
        NodeConnector nc = connectors.get(0);


        byte iphost = 1;

        for (EthernetAddress ea: ethers) {
            byte[] bytes = ea.getValue();
            byte[] src = new byte[] {00, bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
            byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF, (byte)0xFF};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
            byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

            EthernetAddress eaSrc = null;
            try {
                 eaSrc = new EthernetAddress(src);
            } catch (ConstructionException e) {
                unexpected(e);
            }

            RawPacket inPkt = createARPRawPacket (src, dst, sender, target, (short)-1, nc, ARP.REQUEST);
            listenDataPacket.receiveDataPacket(inPkt);

            String emsg = ea.toString();
            MacAddressEntry entry = null;
            List<MacAddressEntry> elist = null;
            try {
                entry = mgr.getMacEntry(bpath, eaSrc);
                assertNotNull(emsg, entry);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertEquals(emsg, eaSrc, entry.getAddress());
            assertEquals(emsg, nc, entry.getNodeConnector());

            Set<InetAddress> ips = entry.getInetAddresses();
            assertArrayEquals(emsg, sender, ips.iterator().next().getAddress());

            try {
                entry = mgr.getMacEntry(bpath2, eaSrc);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertNull(emsg, entry);

            try {
                assertNull(mgr.removeMacEntry(bpath, ea));
            } catch (VTNException e) {
                unexpected(e);
            }

            iphost++;
        }

        List<MacAddressEntry> list = null;
        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(ethers.size(), list.size());

        st = mgr.flushMacEntries(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(0, list.size());

        try {
            list = mgr.getMacEntries(bpath2);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
//        assertTrue(list.size() == ethers.size());

        st = mgr.flushMacEntries(bpath2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        try {
            list = mgr.getMacEntries(bpath2);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertEquals(0, list.size());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }


    /**
     * test method for {@link IObjectReader}
     */
    @Test
    public void testIObjectReader() {
        Object o = new VTenantPath("tenant");
        byte[] bytes = null;
        try {
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            ObjectOutputStream out = new ObjectOutputStream(bout);

            out.writeObject(o);
            out.close();
            bytes = bout.toByteArray();
        } catch (Exception e) {
            unexpected(e);
        }
        assertTrue(bytes.length != 0);

        // Deserialize the object.
        Object newobj = null;
        try {
            ByteArrayInputStream bin = new ByteArrayInputStream(bytes);
            ObjectInputStream in = new ObjectInputStream(bin);
            newobj = objReader.readObject(in);
            in.close();
        } catch (Exception e) {
            unexpected(e);
        }

        assertNotSame(o, newobj);
        assertEquals(o, newobj);

    }

    /**
     * test method for {@link ICacheUpdateAware}
     *
     * @throws InterruptedException  Test was interrupted.
     */
    @Test
    public void testICacheUpdateAware() throws InterruptedException {
        IVTNManager mgr = vtnManager;
        String root = GlobalConstants.STARTUPHOME.toString();
        String tenantListFileName = root + "vtn-default-tenant-names.conf";
        String configFileName100 = root + "vtn-" + "default" + "-" + "tenant100" + ".conf";
        String configFileName = root + "vtn-" + "default" + "-" + "tenant" + ".conf";

        Dictionary<String, Object> props = new Hashtable<String, Object>();
        VTNManagerAware listener = new VTNManagerAware();
        ServiceRegistration svReg =
            ServiceHelper.registerServiceWReg(IVTNManagerAware.class,
                                              "default", listener, props);

        // create tenant
        File tenantList = new File(tenantListFileName);
        tenantList.delete();

        File configFile = new File(configFileName);
        File configFile100 = new File(configFileName100);
        configFile.delete();
        configFile100.delete();

        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByName("0.0.0.0");
        } catch (Exception e) {
            unexpected(e);
        }
        CountDownLatch res = listener.restart(1);
        ClusterEventId evid = new ClusterEventId(ipaddr, 0);
        VTenantPath tpath = new VTenantPath("tenant");
        VTenantConfig tconf = new VTenantConfig(null);
        VTenant vtenant = new VTenant("tenant", tconf);
        mgr.addTenant(tpath, tconf);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        tenantList.delete();
        configFile.delete();
        configFile100.delete();

        VTenantEvent ev = new VTenantEvent(tpath, vtenant, UpdateType.ADDED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        assertFalse(tenantList.exists());
        assertFalse(configFile.exists());
        assertFalse(configFile100.exists());

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        assertTrue(tenantList.exists());
        assertTrue(configFile.exists());
        assertFalse(configFile100.exists());

        VTenantPath tpath100 = new VTenantPath("tenant100");
        VTenantConfig tconf100 = new VTenantConfig("tenant 100");
        VTenant vtenant100 = new VTenant("tenant100", tconf100);
        mgr.addTenant(tpath100, tconf100);
        tenantList.delete();
        configFile.delete();
        configFile100.delete();

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.ADDED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        assertFalse(tenantList.exists());
        assertFalse(configFile.exists());
        assertFalse(configFile100.exists());

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        assertTrue(tenantList.exists());
        assertFalse(configFile.exists());
        assertTrue(configFile100.exists());

        tenantList.delete();
        configFile.delete();
        configFile100.delete();

        // update
        ev = new VTenantEvent(tpath, vtenant, UpdateType.CHANGED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, false, true);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        checkFileExists(configFile, true, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, false, true);

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.CHANGED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, false, true);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, true, true);
        checkFileExists(tenantList, false, true);

        res = listener.restart(2);
        mgr.removeTenant(tpath);
        mgr.removeTenant(tpath100);
        assertTrue(res.await(10L, TimeUnit.SECONDS));

        // delete
        tenantList.delete();
        try {
            configFile.createNewFile();
            configFile100.createNewFile();
        } catch (IOException e) {
            unexpected(e);
        }

        ev = new VTenantEvent(tpath, vtenant, UpdateType.REMOVED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, true, false);
        checkFileExists(configFile100, true, false);
        checkFileExists(tenantList, false, true);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        checkFileExists(configFile, false, false);
        checkFileExists(configFile100, true, false);
        checkFileExists(tenantList, true, true);

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.REMOVED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, true, false);
        checkFileExists(configFile100, true, false);
        checkFileExists(tenantList, false, true);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(10L, TimeUnit.SECONDS));
        checkFileExists(configFile, true, true);
        checkFileExists(configFile100, false, true);
        checkFileExists(tenantList, true, true);

        svReg.unregister();
    }

    private void checkFileExists(File file, boolean result, boolean remove) {
        boolean exists = file.exists();
        assertEquals(result, exists);
        if (remove) {
            if (exists) {
                file.delete();
            }
        } else {
            try {
                file.createNewFile();
            } catch (IOException e) {
               unexpected(e);
            }
        }
    }

    /**
     * test method for {@link IConfigurationContainerAware}
     */
    @Test
    public void testIConfigurationContainerAware() {
        Status st = configContainerAware.saveConfiguration();
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * test method for {@link IInventoryListener}
     */
    @Test
    public void testIInventoryListener() {
        IInventoryListener mgr = inventoryListener;
        short[] vlans = new short[] { 0, 10, 4095 };

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        String ifname = "vinterface";
        VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
        Status st = null;

        List<VBridgePath> bpaths = new ArrayList<VBridgePath>();
        List<VBridgeIfPath> ifpaths = new ArrayList<VBridgeIfPath>();
        bpaths.add(bpath);
        ifpaths.add(ifp);

        createTenantAndBridgeAndInterface(vtnManager, tpath, bpaths, ifpaths);

        Node cnode = NodeCreator.createOFNode(0L);
        Node onode = NodeCreator.createOFNode(1L);
        NodeConnector nc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), cnode);
        NodeConnector otherNc
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), onode);

        // add a vlanmap to a vbridge
        for (Node node : createNodes(2)) {
            for (short vlan : vlans) {
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String emsg = vlconf.toString();
                VlanMap map = null;
                try {
                    map = vtnManager.addVlanMap(bpath, vlconf);
                    if (node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node != null && node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        unexpected(e);
                    } else {
                        continue;
                    }
                }

                VlanMap getmap = null;
                try {
                    getmap = vtnManager.getVlanMap(bpath, map.getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, getmap.getId(), map.getId());
                assertEquals(emsg, getmap.getNode(), node);
                assertEquals(emsg, getmap.getVlan(), vlan);
                checkNodeStatus(vtnManager, bpath, ifp,
                                (node == null) ? VNodeState.UP : VNodeState.DOWN,
                                VNodeState.UNKNOWN, emsg);

                // test for node connector change notify.
                Map<String, Property> propMap = null; // not used now.

                mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                putMacTableEntry(listenDataPacket, bpath, nc);

                mgr.notifyNodeConnector(nc, UpdateType.REMOVED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                checkMacTableEntry(vtnManager, bpath, true, vlconf.toString());

                mgr.notifyNodeConnector(nc, UpdateType.CHANGED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                mgr.notifyNodeConnector(otherNc, UpdateType.ADDED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                mgr.notifyNodeConnector(otherNc, UpdateType.REMOVED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                // test for node change notify.
                mgr.notifyNode(cnode, UpdateType.ADDED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                putMacTableEntry(listenDataPacket, bpath, nc);

                mgr.notifyNode(cnode, UpdateType.REMOVED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                checkMacTableEntry(vtnManager, bpath, true, emsg);

//                mgr.notifyNodeConnector(nc, UpdateType.ADDED, propMap);
//                checkNodeStatus(mgr, bpath, ifp, VNodeState.UP, VNodeState.UP, vlconf.toString());

                mgr.notifyNode(cnode, UpdateType.CHANGED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                mgr.notifyNode(onode, UpdateType.ADDED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                mgr.notifyNode(onode, UpdateType.REMOVED, propMap);
                checkNodeStatus(vtnManager, bpath, ifp,
                        (node == null) ? VNodeState.UP : VNodeState.DOWN,
                        VNodeState.UNKNOWN, emsg);

                // TODO: test for edge change notify

                st = vtnManager.removeVlanMap(bpath, map.getId());
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        }

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * check node and interface status specified bpath.
     */
    private void checkNodeStatus(IVTNManager mgr, VBridgePath bpath, VBridgeIfPath ifp,
            VNodeState bstate, VNodeState ifstate, String msg) {

        VBridge brdg = null;
        VInterface bif = null;
        try {
            bif = mgr.getBridgeInterface(ifp);
            brdg = mgr.getBridge(bpath);
        } catch (VTNException e) {
            unexpected(e);
        }

        assertEquals(msg, bstate, brdg.getState());
        assertEquals(msg, ifstate, bif.getState());
    }

    /**
     * put a Mac Address Table Entry to Mac Address Table of specified bridge.
     *
     * @param mgr   VTNManagerImpl
     * @param bpath VBridgePath
     * @param nc    NodeConnector
     */
    private void putMacTableEntry(IListenDataPacket listenData, VBridgePath bpath, NodeConnector nc) {
        byte[] src = new byte[] {(byte)0x00, (byte)0x01, (byte)0x01,
                                 (byte)0x01, (byte)0x01, (byte)0x01,};
        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                 (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        RawPacket inPkt = createARPRawPacket (src, dst, sender, target, (short)-1, nc, ARP.REQUEST);

        listenData.receiveDataPacket(inPkt);
    }

    /**
     * check a Mac Address Table Entry.
     *
     * @param mgr   VTNManagerImpl
     * @param bpath VBridgePath
     * @param isFlushed if true, expected result is 0. if not 0, execpted result is more than 0.
     * @param msg
     */
    private void checkMacTableEntry(IVTNManager mgr, VBridgePath bpath, boolean isFlushed, String msg) {
        List<MacAddressEntry> list = null;
        try {
            list = mgr.getMacEntries(bpath);
        } catch (VTNException e) {
           unexpected(e);
        }

        if (isFlushed) {
            assertEquals(msg, 0, list.size());
        } else {
            assertTrue(msg, list.size() > 0);
        }
    }

    private void sleep(long millis) {
        Thread.yield();
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            unexpected(e);
        }
    }

    /**
     * test method for {@link ITopologyManagerAware}
     */
    @Test
    public void testITopologyManagerAware() {
        // TODO: edgeUpdate(), edgeOverUtilized(), edgeUtilBackToNormal()
    }

    /**
     * test method for {@link IContainerListener}
     */
    @Test
    public void testIContainerListener() {
        // TODO: tagUpdate(), containerFlowUpdated(), nodeConnecctorUpdated(),
        //       containerModeUpdated()
    }

    /**
     * test case for {@link IListenDataPacket}
     */
    @Test
    public void testIListenDataPacket() {
        short[] vlans = new short[] { 0, 10, 4095 };
        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));
        byte[] cntMac = swmgr.getControllerMAC();

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = vtnManager.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(vtnManager.isActive());

        PacketResult result = listenDataPacket.receiveDataPacket(null);
        assertEquals(PacketResult.IGNORED, result);

        List<NodeConnector> connectors = createNodeConnectors(4);
        for (NodeConnector nc: connectors) {
            byte iphost = 1;
            for (EthernetAddress ea: createEthernetAddresses(false)) {
                byte [] bytes = ea.getValue();
                byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                        bytes[3], bytes[4], bytes[5]};
                byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
                byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
                byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                            (short)-1, nc, ARP.REQUEST);
                result = listenDataPacket.receiveDataPacket(inPkt);

                // because there are no topology, in this case always ignored.
                assertEquals(PacketResult.IGNORED, result);
            }

            // packet from controller.
            byte [] src = new byte[] {cntMac[0], cntMac[1], cntMac[2],
                                    cntMac[3], cntMac[4], cntMac[5]};
            byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                    (byte)0xff, (byte)0xff, (byte)0xff};
            byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
            byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

            RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short) -1, nc, ARP.REQUEST);
            result = listenDataPacket.receiveDataPacket(inPkt);

            assertEquals(PacketResult.IGNORED, result);
        }

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * test method for {@link IListenRoutingUpdates}
     */
    @Test
    public void testIListenRoutingUpdates() {
        // TODO: recalculateDone();
    }

    /**
     * test method for {@link IHostFinder}
     */
    @Test
    public void testIHostFinder() {
        IVTNManager mgr = vtnManager;
        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));
        byte[] cntMac = swmgr.getControllerMAC();
        short[] vlans = new short[] { 0, 10, 4095 };
        ConcurrentMap<VBridgePath, Set<NodeConnector>> mappedConnectors
            = new ConcurrentHashMap<VBridgePath, Set<NodeConnector>>();
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath);
        createTenantAndBridge(mgr, tpath, bpathlist);

        testIHostFinderCommon(bpath);

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // no tenant case
        testIHostFinderCommon(null);
    }

    /**
     * common routine for IHostFinder
     * @param bpath
     */
    private void testIHostFinderCommon(VBridgePath bpath) {
        IVTNManager mgr = vtnManager;
        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));
        short[] vlans = new short[] { 0, 10, 4095 };

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        InetAddress ia = null;
        InetAddress ia6 = null;
        try {
            ia = InetAddress.getByAddress(new byte[] {
                    (byte)10, (byte)0, (byte)0, (byte)1});
            ia6 = InetAddress.getByAddress(new byte[] {
                    (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                    (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                    (byte)0x0e1, (byte)0x23, (byte)0xe6, (byte)0x88,
                    (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0});
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        for (short vlan: vlans) {
            if (bpath != null) {
                VlanMapConfig vlconf = new VlanMapConfig(null, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                } catch (VTNException e) {
                    unexpected(e);
                }
            }

            hostFinder.find(ia);

            // probe()
            byte [] mac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
            Node node = NodeCreator.createOFNode(Long.valueOf(0x0));
            NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), node);
            HostNodeConnector hnode = null;
            try {
                hnode = new HostNodeConnector(mac, ia, nc, vlan);
            } catch (ConstructionException e) {
                unexpected(e);
            }

            hostFinder.probe(hnode);

            // if null case
            hostFinder.probe(null);
        }
    }

    /**
     * test method for
     * {@link IVTNModeListener}
     */
    @Test
    public void testIVTNModeListener() {

        // stub for test
        class VTNModeListenerStub implements IVTNModeListener {
            private int calledCount = 0;
            private Boolean oldactive = null;
            private final long sleepMilliTime = 10L;

            @Override
            public void vtnModeChanged(boolean active) {
                calledCount++;
                oldactive = Boolean.valueOf(active);
            }

            public int getCalledCount() {
                sleep(sleepMilliTime);
                int ret = calledCount;
                calledCount = 0;
                return ret;
            }

            public Boolean getCalledArg() {
                sleep(sleepMilliTime);
                Boolean ret = oldactive;
                oldactive = null;
                return ret;
            }
        };

        VTNModeListenerStub stub = new VTNModeListenerStub();
        Dictionary<String, Object> props = new Hashtable<String, Object>();
        ServiceRegistration listenerSeriveceReg
            = ServiceHelper.registerServiceWReg(IVTNModeListener.class, "default", stub, props);

        assertEquals(1, stub.getCalledCount());
        assertEquals(false, stub.getCalledArg());

        VTenantPath tpath = new VTenantPath("tenant");
        Status st = vtnManager.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertEquals(1, stub.getCalledCount());
        assertEquals(true, stub.getCalledArg());

        containerListener.containerModeUpdated(UpdateType.ADDED);
        assertEquals(1, stub.getCalledCount());
        assertEquals(false, stub.getCalledArg());

        containerListener.containerModeUpdated(UpdateType.REMOVED);
        assertEquals(1, stub.getCalledCount());
        assertEquals(true, stub.getCalledArg());

        // remove Tenant
        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertEquals(1, stub.getCalledCount());
        assertEquals(false, stub.getCalledArg());
    }

    // private methods.

    /**
     * method for setup enviroment.
     * create 1 Tenant and 1 bridge
     */
    private void createTenantAndBridge(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * method for setup enviroment.
     * create 1 Tenant and 1 bridge and vinterface
     */
    private void createTenantAndBridgeAndInterface(IVTNManager mgr, VTenantPath tpath,
            List<VBridgePath> bpaths, List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addBridgeInterface(ifpath, ifconf);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
    }
}
