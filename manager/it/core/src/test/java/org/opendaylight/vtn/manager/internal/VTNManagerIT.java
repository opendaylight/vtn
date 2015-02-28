/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.ops4j.pax.exam.CoreOptions.junitBundles;
import static org.ops4j.pax.exam.CoreOptions.mavenBundle;
import static org.ops4j.pax.exam.CoreOptions.bundle;
import static org.ops4j.pax.exam.CoreOptions.options;
import static org.ops4j.pax.exam.CoreOptions.systemPackages;
import static org.ops4j.pax.exam.CoreOptions.systemProperty;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Dictionary;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Iterator;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import javax.inject.Inject;

import org.ops4j.pax.exam.Configuration;
import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.junit.PaxExam;
import org.ops4j.pax.exam.spi.reactors.ExamReactorStrategy;
import org.ops4j.pax.exam.spi.reactors.PerClass;
import org.ops4j.pax.exam.util.PathUtils;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceRegistration;
import org.osgi.framework.Version;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.IVTNModeListener;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMapConfig;
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
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.internal.cluster.ClusterEventId;
import org.opendaylight.vtn.manager.internal.cluster.VTenantEvent;

import org.opendaylight.vtn.manager.integrationtest.internal.DataPacketServices;
import org.opendaylight.vtn.manager.integrationtest.internal.FlowProgrammerService;
import org.opendaylight.vtn.manager.integrationtest.internal.InventoryService;
import org.opendaylight.vtn.manager.integrationtest.internal.TopologyServices;

import org.opendaylight.controller.clustering.services.ICacheUpdateAware;
import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.topology.IPluginInTopologyService;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.sal.action.Action;
import org.opendaylight.controller.sal.action.Output;
import org.opendaylight.controller.sal.action.PopVlan;
import org.opendaylight.controller.sal.action.PushVlan;
import org.opendaylight.controller.sal.action.SetVlanId;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Edge;
import org.opendaylight.controller.sal.core.IContainerListener;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.Path;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.flowprogrammer.Flow;
import org.opendaylight.controller.sal.flowprogrammer.IPluginInFlowProgrammerService;
import org.opendaylight.controller.sal.inventory.IPluginInInventoryService;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.IListenDataPacket;
import org.opendaylight.controller.sal.packet.IPluginInDataPacketService;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.routing.IListenRoutingUpdates;
import org.opendaylight.controller.sal.routing.IRouting;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.ServiceHelper;
import org.opendaylight.controller.switchmanager.IInventoryListener;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.topologymanager.ITopologyManagerAware;

@RunWith(PaxExam.class)
@ExamReactorStrategy(PerClass.class)
public class VTNManagerIT extends TestBase {
    private static final Logger LOG = LoggerFactory.
        getLogger(VTNManagerIT.class);
    private static final String BUNDLE_VTN_MANAGER_IMPL =
        "org.opendaylight.vtn.manager.implementation";

    /**
     * The name of the package that provides stub OSGi services.
     */
    private static final String STUB_PACKAGE =
        "org.opendaylight.vtn.manager.integrationtest.internal";

    /**
     * The number of seconds to wait for the latch to be counted down to zero.
     */
    private static final long  LATCH_TIMEOUT = 10L;

    /**
     * The number of seconds to ensure that the latch is not counted down.
     */
    private static final long  LATCH_FALSE_TIMEOUT = 2L;

    // get the OSGI bundle context
    @Inject
    private BundleContext bc;

    private IVTNManager vtnManager = null;
    private IVTNGlobal vtnGlobal = null;
    private ICacheUpdateAware<ClusterEventId, Object> cacheUpdateAware = null;
    private IConfigurationContainerAware  configContainerAware = null;
    private IInventoryListener inventoryListener = null;
    private ITopologyManagerAware  topologyManagerAware = null;
    private IContainerListener containerListener = null;
    private IListenDataPacket listenDataPacket = null;
    private IListenRoutingUpdates listenRoutingUpdates = null;
    private IHostFinder hostFinder = null;

    private Bundle  implBundle;

    /**
     * Configure the OSGi container
     */
    @Configuration
    public Option[] config() {
        // Create configuration directory.
        File confdir = new File(GlobalConstants.STARTUPHOME.toString());
        if (confdir.exists()) {
            delete(confdir);
        }
        confdir.mkdirs();

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
                mavenBundle("org.opendaylight.controller", "statisticsmanager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "networkconfig.neutron").versionAsInProject(),

                // VTN Manager bundels
                mavenBundle("org.opendaylight.vtn", "manager").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.implementation").versionAsInProject(),
                mavenBundle("org.opendaylight.vtn", "manager.neutron").versionAsInProject(),

                // require myself for testing with openflow stub
                bundle(new File("./target/classes").toURI().toString()),

                mavenBundle("equinoxSDK381", "org.eclipse.equinox.ds").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.util").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.osgi.services").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.command").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.runtime").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.apache.felix.gogo.shell").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.cm").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.console").versionAsInProject(),
                mavenBundle("equinoxSDK381", "org.eclipse.equinox.launcher").versionAsInProject(),
                mavenBundle("equinoxSDK381", "javax.servlet").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.dependencymanager.shell").versionAsInProject(),

                mavenBundle("org.jboss.spec.javax.transaction", "jboss-transaction-api_1.1_spec").versionAsInProject(),
                mavenBundle("eclipselink", "javax.resource").versionAsInProject(),
                mavenBundle("org.apache.felix", "org.apache.felix.fileinstall").versionAsInProject(),

                mavenBundle("com.fasterxml.jackson.core", "jackson-annotations").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.core", "jackson-core").versionAsInProject(),
                mavenBundle("com.fasterxml.jackson.core", "jackson-databind").versionAsInProject(),
                mavenBundle("commons-net", "commons-net").versionAsInProject(),

                mavenBundle("org.ops4j.pax.exam", "pax-exam-container-native"),
                mavenBundle("org.ops4j.pax.exam", "pax-exam-junit4"),
                mavenBundle("org.ops4j.pax.exam", "pax-exam-link-mvn"),
                mavenBundle("org.ops4j.pax.url", "pax-url-aether"),

                mavenBundle("org.opendaylight.controller.thirdparty", "net.sf.jung2").versionAsInProject(),

                //OVSDB Bundles
                mavenBundle("org.opendaylight.ovsdb", "library").versionAsInProject(),
                mavenBundle("org.opendaylight.ovsdb", "ovsdb-plugin-compatibility-layer").versionAsInProject(),
                mavenBundle("org.opendaylight.ovsdb", "schema.openvswitch").versionAsInProject(),
                mavenBundle("org.opendaylight.ovsdb", "schema.hardwarevtep").versionAsInProject(),

                //List needed by OVSDB modules
                mavenBundle("org.opendaylight.controller", "sal.networkconfiguration").versionAsInProject(),
                mavenBundle("org.opendaylight.controller", "sal.networkconfiguration.implementation").versionAsInProject(),
                mavenBundle("org.mockito", "mockito-all").versionAsInProject(),
                mavenBundle("com.google.guava", "guava").versionAsInProject(),
                mavenBundle("io.netty", "netty-buffer").versionAsInProject(),
                mavenBundle("io.netty", "netty-common").versionAsInProject(),
                mavenBundle("io.netty", "netty-codec").versionAsInProject(),
                mavenBundle("io.netty", "netty-transport").versionAsInProject(),
                mavenBundle("io.netty", "netty-handler").versionAsInProject(),
                mavenBundle("com.google.code.gson", "gson").versionAsInProject(),

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

    // Validator of VTN, VBridge name, and VBridgeIF name.
    private boolean isValidName(String checkName) {
        if (checkName == null) {
            return false;
        } else if (checkName.isEmpty() || checkName.length() > 31) {
            return false;
        }
        return checkName.matches("[\\w&&[^_]][\\w]+");
    }

    @Before
    public void areWeReady() {
        assertNotNull(bc);
        boolean debugit = false;
        for (Bundle b: bc.getBundles()) {
            String name = b.getSymbolicName();
            int state = b.getState();
            if (state != Bundle.ACTIVE && state != Bundle.RESOLVED) {
                LOG.debug("Bundle:" + name + " state:" + stateToString(state));
                debugit = true;
            } else if (BUNDLE_VTN_MANAGER_IMPL.equals(name)) {
                implBundle = b;
            }
        }
        if (debugit) {
            LOG.debug("Do some debugging because some bundle is "
                    + "unresolved");
        }

        // Assert if true, if false we are good to go!
        assertFalse(debugit);
        assertNotNull(implBundle);
        assertEquals(Bundle.ACTIVE, implBundle.getState());

        ServiceReference r = bc.getServiceReference(IVTNManager.class.getName());
        if (r != null) {
            this.vtnManager = (IVTNManager)bc.getService(r);
            this.cacheUpdateAware =
                (ICacheUpdateAware<ClusterEventId, Object>)this.vtnManager;
            this.configContainerAware =
                (IConfigurationContainerAware)this.vtnManager;
            this.inventoryListener = (IInventoryListener)this.vtnManager;
            this.topologyManagerAware = (ITopologyManagerAware)this.vtnManager;
            this.containerListener = (IContainerListener)this.vtnManager;
            this.listenDataPacket = (IListenDataPacket)this.vtnManager;
            this.listenRoutingUpdates = (IListenRoutingUpdates)this.vtnManager;
            this.hostFinder = (IHostFinder)this.vtnManager;
        }

        r = bc.getServiceReference(IVTNGlobal.class.getName());
        if (r != null) {
            this.vtnGlobal = (IVTNGlobal)bc.getService(r);
        }

        // If either IVTNManager or IVTNGlobal is null, cannot run tests.
        assertNotNull(this.vtnManager);
        assertNotNull(this.vtnGlobal);
    }

    /**
     * Called when a test suite quits.
     */
    @After
    public void tearDown() {
        if (vtnManager == null) {
            return;
        }

        try {
            // Remove all VTNs in the default container.
            for (VTenant vtn: vtnManager.getTenants()) {
                String name = vtn.getName();
                LOG.debug("Clean up VTN: {}", name);
                VTenantPath path = new VTenantPath(name);
                Status st = vtnManager.removeTenant(path);
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        } catch (Exception e) {
            unexpected(e);
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
        testFindHost();
        testProbeHost();
        testIsActive();
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
        LOG.info("Running testAddGetRemoveTenant().");

        IVTNManager mgr = this.vtnManager;
        List<VTenantPath> tpathes = new ArrayList<VTenantPath>();
        List<String> descs = new ArrayList<String>();
        List<Integer> ivs = new ArrayList<Integer>();
        List<Integer> hvs = new ArrayList<Integer>();

        tpathes.add(new VTenantPath("tenant"));
        tpathes.add(new VTenantPath("Tenant"));
        tpathes.add(new VTenantPath("123456789012345678901234567890_"));
        tpathes.add(new VTenantPath("123456789012345678901234567890XX"));
        tpathes.add(new VTenantPath("_tenant"));
        tpathes.add(new VTenantPath("Tenant!!"));
        tpathes.add(new VTenantPath("tenant?"));
        tpathes.add(new VTenantPath("%TENANT%"));
        tpathes.add(new VTenantPath(""));
        tpathes.add(new VTenantPath(null));
        tpathes.add(null);

        descs.add(null);
        descs.add(new String("description."));

        ivs.add(null);
        ivs.add(new Integer(-1));
        ivs.add(new Integer(0));
        ivs.add(new Integer(1));
        ivs.add(new Integer(299));
        ivs.add(new Integer(300));
        ivs.add(new Integer(301));
        ivs.add(new Integer(65534));
        ivs.add(new Integer(65535));
        ivs.add(new Integer(65536));

        hvs.add(null);
        hvs.add(new Integer(-1));
        hvs.add(new Integer(0));
        hvs.add(new Integer(1));
        hvs.add(new Integer(299));
        hvs.add(new Integer(300));
        hvs.add(new Integer(301));
        hvs.add(new Integer(65534));
        hvs.add(new Integer(65535));
        hvs.add(new Integer(65536));

        assertFalse(mgr.isActive());

        // test for add
        for (VTenantPath tpath : tpathes) {
            String tname;
            if (tpath != null) {
                tname = tpath.getTenantName();
                assertEquals(
                    "(name)" + ((tname == null) ? "(null)" : (tname.isEmpty() ? "()" : tname)) + "tconf is null",
                    StatusCode.BADREQUEST,
                    mgr.addTenant(tpath, null).getCode()
                );
            } else {
                tname = null;
                assertEquals(
                    "tpath is null, tconf is null",
                    StatusCode.BADREQUEST,
                    mgr.addTenant(tpath, null).getCode()
                );
            }

            // Tests for getTenant() (NOTFOUND condition)
            VTenant tenant = null;
            try {
                if (isValidName(tname)) {
                    tenant = mgr.getTenant(tpath);
                    // It is strange, if VTNException is NOT thrown.
                    fail("Unexpected succeseed: " + tname);
                }
            } catch (VTNException vtne) {
                assertEquals("(name)" + tname + " Not Found", StatusCode.NOTFOUND, vtne.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }

            // Tests for removeTenant() (NOTFOUND condition)
            if (tpath != null && isValidName(tname)) {
                Status st = mgr.removeTenant(tpath);
                assertEquals("(name)" + tname + " Not Found", StatusCode.NOTFOUND, st.getCode());
            }

            for (String desc : descs) {
                for (Integer iv : ivs) {
                    for (Integer hv : hvs) {
                        VTenantConfig tconf = createVTenantConfig(desc, iv, hv);
                        Status st = mgr.addTenant(tpath, tconf);
                        String emsg;
                        if (tpath != null) {
                            emsg  = "(name)" + ((tname == null) ? "(null)" : (tname.isEmpty() ? "()" : tname));
                        } else {
                            emsg = "tpath is null ";
                        }
                        emsg = emsg + "(desc)" + desc + ",(iv)" +
                            ((iv == null) ? "null" : iv.intValue()) + ",(hv)" +
                            ((hv == null) ? "null" : hv.intValue());

                        if (tname == null) {
                            // "null" is invalid for tenant name.
                            // (Include that tpath is null.)
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                        } else if (!isValidName(tname)) {
                            // Invalid for tenant name.
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            //In this case, we do NOT give following tests.
                            continue;
                        } else if (iv != null && hv != null && iv.intValue() > 0 && hv.intValue() > 0
                                && iv.intValue() >= hv.intValue()) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else if ((iv == null || iv.intValue() < 0) && hv != null && hv.intValue() > 0
                                && 300 >= hv.intValue()) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else if ((iv != null && iv.intValue() > 65535) ||
                                   (hv != null && hv.intValue() > 65535)) {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            continue;
                        } else {
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                            assertTrue(emsg, mgr.isActive());
                            //Re-execute addTenant for testing CONFLICT status.
                            st = mgr.addTenant(tpath, tconf);
                            assertEquals(emsg, StatusCode.CONFLICT, st.getCode());
                            assertTrue(emsg, mgr.isActive());
                        }

                        // getTenant()
                        try {
                            tenant = mgr.getTenant(tpath);
                        } catch (VTNException vtne) {
                            if (tname == null) {
                                assertEquals(emsg, StatusCode.BADREQUEST,
                                             vtne.getStatus().getCode());
                            } else {
                                unexpected(vtne);
                            }
                        } catch (Exception e) {
                            unexpected(e);
                        }

                        if (tname != null) {
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
                        }

                        // removeTenant()
                        st = mgr.removeTenant(tpath);
                        if (tname != null) {
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                        } else {
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                        }
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
        LOG.info("Running testModifyTenant().");

        IVTNManager mgr = this.vtnManager;
        List<VTenantPath> tpathes = new ArrayList<VTenantPath>();
        List<String> descs = new ArrayList<String>();
        List<Integer> ivs = new ArrayList<Integer>();
        List<Integer> hvs = new ArrayList<Integer>();

        tpathes.add(new VTenantPath("vtn"));
        tpathes.add(new VTenantPath("123456789012345678901234567890_"));
        tpathes.add(new VTenantPath(null));
        tpathes.add(null);
        descs.add(null);
        descs.add(new String("desc"));
        ivs.add(new Integer(-1));
        ivs.add(new Integer(0));
        ivs.add(new Integer(1));
        ivs.add(new Integer(299));
        ivs.add(new Integer(300));
        ivs.add(new Integer(301));
        ivs.add(new Integer(65534));
        ivs.add(new Integer(65535));
        ivs.add(new Integer(65536));
        ivs.add(null);
        hvs.add(new Integer(-1));
        hvs.add(new Integer(0));
        hvs.add(new Integer(1));
        hvs.add(new Integer(299));
        hvs.add(new Integer(300));
        hvs.add(new Integer(301));
        hvs.add(new Integer(65534));
        hvs.add(new Integer(65535));
        hvs.add(new Integer(65536));
        hvs.add(null);

        int countValidTPath = 0;
        boolean first = true;
        for (VTenantPath tpath : tpathes) {
            VTenantConfig tconf = createVTenantConfig(new String("orig"), 20, 30);

            String tname = null;
            Status st = mgr.modifyTenant(tpath, tconf, true);
            if (tpath != null) {
                tname = tpath.getTenantName();
                if (isValidName(tname)) {
                    ++countValidTPath;
                    assertEquals("(name)" + tname + " Not Found", StatusCode.NOTFOUND, st.getCode());
                } else if (tname == null) {
                    assertEquals("(name)(null)", StatusCode.BADREQUEST, st.getCode());
                    st = mgr.modifyTenant(tpath, null, true);
                    assertEquals("(name)(null)", StatusCode.BADREQUEST, st.getCode());
                }
            } else {
                assertEquals("tpath is null", StatusCode.BADREQUEST, st.getCode());
                st = mgr.modifyTenant(tpath, null, true);
                assertEquals("tpath is null", StatusCode.BADREQUEST, st.getCode());
            }
            st = mgr.modifyTenant(tpath, tconf, false);
            if (tpath != null && tname != null && isValidName(tname)) {
                assertEquals("(name)" + tname + " Not Found", StatusCode.NOTFOUND, st.getCode());
            } else {
                String emsg = (tpath == null) ? "tpath is null" : "(name)(null)";
                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                st = mgr.modifyTenant(tpath, null, false);
                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                continue;
            }

            st = mgr.addTenant(tpath, tconf);

            // Test for condition that tconf is null.
            for (Boolean allSwitch : createBooleans(false)) {
                st = mgr.modifyTenant(tpath, null, allSwitch.booleanValue());
                VTenant tenant = null;
                try {
                    tenant = mgr.getTenant(tpath);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals("(name)" + tname + " tconf is null", StatusCode.BADREQUEST, st.getCode());
                assertEquals("(name)" + tname + " tconf is null", tenant.getDescription(), tconf.getDescription());
                assertEquals("(name)" + tname + " tconf is null", tenant.getIdleTimeout(), tconf.getIdleTimeout());
                assertEquals("(name)" + tname + " tconf is null", tenant.getHardTimeout(), tconf.getHardTimeout());
            }

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
                        } else if ((orgiv != null &&
                                    orgiv.intValue() > 65535) ||
                                   (orghv != null &&
                                    orghv.intValue() > 65535)) {
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
                                        String emsg = "(name)" + tname +
                                            "(ndesc)" + ndesc + ",(iv)" +
                                            ((iv == null) ? "null"
                                             : iv.intValue()) + ",(hv)" +
                                            ((hv == null) ? "null"
                                             : hv.intValue());

                                        try {
                                            tenant = mgr.getTenant(tpath);
                                        } catch (Exception e) {
                                            unexpected(e);
                                        }
                                        olddesc = tenant.getDescription();
                                        oldiv = tenant.getIdleTimeoutValue();
                                        oldhv = tenant.getHardTimeoutValue();

                                        tconf = createVTenantConfig(ndesc, iv, hv);
                                        st = mgr.modifyTenant(tpath, tconf, true);

                                        if (iv != null && hv != null
                                                && iv.intValue() > 0 && hv.intValue() > 0
                                                && iv.intValue() >= hv.intValue()) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else if ((iv != null && iv.intValue() > 65535)
                                                || (hv != null && hv.intValue() > 65535)) {
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
                                        } else {
                                            assertEquals(emsg, tname, tenant.getName());
                                            assertEquals(emsg, olddesc, tenant.getDescription());
                                            assertEquals(emsg, oldiv.intValue(), tenant.getIdleTimeout());
                                            assertEquals(emsg, oldhv.intValue(), tenant.getHardTimeout());
                                        }
                                    }
                                }

                                VTenantConfig tconfOrg = createVTenantConfig(desc,
                                        orgiv, orghv);
                                st = mgr.modifyTenant(tpath, tconfOrg, true);
                                assertEquals("(name)" + tname + " Reset", StatusCode.SUCCESS, st.getCode());

                                olddesc = (desc == null) ? null : new String(desc);
                                oldiv = (orgiv == null || orgiv.intValue() < 0) ? new Integer(300) : orgiv;
                                oldhv = (orghv == null || orghv.intValue() < 0) ? new Integer(0) : orghv;

                                // all == false
                                tconf = createVTenantConfig(desc, iv, hv);
                                st = mgr.modifyTenant(tpath, tconf, false);

                                String emsg = "(VTenangConfig(orig))" +
                                    tconfOrg.toString() + "(name)" + tname +
                                    "(ndesc)" + desc + ",(iv)" +
                                    ((iv == null) ? "null" : iv.intValue()) +
                                    ",(hv)" +
                                    ((hv == null) ? "null" : hv.intValue());

                                if ((iv == null || iv.intValue() < 0) &&
                                    (hv == null || hv.intValue() < 0)) {
                                    // both not set
                                    // not changed.

                                } else if (iv == null || iv.intValue() < 0) {
                                    // idle_timeout is not set
                                    if (hv.intValue() > 0 &&
                                        (oldiv.intValue() != 0 &&
                                         oldiv.intValue() >= hv.intValue())) {
                                        assertEquals(emsg,
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else if (hv.intValue() > 65535) {
                                        assertEquals(emsg,
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                } else if (hv == null || hv.intValue() < 0) {
                                    // hard_timeout is not set
                                    if (iv > 0 && oldhv > 0) {
                                        if (iv >= oldhv) {
                                            assertEquals(emsg,
                                                    StatusCode.BADREQUEST, st.getCode());
                                        } else {
                                            assertEquals(emsg,
                                                    StatusCode.SUCCESS, st.getCode());
                                        }
                                    } else if (iv.intValue() > 65535) {
                                        assertEquals(emsg,
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else {
                                        assertEquals(emsg,
                                                StatusCode.SUCCESS, st.getCode());
                                    }
                                } else {
                                    // set both
                                    if (hv.intValue() > 65535 || iv.intValue() > 65535) {
                                        assertEquals(emsg,
                                                StatusCode.BADREQUEST, st.getCode());
                                    } else if (iv.intValue() > 0 && hv.intValue() > 0) {
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
                                } else {
                                    assertEquals(emsg, tname, tenant.getName());
                                    assertEquals(emsg, olddesc, tenant.getDescription());
                                    assertEquals(emsg, oldiv.intValue(), tenant.getIdleTimeout());
                                    assertEquals(emsg, oldhv.intValue(), tenant.getHardTimeout());
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
            assertEquals(countValidTPath, list.size());
        } catch (Exception e) {
            unexpected(e);
        }

        for (VTenantPath tpath : tpathes) {
            if (tpath != null && tpath.getTenantName() != null) {
                mgr.removeTenant(tpath);
            }
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
        LOG.info("Running testBridge().");

        IVTNManager mgr = this.vtnManager;
        List<Integer> ages = new ArrayList<Integer>();
        List<String> tlist = new ArrayList<String>();
        List<String> blist = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();

        tlist.add("vtn");
        tlist.add("123456789012345678901234567890_");
        tlist.add("12345678901234567890_1234567890");
        tlist.add("123456789012345678901234567890XX");
        tlist.add("_tenant");
        tlist.add("Tenant!");
        tlist.add("%TENANT%");
        tlist.add("");
        tlist.add(null);
        blist.add("vbr");
        blist.add("012345678901234567890123456789_");
        blist.add("01234567890123456789_0123456789");
        blist.add("012345678901234567890123456789YY");
        blist.add("_bridge");
        blist.add("Bridge!");
        blist.add("%BRDG%");
        blist.add("");
        blist.add(null);
        ages.add(null);
        ages.add(-1);
        ages.add(0);
        ages.add(1);
        ages.add(9);
        ages.add(10);
        ages.add(11);
        ages.add(599);
        ages.add(600);
        ages.add(601);
        ages.add(999999);
        ages.add(1000000);
        ages.add(1000001);
        descs.add(null);
        descs.add("description...");

        boolean first = true;
        for (String tname : tlist) {
            boolean isValidBPath = false;
            boolean isValidTName = false;
            VTenantPath tpath = new VTenantPath(tname);

            Status st = mgr.addTenant(tpath, new VTenantConfig(null));
            if (!isValidName(tname)) {
                isValidTName = false;
                isValidBPath = false;
            } else {
                assertEquals(StatusCode.SUCCESS, st.getCode());
                isValidTName = true;
                isValidBPath = true;
            }

            if (first) {
                // Test a null condition.
                st = mgr.addBridge(null, null);
                assertEquals(StatusCode.BADREQUEST, st.getCode());
            }

            for (String bname : blist) {
                VBridgePath bpath = new VBridgePath(tname, bname);

                // Test a null condition.
                String emsg = "(VBridgePath)" + bpath.toString();
                st = mgr.addBridge(bpath, null);
                checkExceptionStatus(st, isValidTName, emsg);

                // Test for NOTFOUND status.
                try {
                    VBridge brgIgn = mgr.getBridge(bpath);
                    // It is strange, if VTNException is NOT thrown.
                    fail("Unexpected succeseed: " + bname);
                } catch (VTNException vtne) {
                    if (!(isValidTName)) {
                        // We tests only following condition.
                        if (tname == null) {
                            assertEquals(emsg, StatusCode.BADREQUEST, vtne.getStatus().getCode());
                        }
                    } else if (bname == null) {
                        assertEquals(emsg, StatusCode.BADREQUEST, vtne.getStatus().getCode());
                        isValidBPath = false;
                    } else if (!isValidName(bname)) {
                        // We DO NOT test this condition. Ignore.
                        isValidBPath = false;
                    } else {
                        assertEquals(emsg, StatusCode.NOTFOUND, vtne.getStatus().getCode());
                    }
                } catch (Exception e) {
                    unexpected(e);
                }

                if (isValidBPath) {
                    st = mgr.removeBridge(bpath);
                    assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
                }

                for (String desc : descs) {
                    for (Integer age : ages) {
                        boolean isValidBConf = false;
                        VBridgeConfig bconf = createVBridgeConfig(desc, age);
                        emsg = "(VBridgePath)" + bpath.toString()
                                + "(VBridgeConfig)" + bconf.toString()
                                + "(age)" + ((age == null) ? "null" : age.intValue());

                        // Test a null condition.
                        st = mgr.addBridge(null, bconf);
                        assertEquals(StatusCode.BADREQUEST, st.getCode());

                        st = mgr.addBridge(bpath, bconf);
                        if (!(isValidTName)) {
                            // We tests only following condition.
                            if (tname == null) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            }
                        } else if (!isValidBPath) {
                            // This is an invalid condition.
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                        } else if ((age != null) &&
                                   ((0 <= age && age < 10) ||
                                    (age > 1000000))) {
                            // This is an invalid condition.
                            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            isValidBConf = false;
                        } else {
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                            isValidBConf = true;

                            //Re-add this Bridge for testing CONFLICT status.
                            st = mgr.addBridge(bpath, bconf);
                            assertEquals(emsg, StatusCode.CONFLICT, st.getCode());
                        }

                        VBridge brdg = null;
                        try {
                            brdg = mgr.getBridge(bpath);
                        } catch (VTNException vtne) {
                            // We DO NOT test when bpath is invalid.
                            if (isValidBPath) {
                                if (!(isValidBConf)) {
                                    assertEquals(emsg, StatusCode.NOTFOUND, vtne.getStatus().getCode());
                                } else {
                                    unexpected(vtne);
                                }
                            }
                        } catch (Exception e) {
                            unexpected(e);
                        }

                        String olddesc = null;
                        int oldage = 0;
                        if (brdg != null) {
                            assertEquals(emsg, bname, brdg.getName());
                            assertEquals(emsg, desc, brdg.getDescription());
                            if (age == null || age < 0) {
                                assertEquals(emsg, 600, brdg.getAgeInterval());
                            } else {
                                assertEquals(emsg, age.intValue(), brdg.getAgeInterval());
                            }
                            assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());

                            olddesc = brdg.getDescription();
                            oldage = brdg.getAgeInterval();
                        }

                        if (!(isValidBConf)) {
                            // If bconf is invalid, go next pattern
                            continue;
                        }

                        for (String newdesc : descs) {
                            for (Integer newage : ages) {
                                boolean isValidBConfNew = false;
                                bconf = createVBridgeConfig(newdesc, newage);

                                if (newage == null || newage < 0) {
                                    isValidBConfNew = true;
                                } else if (10 <= newage && newage <= 1000000) {
                                    isValidBConfNew = true;
                                }

                                st = mgr.modifyBridge(bpath, bconf, false);

                                String emsgMod = emsg
                                        + "(VBridgeConfig(new))" + bconf.toString()
                                        + "(age(new))"
                                        + ((newage == null) ? "null" : newage.intValue());

                                if (!(isValidBPath)) {
                                    if (tname == null || (isValidTName && bname == null)) {
                                        assertEquals(emsgMod, StatusCode.BADREQUEST, st.getCode());
                                    } else {
                                        // In this case, Status code is either BADREQUEST or NOTFOUND
                                        checkExceptionStatus(st, false, emsgMod);
                                    }
                                } else {
                                    if (isValidBConfNew) {
                                        assertEquals(emsgMod, StatusCode.SUCCESS, st.getCode());
                                    } else {
                                        assertEquals(emsgMod, StatusCode.BADREQUEST, st.getCode());
                                    }
                                }

                                brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (VTNException vtne) {
                                    if (isValidBPath) {
                                        unexpected(vtne);
                                    }
                                } catch (Exception e) {
                                    unexpected(e);
                                }

                                if (brdg != null && st.isSuccess()) {
                                    if (newdesc == null) {
                                        assertEquals(emsgMod,
                                                olddesc, brdg.getDescription());
                                    } else {
                                        assertEquals(emsgMod,
                                                newdesc, brdg.getDescription());
                                    }
                                    if (newage == null || newage < 0) {
                                        assertEquals(emsgMod,
                                                oldage, brdg.getAgeInterval());
                                    } else {
                                        assertEquals(emsgMod,
                                                newage.intValue(), brdg.getAgeInterval());
                                    }
                                    olddesc = brdg.getDescription();
                                    oldage = brdg.getAgeInterval();
                                } else if (brdg != null) {
                                    // Not success. No changing is expected.
                                    assertEquals(emsgMod,
                                            olddesc, brdg.getDescription());
                                    assertEquals(emsgMod,
                                            oldage, brdg.getAgeInterval());
                                }

                                if (brdg != null) {
                                    assertEquals(emsgMod, bname, brdg.getName());
                                    assertEquals(emsgMod,
                                            VNodeState.UNKNOWN, brdg.getState());
                                }
                            }
                        }

                        st = mgr.removeBridge(bpath);
                        if (!(isValidBPath)) {
                            if (tname == null || (isValidTName && bname == null)) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            } else {
                                // In this case, Status code is either BADREQUEST or NOTFOUND
                                checkExceptionStatus(st, false, emsg);
                            }
                        } else {
                            assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                        }
                    }

                    if (!(isValidBPath)) {
                        // If bpath is invalid, go next pattern.
                        continue;
                    }

                    if (first) {
                        String olddesc = "desc";
                        int oldage = 10;
                        VBridgeConfig bconf = new VBridgeConfig(olddesc, oldage);
                        st = mgr.addBridge(bpath, bconf);
                        for (String newdesc : descs) {
                            for (Integer newage : ages) {
                                boolean isValidBConfNew = false;
                                bconf = createVBridgeConfig(newdesc, newage);

                                if (newage == null || newage < 0) {
                                    isValidBConfNew = true;
                                } else if (10 <= newage && newage <= 1000000) {
                                    isValidBConfNew = true;
                                }

                                st = mgr.modifyBridge(bpath, bconf, true);

                                String emsgMod =
                                        "(VBridgeConfig(new))" + bconf.toString()
                                        + "(age(new))"
                                        + ((newage == null) ? "null" : newage.intValue());

                                if (isValidBConfNew) {
                                    assertEquals(emsgMod, StatusCode.SUCCESS, st.getCode());
                                } else {
                                    assertEquals(emsgMod, StatusCode.BADREQUEST, st.getCode());
                                }

                                VBridge brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                                assertEquals(emsgMod, bname, brdg.getName());
                                if (st.isSuccess()) {
                                    assertEquals(emsgMod, newdesc, brdg.getDescription());
                                    if (newage == null || newage < 0) {
                                        assertEquals(emsgMod, 600, brdg.getAgeInterval());
                                    } else {
                                        assertEquals(emsgMod,
                                                newage.intValue(), brdg.getAgeInterval());
                                    }
                                    olddesc = brdg.getDescription();
                                    oldage = brdg.getAgeInterval();
                                } else {
                                    // Not success. No changing is expected.
                                    assertEquals(emsgMod,
                                            olddesc, brdg.getDescription());
                                    assertEquals(emsgMod,
                                            oldage, brdg.getAgeInterval());
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

            if (isValidTName) {
                try {
                    List<VBridge> list = mgr.getBridges(tpath);
                    assertEquals(0, list.size());
                } catch (VTNException e) {
                    unexpected(e);
                }
                st = mgr.removeTenant(tpath);
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        }

        // add mulitple entry.
        for (String tname : tlist) {
            boolean isValidTName = false;
            VTenantPath tpath = new VTenantPath(tname);
            Status st;
            if (tname == null) {
                try {
                    List<VBridge> list = mgr.getBridges(tpath);
                    // It is strange, if VTNException is NOT thrown.
                    fail("Unexpected succeseed: " + tpath.toString());
                } catch (VTNException vtne) {
                    st = vtne.getStatus();
                    assertEquals("(VTenantPath)" + tpath.toString(),
                            StatusCode.BADREQUEST, st.getCode());
                } catch (Exception e) {
                    unexpected(e);
                }
                isValidTName = false;
            } else if (!isValidName(tname)) {
                isValidTName = false;
            } else {
                // Test for NOTFOUND status.
                try {
                    List<VBridge> list = mgr.getBridges(tpath);
                    // It is strange, if VTNException is NOT thrown.
                    fail("Unexpected succeseed: " + tpath.toString());
                } catch (VTNException vtne) {
                    st = vtne.getStatus();
                    assertEquals("(VTenantPath)" + tpath.toString(),
                            StatusCode.NOTFOUND, st.getCode());
                } catch (Exception e) {
                    unexpected(e);
                }
                st = mgr.addTenant(tpath, new VTenantConfig(null));
                assertEquals("(VTenantPath)" + tpath.toString(),
                        StatusCode.SUCCESS, st.getCode());
                isValidTName = true;
            }

            int countValidBPath = 0;
            if (isValidTName) {
                for (String bname : blist) {
                    if (!isValidName(bname)) {
                        // These are invalid conditions.
                        continue;
                    }
                    VBridgePath bpath = new VBridgePath(tname, bname);
                    VBridgeConfig bconf = createVBridgeConfig(null, null);

                    st = mgr.addBridge(bpath, bconf);
                    assertEquals("(VBridgePath)" + bpath.toString()
                            + "(VBridgeConfig)" + bconf.toString(),
                            StatusCode.SUCCESS, st.getCode());
                    ++countValidBPath;
                }
            }
            try {
                List<VBridge> list = mgr.getBridges(tpath);
                assertEquals("(VTenantPath)" + tpath.toString(),
                        countValidBPath, list.size());
            } catch (VTNException e) {
                if (isValidTName) {
                    unexpected(e);
                } else {
                    if (tname == null) {
                        assertEquals("(VTenantPath)" + tpath.toString(),
                                StatusCode.BADREQUEST, e.getStatus().getCode());
                    }
                }
            }
            if (isValidTName) {
                st = mgr.removeTenant(tpath);
                assertEquals("(VTenantPath)" + tpath.toString(),
                        StatusCode.SUCCESS, st.getCode());
            }
        }
    }

    /**
     * Test method for
     * {@link IVTNManager#addInterface(VBridgeIfPath, VInterfaceConfig)},
     * {@link IVTNManager#modifyInterface(VBridgeIfPath, VInterfaceConfig, boolean)},
     * {@link IVTNManager#removeInterface(VBridgeIfPath)},
     * {@link IVTNManager#getInterfaces(VBridgePath)},
     * {@link IVTNManager#getInterface(VBridgeIfPath)}.
     */
    private void testBridgeInterface() {
        LOG.info("Running testBridgeInterface().");
        IVTNManager mgr = this.vtnManager;
        List<String> tlist = new ArrayList<String>();
        List<String> blist = new ArrayList<String>();
        List<String> iflist = new ArrayList<String>();
        List<String> descs = new ArrayList<String>();

        VBridgeIfPath nullIfPath = null;

        tlist.add("vtn");
        tlist.add("123456789012345678901234567890_");
        tlist.add("12345678901234567890_1234567890");
        tlist.add("123456789012345678901234567890XX");
        tlist.add("_tenant");
        tlist.add("Tenant!");
        tlist.add("%TENANT%");
        tlist.add("");
        tlist.add(null);
        blist.add("vbr");
        blist.add("012345678901234567890123456789_");
        blist.add("01234567890123456789_0123456789");
        blist.add("012345678901234567890123456789YY");
        blist.add("_bridge");
        blist.add("Bridge!");
        blist.add("%BRDG%");
        blist.add("");
        blist.add(null);
        iflist.add("vinterface");
        iflist.add("abcdefghijklmnoopqrstuvwxyz1234");
        iflist.add("abcdefghijklmnoopqrstuvwxyz12345");
        iflist.add("_interface");
        iflist.add("Interface!!");
        iflist.add("%INTF%");
        iflist.add("$MY_INTF");
        iflist.add("");
        iflist.add(null);
        descs.add(null);
        descs.add("description");

        boolean first = true;

        for (String tname : tlist) {
            boolean isValidTName = isValidName(tname);
            Status st = null;

            // Test for some NOTFOUND conditions.
            if (isValidTName) {
                VBridgeIfPath ifp = new VBridgeIfPath(tname, "brdg", "interface");
                VInterfaceConfig iconf = new VInterfaceConfig(null, Boolean.FALSE);
                st = mgr.addInterface(ifp, iconf);
                assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                        StatusCode.NOTFOUND, st.getCode());

                try {
                    List<VInterface> list = mgr.getInterfaces(ifp);
                    // It is strange, if VTNException is NOT thrown.
                    fail("Unexpected succeseed : " + tname);
                } catch (VTNException vtne) {
                    assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                            StatusCode.NOTFOUND, vtne.getStatus().getCode());
                } catch (Exception e) {
                    unexpected(e);
                }

                try {
                    VInterface vif = mgr.getInterface(ifp);
                    // It is strange, if VTNException is NOT thrown.
                    fail("Unexpected succeseed : " + tname);
                } catch (VTNException vtne) {
                    assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                                 StatusCode.NOTFOUND,
                                 vtne.getStatus().getCode());
                } catch (Exception e) {
                    unexpected(e);
                }

                for (Boolean allSwitch : createBooleans(false)) {
                    st = mgr.modifyInterface(ifp, iconf, allSwitch.booleanValue());
                    assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                            StatusCode.NOTFOUND, st.getCode());
                }

                st = mgr.removeInterface(ifp);
                assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                        StatusCode.NOTFOUND, st.getCode());
            }

            VTenantPath tpath = new VTenantPath(tname);
            st = mgr.addTenant(tpath, new VTenantConfig(null));
            if (isValidTName) {
                assertEquals("(VTenantPath)" + tpath.toString(),
                        StatusCode.SUCCESS, st.getCode());
            } else {
                assertEquals("(VTenantPath)" + tpath.toString(),
                        StatusCode.BADREQUEST, st.getCode());
            }

            for (String bname : blist) {
                boolean isValidBName = isValidName(bname);
                boolean isValidBPath = isValidTName && isValidBName;

                // Test for some NOTFOUND conditions.
                if (isValidBPath) {
                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, "interface");
                    VInterfaceConfig iconf = new VInterfaceConfig(null, Boolean.FALSE);
                    st = mgr.addInterface(ifp, iconf);
                    assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                            StatusCode.NOTFOUND, st.getCode());

                    try {
                        List<VInterface> list = mgr.getInterfaces(ifp);
                        // It is strange, if VTNException is NOT thrown.
                        fail("Unexpected succeseed : " + tname);
                    } catch (VTNException vtne) {
                        assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                                StatusCode.NOTFOUND, vtne.getStatus().getCode());
                    } catch (Exception e) {
                        unexpected(e);
                    }

                    try {
                        VInterface vif = mgr.getInterface(ifp);
                        // It is strange, if VTNException is NOT thrown.
                        fail("Unexpected succeseed : " + tname);
                    } catch (VTNException vtne) {
                        assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                                StatusCode.NOTFOUND, vtne.getStatus().getCode());
                    } catch (Exception e) {
                        unexpected(e);
                    }

                    for (Boolean allSwitch : createBooleans(false)) {
                        st = mgr.modifyInterface(ifp, iconf, allSwitch.booleanValue());
                        assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                                StatusCode.NOTFOUND, st.getCode());
                    }

                    st = mgr.removeInterface(ifp);
                    assertEquals("(VBridgeIfPath)" + tname + "(Not Found)",
                            StatusCode.NOTFOUND, st.getCode());
                }

                VBridgePath bpath = new VBridgePath(tname, bname);
                VBridgeConfig bconf = createVBridgeConfig(null, null);

                List<VInterface> list = null;
                // Test a null condition.
                if (first) {
                    try {
                        list = mgr.getInterfaces(nullIfPath);
                        // It is strange, if VTNException is NOT thrown.
                        fail("Unexpected succeseed : (null)");
                    } catch (VTNException vtne) {
                        assertEquals("(VBridgePath)(null)",
                                StatusCode.BADREQUEST, vtne.getStatus().getCode());
                    } catch (Exception e) {
                        unexpected(e);
                    }
                }

                // Test for NOTFOUND status.
                try {
                    if (isValidBPath) {
                        list = mgr.getInterfaces(bpath);
                        // It is strange, if VTNException is NOT thrown.
                        fail("Unexpected succeseed : " + bpath.toString());
                    }
                } catch (VTNException vtne) {
                    assertEquals("(VBridgePath)" + bpath.toString(),
                            StatusCode.NOTFOUND, vtne.getStatus().getCode());
                } catch (Exception e) {
                    unexpected(e);
                }

                st = mgr.addBridge(bpath, bconf);
                if (isValidBPath) {
                    assertEquals("(VBridgePath)" + bpath.toString(),
                            StatusCode.SUCCESS, st.getCode());
                }

                list = null;
                try {
                    list = mgr.getInterfaces(bpath);
                } catch (VTNException vtne) {
                    if (isValidBPath) {
                        unexpected(vtne);
                    } else if (tname == null || (isValidTName && bname == null)) {
                        assertEquals("(VBridgePath)" + bpath.toString(),
                                StatusCode.BADREQUEST, vtne.getStatus().getCode());
                    }
                } catch (Exception e) {
                    unexpected(e);
                }
                if (isValidBPath) {
                    assertEquals("(VBridgePath)" + bpath.toString(),
                            0, list.size());
                }

                int countValidBIfPath = 0;
                for (String ifname : iflist) {
                    boolean isValidBIfName = isValidName(ifname);
                    boolean isValidBIfPath = isValidBPath && isValidBIfName;
                    if (isValidBIfPath) {
                        ++countValidBIfPath;
                    }

                    VBridgeIfPath ifp = new VBridgeIfPath(tname, bname, ifname);
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            // Test some null conditions.
                            if (first) {
                                st = mgr.addInterface(nullIfPath, null);
                                assertEquals("(VBridgeIfPath)(null)(VInterfaceConfig)(null)",
                                        StatusCode.BADREQUEST, st.getCode());

                                try {
                                    VInterface vif = mgr.getInterface(nullIfPath);
                                    // It is strange, if VTNException is NOT thrown.
                                    fail("Unexpected succeseed : (null)");
                                } catch (VTNException vtne) {
                                    assertEquals("(VBridgeIfPath)(null)",
                                            StatusCode.BADREQUEST, vtne.getStatus().getCode());
                                } catch (Exception e) {
                                    unexpected(e);
                                }
                            }
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + ",(VInterfaceConfig)(null)";
                            st = mgr.addInterface(ifp, null);
                            if (isValidBIfPath) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            } else {
                                // In this case, Status code is either BADREQUEST or NOTFOUND
                                checkExceptionStatus(st, false, emsg);
                            }

                            // Test for NOTFOUND status
                            VInterface vif = null;
                            if (isValidBIfPath) {
                                try {
                                    vif = mgr.getInterface(ifp);
                                    // It is strange, if VTNException is NOT thrown.
                                    fail("Unexpected succeseed : " + ifp.toString());
                                } catch (VTNException vtne) {
                                    assertEquals(emsg, StatusCode.NOTFOUND, vtne.getStatus().getCode());
                                } catch (Exception e) {
                                    unexpected(e);
                                }

                                st = mgr.removeInterface(ifp);
                                assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
                            }

                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + ",(VInterfaceConfig)" + ifconf.toString();
                            st = mgr.addInterface(ifp, ifconf);
                            if (isValidBIfPath) {
                                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());

                                // Re-add for CONFLICT status
                                st = mgr.addInterface(ifp, ifconf);
                                assertEquals(emsg, StatusCode.CONFLICT, st.getCode());
                            } else if (tname == null || (isValidTName && bname == null) ||
                                    (isValidBPath && !isValidBIfName)) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            } else {
                                // In this case, Status code is either BADREQUEST or NOTFOUND
                                checkExceptionStatus(st, false, emsg);
                            }

                            vif = null;
                            try {
                                vif = mgr.getInterface(ifp);
                            } catch (VTNException vtne) {
                                if (!isValidBIfPath) {
                                    if (tname == null || (isValidTName && bname == null) ||
                                        (isValidBPath && ifname == null)) {
                                        assertEquals(emsg, StatusCode.BADREQUEST, vtne.getStatus().getCode());
                                    }
                                } else {
                                    unexpected(vtne);
                                }
                            } catch (Exception e) {
                                unexpected(e);
                            }

                            if (vif != null) {
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
                            }

                            if (isValidBPath) {
                                VBridge brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (VTNException e) {
                                    unexpected(e);
                                }
                                assertEquals(emsg, VNodeState.UNKNOWN, brdg.getState());
                            }

                            // Test a null condition.
                            if (first) {
                                st = mgr.removeInterface(nullIfPath);
                                assertEquals("(VBridgeIfPath)(null)",
                                        StatusCode.BADREQUEST, st.getCode());
                            }
                            st = mgr.removeInterface(ifp);
                            if (isValidBIfPath) {
                                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                            } else if (tname == null || (isValidTName && bname == null) ||
                                    (isValidBPath && ifname == null)) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            } else {
                                // In this case, Status code is either BADREQUEST or NOTFOUND
                                checkExceptionStatus(st, false, emsg);
                            }
                        }
                    }

                    // Test for some NOTFOUND conditions.
                    if (isValidBIfPath) {
                        for (String desc : descs) {
                            for (Boolean enabled : createBooleans()) {
                                VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                                String emsg = "(VBridgeIfPath)" + ifp.toString()
                                        + "(VInterfaceConfig)" + ifconf.toString();
                                for (Boolean allSwitch : createBooleans(false)) {
                                    st = mgr.modifyInterface(ifp, ifconf, allSwitch.booleanValue());
                                    assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
                                }
                            }
                        }
                    }

                    // Test some null condition
                    if (first) {
                        for (Boolean allSwitch : createBooleans(false)) {
                            st = mgr.modifyInterface(nullIfPath, null,
                                                     allSwitch.booleanValue());
                            assertEquals("(VBridgeIfPath)(null) (VInterfaceConfig)(null)",
                                    StatusCode.BADREQUEST, st.getCode());
                        }
                    }

                    if (isValidBIfPath) {
                        // Add bridge Interface for modify(false) and modify(true)
                        st = mgr.addInterface(ifp, new VInterfaceConfig("desc", Boolean.FALSE));
                        assertEquals(StatusCode.SUCCESS, st.getCode());

                        // Test some null condition
                        for (Boolean allSwitch : createBooleans(false)) {
                            st = mgr.modifyInterface(ifp, null, allSwitch.booleanValue());
                            assertEquals("(VBridgeIfPath)" + ifp.toString() + "(VInterfaceConfig)(null)",
                                    StatusCode.BADREQUEST, st.getCode());
                        }
                    }

                    // for modify(false)
                    String olddesc = new String("desc");
                    Boolean oldenabled = Boolean.FALSE;
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            // Test some null condition
                            if (first) {
                                st = mgr.modifyInterface(nullIfPath, ifconf,
                                                         false);
                                assertEquals("(VBridgeIfPath)(null) (VInterfaceConfig)" + ifconf.toString(),
                                        StatusCode.BADREQUEST, st.getCode());
                            }
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + "(VInterfaceConfig)" + ifconf.toString();

                            st = mgr.modifyInterface(ifp, ifconf, false);
                            if (isValidBIfPath) {
                                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                            } else if (tname == null || (isValidTName && bname == null) ||
                                    (isValidBPath && ifname == null)) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            } else {
                                // In this case, Status code is either BADREQUEST or NOTFOUND
                                checkExceptionStatus(st, false, emsg);
                            }

                            if (!isValidBIfPath) {
                                // If ifp is not valid, go next pattern.
                                continue;
                            }

                            VInterface vif = null;
                            try {
                                vif = mgr.getInterface(ifp);
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
                    if (isValidBIfPath) {
                        st = mgr.modifyInterface(ifp, new VInterfaceConfig("desc", Boolean.FALSE), true);
                        assertEquals(StatusCode.SUCCESS, st.getCode());
                    }
                    for (String desc : descs) {
                        for (Boolean enabled : createBooleans()) {
                            VInterfaceConfig ifconf = new VInterfaceConfig(desc, enabled);
                            // Test some null condition
                            if (first) {
                                st = mgr.modifyInterface(nullIfPath, ifconf, true);
                                assertEquals("(VBridgeIfPath)(null) (VInterfaceConfig)" + ifconf.toString(),
                                        StatusCode.BADREQUEST, st.getCode());
                            }
                            String emsg = "(VBridgeIfPath)" + ifp.toString()
                                    + "(VInterfaceConfig)" + ifconf.toString();

                            st = mgr.modifyInterface(ifp, ifconf, true);
                            if (isValidBIfPath) {
                                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                            } else if (tname == null || (isValidTName && bname == null) ||
                                    (isValidBPath && ifname == null)) {
                                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                            } else {
                                // In this case, Status code is either BADREQUEST or NOTFOUND
                                checkExceptionStatus(st, false, emsg);
                            }

                            if (!isValidBIfPath) {
                                // If ifp is not valid, go next pattern.
                                continue;
                            }

                            VInterface vif = null;
                            try {
                                vif = mgr.getInterface(ifp);
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
                    list = mgr.getInterfaces(bpath);
                } catch (VTNException vtne) {
                    if (!isValidBPath) {
                        list = new ArrayList<VInterface>();
                    } else {
                        unexpected(vtne);
                    }
                } catch (Exception e) {
                    unexpected(e);
                }
                assertEquals(countValidBIfPath, list.size());
            }

            if (isValidTName) {
                st = mgr.removeTenant(tpath);
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }

            first = false;
        }

        // TODO: NOTACCEPTABLE
    }

    /**
     * Test method for
     * {@link IVTNManager#addVlanMap(VBridgePath, VlanMapConfig)},
     * {@link IVTNManager#removeVlanMap(VBridgePath, java.lang.String)},
     * {@link IVTNManager#getVlanMap(VBridgePath, java.lang.String)},
     * {@link IVTNManager#getVlanMap(VBridgePath, VlanMapConfig)},
     * {@link IVTNManager#getVlanMaps(VBridgePath)}.
     */
    private void testVlanMap() {
        LOG.info("Running testVlanMap().");

        IVTNManager mgr = this.vtnManager;
        short[] vlans = new short[] {
            -1, 0, 1, 10, 100, 1000, 4094, 4095, 4096
        };

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
        st = mgr.addInterface(ifp, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // add a vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            for (short vlan : vlans) {
                boolean isValidVLANid = (0 <= vlan && vlan < 4096);
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String emsg = "(VlanMapConfig)" + vlconf.toString();

                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if ((node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW)
                            || !isValidVLANid) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node == null || node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        if (isValidVLANid) {
                            unexpected(e);
                        } else {
                            assertEquals(emsg, StatusCode.BADREQUEST, e.getStatus().getCode());
                        }
                    } else {
                        assertEquals(emsg, StatusCode.BADREQUEST, e.getStatus().getCode());
                    }
                    continue;
                }
                assertNotNull(emsg, map);

                VlanMap getmap = null;
                try {
                    getmap = mgr.getVlanMap(bpath, map.getId());
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, getmap.getId(), map.getId());
                assertEquals(emsg, getmap.getNode(), node);
                assertEquals(emsg, getmap.getVlan(), vlan);

                try {
                    getmap = mgr.getVlanMap(bpath, vlconf);
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
                             (node == null) ? VNodeState.UP
                             : VNodeState.DOWN, brdg.getState());

                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }

        // add multi vlanmap to a vbridge
        for (Node node : createNodes(3)) {
            int countValidVLAN = 0;
            for (short vlan : vlans) {
                boolean isValidVLANid = (0 <= vlan && vlan < 4096);
                if (isValidVLANid) {
                    ++countValidVLAN;
                }
                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                VlanMap map = null;
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                    if ((node != null && node.getType() != NodeConnector.NodeConnectorIDType.OPENFLOW)
                            || !isValidVLANid) {
                        fail("throwing Exception was expected.");
                    }
                } catch (VTNException e) {
                    if (node == null || node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                        if (isValidVLANid) {
                            unexpected(e);
                        }
                    }
                    continue;
                } catch (Exception e) {
                    unexpected(e);
                }
                assertNotNull(map);
            }

            List<VlanMap> list = null;
            try {
                list = mgr.getVlanMaps(bpath);
            } catch (Exception e) {
                unexpected(e);
            }
            String emsg = "(Node)" + ((node == null) ? "null" : node.toString());
            if (node == null ||
                node.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                assertEquals(emsg, countValidVLAN, list.size());
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, (node == null) ? VNodeState.UP
                             : VNodeState.DOWN, brdg.getState());
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
                assertEquals(emsg + ",(VlanMap)" + map.toString(),
                             StatusCode.SUCCESS, st.getCode());
            }
        }

        // test NOTFOUND
        List<VBridgePath> listBpath = new ArrayList<VBridgePath>();
        listBpath.add(bpath);
        listBpath.add(new VBridgePath(tname, "dummy"));

        for (VBridgePath vbpath : listBpath) {
            Node node = NodeCreator.createOFNode(0L);
            short vlan = 0;
            VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
            String emsg = "(VlanMapConfig)" + vlconf.toString();
            VlanMap map = null;

            VlanMap getmap = null;
            try {
                getmap = mgr.getVlanMap(vbpath, "0");
                fail("throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(getmap);

            try {
                getmap = mgr.getVlanMap(vbpath, vlconf);
                fail("throwing Exception was expected.");
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(getmap);

            st = mgr.removeVlanMap(vbpath, "0");
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());

            if (!vbpath.equals(bpath)) {
                List<VlanMap> list = null;
                try {
                    list = mgr.getVlanMaps(vbpath);
                    fail("throwing Exception was expected.");
                } catch (VTNException e) {
                    assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
                } catch (Exception e) {
                    unexpected(e);
                }
                assertNull(list);

                try {
                    map = mgr.getVlanMap(bpath, vlconf);
                } catch (Exception e) {
                    unexpected(e);
                }

                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }

            try {
                map = mgr.addVlanMap(vbpath, vlconf);
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
        }

        // test BADREQUEST or NOTFOUND
        List<VBridgePath> listBpath2 = new ArrayList<VBridgePath>();
        listBpath2.add(null);
        listBpath2.add(bpath);
        listBpath2.add(new VBridgePath((String)null, bname));
        listBpath2.add(new VBridgePath(tname, null));
        listBpath2.add(new VBridgePath("dummy", null));
        listBpath2.add(new VBridgePath((String)null, "dummy"));
        listBpath2.add(new VBridgePath("dummy", "dummy"));

        Node node = NodeCreator.createOFNode(0L);
        List<VlanMapConfig> listVLanMapConf = new ArrayList<VlanMapConfig>();
        listVLanMapConf.add(null);
        listVLanMapConf.add(new VlanMapConfig(node, (short)0));

        VlanMap map = null;
        try {
            map = mgr.addVlanMap(bpath, listVLanMapConf.get(1));
        } catch (Exception e) {
            unexpected(e);
        }

        for (VBridgePath vbpath : listBpath2) {
            String emsg = "(bpath)" + ((vbpath == null) ? "(null)" : vbpath.toString());

            for (VlanMapConfig vlconf : listVLanMapConf) {

                String emsgVL = emsg
                        + "(VlanMapConfig)" + ((vlconf == null) ? "(null)" : vlconf.toString());

                boolean exStatus = ((vbpath == null)
                        || (tname.equals(vbpath.getTenantName()) && vbpath.getBridgeName() == null)
                        || (bpath.equals(vbpath) && vlconf == null));

                VlanMap getmap = null;
                try {
                    map = mgr.addVlanMap(vbpath, vlconf);
                    fail("throwing Exception was expected.");
                } catch (VTNException e) {
                    if (bpath.equals(vbpath) && (vlconf != null)) {
                        assertEquals(emsgVL, StatusCode.CONFLICT, e.getStatus().getCode());
                    } else {
                        checkExceptionStatus(e.getStatus(), exStatus, emsgVL);
                    }
                } catch (Exception e) {
                    unexpected(e);
                }

                if (!bpath.equals(vbpath)) {
                    try {
                        getmap = mgr.getVlanMap(vbpath, map.getId());
                        fail("throwing Exception was expected.");
                    } catch (VTNException e) {
                        checkExceptionStatus(e.getStatus(), exStatus, emsgVL);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    assertNull(emsgVL, getmap);
                }

                if (!bpath.equals(vbpath) || (vlconf == null)) {
                    try {
                        getmap = mgr.getVlanMap(vbpath, vlconf);
                        fail("throwing Exception was expected.");
                    } catch (VTNException e) {
                        checkExceptionStatus(e.getStatus(), exStatus, emsgVL);
                    } catch (Exception e) {
                        unexpected(e);
                    }
                    assertNull(getmap);
                }

                if (bpath.equals(vbpath)) {
                    continue;
                }

                st = mgr.removeVlanMap(vbpath, map.getId());
                checkExceptionStatus(st, exStatus, emsgVL);

                List<VlanMap> list = null;
                try {
                    list = mgr.getVlanMaps(vbpath);
                    fail("throwing Exception was expected.");
                } catch (VTNException e) {
                    checkExceptionStatus(e.getStatus(), exStatus, emsgVL);
                } catch (Exception e) {
                    unexpected(e);
                }
                assertNull(emsgVL, list);
            }
        }

        // test for null condition of mapId
        try {
            VlanMap getmap = mgr.getVlanMap(bpath, (String)null);
            fail("throwing Exception was expected.");
        } catch (VTNException e) {
            assertEquals(StatusCode.BADREQUEST, e.getStatus().getCode());
        }

        st = mgr.removeVlanMap(bpath, null);
        assertEquals(StatusCode.BADREQUEST, st.getCode());

        // Clear up
        st = mgr.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    private void checkExceptionStatus(Status st, boolean reqbad, String emsg) {
        if (reqbad) {
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
        } else {
            try {
                assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
            } catch (AssertionError e) {
                assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
            }
        }
    }

    /**
     * Test method for {@link IVTNManager#getPortMap(VBridgeIfPath)} and
     * {@link IVTNManager#setPortMap(VBridgeIfPath, PortMapConfig)}.
     */
    private void testPortMap() {
        LOG.info("Running testPortMap().");

        IVTNManager mgr = this.vtnManager;
        short[] vlans = new short[] {
            -1, 0, 1, 10, 100, 1000, 4094, 4095, 4096
        };

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
        st = mgr.addInterface(ifp, ifconf);
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
            new SwitchPort("port-10",
                           NodeConnector.NodeConnectorIDType.OPENFLOW, "10"),
            new SwitchPort(null,
                           NodeConnector.NodeConnectorIDType.OPENFLOW, "11"),
            new SwitchPort("port-10", null, null),
            new SwitchPort("port-10"),
            new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "13"),
        };

        st = mgr.setPortMap(ifp, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        PortMap map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(map);

        for (SwitchPort port: ports) {
            for (short vlan : vlans) {
                PortMapConfig pmconf = new PortMapConfig(node, port, vlan);
                String emsg = "(PortMapConfig)" + pmconf.toString();
                boolean isValidVLANid = (0 <= vlan && vlan < 4096);

                st = mgr.setPortMap(ifp, pmconf);
                if (isValidVLANid) {
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                } else {
                    assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                    continue;
                }

                try {
                    map = mgr.getPortMap(ifp);
                } catch (Exception e) {
                    unexpected(e);
                }
                if (isValidVLANid) {
                    assertEquals(emsg, pmconf, map.getConfig());
                    assertNull(emsg, map.getNodeConnector());
                }

                VInterface bif = null;
                try {
                    bif = mgr.getInterface(ifp);
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
        st = mgr.addInterface(ifp1, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        String ifname2 = "vinterface2";
        VBridgeIfPath ifp2 = new VBridgeIfPath(tname, bname1, ifname2);
        st = mgr.addInterface(ifp2, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        Node node1 = NodeCreator.createOFNode(0L);
        SwitchPort port1 = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf1 = new PortMapConfig(node1, port1, (short)0);
        st = mgr.setPortMap(ifp1, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        SwitchPort port2 = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "11");
        PortMapConfig pmconf2 = new PortMapConfig(node1, port2, (short)0);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // if specified port is not exist, duplicate portmap success.
        String ifname3 = "vinterface3";
        VBridgeIfPath ifp3 = new VBridgeIfPath(tname, bname2, ifname3);
        st = mgr.addInterface(ifp3, new VInterfaceConfig(null, Boolean.TRUE));
        st = mgr.setPortMap(ifp3, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Exception test for VBridgeInterfacePath
        st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.addBridge(new VBridgePath(tname, bname), new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        node = NodeCreator.createOFNode(0L);
        SwitchPort port = new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, "10");
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);

        List<VBridgeIfPath> listBIfPath1 = new ArrayList<VBridgeIfPath>();
        listBIfPath1.add(null);
        listBIfPath1.add(new VBridgeIfPath(null, bname, ifname));
        listBIfPath1.add(new VBridgeIfPath(tname, null, ifname));
        listBIfPath1.add(new VBridgeIfPath(tname, bname, null));

        for (VBridgeIfPath vifpath: listBIfPath1) {
            String emsg = "(PortMapConfig)" + pmconf.toString() + "(vif)" +
                ((vifpath == null) ? "(null)" : vifpath.toString());

            map = null;
            try {
                map = mgr.getPortMap(vifpath);
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.BADREQUEST, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(map);

            st = mgr.setPortMap(vifpath, pmconf);
            assertEquals(StatusCode.BADREQUEST, st.getCode());
        }

        List<VBridgeIfPath> listBIfPath2 = new ArrayList<VBridgeIfPath>();
        listBIfPath2.add(new VBridgeIfPath("dummy", bname, ifname));
        listBIfPath2.add(new VBridgeIfPath(tname, "dummy", ifname));

        for (VBridgeIfPath vifpath: listBIfPath2) {
            String emsg = "(PortMapConfig)" + pmconf.toString() + "(vif)" +
                vifpath.toString();

            map = null;
            try {
                map = mgr.getPortMap(vifpath);
            } catch (VTNException e) {
                assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(map);

            st = mgr.setPortMap(vifpath, pmconf);
            assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
        }

        List<VBridgeIfPath> listBIfPath3 = new ArrayList<VBridgeIfPath>();
        listBIfPath3.add(new VBridgeIfPath("_tenant", bname, null));
        listBIfPath3.add(new VBridgeIfPath(null, "@bname", ifname));
        listBIfPath3.add(new VBridgeIfPath(tname, null, "!ifname"));

        for (VBridgeIfPath vifpath: listBIfPath3) {
            String emsg = "(PortMapConfig)" + pmconf.toString() + "(vif)" +
                vifpath.toString();

            map = null;
            try {
                map = mgr.getPortMap(vifpath);
                fail("throwing Exception was expected.");
            } catch (VTNException e) {
                checkExceptionStatus(e.getStatus(), false, emsg);
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(map);

            st = mgr.setPortMap(vifpath, pmconf);
            checkExceptionStatus(st, false, emsg);
        }

        // Exception test for portmap config
        ifp = new VBridgeIfPath(tname, bname, ifname);
        st = mgr.addInterface(ifp, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        Node nodePR = null;
        try {
            nodePR = new Node(Node.NodeIDType.PRODUCTION, "nodeId");
        } catch (ConstructionException e) {
            unexpected(e);
        }

        List<PortMapConfig> listPMAP = new ArrayList<PortMapConfig>();
        listPMAP.add(new PortMapConfig(null, port, (short)0));
        listPMAP.add(new PortMapConfig(nodePR, port, (short)0));
        listPMAP.add(new PortMapConfig(node, null, (short)0));
        listPMAP.add(new PortMapConfig(node, new SwitchPort(null, null, "10"),
                                       (short)0));
        listPMAP.add(new PortMapConfig(node, new SwitchPort(null, NodeConnector.NodeConnectorIDType.OPENFLOW, null),
                (short)0));
        for (PortMapConfig pcon: listPMAP) {
            String emsg = "(PortMapConfig)" + pcon.toString();
            st = mgr.setPortMap(ifp, pcon);
            assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
        }

        // specified port map to null
        st = mgr.setPortMap(ifp, null);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertNull(map);

        // switchport name is not null, switch port id and type are null
        pmconf = new PortMapConfig(node, new SwitchPort("port-0", null, null), (short)0);

        st = mgr.setPortMap(ifp, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertEquals(pmconf, map.getConfig());

        // switchport name is null, switch port id and type are not null
        pmconf = new PortMapConfig(node, new SwitchPort(
                null, NodeConnector.NodeConnectorIDType.OPENFLOW, "10"), (short)0);
        st = mgr.setPortMap(ifp, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        map = null;
        try {
            map = mgr.getPortMap(ifp);
        } catch (Exception e) {
            unexpected(e);
        }
        assertEquals(pmconf, map.getConfig());

        // CONFLICT TEST
        st = mgr.addBridge(new VBridgePath(tname, bname1), new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        ifp1 = new VBridgeIfPath(tname, bname1, ifname1);
        st = mgr.addInterface(ifp1, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.addInterface(ifp2, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        try {
            VInterface vif = mgr.getInterface(ifp1);
            assertEquals(VNodeState.UNKNOWN, vif.getState());
        } catch (VTNException e) {
            unexpected(e);
        }
        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        assertNotNull(r);
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));
        assertNotNull(swmgr);

        IPluginInInventoryService pinIVS =
            getStubService(IPluginInInventoryService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinIVS instanceof InventoryService);

        List<Node> existNodes = new ArrayList<Node>();
        existNodes.addAll(swmgr.getNodes());
        assertEquals(3, existNodes.size());
        List<NodeConnector> existConnectors = new ArrayList<NodeConnector>();
        for (Node eNode: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(eNode));
        }
        assertEquals(3, existConnectors.size());

        Node node2 = existNodes.get(0);

        assertTrue(existConnectors.get(0).getID() instanceof Short);
        String portId = ((Short)existConnectors.get(0).getID()).toString();

        pmconf1 = new PortMapConfig(node2, new SwitchPort(
                null, NodeConnector.NodeConnectorIDType.OPENFLOW, portId), (short)10);
        st = mgr.setPortMap(ifp1, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        try {
            VInterface vif = mgr.getInterface(ifp1);
            assertEquals(VNodeState.UP, vif.getState());

            VBridge vbr = mgr.getBridge(new VBridgePath(tname, bname1));
            assertEquals(VNodeState.UP, vbr.getState());
        } catch (VTNException e) {
            unexpected(e);
        }

        pmconf2 = new PortMapConfig(node2,  new SwitchPort(
                null, NodeConnector.NodeConnectorIDType.OPENFLOW, portId), (short)10);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        Name portName = (Name)pinIVS.getNodeConnectorProps(Boolean.FALSE)
                .get(existConnectors.get(0)).get(Name.NamePropName);
        PortMapConfig pmconf3 = new PortMapConfig(node2, new SwitchPort(
                portName.getStringValue(), null, null), (short)10);
        st = mgr.setPortMap(ifp2, pmconf3);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        // clear up
        st = mgr.removeInterface(ifp1);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.removeInterface(ifp2);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeBridge(new VBridgePath(tname, bname1));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test case for {@link IVTNManager#probeHost(HostNodeConnector)}.
     */
    private void testProbeHost() {
        LOG.info("Running testProbeHost().");

        IVTNManager mgr = this.vtnManager;
        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));

        IPluginInDataPacketService pinDPS =
            getStubService(IPluginInDataPacketService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinDPS instanceof DataPacketServices);
        DataPacketServices dps = (DataPacketServices)pinDPS;

        short[] vlans = {0, 10, 4095};

        // Stub emulates 3 nodes and also emulates 1 node connector on each node.
        List<Node> existNodes = new ArrayList<Node>();
        existNodes.addAll(swmgr.getNodes());
        assertEquals(3, existNodes.size());
        List<NodeConnector> existConnectors = new ArrayList<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }
        assertEquals(3, existConnectors.size());

        byte [] mac = new byte [] {
            (byte)0x00, (byte)0x00, (byte)0x00,
            (byte)0x12, (byte)0x34, (byte)0x56
        };
        InetAddress ip4addr = null;
        InetAddress ip6addr = null;
        try {
            byte[] addr = {(byte)192, (byte)168, (byte)254, (byte)1};
            ip4addr = InetAddress.getByAddress(addr);

            addr = new byte[]{
                (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                (byte)0xe1, (byte)0x23, (byte)0xe6, (byte)0x88,
                (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0
            };
            ip6addr = InetAddress.getByAddress(addr);
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        // Wait 3000 (+ 100) ms to be removed from "disabled nodes".
        // (Default value of "disabled" time is 3000 ms.)
        sleep(3000 + 100);

        // Tests for invalid conditions
        HostNodeConnector hnc = null;
        String emsg;

        // null condition
        emsg = "(host)(null)";
        dps.clearPkt();
        CountDownLatch latch = dps.setLatch(1);
        // In this case, packet is NOT sent.
        assertFalse(emsg, mgr.probeHost(hnc));
        try {
            assertFalse(emsg,
                        latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
        } catch (InterruptedException e) {
            unexpected(e);
        }
        assertEquals(emsg, 0, dps.getPktCount());

        // No node connector
        try {
            hnc = new HostNodeConnector(mac, ip4addr, null, (short)0);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        emsg = "(host)" + hnc.toString();
        dps.clearPkt();
        latch = dps.setLatch(1);
        // In this case, packet is NOT sent.
        assertFalse(emsg, mgr.probeHost(hnc));
        try {
            assertFalse(emsg,
                        latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
        } catch (InterruptedException e) {
            unexpected(e);
        }
        assertEquals(emsg, 0, dps.getPktCount());

        // vBridge not available
        for (int i = 0; i < existConnectors.size(); i++) {
            try {
                hnc = new HostNodeConnector(mac, ip4addr, existConnectors.get(i), vlans[i]);
            } catch (ConstructionException e) {
                unexpected(e);
            }
            emsg = "(host)" + hnc.toString();

            dps.clearPkt();
            latch = dps.setLatch(1);
            // In this case, packet is NOT sent.
            assertFalse(emsg, mgr.probeHost(hnc));
            try {
                assertFalse(emsg,
                            latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }
            assertEquals(emsg, 0, dps.getPktCount());
        }

        // Make VTN
        VTenantPath tpath = new VTenantPath("vtn");
        Status st = mgr.addTenant(tpath, new VTenantConfig("for Test"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Make vBridges
        List<VBridgePath> listBPath = new ArrayList<VBridgePath>();
        listBPath.add(new VBridgePath(tpath, "vbridge1"));
        listBPath.add(new VBridgePath(tpath, "vbridge2"));
        listBPath.add(new VBridgePath(tpath, "vbridge3"));

        // Make vBridge Interfaces and set a port mapping.
        List<VBridgeIfPath> listBIfPath = new ArrayList<VBridgeIfPath>();
        for (int i = 0; i < listBPath.size(); i++) {
            VBridgePath bpath = listBPath.get(i);
            st = mgr.addBridge(bpath, new VBridgeConfig(null, 10));
            assertEquals(StatusCode.SUCCESS, st.getCode());

            VBridgeIfPath vifpath = new VBridgeIfPath(bpath, "vif");
            st = mgr.addInterface(vifpath, new VInterfaceConfig(null, Boolean.TRUE));
            listBIfPath.add(vifpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());

            assertTrue(existConnectors.get(i).getID() instanceof Short);
            String ncID = ((Short)existConnectors.get(i).getID()).toString();
            SwitchPort swport =
                    new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, ncID);
            st = mgr.setPortMap(vifpath, new PortMapConfig(existNodes.get(i), swport, vlans[i]));
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // Tests for normal conditions
        for (int i = 0; i < existConnectors.size(); i++) {
            try {
                hnc = new HostNodeConnector(mac, ip4addr, existConnectors.get(i), vlans[i]);
            } catch (ConstructionException e) {
                unexpected(e);
            }
            emsg = "(host)" + hnc.toString();

            dps.clearPkt();
            latch = dps.setLatch(1);
            assertTrue(emsg, mgr.probeHost(hnc));
            try {
                assertTrue(emsg, latch.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }
            assertEquals(emsg, 1, dps.getPktCount());
            assertEquals(emsg, existConnectors.get(i), dps.getPacket().getOutgoingNodeConnector());
        }

        // Test for port not found
        NodeConnector ncDead = null;
        Short idDead = new Short((short)0xDEAD);
        try {
            ncDead = new NodeConnector(NodeConnector.NodeConnectorIDType.OPENFLOW, idDead, existNodes.get(0));
        } catch (ConstructionException e) {
            unexpected(e);
        }
        SwitchPort swport =
                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, idDead.toString());
        VBridgeIfPath vifDead =
                new VBridgeIfPath(listBPath.get(0), "vifDead");
        st = mgr.addInterface(vifDead, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.setPortMap(vifDead, new PortMapConfig(existNodes.get(0), swport, vlans[0]));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        listBIfPath.add(vifDead);

        try {
            hnc = new HostNodeConnector(mac, ip4addr, ncDead, vlans[0]);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        emsg = "(host)" + hnc.toString();

        dps.clearPkt();
        latch = dps.setLatch(1);
        assertFalse(emsg, mgr.probeHost(hnc));
        try {
            assertFalse(emsg,
                        latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
        } catch (InterruptedException e) {
            unexpected(e);
        }
        assertEquals(emsg, 0, dps.getPktCount());

        // Clear up
        for (VBridgeIfPath vifpath : listBIfPath) {
            st = mgr.removeInterface(vifpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
        for (VBridgePath bpath : listBPath) {
            st = mgr.removeBridge(bpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test case for {@link IVTNManager#findHost(InetAddress, Set)}.
     */
    private void testFindHost() {
        LOG.info("Running testFindHost().");

        IVTNManager mgr = this.vtnManager;
        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));

        IPluginInDataPacketService pinDPS =
            getStubService(IPluginInDataPacketService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinDPS instanceof DataPacketServices);
        DataPacketServices dps = (DataPacketServices)pinDPS;

        short[] vlans = {0, 10, 4095};

        // Stub emulates 3 nodes and also emulates 1 node connector on each node.
        List<Node> existNodes = new ArrayList<Node>();
        existNodes.addAll(swmgr.getNodes());
        assertEquals(3, existNodes.size());
        List<NodeConnector> existConnectors = new ArrayList<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }
        assertEquals(3, existConnectors.size());

        // Make VTN
        VTenantPath tpath = new VTenantPath("vtn");
        Status st = mgr.addTenant(tpath, new VTenantConfig("for Test"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Make vBridges
        List<VBridgePath> listBPath = new ArrayList<VBridgePath>();
        listBPath.add(new VBridgePath(tpath, "vbridge1"));
        listBPath.add(new VBridgePath(tpath, "vbridge2"));
        listBPath.add(new VBridgePath(tpath, "vbridge3"));

        // Make vBridge Interfaces and set a port mapping.
        List<VBridgeIfPath> listBIfPath = new ArrayList<VBridgeIfPath>();
        int i = 0;
        for (VBridgePath bpath : listBPath) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null, 10));
            assertEquals(StatusCode.SUCCESS, st.getCode());

            VBridgeIfPath vifpath = new VBridgeIfPath(bpath, "vif");
            st = mgr.addInterface(vifpath, new VInterfaceConfig(null, Boolean.TRUE));
            listBIfPath.add(vifpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());

            assertTrue(existConnectors.get(i).getID() instanceof Short);
            String ncID = ((Short)existConnectors.get(i).getID()).toString();
            SwitchPort swport =
                    new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, ncID);
            st = mgr.setPortMap(vifpath, new PortMapConfig(existNodes.get(i), swport, vlans[i]));
            assertEquals(StatusCode.SUCCESS, st.getCode());
            i++;
        }

        InetAddress ip4addr = null;
        InetAddress ip6addr = null;
        try {
            byte[] addr = {
                (byte)192, (byte)168, (byte)254, (byte)1
            };
            ip4addr = InetAddress.getByAddress(addr);

            addr = new byte[]{
                (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                (byte)0xe1, (byte)0x23, (byte)0xe6, (byte)0x88,
                (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0,
            };
            ip6addr = InetAddress.getByAddress(addr);
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        // Tests
        i = 0;
        for (VBridgePath bpath : listBPath) {
            Set<VBridgePath> setBPath = new HashSet<VBridgePath>();
            setBPath.add(bpath);

            String emsg = "(bpath)" + bpath.toString();
            // for IPv4 Address
            dps.clearPkt();
            CountDownLatch latch = dps.setLatch(setBPath.size());
            mgr.findHost(ip4addr, setBPath);
            try {
                assertTrue(emsg, latch.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }
            assertEquals(emsg, setBPath.size(), dps.getPktCount());
            assertEquals(emsg, existConnectors.get(i), dps.getPacket().getOutgoingNodeConnector());

            // for IPv6 Address
            dps.clearPkt();
            latch = dps.setLatch(setBPath.size());
            mgr.findHost(ip6addr, setBPath);
            try {
                // In this case, No packet is sent.
                assertFalse(emsg,
                            latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }
            assertEquals(emsg, 0, dps.getPktCount());

            // for null condition
            dps.clearPkt();
            latch = dps.setLatch(setBPath.size());
            mgr.findHost(null, setBPath);
            try {
                // In this case, No packet is sent.
                assertFalse(emsg,
                            latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }
            assertEquals(emsg, 0, dps.getPktCount());

            i++;
        }

        // Multiple BPathes
        Set<VBridgePath> setBPath = new HashSet<VBridgePath>();
        setBPath.addAll(listBPath);

        dps.clearPkt();
        CountDownLatch latch = dps.setLatch(setBPath.size());
        mgr.findHost(ip4addr, setBPath);
        try {
            assertTrue("all bpathes",
                       latch.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        } catch (InterruptedException e) {
            unexpected(e);
        }
        assertEquals("all bpathes", setBPath.size(), dps.getPktCount());

        // null condition
        dps.clearPkt();
        latch = dps.setLatch(listBPath.size());
        mgr.findHost(ip4addr, null);
        try {
            assertTrue("bpath is null",
                       latch.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        } catch (InterruptedException e) {
            unexpected(e);
        }
        assertEquals("bpath is null", listBPath.size(), dps.getPktCount());

        // Invalid port
        Node nodeDead = NodeCreator.createOFNode(221L);
        Short ncIDDead = new Short((short)221);
        NodeConnector ncDead =
                NodeConnectorCreator.createNodeConnector(NodeConnectorIDType.OPENFLOW, ncIDDead, nodeDead);
        VBridgePath vbrDead = new VBridgePath(tpath, "vbrDead");
        st = mgr.addBridge(vbrDead, new VBridgeConfig(null, 10));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        listBPath.add(vbrDead);
        Set<VBridgePath> setDead = new HashSet<VBridgePath>();
        setDead.add(vbrDead);

        VBridgeIfPath vifDead = new VBridgeIfPath(vbrDead, "vifDead");
        st = mgr.addInterface(vifDead, new VInterfaceConfig(null, Boolean.TRUE));
        listBIfPath.add(vifDead);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        SwitchPort swportDead =
                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, ncIDDead.toString());
        st = mgr.setPortMap(vifDead, new PortMapConfig(nodeDead, swportDead, (short)221));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        dps.clearPkt();
        latch = dps.setLatch(1);
        mgr.findHost(ip4addr, setDead);
        try {
            // In this case, No packet is sent.
            assertFalse("invalid port",
                        latch.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
        } catch (InterruptedException e) {
            unexpected(e);
        }
        assertEquals("invalid port", 0, dps.getPktCount());

        // Clear up
        for (VBridgeIfPath vifpath : listBIfPath) {
            st = mgr.removeInterface(vifpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
        for (VBridgePath bpath : listBPath) {
            st = mgr.removeBridge(bpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test case for {@link IVTNGlobal}.
     */
    private void testIVTNGlobal() {
        LOG.info("Running testIVTNGlobal().");

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
        VTNManagerAwareData<VTenantPath, VTenant> vtnChangedInfo;
        VTNManagerAwareData<VBridgePath, VBridge> vbrChangedInfo;
        VTNManagerAwareData<VBridgeIfPath, VInterface> vIfChangedInfo;
        VTNManagerAwareData<VBridgePath, VlanMap> vlanMapChangedInfo;
        VTNManagerAwareData<VBridgeIfPath, PortMap> portMapChangedInfo;
        VTNManagerAwareData<VBridgePath, MacMapConfig> macMapChangedInfo;
        VTNManagerAwareData<VTerminalPath, VTerminal> vtermChangedInfo;
        VTNManagerAwareData<VTerminalIfPath, VInterface> vtermIfChangedInfo;
        VTNManagerAwareData<VTerminalIfPath, PortMap> vtermPortMapChangedInfo;

        Update(UpdateType t, VTenantPath path, VTenant vtenant) {
            vtnChangedInfo =
                new VTNManagerAwareData<VTenantPath, VTenant>(path, vtenant, t);
        }

        Update(UpdateType t, VBridgePath path, VBridge vbridge) {
            vbrChangedInfo =
                new VTNManagerAwareData<VBridgePath, VBridge>(path, vbridge, t);
        }

        Update(UpdateType t, VBridgeIfPath path, VInterface iface) {
            vIfChangedInfo =
                new VTNManagerAwareData<VBridgeIfPath, VInterface>(path, iface,
                                                                   t);
        }

        Update(UpdateType t, VBridgePath path, VlanMap vlmap) {
            vlanMapChangedInfo =
                new VTNManagerAwareData<VBridgePath, VlanMap>(path, vlmap, t);
        }

        Update(UpdateType t, VBridgeIfPath path, PortMap pmap) {
            portMapChangedInfo =
                new VTNManagerAwareData<VBridgeIfPath, PortMap>(path, pmap, t);
        }

        Update(UpdateType t, VBridgePath path, MacMapConfig mcconf) {
            macMapChangedInfo =
                new VTNManagerAwareData<VBridgePath, MacMapConfig>(path, mcconf,
                                                                   t);
        }

        Update(UpdateType t, VTerminalPath path, VTerminal vterm) {
            vtermChangedInfo =
                new VTNManagerAwareData<VTerminalPath, VTerminal>(
                    path, vterm, t);
        }

        Update(UpdateType t, VTerminalIfPath path, VInterface iface) {
            vtermIfChangedInfo =
                new VTNManagerAwareData<VTerminalIfPath, VInterface>(
                    path, iface, t);
        }

        Update(UpdateType t, VTerminalIfPath path, PortMap pmap) {
            vtermPortMapChangedInfo =
                new VTNManagerAwareData<VTerminalIfPath, PortMap>(path, pmap,
                                                                  t);
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
            return this.gotUpdates;
        }

        @Override
        public void vtnChanged(VTenantPath path, VTenant vtenant, UpdateType type) {
            LOG.debug("VTNManager[{}] Got a vtn changed for path:{} object:{}", path, vtenant);
            Update u = new Update(type, path, vtenant);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vBridgeChanged(VBridgePath path, VBridge vbridge, UpdateType type) {
            LOG.debug("VTNManager[{}] Got a vbridge changed for path:{} object:{}", path, vbridge);
            Update u = new Update(type, path, vbridge);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vInterfaceChanged(VBridgeIfPath path, VInterface viface, UpdateType type) {
            LOG.debug("VTNManager[{}] Got a vbridge interface changed for path:{} object:{}", path, viface);
            Update u = new Update(type, path, viface);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vlanMapChanged(VBridgePath path, VlanMap vlmap, UpdateType type) {
            LOG.debug("VTNManager[{}] Got a vlan map changed for path:{} object:{}", path, vlmap);
            Update u = new Update(type, path, vlmap);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void portMapChanged(VBridgeIfPath path, PortMap pmap, UpdateType type) {
            LOG.debug("VTNManager[{}] Got a port map(vBridge) changed for " +
                      "path:{} object:{}", path, pmap);
            Update u = new Update(type, path, pmap);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void macMapChanged(VBridgePath path, MacMapConfig mcconf,
                                  UpdateType type) {
            LOG.debug("VTNManager[{}] Got a MAC map changed for path: {} " +
                      "object {}", path, mcconf);
            Update u = new Update(type, path, mcconf);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                                     UpdateType type) {
            LOG.debug("VTNManager[{}] Got a vTerminal changed for path:{} " +
                      "object:{}", path, vterm);
            Update u = new Update(type, path, vterm);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void vInterfaceChanged(VTerminalIfPath path, VInterface viface, UpdateType type) {
            LOG.debug("VTNManager[{}] Got a vTerminal interface changed " +
                      "for path:{} object:{}", path, viface);
            Update u = new Update(type, path, viface);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }

        @Override
        public void portMapChanged(VTerminalIfPath path, PortMap pmap,
                                   UpdateType type) {
            LOG.debug("VTNManager[{}] Got a port map(vTerminal) changed for " +
                      "path:{} object:{}", path, pmap);
            Update u = new Update(type, path, pmap);
            this.gotUpdates.add(u);
            if (latch != null) {
                this.latch.countDown();
            }
        }
    }

    /**
     * An internal class which wait for the specified edge.
     */
    private class EdgeWaiter implements IListenRoutingUpdates {
        /**
         * The number of milliseconds to wait for the completion of
         * shortest path recalculation.
         */
        private static final long  RECALC_TIMEOUT = 10000L;

        /**
         * {@link IRouting} service.
         */
        private final IRouting  routing;

        /**
         * Construct a new instance.
         */
        private EdgeWaiter() {
            routing = (IRouting)ServiceHelper.
                getInstance(IRouting.class, GlobalConstants.DEFAULT.toString(),
                            VTNManagerIT.this);
            assertNotNull(routing);
        }

        /**
         * Wait for the specified edge to be established.
         *
         * @param src  The source node.
         * @param dst  The destination node.
         */
        private void await(Node src, Node dst) {
            if (routing.getRoute(src, dst) != null) {
                return;
            }

            // Register routing listener.
            Dictionary<String, Object> props = new Hashtable<String, Object>();
            ServiceRegistration reg = ServiceHelper.
                registerServiceWReg(IListenRoutingUpdates.class,
                                    GlobalConstants.DEFAULT.toString(), this,
                                    props);
            assertNotNull(reg);

            try {
                long timeout = RECALC_TIMEOUT;
                long limit = System.currentTimeMillis() + timeout;
                synchronized (this) {
                    Path path;
                    while ((path = routing.getRoute(src, dst)) == null) {
                        assertTrue(timeout > 0);
                        LOG.trace("Waiting for the edge from {} to {}.",
                                  src, dst);
                        wait(timeout);

                        long cur = System.currentTimeMillis();
                        timeout = limit - cur;
                    }
                    LOG.trace("A required edge has been established: {}", path);
                }
            } catch (Exception e) {
                TestBase.unexpected(e);
            } finally {
                reg.unregister();
            }
        }

        /**
         * Invoked when the recalculation of the shortest path tree is done.
         */
        @Override
        public synchronized void recalculateDone() {
            notify();
        }
    }

    /**
     * An adapter class for {@link IVTNManagerAware}.
     */
    protected class VTNManagerAwareAdapter implements IVTNManagerAware {
        /**
         * Invoked when the information related to the VTN is changed.
         *
         * @param path     A {@link VTenantPath} object that specifies the
         *                 position of the VTN.
         * @param vtenant  A {@link VTenant} object which represents the VTN
         *                 information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void vtnChanged(VTenantPath path, VTenant vtenant,
                               UpdateType type) {
        }

        /**
         * Invoked when the information related to the vBridge is changed.
         *
         * @param path     A {@link VBridgePath} object that specifies the
         *                 position of the vBridge.
         * @param vbridge  A {@link VBridge} object which represents the
         *                 vBridge information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                                   UpdateType type) {
        }

        /**
         * Invoked when the information related to vTerminal inside the
         * container is changed.
         *
         * @param path   A {@link VTerminalPath} object that specifies the
         *               position of the vTerminal.
         * @param vterm  A {@link VTerminal} object which represents the
         *               vTerminal information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void vTerminalChanged(VTerminalPath path, VTerminal vterm,
                                     UpdateType type) {
        }

        /**
         * Invoked when the information related to the virtual interface
         * configured in the vBridge is changed.
         *
         * @param path    A {@link VBridgeIfPath} object that specifies the
         *                position of the vBridge interface.
         * @param viface  A {@link VInterface} object which represents the
         *                vBridge interface information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void vInterfaceChanged(VBridgeIfPath path, VInterface viface,
                                      UpdateType type) {
        }

        /**
         * Invoked when the information related to the virtual interface
         * configured in the vTerminal is changed.
         *
         * @param path    A {@link VTerminalIfPath} object that specifies the
         *                position of the vBridge interface.
         * @param viface  A {@link VInterface} object which represents the
         *                vTerminal interface information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void vInterfaceChanged(VTerminalIfPath path, VInterface viface,
                                      UpdateType type) {
        }

        /**
         * Invoked when the information related to the VLAN mapping
         * configured in the vBridge is changed.
         *
         * @param path   A {@link VBridgePath} object that specifies the
         *               position of the VBridge.
         * @param vlmap  A {@link VlanMap} object which represents the VLAN
         *               mapping information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void vlanMapChanged(VBridgePath path, VlanMap vlmap,
                                   UpdateType type) {
        }

        /**
         * Invoked when the information related to the port mapping
         * configured in the vBridge interface is changed.
         *
         * @param path  A {@link VBridgeIfPath} object that specifies the
         *              position of the vBridge interface.
         * @param pmap  A {@link PortMap} object which represents the
         *              port mapping information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void portMapChanged(VBridgeIfPath path, PortMap pmap,
                                   UpdateType type) {
        }

        /**
         * Invoked when the information related to the port mapping
         * configured in the vTerminal interface is changed.
         *
         * @param path  A {@link VTerminalIfPath} object that specifies the
         *              position of the vTerminal interface.
         * @param pmap  A {@link PortMap} object which represents the
         *              port mapping information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void portMapChanged(VTerminalIfPath path, PortMap pmap,
                                   UpdateType type) {
        }

        /**
         * Invoked when the information related to the MAC mapping configured
         * in vBridge is changed.
         *
         * @param path    A {@link VBridgePath} object that specifies the
         *                position of the VBridge.
         * @param mcconf  A {@link MacMapConfig} object which represents
         *                the MAC mapping configuration information.
         * @param type
         *   An {@link UpdateType} object which indicates the type of
         *   modification is specified.
         */
        @Override
        public void macMapChanged(VBridgePath path, MacMapConfig mcconf,
                                  UpdateType type) {
        }
    }

    /**
     * An internal class which wait for the vBridge state to be changed.
     */
    private class VBridgeStateWaiter extends VTNManagerAwareAdapter {
        /**
         * The number of milliseconds to wait the vBridge state to be changed.
         */
        private static final long  WAIT_TIMEOUT = 10000L;

        /**
         * The target vBridge.
         */
        private final VBridgePath  bridgePath;

        /**
         * Expected state of the vBridge.
         */
        private final VNodeState  expectedState;

        /**
         * Expected fault count.
         */
        private final int  expectedFaults;

        /**
         * Set {@code true} if the state of the target vBridge is changed
         * as expected.
         */
        private volatile boolean  changed;

        /**
         * Construct a new instance.
         *
         * @param path   A path to the target vBridge.
         * @param state  An expected state of the vBridge.
         */
        private VBridgeStateWaiter(VBridgePath path, VNodeState state) {
            this(path, state, -1);
        }

        /**
         * Construct a new instance.
         *
         * @param path    A path to the target vBridge.
         * @param state   An expected state of the vBridge.
         * @param faults  An expected value of the path fault count.
         *                A negative value means that the path fault count
         *                should not be observed.
         */
        private VBridgeStateWaiter(VBridgePath path, VNodeState state,
                                   int faults) {
            bridgePath = path;
            expectedState = state;
            expectedFaults = faults;
        }

        /**
         * Wait for the vBridge state to be changed as expected.
         */
        private void await() {
            try {
                VBridge vbridge = vtnManager.getBridge(bridgePath);
                if (isExpected(vbridge)) {
                    return;
                }
            } catch (Exception e) {
                TestBase.unexpected(e);
            }

            // Register VTN listener.
            Dictionary<String, Object> props = new Hashtable<String, Object>();
            ServiceRegistration reg = ServiceHelper.
                registerServiceWReg(IVTNManagerAware.class,
                                    GlobalConstants.DEFAULT.toString(), this,
                                    props);
            assertNotNull(reg);

            try {
                long timeout = WAIT_TIMEOUT;
                long limit = System.currentTimeMillis() + timeout;
                synchronized (this) {
                    VBridge vbridge = vtnManager.getBridge(bridgePath);
                    if (isExpected(vbridge)) {
                        return;
                    }

                    while (!changed) {
                        assertTrue(timeout > 0);
                        LOG.trace("Waiting for the vBridge state to be " +
                                  "changed: path={}, state={}, faults={}",
                                  bridgePath, expectedState, expectedFaults);
                        wait(timeout);

                        long cur = System.currentTimeMillis();
                        timeout = limit - cur;
                    }
                    LOG.trace("vBridge state has been changed.");
                }
            } catch (Exception e) {
                TestBase.unexpected(e);
            } finally {
                reg.unregister();
            }
        }

        /**
         * Determine whether the specified vBridge information represents
         * the expected state of the target vBridge.
         *
         * @param vbridge  A {@link VBridge} instance to be tested.
         * @return  {@code true} if the specified vBridge information
         *          represents the expected state of the target vBridge.
         *          Otherwise {@code false}.
         */
        private boolean isExpected(VBridge vbridge) {
            return (expectedState.equals(vbridge.getState()) &&
                    (expectedFaults < 0 ||
                     expectedFaults == vbridge.getFaults()));
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void vBridgeChanged(VBridgePath path, VBridge vbridge,
                                   UpdateType type) {
            if (!type.equals(UpdateType.REMOVED) && isExpected(vbridge)) {
                synchronized (this) {
                    changed = true;
                    notify();
                }
            }
        }
    }

    /**
     * Test method for
     * {@link IVTNManagerAware#vtnChanged(VTenantPath, VTenant, UpdateType)}
     * {@link IVTNManagerAware#vBridgeChanged(VBridgePath, VBridge, UpdateType)}
     * {@link IVTNManagerAware#vInterfaceChanged(VBridgeIfPath, VInterface, UpdateType)}
     * {@link IVTNManagerAware#vlanMapChanged(VBridgePath, VlanMap, UpdateType)}
     * {@link IVTNManagerAware#portMapChanged(VBridgeIfPath, PortMap, UpdateType)}
     * @throws InterruptedException
     * @throws VTNException
     */
    @Test
    public void testIVTNManagerAware() throws InterruptedException, VTNException {
        LOG.info("Running testIVTNManagerAware().");

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

        ServiceRegistration updateServiceReg = ServiceHelper.
            registerServiceWReg(IVTNManagerAware.class,
                                GlobalConstants.DEFAULT.toString(),
                                listener, props);
        assertNotNull(updateServiceReg);

        ServiceRegistration updateServiceRegRepeated = ServiceHelper.
            registerServiceWReg(IVTNManagerAware.class,
                                GlobalConstants.DEFAULT.toString(),
                                listenerRepeated, props);
        assertNotNull(updateServiceRegRepeated);
        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

        res = listener.restart(1);

        // add a tenant
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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
        st = mgr.addInterface(ifpath, ifconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vIfChangedInfo.type.equals(UpdateType.ADDED));
        assertTrue(up.vIfChangedInfo.path.equals(ifpath));
        assertTrue(up.vIfChangedInfo.obj.getName().equals(ifname));

        // set a PortMap
        res = listener.restart(3);
        Node node = NodeCreator.createOFNode(0L);
        SwitchPort port =
            new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW,
                           String.valueOf(10));
        PortMapConfig pmconf = new PortMapConfig(node, port, (short)0);
        st = mgr.setPortMap(ifpath, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.CHANGED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // modify a vbridge interface setting
        res = listener.restart(1);
        st = mgr.modifyInterface(ifpath,
                new VInterfaceConfig("interface", Boolean.TRUE), false);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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
        st = mgr.removeInterface(ifpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

        ups = listener.getUpdates();
        assertEquals(1, ups.size());

        up = ups.get(0);
        assertTrue(up.vbrChangedInfo.type.equals(UpdateType.REMOVED));
        assertTrue(up.vbrChangedInfo.path.equals(bpath));
        assertTrue(up.vbrChangedInfo.obj.getName().equals(bname));

        // remove a tenant
        res = listener.restart(1);
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(LATCH_TIMEOUT, TimeUnit.SECONDS);
        ups = listener.getUpdates();
        assertEquals(0, ups.size());
    }


    /**
     * Test method for
     * {@link VTNManagerImpl#getMacEntries(VBridgePath)},
     * {@link VTNManagerImpl#getMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManagerImpl#removeMacEntry(VBridgePath, DataLinkAddress)},
     * {@link VTNManagerImpl#flushMacEntries(VBridgePath)}
     */
    @Test
    public void testMacEntry() {
        LOG.info("Running testMacEntry().");

        class DataLinkAddressStub extends DataLinkAddress {
            private static final long serialVersionUID = -9043768232113080608L;

            @Override
            public DataLinkAddress clone() {
                return null;
            }
        }

        IVTNManager mgr = this.vtnManager;

        List<VTenantPath> tpathes = new ArrayList<VTenantPath>();

        VTenantPath vtn = new VTenantPath("vtn");
        tpathes.add(vtn);
        tpathes.add(new VTenantPath("vtn_nomake"));
        tpathes.add(new VTenantPath("_invalid"));
        tpathes.add(new VTenantPath(""));
        tpathes.add(new VTenantPath(null));

        Status st = mgr.addTenant(vtn, new VTenantConfig("test"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VBridgePath vbr = new VBridgePath(vtn, "vbr");
        st = mgr.addBridge(vbr, new VBridgeConfig("Test"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        MacAddressEntry macAddrTemp = new MacAddressEntry(null, (short)0,
                                                          null, null);

        List<EthernetAddress> ethers = createEthernetAddresses(false);
        List<DataLinkAddress> dladdrs = new ArrayList<DataLinkAddress>();
        dladdrs.addAll(ethers);
        dladdrs.add(new DataLinkAddressStub());
        dladdrs.add(null);

        // Wait 3000 (+ 100) ms to be removed from "disabled nodes".
        // (Default value of "disabled" time is 3000 ms.)
        sleep(3000 + 100);

        for (VTenantPath tpath : tpathes) {
            boolean isValidTPath =
                    (tpath == null) ? false : isValidName(tpath.getTenantName());
            boolean isPresentT = isValidTPath && tpath.equals(vtn);

            List<VBridgePath> bpathes = new ArrayList<VBridgePath>();
            bpathes.add(new VBridgePath(tpath, "vbr"));
            bpathes.add(new VBridgePath(tpath, "vbr_nomake"));
            bpathes.add(new VBridgePath(tpath, "can'tcreate"));
            bpathes.add(new VBridgePath(tpath, ""));
            bpathes.add(new VBridgePath(tpath, null));
            bpathes.add(null);

            for (VBridgePath bpath : bpathes) {
                boolean isValitBName = (bpath == null) ? false : isValidName(bpath.getBridgeName());
                boolean isValidBPath = isValidTPath && isValitBName;
                boolean isPresentB = isValidBPath && bpath.equals(vbr);

                String emsg = "(tname)" + tpath.getTenantName() +
                              "(bname)" + ((bpath == null) ? "(null)" : bpath.getBridgeName());

                // Test for getEntry() (Abnormal state)
                List<MacAddressEntry> listMacAddr = null;
                try {
                    listMacAddr = mgr.getMacEntries(bpath);
                } catch (VTNException e) {
                    if ((bpath == null) || (tpath.getTenantName() == null)) {
                        assertEquals(emsg, StatusCode.BADREQUEST, e.getStatus().getCode());
                    } else if (isPresentT && (bpath.getBridgeName() == null)) {
                        assertEquals(emsg, StatusCode.BADREQUEST, e.getStatus().getCode());
                    } else if (isValidBPath && !isPresentB) {
                        assertEquals(emsg, StatusCode.NOTFOUND, e.getStatus().getCode());
                    } else if (isPresentB) {
                        // It is strange
                        unexpected(e);
                    }
                }

                if (isPresentB) {
                    assertNotNull(listMacAddr);
                    assertTrue(listMacAddr.isEmpty());
                } else {
                    assertNull(listMacAddr);
                }

                for (DataLinkAddress ea : dladdrs) {
                    MacAddressEntry macAddrEntry = macAddrTemp;
                    String emsgEA = emsg + "(ether)" + ((ea == null) ? "(null)" : ea.toString());

                    // Test for getMacEntry() (Abnormal state)
                    boolean isSuccess = false;
                    try {
                        macAddrEntry = mgr.getMacEntry(bpath, ea);
                        isSuccess = true;
                    } catch (VTNException e) {
                        if ((bpath == null) || (tpath.getTenantName() == null)) {
                            assertEquals(emsgEA, StatusCode.BADREQUEST, e.getStatus().getCode());
                        } else if (isPresentT && (bpath.getBridgeName() == null)) {
                            assertEquals(emsgEA, StatusCode.BADREQUEST, e.getStatus().getCode());
                        } else if (isValidBPath && (ea != null) && !isPresentB) {
                            assertEquals(emsgEA, StatusCode.NOTFOUND, e.getStatus().getCode());
                        } else if (isPresentB && (ea == null)) {
                            assertEquals(emsgEA, StatusCode.BADREQUEST, e.getStatus().getCode());
                        } else if (isPresentB && (ea != null)) {
                            // It is strange
                            unexpected(e);
                        } else {
                            // In this case, Status code is either BADREQUEST or NOTFOUND
                            checkExceptionStatus(e.getStatus(), false, emsgEA);
                        }
                    }
                    if (isSuccess) {
                        assertNull(emsgEA, macAddrEntry);
                    }

                    // Test for removeMacEntry() (Abnormal state)
                    macAddrEntry = macAddrTemp;
                    isSuccess = false;
                    try {
                        macAddrEntry = mgr.removeMacEntry(bpath, ea);
                        isSuccess = true;
                    } catch (VTNException e) {
                        if ((bpath == null) || (tpath.getTenantName() == null)) {
                            assertEquals(emsgEA, StatusCode.BADREQUEST, e.getStatus().getCode());
                        } else if (isPresentT && (bpath.getBridgeName() == null)) {
                            assertEquals(emsgEA, StatusCode.BADREQUEST, e.getStatus().getCode());
                        } else if (isValidBPath && (ea != null) && !isPresentB) {
                            assertEquals(emsgEA, StatusCode.NOTFOUND, e.getStatus().getCode());
                        } else if (isPresentB && (ea == null)) {
                            assertEquals(emsgEA, StatusCode.BADREQUEST, e.getStatus().getCode());
                        } else if (isPresentB && (ea != null)) {
                            // It is strange
                            unexpected(e);
                        } else {
                            // In this case, Status code is either BADREQUEST or NOTFOUND
                            checkExceptionStatus(e.getStatus(), false, emsgEA);
                        }
                    }
                    if (isSuccess) {
                        assertNull(emsgEA, macAddrEntry);
                    }
                } // end of DataLinkAddress

                // Test for flushMacEntries (Abnormal state)
                st = mgr.flushMacEntries(bpath);
                if ((bpath == null) || (tpath.getTenantName() == null)) {
                    assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                } else if (isPresentT && (bpath.getBridgeName() == null)) {
                    assertEquals(emsg, StatusCode.BADREQUEST, st.getCode());
                } else if (isValidBPath && !isPresentB) {
                    assertEquals(emsg, StatusCode.NOTFOUND, st.getCode());
                } else if (isPresentB) {
                    assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
                } else {
                    // In this case, Status code is either BADREQUEST or NOTFOUND
                    checkExceptionStatus(st, false, emsg);
                }

                // If VBridge is NOT present, go next pattern.
                if (!isPresentB) {
                    continue;
                }

                ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
                ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));

                // Stub emulates 3 nodes and also emulates 1 node connector on each node.
                List<Node> existNodes = new ArrayList<Node>();
                existNodes.addAll(swmgr.getNodes());
                assertEquals(3, existNodes.size());
                List<NodeConnector> existConnectors = new ArrayList<NodeConnector>();
                for (Node node: existNodes) {
                    existConnectors.addAll(swmgr.getNodeConnectors(node));
                }
                assertEquals(3, existConnectors.size());

                List<VBridgePath> listVBrTemp = new ArrayList<VBridgePath>();
                List<VBridgePath> listVBr = new ArrayList<VBridgePath>();
                for (int i = 2; i <= 3; i++) {
                    VBridgePath vbrTemp = new VBridgePath(vtn, "vbr" + Integer.valueOf(i).toString());
                    st = mgr.addBridge(vbrTemp, new VBridgeConfig(null));
                    assertEquals(StatusCode.SUCCESS, st.getCode());
                    listVBrTemp.add(vbrTemp);
                }

                VBridgeIfPath vifpath = new VBridgeIfPath(listVBrTemp.get(1), "vif");
                st = mgr.addInterface(vifpath, new VInterfaceConfig(null, Boolean.TRUE));
                assertEquals(StatusCode.SUCCESS, st.getCode());

                listVBr.add(vbr);
                listVBr.addAll(listVBrTemp);

                VlanMap map = null;
                try {
                    map = vtnManager.addVlanMap(bpath, new VlanMapConfig(null, (short)0));
                } catch (VTNException e) {
                    unexpected(e);
                }

                assertTrue(existConnectors.get(2).getID() instanceof Short);
                Short ncID = (Short)existConnectors.get(2).getID();
                SwitchPort swport = new SwitchPort(NodeConnectorIDType.OPENFLOW, ncID.toString());
                st = mgr.setPortMap(vifpath, new PortMapConfig(existNodes.get(2), swport, (short)1));
                assertEquals(StatusCode.SUCCESS, st.getCode());

                for (int j = 0; j < listVBr.size(); j++) {
                    for (int i = 0; i < ethers.size(); i++) {
                        EthernetAddress ea = ethers.get(i);
                        byte[] bytes = ea.getValue();
                        byte[] src = new byte[] {00, bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};
                        byte[] dst =
                                new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                            (byte)0xFF, (byte)0xFF, (byte)0xFF};
                        byte[] sender = new byte[] {(byte)192, (byte)168,
                                                    (byte)0, (byte)(i + 1)};
                        byte[] target = new byte[] {(byte)192, (byte)168,
                                                    (byte)0, (byte)250};

                        String emsgEA = emsg + "(ether)" + ea.toString();

                        // The VTN Manager never learns zero MAC address.
                        if (NetUtils.byteArray6ToLong(src) == 0L) {
                            src[5] = (byte)0xff;
                        }

                        EthernetAddress eaSrc = null;
                        try {
                            eaSrc = new EthernetAddress(src);
                        } catch (ConstructionException e) {
                            unexpected(e);
                        }

                        RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short)(j - 1),
                                existConnectors.get(j), ARP.REQUEST);
                        PacketResult pktRes = listenDataPacket.receiveDataPacket(inPkt);
                        assertEquals(emsgEA, PacketResult.KEEP_PROCESSING, pktRes);

                        /*
                         * Test for getMacEntry (Normal state)
                         *
                         * If "j" is 0 or 1, VLAN ID of ARP packet created above is set to "0".
                         * Hence, we expect MAC Entry of listVBr.get(1) is always null,
                         * and also expect MAC Entry of listVBr.get(2) is null when "j" is NOT 2.
                         *
                         * Following table is the relationship between "j" and "k".
                         * "F" means expecting null. "T" means not expecting null.
                         *
                         *  k/j|0|1|2
                         *  ---+-+-+-
                         *  0  |T|T|F
                         *  ---+-+-+-
                         *  1  |F|F|F
                         *  ---+-+-+-
                         *  2  |F|F|T
                         */
                        for (int k = 0; k < listVBr.size(); k++) {
                            MacAddressEntry entry = null;
                            try {
                                entry = mgr.getMacEntry(listVBr.get(k), eaSrc);
                            } catch (VTNException e) {
                                unexpected(e);
                            }
                            if ((k == 1) || (j + k == 2) || ((j == 1) && (k == 2))) {
                                assertNull(emsgEA, entry);
                            } else {
                                assertNotNull(emsgEA, entry);
                                assertEquals(emsgEA, eaSrc, entry.getAddress());
                                assertEquals(emsgEA, existConnectors.get(j), entry.getNodeConnector());

                                Set<InetAddress> ips = entry.getInetAddresses();
                                assertArrayEquals(emsg, sender, ips.iterator().next().getAddress());
                            }
                        } // end of "int k"
                    } // end of "int i"

                    /*
                     * Test for getMacEntries (Normal state).
                     * The relationship between "j" and "k" is same as getMacEntry().
                     * (See above.)
                     */
                    for (int k = 0; k < listVBr.size(); k++) {
                        List<MacAddressEntry> list = null;
                        try {
                            list = mgr.getMacEntries(listVBr.get(k));
                        } catch (VTNException e) {
                            unexpected(e);
                        }
                        assertNotNull(list);
                        if ((k == 1) || (j + k == 2) || ((j == 1) && (k == 2))) {
                            assertEquals(emsg, 0, list.size());
                        } else {
                            assertEquals(emsg, ethers.size(), list.size());
                        }
                    }

                    // Test for removeMacEntry (Normal state)
                    byte[] bytes = ethers.get(0).getValue();
                    byte[] src = new byte[] {00, bytes[4], bytes[3], bytes[2], bytes[1], bytes[0]};

                    // The VTN Manager never learns zero MAC address.
                    if (NetUtils.byteArray6ToLong(src) == 0L) {
                        src[5] = (byte)0xff;
                    }

                    EthernetAddress eaSrc = null;
                    try {
                        eaSrc = new EthernetAddress(src);
                    } catch (ConstructionException e) {
                        unexpected(e);
                    }

                    MacAddressEntry entry = null;
                    try {
                        entry = mgr.removeMacEntry(listVBr.get((j == 1) ? 0 : j), eaSrc);
                    } catch (VTNException e) {
                        unexpected(e);
                    }
                    assertNotNull(entry);

                    try {
                        for (MacAddressEntry macEntry : mgr.getMacEntries(listVBr.get((j == 1) ? 0 : j))) {
                            assertTrue(macEntry.getAddress() instanceof EthernetAddress);
                            assertFalse(emsg, eaSrc.equals(macEntry.getAddress()));
                        }
                    } catch (VTNException e) {
                        unexpected(e);
                    }

                    // Test for flushMacEntries (Normal state)
                    st = mgr.flushMacEntries(listVBr.get((j == 1) ? 0 : j));
                    assertEquals(StatusCode.SUCCESS, st.getCode());
                    for (int k = 0; k < listVBr.size(); k++) {
                        List<MacAddressEntry> list = null;
                        try {
                            list = mgr.getMacEntries(listVBr.get(k));
                        } catch (VTNException e) {
                            unexpected(e);
                        }
                        assertNotNull(list);
                        assertEquals(0, list.size());
                    } // end of "k"
                } // end of "j"

                // Clear up
                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(StatusCode.SUCCESS, st.getCode());

                st = mgr.removeInterface(vifpath);
                assertEquals(StatusCode.SUCCESS, st.getCode());

                for (VBridgePath vbrTemp : listVBrTemp) {
                    st = mgr.removeBridge(vbrTemp);
                    assertEquals(StatusCode.SUCCESS, st.getCode());
                }
            } // end of VBridgePath
        } // end of VTenantPath

        // Clear up
        st = mgr.removeBridge(vbr);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = mgr.removeTenant(vtn);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }


    /**
     * test method for {@link IVTNManager#isActive()}
     */
    private void testIsActive() {
        LOG.info("Running testIsActive().");

        // There is NO VTN.
        IVTNManager mgr = this.vtnManager;
        try {
            List<VTenant> listVTN = mgr.getTenants();
            assertTrue(listVTN.isEmpty());
        } catch (VTNException e) {
            unexpected(e);
        }
        assertFalse(mgr.isActive());

        // Make VTN.
        VTenantPath tpath = new VTenantPath("vtn");
        mgr.addTenant(tpath, new VTenantConfig(null));
        assertTrue(mgr.isActive());

        // Clear up
        mgr.removeTenant(tpath);

        // TODO : In case if container is available expected "default".
    }

    /**
     * test method for {@link ICacheUpdateAware}
     *
     * @throws InterruptedException  Test was interrupted.
     */
    @Test
    public void testICacheUpdateAware() throws InterruptedException {
        LOG.info("Running testICacheUpdateAware().");

        IVTNManager mgr = vtnManager;
        String containerName = GlobalConstants.DEFAULT.toString();
        File containerDir = new File(GlobalConstants.STARTUPHOME.toString(),
                                     containerName);
        File vtnDir = new File(containerDir, "vtn");
        File tenantDir = new File(vtnDir, "TENANT");
        String configFileName100 = "tenant100.conf";
        String configFileName = "tenant.conf";

        Dictionary<String, Object> props = new Hashtable<String, Object>();
        VTNManagerAware listener = new VTNManagerAware();
        ServiceRegistration svReg = ServiceHelper.
            registerServiceWReg(IVTNManagerAware.class, containerName,
                                listener, props);

        // create tenant
        File configFile = new File(tenantDir, configFileName);
        File configFile100 = new File(tenantDir, configFileName100);
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
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        configFile.delete();
        configFile100.delete();

        VTenantEvent ev = new VTenantEvent(tpath, vtenant, UpdateType.ADDED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        assertFalse(configFile.exists());
        assertFalse(configFile100.exists());

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        assertTrue(configFile.exists());
        assertFalse(configFile100.exists());

        VTenantPath tpath100 = new VTenantPath("tenant100");
        VTenantConfig tconf100 = new VTenantConfig("tenant 100");
        VTenant vtenant100 = new VTenant("tenant100", tconf100);
        res = listener.restart(1);
        mgr.addTenant(tpath100, tconf100);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        configFile.delete();
        configFile100.delete();

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.ADDED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        assertFalse(configFile.exists());
        assertFalse(configFile100.exists());

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        assertFalse(configFile.exists());
        assertTrue(configFile100.exists());

        configFile.delete();
        configFile100.delete();

        // update
        ev = new VTenantEvent(tpath, vtenant, UpdateType.CHANGED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, false, true);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        checkFileExists(configFile, true, true);
        checkFileExists(configFile100, false, true);

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.CHANGED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, false, true);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        checkFileExists(configFile, false, true);
        checkFileExists(configFile100, true, true);

        res = listener.restart(2);
        mgr.removeTenant(tpath);
        mgr.removeTenant(tpath100);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));

        // delete
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

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        checkFileExists(configFile, false, false);
        checkFileExists(configFile100, true, false);

        ev = new VTenantEvent(tpath100, vtenant100, UpdateType.REMOVED);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      true);
        checkFileExists(configFile, true, false);
        checkFileExists(configFile100, true, false);

        res = listener.restart(1);
        cacheUpdateAware.entryUpdated(evid, ev, VTNManagerImpl.CACHE_EVENT,
                                      false);
        assertTrue(res.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
        checkFileExists(configFile, true, true);
        checkFileExists(configFile100, false, true);

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
        LOG.info("Running testIConfigurationContainerAware().");
        Status st = configContainerAware.saveConfiguration();
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * test method for {@link IInventoryListener}
     */
    @Test
    public void testIInventoryListener() {
        LOG.info("Running testIInventoryListener().");

        IInventoryListener mgr = inventoryListener;
        short[] vlans = {0, 10, 4095};

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
                Map<String, Property> propMap = null;

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
            bif = mgr.getInterface(ifp);
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
     * @param listenData  {@code IListenDataPacket} service.
     * @param bpath       VBridgePath
     * @param nc          NodeConnector
     */
    private void putMacTableEntry(IListenDataPacket listenData, VBridgePath bpath, NodeConnector nc) {
        byte[] src = new byte[] {(byte)0x00, (byte)0x01, (byte)0x01,
                                 (byte)0x01, (byte)0x01, (byte)0x01};
        byte[] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                 (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                             (short)-1, nc, ARP.REQUEST);
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
    private void checkMacTableEntry(IVTNManager mgr, VBridgePath bpath,
                                    boolean isFlushed, String msg) {
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
     * test method for {@link IContainerListener}
     */
    @Test
    @Ignore
    public void testIContainerListener() {
        // TODO: tagUpdate(), containerFlowUpdated(), nodeConnecctorUpdated(),
        //       containerModeUpdated()
    }

    /**
     * test case for {@link IListenDataPacket}
     */
    @Test
    public void testIListenDataPacket() {
        LOG.info("Running testIListenDataPacket().");

        ServiceReference r = bc.getServiceReference(ISwitchManager.class.getName());
        ISwitchManager swmgr = (ISwitchManager)(bc.getService(r));
        byte[] cntMac = swmgr.getControllerMAC();
        byte[] tgtMac = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                    (byte)0x11, (byte)0x11, (byte)0x11};

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
                byte[] src = ea.getValue();
                byte[] dst = {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                              (byte)0xFF, (byte)0xFF, (byte)0xFF};
                byte[] sender = {(byte)192, (byte)168, (byte)0, iphost};
                byte[] target = {(byte)192, (byte)168, (byte)0, (byte)250};
                RawPacket inPkt = createARPRawPacket(
                    src, dst, sender, target, (short)-1, nc, ARP.REQUEST);
                result = listenDataPacket.receiveDataPacket(inPkt);

                // because there are no topology, in this case always ignored.
                assertEquals(PacketResult.IGNORED, result);
            }

            // packet from controller.
            byte []src = cntMac.clone();
            byte []dst = {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                          (byte)0xFF, (byte)0xFF, (byte)0xFF};
            byte []sender = {(byte)192, (byte)168, (byte)0, (byte)1};
            byte []target = {(byte)192, (byte)168, (byte)0, (byte)250};

            RawPacket inPkt = createARPRawPacket(
                src, dst, sender, target, (short)-1, nc, ARP.REQUEST);
            result = listenDataPacket.receiveDataPacket(inPkt);
            assertEquals(PacketResult.IGNORED, result);
        }

        // Make 3 OF node and 4 NodeConnectors on each node.
        List<Node> listNode = new ArrayList<Node>();
        Map<Node, Collection<NodeConnector>> mapNc
            = new HashMap<Node, Collection<NodeConnector>>();
        for (int i = 0; i < 3; i++) {
            Node node = NodeCreator.createOFNode(new Long(0xDD00 + i));
            assertNotNull(node);
            listNode.add(node);

            List<NodeConnector> listNC = new ArrayList<NodeConnector>();
            for (int j = 0; j < 4; j++) {
                NodeConnector nc = NodeConnectorCreator
                        .createOFNodeConnector(new Short((short)(0xA0 + j)), node);
                assertNotNull(nc);
                listNC.add(nc);
            }
            mapNc.put(node, listNC);
        }

        List<Edge> listEdge = new ArrayList<Edge>();
        Set<NodeConnector> setInterSwLink = new HashSet<NodeConnector>();
        for (int i = 0; i < 3; i++) {
            Node hnode = listNode.get(i);
            Node tnode = null;
            switch (i) {
            case 0 :
            case 1 :
                tnode = listNode.get(i + 1);
                break;
            case 2 :
                tnode = listNode.get(0);
                break;
            default:
                fail("not supported case.");
                break;
            }
            assertTrue(mapNc.get(hnode) instanceof ArrayList);
            NodeConnector head =
                ((ArrayList<NodeConnector>)mapNc.get(hnode)).get(3);
            assertNotNull(head);
            assertTrue(mapNc.get(tnode) instanceof ArrayList);
            NodeConnector tail =
                ((ArrayList<NodeConnector>)mapNc.get(tnode)).get(2);
            assertNotNull(tail);
            Edge edge = null;
            Edge redge = null;
            try {
                edge = new Edge(tail, head);
                redge = new Edge(head, tail);
            } catch (ConstructionException e) {
                unexpected(e);
            }
            listEdge.add(edge);
            listEdge.add(redge);
            setInterSwLink.add(head);
            setInterSwLink.add(tail);
        }

        // Make VBridge
        VBridgePath bpath = new VBridgePath(tpath, "vbr");
        st = vtnManager.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Get FlowProgrammerService from openflow stub
        IPluginInFlowProgrammerService pinFPS =
            getStubService(IPluginInFlowProgrammerService.class);
        assertTrue(pinFPS instanceof FlowProgrammerService);
        FlowProgrammerService fps = (FlowProgrammerService)pinFPS;

        // Get InventoryService from openflow stub
        IPluginInInventoryService pinIVS =
            getStubService(IPluginInInventoryService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinIVS instanceof InventoryService);
        InventoryService ivs = (InventoryService)pinIVS;

        // Get TopologyService from openflow stub
        IPluginInTopologyService pinTPS =
            getStubService(IPluginInTopologyService.class,
                           GlobalConstants.DEFAULT.toString());
        assertNotNull(pinTPS);
        assertTrue(pinTPS instanceof TopologyServices);
        TopologyServices tps = (TopologyServices)pinTPS;

        // Now, add Node and NodeConnectors
        ivs.addNode(mapNc);

        // Wait 3000 (+ 100) ms to be removed from "disabled nodes".
        // (Default value of "disabled" time is 3000 ms.)
        sleep(3000 + 100);

        // Update edges, and wait for the shortest path graph to be updated.
        LOG.trace("Update edge: {}", listEdge);
        Set<Property> properties = new HashSet<Property>();
        tps.addEdge(listEdge, properties, UpdateType.ADDED);
        EdgeWaiter waiter = new EdgeWaiter();
        waiter.await(listNode.get(1), listNode.get(0));

        short vtnFlowPriorityDefault = 10;
        short vtnFlowIdleTimeoutDefault = 300;
        for (int mapType = 0; mapType < 2; mapType++) {
            short mappedVlan = 0;
            VlanMap map = null;
            List<VBridgeIfPath> listVIFpath = null;

            switch (mapType) {
            case 0 :
                // Make VLAN Map
                VlanMapConfig vlconf = new VlanMapConfig(null, mappedVlan);
                try {
                    map = vtnManager.addVlanMap(bpath, vlconf);
                } catch (VTNException e) {
                    unexpected(e);
                }
                break;

            case 1 :
                // Make Port Map
                listVIFpath = new ArrayList<VBridgeIfPath>();
                int i = 0;
                for (Collection<NodeConnector> collectionNC : mapNc.values()) {
                    assertTrue(collectionNC instanceof ArrayList);
                    List<NodeConnector> list = (ArrayList<NodeConnector>)collectionNC;

                    // "2" and "3" are both used for connecting to other nodes. (Edge)
                    for (int j = 0; j < 2; j++) {
                        VBridgeIfPath vifpath = new VBridgeIfPath(bpath, "vif" + Integer.toString(i));
                        st = vtnManager.addInterface(vifpath, new VInterfaceConfig(null, Boolean.TRUE));
                        assertEquals(StatusCode.SUCCESS, st.getCode());
                        listVIFpath.add(vifpath);
                        i++;

                        assertTrue(list.get(j).getID() instanceof Short);
                        Short id = (Short)list.get(j).getID();
                        SwitchPort swport =
                                new SwitchPort(NodeConnector.NodeConnectorIDType.OPENFLOW, id.toString());
                        PortMapConfig pmconf =
                                new PortMapConfig(list.get(j).getNode(), swport, mappedVlan);
                        st = vtnManager.setPortMap(vifpath, pmconf);
                        assertEquals(StatusCode.SUCCESS, st.getCode());

                        try {
                            VInterface vif = vtnManager.getInterface(vifpath);
                            assertEquals(VNodeState.UP, vif.getState());
                        } catch (VTNException e) {
                            unexpected(e);
                        }
                    }
                }
                break;

            default:
                fail("Unsupported type");
                break;
            } // end if switch

            // Check if VBridge is UP state.
            try {
                VBridge vbr = vtnManager.getBridge(bpath);
                assertEquals(VNodeState.UP, vbr.getState());
            } catch (VTNException e) {
                unexpected(e);
            }

            // Study MAC Address
            // from node0 to node0
            byte iphost = 1;
            List<EthernetAddress> ethers = createEthernetAddresses(false);
            Map<Node, List<Flow>> mapNodeFlow = new HashMap<Node, List<Flow>>();

            CountDownLatch latch = fps.setLatch(listNode.get(0), ethers.size());
            for (EthernetAddress ea: ethers) {
                List<NodeConnector> listNc =
                    (List<NodeConnector>)mapNc.get(listNode.get(0));
                NodeConnector reqNc = listNc.get(0);
                NodeConnector replyNc = listNc.get(1);

                byte[] src = ea.getValue();
                byte[] dstBC = {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                (byte)0xFF, (byte)0xFF, (byte)0xFF};
                byte[] dst = tgtMac;
                byte[] sender = {(byte)192, (byte)168, (byte)0, iphost};
                byte[] target = {(byte)192, (byte)168, (byte)0, (byte)250};
                RawPacket inPkt = createARPRawPacket(
                    src, dstBC, sender, target, (short)-1, reqNc, ARP.REQUEST);
                result = listenDataPacket.receiveDataPacket(inPkt);

                // There is a topology, So in this case, returns "KEEP_PROCESSING".
                assertEquals(PacketResult.KEEP_PROCESSING, result);

                inPkt = createARPRawPacket(dst, src, target, sender, (short)-1,
                                           replyNc, ARP.REPLY);
                result = listenDataPacket.receiveDataPacket(inPkt);

                assertEquals(PacketResult.KEEP_PROCESSING, result);
                iphost++;

                // create Flow to check later.
                Node flowNode = listNode.get(0);
                Match match = createMatch(dst, src, mappedVlan, replyNc);
                List<Action> actList = createActionList(reqNc, (short)0,
                                                        mappedVlan);
                Flow flow = new Flow(match, actList);
                flow.setPriority(vtnFlowPriorityDefault);
                flow.setIdleTimeout(vtnFlowIdleTimeoutDefault);

                List<Flow> flowList = mapNodeFlow.get(flowNode);
                if (flowList == null) {
                    flowList = new ArrayList<Flow>();
                    mapNodeFlow.put(flowNode, flowList);
                }
                flowList.add(flow);
            }

            try {
                assertTrue(latch.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }

            for (int i = 0; i < 3; i++) {
                String emsg = "(node)" + Integer.toString(i);
                Node checkedNode = listNode.get(i);
                if (i == 0) {
                    assertEquals(emsg, ethers.size(), fps.getFlowCount(checkedNode));

                    List<Flow> installedFlows = fps.getFlows(checkedNode);
                    List<Flow> flowList = mapNodeFlow.get(checkedNode);
                    assertNotNull(emsg, installedFlows);
                    assertNotNull(emsg, flowList);
                    assertEquals(emsg, flowList.size(), installedFlows.size());

                    for (Flow flow : flowList) {
                        assertTrue(flow.toString(), installedFlows.contains(flow));
                    }
                } else {
                    assertEquals(emsg, 0, fps.getFlowCount(checkedNode));
                }
            }
            mapNodeFlow.clear();

            // Send ARP reply from node1 to node0.
            // This test will remove flows installed by previous test because
            // tgtMac is moved from node0 to node1.
            latch = fps.setLatch(listNode.get(0), ethers.size() * 2);
            CountDownLatch latch2 = fps.setLatch(listNode.get(1), ethers.size());
            iphost = 1;
            for (EthernetAddress ea: ethers) {
                List<NodeConnector> listNc =
                    (List<NodeConnector>)mapNc.get(listNode.get(0));
                NodeConnector reqNc = null;
                for (NodeConnector nc : listNc) {
                    if (!setInterSwLink.contains(nc)) {
                        reqNc = nc;
                        break;
                    }
                }
                listNc = (List<NodeConnector>)mapNc.get(listNode.get(1));
                NodeConnector replyNc = null;
                for (NodeConnector nc : listNc) {
                    if (!setInterSwLink.contains(nc)) {
                        replyNc = nc;
                        break;
                    }
                }

                Edge edge = null;
                for (Edge e : listEdge) {
                    if (e.getHeadNodeConnector().getNode().equals(replyNc.getNode())
                            && e.getTailNodeConnector().getNode().equals(reqNc.getNode())) {
                        edge = e;
                        break;
                    }
                }

                byte[] src = ea.getValue();
                byte[] dstBC = {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                (byte)0xFF, (byte)0xFF, (byte)0xFF};
                byte[] dst = tgtMac;
                byte[] sender = {(byte)192, (byte)168, (byte)0, iphost};
                byte[] target = {(byte)192, (byte)168, (byte)0, (byte)250};
                RawPacket inPkt = createARPRawPacket(
                    src, dstBC, sender, target, (short)-1, reqNc, ARP.REQUEST);
                result = listenDataPacket.receiveDataPacket(inPkt);

                // There is a topology, So in this case, returns "KEEP_PROCESSING".
                assertEquals(PacketResult.KEEP_PROCESSING, result);

                inPkt = createARPRawPacket(dst, src, target, sender, (short)-1,
                                           replyNc, ARP.REPLY);
                result = listenDataPacket.receiveDataPacket(inPkt);

                assertEquals(PacketResult.KEEP_PROCESSING, result);
                iphost++;

                // create Flow on node1 to check later.
                Node flowNode = replyNc.getNode();
                Match match = createMatch(dst, src, mappedVlan, replyNc);
                List<Action> actList =
                    createActionList(edge.getHeadNodeConnector(), (short)0,
                                     (short)0);

                Flow flow = new Flow(match, actList);
                flow.setPriority(vtnFlowPriorityDefault);
                flow.setIdleTimeout(vtnFlowIdleTimeoutDefault);

                List<Flow> flowList = mapNodeFlow.get(flowNode);
                if (flowList == null) {
                    flowList = new ArrayList<Flow>();
                    mapNodeFlow.put(flowNode, flowList);
                }
                flowList.add(flow);

                // create Flow on node0 to check later.
                flowNode = reqNc.getNode();
                match = createMatch(dst, src, mappedVlan,
                                    edge.getTailNodeConnector());
                actList = createActionList(reqNc, (short)0, mappedVlan);

                flow = new Flow(match, actList);
                flow.setPriority(vtnFlowPriorityDefault);
                flow.setIdleTimeout((short)0);

                flowList = mapNodeFlow.get(flowNode);
                if (flowList == null) {
                    flowList = new ArrayList<Flow>();
                    mapNodeFlow.put(flowNode, flowList);
                }
                flowList.add(flow);
            }

            try {
                assertTrue(latch.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
                assertTrue(latch2.await(LATCH_TIMEOUT, TimeUnit.SECONDS));
            } catch (InterruptedException e) {
                unexpected(e);
            }

            for (int i = 0; i < 3; i++) {
                String emsg = "(node)" + Integer.toString(i);
                Node checkedNode = listNode.get(i);
                if (i == 2) {
                    assertEquals(emsg, 0, fps.getFlowCount(checkedNode));
                } else {
                    assertEquals(emsg, ethers.size(), fps.getFlowCount(checkedNode));

                    List<Flow> installedFlows = fps.getFlows(checkedNode);
                    List<Flow> flowList = mapNodeFlow.get(checkedNode);
                    assertNotNull(emsg, installedFlows);
                    assertNotNull(emsg, flowList);
                    assertEquals(flowList.size(), installedFlows.size());

                    for (Flow flow : flowList) {
                        assertTrue(emsg + flow.toString(), installedFlows.contains(flow));
                    }
                }
            }

            switch (mapType) {
            case 0 :
                // Remove VLAN Map
                assertNotNull(map);
                st = vtnManager.removeVlanMap(bpath, map.getId());
                assertEquals(StatusCode.SUCCESS, st.getCode());
                break;

            case 1 :
                // Remove Port Map
                assertNotNull(listVIFpath);
                assertFalse(listVIFpath.isEmpty());
                for (VBridgeIfPath vifpath : listVIFpath) {
                    st = vtnManager.removeInterface(vifpath);
                    assertEquals(StatusCode.SUCCESS, st.getCode());
                }
                break;

            default :
                fail("Unsupported type");
                break;
            } //end of switch
        } // end of "int mapType"

        st = vtnManager.removeBridge(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        ivs.removeNode(listNode);

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    private Match createMatch(byte[] src, byte[] dst, short vlan, NodeConnector inport) {
        Match match = new Match();
        match.setField(MatchType.DL_SRC, src);
        match.setField(MatchType.DL_DST, dst);
        match.setField(MatchType.DL_VLAN, vlan);
        match.setField(MatchType.IN_PORT, inport);

        return match;
    }

    private List<Action> createActionList(NodeConnector outport, short inVlan,
                                          short outVlan) {
        List<Action> actList = new ArrayList<Action>();
        if (inVlan != outVlan) {
            if (outVlan == 0) {
                actList.add(new PopVlan());
            } else {
                if (inVlan == 0) {
                    actList.add(new PushVlan(EtherTypes.VLANTAGGED));
                }
                actList.add(new SetVlanId(outVlan));
            }
        }
        actList.add(new Output(outport));

        return actList;
    }
    /**
     * test method for {@link IListenRoutingUpdates}
     */
    @Test
    public void testIListenRoutingUpdates() {
        LOG.info("Running testIListenRoutingUpdates().");

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        Status st = vtnManager.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(vtnManager.isActive());

        // Make 3 OF node and 4 NodeConnectors on each node.
        List<Node> listNode = new ArrayList<Node>();
        Map<Node, Collection<NodeConnector>> mapNc
            = new HashMap<Node, Collection<NodeConnector>>();
        for (int i = 0; i < 3; i++) {
            Node node = NodeCreator.createOFNode(new Long(0xDD10 + i));
            assertNotNull(node);
            listNode.add(node);

            List<NodeConnector> listNC = new ArrayList<NodeConnector>();
            for (int j = 0; j < 4; j++) {
                NodeConnector nc = NodeConnectorCreator
                        .createOFNodeConnector(new Short((short)(0xAA + j)), node);
                assertNotNull(nc);
                listNC.add(nc);
            }
            mapNc.put(node, listNC);
        }

        List<Edge> listEdge = new ArrayList<Edge>();
        List<NodeConnector> listISL = new ArrayList<NodeConnector>();
        for (int i = 0; i < 3; i++) {
            Node hnode = listNode.get(i);
            Node tnode = null;

            switch (i) {
            case 0 :
            case 1 :
                tnode = listNode.get(i + 1);
                break;
            case 2 :
                tnode = listNode.get(0);
                break;
            default:
                fail("not supported case.");
            }

            assertTrue(mapNc.get(hnode) instanceof ArrayList);
            NodeConnector head = ((ArrayList<NodeConnector>)mapNc.get(hnode)).get(3);
            assertNotNull(head);

            assertTrue(mapNc.get(tnode) instanceof ArrayList);
            NodeConnector tail = ((ArrayList<NodeConnector>)mapNc.get(tnode)).get(2);
            assertNotNull(tail);

            Edge edge = null;
            Edge redge = null;
            try {
                edge = new Edge(tail, head);
                redge = new Edge(head, tail);
            } catch (ConstructionException e) {
                unexpected(e);
            }
            listEdge.add(edge);
            listEdge.add(redge);
            listISL.add(head);
            listISL.add(tail);
        }

        // Make VBridge
        VBridgePath bpath = new VBridgePath(tpath, "vbr");
        st = vtnManager.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Make VLAN Map
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)0);
        VlanMap map = null;
        try {
            map = vtnManager.addVlanMap(bpath, vlconf);
        } catch (VTNException e) {
            unexpected(e);
        }
        VBridgeStateWaiter waiter =
            new VBridgeStateWaiter(bpath, VNodeState.UP, 0);
        waiter.await();

        // Get FlowProgrammerService from openflow stub
        IPluginInFlowProgrammerService pinFPS =
            getStubService(IPluginInFlowProgrammerService.class);
        assertTrue(pinFPS instanceof FlowProgrammerService);
        FlowProgrammerService fps = (FlowProgrammerService)pinFPS;

        // Get InventoryService from openflow stub
        IPluginInInventoryService pinIVS =
            getStubService(IPluginInInventoryService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinIVS instanceof InventoryService);
        InventoryService ivs = (InventoryService)pinIVS;

        // Get TopologyService from openflow stub
        IPluginInTopologyService pinTPS =
            getStubService(IPluginInTopologyService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinTPS instanceof TopologyServices);
        TopologyServices tps = (TopologyServices)pinTPS;

        // Now, add Node and NodeConnectors
        ivs.addNode(mapNc);

        // Wait 3000 (+ 100) ms to be removed from "disabled nodes".
        // (Default value of "disabled" time is 3000 ms.)
        sleep(3000 + 100);

        // from node1 to node0
        List<NodeConnector> listNc =
            (List<NodeConnector>)mapNc.get(listNode.get(0));
        NodeConnector reqNc = null;
        for (NodeConnector nc : listNc) {
            if (!listISL.contains(nc)) {
                reqNc = nc;
                break;
            }
        }
        listNc = (List<NodeConnector>)mapNc.get(listNode.get(1));
        NodeConnector replyNc = null;
        for (NodeConnector nc : listNc) {
            if (!listISL.contains(nc)) {
                replyNc = nc;
                break;
            }
        }

        byte [] src = {(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x00, (byte)0x00, (byte)0x01};
        byte [] dstBC = {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                         (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte [] dst = {(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x11, (byte)0x11, (byte)0x11};
        byte [] sender = {(byte)192, (byte)168, (byte)0, (byte)1};
        byte [] target = {(byte)192, (byte)168, (byte)0, (byte)250};
        RawPacket inPkt = createARPRawPacket(
            src, dstBC, sender, target, (short)-1, reqNc, ARP.REQUEST);
        PacketResult result = listenDataPacket.receiveDataPacket(inPkt);

        // because there are no topology, flow entry isn't installed.
        assertEquals(PacketResult.KEEP_PROCESSING, result);

        inPkt = createARPRawPacket(dst, src, target, sender, (short)-1,
                                   replyNc, ARP.REPLY);
        result = listenDataPacket.receiveDataPacket(inPkt);

        assertEquals(PacketResult.KEEP_PROCESSING, result);

        sleep(2000);
        for (int i = 0; i < 3; i++) {
            String emsg = "(node)" + Integer.toString(i);
            assertEquals(emsg, 0, fps.getFlowCount(listNode.get(i)));
        }

        waiter = new VBridgeStateWaiter(bpath, VNodeState.DOWN, 1);
        waiter.await();

        // add Edge. (recalculateDone is invoked.)
        Set<Property> properties = new HashSet<Property>();
        tps.addEdge(listEdge, properties, UpdateType.ADDED);

        waiter = new VBridgeStateWaiter(bpath, VNodeState.UP, 0);
        waiter.await();

        st = vtnManager.removeVlanMap(bpath, map.getId());
        assertEquals(StatusCode.SUCCESS, st.getCode());
        waiter = new VBridgeStateWaiter(bpath, VNodeState.UNKNOWN, 0);
        waiter.await();

        st = vtnManager.removeBridge(bpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        ivs.removeNode(listNode);

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * test method for {@link IHostFinder}
     */
    @Test
    public void testIHostFinder() {
        LOG.info("Running testIHostFinder().");

        IVTNManager mgr = vtnManager;
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath);
        createTenantAndBridge(mgr, tpath, bpathlist);

        // Wait 3000 (+ 100) ms to be removed from "disabled nodes".
        // (Default value of "disabled" time is 3000 ms.)
        sleep(3000 + 100);
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

        IPluginInDataPacketService pinDPS =
            getStubService(IPluginInDataPacketService.class,
                           GlobalConstants.DEFAULT.toString());
        assertTrue(pinDPS instanceof DataPacketServices);
        DataPacketServices dps = (DataPacketServices)pinDPS;

        short[] vlans = {0, 10, 4095};

        Set<Node> existNodes = swmgr.getNodes();
        assertFalse(existNodes.isEmpty());
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }
        assertFalse(existConnectors.isEmpty());

        InetAddress ia = null;
        InetAddress ia6 = null;
        try {
            byte[] addr = {(byte)10, (byte)0, (byte)0, (byte)1};
            ia = InetAddress.getByAddress(addr);

            addr = new byte[]{
                (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                (byte)0xe1, (byte)0x23, (byte)0xe6, (byte)0x88,
                (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0,
            };
            ia6 = InetAddress.getByAddress(addr);
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        for (short vlan: vlans) {
            VlanMap map = null;
            if (bpath != null) {
                VlanMapConfig vlconf = new VlanMapConfig(null, vlan);
                try {
                    map = mgr.addVlanMap(bpath, vlconf);
                } catch (VTNException e) {
                    unexpected(e);
                }
            }

            // All packets should be discarded unless VTN is present.
            boolean found = (bpath != null);
            long timeout = (found) ? LATCH_TIMEOUT : LATCH_FALSE_TIMEOUT;

            String emsg =
                    "(bpath)" + ((bpath == null) ? "(null)" : bpath.toString()) +
                    "(vlan)" + (new Short(vlan)).toString();
            dps.clearPkt();
            CountDownLatch res = dps.setLatch(existConnectors.size());
            hostFinder.find(ia);
            try {
                assertEquals(emsg, found, res.await(timeout, TimeUnit.SECONDS));
            } catch (Exception e) {
                unexpected(e);
            }

            int npackets = (found) ? existConnectors.size() : 0;
            assertEquals(emsg, npackets, dps.getPktCount());

            // IPv6 Address case
            dps.clearPkt();
            res = dps.setLatch(existConnectors.size());
            hostFinder.find(ia6);
            try {
                // In this state, NO packet is sent.
                assertFalse(emsg,
                            res.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(emsg, 0, dps.getPktCount());

            // if null case
            dps.clearPkt();
            res = dps.setLatch(existConnectors.size());
            hostFinder.find(null);
            try {
                // In this state, NO packet is sent.
                assertFalse(emsg,
                            res.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(emsg, 0, dps.getPktCount());

            // probe()
            byte [] mac = new byte [] {(byte)0x00, (byte)0x00, (byte)0x00,
                                       (byte)0x11, (byte)0x22, (byte)0x33};
            Iterator<NodeConnector> ite = existConnectors.iterator();
            assertTrue(ite.hasNext());
            NodeConnector nc = ite.next();
            HostNodeConnector hnode = null;
            try {
                hnode = new HostNodeConnector(mac, ia, nc, vlan);
            } catch (ConstructionException e) {
                unexpected(e);
            }

            int count;
            long tmout;
            if (found) {
                count = 1;
                tmout = LATCH_TIMEOUT;
            } else {
                count = 0;
                tmout = LATCH_FALSE_TIMEOUT;
            }

            dps.clearPkt();
            res = dps.setLatch(1);
            hostFinder.probe(hnode);
            try {
                assertEquals(emsg, found, res.await(tmout, TimeUnit.SECONDS));
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(emsg, count, dps.getPktCount());

            // if null case
            dps.clearPkt();
            res = dps.setLatch(existConnectors.size());
            hostFinder.probe(null);
            try {
                // In this state, NO packet is sent.
                assertFalse(emsg,
                            res.await(LATCH_FALSE_TIMEOUT, TimeUnit.SECONDS));
            } catch (Exception e) {
                unexpected(e);
            }
            assertEquals(emsg, 0, dps.getPktCount());

            if (map != null) {
                Status st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }
    }

    /**
     * test method for
     * {@link IVTNModeListener}
     */
    @Test
    public void testIVTNModeListener() {
        LOG.info("Running testIVTNModeListener().");

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
        }

        VTNModeListenerStub stub = new VTNModeListenerStub();
        Dictionary<String, Object> props = new Hashtable<String, Object>();
        ServiceRegistration listenerSeriveceReg = ServiceHelper.
            registerServiceWReg(IVTNModeListener.class,
                                GlobalConstants.DEFAULT.toString(),
                                stub, props);

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
    private void createTenantAndBridgeAndInterface(
        IVTNManager mgr, VTenantPath tpath, List<VBridgePath> bpaths,
        List<VBridgeIfPath> ifpaths) {

        Status st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertTrue(mgr.isActive());

        for (VBridgePath bpath : bpaths) {
            st = mgr.addBridge(bpath, new VBridgeConfig(null));
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        for (VBridgeIfPath ifpath : ifpaths) {
            VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
            st = mgr.addInterface(ifpath, ifconf);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
    }

    /**
     * Return a global stub OSGi service provided by this integration test
     * package.
     *
     * @param <T>  Type of service.
     * @param cls  The class of registered OSGi service.
     * @return     A OSGi service instance.
     */
    private <T> T getStubService(Class<T> cls) {
        try {
            for (ServiceReference<T> ref: bc.getServiceReferences(cls, null)) {
                T service = bc.getService(ref);
                if (service != null) {
                    Class<?> icls = service.getClass();
                    Package pkg = icls.getPackage();
                    if (STUB_PACKAGE.equals(pkg.getName())) {
                        return service;
                    }
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }
        return null;
    }

    /**
     * Return a container-specific stub OSGi service provided by this
     * integration test package.
     *
     * @param <T>        Type of service.
     * @param cls        The class of registered OSGi service.
     * @param container  The name of the container.
     * @return     A OSGi service instance.
     */
    private <T> T getStubService(Class<T> cls, String container) {
        StringBuilder builder = new StringBuilder("(containerName=");
        String f = builder.append(container).append(')').toString();
        try {
            for (ServiceReference<T> ref: bc.getServiceReferences(cls, f)) {
                T service = bc.getService(ref);
                if (service != null) {
                    Class<?> icls = service.getClass();
                    Package pkg = icls.getPackage();
                    if (STUB_PACKAGE.equals(pkg.getName())) {
                        return service;
                    }
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }
        return null;
    }
}
