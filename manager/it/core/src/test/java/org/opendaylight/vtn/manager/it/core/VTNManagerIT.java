/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.ops4j.pax.exam.CoreOptions.options;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Deque;
import java.util.Dictionary;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import javax.inject.Inject;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;

import org.ops4j.pax.exam.Configuration;
import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.junit.PaxExam;
import org.ops4j.pax.exam.spi.reactors.ExamReactorStrategy;
import org.ops4j.pax.exam.spi.reactors.PerClass;
import org.ops4j.pax.exam.util.Filter;

import org.osgi.framework.Bundle;
import org.osgi.framework.BundleContext;
import org.osgi.framework.ServiceReference;
import org.osgi.framework.ServiceRegistration;
import org.osgi.framework.Version;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.BundleVersion;
import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.IVTNGlobal;
import org.opendaylight.vtn.manager.IVTNManager;
import org.opendaylight.vtn.manager.IVTNManagerAware;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.MacMap;
import org.opendaylight.vtn.manager.MacMapConfig;
import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.PortMap;
import org.opendaylight.vtn.manager.PortMapConfig;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VInterfacePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.VlanMapConfig;
import org.opendaylight.vtn.manager.flow.cond.EthernetMatch;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.cond.Inet4Match;
import org.opendaylight.vtn.manager.flow.cond.TcpMatch;
import org.opendaylight.vtn.manager.flow.cond.UdpMatch;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.InetProtocols;

import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.option.TestOption;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.TestPort;
import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;
import org.opendaylight.vtn.manager.it.util.packet.ArpFactory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Icmp4Factory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;
import org.opendaylight.vtn.manager.it.util.unicast.ArpFlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.Tcp4FlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.Udp4FlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlowFactory;

import org.opendaylight.controller.configuration.IConfigurationContainerAware;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.hosttracker.hostAware.IHostFinder;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.core.UpdateType;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.ServiceHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;

@RunWith(PaxExam.class)
@ExamReactorStrategy(PerClass.class)
public final class VTNManagerIT extends ModelDrivenTestBase {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.
        getLogger(VTNManagerIT.class);

    // Inject the OSGI bundle context
    @Inject
    private BundleContext  bundleContext;

    // Inject VTN Manager services.
    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private IVTNManager vtnManager;

    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private IVTNGlobal vtnGlobal;

    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private OfMockService  ofMockService;

    private IConfigurationContainerAware  configContainerAware = null;
    private IHostFinder hostFinder = null;

    private Bundle  implBundle;

    /**
     * Configure the OSGi container
     */
    @Configuration
    public Option[] config() {
        return options(TestOption.vtnManagerCommonBundles());
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
    public void areWeReady() throws Exception {
        if (implBundle == null) {
            assertNotNull(bundleContext);
            assertNotNull(vtnManager);
            assertNotNull(vtnGlobal);
            assertNotNull(ofMockService);

            // Determine manager.implementation bundle.
            implBundle = getManagerBundle(bundleContext);
            assertNotNull(implBundle);
            assertEquals(Bundle.ACTIVE, implBundle.getState());

            ServiceReference r =
                bundleContext.getServiceReference(IVTNManager.class.getName());
            if (r != null) {
                this.configContainerAware =
                    (IConfigurationContainerAware)this.vtnManager;
                this.hostFinder = (IHostFinder)this.vtnManager;
            }

            // Initialize the openflowplugin mock-up.
            ofMockService.initialize();
        }
    }

    /**
     * Called when a test suite quits.
     *
     * @throws Exception  An error occurred.
     */
    @After
    public void tearDown() throws Exception {
        if (vtnManager == null) {
            return;
        }

        // Remove all global path maps.
        Status st = vtnManager.clearPathMap();
        if (st != null) {
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // Remove all flow conditions.
        st = vtnManager.clearFlowCondition();
        if (st != null) {
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // Remove all path policies.
        st = vtnManager.clearPathPolicy();
        if (st != null) {
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // Remove all VTNs in the default container.
        for (VTenant vtn: vtnManager.getTenants()) {
            String name = vtn.getName();
            LOG.debug("Clean up VTN: {}", name);
            VTenantPath path = new VTenantPath(name);
            st = vtnManager.removeTenant(path);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        // Reset the inventory configuration.
        ofMockService.reset();
    }

    /**
     * Test method for {@link IVTNManager}
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIVTNManager() throws Exception {
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

                                        @SuppressWarnings("deprecation")
                                        Integer oldiv =
                                            tenant.getIdleTimeoutValue();
                                        @SuppressWarnings("deprecation")
                                        Integer oldhv =
                                            tenant.getHardTimeoutValue();

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
                                Integer oldiv =
                                    (orgiv == null || orgiv.intValue() < 0)
                                    ? new Integer(300) : orgiv;
                                Integer oldhv =
                                    (orghv == null || orghv.intValue() < 0)
                                    ? new Integer(0) : orghv;

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
                            assertEquals(emsg, VnodeState.UNKNOWN, brdg.getState());

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
                                            VnodeState.UNKNOWN, brdg.getState());
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
                                        VnodeState.UNKNOWN, brdg.getState());
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
                                    assertEquals(emsg, VnodeState.DOWN, vif.getState());
                                } else {
                                    assertEquals(emsg, VnodeState.UNKNOWN, vif.getState());
                                }
                            }

                            if (isValidBPath) {
                                VBridge brdg = null;
                                try {
                                    brdg = mgr.getBridge(bpath);
                                } catch (VTNException e) {
                                    unexpected(e);
                                }
                                assertEquals(emsg, VnodeState.UNKNOWN, brdg.getState());
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
                                assertEquals(emsg, VnodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VnodeState.UNKNOWN, vif.getState());
                            }
                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                                unexpected(e);
                            }
                            assertEquals(emsg, VnodeState.UNKNOWN, brdg.getState());

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
                                assertEquals(emsg, VnodeState.DOWN, vif.getState());
                            } else {
                                assertEquals(emsg, VnodeState.UNKNOWN, vif.getState());
                            }

                            VBridge brdg = null;
                            try {
                                brdg = mgr.getBridge(bpath);
                            } catch (VTNException e) {
                                unexpected(e);
                            }
                            assertEquals(emsg, VnodeState.UNKNOWN, brdg.getState());
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
                             (node == null) ? VnodeState.UP
                             : VnodeState.DOWN, brdg.getState());

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
                assertEquals(emsg, (node == null) ? VnodeState.UP
                             : VnodeState.DOWN, brdg.getState());
            } else {
                assertEquals(0, list.size());
                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, VnodeState.UNKNOWN, brdg.getState());
            }

            for (VlanMap map : list) {
                st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg + ",(VlanMap)" + map.toString(),
                             StatusCode.SUCCESS, st.getCode());
            }
        }

        // Create VLAN mappin with specifying existing node.
        for (String nid: ofMockService.getNodes()) {
            Node node = toAdNode(nid);
            boolean noEdge = ofMockService.getPorts(nid, true).isEmpty();
            for (short vlan: vlans) {
                if (vlan < 0 || vlan >= 4096) {
                    continue;
                }

                VlanMapConfig vlconf = new VlanMapConfig(node, vlan);
                String emsg = "(VlanMapConfig)" + vlconf.toString();
                VlanMap vmap = null;
                try {
                    vmap = mgr.addVlanMap(bpath, vlconf);
                } catch (Exception e) {
                    unexpected(e);
                }

                assertNotNull(emsg, vmap);

                VBridge vbr = null;
                try {
                    vbr = mgr.getBridge(bpath);
                } catch (Exception e) {
                    unexpected(e);
                }

                VnodeState expected = (noEdge)
                    ? VnodeState.DOWN
                    : VnodeState.UP;
                assertEquals(emsg, expected, vbr.getState());

                st = mgr.removeVlanMap(bpath, vmap.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
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
            new SwitchPort("port-10", NodeConnectorIDType.OPENFLOW, "10"),
            new SwitchPort(null, NodeConnectorIDType.OPENFLOW, "11"),
            new SwitchPort("port-10", null, null),
            new SwitchPort("port-10"),
            new SwitchPort(NodeConnectorIDType.OPENFLOW, "13"),
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
                assertEquals(emsg, VnodeState.DOWN, bif.getState());

                VBridge brdg = null;
                try {
                    brdg = mgr.getBridge(bpath);
                } catch (VTNException e) {
                    unexpected(e);
                }
                assertEquals(emsg, VnodeState.DOWN, brdg.getState());
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
            assertEquals(VnodeState.UNKNOWN, vif.getState());
        } catch (VTNException e) {
            unexpected(e);
        }

        // Determine an edge port to be mapped by port mapping.
        List<String> allNodes = ofMockService.getNodes();
        List<String> edgePorts = null;
        for (String nid: allNodes) {
            edgePorts = ofMockService.getPorts(nid, true);
            if (!edgePorts.isEmpty()) {
                break;
            }
        }

        assertTrue(edgePorts != null && !edgePorts.isEmpty());
        String edgePort = edgePorts.get(0);
        NodeConnector edgeNc = toAdNodeConnector(edgePort);
        Node node2 = edgeNc.getNode();
        String portId = edgeNc.getNodeConnectorIDString();

        String ncType = NodeConnector.NodeConnectorIDType.OPENFLOW;
        SwitchPort swport = new SwitchPort(null, ncType, portId);
        pmconf1 = new PortMapConfig(node2, swport, (short)10);
        st = mgr.setPortMap(ifp1, pmconf1);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        try {
            VInterface vif = mgr.getInterface(ifp1);
            assertEquals(VnodeState.UP, vif.getState());

            VBridge vbr = mgr.getBridge(new VBridgePath(tname, bname1));
            assertEquals(VnodeState.UP, vbr.getState());
        } catch (Exception e) {
            unexpected(e);
        }

        pmconf2 = new PortMapConfig(node2, swport, (short)10);
        st = mgr.setPortMap(ifp2, pmconf2);
        assertEquals(StatusCode.CONFLICT, st.getCode());

        String portName = ofMockService.getPortName(edgePort);
        swport = new SwitchPort(portName, null, null);
        PortMapConfig pmconf3 = new PortMapConfig(node2, swport, (short)10);
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
     *
     * @throws Exception  An error occurred.
     */
    private void testProbeHost() throws Exception {
        LOG.info("Running testProbeHost().");

        IVTNManager mgr = vtnManager;
        Set<String> allPorts = new HashSet<>();
        List<TestPort> edgePorts = getEdgePorts(allPorts);

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

        // Null host should be ignored.
        HostNodeConnector hnc = null;
        String emsg = "(host)(null)";
        assertFalse(emsg, mgr.probeHost(hnc));
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // Host without specifying node connector should be ignored.
        hnc = new HostNodeConnector(mac, ip4addr, null, (short)0);
        emsg = "(host)" + hnc.toString();
        assertFalse(emsg, mgr.probeHost(hnc));
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // No ARP request should be transmitted if no vBridge is present.
        for (TestPort tp: edgePorts) {
            NodeConnector nc = tp.getNodeConnector();
            hnc = new HostNodeConnector(mac, ip4addr, nc, tp.getVlan());
            emsg = "(host)" + hnc.toString();
            assertFalse(emsg, mgr.probeHost(hnc));
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }
        }

        // Create VTN.
        VTenantPath tpath = new VTenantPath("vtn");
        Status st = mgr.addTenant(tpath, new VTenantConfig("for Test"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Create vBridges, and establish port mappings.
        Map<TestPort, VBridgePath> bridgeMap =
            setUpPortMapping(tpath, edgePorts);

        // Ensure that an ARP request is sent to each host.
        byte[] ctlrMac = ofMockService.getControllerMacAddress();
        EthernetFactory efc = new EthernetFactory(ctlrMac, mac);
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(ctlrMac).
            setTargetHardwareAddress(mac).
            setSenderProtocolAddress(IPV4_ZERO).
            setTargetProtocolAddress(ip4addr.getAddress());
        for (TestPort tp: bridgeMap.keySet()) {
            NodeConnector nc = tp.getNodeConnector();
            hnc = new HostNodeConnector(mac, ip4addr, nc, tp.getVlan());
            emsg = "(host)" + hnc.toString();
            assertTrue(emsg, mgr.probeHost(hnc));

            String pid = tp.getPortIdentifier();
            byte[] payload = ofMockService.awaitTransmittedPacket(pid);
            efc.setVlanId(tp.getVlan()).verify(ofMockService, payload);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }

            // IPv6 address should be ignored.
            hnc = new HostNodeConnector(mac, ip6addr, nc, tp.getVlan());
            assertFalse(emsg, mgr.probeHost(hnc));
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }
        }

        // Host on unknown node connector should be ignored.
        TestPort edge0 = edgePorts.get(0);
        Node node = edge0.getNodeConnector().getNode();
        Short idDead = new Short((short)0xDEAD);
        NodeConnector ncDead = new NodeConnector(
            NodeConnector.NodeConnectorIDType.OPENFLOW, idDead, node);
        SwitchPort swport =
            new SwitchPort(ncDead.getType(), idDead.toString());
        VBridgePath bpath0 = bridgeMap.get(edge0);
        VBridgeIfPath vifDead =
            new VBridgeIfPath(bpath0, "vifDead");
        st = mgr.addInterface(vifDead,
                              new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        PortMapConfig pmconfDead =
            new PortMapConfig(ncDead.getNode(), swport, edge0.getVlan());
        st = mgr.setPortMap(vifDead, pmconfDead);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        VInterface vif = mgr.getInterface(vifDead);
        assertEquals(VnodeState.DOWN, vif.getState());
        assertEquals(VnodeState.UNKNOWN, vif.getEntityState());
        VBridge vbr = vtnManager.getBridge(bpath0);
        assertEquals(VnodeState.DOWN, vbr.getState());

        hnc = new HostNodeConnector(mac, ip4addr, ncDead, edge0.getVlan());
        emsg = "(host)" + hnc.toString();
        assertFalse(emsg, mgr.probeHost(hnc));
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // Map all edge ports to one vBridge.
        for (VBridgePath bpath: bridgeMap.values()) {
            st = mgr.removeBridge(bpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        VBridgePath bpath1 = new VBridgePath(tpath, "vbridge");
        Set<TestPort> portSet = bridgeMap.keySet();
        mapAllPorts(bpath1, portSet);

        for (TestPort tp: bridgeMap.keySet()) {
            NodeConnector nc = tp.getNodeConnector();
            hnc = new HostNodeConnector(mac, ip4addr, nc, tp.getVlan());
            emsg = "(host)" + hnc.toString();
            assertTrue(emsg, mgr.probeHost(hnc));

            String pid = tp.getPortIdentifier();
            byte[] payload = ofMockService.awaitTransmittedPacket(pid);
            efc.setVlanId(tp.getVlan()).verify(ofMockService, payload);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }
        }

        // Clean up.
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Test case for {@link IVTNManager#findHost(InetAddress, Set)}.
     *
     * @throws Exception  An error occurred.
     */
    private void testFindHost() throws Exception {
        LOG.info("Running testFindHost().");

        IVTNManager mgr = vtnManager;
        VTenantPath tpath = new VTenantPath("vtn");
        Set<String> allPorts = new HashSet<>();
        List<TestPort> edgePorts = getEdgePorts(allPorts);

        // Create VTN.
        Status st = mgr.addTenant(tpath, new VTenantConfig("for Test"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Create vBridges, and establish port mappings.
        Map<TestPort, VBridgePath> bridgeMap =
            setUpPortMapping(tpath, edgePorts);

        byte[] ctlrMac = ofMockService.getControllerMacAddress();
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
        EthernetFactory efc = new EthernetFactory(ctlrMac, MAC_BROADCAST);
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(ctlrMac).
            setTargetHardwareAddress(MAC_ZERO).
            setSenderProtocolAddress(IPV4_ZERO).
            setTargetProtocolAddress(ip4addr.getAddress());
        for (Map.Entry<TestPort, VBridgePath> entry: bridgeMap.entrySet()) {
            TestPort tp = entry.getKey();
            VBridgePath bpath = entry.getValue();
            String pid = tp.getPortIdentifier();
            Set<VBridgePath> bpathSet = Collections.singleton(bpath);

            // for IPv4 Address
            mgr.findHost(ip4addr, bpathSet);
            byte[] payload = ofMockService.awaitTransmittedPacket(pid);
            efc.setVlanId(tp.getVlan()).verify(ofMockService, payload);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }

            // Request for IPv6 host should be ignored.
            mgr.findHost(ip6addr, bpathSet);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }

            // Null IP address should be ignored.
            mgr.findHost(null, bpathSet);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }
        }

        // Specifying multiple vBridges.
        Map<String, Set<Short>> mappedVlans = new HashMap<>();
        for (TestPort tp: bridgeMap.keySet()) {
            String pid = tp.getPortIdentifier();
            Short vid = Short.valueOf(tp.getVlan());
            Set<Short> vids = mappedVlans.get(pid);
            if (vids == null) {
                vids = new HashSet<Short>();
                mappedVlans.put(pid, vids);
            }
            assertTrue(vids.add(vid));
        }

        Set<VBridgePath> allBridges = new HashSet<>(bridgeMap.values());
        mgr.findHost(ip4addr, allBridges);
        for (Map.Entry<String, Set<Short>> entry: mappedVlans.entrySet()) {
            String pid = entry.getKey();
            Set<Short> vids = entry.getValue();
            int count = vids.size();
            for (int i = 0; i < count; i++) {
                byte[] payload = ofMockService.awaitTransmittedPacket(pid);
                vids = efc.verify(ofMockService, payload, vids);
            }
            assertTrue(vids.isEmpty());
        }
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // Specifying null set.
        // ARP request should be sent to all vBridges.
        mgr.findHost(ip4addr, null);
        for (Map.Entry<String, Set<Short>> entry: mappedVlans.entrySet()) {
            String pid = entry.getKey();
            Set<Short> vids = entry.getValue();
            int count = vids.size();
            for (int i = 0; i < count; i++) {
                byte[] payload = ofMockService.awaitTransmittedPacket(pid);
                vids = efc.verify(ofMockService, payload, vids);
            }
            assertTrue(vids.isEmpty());
        }
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // Invalid port
        Node nodeDead = NodeCreator.createOFNode(221L);
        Short ncIDDead = new Short((short)221);
        NodeConnector ncDead = NodeConnectorCreator.createNodeConnector(
            NodeConnectorIDType.OPENFLOW, ncIDDead, nodeDead);
        VBridgePath vbrDead = new VBridgePath(tpath, "vbrDead");
        st = mgr.addBridge(vbrDead, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        Set<VBridgePath> setDead = new HashSet<VBridgePath>();
        setDead.add(vbrDead);

        VBridgeIfPath vifDead = new VBridgeIfPath(vbrDead, "vifDead");
        st = mgr.addInterface(vifDead,
                              new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        SwitchPort swportDead = new SwitchPort(ncDead, ncIDDead.toString());
        PortMapConfig pmconfDead =
            new PortMapConfig(nodeDead, swportDead, (short)221);
        st = mgr.setPortMap(vifDead, pmconfDead);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        mgr.findHost(ip4addr, setDead);

        // No ARP request should be transmitted.
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // Map all edge ports to one vBridge.
        for (VBridgePath bpath: bridgeMap.values()) {
            st = mgr.removeBridge(bpath);
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }

        VBridgePath bpath1 = new VBridgePath(tpath, "vbridge");
        Set<TestPort> portSet = bridgeMap.keySet();
        mapAllPorts(bpath1, portSet);
        mgr.findHost(ip4addr, Collections.singleton(bpath1));

        for (Map.Entry<String, Set<Short>> entry: mappedVlans.entrySet()) {
            String pid = entry.getKey();
            Set<Short> vids = entry.getValue();
            int count = vids.size();
            for (int i = 0; i < count; i++) {
                byte[] payload = ofMockService.awaitTransmittedPacket(pid);
                vids = efc.verify(ofMockService, payload, vids);
            }
            assertTrue(vids.isEmpty());
        }
        sleep(SHORT_DELAY);
        for (String p: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }

        // Clean up.
        st = mgr.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Collect all edge ports for test.
     *
     * @param portSet  A set to store all port identifiers, including
     *                 ISL ports.
     * @return  A list of {@link TestPort} instances.
     */
    private List<TestPort> getEdgePorts(Set<String> portSet) {
        List<TestPort> edgePorts = new ArrayList<>();
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                assertTrue(portSet.add(pid));
                edgePorts.add(new TestPort(pid));
            }
            for (String pid: ofMockService.getPorts(nid, false)) {
                assertTrue(portSet.add(pid));
            }
        }

        assertFalse(edgePorts.isEmpty());
        assertTrue(portSet.size() > edgePorts.size());

        return edgePorts;
    }

    /**
     * Set up port mappings for test.
     *
     * <p>
     *   This method maps all the ports in the given list to individual
     *   vBridges.
     * </p>
     *
     * @param tpath     Path to the VTN.
     * @param portList  A list of {@link TestPort} instances corresponding to
     *                  edge ports.
     * @return  A map that keeps vBridges associated with the given ports.
     * @throws Exception  An error occurred.
     */
    private Map<TestPort, VBridgePath> setUpPortMapping(
        VTenantPath tpath, List<TestPort> portList) throws Exception {
        VBridgeConfig bconf = new VBridgeConfig(null);
        VInterfaceConfig iconf = new VInterfaceConfig(null, Boolean.TRUE);

        Map<TestPort, VBridgePath> bridgeMap = new HashMap<>();
        short[] vlans = {0, 1, 100, 2048, 4095};
        int idx = 1;
        for (short vlan: vlans) {
            for (TestPort tp: portList) {
                VBridgePath bpath = new VBridgePath(tpath, "vbr" + idx);
                VBridgeIfPath ipath = new VBridgeIfPath(bpath, "vif");
                createVirtualNodes(null, Collections.singleton(bpath),
                                   Collections.singleton(ipath));

                NodeConnector nc = tp.getNodeConnector();
                String ncId = nc.getNodeConnectorIDString();
                SwitchPort swport = new SwitchPort(nc.getType(), ncId);
                PortMapConfig pmconf =
                    new PortMapConfig(nc.getNode(), swport, vlan);
                Status st = vtnManager.setPortMap(ipath, pmconf);
                assertEquals(StatusCode.SUCCESS, st.getCode());

                VBridge vbr = vtnManager.getBridge(bpath);
                assertEquals(VnodeState.UP, vbr.getState());

                TestPort tport = tp.clone();
                tport.setVlan(vlan);
                assertEquals(null, bridgeMap.put(tport, bpath));
                idx++;
            }
        }

        return bridgeMap;
    }

    /**
     * Create a new vBridge, and map all the given edge ports to that vBridge.
     *
     * @param bpath  A path to the vBridge.
     * @param ports  A collection of {@link TestPort} instances corresponding
     *               to VLAN on edge ports.
     * @throws Exception  An error occurred.
     */
    private void mapAllPorts(VBridgePath bpath, Collection<TestPort> ports)
        throws Exception {
        Status st = vtnManager.addBridge(bpath, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        VInterfaceConfig iconf = new VInterfaceConfig(null, Boolean.TRUE);
        int idx = 1;
        for (TestPort tp: ports) {
            VBridgeIfPath ipath = new VBridgeIfPath(bpath, "if_" + idx);
            st = vtnManager.addInterface(ipath, iconf);
            assertEquals(StatusCode.SUCCESS, st.getCode());

            NodeConnector nc = tp.getNodeConnector();
            String ncId = nc.getNodeConnectorIDString();
            SwitchPort swport = new SwitchPort(nc.getType(), ncId);
            PortMapConfig pmconf =
                new PortMapConfig(nc.getNode(), swport, tp.getVlan());
            st = vtnManager.setPortMap(ipath, pmconf);
            assertEquals(StatusCode.SUCCESS, st.getCode());
            idx++;
        }

        VBridge vbr = vtnManager.getBridge(bpath);
        assertEquals(VnodeState.UP, vbr.getState());
    }

    /**
     * Test case for {@link IVTNGlobal}.
     */
    @Test
    public void testIVTNGlobal() {
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
        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

        res = listener.restart(1);

        // add a tenant
        String tname = "tenant";
        VTenantPath tpath = new VTenantPath(tname);
        st = mgr.addTenant(tpath, new VTenantConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);

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

        res.await(TASK_TIMEOUT, TimeUnit.SECONDS);
        ups = listener.getUpdates();
        assertEquals(0, ups.size());
    }


    /**
     * Test method for MAC address table.
     *
     * <ul>
     *   <li>
     *     {@link IVTNManager#getMacEntries(VBridgePath)}
     *   </li>
     *   <li>
     *     {@link IVTNManager#getMacEntry(VBridgePath, DataLinkAddress)}
     *   </li>
     *   <li>
     *     {@link IVTNManager#removeMacEntry(VBridgePath, DataLinkAddress)}
     *   </li>
     *   <li>
     *     {@link IVTNManager#flushMacEntries(VBridgePath)}
     *   <li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMacEntry() throws Exception {
        LOG.info("Running testMacEntry().");

        class DataLinkAddressStub extends DataLinkAddress {
            private static final long serialVersionUID = -9043768232113080608L;

            @Override
            public DataLinkAddress clone() {
                return null;
            }
        }

        // Create one VTN and 2 vBridges.
        VTenantPath tpath = new VTenantPath("vtn");
        VBridgePath bpath1 = new VBridgePath(tpath, "vbr1");
        VBridgePath bpath2 = new VBridgePath(tpath, "vbr2");
        List<VBridgePath> bpathList = new ArrayList<>();
        Collections.addAll(bpathList, bpath1, bpath2);
        createVirtualNodes(tpath, bpathList);

        // Map VLAN 0, 1 and 2, 3 to bpath1 and bpath2 respectively.
        short vlan = 0;
        for (VBridgePath bpath: bpathList) {
            for (short s = 0; s < 2; s++) {
                VlanMapConfig vlconf = new VlanMapConfig(null, vlan);
                vtnManager.addVlanMap(bpath, vlconf);
                vlan++;
            }
            VNodeStateWaiter waiter = new VNodeStateWaiter(vtnManager).
                set(bpath, VnodeState.UP);
            waiter.await();
        }

        // Collect edge ports, and create test hosts.
        BridgeNetwork bridge1 = new BridgeNetwork(bpath1);
        BridgeNetwork bridge2 = new BridgeNetwork(bpath2);
        Set<MacAddressEntry> ments1 = new HashSet<>();
        Set<MacAddressEntry> ments2 = new HashSet<>();
        Set<String> islPorts = new HashSet<>();
        int idx = 1;
        final int nhosts = 2;
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                for (vlan = 0; vlan <= 1; vlan++) {
                    for (int i = 0; i < nhosts; i++) {
                        TestHost th = new TestHost(idx, pid, vlan);
                        bridge1.addHost(nid, th);
                        assertTrue(ments1.add(th.getMacAddressEntry()));
                        idx++;
                    }
                }

                Set<Short> vids = new HashSet<>();
                for (vlan = 2; vlan <= 3; vlan++) {
                    for (int i = 0; i < nhosts; i++) {
                        TestHost th = new TestHost(idx, pid, vlan);
                        bridge2.addHost(nid, th);
                        assertTrue(ments2.add(th.getMacAddressEntry()));
                        idx++;
                    }
                }
            }

            for (String pid: ofMockService.getPorts(nid, false)) {
                bridge1.setUnmappedPort(pid);
                bridge2.setUnmappedPort(pid);
                assertTrue(islPorts.add(pid));
            }
        }
        bridge1.verify();
        bridge2.verify();
        assertFalse(islPorts.isEmpty());

        checkMacAddressTable(bpath1, null);
        checkMacAddressTable(bpath2, null);

        // Let the vBridge at bpath1 learn MAC addresses.
        learnHosts(bridge1);
        checkMacAddressTable(bpath1, ments1);
        checkMacAddressTable(bpath2, null);

        // Let the vBridge at bpath2 learn MAC addresses.
        learnHosts(bridge2);
        checkMacAddressTable(bpath1, ments1);
        checkMacAddressTable(bpath2, ments2);

        // Error tests.
        byte[] unknownMac = {
            (byte)0xa0, (byte)0xb0, (byte)0xc0,
            (byte)0xd0, (byte)0xe0, (byte)0xf0,
        };
        EthernetAddress unknownEther = new EthernetAddress(unknownMac);
        VBridgePath[] badPaths = {
            null,
            new VBridgePath((String)null, "vbr"),
            new VBridgePath("vtn", null),
        };

        for (VBridgePath bpath: badPaths) {
            macEntryErrorTest(bpath, unknownEther, StatusCode.BADREQUEST);
        }

        VBridgePath unknownBridge = new VBridgePath(tpath, "unknown");
        macEntryErrorTest(unknownBridge, unknownEther, StatusCode.NOTFOUND);

        for (VBridgePath bpath: bpathList) {
            macEntryErrorTest(bpath, null, StatusCode.BADREQUEST);
        }

        // Install unicast flow entries.
        List<UnicastFlow> flows1 = unicastTest(bridge1, islPorts, true);
        List<UnicastFlow> flows2 = unicastTest(bridge2, islPorts, true);

        // Move all hosts on untagged network to VLAN 1.
        Map<String, List<TestHost>> edgeMap1 = bridge1.getTestHosts();
        Map<String, List<TestHost>> edgeMap2 = bridge2.getTestHosts();
        Map<TestHost, TestHost> oldHosts0 = new HashMap<>();
        for (Map.Entry<String, List<TestHost>> entry: edgeMap1.entrySet()) {
            List<TestHost> hosts = new ArrayList<>();
            for (TestHost th: entry.getValue()) {
                if (th.getVlan() == 0) {
                    TestHost host = new TestHost(th, (short)1);
                    hosts.add(host);
                    assertEquals(null, oldHosts0.put(th, host));
                } else {
                    hosts.add(th);
                }
            }
            entry.setValue(hosts);
        }

        checkMacAddressTable(bpath1, ments1);
        checkMacAddressTable(bpath2, ments2);

        // Query unknown MAC addresses.
        DataLinkAddress[] dladdrs = {
            new DataLinkAddressStub(),
            new EthernetAddress(MAC_BROADCAST),
            new EthernetAddress(MAC_ZERO),
        };
        for (VBridgePath bpath: bpathList) {
            for (DataLinkAddress dladdr: dladdrs) {
                assertEquals(null, vtnManager.getMacEntry(bpath, dladdr));
                assertEquals(null, vtnManager.removeMacEntry(bpath, dladdr));
            }
        }

        // Send ICMP packet from moved hosts.
        // This invalidates old MAC address table entries.
        InetAddress dstIp = InetAddress.getByName("192.168.100.255");
        for (Map.Entry<TestHost, TestHost> entry: oldHosts0.entrySet()) {
            TestHost oldHost = entry.getKey();
            TestHost newHost = entry.getValue();
            assertTrue(ments1.remove(oldHost.getMacAddressEntry()));
            sendBroadcastIcmp(newHost, dstIp, bridge1.getMappedVlans());

            // IP address in ICMP packet should not be copied into MAC address
            // table entry.
            MacAddressEntry ment = newHost.getMacAddressEntry(false);
            assertTrue(ments1.add(ment));
            checkMacAddressTable(bpath1, ments1);
            EthernetAddress eaddr = newHost.getEthernetAddress();
            assertEquals(ment, vtnManager.getMacEntry(bpath1, eaddr));
        }

        // All flow entries for an entry should be uninstalled.
        final int tableId = OfMockService.DEFAULT_TABLE;
        for (UnicastFlow unicast: flows1) {
            for (OfMockFlow flow: unicast.getFlowList()) {
                Match match = flow.getMatch();
                if (getVlanMatch(match) == (short)0) {
                    String nid = flow.getNodeIdentifier();
                    int pri = flow.getPriority();
                    OfMockFlow newFlow = ofMockService.
                        awaitFlow(nid, tableId, match, pri, false);
                    assertEquals(null, newFlow);
                }
            }
        }

        // bpath2 should not be affected.
        checkMacAddressTable(bpath2, ments2);
        UnicastFlow.verifyFlows(flows2, true, false);

        // Remove all MAC address table entries in bpath1.
        for (Iterator<MacAddressEntry> it = ments1.iterator(); it.hasNext();) {
            MacAddressEntry ment = it.next();
            it.remove();

            DataLinkAddress dladdr = ment.getAddress();
            assertEquals(ment, vtnManager.removeMacEntry(bpath1, dladdr));
            assertEquals(null, vtnManager.getMacEntry(bpath1, dladdr));
            assertEquals(null, vtnManager.removeMacEntry(bpath1, dladdr));
            checkMacAddressTable(bpath1, ments1);
        }
        checkMacAddressTable(bpath1, null);
        checkMacAddressTable(bpath2, ments2);

        // Flush MAC address table in bpath2.
        Status st = vtnManager.flushMacEntries(bpath2);
        assertTrue(st.isSuccess());
        for (MacAddressEntry ment: ments2) {
            DataLinkAddress dladdr = ment.getAddress();
            assertEquals(null, vtnManager.getMacEntry(bpath2, dladdr));
            assertEquals(null, vtnManager.removeMacEntry(bpath2, dladdr));
        }
        checkMacAddressTable(bpath2, null);
        st = vtnManager.flushMacEntries(bpath2);
        assertTrue(st.isSuccess());

        // Clean up.
        st = vtnManager.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Send an ICMPv4 broadcast packet.
     *
     * @param host      Source host.
     * @param dstIp     Destination IP address.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must be
     *                  associated with the key.
     */
    private void sendBroadcastIcmp(TestHost host, InetAddress dstIp,
                                   Map<String, Set<Short>> allPorts)
        throws Exception {
        String pid = host.getPortIdentifier();
        byte[] src = host.getMacAddress();
        InetAddress srcIp = host.getInetAddress();
        short vlan = host.getVlan();
        byte type = 8;
        byte code = 0;

        EthernetFactory efc = new EthernetFactory(src, MAC_BROADCAST).
            setVlanId(vlan);
        Inet4Factory i4fc = Inet4Factory.newInstance(efc, srcIp, dstIp);
        Icmp4Factory ic4fc = Icmp4Factory.newInstance(i4fc, type, code);
        byte[] raw = {
            (byte)0x00, (byte)0x11, (byte)0x22, (byte)0x33,
            (byte)0x44, (byte)0x55, (byte)0x66, (byte)0x77,
            (byte)0x88, (byte)0x99, (byte)0xaa, (byte)0xbb,
            (byte)0xcc, (byte)0xdd, (byte)0xee, (byte)0xff,
            (byte)0x12,
        };
        ic4fc.setRawPayload(raw);
        byte[] payload = efc.create();
        ofMockService.sendPacketIn(pid, payload);

        for (Map.Entry<String, Set<Short>> entry: allPorts.entrySet()) {
            Set<Short> vids = entry.getValue();
            if (vids == null) {
                continue;
            }

            String portId = entry.getKey();
            if (portId.equals(pid)) {
                // VTN Manager will send an ARP request to probe IP address.
                efc.setProbe(srcIp, vlan);
            } else {
                efc.setProbe(null, (short)-1);
            }

            int count = vids.size();
            for (int i = 0; i < count; i++) {
                byte[] transmitted =
                    ofMockService.awaitTransmittedPacket(portId);
                vids = efc.verify(ofMockService, transmitted, vids);
            }
            assertTrue(vids.isEmpty());
        }

        for (String p: allPorts.keySet()) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }
    }

    /**
     * Run error test for MAC address table APIs.
     *
     * @param bpath   Path to the vBridge.
     * @param dladdr  A data link layer address.
     * @param code    Expected status code.
     */
    private void macEntryErrorTest(VBridgePath bpath, DataLinkAddress dladdr,
                                   StatusCode code) {
        try {
            vtnManager.getMacEntry(bpath, dladdr);
            unexpected();
        } catch (VTNException e) {
            assertEquals(code, e.getStatus().getCode());
        }

        try {
            vtnManager.removeMacEntry(bpath, dladdr);
            unexpected();
        } catch (VTNException e) {
            assertEquals(code, e.getStatus().getCode());
        }

        // If dladdr is not null, bpath may be valid.
        if (dladdr != null) {
            try {
                vtnManager.getMacEntries(bpath);
                unexpected();
            } catch (VTNException e) {
                assertEquals(code, e.getStatus().getCode());
            }

            Status st = vtnManager.flushMacEntries(bpath);
            assertEquals(code, st.getCode());
        }
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
    }

    /**
     * test method for {@link IConfigurationContainerAware}
     */
    @Test
    public void testIConfigurationContainerAware() {
        LOG.info("Running testIConfigurationContainerAware().");
        Status st = configContainerAware.saveConfiguration();
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // Create 2 VTNs.
        for (int i = 0; i < 2; i++) {
            VTenantPath tpath = new VTenantPath("vtn" + i);
            List<VBridgePath> bpaths = new ArrayList<>();
            for (int j = 0; j < 4; j++) {
                VBridgePath bpath = new VBridgePath(tpath, "vbr" + j);
                bpaths.add(bpath);
            }
            createVirtualNodes(tpath, bpaths);
        }

        st = configContainerAware.saveConfiguration();
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Ensure that the vBridge and interface state are changed according to
     * inventory events.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInventoryEvent() throws Exception {
        LOG.info("Running testInventoryEvent().");

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        List<VBridgePath> bpathList = new ArrayList<>();
        List<VBridgeIfPath> bipathList = new ArrayList<>();
        List<VTerminalPath> vtpathList = new ArrayList<>();
        List<VTerminalIfPath> vtipathList = new ArrayList<>();

        // Create a vBridge that does not map any network.
        VBridgePath bpathNomap = new VBridgePath(tpath, "nomap");
        VBridgeIfPath bipathNomap = new VBridgeIfPath(bpathNomap, "if");
        bpathList.add(bpathNomap);
        bipathList.add(bipathNomap);

        // Create a vBridge that maps all untagged network.
        VBridgePath bpathVlan0 = new VBridgePath(tpath, "vlan0");
        bpathList.add(bpathVlan0);

        // Create a vBridge that maps VLAN 1 on node 1.
        VBridgePath bpathNode1 = new VBridgePath(tpath, "node1");
        bpathList.add(bpathNode1);

        // Create a vBridge that maps port 1 on node 2.
        VBridgePath bpathNode2Port1 = new VBridgePath(tpath, "node2_port1");
        VBridgeIfPath bipathNode2Port1 =
            new VBridgeIfPath(bpathNode2Port1, "if");
        bpathList.add(bpathNode2Port1);
        bipathList.add(bipathNode2Port1);

        // Create a vBridge that maps a host at port 2 on node 2 using
        // MAC mapping.
        VBridgePath bpathMac = new VBridgePath(tpath, "mac");
        bpathList.add(bpathMac);

        // Create a vTerminal that does not map any network.
        VTerminalPath vtpathNomap = new VTerminalPath(tpath, "nomap");
        VTerminalIfPath vtipathNomap = new VTerminalIfPath(vtpathNomap, "if");
        vtpathList.add(vtpathNomap);
        vtipathList.add(vtipathNomap);

        // Create a vTerminal that maps port 3 on node 1.
        VTerminalPath vtpathNode1Port3 =
            new VTerminalPath(tpath, "node1_port3");
        VTerminalIfPath vtipathNode1Port3 =
            new VTerminalIfPath(vtpathNode1Port3, "if");
        vtpathList.add(vtpathNode1Port3);
        vtipathList.add(vtipathNode1Port3);

        createVirtualNodes(tpath, bpathList, bipathList, vtpathList,
                           vtipathList);
        VNodeStateWaiter waiter = new VNodeStateWaiter(vtnManager).
            set(bpathNomap, VnodeState.UNKNOWN).
            set(bipathNomap, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            set(bpathVlan0, VnodeState.UNKNOWN).
            set(bpathNode1, VnodeState.UNKNOWN).
            set(bpathNode2Port1, VnodeState.UNKNOWN).
            set(bipathNode2Port1, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            set(bpathMac, VnodeState.UNKNOWN).
            set(vtpathNomap, VnodeState.UNKNOWN).
            set(vtipathNomap, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            set(vtpathNode1Port3, VnodeState.UNKNOWN).
            set(vtipathNode1Port3, VnodeState.UNKNOWN, VnodeState.UNKNOWN);
        waiter.await();

        // Map VLAN 0 to bpathVlan0.
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)0);
        VlanMap vmap0 = vtnManager.addVlanMap(bpathVlan0, vlconf);
        waiter.set(bpathVlan0, VnodeState.UP).await();

        // Map VLAN 1 on node1 to bpathNode1.
        BigInteger dpid1 = BigInteger.ONE;
        String nid1 = ID_OPENFLOW + dpid1;
        Node node1 = toAdNode(nid1);
        vlconf = new VlanMapConfig(node1, (short)1);
        VlanMap vmap1 = vtnManager.addVlanMap(bpathNode1, vlconf);
        waiter.set(bpathNode1, VnodeState.DOWN).await();

        // Map VLAN 2 on port1 on node 2 to bpathNode2Port1.
        BigInteger dpid2 = BigInteger.valueOf(2L);
        String nid2 = ID_OPENFLOW + dpid2;
        Node node2 = toAdNode(nid2);
        SwitchPort swport = new SwitchPort(NodeConnectorIDType.OPENFLOW, "1");
        PortMapConfig pmconf = new PortMapConfig(node2, swport, (short)2);
        Status st = vtnManager.setPortMap(bipathNode2Port1, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();

        // Map a host at port 2 on node 2 to bpathMac.
        String pid22 = OfMockUtils.getPortIdentifier(nid2, 2L);
        TestHost host = new TestHost(1, pid22, (short)4095);
        int hostIdx = 2;
        Set<EthernetHost> dlhosts =
            Collections.singleton(host.getEthernetHost());
        assertEquals(UpdateType.ADDED,
                     vtnManager.setMacMap(bpathMac, VtnUpdateOperationType.ADD,
                                          VtnAclType.ALLOW, dlhosts));
        waiter.set(bpathMac, VnodeState.DOWN).await();

        // Map VLAN 10 on port 3 on node 1 to vtpathNode1Port3.
        swport = new SwitchPort(NodeConnectorIDType.OPENFLOW, "3");
        pmconf = new PortMapConfig(node1, swport, (short)10);
        st = vtnManager.setPortMap(vtipathNode1Port3, pmconf);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        waiter.set(vtpathNode1Port3, VnodeState.DOWN).
            set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();

        List<String> nodeIds = new ArrayList<>();
        final long nports = 4L;
        final int npids = (int)nports + 1;
        String[][] pids = {
            null,
            new String[npids],
            new String[npids],
            new String[npids],
        };

        // Below test causes NumberFormatException in sal-compatibility.

        // // Create a non-OpenFlow node.
        // // This should never affect virtual node state.
        // String badProto = "unknown:";
        // String badNid = badProto + dpid1;
        // nodeIds.add(badNid);
        // ofMockService.addNode(badProto, dpid1);
        // String[] badPorts = new String[npids];
        // for (long idx = 1; idx <= nports; idx++) {
        //     badPorts[idx] = ofMockService.addPort(badNid, idx, false);
        // }
        // for (int i = 1; i < npids; i++) {
        //     ofMockService.awaitPortCreated(badPorts[i]);
        // }
        // waiter.await();

        // Create node 3 that will not be mapped to virtual nodes except
        // bpathVlan0. This should never affect virtual node state because
        // no edge port is available yet.
        BigInteger dpid3 = BigInteger.valueOf(3L);
        String nid3 = ID_OPENFLOW + dpid3;
        ofMockService.addNode(dpid3);
        for (long idx = 1; idx <= nports; idx++) {
            pids[3][(int)idx] = ofMockService.addPort(nid3, idx, false);
        }
        for (int i = 1; i < npids; i++) {
            ofMockService.awaitPortCreated(pids[3][i]);
        }
        waiter.await();

        // Up port 2 on node 3.
        // This will activate bpathVlan0.
        ofMockService.setPortState(pids[3][2], true, false);
        waiter.set(bpathVlan0, VnodeState.UP).await();

        // Create node 1.
        // This should never affect virtual node state because no edge port
        // is available yet.
        ofMockService.addNode(dpid1);
        waiter.await();

        // Add ports to node 1 except port 3.
        // This should never affect virtual node state because:
        //   - Port 3, which will be mapped to vtipathNode1Port3, is not yet
        //     added.
        //   - All edge ports are down.
        for (long idx = 1; idx <= nports; idx++) {
            if (idx != 3) {
                pids[1][(int)idx] = ofMockService.addPort(nid1, idx, false);
            }
        }
        for (int i = 1; i < npids; i++) {
            if (i != 3) {
                ofMockService.awaitPortCreated(pids[1][i]);
            }
        }
        waiter.await();

        // Enable port 4 on node 1.
        // This will activate bpathNode1.
        ofMockService.setPortState(pids[1][4], true, false);
        waiter.set(bpathNode1, VnodeState.UP).await();

        // Add port 3 on node 1.
        // This will establish port mapping on vtipathNode1Port3.
        pids[1][3] = ofMockService.addPort(nid1, 3L, false);
        waiter.set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.DOWN).
            await();

        // Enable port 3 on node 1.
        // This will activate vtpathNode1Port3.
        ofMockService.setPortState(pids[1][3], true, false);
        waiter.set(vtpathNode1Port3, VnodeState.UP).
            set(vtipathNode1Port3, VnodeState.UP, VnodeState.UP).
            await();

        // Create node 2.
        // This should never affect virtual node state because no edge port
        // is available yet.
        ofMockService.addNode(dpid2);
        waiter.await();

        // Add ports to node 2 except port 1.
        // This should never affect virtual node state because:
        //   - Port 1, which will be mapped to bipathNode2Port1, is not yet
        //     added.
        //   - Although port 2 is created, no host is detected yet on port 2.
        for (long idx = 1; idx <= nports; idx++) {
            if (idx != 1) {
                String pid = ofMockService.addPort(nid2, idx, false);
                pids[2][(int)idx] = pid;
                ofMockService.setPortState(pid, true, false);
            }
        }
        for (int i = 1; i < npids; i++) {
            String pid = pids[2][i];
            if (pid != null) {
                ofMockService.awaitLinkState(pid, true);
            }
        }
        waiter.await();

        // Create port 1.
        // This will establish port mapping on bipathNode2Port1.
        pids[2][1] = ofMockService.addPort(nid2, 1L);
        waiter.set(bipathNode2Port1, VnodeState.DOWN, VnodeState.DOWN).await();

        // Enable port 1.
        ofMockService.setPortState(pids[2][1], true, false);
        waiter.set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            await();

        // Enable all ports, and let the vBridge at bpathVlan0 learn some
        // MAC addresses.
        Set<MacAddressEntry> bpathVlan0Hosts = new HashSet<>();
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofMockService.setPortState(ports[j], true, false);
            }
        }

        // Need to synchronize background tasks triggered by port state events.
        sleep(BGTASK_DELAY);

        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                String pid = ports[j];
                ofMockService.awaitLinkState(pid, true);
                TestHost th = new TestHost(hostIdx, pid, (short)0);
                hostIdx++;
                sendBroadcast(ofMockService, th);
                assertTrue(bpathVlan0Hosts.add(th.getMacAddressEntry()));
            }
        }

        // Let all vBridges learn MAC addresses.
        Set<MacAddressEntry> bpathNode1Hosts = new HashSet<>();
        for (int i = 1; i < npids; i++) {
            String pid = pids[1][i];
            TestHost th = new TestHost(hostIdx, pid, (short)1);
            hostIdx++;
            sendBroadcast(ofMockService, th);
            assertTrue(bpathNode1Hosts.add(th.getMacAddressEntry()));
        }

        Set<MacAddressEntry> bpathNode2Port1Hosts = new HashSet<>();
        Set<MacAddressEntry> bpathMacHosts = new HashSet<>();
        final int nhosts = 4;
        for (int i = 0; i < nhosts; i++) {
            TestHost th = new TestHost(hostIdx, pids[2][1], (short)2);
            hostIdx++;
            sendBroadcast(ofMockService, th);
            assertTrue(bpathNode2Port1Hosts.add(th.getMacAddressEntry()));

            th = new TestHost(hostIdx, pids[2][2], host.getVlan());
            hostIdx++;
            sendBroadcast(ofMockService, th);
        }
        waiter.await();
        checkMacAddressTable(bpathMac, null);

        // Send a broadcast packet from host.
        // This will activate bpathMac.
        sendBroadcast(ofMockService, host);
        assertTrue(bpathMacHosts.add(host.getMacAddressEntry()));
        waiter.set(bpathMac, VnodeState.UP).await();
        checkMacAddressTable(bpathVlan0, bpathVlan0Hosts);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, bpathNode2Port1Hosts);
        checkMacAddressTable(bpathMac, bpathMacHosts);

        // Below test causes NumberFormatException in sal-compatibility.

        // // Disable all ports on a non-OpenFlow node.
        // // This should never affect virtual node state.
        // for (int i = 1; i <= npids; i++) {
        //     ofMockService.setPortState(badPorts[i], false, false);
        // }
        // for (int i = 1; i <= npids; i++) {
        //     ofMockService.awaitLinkState(badPorts[i], false);
        // }
        // waiter.await();

        // // Remove a non-OpenFlow node.
        // // This should never affect virtual node state.
        // ofMockService.removeNode(badNid);
        // waiter.await();

        // Disable port 1 on node 2.
        ofMockService.setPortState(pids[2][1], false, false);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.DOWN).
            await();
        Set<MacAddressEntry> bpathVlan0HostsTmp =
            filterOut(bpathVlan0Hosts, pids[2][1]);
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, bpathMacHosts);

        // Disable port 2 on node 2.
        ofMockService.setPortState(pids[2][2], false, false);
        waiter.set(bpathMac, VnodeState.DOWN).await();
        bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pids[2][2]);
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Disable all ports on node 1 but port 4.
        Set<MacAddressEntry> bpathNode1HostsTmp =
            new HashSet<>(bpathNode1Hosts);
        for (int i = 1; i < npids; i++) {
            String pid = pids[1][i];
            ofMockService.setPortState(pid, false, false);
            bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pid);
            bpathNode1HostsTmp = filterOut(bpathNode1HostsTmp, pid);
        }
        for (int i = 1; i < npids; i++) {
            ofMockService.awaitLinkState(pids[1][i], false);
        }
        waiter.await();
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1HostsTmp);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Disable port 4 on node 1.
        ofMockService.setPortState(pids[1][4], false);
        waiter.set(bpathNode1, VnodeState.DOWN).await();
        bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pids[1][4]);
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, null);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Disable all ports on node 3.
        for (int i = 1; i < npids; i++) {
            String pid = pids[3][i];
            ofMockService.setPortState(pid, false, false);
            bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pid);
        }
        for (int i = 1; i < npids; i++) {
            ofMockService.awaitLinkState(pids[3][i], false);
        }
        waiter.await();
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, null);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Disable port 3 on node 2.
        ofMockService.setPortState(pids[2][3], false);
        waiter.await();
        bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pids[2][3]);
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, null);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Disable port 4 on node 2.
        ofMockService.setPortState(pids[2][4], false);
        waiter.await();
        checkMacAddressTable(bpathVlan0, null);
        checkMacAddressTable(bpathNode1, null);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Up all ports, and restore MAC address table entries.
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofMockService.setPortState(ports[j], true, false);
            }
        }
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofMockService.awaitLinkState(ports[j], true);
            }
        }

        // Need to synchronize background tasks triggered by port state events.
        sleep(BGTASK_DELAY);

        for (MacAddressEntry ment: bpathVlan0Hosts) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacAddressEntry ment: bpathNode1Hosts) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacAddressEntry ment: bpathNode2Port1Hosts) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacAddressEntry ment: bpathMacHosts) {
            sendBroadcast(ofMockService, ment);
        }

        waiter.set(bpathNode1, VnodeState.UP).
            set(vtpathNode1Port3, VnodeState.UP).
            set(vtipathNode1Port3, VnodeState.UP, VnodeState.UP).
            set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            set(bpathMac, VnodeState.UP).await();
        checkMacAddressTable(bpathVlan0, bpathVlan0Hosts);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, bpathNode2Port1Hosts);
        checkMacAddressTable(bpathMac, bpathMacHosts);

        // Connect port2 on node2 to port1 on node2.
        String src = pids[2][2];
        String dst = pids[2][1];
        ofMockService.setPeerIdentifier(src, dst);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UP).
            set(bpathMac, VnodeState.DOWN).await();
        bpathVlan0HostsTmp = filterOut(bpathVlan0Hosts, src);
        bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, dst);
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        ofMockService.setPeerIdentifier(src, null);
        waiter.set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).await();
        for (MacAddressEntry ment: bpathNode2Port1Hosts) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacAddressEntry ment: bpathMacHosts) {
            sendBroadcast(ofMockService, ment);
        }
        waiter.set(bpathMac, VnodeState.UP);
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, bpathNode2Port1Hosts);
        checkMacAddressTable(bpathMac, bpathMacHosts);

        // Remove node 3.
        ofMockService.removeNode(nid3);
        waiter.await();
        for (int i = 1; i < npids; i++) {
            bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pids[3][i]);
        }
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, bpathNode2Port1Hosts);
        checkMacAddressTable(bpathMac, bpathMacHosts);

        // Remove node 2.
        ofMockService.removeNode(nid2);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UNKNOWN).
            set(bpathMac, VnodeState.DOWN).await();
        for (int i = 1; i < npids; i++) {
            bpathVlan0HostsTmp = filterOut(bpathVlan0HostsTmp, pids[2][i]);
        }
        checkMacAddressTable(bpathVlan0, bpathVlan0HostsTmp);
        checkMacAddressTable(bpathNode1, bpathNode1Hosts);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Remove node 1.
        ofMockService.removeNode(nid1);
        waiter.set(bpathNode1, VnodeState.DOWN).await();
        checkMacAddressTable(bpathVlan0, null);
        checkMacAddressTable(bpathNode1, null);
        checkMacAddressTable(bpathNode2Port1, null);
        checkMacAddressTable(bpathMac, null);

        // Clean up.
        st = vtnManager.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Check the MAC address table in the given vBridge.
     *
     * @param bpath   Path to the target vBridge.
     * @param hosts   A set of {@link MacAddressEntry} expected to be present
     *                in the MAC adddress table.
     * @throws VTNException  An error occurred.
     */
    private void checkMacAddressTable(VBridgePath bpath,
                                      Set<MacAddressEntry> hosts)
        throws VTNException {
        long timeout = TimeUnit.SECONDS.toMillis(TASK_TIMEOUT);
        long limit = System.currentTimeMillis() + timeout;

        do {
            if (checkMacAddressTable(bpath, hosts, false)) {
                return;
            }

            sleep(SHORT_DELAY);
            timeout = limit - System.currentTimeMillis();
        } while (timeout > 0);

        checkMacAddressTable(bpath, hosts, true);
    }

    /**
     * Check the MAC address table in the given vBridge.
     *
     * @param bpath     Path to the target vBridge.
     * @param hosts     A set of {@link MacAddressEntry} expected to be present
     *                  in the MAC adddress table.
     * @param doAssert  If {@code true}, an error will be thrown on failure.
     * @return  {@code true} only if the check passed.
     * @throws VTNException  An error occurred.
     */
    private boolean checkMacAddressTable(VBridgePath bpath,
                                         Set<MacAddressEntry> hosts,
                                         boolean doAssert)
        throws VTNException {
        List<MacAddressEntry> list = vtnManager.getMacEntries(bpath);
        if (hosts == null || hosts.isEmpty()) {
            if (doAssert) {
                assertTrue(list.isEmpty());
                return true;
            }
            return list.isEmpty();
        }

        if (doAssert) {
            assertEquals(hosts.size(), list.size());
            assertTrue(hosts.containsAll(list));
            return true;
        }

        if (hosts.size() != list.size()) {
            return false;
        }

        return hosts.containsAll(list);
    }

    /**
     * Unicast flow test.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUnicastFlow() throws Exception {
        LOG.info("Running testUnicastFlow().");

        // Create one VTN, 2 vBridges, and 1 vTerminal.
        VTenantPath tpath = new VTenantPath("vtn");
        VBridgePath bpath1 = new VBridgePath(tpath, "vbr1");
        VBridgePath bpath2 = new VBridgePath(tpath, "vbr2");
        VBridgeIfPath bipath11 = new VBridgeIfPath(bpath1, "if1");
        VBridgeIfPath bipath21 = new VBridgeIfPath(bpath2, "if1");
        List<VBridgePath> bpathList = new ArrayList<>();
        List<VBridgeIfPath> bipathList = new ArrayList<>();
        VTerminalPath vtpath = new VTerminalPath(tpath, "vterm");
        VTerminalIfPath vtipath = new VTerminalIfPath(vtpath, "if");
        Collections.addAll(bpathList, bpath1, bpath2);
        Collections.addAll(bipathList, bipath11, bipath21);
        createVirtualNodes(tpath, bpathList, bipathList,
                           Collections.singleton(vtpath),
                           Collections.singleton(vtipath));

        // Collect edge ports per node, and all ISL ports.
        Map<String, List<String>> edgePorts = new HashMap<>();
        List<String> edgeNodes = new ArrayList<>();
        Set<String> islPorts = new HashSet<>();
        Set<String> allPorts = new HashSet<>();
        BridgeNetwork bridge1 = new BridgeNetwork(bpath1);
        BridgeNetwork bridge2 = new BridgeNetwork(bpath2);
        Map<VBridgePath, BridgeNetwork> bridges = new HashMap<>();
        bridges.put(bpath1, bridge1);
        bridges.put(bpath2, bridge2);
        for (String nid: ofMockService.getNodes()) {
            List<String> ports = ofMockService.getPorts(nid, true);
            if (!ports.isEmpty()) {
                edgeNodes.add(nid);
                allPorts.addAll(ports);
                assertEquals(null, edgePorts.put(nid, ports));
            }

            for (String pid: ofMockService.getPorts(nid, false)) {
                assertTrue(islPorts.add(pid));
                allPorts.add(pid);
                bridge1.setUnmappedPort(pid);
                bridge2.setUnmappedPort(pid);
            }
        }
        assertFalse(edgeNodes.isEmpty());
        assertFalse(edgePorts.isEmpty());
        assertFalse(islPorts.isEmpty());
        assertFalse(allPorts.isEmpty());

        // Map untagged network to vbr1 using VLAN mapping.
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)0);
        vtnManager.addVlanMap(bpath1, vlconf);

        // But untagged network on edgeNodes[0].ports[0] to vbr2 using
        // port mapping.
        Set<TestPort> portMapped = new HashSet<>();
        String targetPort = edgePorts.get(edgeNodes.get(0)).get(0);
        setPortMap(bipath21, targetPort, (short)0, StatusCode.SUCCESS);
        assertTrue(portMapped.add(new TestPort(targetPort).setVlan((short)0)));

        for (List<String> ports: edgePorts.values()) {
            for (String pid: ports) {
                BridgeNetwork bridge = (pid.equals(targetPort))
                    ? bridge2 : bridge1;
                bridge.addMappedVlan(pid, (short)0);
            }
        }

        // Map VLAN 1 on edgeNodes[0] to vbr1.
        Node node = toAdNode(edgeNodes.get(0));
        vlconf = new VlanMapConfig(node, (short)1);
        vtnManager.addVlanMap(bpath1, vlconf);

        // Map VLAN 2 on edgeNodes[1] to vbr2.
        node = toAdNode(edgeNodes.get(1));
        vlconf = new VlanMapConfig(node, (short)1);
        vtnManager.addVlanMap(bpath2, vlconf);

        // But one port in edgeNodes[1] to vbr1 using port mapping.
        targetPort = edgePorts.get(edgeNodes.get(1)).get(1);
        setPortMap(bipath11, targetPort, (short)1, StatusCode.SUCCESS);
        assertTrue(portMapped.add(new TestPort(targetPort).setVlan((short)1)));

        String mmapPort = null;
        for (String pid: edgePorts.get(edgeNodes.get(0))) {
            if (mmapPort == null) {
                // Override this port using MAC mapping.
                mmapPort = pid;
            }
            bridge1.addMappedVlan(pid, (short)1);
        }
        assertNotNull(mmapPort);
        String mmapNode = OfMockUtils.getNodeIdentifier(mmapPort);

        for (String pid: edgePorts.get(edgeNodes.get(1))) {
            BridgeNetwork bridge = (pid.equals(targetPort))
                ? bridge1 : bridge2;
            bridge.addMappedVlan(pid, (short)1);
        }

        // Create test hosts.
        final int nhosts = 4;
        int hostIdx = bridge1.addTestHosts(1, nhosts);
        hostIdx = bridge2.addTestHosts(hostIdx, nhosts);

        // Map hosts on mmapPort (VLAN 1) to vbr2 using MAC mapping.
        Set<EthernetHost> hostSet = new HashSet<>();
        Map<VBridgePath, Set<TestHost>> mmapAllowed = new HashMap<>();
        assertEquals(null, mmapAllowed.put(bpath2, new HashSet<TestHost>()));
        for (int i = 0; i < nhosts; i++) {
            TestHost th = new TestHost(hostIdx, mmapPort, (short)1);
            hostIdx++;
            assertTrue(mmapAllowed.get(bpath2).add(th));
            assertTrue(hostSet.add(th.getEthernetHost()));
        }
        assertEquals(UpdateType.ADDED,
                     vtnManager.setMacMap(bpath2, VtnUpdateOperationType.ADD,
                                          VtnAclType.ALLOW, hostSet));

        // Configure MAC mappings for VLAN 4095.
        Deque<VBridgePath> bpathQueue = new LinkedList<>();
        Collections.addAll(bpathQueue, bpath1, bpath2);
        for (Map.Entry<String, List<String>> entry: edgePorts.entrySet()) {
            String nid = entry.getKey();
            List<String> ports = entry.getValue();
            for (String pid: ports) {
                VBridgePath bpath = bpathQueue.removeFirst();
                bpathQueue.addLast(bpath);

                Set<TestHost> allowed = mmapAllowed.get(bpath);
                UpdateType expected;
                if (allowed == null) {
                    expected = UpdateType.ADDED;
                    allowed = new HashSet<TestHost>();
                    assertEquals(null, mmapAllowed.put(bpath, allowed));
                } else {
                    expected = UpdateType.CHANGED;
                }

                hostSet = new HashSet<>();
                for (int i = 0; i < nhosts; i++) {
                    TestHost th = new TestHost(hostIdx, pid, (short)4095);
                    hostIdx++;
                    EthernetHost ehost = th.getEthernetHost();
                    assertTrue(hostSet.add(ehost));
                    assertTrue(allowed.add(th));
                }

                assertEquals(expected,
                             vtnManager.setMacMap(
                                 bpath, VtnUpdateOperationType.ADD,
                                 VtnAclType.ALLOW, hostSet));
            }
        }

        for (VBridgePath bpath: bpathQueue) {
            MacMap mmap = vtnManager.getMacMap(bpath);
            Set<TestHost> allowedHosts = mmapAllowed.get(bpath);
            Set<DataLinkHost> allowed = mmap.getAllowedHosts();
            assertEquals(allowed.size(), allowedHosts.size());
            for (TestHost th: allowedHosts) {
                assertTrue(allowed.contains(th.getEthernetHost()));
            }
            assertTrue(mmap.getDeniedHosts().isEmpty());
            assertTrue(mmap.getMappedHosts().isEmpty());
        }

        bridge1.verify();
        bridge2.verify();

        // Ensure that all virtual interfaces are ready.
        // Note that vBridge state should be DOWN because all MAC mappings are
        // still inactivated.
        VNodeStateWaiter waiter = new VNodeStateWaiter(vtnManager).
            set(bpath1, VnodeState.DOWN).set(bpath2, VnodeState.DOWN).
            set(bipath11, VnodeState.UP).set(bipath21, VnodeState.UP);
        waiter.await();

        // Ensure any incoming packet from internal port is ignored.
        for (String pid: islPorts) {
            TestHost th = new TestHost(hostIdx, pid, (short)0);
            hostIdx++;
            sendBroadcast(ofMockService, th);
        }
        sleep(SHORT_DELAY);
        for (List<String> ports: edgePorts.values()) {
            for (String pid: ports) {
                assertNull(ofMockService.getTransmittedPacket(pid));
            }
        }
        for (String pid: islPorts) {
            assertNull(ofMockService.getTransmittedPacket(pid));
        }

        // Let test vBridges learn MAC addresses except MAC mapped hosts.
        learnHosts(bridge1);
        learnHosts(bridge2);
        checkMacAddressTable(bpath1, bridge1.getMacAddressEntries());
        checkMacAddressTable(bpath2, bridge2.getMacAddressEntries());

        // Send unicast packets.
        List<UnicastFlow> flows1 =
            unicastTest(bridge1, islPorts, true, VnodeState.DOWN);
        List<UnicastFlow> flows2 =
            unicastTest(bridge2, islPorts, true, VnodeState.DOWN);
        FlowCounter counter = new FlowCounter(ofMockService).
            add(flows1).add(flows2).verify();

        // Activate MAC mappings.
        Set<String> remappedVlans = new HashSet<>();
        for (Map.Entry<VBridgePath, BridgeNetwork> entry: bridges.entrySet()) {
            VBridgePath bpath = entry.getKey();
            BridgeNetwork bridge = entry.getValue();
            Set<TestHost> allowed = mmapAllowed.get(bpath);
            for (TestHost th: allowed) {
                String pid = th.getPortIdentifier();
                short vlan = th.getVlan();
                String remapKey = pid + "," + vlan;
                boolean remapped = remappedVlans.add(remapKey);
                if (remapped) {
                    bridge1.removeMappedVlan(pid, vlan);
                    bridge2.removeMappedVlan(pid, vlan);
                }
                bridge.addHost(th);
                learnHost(bpath, bridge.getMappedVlans(), th);

                if (remapped) {
                    // Ensure that flow entries for unmapped hosts have been
                    // uninstalled.
                    verifyFlowUninstalled(pid, vlan, flows1);
                    verifyFlowUninstalled(pid, vlan, flows2);
                }
            }
        }

        // Ensure that unexpected flow enty is not installed.
        counter.clear().add(flows1).add(flows2).verify();

        // Ensure MAC address entries for unmapped hosts have been removed.
        checkMacAddressTable(bpath1, bridge1.getMacAddressEntries());
        checkMacAddressTable(bpath2, bridge2.getMacAddressEntries());

        // Ensure that all virtual nodes are ready.
        waiter.set(bpath1, VnodeState.UP).set(bpath2, VnodeState.UP).await();

        // Verify MAC mapping state.
        for (Map.Entry<VBridgePath, Set<TestHost>> entry:
                 mmapAllowed.entrySet()) {
            VBridgePath bpath = entry.getKey();
            Set<TestHost> allowedHosts = entry.getValue();
            MacMap mmap = vtnManager.getMacMap(bpath);
            Set<DataLinkHost> allowed = mmap.getAllowedHosts();
            Set<MacAddressEntry> mapped = new HashSet<>(mmap.getMappedHosts());
            assertEquals(allowed.size(), allowedHosts.size());
            assertEquals(mapped.size(), allowedHosts.size());
            for (TestHost th: allowedHosts) {
                assertTrue(allowed.contains(th.getEthernetHost()));
                assertTrue(mapped.contains(th.getMacAddressEntry()));
            }
            assertTrue(mmap.getDeniedHosts().isEmpty());
        }

        // Send unicast packets again.
        flows1 = unicastTest(bridge1, islPorts, true);
        flows2 = unicastTest(bridge2, islPorts, true);
        counter.clear().add(flows1).add(flows2).verify();

        // Ensure that flow entries are not changed when port mapping fails.
        Set<String> skip = new HashSet<>();
        for (TestPort tp: portMapped) {
            String pid = tp.getPortIdentifier();
            skip.add(pid);
            setPortMap(vtipath, pid, tp.getVlan(), StatusCode.CONFLICT);
            counter.verify();
            checkMacAddressTable(bpath1, bridge1.getMacAddressEntries());
            checkMacAddressTable(bpath2, bridge2.getMacAddressEntries());
        }

        // Determine port for vTerminal test.
        for (String pid: allPorts) {
            if (!skip.contains(pid) && !islPorts.contains(pid)) {
                targetPort = pid;
                break;
            }
        }
        // Try to map edge port to vTerminal.
        OfMockFlow dropFlow = null;
        for (short vlan: new short[]{0, 1, 4095}) {
            setPortMap(vtipath, targetPort, vlan, StatusCode.SUCCESS);
            bridge1.removeMappedVlan(targetPort, vlan);
            bridge2.removeMappedVlan(targetPort, vlan);
            verifyFlowUninstalled(targetPort, vlan, flows1);
            verifyFlowUninstalled(targetPort, vlan, flows2);
            counter.clear().add(flows1).add(flows2).verify();
            checkMacAddressTable(bpath1, bridge1.getMacAddressEntries());
            checkMacAddressTable(bpath2, bridge2.getMacAddressEntries());

            if (dropFlow != null) {
                String nid = dropFlow.getNodeIdentifier();
                int pri = dropFlow.getPriority();
                Match match = dropFlow.getMatch();
                OfMockFlow f = ofMockService.
                    awaitFlow(nid, OfMockService.DEFAULT_TABLE, match, pri,
                              false);
                assertEquals(null, f);
            }

            // Ensure that vTerminal drops incoming packet.
            TestHost host = new TestHost(hostIdx, targetPort, vlan);
            hostIdx++;
            dropFlow = dropTest(host, allPorts);
            counter.add(dropFlow).verify();
            checkMacAddressTable(bpath1, bridge1.getMacAddressEntries());
            checkMacAddressTable(bpath2, bridge2.getMacAddressEntries());
        }

        // Clean up.
        Status st = vtnManager.removeTenant(tpath);
        assertTrue(st.isSuccess());
    }

    /**
     * Create a {@link PortLocation} instance that specifies the given port.
     *
     * <p>
     *   This method creates a {@link PortLocation} that specifies the port
     *   by type and ID.
     * </p>
     *
     * @param pid     The identifier of the switch port.
     * @return  A {@link PortLocation} instance.
     */
    private PortLocation toPortLocation(String pid) {
        NodeConnector nc = toAdNodeConnector(pid);
        SwitchPort swport = new SwitchPort(nc.getType(),
                                           nc.getNodeConnectorIDString());
        return new PortLocation(nc.getNode(), swport);
    }

    /**
     * Create a {@link PortLocation} instance that specifies the given port.
     *
     * @param pid     The identifier of the switch port.
     * @param byType  If {@code true}, the given port is specified by type and
     *                ID in addition to port name.
     *                If {@code false}, the given port is specified by only
     *                port name.
     * @return  A {@link PortLocation} instance.
     */
    private PortLocation toPortLocation(String pid, boolean byType) {
        NodeConnector nc = toAdNodeConnector(pid);
        SwitchPort swport;
        String name = ofMockService.getPortName(pid);
        if (byType) {
            swport = new SwitchPort(name, nc.getType(),
                                    nc.getNodeConnectorIDString());
        } else {
            swport = new SwitchPort(name);
        }

        return new PortLocation(nc.getNode(), swport);
    }

    /**
     * Configure port mapping into the given virtual interface.
     *
     * @param ifPath  Path to the virtual interface.
     * @param pid     Identifier of the port to be mapped.
     * @param vlan    VLAN ID to be mapped.
     * @param code    Expected result of port mapping operation.
     */
    private void setPortMap(VInterfacePath ifPath, String pid, short vlan,
                            StatusCode code) {
        NodeConnector nc = toAdNodeConnector(pid);
        SwitchPort swport =
            new SwitchPort(nc.getType(), nc.getNodeConnectorIDString());
        PortMapConfig pmconf =
            new PortMapConfig(nc.getNode(), swport, vlan);
        Status st;

        if (ifPath instanceof VBridgeIfPath) {
            st = vtnManager.setPortMap((VBridgeIfPath)ifPath, pmconf);
        } else if (ifPath instanceof VTerminalIfPath) {
            st = vtnManager.setPortMap((VTerminalIfPath)ifPath, pmconf);
        } else {
            fail("Unexpected interface path: " + ifPath);
            return;
        }

        assertEquals(code, st.getCode());
    }

    /**
     * Ensure that all flow entries related to the given network have been
     * uninstalled.
     *
     * @param pid    The port identifier of the unmapped network.
     * @param vlan   The VLAN ID of the unmapped network.
     * @param flows  A list of {@link UnicastFlow} instances.
     */
    private void verifyFlowUninstalled(String pid, short vlan,
                                       List<UnicastFlow> flows)
        throws Exception {
        // Determine flow entries to be uninstalled.
        final int tableId = OfMockService.DEFAULT_TABLE;
        List<UnicastFlow> uninstalled = new ArrayList<>();
        for (Iterator<UnicastFlow> it = flows.iterator(); it.hasNext();) {
            // Check ingress flow.
            UnicastFlow unicast = it.next();
            List<OfMockFlow> flowList = unicast.getFlowList();
            OfMockFlow ingress = flowList.get(0);
            Match match = ingress.getMatch();
            short inVlan = getVlanMatch(match);
            if (inVlan == vlan && pid.equals(getInPortMatch(match))) {
                uninstalled.add(unicast);
                it.remove();
                continue;
            }

            // Check egress flow.
            OfMockFlow egress = flowList.get(flowList.size() - 1);
            if (hasOutput(egress.getInstructions(), pid, vlan, inVlan)) {
                uninstalled.add(unicast);
                it.remove();
                continue;
            }
        }

        UnicastFlow.verifyFlows(uninstalled, false, true);

        // Rest of flow entries should be still installed.
        UnicastFlow.verifyFlows(flows, true, false);
    }

    /**
     * Test method for the packet routing table maintenance.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRoutingTable() throws Exception {
        LOG.info("Running testRoutingTable().");

        // Create a VTN and a vBridge for test.
        VTenantPath tpath = new VTenantPath("vtn");
        VBridgePath bpath = new VBridgePath(tpath, "vbr");
        createVirtualNodes(tpath, Collections.singleton(bpath));

        // Collect edge ports per node, and all ISL ports.
        BridgeNetwork bridge = new BridgeNetwork(bpath);
        Set<String> islPorts = new HashSet<>();
        int idx = 1;
        short vlan = 0;
        Short vid = Short.valueOf(vlan);
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                bridge.addHost(nid, new TestHost(idx, pid, vlan));
                idx++;
            }

            for (String pid: ofMockService.getPorts(nid, false)) {
                bridge.setUnmappedPort(pid);
                assertTrue(islPorts.add(pid));
            }
        }
        bridge.verify();
        assertFalse(islPorts.isEmpty());

        // Map untagged frame using VLAN mapping.
        VlanMapConfig vlconf = new VlanMapConfig(null, (short)0);
        VlanMap map = vtnManager.addVlanMap(bpath, vlconf);
        VNodeStateWaiter waiter = new VNodeStateWaiter(vtnManager).
            set(bpath, VnodeState.UP);
        waiter.await();

        // Let the test vBridge learn MAC addresses.
        learnHosts(bridge);

        // Flow entry should not be present.
        new FlowCounter(ofMockService).verify();

        // Cause path fault.
        unicastTest(bridge, islPorts, false);

        // Packet processing for previous test may not complete.
        // So we need to install a valid flow entry here in order to
        // synchronize packet processing.
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        Set<String> edgeNodes = new HashSet<>();
        for (Map.Entry<String, List<TestHost>> entry: hostMap.entrySet()) {
            List<TestHost> hosts = entry.getValue();
            if (hosts.size() >= 2) {
                edgeNodes.add(entry.getKey());
                TestHost src = hosts.get(0);
                TestHost dst = hosts.get(1);
                ArpFlowFactory factory = new ArpFlowFactory(ofMockService);
                List<OfMockLink> route = Collections.<OfMockLink>emptyList();
                UnicastFlow unicast = factory.create(src, dst, route);
                unicast.runTest();
                assertEquals(1, unicast.getFlowCount());
            }
        }
        assertFalse(edgeNodes.isEmpty());

        for (String nid: ofMockService.getNodes()) {
            int expected = (edgeNodes.contains(nid)) ? 1 : 0;
            assertEquals(expected, ofMockService.getFlowCount(nid));
        }

        // Resolve path fault.
        unicastTest(bridge, islPorts, true);

        Status st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
    }

    /**
     * Learn all hosts mapped to the given bridge network.
     *
     * @param bridge  A {@link BridgeNetwork} instance.
     * @throws Exception  An error occurred.
     */
    private void learnHosts(BridgeNetwork bridge) throws Exception {
        VBridgePath bpath = bridge.getPath();
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        Map<String, Set<Short>> allPorts = bridge.getMappedVlans();

        for (List<TestHost> hosts: hostMap.values()) {
            for (TestHost host: hosts) {
                learnHost(bpath, allPorts, host);
            }
        }
    }

    /**
     * Learn the given host mapped to the given bridge network.
     *
     * @param bpath     Path to the vBridge.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must
     *                  be associated with the key.
     * @param host      A test host to learn.
     * @throws Exception  An error occurred.
     */
    private void learnHost(VBridgePath bpath, Map<String, Set<Short>> allPorts,
                           TestHost host) throws Exception {
        learnHost(ofMockService, allPorts, host);

        EthernetAddress ethAddr = host.getEthernetAddress();
        assertEquals(host.getMacAddressEntry(),
                     vtnManager.getMacEntry(bpath, ethAddr));
    }

    /**
     * Do the ARP unicast packet forwarding test.
     *
     * @param bridge  A {@link BridgeNetwork} instance.
     * @param islPorts  A set of ISL ports.
     * @param up        {@code true} means that all ISL ports should be up.
     *                  {@code false} means that all ISL ports should be down.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(BridgeNetwork bridge,
                                          Set<String> islPorts, boolean up)
        throws Exception {
        ArpFlowFactory factory = new ArpFlowFactory(ofMockService);
        return unicastTest(factory, bridge, islPorts, up);
    }

    /**
     * Do the ARP unicast packet forwarding test.
     *
     * @param bridge       A {@link BridgeNetwork} instance.
     * @param islPorts     A set of ISL ports.
     * @param up           {@code true} means that all ISL ports should be up.
     *                     {@code false} means that all ISL ports should be
     *                     down.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(BridgeNetwork bridge,
                                          Set<String> islPorts, boolean up,
                                          VnodeState bridgeState)
        throws Exception {
        ArpFlowFactory factory = new ArpFlowFactory(ofMockService);
        return unicastTest(factory, bridge, islPorts, up, bridgeState);
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param bridge  A {@link BridgeNetwork} instance.
     * @param islPorts  A set of ISL ports.
     * @param up        {@code true} means that all ISL ports should be up.
     *                  {@code false} means that all ISL ports should be down.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(UnicastFlowFactory factory,
                                          BridgeNetwork bridge,
                                          Set<String> islPorts, boolean up)
        throws Exception {
        return unicastTest(factory, bridge, islPorts, up, VnodeState.UP);
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param bridge       A {@link BridgeNetwork} instance.
     * @param islPorts     A set of ISL ports.
     * @param up           {@code true} means that all ISL ports should be up.
     *                     {@code false} means that all ISL ports should be
     *                     down.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(UnicastFlowFactory factory,
                                          BridgeNetwork bridge,
                                          Set<String> islPorts, boolean up,
                                          VnodeState bridgeState)
        throws Exception {
        VBridgePath bpath = bridge.getPath();
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        factory.addPortSet(bridge.getMappedVlans().keySet());

        boolean changed = false;
        for (String pid: islPorts) {
            if (ofMockService.setPortState(pid, up)) {
                changed = true;
            }
        }
        for (String pid: islPorts) {
            ofMockService.awaitLink(pid, null, up);
        }

        if (changed) {
            // remove-flow operation is triggered when port state is changed.
            // So we need to wait for its completion.
            sleep(BGTASK_DELAY);
        }

        List<UnicastFlow> flows = new ArrayList<>();
        int nfaults = 0;
        for (Map.Entry<String, List<TestHost>> srcEnt: hostMap.entrySet()) {
            String srcNid = srcEnt.getKey();
            List<TestHost> srcHosts = srcEnt.getValue();
            for (Map.Entry<String, List<TestHost>> dstEnt:
                     hostMap.entrySet()) {
                String dstNid = dstEnt.getKey();
                if (srcNid.equals(dstNid)) {
                    continue;
                }

                List<TestHost> dstHosts = dstEnt.getValue();
                List<OfMockLink> route =
                    ofMockService.getRoute(srcNid, dstNid);
                if (up) {
                    assertNotNull(route);
                } else {
                    nfaults++;
                    assertEquals(null, route);
                }

                flows.addAll(unicastTest(factory, bpath, bridgeState, srcHosts,
                                         dstHosts, nfaults, route));
            }
        }

        return flows;
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param bpath        Path to the test vBridge.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @param srcList      A list of source hosts.
     * @param dstList      A list of destination hosts.
     * @param nfaults      Expected number of path faults.
     * @param route        A list of {@link OfMockLink} which represents the
     *                     packet route.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(UnicastFlowFactory factory,
                                          VBridgePath bpath,
                                          VnodeState bridgeState,
                                          List<TestHost> srcList,
                                          List<TestHost> dstList, int nfaults,
                                          List<OfMockLink> route)
        throws Exception {
        List<UnicastFlow> flows = new ArrayList<>();
        boolean reachable = (nfaults == 0);
        VnodeState bstate;
        if (reachable) {
            bstate = bridgeState;
            assertNotNull(route);
        } else {
            bstate = VnodeState.DOWN;
            assertEquals(null, route);
        }

        for (TestHost src: srcList) {
            for (TestHost dst: dstList) {
                // Run tests.
                VNodeStateWaiter waiter = new VNodeStateWaiter(vtnManager).
                    set(bpath, bstate, nfaults);
                UnicastFlow unicast = factory.create(src, dst, route);
                unicast.runTest(waiter);

                // Verify results.
                int count = unicast.getFlowCount();
                if (reachable && !factory.isAlreadyMapped()) {
                    assertTrue(count > 0);
                    flows.add(unicast);
                } else {
                    assertEquals(0, count);
                }
            }
        }

        return flows;
    }

    /**
     * Ensure that an unicast packet sent by the given host is dropped.
     *
     * @param host      The source host.
     * @param allPorts  A set of all port identifiers.
     * @return  A flow entry that drops incoming packet from the given host.
     */
    private OfMockFlow dropTest(TestHost host, Set<String> allPorts)
        throws Exception {
        String ingress = host.getPortIdentifier();
        byte[] sha = host.getMacAddress();
        byte[] spa = host.getRawInetAddress();
        short vlan = host.getVlan();
        EthernetFactory efc = new EthernetFactory(sha, MAC_DUMMY).
            setVlanId(vlan);
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(MAC_DUMMY).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(IPV4_DUMMY);
        byte[] payload = efc.create();
        ofMockService.sendPacketIn(ingress, payload);

        // Ensure that a flow entry that drops the packet was installed.
        String nid = OfMockUtils.getNodeIdentifier(ingress);
        int pri = ofMockService.getL2FlowPriority();
        Match match = createMatch(ingress, vlan).build();
        OfMockFlow flow = ofMockService.
            awaitFlow(nid, OfMockService.DEFAULT_TABLE, match, pri, true);
        verifyDropFlow(flow.getInstructions());

        // Ensure that no packet was forwarded.
        for (String pid: allPorts) {
            assertNull(ofMockService.getTransmittedPacket(pid));
        }

        return flow;
    }

    /**
     * Test method for path map.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPathMap() throws Exception {
        LOG.info("Running testPathMap().");

        // Collect existing switch ports.
        Map<String, Set<Short>> allPorts = new HashMap<>();
        for (String nid: ofMockService.getNodes()) {
            for (boolean b: new boolean[]{true, false}) {
                for (String pid: ofMockService.getPorts(nid, b)) {
                    assertEquals(null, allPorts.put(pid, null));
                }
            }
        }

        // Create 4 nodes.
        final int numNodes = 4;
        String[] nodeIds = new String[numNodes];
        for (int i = 0; i < numNodes; i++) {
            BigInteger dpid = BigInteger.valueOf((long)(i + 1));
            String nid = ofMockService.addNode(dpid, false);
            nodeIds[i] = nid;

            for (int j = 1; j <= numNodes; j++) {
                String pid = ofMockService.addPort(nid, (long)j, false);
                assertEquals(null, allPorts.put(pid, null));
                ofMockService.setPortState(pid, true);
            }
        }

        // Construct full-mesh network.
        Map<String, String> allLinks = new HashMap<>();
        for (int i = 0; i < numNodes; i++) {
            String srcNid = nodeIds[i];
            Iterator<String> srcIt = ofMockService.getPorts(srcNid, true).
                iterator();
            for (int j = i + 1; j < numNodes; j++) {
                String dstNid = nodeIds[j];
                String src = srcIt.next();
                String dst = ofMockService.getPorts(dstNid, true).get(0);
                assertEquals(null, allLinks.put(src, dst));
                assertEquals(null, allLinks.put(dst, src));
                ofMockService.setPeerIdentifier(src, dst, false);
                ofMockService.setPeerIdentifier(dst, src, false);
            }
        }
        assertEquals(numNodes * (numNodes - 1), allLinks.size());

        // Need to synchronize background tasks triggered by port state events.
        sleep(BGTASK_DELAY);

        // Wait for network topology to be established.
        for (Map.Entry<String, String> entry: allLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            ofMockService.awaitLink(src, dst, true);
        }

        // Create a VTN and a vBridge for test.
        VTenantPath tpath = new VTenantPath("vtn");
        VBridgePath bpath = new VBridgePath(tpath, "vbr");
        VBridgeIfPath ipath0 = new VBridgeIfPath(bpath, "if0");
        VBridgeIfPath ipath2 = new VBridgeIfPath(bpath, "if2");
        List<VBridgeIfPath> ipaths = new ArrayList<>();
        Collections.addAll(ipaths, ipath0, ipath2);
        createVirtualNodes(tpath, Collections.singleton(bpath), ipaths);

        // Map an edge port on nodeIds[0] to ipath0.
        short vlan0 = 0;
        String edge0 = ofMockService.getPorts(nodeIds[0], true).get(0);
        setPortMap(ipath0, edge0, vlan0, StatusCode.SUCCESS);
        allPorts.put(edge0, Collections.singleton(vlan0));

        // Map an edge port on nodeIds[2] to ipath2.
        short vlan2 = 2;
        String edge2 = ofMockService.getPorts(nodeIds[2], true).get(0);
        setPortMap(ipath2, edge2, vlan2, StatusCode.SUCCESS);
        allPorts.put(edge2, Collections.singleton(vlan2));

        // Create 2 hosts connected to edge0.
        int hostIdx = 1;
        final int numHosts = 2;
        List<TestHost> hosts0 = new ArrayList<>();
        for (int i = 0; i < numHosts; i++) {
            hosts0.add(new TestHost(hostIdx, edge0, vlan0));
            hostIdx++;
        }

        // Create 2 hosts connected to edge2.
        List<TestHost> hosts2 = new ArrayList<>();
        for (int i = 0; i < numHosts; i++) {
            hosts2.add(new TestHost(hostIdx, edge2, vlan2));
            hostIdx++;
        }

        // Determine shortest path from nodeIds[0] to nodeIds[2].
        OfMockLink link = getInterSwitchLink(nodeIds[0], nodeIds[2]);
        assertNotNull(link);
        List<OfMockLink> route0 = Collections.singletonList(link);

        // Configure path policy 1.
        //   Route from nodeIds[0] to nodeIds[2]:
        //     0 -> 1 -> 2
        List<OfMockLink> route1 = new ArrayList<>();
        link = getInterSwitchLink(nodeIds[0], nodeIds[1]);
        assertNotNull(link);
        route1.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[2]);
        assertNotNull(link);
        route1.add(link);

        Set<PathCost> costs = new HashSet<>();
        for (OfMockLink l: route1) {
            PortLocation ploc = toPortLocation(l.getSourcePort());
            assertTrue(costs.add(new PathCost(ploc, 1L)));
        }

        long defCost1 = 10000L;
        PathPolicy pp1 =
            new PathPolicy(defCost1, new ArrayList<PathCost>(costs));
        assertEquals(UpdateType.ADDED, vtnManager.setPathPolicy(1, pp1));
        assertEquals(null, vtnManager.setPathPolicy(1, pp1));
        pp1 = vtnManager.getPathPolicy(1);
        assertEquals(Integer.valueOf(1), pp1.getPolicyId());
        assertEquals(defCost1, pp1.getDefaultCost());
        assertEquals(costs, new HashSet<PathCost>(pp1.getPathCosts()));

        // Configure path policy 2.
        //   Route from nodeIds[0] to nodeIds[2]:
        //     0 -> 3 -> 2
        List<OfMockLink> route2 = new ArrayList<>();
        link = getInterSwitchLink(nodeIds[0], nodeIds[3]);
        assertNotNull(link);
        route2.add(link);
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        route2.add(link);

        long defCost2 = 20000L;
        PathPolicy pp2 = new PathPolicy(defCost2, null);
        assertEquals(UpdateType.ADDED, vtnManager.setPathPolicy(2, pp2));
        assertEquals(null, vtnManager.setPathPolicy(2, pp2));
        pp2 = vtnManager.getPathPolicy(2);
        assertEquals(Integer.valueOf(2), pp2.getPolicyId());
        assertEquals(defCost2, pp2.getDefaultCost());
        assertEquals(Collections.<PathCost>emptySet(),
                     new HashSet<PathCost>(pp2.getPathCosts()));

        costs.clear();
        for (OfMockLink l: route2) {
            PortLocation ploc = toPortLocation(l.getSourcePort(), false);
            assertEquals(PathPolicy.COST_UNDEF,
                         vtnManager.getPathPolicyCost(2, ploc));
            long cost = 2L;
            assertEquals(UpdateType.ADDED,
                         vtnManager.setPathPolicyCost(2, ploc, cost));
            assertEquals(null, vtnManager.setPathPolicyCost(2, ploc, cost));
            assertEquals(cost, vtnManager.getPathPolicyCost(2, ploc));
            assertTrue(costs.add(new PathCost(ploc, cost)));

            pp2 = vtnManager.getPathPolicy(2);
            assertEquals(Integer.valueOf(2), pp2.getPolicyId());
            assertEquals(defCost2, pp2.getDefaultCost());
            assertEquals(costs, new HashSet<PathCost>(pp2.getPathCosts()));
        }

        // Increase cost for the links from nodeIds[0] to nodeIds[1]/nodeIds[2]
        // for later test.
        for (String dst: new String[]{nodeIds[1], nodeIds[2]}) {
            link = getInterSwitchLink(nodeIds[0], dst);
            assertNotNull(link);
            PortLocation ploc = toPortLocation(link.getSourcePort(), true);
            long cost = 100000000000L;
            assertEquals(UpdateType.ADDED,
                         vtnManager.setPathPolicyCost(2, ploc, cost));
            assertEquals(null, vtnManager.setPathPolicyCost(2, ploc, cost));
            assertTrue(costs.add(new PathCost(ploc, cost)));
            pp2 = vtnManager.getPathPolicy(2);
            assertEquals(Integer.valueOf(2), pp2.getPolicyId());
            assertEquals(defCost2, pp2.getDefaultCost());
            assertEquals(costs, new HashSet<PathCost>(pp2.getPathCosts()));
        }

        // Configure path policy 3.
        //   Route from nodeIds[0] to nodeIds[2]:
        //     0 -> 1 -> 3 -> 2
        List<OfMockLink> route3 = new ArrayList<>();
        link = getInterSwitchLink(nodeIds[0], nodeIds[1]);
        assertNotNull(link);
        route3.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[3]);
        assertNotNull(link);
        route3.add(link);
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        route3.add(link);

        long defCost3 = 30000L;
        PathPolicy pp3 = new PathPolicy(defCost3, null);
        assertEquals(UpdateType.ADDED, vtnManager.setPathPolicy(3, pp3));
        assertEquals(null, vtnManager.setPathPolicy(3, pp3));

        costs.clear();
        long cost3 = 10L;
        for (OfMockLink l: route3) {
            PortLocation ploc = toPortLocation(l.getSourcePort(), true);
            assertTrue(costs.add(new PathCost(ploc, cost3)));
        }
        pp3 = new PathPolicy(defCost3, new ArrayList<PathCost>(costs));
        assertEquals(UpdateType.CHANGED, vtnManager.setPathPolicy(3, pp3));
        assertEquals(null, vtnManager.setPathPolicy(3, pp3));
        pp3 = vtnManager.getPathPolicy(3);
        assertEquals(Integer.valueOf(3), pp3.getPolicyId());
        assertEquals(defCost3, pp3.getDefaultCost());
        assertEquals(costs, new HashSet<PathCost>(pp3.getPathCosts()));

        // Increase cost for the link from nodeIds[0] to nodeIds[2]/nodeIds[3]
        // for later test.
        for (String dst: new String[]{nodeIds[2], nodeIds[3]}) {
            link = getInterSwitchLink(nodeIds[0], dst);
            assertNotNull(link);
            PortLocation ploc = toPortLocation(link.getSourcePort(), true);
            long cost = defCost3  * 2;
            assertEquals(UpdateType.ADDED,
                         vtnManager.setPathPolicyCost(3, ploc, cost));
            assertEquals(null, vtnManager.setPathPolicyCost(3, ploc, cost));
            assertTrue(costs.add(new PathCost(ploc, cost)));
            pp3 = vtnManager.getPathPolicy(3);
            assertEquals(Integer.valueOf(3), pp3.getPolicyId());
            assertEquals(defCost3, pp3.getDefaultCost());
            assertEquals(costs, new HashSet<PathCost>(pp3.getPathCosts()));
        }

        // Configure flow conditions.

        // udp_src_50000: Match UDP packet from port 50000.
        final short udpSrcPort = (short)50000;
        UdpMatch udm = new UdpMatch(Short.valueOf(udpSrcPort), null);
        FlowMatch fm = new FlowMatch(1, null, null, udm);
        String cnameUdp = "udp_src_50000";
        FlowCondition condUdp =
            new FlowCondition(cnameUdp, Collections.singletonList(fm));
        assertEquals(UpdateType.ADDED,
                     vtnManager.setFlowCondition(cnameUdp, condUdp));
        assertEquals(null, vtnManager.setFlowCondition(cnameUdp, condUdp));

        EtherAddress nullMac = null;
        EthernetMatch em = new EthernetMatch(
            nullMac, nullMac, EtherTypes.IPV4.intValue(), null, null);
        Inet4Match im = new Inet4Match(null, null, null, null,
                                       InetProtocols.UDP.shortValue(), null);
        fm = new FlowMatch(1, em, im, udm);
        condUdp = new FlowCondition(cnameUdp, Collections.singletonList(fm));
        assertEquals(condUdp, vtnManager.getFlowCondition(cnameUdp));

        // tcp_dst_23: Match TCP packet destinated to port 23.
        final short tcpDstPort = 23;
        String cnameTcp = "tcp_dst_23";
        FlowCondition condTcp = new FlowCondition(null, null);
        assertEquals(UpdateType.ADDED,
                     vtnManager.setFlowCondition(cnameTcp, condTcp));
        assertEquals(null, vtnManager.setFlowCondition(cnameTcp, condTcp));

        TcpMatch tcm = new TcpMatch(null, Short.valueOf(tcpDstPort));
        fm = new FlowMatch(null, null, tcm);
        assertEquals(UpdateType.ADDED,
                     vtnManager.setFlowConditionMatch(cnameTcp, 2, fm));
        assertEquals(null,
                     vtnManager.setFlowConditionMatch(cnameTcp, 2, fm));
        im = new Inet4Match(null, null, null, null,
                            InetProtocols.TCP.shortValue(), null);
        fm = new FlowMatch(2, em, im, tcm);
        condTcp = new FlowCondition(cnameTcp, Collections.singletonList(fm));
        assertEquals(condTcp, vtnManager.getFlowCondition(cnameTcp));

        // dscp_10: Match IPv4 packet assigned 10 as DSCP.
        final byte dscpValue = 10;
        String cnameDscp = "dscp_10";
        FlowCondition condDscp = new FlowCondition(null, null);
        assertEquals(UpdateType.ADDED,
                     vtnManager.setFlowCondition(cnameDscp, condDscp));
        assertEquals(null, vtnManager.setFlowCondition(cnameDscp, condDscp));

        im = new Inet4Match(null, null, null, null, null,
                            Byte.valueOf(dscpValue));
        fm = new FlowMatch(3, em, im, null);
        condDscp = new FlowCondition(cnameDscp, Collections.singletonList(fm));
        assertEquals(UpdateType.CHANGED,
                     vtnManager.setFlowCondition(cnameDscp, condDscp));
        assertEquals(null, vtnManager.setFlowCondition(cnameDscp, condDscp));
        assertEquals(condDscp, vtnManager.getFlowCondition(cnameDscp));

        // Configure path maps.

        // Map udp_src_50000 packets to path policy 1 using VTN-local path map.
        PathMap vpmap1 = new PathMap(cnameUdp, 1);
        assertEquals(UpdateType.ADDED,
                     vtnManager.setPathMap(tpath, 1, vpmap1));
        assertEquals(null, vtnManager.setPathMap(tpath, 1, vpmap1));
        vpmap1 = new PathMap(1, cnameUdp, 1);
        assertEquals(vpmap1, vtnManager.getPathMap(tpath, 1));

        // Map dscp_10 packets to path policy 2 using VTN-local path map.
        int idle2 = 10000;
        int hard2 = 30000;
        PathMap vpmap2 = new PathMap(2, cnameDscp, 2, idle2, hard2);
        assertEquals(UpdateType.ADDED,
                     vtnManager.setPathMap(tpath, 2, vpmap2));
        assertEquals(null, vtnManager.setPathMap(tpath, 2, vpmap2));
        assertEquals(vpmap2, vtnManager.getPathMap(tpath, 2));
        List<PathMap> pmapList = new ArrayList<>();
        Collections.addAll(pmapList, vpmap1, vpmap2);
        assertEquals(pmapList, vtnManager.getPathMaps(tpath));

        // Map tcp_dst_23 packets to path policy 3 using global path map.
        int idle3 = 11111;
        int hard3 = 32000;
        PathMap gpmap = new PathMap(1, cnameTcp, 3, idle3, hard3);
        assertEquals(UpdateType.ADDED, vtnManager.setPathMap(1, gpmap));
        assertEquals(null, vtnManager.setPathMap(1, gpmap));
        assertEquals(gpmap, vtnManager.getPathMap(1));
        assertEquals(Collections.singletonList(gpmap),
                     vtnManager.getPathMaps());

        // Ensure that network topology is established.
        for (Map.Entry<String, String> entry: allLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            ofMockService.awaitLink(src, dst, true);
        }

        // Ensure that the test vBridge is up.
        VNodeStateWaiter waiter = new VNodeStateWaiter(vtnManager).
            set(bpath, VnodeState.UP);
        waiter.await();

        // Let the test vBridge learn MAC addresses.
        for (TestHost host: hosts0) {
            learnHost(bpath, allPorts, host);
        }
        for (TestHost host: hosts2) {
            learnHost(bpath, allPorts, host);
        }

        // Ensure no flow entry is installed.
        FlowCounter counter = new FlowCounter(ofMockService).verify();

        // Send ARP unicast packets that should be mapped by the default
        // route resolver. Ethernet type will be configured in flow match.
        ArpFlowFactory arpfc = new ArpFlowFactory(ofMockService);
        arpfc.addMatchType(FlowMatchType.ETH_TYPE);
        List<UnicastFlow> flows00 =
            unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                        route0);

        // Send UDP packet that should be mapped by the path policy 1.
        // Ethernet type, IPv4 protocol, and UDP source port will be
        // configured in flow match. Although UDP packets will have DSCP
        // value 10, DSCP should not be configured in flow match because
        // udp_src_50000 will be evaluated before dscp_10.
        Udp4FlowFactory udpfc =
            new Udp4FlowFactory(ofMockService, udpSrcPort, (short)53);
        udpfc.setDscp(dscpValue).
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.UDP_SRC);
        List<UnicastFlow> flows1 =
            unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                        route1);

        // Send TCP packet that should be mapped by the path policy 2.
        // Ethernet type, IPv4 protocol, and IP DSCP will be configured in
        // flow match. Although the TCP destination port is 23, packets
        // should be mapped by dscp_10 because it will be evaluated before
        // tcp_dst_23.
        Tcp4FlowFactory tcpfc1 =
            new Tcp4FlowFactory(ofMockService, (short)12345, tcpDstPort);
        tcpfc1.setDscp(dscpValue).
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            setIdleTimeout(idle2).setHardTimeout(hard2);
        List<UnicastFlow> flows2 =
            unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2, 0,
                        route2);

        // Send TCP packet that should be mapped by the path policy 3.
        // Ethernet type, IPv4 protocol, IP DSCP, and TCP destination port
        // will be configured in flow match.
        Tcp4FlowFactory tcpfc2 =
            new Tcp4FlowFactory(ofMockService, (short)12345, tcpDstPort);
        tcpfc2.addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            addMatchType(FlowMatchType.TCP_DST).
            setIdleTimeout(idle3).setHardTimeout(hard3);
        List<UnicastFlow> flows3 =
            unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2, 0,
                        route3);

        // Send TCP packet that should not be matched by any flow condition.
        Tcp4FlowFactory tcpfc3 =
            new Tcp4FlowFactory(ofMockService, (short)333, (short)444);
        tcpfc3.addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            addMatchType(FlowMatchType.TCP_DST);
        List<UnicastFlow> flows01 =
            unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2, 0,
                        route0);

        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Remove path policy 3.
        // This should remove unicast flows routed by the path policy 3.
        Status st = vtnManager.removePathPolicy(3);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, false, true);
        st = vtnManager.removePathPolicy(3);
        assertEquals(StatusCode.NOTFOUND, st.getCode());

        // Restore path policy 3.
        // This should remove all flow entries.
        assertEquals(UpdateType.ADDED, vtnManager.setPathPolicy(3, pp3));
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, false, true);

        PathPolicy pp = vtnManager.getPathPolicy(3);
        assertEquals(Integer.valueOf(3), pp.getPolicyId());
        assertEquals(defCost3, pp.getDefaultCost());
        assertEquals(new HashSet<PathCost>(pp3.getPathCosts()),
                     new HashSet<PathCost>(pp.getPathCosts()));

        // Ensure that no flow entry is installed.
        counter.clear().verify();

        // Restore unicast flows in reverse order.
        flows01 = unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2, 0,
                              route0);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route3);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route2);
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route1);
        flows00 = unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                              route0);

        // Set 1 to the default cost for the path policy 1.
        // This should remove unicast flows routed by the path policy 1.
        assertEquals(defCost1, vtnManager.getPathPolicyDefaultCost(1));
        defCost1 = 1L;
        assertEquals(true, vtnManager.setPathPolicyDefaultCost(1, defCost1));
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        assertEquals(false, vtnManager.setPathPolicyDefaultCost(1, defCost1));
        assertEquals(defCost1, vtnManager.getPathPolicyDefaultCost(1));
        pp = vtnManager.getPathPolicy(1);
        assertEquals(Integer.valueOf(1), pp.getPolicyId());
        assertEquals(defCost1, pp.getDefaultCost());
        assertEquals(new HashSet<PathCost>(pp1.getPathCosts()),
                     new HashSet<PathCost>(pp.getPathCosts()));

        // Now the path policy 1 will choose the shortest path for packets
        // from nodeIds[0] to nodeIds[2].
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route0);

        // Increase the cost for transmitting packet from nodeIds[3] to
        // nodeIds[2] in the path policy 2.
        // This should remove unicast flows routed by the path policy 2.
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        PortLocation ploc2 = toPortLocation(link.getSourcePort(), true);
        assertEquals(PathPolicy.COST_UNDEF,
                     vtnManager.getPathPolicyCost(2, ploc2));
        long cost2 = 200000000L;
        assertEquals(UpdateType.ADDED,
                     vtnManager.setPathPolicyCost(2, ploc2, cost2));
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, true, false);
        assertEquals(null, vtnManager.setPathPolicyCost(2, ploc2, cost2));
        assertEquals(cost2, vtnManager.getPathPolicyCost(2, ploc2));

        costs = new HashSet<>(pp2.getPathCosts());
        assertTrue(costs.add(new PathCost(ploc2, cost2)));
        pp = vtnManager.getPathPolicy(2);
        assertEquals(Integer.valueOf(2), pp.getPolicyId());
        assertEquals(defCost2, pp.getDefaultCost());
        assertEquals(costs, new HashSet<PathCost>(pp.getPathCosts()));

        // Now path policy 2 should route packet from nodeIds[0] to nodeIds[2]
        // as follows:
        //    0 -> 3 -> 1 -> 2
        route2.clear();
        link = getInterSwitchLink(nodeIds[0], nodeIds[3]);
        assertNotNull(link);
        route2.add(link);
        link = getInterSwitchLink(nodeIds[3], nodeIds[1]);
        assertNotNull(link);
        route2.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[2]);
        assertNotNull(link);
        route2.add(link);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route2);
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Remove the cost for transmitting packet from nodeIds[3] to
        // nodeIds[2] in the path policy 3.
        // This should remove unicast flows routed by the path policy 3.
        assertEquals(cost3, vtnManager.getPathPolicyCost(3, ploc2));
        st = vtnManager.removePathPolicyCost(3, ploc2);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, false, true);
        assertEquals(null, vtnManager.removePathPolicyCost(3, ploc2));

        costs = new HashSet<>(pp3.getPathCosts());
        assertTrue(costs.remove(new PathCost(ploc2, cost3)));
        pp = vtnManager.getPathPolicy(3);
        assertEquals(Integer.valueOf(3), pp.getPolicyId());
        assertEquals(defCost3, pp.getDefaultCost());
        assertEquals(costs, new HashSet<PathCost>(pp.getPathCosts()));

        // Now path policy 3 should route packet from nodeIds[0] to nodeIds[2]
        // as follows:
        //    0 -> 1 -> 2
        route3.clear();
        link = getInterSwitchLink(nodeIds[0], nodeIds[1]);
        assertNotNull(link);
        route3.add(link);
        link = getInterSwitchLink(nodeIds[1], nodeIds[2]);
        assertNotNull(link);
        route3.add(link);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route3);

        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);
        counter.clear().add(flows00).add(flows01).add(flows1).
            add(flows2).add(flows3).verify();

        // Remove path policy 2.
        // This will remove unicast flows routed by the path policy 2.
        st = vtnManager.removePathPolicy(2);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, true, false);
        st = vtnManager.removePathPolicy(2);
        assertEquals(StatusCode.NOTFOUND, st.getCode());
        counter.clear().add(flows00).add(flows01).add(flows1).add(flows3).
            verify();

        // The VTN-local path map 2 should be ignored because the path policy
        // 2 is not present. So flows2 should be mapped by the global path 1.
        tcpfc1.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.TCP_DST).
            setIdleTimeout(idle3).setHardTimeout(hard3);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route3);
        counter.clear().add(flows00).add(flows01).add(flows1).add(flows2).
            add(flows3).verify();

        // Remove flow condition tcp_dst_23 used by the global path map 1.
        // This will remove all flow entries.
        st = vtnManager.removeFlowCondition(cnameTcp);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, false, true);
        st = vtnManager.removeFlowCondition(cnameTcp);
        assertEquals(StatusCode.NOTFOUND, st.getCode());
        counter.clear().verify();

        // Configure flow timeout for the VTN-local path map 1.
        int idle1 = 12345;
        int hard1 = 23456;
        vpmap1 = new PathMap(1, cnameUdp, 1, idle1, hard1);
        assertEquals(UpdateType.CHANGED,
                     vtnManager.setPathMap(tpath, 1, vpmap1));
        assertEquals(null, vtnManager.setPathMap(tpath, 1, vpmap1));
        assertEquals(vpmap1, vtnManager.getPathMap(tpath, 1));
        udpfc.setIdleTimeout(idle1).setHardTimeout(hard1);

        // The global path map 2 should be ignored because the flow condition
        // tcp_dst_23 is not present.
        tcpfc2.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            resetTimeout();
        tcpfc3.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO);
        flows00 = unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                              route0);
        flows01 = unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2, 0,
                              route0);
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route0);

        // Unicast flows that will be created by tcpfc1 and tcpfc2 should be
        // forwarded by flow entries craeted by tcpfc3.
        tcpfc1.setAlreadyMapped(true);
        tcpfc2.setAlreadyMapped(true);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route0);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2, 0,
                             route0);
        counter.clear().add(flows00).add(flows01).add(flows1).verify();

        // Remove global path map.
        // This will remove all flow entries.
        st = vtnManager.removePathMap(1);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        counter.clear().verify();
        assertEquals(null, vtnManager.removePathMap(1));

        // Clean up.
        st = vtnManager.clearFlowCondition();
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertEquals(null, vtnManager.clearFlowCondition());

        for (String cond: new String[]{cnameUdp, cnameDscp}) {
            st = vtnManager.removeFlowCondition(cond);
            assertEquals(StatusCode.NOTFOUND, st.getCode());
        }

        st = vtnManager.clearPathPolicy();
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertEquals(null, vtnManager.clearPathPolicy());
        for (int i = 1; i <= 3; i++) {
            st = vtnManager.removePathPolicy(i);
            assertEquals(StatusCode.NOTFOUND, st.getCode());
        }

        st = vtnManager.clearPathMap(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());
        assertEquals(null, vtnManager.clearPathMap(tpath));
        for (int i = 1; i <= 2; i++) {
            assertEquals(null, vtnManager.removePathMap(tpath, i));
        }

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        for (int i = 1; i <= 2; i++) {
            st = vtnManager.removePathMap(tpath, i);
            assertEquals(StatusCode.NOTFOUND, st.getCode());
        }
    }

    /**
     * Determine the inter-switch link between the given nodes.
     *
     * @param src  The source node identifier.
     * @param dst  The destination node identifier.
     * @return  An {@link OfMockLink} instance.
     *          {@code null} if not found.
     */
    private OfMockLink getInterSwitchLink(String src, String dst) {
        String prefix = dst + OfMockService.ID_SEPARATOR;
        for (String pid: ofMockService.getPorts(src, false)) {
            String peer = ofMockService.getPeerIdentifier(pid);
            assertNotNull(peer);
            if (peer.startsWith(prefix)) {
                return new OfMockLink(pid, peer);
            }
        }

        return null;
    }

    /**
     * Test method for {@link IHostFinder}
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIHostFinder() throws Exception {
        LOG.info("Running testIHostFinder().");

        IVTNManager mgr = vtnManager;
        Status st = null;

        String tname = "vtn";
        VTenantPath tpath = new VTenantPath(tname);
        String bname = "vbridge";
        VBridgePath bpath = new VBridgePath(tname, bname);
        List<VBridgePath> bpathlist = new ArrayList<VBridgePath>();
        bpathlist.add(bpath);
        createVirtualNodes(tpath, bpathlist);

        testIHostFinderCommon(bpath);

        st = vtnManager.removeTenant(tpath);
        assertEquals(StatusCode.SUCCESS, st.getCode());

        // no tenant case
        testIHostFinderCommon(null);
    }

    /**
     * Common routine for IHostFinder test.
     *
     * @param bpath  Path to the test vBridge.
     * @throws Exception  An error occurred.
     */
    private void testIHostFinderCommon(VBridgePath bpath) throws Exception {
        IVTNManager mgr = vtnManager;

        // Determine edge ports.
        List<TestPort> edgePorts = new ArrayList<>();
        Set<String> allPorts = new HashSet<>();
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                assertTrue(allPorts.add(pid));
                TestPort tp = new TestPort(pid);
                edgePorts.add(tp);
            }
            for (String pid: ofMockService.getPorts(nid, false)) {
                assertTrue(allPorts.add(pid));
            }
        }

        assertFalse(edgePorts.isEmpty());
        assertTrue(allPorts.size() > edgePorts.size());

        short[] vlans = {0, 10, 4095};
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

        byte[] ctlrMac = ofMockService.getControllerMacAddress();

        EthernetFactory efc = new EthernetFactory().setSourceAddress(ctlrMac);
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(ctlrMac).
            setTargetHardwareAddress(MAC_ZERO).
            setSenderProtocolAddress(IPV4_ZERO).
            setTargetProtocolAddress(ia.getAddress());
        for (short vlan: vlans) {
            VlanMap map = null;
            if (bpath != null) {
                VlanMapConfig vlconf = new VlanMapConfig(null, vlan);
                map = mgr.addVlanMap(bpath, vlconf);
            }

            // All packets should be discarded unless VTN is present.
            String emsg =
                "(bpath)" + ((bpath == null) ? "(null)" : bpath.toString()) +
                "(vlan)" + vlan;
            hostFinder.find(ia);
            if (bpath != null) {
                efc.setDestinationAddress(MAC_BROADCAST);
                afc.setTargetHardwareAddress(MAC_ZERO);
                for (TestPort tp: edgePorts) {
                    String pid = tp.getPortIdentifier();
                    byte[] payload = ofMockService.awaitTransmittedPacket(pid);
                    efc.setVlanId(vlan).verify(ofMockService, payload);
                }
            }
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }

            // IPv6 Address should be ignored.
            hostFinder.find(ia6);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }

            // Null IP address should be ignored.
            hostFinder.find(null);
            sleep(SHORT_DELAY);
            for (String p: allPorts) {
                assertNull(ofMockService.getTransmittedPacket(p));
            }

            // probe()
            byte [] mac = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                      (byte)0x11, (byte)0x22, (byte)0x33};
            efc.setDestinationAddress(mac);
            afc.setTargetHardwareAddress(mac);
            for (TestPort tp: edgePorts) {
                NodeConnector nc = tp.getNodeConnector();
                HostNodeConnector hnode =
                    new HostNodeConnector(mac, ia, nc, vlan);
                hostFinder.probe(hnode);

                if (bpath != null) {
                    String pid = tp.getPortIdentifier();
                    byte[] payload = ofMockService.awaitTransmittedPacket(pid);
                    efc.setVlanId(vlan).verify(ofMockService, payload);
                }
                sleep(SHORT_DELAY);
                for (String p: allPorts) {
                    assertNull(ofMockService.getTransmittedPacket(p));
                }

                // IPv6 Address should be ignored.
                hnode = new HostNodeConnector(mac, ia6, nc, vlan);
                hostFinder.probe(hnode);
                sleep(SHORT_DELAY);
                for (String p: allPorts) {
                    assertNull(ofMockService.getTransmittedPacket(p));
                }

                // Null IP address should be ignored.
                hostFinder.probe(null);
                sleep(SHORT_DELAY);
                for (String p: allPorts) {
                    assertNull(ofMockService.getTransmittedPacket(p));
                }
            }

            if (map != null) {
                Status st = mgr.removeVlanMap(bpath, map.getId());
                assertEquals(emsg, StatusCode.SUCCESS, st.getCode());
            }
        }
    }

    /**
     * Test method for the static network topology configuration.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testStaticTopology() throws Exception {
        LOG.info("Running testStaticTopology().");

        // Below are initial topology configured by ofmock.
        SwitchLinkMap linkMap = new SwitchLinkMap(ofMockService);
        String port1000n1 = "openflow:1000:1";
        String port1000n2 = "openflow:1000:2";
        String port1001n1 = "openflow:1001:1";
        String port1002n1 = "openflow:1002:1";
        linkMap.add(port1000n1, port1001n1);
        linkMap.add(port1001n1, port1000n1);
        linkMap.add(port1000n2, port1002n1);
        linkMap.add(port1002n1, port1000n2);
        linkMap.verify();
        SwitchLinkMap initial = new SwitchLinkMap(linkMap);

        // Configuration should be ignored if the port is not present.
        StaticTopology stopo = new StaticTopology(ofMockService);
        stopo.addSwitchLink("openflow:111:222", "openflow:333:444");
        stopo.addSwitchLink("unknown:1:2", "unknown:3:4");
        stopo.addEdgePort("openflow:555:666");
        stopo.addEdgePort("unknown:5:6");
        sleep(10);
        initial.verify();

        // Invalid configuration should be ignored.
        stopo.addSwitchLink(port1000n1, port1000n1);
        stopo.addSwitchLink(port1000n2, port1000n2);
        sleep(10);
        initial.verify();

        // Configure a static link from openflow:1000:1 to openflow:1001:2.
        String port1001n2 = "openflow:1001:2";
        stopo.addSwitchLink(port1000n1, port1001n2);
        stopo.apply();
        linkMap.add(port1000n1, port1001n2, true);
        linkMap.addIgnored(port1000n1, port1001n1);
        linkMap.verify();

        // Configure a static link from openflow:1001:2 to openflow:1000:1.
        stopo.addSwitchLink(port1001n2, port1000n1);
        stopo.apply();
        linkMap.add(port1001n2, port1000n1, true);
        linkMap.verify();

        // Configure "openflow:1001:1" as an edge port.
        stopo.addEdgePort(port1001n1);
        stopo.apply();
        linkMap.remove(port1001n1);
        linkMap.addIgnored(port1001n1, port1000n1);
        linkMap.verify();

        // Add the following configuration at once.
        //   add link between openflow:1000:2 and openflow:1002:2
        //   make openflow:1002:1 as an edge port
        String port1002n2 = "openflow:1002:2";
        stopo.addSwitchLink(port1000n2, port1002n2);
        stopo.addSwitchLink(port1002n2, port1000n2);
        stopo.addEdgePort(port1002n1);
        stopo.apply();
        linkMap.add(port1000n2, port1002n2, true);
        linkMap.add(port1002n2, port1000n2, true);
        linkMap.remove(port1002n1);
        linkMap.addIgnored(port1000n2, port1002n1);
        linkMap.addIgnored(port1002n1, port1000n2);
        linkMap.verify();

        // Add a static link between openflow:1001:3 and openflow:1002:3.
        String port1001n3 = "openflow:1001:3";
        String port1002n3 = "openflow:1002:3";
        stopo.addSwitchLink(port1001n3, port1002n3);
        stopo.addSwitchLink(port1002n3, port1001n3);
        stopo.apply();
        linkMap.add(port1001n3, port1002n3, true);
        linkMap.add(port1002n3, port1001n3, true);
        linkMap.verify();

        // Add the following configuration at once.
        //   add link between openflow:1000:1 and openflow:1001:3
        //   add link between openflow:1000:2 and openflow:1002:3
        //   add link between openflow:1001:2 and openflow:1002:2
        stopo.addSwitchLink(port1000n1, port1001n3);
        stopo.addSwitchLink(port1001n3, port1000n1);
        stopo.addSwitchLink(port1000n2, port1002n3);
        stopo.addSwitchLink(port1002n3, port1000n2);
        stopo.addSwitchLink(port1001n2, port1002n2);
        stopo.addSwitchLink(port1002n2, port1001n2);
        stopo.apply();
        linkMap.add(port1000n1, port1001n3, true);
        linkMap.add(port1001n3, port1000n1, true);
        linkMap.add(port1000n2, port1002n3, true);
        linkMap.add(port1002n3, port1000n2, true);
        linkMap.add(port1001n2, port1002n2, true);
        linkMap.add(port1002n2, port1001n2, true);
        linkMap.verify();

        // Put all the switch ports to static-edge-ports.
        StaticTopology stopo1 = new StaticTopology(stopo);
        SwitchLinkMap linkMap1 = new SwitchLinkMap(linkMap);
        String[] allPorts = {
            port1000n1,
            port1000n2,
            port1001n1,
            port1001n2,
            port1001n3,
            port1002n1,
            port1002n2,
            port1002n3,
        };
        for (String port: allPorts) {
            stopo1.addEdgePort(port);
            linkMap1.remove(port);
        }
        stopo1.apply();
        linkMap1.verify();
        stopo1 = null;
        linkMap1 = null;

        // Restore the configuration.
        stopo.apply();
        linkMap.verify();

        // Configure static network topology for new switches.
        //   openflow:1:1 <-> openflow:2:1
        //   openflow:1:2 <-> openflow:3:1
        //   openflow:1:3 <-> openflow:4:1
        //   openflow:2:2 <-> openflow:3:2
        //   openflow:2:3 <-> openflow:4:2
        //   openflow:3:3 <-> openflow:4:3
        for (int i = 1; i <= 3; i++) {
            for (int j = i; j <= 3; j++) {
                String src = String.format("openflow:%d:%d", i, j);
                String dst = String.format("openflow:%d:%d", j + 1, i);
                stopo.addSwitchLink(src, dst);
                stopo.addSwitchLink(dst, src);
            }
        }

        stopo.apply();
        sleep(10);
        linkMap.verify();
        SwitchLinkMap savedLinks = new SwitchLinkMap(linkMap);

        // Create 4 nodes, and 4 ports per node.
        List<String> newNodes = new ArrayList<>();
        List<String> newPorts = new ArrayList<>();
        for (long dpid = 1L; dpid <= 4L; dpid++) {
            newNodes.add(ofMockService.addNode(BigInteger.valueOf(dpid)));
            String nid = "openflow:" + dpid;
            for (long idx = 1; idx <= 4L; idx++) {
                newPorts.add(ofMockService.addPort(nid, idx, false));
            }
        }

        // Static network topology should not be changed because all the
        // created ports are still in DOWN state.
        sleep(10);
        savedLinks.verify();

        // Up all the created ports.
        // Static network topology configuration should be appied to newly
        // detected inventories.
        for (String pid: newPorts) {
            ofMockService.setPortState(pid, true, false);
        }
        for (int i = 1; i <= 3; i++) {
            for (int j = i; j <= 3; j++) {
                String src = String.format("openflow:%d:%d", i, j);
                String dst = String.format("openflow:%d:%d", j + 1, i);
                linkMap.add(src, dst, true);
                linkMap.add(dst, src, true);
            }
        }
        linkMap.verify();

        // Establish dynamic links.
        // They will be put into ignored-links container because static links
        // are already established.
        String port1n1 = "openflow:1:1";
        String port2n2 = "openflow:2:2";
        String port3n3 = "openflow:3:3";
        String port4n3 = "openflow:4:3";
        ofMockService.setPeerIdentifier(port1n1, port2n2, false);
        ofMockService.setPeerIdentifier(port2n2, port1n1, false);
        ofMockService.setPeerIdentifier(port3n3, port4n3, false);
        ofMockService.setPeerIdentifier(port4n3, port3n3, false);

        linkMap.addIgnored(port1n1, port2n2);
        linkMap.addIgnored(port2n2, port1n1);
        linkMap.addIgnored(port3n3, port4n3);
        linkMap.addIgnored(port4n3, port3n3);
        linkMap.verify();

        // Down port openflow:1:1, openflow:1:3, and openflow:2:2.
        // Below static links should be removed.
        //   openflow:1:1 <-> openflow:2:1
        //   openflow:1:3 <-> openflow:4:1
        //   openflow:2:2 <-> openflow:3:2
        String port1n3 = "openflow:1:3";
        String port2n1 = "openflow:2:1";
        String port3n2 = "openflow:3:2";
        String port4n1 = "openflow:4:1";
        ofMockService.setPortState(port1n1, false, false);
        ofMockService.setPortState(port1n3, false, false);
        ofMockService.setPortState(port2n2, false, false);
        linkMap1 = new SwitchLinkMap(linkMap);
        linkMap1.remove(port1n1);
        linkMap1.remove(port2n1);
        linkMap1.remove(port1n3);
        linkMap1.remove(port4n1);
        linkMap1.remove(port2n2);
        linkMap1.remove(port3n2);

        // openflow:1:1 <-> openflow:2:2 in ignored-links should also be
        // removed because topology-manager will remove it.
        linkMap1.removeIgnored(port1n1);
        linkMap1.removeIgnored(port2n2);
        linkMap1.verify();
        linkMap1 = null;

        // Restore port state.
        ofMockService.setPortState(port1n1, true, false);
        ofMockService.setPortState(port1n3, true, false);
        ofMockService.setPortState(port2n2, true, false);
        linkMap.verify();

        // Delete static link configuration for openflow:3:3 <-> openflow:4:3.
        stopo.removeSwitchLink(port3n3);
        stopo.removeSwitchLink(port4n3);
        stopo.apply();
        linkMap.add(port3n3, port4n3);
        linkMap.add(port4n3, port3n3);
        linkMap.removeIgnored(port3n3);
        linkMap.removeIgnored(port4n3);
        linkMap.verify();

        // Add openflow:2:1 and openflow:2:3 to the static edge port
        // configuration. This should remove below static links.
        //   openflow:1:1 <-> openflow:2:1
        //   openflow:2:3 <-> openflow:4:2
        String port2n3 = "openflow:2:3";
        String port4n2 = "openflow:4:2";
        stopo.addEdgePort(port2n1);
        stopo.addEdgePort(port2n3);
        stopo.apply();
        linkMap1 = new SwitchLinkMap(linkMap);
        linkMap1.remove(port2n1);
        linkMap1.remove(port1n1);
        linkMap1.remove(port2n3);
        linkMap1.remove(port4n2);

        // openflow:1:1 -> openflow:2:2 in the ignored-links container should
        // be restored. Note that openflow:2:2 -> openflow:1:1 should not be
        // restored because static link configuration is present for
        // openflow:2:2. Although static link configuration for openflow:1:1
        // (openflow:1:1 -> openflow:2:1) is present, it will be ignored
        // because the destination port (openflow:2:1) is configured as a
        // static edge port.
        linkMap1.add(port1n1, port2n2);
        linkMap1.removeIgnored(port1n1);
        linkMap1.verify();
        linkMap1 = null;

        // Restore configuration.
        stopo.removeEdgePort(port2n1);
        stopo.removeEdgePort(port2n3);
        stopo.apply();
        linkMap.verify();

        // Remove all nodes created by this test.
        for (String nid: newNodes) {
            ofMockService.removeNode(nid);
        }
        savedLinks.verify();

        // Delete static network topology configuration.
        // This should restore initial network topology.
        stopo.clear();
        stopo.apply();
        initial.verify();
    }

    /**
     * Create virtual nodes.
     *
     * @param tpath     Path to the virtual tenant to be created.
     * @param bpaths    A collection of vBridge paths to be created.
     */
    private void createVirtualNodes(VTenantPath tpath,
                                    Collection<VBridgePath> bpaths) {
        createVirtualNodes(tpath, bpaths, null, null, null);
    }

    /**
     * Create virtual nodes.
     *
     * @param tpath     Path to the virtual tenant to be created.
     * @param bpaths    A collection of vBridge paths to be created.
     * @param bipaths   A collection of vBridge interface paths to be created.
     */
    private void createVirtualNodes(VTenantPath tpath,
                                    Collection<VBridgePath> bpaths,
                                    Collection<VBridgeIfPath> bipaths) {
        createVirtualNodes(tpath, bpaths, bipaths, null, null);
    }

    /**
     * Create virtual nodes.
     *
     * @param tpath     Path to the virtual tenant to be created.
     * @param bpaths    A collection of vBridge paths to be created.
     * @param bipaths   A collection of vBridge interface paths to be created.
     * @param vtpaths   A collection of vTerminal paths to be created.
     * @param vtipaths  A collection of vTerminal interface paths to be
     *                  created.
     */
    private void createVirtualNodes(VTenantPath tpath,
                                    Collection<VBridgePath> bpaths,
                                    Collection<VBridgeIfPath> bipaths,
                                    Collection<VTerminalPath> vtpaths,
                                    Collection<VTerminalIfPath> vtipaths) {
        Status st;
        if (tpath != null) {
            st = vtnManager.addTenant(tpath, new VTenantConfig(null));
            assertEquals(StatusCode.SUCCESS, st.getCode());
        }
        assertTrue(vtnManager.isActive());

        if (bpaths != null) {
            VBridgeConfig bconf = new VBridgeConfig(null, 1000000);
            for (VBridgePath bpath: bpaths) {
                st = vtnManager.addBridge(bpath, bconf);
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        }

        if (bipaths != null) {
            for (VBridgeIfPath ifpath: bipaths) {
                VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
                st = vtnManager.addInterface(ifpath, ifconf);
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        }

        if (vtpaths != null) {
            for (VTerminalPath vtpath: vtpaths) {
                st = vtnManager.addTerminal(vtpath, new VTerminalConfig(null));
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        }

        if (vtipaths != null) {
            for (VTerminalIfPath ifpath: vtipaths) {
                VInterfaceConfig ifconf = new VInterfaceConfig(null, null);
                st = vtnManager.addInterface(ifpath, ifconf);
                assertEquals(StatusCode.SUCCESS, st.getCode());
            }
        }
    }
}
