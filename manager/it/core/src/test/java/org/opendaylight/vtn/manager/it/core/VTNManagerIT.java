/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.ops4j.pax.exam.CoreOptions.options;

import static org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet.clearPathMap;
import static org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig.removeVtn;

import java.math.BigInteger;

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
import org.osgi.framework.Version;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.option.TestOption;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.NextFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappings;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.GetManagerVersionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.VtnVersionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.get.manager.version.output.BundleVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;

/**
 * Integration test for the VTN Manager using openflowplugin mock-up.
 */
@RunWith(PaxExam.class)
@ExamReactorStrategy(PerClass.class)
public final class VTNManagerIT extends ModelDrivenTestBase
    implements VTNServices {
    /**
     * Logger instance.
     */
    static final Logger  LOG = LoggerFactory.getLogger(VTNManagerIT.class);

    /**
     * Set true only if the first run.
     */
    private static boolean  firstRun = true;

    /**
     * OSGi bundle context.
     */
    @Inject
    private BundleContext  bundleContext;

    /**
     * openflowplugin mock-up service.
     */
    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private OfMockService  ofMockService;

    /**
     * OSGi bundle for the manager.implementation bundle.
     */
    private Bundle  implBundle;

    /**
     * vtn RPC service.
     */
    private VtnService  vtnService;

    /**
     * vtn-vbridge RPC service.
     */
    private VtnVbridgeService  vbridgeService;

    /**
     * vtn-vterminal RPC service.
     */
    private VtnVterminalService  vterminalService;

    /**
     * vtn-vinterface RPC service.
     */
    private VtnVinterfaceService  vinterfaceService;

    /**
     * vtn-mac-table RPC service.
     */
    private VtnMacTableService  macTableService;

    /**
     * vtn-vlan-map RPC service.
     */
    private VtnVlanMapService  vlanMapService;

    /**
     * vtn-mac-map RPC service.
     */
    private VtnMacMapService  macMapService;

    /**
     * vtn-port-map RPC service.
     */
    private VtnPortMapService  portMapService;

    /**
     * vtn-flow-filter RPC service.
     */
    private VtnFlowFilterService  flowFilterService;

    /**
     * vtn-flow-condition RPC service.
     */
    private VtnFlowConditionService  flowCondService;

    /**
     * vtn-path-policy RPC service.
     */
    private VtnPathPolicyService  pathPolicyService;

    /**
     * vtn-path-map RPC service.
     */
    private VtnPathMapService  pathMapService;

    /**
     * vtn-version RPC service.
     */
    private VtnVersionService  versionService;

    /**
     * Configure the OSGi container.
     *
     * @return  An array of test options.
     */
    @Configuration
    public Option[] config() {
        return options(TestOption.vtnManagerCommonBundles());
    }

    /**
     * Verify the test environment.
     *
     * @throws Exception  An error occurred.
     */
    @Before
    public void areWeReady() throws Exception {
        assertNotNull(bundleContext);
        assertNotNull(ofMockService);

        // Determine manager.implementation bundle.
        implBundle = getManagerBundle(bundleContext);
        assertNotNull(implBundle);
        assertEquals(Bundle.ACTIVE, implBundle.getState());

        // Initialize the openflowplugin mock-up.
        ofMockService.initialize();

        // Get VTN RPC services.
        vtnService = ofMockService.getRpcService(VtnService.class);
        vbridgeService = ofMockService.getRpcService(VtnVbridgeService.class);
        vterminalService =
            ofMockService.getRpcService(VtnVterminalService.class);
        vinterfaceService =
            ofMockService.getRpcService(VtnVinterfaceService.class);
        macTableService =
            ofMockService.getRpcService(VtnMacTableService.class);
        vlanMapService = ofMockService.getRpcService(VtnVlanMapService.class);
        macMapService = ofMockService.getRpcService(VtnMacMapService.class);
        portMapService = ofMockService.getRpcService(VtnPortMapService.class);
        flowFilterService =
            ofMockService.getRpcService(VtnFlowFilterService.class);
        flowCondService =
            ofMockService.getRpcService(VtnFlowConditionService.class);
        pathPolicyService =
            ofMockService.getRpcService(VtnPathPolicyService.class);
        pathMapService = ofMockService.getRpcService(VtnPathMapService.class);
        versionService = ofMockService.getRpcService(VtnVersionService.class);

        if (firstRun) {
            // Verify initial state.
            try (ReadOnlyTransaction rtx = newReadOnlyTransaction()) {
                checkContainers(rtx);
            }
            firstRun = false;
        }
    }

    /**
     * Called when a test suite quits.
     *
     * @throws Exception  An error occurred.
     */
    @After
    public void tearDown() throws Exception {
        if (versionService != null) {
            // Remove all global path maps.
            clearPathMap(pathMapService, null);

            // Remove all flow conditions.
            getRpcOutput(flowCondService.clearFlowCondition());

            // Remove all path policies.
            getRpcOutput(pathPolicyService.clearPathPolicy());

            // Remove all VTNs.
            for (Vtn vtn: getVtns(this)) {
                removeVtn(vtnService, vtn.getName().getValue());
            }

            // Reset the inventory configuration.
            ofMockService.reset();
        }
    }

    /**
     * Test case for {@link VtnVersionService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVtnVersionService() throws Exception {
        LOG.info("Running testVtnVersionService().");

        GetManagerVersionOutput output =
            getRpcOutput(versionService.getManagerVersion());
        Long api = output.getApiVersion();
        assertNotNull(api);
        assertTrue("API version = " + api, api.longValue() > 0L);

        BundleVersion bv = output.getBundleVersion();
        assertNotNull(bv);

        // Determine actual bundle version of manager.implementation.
        assertNotNull(implBundle);
        Version ver = implBundle.getVersion();
        assertNotNull(ver);

        assertEquals(ver.getMajor(), bv.getMajor().intValue());
        assertEquals(ver.getMinor(), bv.getMinor().intValue());
        assertEquals(ver.getMicro(), bv.getMicro().intValue());

        String qf = ver.getQualifier();
        if (qf == null || qf.isEmpty()) {
            assertNull(bv.getQualifier());
        } else {
            assertEquals(qf, bv.getQualifier());
        }
    }

    /**
     * Test case for {@link VtnService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVtnService() throws Exception {
        LOG.info("Running testVtnService().");
        new VtnServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnVbridgeService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVbridgeService() throws Exception {
        LOG.info("Running testVbridgeService().");
        new VbridgeServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnVterminalService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVterminalService() throws Exception {
        LOG.info("Running testVbridgeService().");
        new VterminalServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnVinterfaceService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVinterfaceService() throws Exception {
        LOG.info("Running testVinterfaceService().");
        new VinterfaceServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnVlanMapService}.
     *
     * <p>
     *   This test is independent of inventory information.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVlanMapService() throws Exception {
        LOG.info("Running testVlanMapService().");
        new VlanMapServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnVlanMapService} that depends on inventory
     * information.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVlanMapStatus() throws Exception {
        LOG.info("Running testVlanMapStatus().");
        new VlanMapStatusTest(this).runTest();
    }

    /**
     * Test case for {@link VtnPortMapService}.
     *
     * <p>
     *   This test is independent of inventory information.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPortMapService() throws Exception {
        LOG.info("Running testPortMapService().");
        new PortMapServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnPortMapService} that depends on inventory
     * information.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPortMapStatus() throws Exception {
        LOG.info("Running testPortMapStatus().");
        new PortMapStatusTest(this).runTest();
    }

    /**
     * Test case for MAC address table management and
     * {@link VtnMacTableService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMacTableService() throws Exception {
        LOG.info("Running testMacTableService().");
        new MacTableServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnFlowFilterService}.
     *
     * <p>
     *   This test is independent of inventory information.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFlowFilterService() throws Exception {
        LOG.info("Running testFlowFilterService().");
        new FlowFilterServiceTest(this).runTest();
    }

    /**
     * Test case for {@link VtnPathPolicyService}.
     *
     * <p>
     *   This test is independent of inventory information.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPathPolicyService() throws Exception {
        LOG.info("Running testPathPolicyService().");
        new PathPolicyServiceTest(this).runTest();
    }

    /**
     * Ensure that the state of virtual bridge and virtual interface are
     * changed according to inventory events.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInventoryEvent() throws Exception {
        LOG.info("Running testInventoryEvent().");
        new InventoryEventTest(this).runTest();
    }

    /**
     * Unicast flow test.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUnicastFlow() throws Exception {
        LOG.info("Running testUnicastFlow().");
        new UnicastFlowTest(this).runTest();
    }

    /**
     * Test method for the packet routing table maintenance.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRoutingTable() throws Exception {
        LOG.info("Running testRoutingTable().");
        new RoutingTableTest(this).runTest();
    }

    /**
     * Test case for path map.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPathMap() throws Exception {
        LOG.info("Running testPathMap().");
        new PathMapTest(this).runTest();
    }

    /**
     * Test method for the static network topology configuration.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testStaticTopology() throws Exception {
        LOG.info("Running testStaticTopology().");
        new StaticTopologyTest(this).runTest();
    }

    /**
     * Return the openflowplugin mock-up service.
     *
     * @return  An {@link OfMockService} instance.
     */
    public OfMockService getOfMockService() {
        return ofMockService;
    }

    /**
     * Ensure that mandatory containers are initialized collectly.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    private void checkContainers(ReadTransaction rtx) {
        // Ensure that the VTN tree is initialized.
        Vtns vtns = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(Vtns.class)).orNull();
        assertNotNull(vtns);
        assertEquals(null, vtns.getVtn());

        // Ensure that the MAC address table tree is initialized.
        MacTables mtables = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(MacTables.class)).orNull();
        assertNotNull(mtables);
        assertEquals(null, mtables.getTenantMacTable());

        // Ensure that the VTN configuration is initialized.
        VtnConfig config = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(VtnConfig.class)).orNull();
        assertNotNull(config);

        // Ensure that the flow condition tree is initialized.
        VtnFlowConditions conds = DataStoreUtils.read(
            rtx,  InstanceIdentifier.create(VtnFlowConditions.class)).orNull();
        assertNotNull(conds);
        assertEquals(null, conds.getVtnFlowCondition());

        // Ensure that the global path map tree is initialized.
        GlobalPathMaps pathMaps = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(GlobalPathMaps.class)).orNull();
        assertNotNull(pathMaps);
        assertEquals(null, pathMaps.getVtnPathMap());

        // Ensure that the path policy tree is initialized.
        VtnPathPolicies pathPolicies = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(VtnPathPolicies.class)).orNull();
        assertNotNull(pathPolicies);
        assertEquals(null, pathPolicies.getVtnPathPolicy());

        // Ensure that the internal containers for flow management are
        // initialized.
        VtnFlows flows = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(VtnFlows.class)).orNull();
        assertNotNull(flows);
        assertEquals(null, flows.getVtnFlowTable());

        NextFlowId nextId = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(NextFlowId.class)).orNull();
        assertNotNull(nextId);
        assertEquals(BigInteger.ONE, nextId.getNextId().getValue());

        // Ensure that the internal container for virtual network mapping
        // management is initialized.
        VtnMappings mappings = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(VtnMappings.class)).orNull();
        assertNotNull(mappings);
        assertEquals(null, mappings.getVlanMapping());
        assertEquals(null, mappings.getPortMapping());
        assertEquals(null, mappings.getMacMapAllowed());
        assertEquals(null, mappings.getMacMapDenied());

        // Ensure that the internal containers for inventory management are
        // initialized.
        VtnNodes nodes = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(VtnNodes.class)).orNull();
        assertNotNull(nodes);
        assertNotNull(nodes.getVtnNode());

        VtnTopology topo = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(VtnTopology.class)).orNull();
        assertNotNull(topo);
        assertNotNull(topo.getVtnLink());

        IgnoredLinks ignored = DataStoreUtils.read(
            rtx, InstanceIdentifier.create(IgnoredLinks.class)).orNull();
        assertNotNull(ignored);
        assertEquals(null, ignored.getIgnoredLink());
    }

    // VTNServices

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnService getVtnService() {
        return vtnService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnVbridgeService getVbridgeService() {
        return vbridgeService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnVterminalService getVterminalService() {
        return vterminalService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnVinterfaceService getVinterfaceService() {
        return vinterfaceService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnMacTableService getMacTableService() {
        return macTableService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnVlanMapService getVlanMapService() {
        return vlanMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnMacMapService getMacMapService() {
        return macMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnPortMapService getPortMapService() {
        return portMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnFlowFilterService getFlowFilterService() {
        return flowFilterService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnFlowConditionService getFlowConditionService() {
        return flowCondService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnPathPolicyService getPathPolicyService() {
        return pathPolicyService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnPathMapService getPathMapService() {
        return pathMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VtnVersionService getVersionService() {
        return versionService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ReadOnlyTransaction newReadOnlyTransaction() {
        return ofMockService.newReadOnlyTransaction();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ReadWriteTransaction newReadWriteTransaction() {
        return ofMockService.newReadWriteTransaction();
    }
}
