/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.ops4j.pax.exam.CoreOptions.options;

import static org.opendaylight.vtn.manager.util.NumberUtils.getUnsigned;

import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;
import static org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondSet.removeFlowCondition;
import static org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet.clearPathMap;
import static org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet.removePathMap;
import static org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicySet.removePathPolicy;
import static org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig.removeVbridge;
import static org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig.removeVinterface;
import static org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig.removePortMap;
import static org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig.removeVlanMap;
import static org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig.removeVtn;
import static org.opendaylight.vtn.manager.it.util.vnode.VTerminalConfig.removeVterminal;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import javax.inject.Inject;

import com.google.common.base.Optional;

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

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.ofmock.OfMockFlow;
import org.opendaylight.vtn.manager.it.ofmock.OfMockLink;
import org.opendaylight.vtn.manager.it.ofmock.OfMockService;
import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;
import org.opendaylight.vtn.manager.it.option.TestOption;
import org.opendaylight.vtn.manager.it.util.BridgeNetwork;
import org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase;
import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.TestHost;
import org.opendaylight.vtn.manager.it.util.TestPort;
import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.it.util.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNInet4Match;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNTcpMatch;
import org.opendaylight.vtn.manager.it.util.flow.match.VTNUdpMatch;
import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.it.util.inventory.PortVlan;
import org.opendaylight.vtn.manager.it.util.match.FlowMatchType;
import org.opendaylight.vtn.manager.it.util.packet.ArpFactory;
import org.opendaylight.vtn.manager.it.util.packet.EthernetFactory;
import org.opendaylight.vtn.manager.it.util.packet.Icmp4Factory;
import org.opendaylight.vtn.manager.it.util.packet.Inet4Factory;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMap;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathCost;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicy;
import org.opendaylight.vtn.manager.it.util.unicast.ArpFlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.Tcp4FlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.Udp4FlowFactory;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlow;
import org.opendaylight.vtn.manager.it.util.unicast.UnicastFlowFactory;
import org.opendaylight.vtn.manager.it.util.vnode.BridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VInterfaceIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNPortMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTNVlanMapConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTerminalIfIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.NextFlowId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.VtnFlows;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.mapping.rev151001.VtnMappings;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.VtnTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.RemovePortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.SetPortMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.AddVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.GetVlanMapOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.RemoveVlanMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.RemoveMacEntryOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.remove.mac.entry.output.RemoveMacEntryResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.GetManagerVersionOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.VtnVersionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.get.manager.version.output.BundleVersion;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.RemoveVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.UpdateVinterfaceInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.RemoveVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.UpdateVterminalInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.rev131026.flow.Match;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

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
    private static final Logger LOG = LoggerFactory.
        getLogger(VTNManagerIT.class);

    /**
     * The bit that indicates the port ID should be specified in the
     * vtn-port-desc.
     */
    private static final int  PATH_COST_MODE_ID = 0x1;

    /**
     * The bit that indicates the port name should be specified in the
     * vtn-port-desc.
     */
    private static final int  PATH_COST_MODE_NAME = 0x2;

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

        // No VTN should be present.
        VirtualNetwork vnet = new VirtualNetwork(this);
        vnet.verify();

        // Create 3 VTNs.
        String tname1 = "vtn_1";
        VTenantConfig tconf1 = new VTenantConfig();
        vnet.addTenant(tname1, tconf1);
        assertEquals(VtnUpdateType.CREATED,
                     tconf1.update(vtnService, tname1, null, null));
        vnet.verify();

        String tname2 = "vtn_2";
        VTenantConfig tconf2 = new VTenantConfig("vtn-2", 0, null);
        vnet.addTenant(tname2, tconf2);
        assertEquals(VtnUpdateType.CREATED,
                     tconf2.update(vtnService, tname2,
                                   VnodeUpdateMode.CREATE, null));
        vnet.verify();

        String tname3 = "0123456789012345678901234567890";
        VTenantConfig tconf3 =
            new VTenantConfig("Virtual Tenant Network 3", 65534, 65535);
        vnet.addTenant(tname3, tconf3);
        assertEquals(VtnUpdateType.CREATED,
                     tconf3.update(vtnService, tname3,
                                   VnodeUpdateMode.UPDATE, null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(0x12345L);
        for (VTenantConfig tconf: vnet.getTenants().values()) {
            tconf.add(rand);
        }

        vnet.apply().verify();

        // Try to update with the same parameter.
        VnodeUpdateMode[] modifyModes = {
            null,
            VnodeUpdateMode.UPDATE,
            VnodeUpdateMode.MODIFY,
        };
        VtnUpdateOperationType[] modifyOperations = {
            null,
            VtnUpdateOperationType.SET,
            VtnUpdateOperationType.ADD,
        };
        for (Entry<String, VTenantConfig> entry:
                 vnet.getTenants().entrySet()) {
            String name = entry.getKey();
            VTenantConfig tconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null,
                                 tconf.update(vtnService, name, mode, op));
                }
            }
        }

        // Change parameters using ADD operation.
        tconf3.setIdleTimeout(12345);
        UpdateVtnInput input = new UpdateVtnInputBuilder().
            setTenantName(tname3).
            setIdleTimeout(tconf3.getIdleTimeout()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtnService.updateVtn(input)));
        vnet.verify();

        tconf2.setHardTimeout(60000);
        input = new UpdateVtnInputBuilder().
            setTenantName(tname2).
            setHardTimeout(tconf2.getHardTimeout()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtnService.updateVtn(input)));
        vnet.verify();

        tconf2.setDescription("test VTN 2");
        input = new UpdateVtnInputBuilder().
            setTenantName(tname2).
            setDescription(tconf2.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vtnService.updateVtn(input)));
        vnet.verify();

        // Change parameters using SET operation.
        tconf1.setDescription("test VTN 1").
            setIdleTimeout(1).
            setHardTimeout(2);
        assertEquals(VtnUpdateType.CHANGED,
                     tconf1.update(vtnService, tname1, null,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        tconf2.setDescription(null).
            setHardTimeout(33333);
        assertEquals(VtnUpdateType.CHANGED,
                     tconf2.update(vtnService, tname2, VnodeUpdateMode.UPDATE,
                                   VtnUpdateOperationType.SET));

        tconf3.setIdleTimeout(null).
            setHardTimeout(null);
        assertEquals(VtnUpdateType.CHANGED,
                     tconf3.update(vtnService, tname3, VnodeUpdateMode.MODIFY,
                                   VtnUpdateOperationType.SET));

        // Error tests.

        // Null input.
        checkRpcError(vtnService.updateVtn(null), RpcErrorTag.MISSING_ELEMENT,
                      VtnErrorTag.BADREQUEST);
        checkRpcError(vtnService.removeVtn(null), RpcErrorTag.MISSING_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            input = new UpdateVtnInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vtnService.updateVtn(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        RemoveVtnInput rinput = new RemoveVtnInputBuilder().build();
        checkRpcError(vtnService.removeVtn(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid tenant-name.
        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                input = new UpdateVtnInputBuilder().
                    setTenantName(name).
                    setUpdateMode(mode).
                    build();
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                checkRpcError(vtnService.updateVtn(input), etag, vtag);
            }

            rinput = new RemoveVtnInputBuilder().
                setTenantName(name).build();
            checkRpcError(vtnService.removeVtn(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Modifying VTN that is not present.
        String unknownName = "unknown";
        input = new UpdateVtnInputBuilder().
            setTenantName(unknownName).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vtnService.updateVtn(input), RpcErrorTag.DATA_MISSING,
                      VtnErrorTag.NOTFOUND);

        // Removing VTN that is not present.
        rinput = new RemoveVtnInputBuilder().
            setTenantName(unknownName).
            build();
        checkRpcError(vtnService.removeVtn(rinput), RpcErrorTag.DATA_MISSING,
                      VtnErrorTag.NOTFOUND);

        // Invalid update operation.
        input = new UpdateVtnInputBuilder().
            setTenantName(tname1).
            setOperation(VtnUpdateOperationType.REMOVE).
            build();
        checkRpcError(vtnService.updateVtn(input), RpcErrorTag.BAD_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        // Removing VTN that is not present.
        rinput = new RemoveVtnInputBuilder().
            setTenantName(unknownName).build();
        checkRpcError(vtnService.removeVtn(rinput), RpcErrorTag.DATA_MISSING,
                      VtnErrorTag.NOTFOUND);

        // Name confliction.
        for (String name: vnet.getTenants().keySet()) {
            input = new UpdateVtnInputBuilder().
                setTenantName(name).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vtnService.updateVtn(input), RpcErrorTag.DATA_EXISTS,
                          VtnErrorTag.CONFLICT);
        }

        // Inconsistent flow timeouts.
        input = new UpdateVtnInputBuilder().
            setTenantName(unknownName).
            setHardTimeout(VTenantConfig.DEFAULT_IDLE_TIMEOUT).
            build();
        checkRpcError(vtnService.updateVtn(input), RpcErrorTag.BAD_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        input = new UpdateVtnInputBuilder().
            setTenantName(unknownName).
            setIdleTimeout(30000).
            setHardTimeout(29999).
            build();
        checkRpcError(vtnService.updateVtn(input), RpcErrorTag.BAD_ELEMENT,
                      VtnErrorTag.BADREQUEST);

        for (Entry<String, VTenantConfig> entry:
                 vnet.getTenants().entrySet()) {
            String name = entry.getKey();
            VTenantConfig tconf = entry.getValue().complete();
            Integer idle = tconf.getIdleTimeout();
            Integer hard = tconf.getHardTimeout();
            for (VnodeUpdateMode mode: modifyModes) {
                if (hard.intValue() != 0) {
                    input = new UpdateVtnInputBuilder().
                        setTenantName(name).
                        setUpdateMode(mode).
                        setIdleTimeout(65535).
                        build();
                    checkRpcError(vtnService.updateVtn(input),
                                  RpcErrorTag.BAD_ELEMENT,
                                  VtnErrorTag.BADREQUEST);
                }
                if (idle.intValue() > 1) {
                    input = new UpdateVtnInputBuilder().
                        setTenantName(name).
                        setUpdateMode(mode).
                        setOperation(VtnUpdateOperationType.ADD).
                        setHardTimeout(2).
                        build();
                    checkRpcError(vtnService.updateVtn(input),
                                  RpcErrorTag.BAD_ELEMENT,
                                  VtnErrorTag.BADREQUEST);
                }

                input = new UpdateVtnInputBuilder().
                    setTenantName(name).
                    setUpdateMode(mode).
                    setOperation(VtnUpdateOperationType.SET).
                    setIdleTimeout(2).
                    setHardTimeout(1).
                    build();
                checkRpcError(vtnService.updateVtn(input),
                              RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
            }
        }

        // Errors should never affect existing VTNs.
        vnet.verify();

        // Remove VTNs.
        String[] tenants = {tname2, tname1, tname3};
        for (String tname: tenants) {
            removeVtn(vtnService, tname);
            vnet.removeTenant(tname).verify();
        }
    }

    /**
     * Test case for {@link VtnVbridgeService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVbridgeService() throws Exception {
        LOG.info("Running testVbridgeService().");

        // Create 2 VTNs.
        VirtualNetwork vnet = new VirtualNetwork(this);
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        VTenantConfig tconf1 = new VTenantConfig();
        vnet.addTenant(tname1, tconf1).
            addTenant(tname2, new VTenantConfig()).
            apply();

        // Create 3 vBridges in vtn_1.
        Map<VBridgeIdentifier, VBridgeConfig> bridges = new HashMap<>();
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, "vbr_1");
        VBridgeConfig bconf1 = new VBridgeConfig();
        assertEquals(null, bridges.put(vbrId1, bconf1));
        tconf1.addBridge(vbrId1.getBridgeNameString(), bconf1);
        assertEquals(VtnUpdateType.CREATED,
                     bconf1.update(vbridgeService, vbrId1, null, null));
        vnet.verify();

        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, "vbr_2");
        VBridgeConfig bconf2 = new VBridgeConfig("vBridge 2", 10);
        assertEquals(null, bridges.put(vbrId2, bconf2));
        tconf1.addBridge(vbrId2.getBridgeNameString(), bconf2);
        assertEquals(VtnUpdateType.CREATED,
                     bconf2.update(vbridgeService, vbrId2,
                                   VnodeUpdateMode.CREATE, null));
        vnet.verify();

        VBridgeIdentifier vbrId3 =
            new VBridgeIdentifier(tname1, "0123456789012345678901234567890");
        VBridgeConfig bconf3 = new VBridgeConfig("Test vBridge 3", 1000000);
        assertEquals(null, bridges.put(vbrId3, bconf3));
        tconf1.addBridge(vbrId3.getBridgeNameString(), bconf3);
        assertEquals(VtnUpdateType.CREATED,
                     bconf3.update(vbridgeService, vbrId3,
                                   VnodeUpdateMode.UPDATE, null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(77777777L);
        bconf1.add(rand).
            addVlanMap(new VTNVlanMapConfig());
        bconf2.add(rand).
            addVlanMap(new VTNVlanMapConfig(12));
        VTNMacMapConfig macMap = new VTNMacMapConfig().
            addAllowed(new MacVlan(MacVlan.UNDEFINED, 0),
                       new MacVlan(0x1234567L, 32));
        bconf3.add(rand).
            addVlanMap(new VTNVlanMapConfig(ID_OPENFLOW + "123", 4095)).
            setMacMap(macMap);

        vnet.apply().verify();

        // Try to update with the same parameter.
        VnodeUpdateMode[] modifyModes = {
            null,
            VnodeUpdateMode.UPDATE,
            VnodeUpdateMode.MODIFY,
        };
        VtnUpdateOperationType[] modifyOperations = {
            null,
            VtnUpdateOperationType.SET,
            VtnUpdateOperationType.ADD,
        };
        for (Entry<VBridgeIdentifier, VBridgeConfig> entry:
                 bridges.entrySet()) {
            VBridgeIdentifier vbrId = entry.getKey();
            VBridgeConfig bconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null,
                                 bconf.update(vbridgeService, vbrId, mode,
                                              op));
                }
            }
        }

        // Change parameters using ADD operation.
        bconf1.setAgeInterval(123456);
        UpdateVbridgeInput input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId1.getTenantNameString()).
            setBridgeName(vbrId1.getBridgeNameString()).
            setAgeInterval(bconf1.getAgeInterval()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vbridgeService.updateVbridge(input)));
        vnet.verify();

        bconf2.setDescription("Virtual bridge 2");
        input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId2.getTenantNameString()).
            setBridgeName(vbrId2.getBridgeNameString()).
            setDescription(bconf2.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vbridgeService.updateVbridge(input)));
        vnet.verify();

        bconf3.setDescription("Virtual bridge 3").
            setAgeInterval(9876);
        input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId3.getTenantNameString()).
            setBridgeName(vbrId3.getBridgeNameString()).
            setDescription(bconf3.getDescription()).
            setAgeInterval(bconf3.getAgeInterval()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vbridgeService.updateVbridge(input)));
        vnet.verify();

        // Change parameters using SET operation.
        bconf1.setDescription("Test vBridge 1 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     bconf1.update(vbridgeService, vbrId1, null,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        bconf2.setDescription(null).
            setAgeInterval(314159);
        assertEquals(VtnUpdateType.CHANGED,
                     bconf2.update(vbridgeService, vbrId2,
                                   VnodeUpdateMode.UPDATE,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        bconf3.setDescription(null).
            setAgeInterval(null);
        assertEquals(VtnUpdateType.CHANGED,
                     bconf3.update(vbridgeService, vbrId3,
                                   VnodeUpdateMode.MODIFY,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        // Error tests.

        // Null input.
        checkRpcError(vbridgeService.updateVbridge(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vbridgeService.removeVbridge(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            // Null tenant-name.
            input = new UpdateVbridgeInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vbridgeService.updateVbridge(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVbridgeInputBuilder().
                setBridgeName(vbrId1.getBridgeNameString()).
                setUpdateMode(mode).
                build();
            checkRpcError(vbridgeService.updateVbridge(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // Null bridge-name.
            input = new UpdateVbridgeInputBuilder().
                setTenantName(tname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vbridgeService.updateVbridge(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Null tenant-name.
        RemoveVbridgeInput rinput = new RemoveVbridgeInputBuilder().build();
        checkRpcError(vbridgeService.removeVbridge(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVbridgeInputBuilder().
            setBridgeName(vbrId1.getBridgeNameString()).
            build();
        checkRpcError(vbridgeService.removeVbridge(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        rinput = new RemoveVbridgeInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vbridgeService.removeVbridge(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                // Invalid tenant-name.
                input = new UpdateVbridgeInputBuilder().
                    setTenantName(name).
                    setBridgeName(vbrId1.getBridgeNameString()).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vbridgeService.updateVbridge(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid bridge-name.
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                input = new UpdateVbridgeInputBuilder().
                    setTenantName(tname1).
                    setBridgeName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vbridgeService.updateVbridge(input), etag, vtag);
            }

            // Invalid tenant-name.
            rinput = new RemoveVbridgeInputBuilder().
                setTenantName(name).
                setBridgeName(vbrId1.getBridgeNameString()).
                build();
            checkRpcError(vbridgeService.removeVbridge(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            rinput = new RemoveVbridgeInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vbridgeService.removeVbridge(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Modifying vBridge that is not present.
        String unknownName = "unknown";
        input = new UpdateVbridgeInputBuilder().
            setTenantName(unknownName).
            setBridgeName(vbrId1.getBridgeNameString()).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vbridgeService.updateVbridge(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new UpdateVbridgeInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vbridgeService.updateVbridge(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Removing vBridge that is not present.
        rinput = new RemoveVbridgeInputBuilder().
            setTenantName(unknownName).
            setBridgeName(vbrId1.getBridgeNameString()).
            build();
        checkRpcError(vbridgeService.removeVbridge(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVbridgeInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vbridgeService.removeVbridge(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Invalid update operation.
        input = new UpdateVbridgeInputBuilder().
            setTenantName(vbrId1.getTenantNameString()).
            setBridgeName(vbrId1.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.REMOVE).
            build();
        checkRpcError(vbridgeService.updateVbridge(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Removing vBridge that is not present.
        String[] tnames = {unknownName, tname2};
        String[] bnames = {
            unknownName,
            vbrId1.getBridgeNameString(),
            vbrId2.getBridgeNameString(),
            vbrId3.getBridgeNameString(),
        };
        for (String tname: tnames) {
            for (String bname: bnames) {
                rinput = new RemoveVbridgeInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    build();
                checkRpcError(vbridgeService.removeVbridge(rinput),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
            }
        }

        // Name confliction.
        for (VBridgeIdentifier vbrId: bridges.keySet()) {
            input = new UpdateVbridgeInputBuilder().
                setTenantName(vbrId.getTenantNameString()).
                setBridgeName(vbrId.getBridgeNameString()).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vbridgeService.updateVbridge(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
        }

        // Errors should never affect existing vBridges.
        vnet.verify();

        // Remove vbrId1 and vbrId3.
        assertEquals(bconf2, bridges.remove(vbrId2));
        for (VBridgeIdentifier vbrId: bridges.keySet()) {
            removeVbridge(vbridgeService, vbrId);
            tconf1.removeBridge(vbrId.getBridgeNameString());
            vnet.verify();
        }

        // Remove VTNs.
        Set<String> nameSet = new HashSet<>(vnet.getTenants().keySet());
        for (String tname: nameSet) {
            removeVtn(vtnService, tname);
            vnet.removeTenant(tname).verify();
        }
    }

    /**
     * Test case for {@link VtnVterminalService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVterminalService() throws Exception {
        LOG.info("Running testVbridgeService().");

        // Create 2 VTNs.
        VirtualNetwork vnet = new VirtualNetwork(this);
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        VTenantConfig tconf1 = new VTenantConfig();
        vnet.addTenant(tname1, tconf1).
            addTenant(tname2, new VTenantConfig());
        vnet.apply();

        // Create 3 vTerminals in vtn_1.
        Map<VTerminalIdentifier, VTerminalConfig> terminals = new HashMap<>();
        VTerminalIdentifier vtermId1 =
            new VTerminalIdentifier(tname1, "vterm_1");
        VTerminalConfig vtconf1 = new VTerminalConfig();
        assertEquals(null, terminals.put(vtermId1, vtconf1));
        tconf1.addTerminal(vtermId1.getBridgeNameString(), vtconf1);
        assertEquals(VtnUpdateType.CREATED,
                     vtconf1.update(vterminalService, vtermId1, null, null));
        vnet.verify();

        VTerminalIdentifier vtermId2 =
            new VTerminalIdentifier(tname1, "vterm_2");
        VTerminalConfig vtconf2 = new VTerminalConfig("vTerminal 2");
        assertEquals(null, terminals.put(vtermId2, vtconf2));
        tconf1.addTerminal(vtermId2.getBridgeNameString(), vtconf2);
        assertEquals(VtnUpdateType.CREATED,
                     vtconf2.update(vterminalService, vtermId2,
                                    VnodeUpdateMode.CREATE, null));
        vnet.verify();

        VTerminalIdentifier vtermId3 =
            new VTerminalIdentifier(tname1, "1234567890123456789012345678901");
        VTerminalConfig vtconf3 = new VTerminalConfig("Test vTerminal 3");
        assertEquals(null, terminals.put(vtermId3, vtconf3));
        tconf1.addTerminal(vtermId3.getBridgeNameString(), vtconf3);
        assertEquals(VtnUpdateType.CREATED,
                     vtconf3.update(vterminalService, vtermId3,
                                    VnodeUpdateMode.UPDATE, null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(0xabcdef123L);
        for (VTerminalConfig vtconf: terminals.values()) {
            vtconf.add(rand);
        }

        vnet.apply().verify();

        // Try to update with the same parameter.
        VnodeUpdateMode[] modifyModes = {
            null,
            VnodeUpdateMode.UPDATE,
            VnodeUpdateMode.MODIFY,
        };
        VtnUpdateOperationType[] modifyOperations = {
            null,
            VtnUpdateOperationType.SET,
            VtnUpdateOperationType.ADD,
        };
        for (Entry<VTerminalIdentifier, VTerminalConfig> entry:
                 terminals.entrySet()) {
            VTerminalIdentifier vtermId = entry.getKey();
            VTerminalConfig vtconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null,
                                 vtconf.update(vterminalService, vtermId, mode,
                                               op));
                }
            }
        }

        // Change parameters using ADD operation.
        vtconf1.setDescription("Virtual terminal 1");
        UpdateVterminalInput input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId1.getTenantNameString()).
            setTerminalName(vtermId1.getBridgeNameString()).
            setDescription(vtconf1.getDescription()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vterminalService.updateVterminal(input)));
        vnet.verify();

        vtconf2.setDescription("Virtual terminal 2");
        input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId2.getTenantNameString()).
            setTerminalName(vtermId2.getBridgeNameString()).
            setDescription(vtconf2.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vterminalService.updateVterminal(input)));
        vnet.verify();

        vtconf3.setDescription("vTerminal 3");
        input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId3.getTenantNameString()).
            setTerminalName(vtermId3.getBridgeNameString()).
            setDescription(vtconf3.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vterminalService.updateVterminal(input)));
        vnet.verify();

        // Change parameters using SET operation.
        vtconf1.setDescription("vTerminal 1 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     vtconf1.update(vterminalService, vtermId1, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        vtconf2.setDescription("vTerminal 2 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     vtconf2.update(vterminalService, vtermId2,
                                    VnodeUpdateMode.UPDATE,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        vtconf3.setDescription("vTerminal 3 (SET)");
        assertEquals(VtnUpdateType.CHANGED,
                     vtconf3.update(vterminalService, vtermId3,
                                    VnodeUpdateMode.MODIFY,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        // Error tests.

        // Null input.
        checkRpcError(vterminalService.updateVterminal(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vterminalService.removeVterminal(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            // Null tenant-name.
            input = new UpdateVterminalInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vterminalService.updateVterminal(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVterminalInputBuilder().
                setTerminalName(vtermId1.getBridgeNameString()).
                setUpdateMode(mode).
                build();
            checkRpcError(vterminalService.updateVterminal(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // Null terminal-name.
            input = new UpdateVterminalInputBuilder().
                setTenantName(tname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vterminalService.updateVterminal(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Null tenant-name.
        RemoveVterminalInput rinput = new RemoveVterminalInputBuilder().
            build();
        checkRpcError(vterminalService.removeVterminal(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVterminalInputBuilder().
            setTerminalName(vtermId1.getBridgeNameString()).
            build();
        checkRpcError(vterminalService.removeVterminal(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null terminal-name.
        rinput = new RemoveVterminalInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vterminalService.removeVterminal(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                // Invalid tenant-name.
                input = new UpdateVterminalInputBuilder().
                    setTenantName(name).
                    setTerminalName(vtermId1.getBridgeNameString()).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vterminalService.updateVterminal(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid terminal-name.
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                input = new UpdateVterminalInputBuilder().
                    setTenantName(tname1).
                    setTerminalName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vterminalService.updateVterminal(input),
                              etag, vtag);
            }

            // Invalid tenant-name.
            rinput = new RemoveVterminalInputBuilder().
                setTenantName(name).
                setTerminalName(vtermId1.getBridgeNameString()).
                build();
            checkRpcError(vterminalService.removeVterminal(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid terminal-name.
            rinput = new RemoveVterminalInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                build();
            checkRpcError(vterminalService.removeVterminal(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Modifying vTerminal that is not present.
        String unknownName = "unknown";
        input = new UpdateVterminalInputBuilder().
            setTenantName(unknownName).
            setTerminalName(vtermId1.getBridgeNameString()).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vterminalService.updateVterminal(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new UpdateVterminalInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        checkRpcError(vterminalService.updateVterminal(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Removing vTerminal that is not present.
        rinput = new RemoveVterminalInputBuilder().
            setTenantName(unknownName).
            setTerminalName(vtermId1.getBridgeNameString()).
            build();
        checkRpcError(vterminalService.removeVterminal(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVterminalInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            build();
        checkRpcError(vterminalService.removeVterminal(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Invalid update operation.
        input = new UpdateVterminalInputBuilder().
            setTenantName(vtermId1.getTenantNameString()).
            setTerminalName(vtermId1.getBridgeNameString()).
            setOperation(VtnUpdateOperationType.REMOVE).
            build();
        checkRpcError(vterminalService.updateVterminal(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Removing vTerminal that is not present.
        String[] tnames = {unknownName, tname2};
        String[] bnames = {
            unknownName,
            vtermId1.getBridgeNameString(),
            vtermId2.getBridgeNameString(),
            vtermId3.getBridgeNameString(),
        };
        for (String tname: tnames) {
            for (String bname: bnames) {
                rinput = new RemoveVterminalInputBuilder().
                    setTenantName(tname).
                    setTerminalName(bname).
                    build();
                checkRpcError(vterminalService.removeVterminal(rinput),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
            }
        }

        // Name confliction.
        for (VTerminalIdentifier vtermId: terminals.keySet()) {
            input = new UpdateVterminalInputBuilder().
                setTenantName(vtermId.getTenantNameString()).
                setTerminalName(vtermId.getBridgeNameString()).
                setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vterminalService.updateVterminal(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
        }

        // Errors should never affect existing vTerminals.
        vnet.verify();

        // Remove vtermId2 and vtermId3.
        assertEquals(vtconf1, terminals.remove(vtermId1));
        for (VTerminalIdentifier vtermId: terminals.keySet()) {
            removeVterminal(vterminalService, vtermId);
            tconf1.removeTerminal(vtermId.getBridgeNameString());
            vnet.verify();
        }

        // Remove VTNs.
        Set<String> nameSet = new HashSet<>(vnet.getTenants().keySet());
        for (String tname: nameSet) {
            removeVtn(vtnService, tname);
            vnet.removeTenant(tname).verify();
        }
    }

    /**
     * Test case for {@link VtnVinterfaceService}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVinterfaceService() throws Exception {
        LOG.info("Running testVinterfaceService().");

        // Create 2 VTNs, 2 vBridges, and 3 vTerminals.
        VirtualNetwork vnet = new VirtualNetwork(this);
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname2, bname1);
        VTerminalIdentifier vtermId1 = new VTerminalIdentifier(tname1, bname1);
        VTerminalIdentifier vtermId2 = new VTerminalIdentifier(tname2, bname1);
        VTerminalIdentifier vtermId3 = new VTerminalIdentifier(tname2, bname2);
        vnet.addBridge(vbrId1, vbrId2).
            addTerminal(vtermId1, vtermId2, vtermId3).
            apply();

        // Create 3 virtual interfaces in vbrId1.
        Map<VInterfaceIdentifier<?>, VInterfaceConfig> interfaces =
            new HashMap<>();
        Map<VBridgeIfIdentifier, VInterfaceConfig> bridgeIfs = new HashMap<>();
        String iname1 = "if_1";
        String iname2 = "1234567890123456789012345678901";
        String iname3 = "if_3";
        VBridgeConfig bconf1 = vnet.getBridge(vbrId1);
        VBridgeIfIdentifier bifId1 = vbrId1.childInterface(iname1);
        VInterfaceConfig biconf1 = new VInterfaceConfig();
        assertEquals(null, interfaces.put(bifId1, biconf1));
        assertEquals(null, bridgeIfs.put(bifId1, biconf1));
        bconf1.addInterface(bifId1.getInterfaceNameString(), biconf1);
        assertEquals(VtnUpdateType.CREATED,
                     biconf1.update(vinterfaceService, bifId1, null, null));
        vnet.verify();

        VBridgeIfIdentifier bifId2 = vbrId1.childInterface(iname2);
        VInterfaceConfig biconf2 = new VInterfaceConfig("vBridge-IF 2", false);
        assertEquals(null, interfaces.put(bifId2, biconf2));
        assertEquals(null, bridgeIfs.put(bifId2, biconf2));
        bconf1.addInterface(bifId2.getInterfaceNameString(), biconf2);
        assertEquals(VtnUpdateType.CREATED,
                     biconf2.update(vinterfaceService, bifId2,
                                    VnodeUpdateMode.CREATE, null));
        vnet.verify();

        VBridgeIfIdentifier bifId3 = vbrId1.childInterface(iname3);
        VInterfaceConfig biconf3 = new VInterfaceConfig("vBridge-IF 3", true);
        assertEquals(null, interfaces.put(bifId3, biconf3));
        assertEquals(null, bridgeIfs.put(bifId3, biconf3));
        bconf1.addInterface(bifId3.getInterfaceNameString(), biconf3);
        assertEquals(VtnUpdateType.CREATED,
                     biconf3.update(vinterfaceService, bifId3,
                                    VnodeUpdateMode.UPDATE, null));
        vnet.verify();

        // Create 1 virtual interfaces per vTerminal.
        Map<VTerminalIfIdentifier, VInterfaceConfig> termIfs = new HashMap<>();
        VTerminalConfig vtconf1 = vnet.getTerminal(vtermId1);
        VTerminalIfIdentifier tifId1 = vtermId1.childInterface(iname1);
        VInterfaceConfig ticonf1 = new VInterfaceConfig();
        assertEquals(null, interfaces.put(tifId1, ticonf1));
        assertEquals(null, termIfs.put(tifId1, ticonf1));
        vtconf1.addInterface(tifId1.getInterfaceNameString(), ticonf1);
        assertEquals(VtnUpdateType.CREATED,
                     ticonf1.update(vinterfaceService, tifId1, null, null));
        vnet.verify();

        VTerminalConfig vtconf2 = vnet.getTerminal(vtermId2);
        VTerminalIfIdentifier tifId2 = vtermId2.childInterface(iname2);
        VInterfaceConfig ticonf2 = new VInterfaceConfig(null, true);
        assertEquals(null, interfaces.put(tifId2, ticonf2));
        assertEquals(null, termIfs.put(tifId2, ticonf2));
        vtconf2.addInterface(tifId2.getInterfaceNameString(), ticonf2);
        assertEquals(VtnUpdateType.CREATED,
                     ticonf2.update(vinterfaceService, tifId2,
                                    VnodeUpdateMode.CREATE, null));
        vnet.verify();

        VTerminalConfig vtconf3 = vnet.getTerminal(vtermId3);
        VTerminalIfIdentifier tifId3 = vtermId3.childInterface(iname3);
        VInterfaceConfig ticonf3 =
            new VInterfaceConfig("vTerminal-IF 3", false);
        assertEquals(null, interfaces.put(tifId3, ticonf3));
        assertEquals(null, termIfs.put(tifId3, ticonf3));
        vtconf3.addInterface(tifId3.getInterfaceNameString(), ticonf3);
        assertEquals(VtnUpdateType.CREATED,
                     ticonf3.update(vinterfaceService, tifId3,
                                    VnodeUpdateMode.UPDATE, null));
        vnet.verify();

        // Add random configuration.
        Random rand = new Random(271828L);
        bconf1.add(rand);
        vnet.getBridge(vbrId2).add(rand);
        for (VInterfaceConfig iconf: interfaces.values()) {
            iconf.add(rand);
        }

        vnet.apply().verify();

        // Try to update with the same parameter.
        VnodeUpdateMode[] modifyModes = {
            null,
            VnodeUpdateMode.UPDATE,
            VnodeUpdateMode.MODIFY,
        };
        VtnUpdateOperationType[] modifyOperations = {
            null,
            VtnUpdateOperationType.SET,
            VtnUpdateOperationType.ADD,
        };
        for (Entry<VInterfaceIdentifier<?>, VInterfaceConfig> entry:
                 interfaces.entrySet()) {
            VInterfaceIdentifier<?> ifId = entry.getKey();
            VInterfaceConfig iconf = entry.getValue();
            for (VnodeUpdateMode mode: modifyModes) {
                for (VtnUpdateOperationType op: modifyOperations) {
                    assertEquals(null,
                                 iconf.update(vinterfaceService, ifId, mode,
                                              op));
                }
            }
        }

        // Change parameters using ADD operation.
        // terminal-name should be ignored if bridge-name is specified.
        String unknownName = "unknown";
        biconf1.setDescription("vBridge interface 1");
        UpdateVinterfaceInput input = new UpdateVinterfaceInputBuilder().
            setTenantName(bifId1.getTenantNameString()).
            setBridgeName(bifId1.getBridgeNameString()).
            setTerminalName(unknownName).
            setInterfaceName(bifId1.getInterfaceNameString()).
            setDescription(biconf1.getDescription()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vinterfaceService.updateVinterface(input)));
        vnet.verify();

        biconf2.setEnabled(true);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(bifId2.getTenantNameString()).
            setBridgeName(bifId2.getBridgeNameString()).
            setTerminalName(unknownName).
            setInterfaceName(bifId2.getInterfaceNameString()).
            setEnabled(biconf2.isEnabled()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vinterfaceService.updateVinterface(input)));
        vnet.verify();

        biconf3.setDescription("vBridge interface 3").
            setEnabled(false);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(bifId3.getTenantNameString()).
            setBridgeName(bifId3.getBridgeNameString()).
            setTerminalName(unknownName).
            setInterfaceName(bifId3.getInterfaceNameString()).
            setDescription(biconf3.getDescription()).
            setEnabled(biconf3.isEnabled()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vinterfaceService.updateVinterface(input)));
        vnet.verify();

        ticonf1.setDescription("vTerminal interface 1");
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(tifId1.getTenantNameString()).
            setTerminalName(tifId1.getBridgeNameString()).
            setInterfaceName(tifId1.getInterfaceNameString()).
            setDescription(ticonf1.getDescription()).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vinterfaceService.updateVinterface(input)));
        vnet.verify();

        ticonf2.setEnabled(false);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(tifId2.getTenantNameString()).
            setTerminalName(tifId2.getBridgeNameString()).
            setInterfaceName(tifId2.getInterfaceNameString()).
            setEnabled(ticonf2.isEnabled()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vinterfaceService.updateVinterface(input)));
        vnet.verify();

        ticonf3.setDescription("vTerminal interface 3").
            setEnabled(true);
        input = new UpdateVinterfaceInputBuilder().
            setTenantName(tifId3.getTenantNameString()).
            setTerminalName(tifId3.getBridgeNameString()).
            setInterfaceName(tifId3.getInterfaceNameString()).
            setDescription(ticonf3.getDescription()).
            setEnabled(ticonf3.isEnabled()).
            setOperation(VtnUpdateOperationType.ADD).
            setUpdateMode(VnodeUpdateMode.MODIFY).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(vinterfaceService.updateVinterface(input)));
        vnet.verify();

        // Change parameters using SET operation.
        biconf1.setDescription("vBridge interface 1 (SET)").
            setEnabled(false);
        assertEquals(VtnUpdateType.CHANGED,
                     biconf1.update(vinterfaceService, bifId1, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        biconf2.setDescription("vBridge interface 2 (SET)").
            setEnabled(null);
        assertEquals(VtnUpdateType.CHANGED,
                     biconf2.update(vinterfaceService, bifId2, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        biconf3.setDescription(null).
            setEnabled(null);
        assertEquals(VtnUpdateType.CHANGED,
                     biconf3.update(vinterfaceService, bifId3, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        ticonf1.setEnabled(false);
        assertEquals(VtnUpdateType.CHANGED,
                     ticonf1.update(vinterfaceService, tifId1, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        ticonf2.setDescription(null).
            setEnabled(null);
        assertEquals(VtnUpdateType.CHANGED,
                     ticonf2.update(vinterfaceService, tifId2, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        ticonf3.setDescription("vTerminal interface 3 (SET)").
            setEnabled(false);
        assertEquals(VtnUpdateType.CHANGED,
                     ticonf3.update(vinterfaceService, tifId3, null,
                                    VtnUpdateOperationType.SET));
        vnet.verify();

        // Error tests.

        // Null input.
        checkRpcError(vinterfaceService.updateVinterface(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vinterfaceService.removeVinterface(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
            // Null tenant-name.
            input = new UpdateVinterfaceInputBuilder().
                setUpdateMode(mode).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVinterfaceInputBuilder().
                setBridgeName(bname1).
                setInterfaceName(iname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVinterfaceInputBuilder().
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // No bridge name.
            input = new UpdateVinterfaceInputBuilder().
                setTenantName(tname1).
                setInterfaceName(iname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            // Null interface-name.
            input = new UpdateVinterfaceInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new UpdateVinterfaceInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setUpdateMode(mode).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Null tenant-name.
        RemoveVinterfaceInput rinput = new RemoveVinterfaceInputBuilder().
            build();
        checkRpcError(vinterfaceService.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVinterfaceInputBuilder().
            setBridgeName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(vinterfaceService.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVinterfaceInputBuilder().
            setTerminalName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(vinterfaceService.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No bridge name.
        rinput = new RemoveVinterfaceInputBuilder().
            setTenantName(tname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(vinterfaceService.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null interface-name.
        rinput = new RemoveVinterfaceInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            build();
        checkRpcError(vinterfaceService.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVinterfaceInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            build();
        checkRpcError(vinterfaceService.removeVinterface(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                // Invalid tenant-name.
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(name).
                    setBridgeName(bname1).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(name).
                    setTerminalName(bname1).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid bridge-name.
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setBridgeName(name).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid terminal-name.
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setTerminalName(name).
                    setInterfaceName(iname1).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

                // Invalid interface-name.
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.BAD_ELEMENT;
                    vtag = VtnErrorTag.BADREQUEST;
                }
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setBridgeName(bname1).
                    setInterfaceName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              etag, vtag);

                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(tname1).
                    setTerminalName(bname1).
                    setInterfaceName(name).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              etag, vtag);
            }

            // Invalid tenant-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(name).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid terminal-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid interface-name.
            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVinterfaceInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        for (VInterfaceIdentifier<?> ifId: interfaces.keySet()) {
            String tname = ifId.getTenantNameString();
            VirtualNodePath vpath = ifId.getVirtualNodePath();

            // Modifying virtual interface that is not present.
            UpdateVinterfaceInputBuilder builder =
                new UpdateVinterfaceInputBuilder();
            builder.fieldsFrom(vpath);
            input = builder.setTenantName(unknownName).
                setUpdateMode(VnodeUpdateMode.MODIFY).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = builder.setTenantName(tname).
                setBridgeName(unknownName).
                setTerminalName(null).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = builder.setBridgeName(null).
                setTerminalName(unknownName).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            builder.fieldsFrom(vpath);
            input = builder.setInterfaceName(unknownName).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Removing virtual interface that is not present.
            RemoveVinterfaceInputBuilder rbuilder =
                new RemoveVinterfaceInputBuilder();
            rbuilder.fieldsFrom(vpath);
            rinput = rbuilder.setTenantName(unknownName).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = rbuilder.setTenantName(tname).
                setBridgeName(unknownName).
                setTerminalName(null).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = rbuilder.setBridgeName(null).
                setTerminalName(unknownName).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rbuilder.fieldsFrom(vpath);
            rinput = rbuilder.setInterfaceName(unknownName).
                build();
            checkRpcError(vinterfaceService.removeVinterface(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Name confliction.
            builder.fieldsFrom(vpath);
            input = builder.setUpdateMode(VnodeUpdateMode.CREATE).
                build();
            checkRpcError(vinterfaceService.updateVinterface(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
        }

        // Adding more than one interfaces to vTerminal.
        for (VTerminalIfIdentifier ifId: termIfs.keySet()) {
            for (VnodeUpdateMode mode: VnodeUpdateMode.values()) {
                RpcErrorTag etag;
                VtnErrorTag vtag;
                if (mode == VnodeUpdateMode.MODIFY) {
                    etag = RpcErrorTag.DATA_MISSING;
                    vtag = VtnErrorTag.NOTFOUND;
                } else {
                    etag = RpcErrorTag.DATA_EXISTS;
                    vtag = VtnErrorTag.CONFLICT;
                }
                input = new UpdateVinterfaceInputBuilder().
                    setTenantName(ifId.getTenantNameString()).
                    setTerminalName(ifId.getBridgeNameString()).
                    setInterfaceName(unknownName).
                    setUpdateMode(mode).
                    build();
                checkRpcError(vinterfaceService.updateVinterface(input),
                              etag, vtag);
            }
        }

        // Errors should never affect existing virtual interfaces.
        vnet.verify();

        // Remove bifId1, bifId3, and tifId2.
        assertEquals(biconf2, interfaces.remove(bifId2));
        assertEquals(ticonf1, interfaces.remove(tifId1));
        assertEquals(ticonf3, interfaces.remove(tifId3));
        for (VInterfaceIdentifier<?> ifId: interfaces.keySet()) {
            removeVinterface(vinterfaceService, ifId);
            vnet.removeInterface(ifId).verify();
        }

        // Remove vbrId1, vtermId1, and vtermId2.
        removeVbridge(vbridgeService, vbrId1);
        vnet.removeBridge(vbrId1).verify();
        removeVterminal(vterminalService, vtermId1);
        vnet.removeTerminal(vtermId1).verify();
        removeVterminal(vterminalService, vtermId2);
        vnet.removeTerminal(vtermId2).verify();

        // Remove VTNs.
        Set<String> nameSet = new HashSet<>(vnet.getTenants().keySet());
        for (String tname: nameSet) {
            removeVtn(vtnService, tname);
            vnet.removeTenant(tname).verify();
        }
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

        // Create 3 vBridges.
        VirtualNetwork vnet = new VirtualNetwork(this);
        String tname1 = "vtn_1";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";
        String bname3 = "bridge_3";

        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, bname2);
        VBridgeIdentifier vbrId3 = new VBridgeIdentifier(tname1, bname3);
        vnet.addBridge(vbrId1, vbrId2, vbrId3).apply();

        // Map VLAN 0 to vbrId1.
        List<VTNVlanMapConfig> vmaps1 = new ArrayList<>();
        VBridgeConfig bconf1 = vnet.getBridge(vbrId1);
        VTNVlanMapConfig vmap11 = new VTNVlanMapConfig();
        vmaps1.add(vmap11);
        bconf1.addVlanMap(vmap11);
        AddVlanMapInput input = vmap11.newInputBuilder(vbrId1).build();
        AddVlanMapOutput output =
            getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap11.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 4095 to vbrId1.
        VTNVlanMapConfig vmap12 = new VTNVlanMapConfig(4095);
        vmaps1.add(vmap12);
        bconf1.addVlanMap(vmap12);
        input = vmap12.newInputBuilder(vbrId1).build();
        output = getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap12.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 10 on node1 (should not be present) to vbrId1.
        String node1 = ID_OPENFLOW + "1";
        VTNVlanMapConfig vmap13 = new VTNVlanMapConfig(node1, 10).
            setActive(false);
        vmaps1.add(vmap13);
        bconf1.addVlanMap(vmap13);
        input = vmap13.newInputBuilder(vbrId1).build();
        output = getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap13.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // Map VLAN 0 on node1 to vbrId2.
        List<VTNVlanMapConfig> vmaps2 = new ArrayList<>();
        VBridgeConfig bconf2 = vnet.getBridge(vbrId2);
        VTNVlanMapConfig vmap21 = new VTNVlanMapConfig(node1, null).
            setActive(false);
        vmaps2.add(vmap21);
        bconf2.addVlanMap(vmap21);
        input = vmap21.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap21.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // Map VLAN 1 to vbrId2.
        VTNVlanMapConfig vmap22 = new VTNVlanMapConfig(1);
        vmaps2.add(vmap22);
        bconf2.addVlanMap(vmap22);
        input = vmap22.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap22.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 30 on node2 (should not be present) to vbrId2.
        String node2 = ID_OPENFLOW + "18446744073709551615";
        VTNVlanMapConfig vmap23 = new VTNVlanMapConfig(node2, 30).
            setActive(false);
        vmaps2.add(vmap23);
        bconf2.addVlanMap(vmap23);
        input = vmap23.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap23.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // get-vlan-id test.
        VBridgeIdentifier[] vbrIds = {vbrId2, vbrId3};
        for (VTNVlanMapConfig vmap: vmaps1) {
            GetVlanMapInput ginput = vmap.newGetInputBuilder(vbrId1).build();
            GetVlanMapOutput goutput =
                getRpcOutput(vlanMapService.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig copy = vmap.complete();
            ginput = copy.newGetInputBuilder(vbrId1).build();
            goutput = getRpcOutput(vlanMapService.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig[] vmaps = {vmap, copy};
            for (VBridgeIdentifier vbrId: vbrIds) {
                for (VTNVlanMapConfig vmc: vmaps) {
                    ginput = vmc.newGetInputBuilder(vbrId).build();
                    goutput = getRpcOutput(vlanMapService.getVlanMap(ginput));
                    assertEquals(null, goutput.getMapId());
                    assertEquals(null, goutput.isActive());
                }
            }
        }

        vbrIds = new VBridgeIdentifier[]{vbrId1, vbrId3};
        for (VTNVlanMapConfig vmap: vmaps2) {
            GetVlanMapInput ginput = vmap.newGetInputBuilder(vbrId2).build();
            GetVlanMapOutput goutput =
                getRpcOutput(vlanMapService.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig copy = vmap.complete();
            ginput = copy.newGetInputBuilder(vbrId2).build();
            goutput = getRpcOutput(vlanMapService.getVlanMap(ginput));
            assertEquals(vmap.getMapId(), goutput.getMapId());
            assertEquals(active, goutput.isActive());

            VTNVlanMapConfig[] vmaps = {vmap, copy};
            for (VBridgeIdentifier vbrId: vbrIds) {
                for (VTNVlanMapConfig vmc: vmaps) {
                    ginput = vmc.newGetInputBuilder(vbrId).build();
                    goutput = getRpcOutput(vlanMapService.getVlanMap(ginput));
                    assertEquals(null, goutput.getMapId());
                    assertEquals(null, goutput.isActive());
                }
            }
        }

        // Error tests.

        // Null input.
        checkRpcError(vlanMapService.addVlanMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vlanMapService.removeVlanMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(vlanMapService.getVlanMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        input = new AddVlanMapInputBuilder().build();
        checkRpcError(vlanMapService.addVlanMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new AddVlanMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(vlanMapService.addVlanMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        RemoveVlanMapInput rinput = new RemoveVlanMapInputBuilder().build();
        checkRpcError(vlanMapService.removeVlanMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVlanMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(vlanMapService.removeVlanMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        GetVlanMapInput ginput = new GetVlanMapInputBuilder().build();
        checkRpcError(vlanMapService.getVlanMap(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        ginput = new GetVlanMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(vlanMapService.getVlanMap(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        input = new AddVlanMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vlanMapService.addVlanMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemoveVlanMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vlanMapService.removeVlanMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        ginput = new GetVlanMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(vlanMapService.getVlanMap(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            input = new AddVlanMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(vlanMapService.addVlanMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVlanMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(vlanMapService.removeVlanMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            ginput = new GetVlanMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(vlanMapService.getVlanMap(ginput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            input = new AddVlanMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vlanMapService.addVlanMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemoveVlanMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vlanMapService.removeVlanMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            ginput = new GetVlanMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                build();
            checkRpcError(vlanMapService.getVlanMap(ginput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Specifying vBridge that is not present.
        String unknownName = "unknown";
        input = new AddVlanMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            build();
        checkRpcError(vlanMapService.addVlanMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new AddVlanMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vlanMapService.addVlanMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVlanMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            build();
        checkRpcError(vlanMapService.removeVlanMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemoveVlanMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vlanMapService.removeVlanMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        ginput = new GetVlanMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            build();
        checkRpcError(vlanMapService.getVlanMap(ginput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        ginput = new GetVlanMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            build();
        checkRpcError(vlanMapService.getVlanMap(ginput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Invalid node ID.
        for (String node: INVALID_NODE_IDS) {
            VTNVlanMapConfig vmap = new VTNVlanMapConfig(node, 0);
            input = vmap.newInputBuilder(vbrId1).build();
            checkRpcError(vlanMapService.addVlanMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            ginput = vmap.newGetInputBuilder(vbrId1).build();
            GetVlanMapOutput goutput =
                getRpcOutput(vlanMapService.getVlanMap(ginput));
            assertEquals(null, goutput.getMapId());
            assertEquals(null, goutput.isActive());
        }

        // Already mapped.
        List<VTNVlanMapConfig> allMaps = new ArrayList<>(vmaps1);
        allMaps.addAll(vmaps2);
        vbrIds = new VBridgeIdentifier[]{vbrId1, vbrId2, vbrId3};
        for (VBridgeIdentifier vbrId: vbrIds) {
            for (VTNVlanMapConfig vmap: allMaps) {
                input = vmap.newInputBuilder(vbrId).build();
                checkRpcError(vlanMapService.addVlanMap(input),
                              RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
            }
        }

        // Errors should never affect existing VLAN mappings.
        vnet.verify();

        // Remove all VLAN mappings.
        for (VTNVlanMapConfig vmap: vmaps1) {
            String mapId = vmap.getMapId();
            Map<String, VtnUpdateType> expected = new HashMap<>();
            expected.put(mapId, VtnUpdateType.REMOVED);
            List<String> mapIds = Collections.singletonList(mapId);
            Map<String, VtnUpdateType> result =
                removeVlanMap(vlanMapService, vbrId1, mapIds);
            assertEquals(expected, result);
            bconf1.removeVlanMap(mapId);
            vnet.verify();
        }

        Map<String, VtnUpdateType> expected = new HashMap<>();
        List<String> mapIds = new ArrayList<>();
        for (VTNVlanMapConfig vmap: vmaps1) {
            String mapId = vmap.getMapId();
            expected.put(mapId, null);
            mapIds.add(mapId);
        }

        for (VTNVlanMapConfig vmap: vmaps2) {
            String mapId = vmap.getMapId();
            expected.put(mapId, VtnUpdateType.REMOVED);
            mapIds.add(mapId);
        }

        Map<String, VtnUpdateType> result =
            removeVlanMap(vlanMapService, vbrId2, mapIds);
        assertEquals(expected, result);
        bconf2.clearVlanMap();
        vnet.verify();

        // Restore all the VLAN mappigs into vbrId3.
        VBridgeConfig bconf3 = vnet.getBridge(vbrId3);
        for (VTNVlanMapConfig vmap: allMaps) {
            bconf3.addVlanMap(vmap);
            input = vmap.newInputBuilder(vbrId3).build();
            output = getRpcOutput(vlanMapService.addVlanMap(input));
            assertEquals(vmap.getMapId(), output.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, output.isActive());
            vnet.verify();
        }

        // Remove vbrId3.
        removeVbridge(vbridgeService, vbrId3);
        vnet.removeBridge(vbrId3).verify();

        // Restore all the VLAN mappings into another VTN.
        String tname2 = "vtn_2";
        VBridgeIdentifier vbrId4 = new VBridgeIdentifier(tname2, bname1);
        vnet.addBridge(vbrId4).apply();
        VBridgeConfig bconf4 = vnet.getBridge(vbrId4);
        for (VTNVlanMapConfig vmap: allMaps) {
            bconf4.addVlanMap(vmap);
            input = vmap.newInputBuilder(vbrId4).build();
            output = getRpcOutput(vlanMapService.addVlanMap(input));
            assertEquals(vmap.getMapId(), output.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, output.isActive());
            vnet.verify();
        }

        // Remove vtn_2.
        removeVtn(vtnService, tname2);
        vnet.removeTenant(tname2).verify();

        // Restore all the VLAN mappings into vbrId1.
        for (VTNVlanMapConfig vmap: allMaps) {
            bconf1.addVlanMap(vmap);
            input = vmap.newInputBuilder(vbrId1).build();
            output = getRpcOutput(vlanMapService.addVlanMap(input));
            assertEquals(vmap.getMapId(), output.getMapId());
            Boolean active = Boolean.valueOf(vmap.getNode() == null);
            assertEquals(active, output.isActive());
            vnet.verify();
        }

        // Remove vtn_1.
        removeVtn(vtnService, tname1);
        vnet.removeTenant(tname1).verify();
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

        // Determine nodes that has at least one edge port.
        String edgeNode = null;
        String internalNode = null;
        List<String> edgePorts = null;
        for (String nid: ofMockService.getNodes()) {
            List<String> edges = ofMockService.getPorts(nid, true);
            if (edges.isEmpty()) {
                internalNode = nid;
            } else {
                edgeNode = nid;
                edgePorts = edges;
            }

            if (edgeNode != null && internalNode != null) {
                break;
            }
        }

        assertNotNull(edgeNode);
        assertNotNull(internalNode);

        // Determine one port in internalNode that is not connected to
        // edgeNode.
        String internalPort = null;
        for (String pid: ofMockService.getPorts(internalNode, false)) {
            String peer = ofMockService.getPeerIdentifier(pid);
            if (!internalNode.equals(OfMockUtils.getNodeIdentifier(peer))) {
                internalPort = pid;
            }
        }
        assertNotNull(internalPort);

        // Create 2 vBridges.
        VirtualNetwork vnet = new VirtualNetwork(this);
        String tname1 = "vtn_1";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";

        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname1, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname1, bname2);
        vnet.addBridge(vbrId1, vbrId2).apply();
        VBridgeConfig bconf1 = vnet.getBridge(vbrId1);
        VBridgeConfig bconf2 = vnet.getBridge(vbrId2);

        // Map VLAN 1 on edgeNode to vbrId1.
        VTNVlanMapConfig vmap1 = new VTNVlanMapConfig(edgeNode, 1).
            setActive(Boolean.TRUE);
        bconf1.addVlanMap(vmap1);
        AddVlanMapInput input = vmap1.newInputBuilder(vbrId1).build();
        AddVlanMapOutput output =
            getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap1.getMapId(), output.getMapId());
        assertEquals(Boolean.TRUE, output.isActive());
        vnet.verify();

        // Map VLAN 1 on internalNode to vbrId2.
        VTNVlanMapConfig vmap2 = new VTNVlanMapConfig(internalNode, 1).
            setActive(Boolean.FALSE);
        bconf2.addVlanMap(vmap2);
        input = vmap2.newInputBuilder(vbrId2).build();
        output = getRpcOutput(vlanMapService.addVlanMap(input));
        assertEquals(vmap2.getMapId(), output.getMapId());
        assertEquals(Boolean.FALSE, output.isActive());
        vnet.verify();

        // Down all the edge ports in edgeNode.
        // Then the state of vbrId1 should be changed to DOWN.
        for (String pid: edgePorts) {
            assertEquals(true, ofMockService.setPortState(pid, false, false));
        }

        VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
            set(vbrId1, VnodeState.DOWN).
            await();
        vmap1.setActive(Boolean.FALSE);
        vnet.verify();

        // Remove inter-switch link on internalPort.
        // Then the state of vbrId2 should be changed to UP.
        addStaticEdgePort(this, internalPort);
        waiter.set(vbrId2, VnodeState.UP).await();
        vmap2.setActive(Boolean.TRUE);
        vnet.verify();

        // Up all the edge ports in edgeNode.
        // Then the state of vbrId1 should be changed to UP.
        for (String pid: edgePorts) {
            assertEquals(true, ofMockService.setPortState(pid, true, false));
            waiter.set(vbrId1, VnodeState.UP).await();
            vmap1.setActive(Boolean.TRUE);
            vnet.verify();
        }

        // Remove static topology configuration.
        // Then the state of vbrId2 should be changed to DOWN.
        removeVtnStaticTopology(this);
        waiter.set(vbrId2, VnodeState.DOWN).await();
        vmap2.setActive(Boolean.FALSE);
        vnet.verify();

        // Remove VTN.
        removeVtn(vtnService, tname1);
        vnet.removeTenant(tname1).verify();
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

        // Create virtual interfaces.
        VirtualNetwork vnet = new VirtualNetwork(this);
        String tname1 = "vtn_1";
        String bname1 = "bridge_1";
        String bname2 = "bridge_2";
        String bname3 = "bridge_3";
        String bname4 = "bridge_4";
        String iname1 = "if_1";
        String iname2 = "if_2";
        String iname3 = "if_3";
        String[] bnames = {bname1, bname2, bname3, bname4};
        String[] inames = {iname1, iname2, iname3};

        List<VInterfaceIdentifier<?>> ifIds = new ArrayList<>();
        for (String bname: bnames) {
            for (String iname: inames) {
                VBridgeIfIdentifier bifId =
                    new VBridgeIfIdentifier(tname1, bname, iname);
                ifIds.add(bifId);
                vnet.addInterface(bifId);
            }

            VTerminalIfIdentifier tifId =
                new VTerminalIfIdentifier(tname1, bname, iname1);
            ifIds.add(tifId);
            vnet.addInterface(tifId);
        }

        vnet.apply().verify();

        // Set port mappings.
        Random rand = new Random(3141592L);
        String[] nodes = {
            ID_OPENFLOW + "12345",
            ID_OPENFLOW + "18446744073709551615",
            ID_OPENFLOW + "1",
            ID_OPENFLOW + "9999999",
        };
        String[] portIds = {
            "1", "2", "45", "4294967040",
        };
        String[] portNames = {
            "port-1", "ether-2", "if-4294967040",
        };
        Integer[] vlanIds = {
            null, 0, 123, 4095,
        };
        for (VInterfaceIdentifier<?> ifId: ifIds) {
            String node = nodes[rand.nextInt(nodes.length)];
            String id = portIds[rand.nextInt(portIds.length)];
            String name = portNames[rand.nextInt(portNames.length)];
            Integer vid = vlanIds[rand.nextInt(vlanIds.length)];

            int i = rand.nextInt(5);
            if (i == 0) {
                id = null;
            } else if (i == 1) {
                name = null;
            }

            VTNPortMapConfig pmap = new VTNPortMapConfig(node, id, name, vid);
            assertEquals(VtnUpdateType.CREATED,
                         pmap.update(portMapService, ifId));
            vnet.getInterface(ifId).setPortMap(pmap);
            vnet.verify();

            // Try to set port mapping with the same parameter.
            assertEquals(null, pmap.update(portMapService, ifId));
            vnet.verify();
        }

        // Update port mappings.
        String anotherNode = ID_OPENFLOW + "5";
        String anotherPortId = "3";
        String anotherPortName = "port-12345";
        Integer anotherVid = 1;
        for (VInterfaceIdentifier<?> ifId: ifIds) {
            VInterfaceConfig iconf = vnet.getInterface(ifId);
            VTNPortMapConfig pmap = iconf.getPortMap();
            String node = pmap.getNode();
            String id = pmap.getPortId();
            String name = pmap.getPortName();
            Integer vid = pmap.getVlanId();

            pmap.setNode(anotherNode);
            assertEquals(VtnUpdateType.CHANGED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            pmap.setPortId(anotherPortId);
            assertEquals(VtnUpdateType.CHANGED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            pmap.setPortName(anotherPortName);
            assertEquals(VtnUpdateType.CHANGED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            pmap.setVlanId(anotherVid);
            assertEquals(VtnUpdateType.CHANGED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            pmap.setNode(node).
                setPortId(id).
                setPortName(name).
                setVlanId(vid);
            assertEquals(VtnUpdateType.CHANGED,
                         pmap.update(portMapService, ifId));
            vnet.verify();
        }

        // Error tests.

        // Null input.
        checkRpcError(portMapService.setPortMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(portMapService.removePortMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        NodeId nodeId = new NodeId(nodes[0]);
        SetPortMapInput input = new SetPortMapInputBuilder().build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new SetPortMapInputBuilder().
            setBridgeName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        RemovePortMapInput rinput = new RemovePortMapInputBuilder().build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setBridgeName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No bridge name.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null interface-name.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            input = new SetPortMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = new SetPortMapInputBuilder().
                setTenantName(name).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(portMapService.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(name).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                build();
            checkRpcError(portMapService.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(portMapService.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid terminal-name.
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(name).
                setInterfaceName(iname1).
                build();
            checkRpcError(portMapService.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid interface-name.
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setInterfaceName(name).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(name).
                setNode(nodeId).
                setPortId(portIds[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(portMapService.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePortMapInputBuilder().
                setTenantName(tname1).
                setBridgeName(bname1).
                setInterfaceName(name).
                build();
            checkRpcError(portMapService.removePortMap(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Specifying virtual interface that is not present.
        String unknownName = "unknown";
        input = new SetPortMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(unknownName).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(unknownName).
            setBridgeName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(unknownName).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            setInterfaceName(iname1).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(unknownName).
            setInterfaceName(iname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(unknownName).
            setInterfaceName(iname1).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            setInterfaceName(unknownName).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(unknownName).
            setNode(nodeId).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setBridgeName(bname1).
            setInterfaceName(unknownName).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        rinput = new RemovePortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(unknownName).
            build();
        checkRpcError(portMapService.removePortMap(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // No node.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            setPortId(portIds[0]).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Both port-id and port-name are null.
        input = new SetPortMapInputBuilder().
            setTenantName(tname1).
            setTerminalName(bname1).
            setInterfaceName(iname1).
            setNode(nodeId).
            build();
        checkRpcError(portMapService.setPortMap(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid node ID.
        for (String node: INVALID_NODE_IDS) {
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setNode(new NodeId(node)).
                setPortName(portNames[0]).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Invalid port ID.
        for (String port: INVALID_PORT_IDS) {
            input = new SetPortMapInputBuilder().
                setTenantName(tname1).
                setTerminalName(bname1).
                setInterfaceName(iname1).
                setNode(nodeId).
                setPortId(port).
                build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Errors should never affect existing port mappings.
        vnet.verify();

        // Remove all port mappings.
        for (VInterfaceIdentifier<?> ifId: ifIds) {
            assertEquals(VtnUpdateType.REMOVED,
                         removePortMap(portMapService, ifId));
            VInterfaceConfig iconf = vnet.getInterface(ifId);
            iconf.setPortMap(null);
            vnet.verify();

            // Try to remove port mapping that was already removed.
            assertEquals(null, removePortMap(portMapService, ifId));
        }

        // Remove VTN.
        removeVtn(vtnService, tname1);
        vnet.removeTenant(tname1).verify();
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

        // Collect edge ports and internal ports.
        List<String> edgePorts = new ArrayList<>();
        Set<String> internalPorts = new HashSet<>();
        Map<String, String> nodeMap = new HashMap<>();
        Map<String, String> portIdMap = new HashMap<>();
        Map<String, String> portNameMap = new HashMap<>();
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                nodeMap.put(pid, nid);
                portIdMap.put(pid, OfMockUtils.getPortId(pid));
                portNameMap.put(pid, ofMockService.getPortName(pid));
                edgePorts.add(pid);
            }
            for (String pid: ofMockService.getPorts(nid, false)) {
                nodeMap.put(pid, nid);
                portIdMap.put(pid, OfMockUtils.getPortId(pid));
                portNameMap.put(pid, ofMockService.getPortName(pid));

                String peer = ofMockService.getPeerIdentifier(pid);
                if (!internalPorts.contains(peer)) {
                    internalPorts.add(pid);
                }
            }
        }
        assertEquals(false, edgePorts.isEmpty());
        assertEquals(false, internalPorts.isEmpty());

        // Map edge ports by specifying port ID.
        VirtualNetwork vnet = new VirtualNetwork(this);
        Map<String, Set<VInterfaceIdentifier<?>>> portMaps = new HashMap<>();
        Map<TestPort, VInterfaceIdentifier<?>> mappedPorts = new HashMap<>();
        String tname1 = "vtn_1";
        String iname = "if";
        int vbrCount = 0;
        int vid = 0;
        for (String pid: edgePorts) {
            String vbrName = "vbridge_" + vbrCount;
            vbrCount++;

            VBridgeIfIdentifier ifId =
                new VBridgeIfIdentifier(tname1, vbrName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            String nid = nodeMap.get(pid);
            String id = portIdMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, id, null, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.UP).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Map edge ports by specifying port name.
        vid = 4095 - edgePorts.size() + 1;
        int termCount = 0;
        for (String pid: edgePorts) {
            String termName = "vterm_" + termCount;
            termCount++;

            VTerminalIfIdentifier ifId =
                new VTerminalIfIdentifier(tname1, termName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            String nid = nodeMap.get(pid);
            String name = portNameMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, null, name, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.UP).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Map edge ports by specifying port ID and port name.
        vid = 123;
        for (String pid: edgePorts) {
            String vbrName = "vbridge_" + vbrCount;
            vbrCount++;

            VBridgeIfIdentifier ifId =
                new VBridgeIfIdentifier(tname1, vbrName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            // At first, map the port with specifying only port ID.
            String nid = nodeMap.get(pid);
            String id = portIdMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, id, null, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.UP).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            // Add port name to the port mapping configuration.
            pmap.setPortName(portNameMap.get(pid));
            assertEquals(VtnUpdateType.CHANGED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Map internal ports by specifying port ID.
        vid = 1024;
        for (String pid: internalPorts) {
            String vbrName = "vbridge_" + vbrCount;
            vbrCount++;

            VBridgeIfIdentifier ifId =
                new VBridgeIfIdentifier(tname1, vbrName, iname);
            vnet.addInterface(ifId).apply();
            VInterfaceConfig iconf = vnet.getInterface(ifId);

            String nid = nodeMap.get(pid);
            String id = portIdMap.get(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(nid, id, null, vid).
                setMappedPort(pid);
            iconf.setPortMap(pmap).
                setState(VnodeState.DOWN).
                setEntityState(VnodeState.UP);
            assertEquals(VtnUpdateType.CREATED,
                         pmap.update(portMapService, ifId));
            vnet.verify();

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            if (ifSet == null) {
                ifSet = new HashSet<>();
                portMaps.put(pid, ifSet);
            }
            ifSet.add(ifId);

            assertEquals(null, mappedPorts.put(new TestPort(pid, vid), ifId));
            vid++;
        }

        // Ensure that VLANs already mapped cannot be mapped to another
        // virtual interface.
        for (Entry<TestPort, VInterfaceIdentifier<?>> entry:
                 mappedPorts.entrySet()) {
            TestPort tp = entry.getKey();
            VInterfaceIdentifier<?> mapIfId = entry.getValue();
            for (VInterfaceIdentifier<?> ifId: mappedPorts.values()) {
                if (mapIfId.equals(ifId)) {
                    VInterfaceConfig iconf = vnet.getInterface(ifId);
                    VTNPortMapConfig pmap = iconf.getPortMap();
                    assertEquals(null, pmap.update(portMapService, ifId));
                } else {
                    String pid = tp.getPortIdentifier();
                    String nid = nodeMap.get(pid);
                    String id = portIdMap.get(pid);
                    VTNPortMapConfig pmap =
                        new VTNPortMapConfig(nid, id, null, tp.getVlanId());
                    SetPortMapInput input = pmap.newInputBuilder(ifId).
                        build();
                    checkRpcError(portMapService.setPortMap(input),
                                  RpcErrorTag.DATA_EXISTS,
                                  VtnErrorTag.CONFLICT);
                }
            }
        }

        // Errors should never affect existing port mappings.
        vnet.verify();

        // Down all the edge ports.
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService);
        for (String pid: edgePorts) {
            assertEquals(true, ofMockService.setPortState(pid, false, false));

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.DOWN).
                    set(ifId, VnodeState.DOWN, VnodeState.DOWN);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.DOWN).
                    setEntityState(VnodeState.DOWN);
            }
            waiter.await();
            vnet.verify();
        }

        // Remove inter-switch links on internalPorts.
        for (String pid: internalPorts) {
            addStaticEdgePort(this, pid);

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.UP).
                    set(ifId, VnodeState.UP, VnodeState.UP);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.UP).
                    setEntityState(VnodeState.UP);
            }
            waiter.await();
            vnet.verify();
        }

        // Up all the edge ports.
        for (String pid: edgePorts) {
            assertEquals(true, ofMockService.setPortState(pid, true, false));

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.UP).
                    set(ifId, VnodeState.UP, VnodeState.UP);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.UP).
                    setEntityState(VnodeState.UP);
            }
            waiter.await();
            vnet.verify();
        }

        // Remove internalPorts from static-edge-ports.
        for (String pid: internalPorts) {
            removeStaticEdgePort(this, pid);

            Set<VInterfaceIdentifier<?>> ifSet = portMaps.get(pid);
            for (VInterfaceIdentifier<?> ifId: ifSet) {
                BridgeIdentifier<?> brId = ifId.getBridgeIdentifier();
                waiter.set(brId, VnodeState.DOWN).
                    set(ifId, VnodeState.DOWN, VnodeState.UP);
                VInterfaceConfig iconf = vnet.getInterface(ifId);
                iconf.setState(VnodeState.DOWN).
                    setEntityState(VnodeState.UP);
            }
            waiter.await();
            vnet.verify();
        }

        // Remove all the port mappings.
        List<VTNPortMapConfig> allMaps = new ArrayList<>();
        boolean removeIf = false;
        for (VInterfaceIdentifier<?> ifId: mappedPorts.values()) {
            VInterfaceConfig iconf = vnet.getInterface(ifId);
            allMaps.add(iconf.getPortMap());
            if (removeIf) {
                removeVinterface(vinterfaceService, ifId);
                vnet.removeInterface(ifId);
            } else {
                assertEquals(VtnUpdateType.REMOVED,
                             removePortMap(portMapService, ifId));
                iconf.setPortMap(null).
                    setState(VnodeState.UNKNOWN).
                    setEntityState(VnodeState.UNKNOWN);
            }

            vnet.verify();
            removeIf = !removeIf;
        }

        for (int i = 0; i < 3; i++) {
            // Restore port mappings.
            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname1, "vbridge");
            vnet.addBridge(vbrId);
            VBridgeConfig bconf = vnet.getBridge(vbrId);

            List<VTerminalIdentifier> vtermIds = new ArrayList<>();
            int count = 0;
            boolean mapBridge = true;
            for (VTNPortMapConfig pmap: allMaps) {
                VInterfaceConfig iconf;
                VInterfaceIdentifier<?> ifId;
                if (mapBridge) {
                    String bifName = "if_" + count;
                    iconf = new VInterfaceConfig();
                    bconf.addInterface(bifName, iconf);
                    ifId = vbrId.childInterface(bifName);
                } else {
                    String termName = "vterm_" + termCount;
                    termCount++;
                    VTerminalIdentifier vtermId =
                        new VTerminalIdentifier(tname1, termName);
                    ifId = vtermId.childInterface(iname);
                    vtermIds.add(vtermId);
                    vnet.addInterface(ifId);
                    iconf = vnet.getInterface(ifId);
                }
                vnet.apply();

                String pid = pmap.getMappedPort();
                assertNotNull(pid);
                VnodeState state = (internalPorts.contains(pid))
                    ? VnodeState.DOWN : VnodeState.UP;
                iconf.setPortMap(pmap).
                    setState(state).
                    setEntityState(VnodeState.UP);
                assertEquals(VtnUpdateType.CREATED,
                             pmap.update(portMapService, ifId));
                vnet.verify();

                mapBridge = !mapBridge;
            }

            if (i == 0) {
                // Remove vBridge and all the vTerminals.
                removeVbridge(vbridgeService, vbrId);
                vnet.removeBridge(vbrId).verify();

                for (VTerminalIdentifier vtermId: vtermIds) {
                    removeVterminal(vterminalService, vtermId);
                    vnet.removeTerminal(vtermId).verify();
                }
            } else {
                // Remove VTN.
                removeVtn(vtnService, tname1);
                vnet.removeTenant(tname1).verify();
            }
        }
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

        // Create one VTN and 2 vBridges.
        String tname = "vtn";
        String bname1 = "vbr1";
        String bname2 = "vbr2";
        VBridgeIdentifier vbrId1 = new VBridgeIdentifier(tname, bname1);
        VBridgeIdentifier vbrId2 = new VBridgeIdentifier(tname, bname2);
        MacEntryWaiter macWaiter1 = new MacEntryWaiter(ofMockService, vbrId1);
        MacEntryWaiter macWaiter2 = new MacEntryWaiter(ofMockService, vbrId2);
        macWaiter1.await();
        macWaiter2.await();

        VBridgeIdentifier[] vbrIds = {vbrId1, vbrId2};
        VirtualNetwork vnet = new VirtualNetwork(this).
            addBridge(vbrIds).
            apply();

        Set<MacEntry> empty = Collections.<MacEntry>emptySet();
        macWaiter1.set(empty).await();
        macWaiter2.set(empty).await();

        // Try to clearn empty MAC address table.
        List<MacAddress> emptyAddr = Collections.<MacAddress>emptyList();
        assertEquals(null, clearMacEntry(vbrId1));
        assertEquals(null, removeMacEntry(vbrId2, emptyAddr));

        // Map VLAN 0, 1 and 2, 3 to bpath1 and bpath2 respectively.
        int vid = 0;
        for (VBridgeIdentifier vbrId: vbrIds) {
            for (int i = 0; i < 2; i++) {
                VBridgeConfig bconf = vnet.getBridge(vbrId);
                VTNVlanMapConfig vmap = new VTNVlanMapConfig(vid);
                bconf.addVlanMap(vmap);
                vid++;
            }

            vnet.apply().verify();
            VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
                set(vbrId, VnodeState.UP);
            waiter.await();
        }

        // Collect edge ports, and create test hosts.
        BridgeNetwork bridge1 = new BridgeNetwork(ofMockService, vbrId1);
        BridgeNetwork bridge2 = new BridgeNetwork(ofMockService, vbrId2);
        Set<MacEntry> ments1 = new HashSet<>();
        Set<MacEntry> ments2 = new HashSet<>();
        Set<String> islPorts = new HashSet<>();
        int idx = 1;
        final int nhosts = 2;
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                String pname = ofMockService.getPortName(pid);
                for (vid = 0; vid <= 1; vid++) {
                    for (int i = 0; i < nhosts; i++) {
                        TestHost th = new TestHost(idx, pid, pname, vid);
                        bridge1.addHost(nid, th);
                        assertTrue(ments1.add(th.getMacEntry()));
                        idx++;
                    }
                }

                Set<Short> vids = new HashSet<>();
                for (vid = 2; vid <= 3; vid++) {
                    for (int i = 0; i < nhosts; i++) {
                        TestHost th = new TestHost(idx, pid, pname, vid);
                        bridge2.addHost(nid, th);
                        assertTrue(ments2.add(th.getMacEntry()));
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

        macWaiter1.await();
        macWaiter2.await();

        // Let the vBridge at bpath1 learn MAC addresses.
        learnHosts(bridge1);
        macWaiter1.set(ments1).await();
        macWaiter2.await();

        // Let the vBridge at bpath2 learn MAC addresses.
        learnHosts(bridge2);
        macWaiter1.await();
        macWaiter2.set(ments2).await();

        // Error tests for remove-mac-entry RPC.

        // Null input.
        checkRpcError(macTableService.removeMacEntry(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        RemoveMacEntryInput input = new RemoveMacEntryInputBuilder().build();
        checkRpcError(macTableService.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        input = new RemoveMacEntryInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(macTableService.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        input = new RemoveMacEntryInputBuilder().
            setTenantName(tname).
            build();
        checkRpcError(macTableService.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            input = new RemoveMacEntryInputBuilder().
                setTenantName(name).
                setBridgeName(bname1).
                build();
            checkRpcError(macTableService.removeMacEntry(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            // Invalid bridge-name.
            input = new RemoveMacEntryInputBuilder().
                setTenantName(tname).
                setBridgeName(name).
                build();
            checkRpcError(macTableService.removeMacEntry(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // null in mac-addresses.
        EtherAddress unknown1 = new EtherAddress(0xa0b0c0d0e0f0L);
        EtherAddress unknown2 = new EtherAddress(0xfeff123456e8L);
        input = newRemoveMacEntryInput(vbrId1, unknown1, null, unknown2);
        checkRpcError(macTableService.removeMacEntry(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Specifying vBridge that is not present.
        String unknownName = "unknown";
        VBridgeIdentifier[] unknownVbrIds = {
            new VBridgeIdentifier(unknownName, bname1),
            new VBridgeIdentifier(unknownName, bname2),
            new VBridgeIdentifier(tname, unknownName),
        };
        for (VBridgeIdentifier vbrId: unknownVbrIds) {
            input = newRemoveMacEntryInput(vbrId, unknown1, unknown2);
            checkRpcError(macTableService.removeMacEntry(input),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Errors should never affect MAC address tables.
        macWaiter1.await();
        macWaiter2.await();

        // Install unicast flow entries.
        List<UnicastFlow> flows1 = unicastTest(bridge1, islPorts, true);
        List<UnicastFlow> flows2 = unicastTest(bridge2, islPorts, true);

        // Move all hosts on untagged network to VLAN 1.
        Map<String, List<TestHost>> edgeMap1 = bridge1.getTestHosts();
        Map<String, List<TestHost>> edgeMap2 = bridge2.getTestHosts();
        Map<TestHost, TestHost> oldHosts0 = new HashMap<>();
        for (Entry<String, List<TestHost>> entry: edgeMap1.entrySet()) {
            List<TestHost> hosts = new ArrayList<>();
            for (TestHost th: entry.getValue()) {
                if (th.getVlanId() == 0) {
                    TestHost host = new TestHost(th, 1);
                    hosts.add(host);
                    assertEquals(null, oldHosts0.put(th, host));
                } else {
                    hosts.add(th);
                }
            }
            entry.setValue(hosts);
        }

        macWaiter1.await();
        macWaiter2.await();

        // Trying to remove MAC addresses that are not learned.
        Map<MacAddress, VtnUpdateType> result = new HashMap<>();
        result.put(unknown1.getMacAddress(), null);
        result.put(unknown2.getMacAddress(), null);
        for (VBridgeIdentifier vbrId: vbrIds) {
            assertEquals(result, removeMacEntry(vbrId, unknown1, unknown2));
        }
        macWaiter1.await();
        macWaiter2.await();

        // Send ICMP packet from moved hosts.
        // This invalidates old MAC address table entries.
        IpNetwork dstIp = IpNetwork.create("192.168.100.255");
        for (Entry<TestHost, TestHost> entry: oldHosts0.entrySet()) {
            TestHost oldHost = entry.getKey();
            TestHost newHost = entry.getValue();
            assertTrue(ments1.remove(oldHost.getMacEntry()));
            sendBroadcastIcmp(newHost, dstIp, bridge1.getMappedVlans());

            // IP address in ICMP packet should not be copied into MAC address
            // table entry.
            MacEntry ment = newHost.getMacEntry(false);
            assertTrue(ments1.add(ment));
            macWaiter1.add(ment).await();
        }

        // All flow entries for an entry should be uninstalled.
        final int tableId = OfMockService.DEFAULT_TABLE;
        for (UnicastFlow unicast: flows1) {
            for (OfMockFlow flow: unicast.getFlowList()) {
                Match match = flow.getMatch();
                if (getVlanMatch(match) == 0) {
                    String nid = flow.getNodeIdentifier();
                    int pri = flow.getPriority();
                    OfMockFlow newFlow = ofMockService.
                        awaitFlow(nid, tableId, match, pri, false);
                    assertEquals(null, newFlow);
                }
            }
        }

        // bpath2 should not be affected.
        macWaiter2.await();
        UnicastFlow.verifyFlows(flows2, true, false);

        // Remove 2 MAC addresses from vbrId1.
        int removed = 0;
        for (Iterator<MacEntry> it = ments1.iterator(); it.hasNext();) {
            EtherAddress eaddr = it.next().getMacAddress();
            MacAddress mac = eaddr.getMacAddress();
            it.remove();
            result.clear();
            assertEquals(null, result.put(mac, VtnUpdateType.REMOVED));

            // Duplicate address should be ignored.
            assertEquals(result, removeMacEntry(vbrId1, eaddr, eaddr));
            macWaiter1.remove(eaddr).await();

            removed++;
            if (removed == 2) {
                break;
            }
        }

        // Remove all the MAC addresses learned by vbrId1.
        result.clear();
        List<MacAddress> addrs = new ArrayList<>(ments1.size());
        for (MacEntry ment: ments1) {
            EtherAddress eaddr = ment.getMacAddress();
            MacAddress mac = eaddr.getMacAddress();
            addrs.add(mac);
            macWaiter1.remove(eaddr);
            assertEquals(null, result.put(mac, VtnUpdateType.REMOVED));
        }
        assertEquals(result, removeMacEntry(vbrId1, addrs));
        macWaiter1.await();
        macWaiter2.await();

        // Purge MAC addresses learned by vbrId2.
        result.clear();
        for (MacEntry ment: ments2) {
            EtherAddress eaddr = ment.getMacAddress();
            MacAddress mac = eaddr.getMacAddress();
            macWaiter2.remove(eaddr);
            assertEquals(null, result.put(mac, VtnUpdateType.REMOVED));
        }
        assertEquals(result, clearMacEntry(vbrId2));
        macWaiter1.await();
        macWaiter2.await();

        assertEquals(null, clearMacEntry(vbrId1));
        assertEquals(null, clearMacEntry(vbrId2));

        // Remove vbrId1.
        removeVbridge(vbridgeService, vbrId1);
        macWaiter1.set(null).await();
        macWaiter2.await();
        vnet.removeBridge(vbrId1).verify();

        // Remove VTN.
        removeVtn(vtnService, tname);
        macWaiter1.await();
        macWaiter2.set(null).await();
        vnet.removeTenant(tname).verify();
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

        // Create a vBridge that does not map any network.
        String tname = "vtn";
        String iname = "if";
        String nomap = "nomap";
        VBridgeIdentifier bpathNomap = new VBridgeIdentifier(tname, nomap);
        VBridgeIfIdentifier bipathNomap = bpathNomap.childInterface(iname);

        // Create a vBridge that maps all untagged network.
        VBridgeIdentifier bpathVlan0 = new VBridgeIdentifier(tname, "vlan0");

        // Create a vBridge that maps VLAN 1 on node 1.
        VBridgeIdentifier bpathNode1 = new VBridgeIdentifier(tname, "node1");

        // Create a vBridge that maps port 1 on node 2.
        VBridgeIdentifier bpathNode2Port1 =
            new VBridgeIdentifier(tname, "node2_port1");
        VBridgeIfIdentifier bipathNode2Port1 =
            bpathNode2Port1.childInterface(iname);

        // Create a vBridge that maps a host at port 2 on node 2 using
        // MAC mapping.
        VBridgeIdentifier bpathMac = new VBridgeIdentifier(tname, "mac");

        // Create a vTerminal that does not map any network.
        VTerminalIdentifier vtpathNomap =
            new VTerminalIdentifier(tname, nomap);
        VTerminalIfIdentifier vtipathNomap = vtpathNomap.childInterface(iname);

        // Create a vTerminal that maps port 3 on node 1.
        VTerminalIdentifier vtpathNode1Port3 =
            new VTerminalIdentifier(tname, "node1_port3");
        VTerminalIfIdentifier vtipathNode1Port3 =
            vtpathNode1Port3.childInterface(iname);

        VirtualNetwork vnet = new VirtualNetwork(this).
            addBridge(bpathVlan0, bpathNode1, bpathMac).
            addInterface(bipathNomap, bipathNode2Port1,
                         vtipathNomap, vtipathNode1Port3).
            apply().
            verify();

        VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
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
            set(vtipathNode1Port3, VnodeState.UNKNOWN, VnodeState.UNKNOWN).
            await();

        // Map VLAN 0 to bpathVlan0.
        VTNVlanMapConfig vmap0 = new VTNVlanMapConfig(0);
        VBridgeConfig bconfVlan0 = vnet.getBridge(bpathVlan0).
            addVlanMap(vmap0);
        vnet.apply().verify();
        waiter.set(bpathVlan0, VnodeState.UP).await();

        // Map VLAN 1 on node1 to bpathNode1.
        BigInteger dpid1 = BigInteger.ONE;
        String nid1 = ID_OPENFLOW + dpid1;
        VTNVlanMapConfig vmap1 = new VTNVlanMapConfig(nid1, 1).
            setActive(false);
        VBridgeConfig bconfNode1 = vnet.getBridge(bpathNode1).
            addVlanMap(vmap1);
        vnet.apply().verify();
        waiter.set(bpathNode1, VnodeState.DOWN).await();

        // Map VLAN 2 on port1 on node 2 to bpathNode2Port1.
        BigInteger dpid2 = BigInteger.valueOf(2L);
        String nid2 = ID_OPENFLOW + dpid2;
        VTNPortMapConfig pmap = new VTNPortMapConfig(nid2, "1", null, 2);
        VInterfaceConfig biconfNode2Port1 =
            vnet.getInterface(bipathNode2Port1).
            setPortMap(pmap).
            setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN);
        vnet.apply().verify();
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();

        // Map a host at port 2 on node 2 to bpathMac.
        String pid22 = OfMockUtils.getPortIdentifier(nid2, 2L);
        TestHost host = new TestHost(1, pid22, "eth2", 4095);
        int hostIdx = 2;
        VTNMacMapConfig macMap = new VTNMacMapConfig().
            addAllowed(new MacVlan(host.getEtherAddress(), host.getVlanId()));
        VBridgeConfig bconfMac = vnet.getBridge(bpathMac).setMacMap(macMap);
        vnet.apply().verify();
        waiter.set(bpathMac, VnodeState.DOWN).await();

        // Map VLAN 10 on port 3 on node 1 to vtpathNode1Port3.
        pmap = new VTNPortMapConfig(nid1, "3", null, 10);
        VInterfaceConfig ticonfNode1Port3 = vnet.
            getInterface(vtipathNode1Port3).
            setPortMap(pmap).
            setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN);
        vnet.apply().verify();
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

        // Create a non-OpenFlow node.
        // This should never affect virtual node state.
        String badProto = "unknown:";
        String badNid = badProto + dpid1;
        nodeIds.add(badNid);
        ofMockService.addNode(badProto, dpid1, false);
        String[] badPorts = new String[npids];
        for (long idx = 1; idx <= nports; idx++) {
            badPorts[(int)idx] = ofMockService.addPort(badNid, idx, false);
        }
        waiter.await();
        vnet.verify();

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
        vnet.verify();

        // Up port 2 on node 3.
        ofMockService.setPortState(pids[3][2], true, false);
        waiter.await();
        vnet.verify();

        // Create node 1.
        // This should never affect virtual node state because no edge port
        // is available yet.
        ofMockService.addNode(dpid1);
        waiter.await();
        vnet.verify();

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
        vnet.verify();

        // Enable port 4 on node 1.
        // This will activate bpathNode1.
        ofMockService.setPortState(pids[1][4], true, false);
        vmap1.setActive(true);
        waiter.set(bpathNode1, VnodeState.UP).await();
        vnet.verify();

        // Add port 3 on node 1.
        // This will establish port mapping on vtipathNode1Port3.
        pids[1][3] = ofMockService.addPort(nid1, 3L, false);
        waiter.set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.DOWN).
            await();
        ticonfNode1Port3.
            setEntityState(VnodeState.DOWN).
            getPortMap().
            setMappedPort(pids[1][3]);
        vnet.verify();

        // Enable port 3 on node 1.
        // This will activate vtpathNode1Port3.
        ofMockService.setPortState(pids[1][3], true, false);
        waiter.set(vtpathNode1Port3, VnodeState.UP).
            set(vtipathNode1Port3, VnodeState.UP, VnodeState.UP).
            await();
        ticonfNode1Port3.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vnet.verify();

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
        biconfNode2Port1.setEntityState(VnodeState.DOWN).
            getPortMap().
            setMappedPort(pids[2][1]);
        vnet.verify();

        // Enable port 1.
        ofMockService.setPortState(pids[2][1], true, false);
        waiter.set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            await();
        biconfNode2Port1.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vnet.verify();

        // Enable all ports, and let the vBridge at bpathVlan0 learn some
        // MAC addresses.
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                ofMockService.setPortState(ports[j], true, false);
            }
        }

        // Need to synchronize background tasks triggered by port state events.
        sleep(BGTASK_DELAY);

        MacEntryWaiter bpathVlan0Hosts =
            new MacEntryWaiter(ofMockService, bpathVlan0);
        for (int i = 1; i < pids.length; i++) {
            String[] ports = pids[i];
            for (int j = 1; j < ports.length; j++) {
                String pid = ports[j];
                String pname = ofMockService.getPortName(pid);
                ofMockService.awaitLinkState(pid, true);
                TestHost th = new TestHost(hostIdx, pid, pname, 0);
                hostIdx++;
                sendBroadcast(ofMockService, th);
                bpathVlan0Hosts.add(th.getMacEntry());
            }
        }

        // Let all vBridges learn MAC addresses.
        MacEntryWaiter bpathNode1Hosts =
            new MacEntryWaiter(ofMockService, bpathNode1);
        for (int i = 1; i < npids; i++) {
            String pid = pids[1][i];
            String pname = ofMockService.getPortName(pid);
            TestHost th = new TestHost(hostIdx, pid, pname, 1);
            hostIdx++;
            sendBroadcast(ofMockService, th);
            bpathNode1Hosts.add(th.getMacEntry());
        }

        MacEntryWaiter bpathNode2Port1Hosts =
            new MacEntryWaiter(ofMockService, bpathNode2Port1);
        MacEntryWaiter bpathMacHosts =
            new MacEntryWaiter(ofMockService, bpathMac);
        final int nhosts = 4;
        for (int i = 0; i < nhosts; i++) {
            String pid = pids[2][1];
            String pname = ofMockService.getPortName(pid);
            TestHost th = new TestHost(hostIdx, pid, pname, 2);
            hostIdx++;
            sendBroadcast(ofMockService, th);
            bpathNode2Port1Hosts.add(th.getMacEntry());

            pid = pids[2][2];
            pname = ofMockService.getPortName(pid);
            th = new TestHost(hostIdx, pid, pname, host.getVlanId());
            hostIdx++;
            sendBroadcast(ofMockService, th);
        }
        waiter.await();
        bpathMacHosts.set(Collections.<MacEntry>emptySet()).await();

        // Send a broadcast packet from host.
        // This will activate bpathMac.
        sendBroadcast(ofMockService, host);
        bpathMacHosts.add(host.getMacEntry()).await();
        waiter.set(bpathMac, VnodeState.UP).await();
        macMap.addMapped(host.getMacMappedHost());
        vnet.verify();
        bpathVlan0Hosts.await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable all ports on a non-OpenFlow node.
        // This should never affect virtual node state.
        for (int i = 1; i < npids; i++) {
            ofMockService.setPortState(badPorts[i], false, false);
        }
        waiter.await();
        vnet.verify();

        // Remove a non-OpenFlow node.
        // This should never affect virtual node state.
        ofMockService.removeNode(badNid);
        waiter.await();
        vnet.verify();

        // Disable port 1 on node 2.
        ofMockService.setPortState(pids[2][1], false, false);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.DOWN).
            await();
        biconfNode2Port1.setState(VnodeState.DOWN).
            setEntityState(VnodeState.DOWN);
        vnet.verify();

        Set<MacEntry> bpathVlan0Entries =
            new HashSet<>(bpathVlan0Hosts.getMacEntries());
        Set<MacEntry> bpathNode1Entries =
            new HashSet<>(bpathNode1Hosts.getMacEntries());
        Set<MacEntry> bpathNode2Port1Entries =
            new HashSet<>(bpathNode2Port1Hosts.getMacEntries());
        Set<MacEntry> bpathMacEntries =
            new HashSet<>(bpathMacHosts.getMacEntries());
        bpathVlan0Hosts.filterOut(pids[2][1]).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.clear().await();
        bpathMacHosts.await();

        // Disable port 2 on node 2.
        ofMockService.setPortState(pids[2][2], false, false);
        waiter.set(bpathMac, VnodeState.DOWN).await();
        macMap.removeMapped(host.getEtherAddress());
        vnet.verify();

        bpathVlan0Hosts.filterOut(pids[2][2]).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.clear().await();

        // Disable all ports on node 1 but port 4.
        Set<String> pidSet = new HashSet<>();
        for (int i = 1; i < npids; i++) {
            if (i != 4) {
                String pid = pids[1][i];
                ofMockService.setPortState(pid, false, false);
                pidSet.add(pid);
            }
        }
        bpathVlan0Hosts.filterOut(pidSet);
        bpathNode1Hosts.filterOut(pidSet);

        for (int i = 1; i < npids; i++) {
            if (i != 4) {
                ofMockService.awaitLinkState(pids[1][i], false);
            }
        }

        // vtipathNode1Port3 should be changed to DOWN because it maps
        // pids[1][3].
        waiter.set(vtpathNode1Port3, VnodeState.DOWN).
            set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.DOWN).
            await();
        ticonfNode1Port3.setState(VnodeState.DOWN).
            setEntityState(VnodeState.DOWN);
        vnet.verify();

        bpathVlan0Hosts.await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable port 4 on node 1.
        ofMockService.setPortState(pids[1][4], false);
        waiter.set(bpathNode1, VnodeState.DOWN).await();
        vmap1.setActive(false);
        vnet.verify();

        bpathVlan0Hosts.filterOut(pids[1][4]).await();
        bpathNode1Hosts.clear().await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable all ports on node 3.
        pidSet.clear();
        for (int i = 1; i < npids; i++) {
            String pid = pids[3][i];
            ofMockService.setPortState(pid, false, false);
            pidSet.add(pid);
        }

        for (int i = 1; i < npids; i++) {
            ofMockService.awaitLinkState(pids[3][i], false);
        }
        waiter.await();
        vnet.verify();
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable port 3 on node 2.
        ofMockService.setPortState(pids[2][3], false);
        waiter.await();
        vnet.verify();
        bpathVlan0Hosts.filterOut(pids[2][3]).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Disable port 4 on node 2.
        ofMockService.setPortState(pids[2][4], false);
        waiter.await();
        vnet.verify();
        bpathVlan0Hosts.clear().await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

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

        for (MacEntry ment: bpathVlan0Entries) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacEntry ment: bpathNode1Entries) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacEntry ment: bpathNode2Port1Entries) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacEntry ment: bpathMacEntries) {
            sendBroadcast(ofMockService, ment);
        }

        waiter.set(bpathNode1, VnodeState.UP).
            set(vtpathNode1Port3, VnodeState.UP).
            set(vtipathNode1Port3, VnodeState.UP, VnodeState.UP).
            set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            set(bpathMac, VnodeState.UP).
            await();
        ticonfNode1Port3.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        biconfNode2Port1.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vmap1.setActive(true);
        macMap.addMapped(host.getMacMappedHost());
        vnet.verify();
        bpathVlan0Hosts.set(bpathVlan0Entries).await();
        bpathNode1Hosts.set(bpathNode1Entries).await();
        bpathNode2Port1Hosts.set(bpathNode2Port1Entries).await();
        bpathMacHosts.set(bpathMacEntries).await();

        // Connect port2 on node2 to port1 on node2.
        String src = pids[2][2];
        String dst = pids[2][1];
        ofMockService.setPeerIdentifier(src, dst);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UP).
            set(bpathMac, VnodeState.DOWN).
            await();
        biconfNode2Port1.setState(VnodeState.DOWN);
        macMap.removeMapped(host.getEtherAddress());
        vnet.verify();

        pidSet.clear();
        Collections.addAll(pidSet, src, dst);
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.clear().await();
        bpathMacHosts.clear().await();

        ofMockService.setPeerIdentifier(src, null);
        waiter.set(bpathNode2Port1, VnodeState.UP).
            set(bipathNode2Port1, VnodeState.UP, VnodeState.UP).
            await();
        biconfNode2Port1.setState(VnodeState.UP).
            setEntityState(VnodeState.UP);
        vnet.verify();

        for (MacEntry ment: bpathNode2Port1Entries) {
            sendBroadcast(ofMockService, ment);
        }
        for (MacEntry ment: bpathMacEntries) {
            sendBroadcast(ofMockService, ment);
        }
        waiter.set(bpathMac, VnodeState.UP).await();
        macMap.addMapped(host.getMacMappedHost());
        vnet.verify();

        bpathVlan0Hosts.await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.set(bpathNode2Port1Entries).await();
        bpathMacHosts.set(bpathMacEntries).await();

        // Remove node 3.
        ofMockService.removeNode(nid3);
        waiter.await();
        vnet.verify();
        pidSet.clear();
        for (int i = 1; i < npids; i++) {
            pidSet.add(pids[3][i]);
        }
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Remove node 2.
        ofMockService.removeNode(nid2);
        waiter.set(bpathNode2Port1, VnodeState.DOWN).
            set(bipathNode2Port1, VnodeState.DOWN, VnodeState.UNKNOWN).
            set(bpathMac, VnodeState.DOWN).
            await();
        biconfNode2Port1.setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN).
            getPortMap().
            setMappedPort(null);
        macMap.removeMapped(host.getEtherAddress());
        vnet.verify();
        pidSet.clear();
        for (int i = 1; i < npids; i++) {
            pidSet.add(pids[2][i]);
        }
        bpathVlan0Hosts.filterOut(pidSet).await();
        bpathNode1Hosts.await();
        bpathNode2Port1Hosts.clear().await();
        bpathMacHosts.clear().await();

        // Remove node 1.
        ofMockService.removeNode(nid1);
        waiter.set(bpathNode1, VnodeState.DOWN).
            set(vtpathNode1Port3, VnodeState.DOWN).
            set(vtipathNode1Port3, VnodeState.DOWN, VnodeState.UNKNOWN).
            await();
        vmap1.setActive(false);
        ticonfNode1Port3.setState(VnodeState.DOWN).
            setEntityState(VnodeState.UNKNOWN).
            getPortMap().
            setMappedPort(null);
        vnet.verify();
        bpathVlan0Hosts.clear().await();
        bpathNode1Hosts.clear().await();
        bpathNode2Port1Hosts.await();
        bpathMacHosts.await();

        // Clean up.
        removeVtn(vtnService, tname);
        bpathVlan0Hosts.set(null).await();
        bpathNode1Hosts.set(null).await();
        bpathNode2Port1Hosts.set(null).await();
        bpathMacHosts.set(null).await();
        vnet.removeTenant(tname).verify();
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
        String tname = "vtn";
        String iname1 = "if1";
        VBridgeIdentifier bpath1 = new VBridgeIdentifier(tname, "vbr1");
        VBridgeIdentifier bpath2 = new VBridgeIdentifier(tname, "vbr2");
        VBridgeIfIdentifier bipath11 = bpath1.childInterface(iname1);
        VBridgeIfIdentifier bipath21 = bpath2.childInterface(iname1);
        VTerminalIfIdentifier vtipath =
            new VTerminalIfIdentifier(tname, "vterm", iname1);
        VirtualNetwork vnet = new VirtualNetwork(this).
            addInterface(bipath11, bipath21, vtipath).
            apply().
            verify();

        // Collect edge ports per node, and all ISL ports.
        Map<String, List<String>> edgePorts = new HashMap<>();
        List<String> edgeNodes = new ArrayList<>();
        Set<String> islPorts = new HashSet<>();
        Set<String> allPorts = new HashSet<>();
        BridgeNetwork bridge1 = new BridgeNetwork(ofMockService, bpath1);
        BridgeNetwork bridge2 = new BridgeNetwork(ofMockService, bpath2);
        Map<VBridgeIdentifier, BridgeNetwork> bridges = new HashMap<>();
        Map<String, String> portNames = new HashMap<>();
        bridges.put(bpath1, bridge1);
        bridges.put(bpath2, bridge2);
        for (String nid: ofMockService.getNodes()) {
            List<String> ports = ofMockService.getPorts(nid, true);
            if (!ports.isEmpty()) {
                edgeNodes.add(nid);
                allPorts.addAll(ports);
                assertEquals(null, edgePorts.put(nid, ports));
            }
            for (String pid: ports) {
                String pname = ofMockService.getPortName(pid);
                assertEquals(null, portNames.put(pid, pname));
            }

            for (String pid: ofMockService.getPorts(nid, false)) {
                String pname = ofMockService.getPortName(pid);
                assertEquals(null, portNames.put(pid, pname));
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
        VTNVlanMapConfig vmap0 = new VTNVlanMapConfig(0);
        VBridgeConfig bconf1 = vnet.getBridge(bpath1).
            addVlanMap(vmap0);

        // But untagged network on edgeNodes[0].ports[0] is mapped to vbr2
        // using port mapping.
        Set<TestPort> portMapped = new HashSet<>();
        String nid0 = edgeNodes.get(0);
        String targetPort = edgePorts.get(nid0).get(0);
        VTNPortMapConfig pmap0 = new VTNPortMapConfig(targetPort, 0).
            setMappedPort(targetPort);
        VInterfaceConfig iconf21 = vnet.getInterface(bipath21).
            setPortMap(pmap0);
        assertTrue(portMapped.add(new TestPort(targetPort, 0)));

        for (List<String> ports: edgePorts.values()) {
            for (String pid: ports) {
                BridgeNetwork bridge = (pid.equals(targetPort))
                    ? bridge2 : bridge1;
                bridge.addMappedVlan(pid, 0);
            }
        }

        // Map VLAN 1 on edgeNodes[0] to vbr1.
        VTNVlanMapConfig vmap1 = new VTNVlanMapConfig(nid0, 1);
        bconf1.addVlanMap(vmap1);

        // Map VLAN 1 on edgeNodes[1] to vbr2.
        String nid1 = edgeNodes.get(1);
        VTNVlanMapConfig vmap2 = new VTNVlanMapConfig(nid1, 1);
        VBridgeConfig bconf2 = vnet.getBridge(bpath2).
            addVlanMap(vmap2);

        // But one port in edgeNodes[1] is mapped to vbr1 using port mapping.
        targetPort = edgePorts.get(nid1).get(1);
        VTNPortMapConfig pmap1 = new VTNPortMapConfig(targetPort, 1).
            setMappedPort(targetPort);
        VInterfaceConfig iconf11 = vnet.getInterface(bipath11).
            setPortMap(pmap1);
        assertTrue(portMapped.add(new TestPort(targetPort, 1)));

        // Apply configuration.
        vnet.apply().verify();

        String mmapPort = null;
        for (String pid: edgePorts.get(nid0)) {
            if (mmapPort == null) {
                // Override this port using MAC mapping.
                mmapPort = pid;
            }
            bridge1.addMappedVlan(pid, 1);
        }
        assertNotNull(mmapPort);
        String mmapNode = OfMockUtils.getNodeIdentifier(mmapPort);

        for (String pid: edgePorts.get(nid1)) {
            BridgeNetwork bridge = (pid.equals(targetPort))
                ? bridge1 : bridge2;
            bridge.addMappedVlan(pid, 1);
        }

        // Create test hosts.
        final int nhosts = 4;
        int hostIdx = bridge1.addTestHosts(1, nhosts);
        hostIdx = bridge2.addTestHosts(hostIdx, nhosts);

        // Map hosts on mmapPort (VLAN 1) to vbr2 using MAC mapping.
        Map<VBridgeIdentifier, Set<TestHost>> mmapAllowed = new HashMap<>();
        assertEquals(null, mmapAllowed.put(bpath2, new HashSet<TestHost>()));
        VTNMacMapConfig macMap1 = new VTNMacMapConfig();
        bconf2.setMacMap(macMap1);
        for (int i = 0; i < nhosts; i++) {
            String pname = portNames.get(mmapPort);
            TestHost th = new TestHost(hostIdx, mmapPort, pname, 1);
            hostIdx++;
            MacVlan mv = new MacVlan(th.getEtherAddress(), th.getVlanId());
            assertTrue(mmapAllowed.get(bpath2).add(th));
            macMap1.addAllowed(mv);
        }
        assertEquals(VtnUpdateType.CREATED,
                     macMap1.update(macMapService, bpath2, VtnAclType.ALLOW,
                                   VtnUpdateOperationType.SET));
        vnet.verify();

        // Configure MAC mappings for VLAN 4095.
        Deque<VBridgeIdentifier> bpathQueue = new LinkedList<>();
        Collections.addAll(bpathQueue, bpath1, bpath2);
        for (Entry<String, List<String>> entry: edgePorts.entrySet()) {
            String nid = entry.getKey();
            List<String> ports = entry.getValue();
            for (String pid: ports) {
                VBridgeIdentifier bpath = bpathQueue.removeFirst();
                bpathQueue.addLast(bpath);

                Set<TestHost> allowed = mmapAllowed.get(bpath);
                VtnUpdateType expected;
                if (allowed == null) {
                    expected = VtnUpdateType.CREATED;
                    allowed = new HashSet<TestHost>();
                    assertEquals(null, mmapAllowed.put(bpath, allowed));
                } else {
                    expected = VtnUpdateType.CHANGED;
                }

                VBridgeConfig bconf = vnet.getBridge(bpath);
                VTNMacMapConfig macMap = bconf.getMacMap();
                if (macMap == null) {
                    macMap = new VTNMacMapConfig();
                    bconf.setMacMap(macMap);
                }

                for (int i = 0; i < nhosts; i++) {
                    String pname = portNames.get(pid);
                    TestHost th = new TestHost(hostIdx, pid, pname, 4095);
                    hostIdx++;
                    assertTrue(allowed.add(th));
                    macMap.addAllowed(th.getMacVlan());
                }

                VtnUpdateType result = macMap.update(
                    macMapService, bpath, VtnAclType.ALLOW,
                    VtnUpdateOperationType.ADD);
                assertEquals(expected, result);
                vnet.verify();
            }
        }

        bridge1.verify();
        bridge2.verify();

        // Ensure that all virtual interfaces are ready.
        // Note that vBridge state should be DOWN because all MAC mappings are
        // still inactivated.
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
            set(bpath1, VnodeState.DOWN).set(bpath2, VnodeState.DOWN).
            set(bipath11, VnodeState.UP, VnodeState.UP).
            set(bipath21, VnodeState.UP, VnodeState.UP);
        waiter.await();
        vnet.verify();

        // Ensure any incoming packet from internal port is ignored.
        for (String pid: islPorts) {
            String pname = portNames.get(pid);
            TestHost th = new TestHost(hostIdx, pid, pname, 0);
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
        MacEntryWaiter macWaiter1 = new MacEntryWaiter(ofMockService, bpath1).
            set(bridge1.getMacEntries()).
            await();
        MacEntryWaiter macWaiter2 = new MacEntryWaiter(ofMockService, bpath2).
            set(bridge2.getMacEntries()).
            await();
        vnet.verify();

        // Send unicast packets.
        List<UnicastFlow> flows1 =
            unicastTest(bridge1, islPorts, true, VnodeState.DOWN);
        List<UnicastFlow> flows2 =
            unicastTest(bridge2, islPorts, true, VnodeState.DOWN);
        FlowCounter counter = new FlowCounter(ofMockService).
            add(flows1).add(flows2).verify();

        // Activate MAC mappings.
        Set<PortVlan> remappedVlans = new HashSet<>();
        for (Entry<VBridgeIdentifier, BridgeNetwork> entry:
                 bridges.entrySet()) {
            VBridgeIdentifier bpath = entry.getKey();
            BridgeNetwork bridge = entry.getValue();
            Set<TestHost> allowed = mmapAllowed.get(bpath);
            for (TestHost th: allowed) {
                String pid = th.getPortIdentifier();
                int vid = th.getVlanId();
                PortVlan pv = new PortVlan(pid, vid);
                boolean remapped = remappedVlans.add(pv);
                if (remapped) {
                    bridge1.removeMappedVlan(pid, vid);
                    bridge2.removeMappedVlan(pid, vid);
                }
                bridge.addHost(th);
                learnHost(bpath, bridge.getMappedVlans(), th);
                vnet.getBridge(bpath).
                    getMacMap().
                    addAllowed(th.getMacVlan()).
                    addMapped(th.getMacMappedHost());

                if (remapped) {
                    // Ensure that flow entries for unmapped hosts have been
                    // uninstalled.
                    verifyFlowUninstalled(pid, vid, flows1);
                    verifyFlowUninstalled(pid, vid, flows2);
                }
            }
        }

        // Ensure that unexpected flow enty is not installed.
        counter.clear().add(flows1).add(flows2).verify();

        // Ensure MAC address entries for unmapped hosts have been removed.
        macWaiter1.set(bridge1.getMacEntries()).await();
        macWaiter2.set(bridge2.getMacEntries()).await();

        // Ensure that all virtual nodes are ready.
        waiter.set(bpath1, VnodeState.UP).set(bpath2, VnodeState.UP).await();
        vnet.verify();

        // Send unicast packets again.
        flows1 = unicastTest(bridge1, islPorts, true);
        flows2 = unicastTest(bridge2, islPorts, true);
        counter.clear().add(flows1).add(flows2).verify();

        // Ensure that flow entries are not changed when port mapping fails.
        Set<String> skip = new HashSet<>();
        for (TestPort tp: portMapped) {
            String pid = tp.getPortIdentifier();
            skip.add(pid);
            VTNPortMapConfig pmap = new VTNPortMapConfig(pid, tp.getVlanId());
            SetPortMapInput input = pmap.newInputBuilder(vtipath).build();
            checkRpcError(portMapService.setPortMap(input),
                          RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
            vnet.verify();
            counter.verify();
            macWaiter1.await();
            macWaiter2.await();
        }

        // Determine port for vTerminal test.
        targetPort = null;
        for (String pid: allPorts) {
            if (!skip.contains(pid) && !islPorts.contains(pid)) {
                targetPort = pid;
                break;
            }
        }
        assertNotNull(targetPort);

        // Try to map edge port to vTerminal.
        OfMockFlow dropFlow = null;
        VInterfaceConfig ticonf = vnet.getInterface(vtipath);
        String portName = portNames.get(targetPort);
        for (int vid: new int[]{0, 1, 4095}) {
            VTNPortMapConfig old = ticonf.getPortMap();
            VtnUpdateType expected = (old == null)
                ? VtnUpdateType.CREATED
                : VtnUpdateType.CHANGED;
            VTNPortMapConfig pmap = new VTNPortMapConfig(targetPort, vid).
                setMappedPort(targetPort);
            ticonf.setPortMap(pmap);
            assertEquals(expected, pmap.update(portMapService, vtipath));

            verifyFlowUninstalled(targetPort, vid, flows1);
            verifyFlowUninstalled(targetPort, vid, flows2);
            counter.clear().add(flows1).add(flows2).verify();
            macWaiter1.remove(bridge1.removeMappedVlan(targetPort, vid)).
                await();
            macWaiter2.remove(bridge2.removeMappedVlan(targetPort, vid)).
                await();

            if (dropFlow != null) {
                String nid = dropFlow.getNodeIdentifier();
                int pri = dropFlow.getPriority();
                Match match = dropFlow.getMatch();
                OfMockFlow f = ofMockService.awaitFlow(
                    nid, OfMockService.DEFAULT_TABLE, match, pri, false);
                assertEquals(null, f);
            }

            // Ensure that vTerminal drops incoming packet.
            TestHost host = new TestHost(hostIdx, targetPort, portName, vid);
            hostIdx++;
            dropFlow = dropTest(host, allPorts);
            counter.add(dropFlow).verify();
            macWaiter1.await();
            macWaiter2.await();
        }

        // Clean up.
        removeVtn(vtnService, tname);
        vnet.removeTenant(tname).verify();
        macWaiter1.set(null).await();
        macWaiter2.set(null).await();
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
        String tname = "vtn";
        VBridgeIdentifier bpath = new VBridgeIdentifier(tname, "vbr");
        VirtualNetwork vnet = new VirtualNetwork(this).
            addBridge(bpath).
            apply().
            verify();

        // Collect edge ports per node, and all ISL ports.
        BridgeNetwork bridge = new BridgeNetwork(ofMockService, bpath);
        Set<String> islPorts = new HashSet<>();
        int idx = 1;
        int vid = 0;
        for (String nid: ofMockService.getNodes()) {
            for (String pid: ofMockService.getPorts(nid, true)) {
                String pname = ofMockService.getPortName(pid);
                bridge.addHost(nid, new TestHost(idx, pid, pname, vid));
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
        VTNVlanMapConfig vmap0 = new VTNVlanMapConfig();
        VBridgeConfig bconf = vnet.getBridge(bpath).
            addVlanMap(vmap0);
        vnet.apply().verify();
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
            set(bpath, VnodeState.UP);

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
        for (Entry<String, List<TestHost>> entry: hostMap.entrySet()) {
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

        vnet.verify();
        removeVtn(vtnService, tname);
        vnet.removeTenant(tname).verify();
    }

    /**
     * Test case for path map.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testPathMap() throws Exception {
        LOG.info("Running testPathMap().");

        // Collect existing switch ports.
        Map<String, Set<Integer>> allPorts = new HashMap<>();
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
            BigInteger dpid = getUnsigned((long)(i + 1));
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

        // Wait for network topology to be established.
        for (Map.Entry<String, String> entry: allLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            ofMockService.awaitLink(src, dst, true);
        }

        // Create a VTN and a vBridge for test.
        String tname = "vtn";
        VBridgeIdentifier bpath = new VBridgeIdentifier(tname, "vbr");
        VBridgeIfIdentifier ipath0 = bpath.childInterface("if0");
        VBridgeIfIdentifier ipath2 = bpath.childInterface("if2");
        VirtualNetwork vnet = new VirtualNetwork(this).
            addInterface(ipath0, ipath2);

        // Map an edge port on nodeIds[0] to ipath0.
        int vid0 = 0;
        String edge0 = ofMockService.getPorts(nodeIds[0], true).get(0);
        VInterfaceConfig iconf0 = vnet.getInterface(ipath0);
        VTNPortMapConfig pmap0 = new VTNPortMapConfig(edge0, vid0).
            setMappedPort(edge0);
        iconf0.setPortMap(pmap0);
        allPorts.put(edge0, Collections.singleton(vid0));

        // Map an edge port on nodeIds[2] to ipath2.
        int vid2 = 2;
        String edge2 = ofMockService.getPorts(nodeIds[2], true).get(0);
        VInterfaceConfig iconf2 = vnet.getInterface(ipath2);
        VTNPortMapConfig pmap2 = new VTNPortMapConfig(edge2, vid2).
            setMappedPort(edge2);
        iconf2.setPortMap(pmap2);
        allPorts.put(edge2, Collections.singleton(vid2));

        // Apply configuration, and wait for the test vBridge to change its
        // state to UP.
        vnet.apply();
        VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
            set(bpath, VnodeState.UP).
            await();
        vnet.verify();

        // Create 2 hosts connected to edge0.
        int hostIdx = 1;
        final int numHosts = 2;
        List<TestHost> hosts0 = new ArrayList<>();
        String pname0 = ofMockService.getPortName(edge0);
        for (int i = 0; i < numHosts; i++) {
            hosts0.add(new TestHost(hostIdx, edge0, pname0, vid0));
            hostIdx++;
        }

        // Create 2 hosts connected to edge2.
        List<TestHost> hosts2 = new ArrayList<>();
        String pname2 = ofMockService.getPortName(edge2);
        for (int i = 0; i < numHosts; i++) {
            hosts2.add(new TestHost(hostIdx, edge2, pname2, vid2));
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

        long defCost1 = 10000L;
        PathPolicy policy1 = new PathPolicy(1, defCost1);
        int pcMode = PATH_COST_MODE_NAME;
        for (OfMockLink l: route1) {
            policy1.add(newPathCost(l.getSourcePort(), null, pcMode));
        }

        assertEquals(VtnUpdateType.CREATED,
                     policy1.update(pathPolicyService,
                                    VtnUpdateOperationType.ADD, false));
        vnet.getPathPolicies().add(policy1);
        vnet.verify();

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
        PathPolicy policy2 = new PathPolicy(2, defCost2);
        assertEquals(VtnUpdateType.CREATED,
                     policy2.update(pathPolicyService,
                                    VtnUpdateOperationType.SET, null));
        vnet.getPathPolicies().add(policy2);
        vnet.verify();

        Map<VtnPortDesc, VtnUpdateType> pdResult = new HashMap<>();
        List<PathCost> costs = new ArrayList<>();
        for (OfMockLink l: route2) {
            PathCost pc = newPathCost(l.getSourcePort(), 2L);
            costs.add(pc);
            VtnPortDesc pdesc = pc.getPortDesc();
            assertEquals(null, pdResult.put(pdesc, VtnUpdateType.CREATED));
        }

        assertEquals(pdResult, policy2.add(pathPolicyService, costs));
        vnet.verify();

        // Increase cost for the links from nodeIds[0] to nodeIds[1]/nodeIds[2]
        // for later test.
        pdResult.clear();
        costs.clear();
        pcMode = PATH_COST_MODE_NAME | PATH_COST_MODE_ID;
        for (String dst: new String[]{nodeIds[1], nodeIds[2]}) {
            link = getInterSwitchLink(nodeIds[0], dst);
            assertNotNull(link);
            long cost = 100000000000L;
            PathCost pc = newPathCost(link.getSourcePort(), cost, pcMode);
            costs.add(pc);
            VtnPortDesc pdesc = pc.getPortDesc();
            assertEquals(null, pdResult.put(pdesc, VtnUpdateType.CREATED));
        }
        assertEquals(pdResult, policy2.add(pathPolicyService, costs));
        vnet.verify();

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
        PathPolicy policy3 = new PathPolicy(3, defCost3);
        assertEquals(VtnUpdateType.CREATED,
                     policy3.update(pathPolicyService, null, false));
        vnet.getPathPolicies().add(policy3);
        vnet.verify();

        PathCost pc3 = null;
        long cost3 = 10L;
        for (OfMockLink l: route3) {
            String pid = l.getSourcePort();
            PathCost pc = newPathCost(pid, cost3);
            if (OfMockUtils.getNodeIdentifier(pid).equals(nodeIds[3])) {
                assertEquals(null, pc3);
                pc3 = pc;
            }
            policy3.add(pc);
        }
        assertNotNull(pc3);
        assertEquals(VtnUpdateType.CHANGED,
                     policy3.update(pathPolicyService,
                                    VtnUpdateOperationType.SET, false));

        // Increase cost for the link from nodeIds[0] to nodeIds[2]/nodeIds[3]
        // for later test.
        pdResult.clear();
        costs.clear();
        for (String dst: new String[]{nodeIds[2], nodeIds[3]}) {
            link = getInterSwitchLink(nodeIds[0], dst);
            assertNotNull(link);
            long cost = defCost3 * 2L;
            PathCost pc = newPathCost(link.getSourcePort(), cost);
            costs.add(pc);
            VtnPortDesc pdesc = pc.getPortDesc();
            assertEquals(null, pdResult.put(pdesc, VtnUpdateType.CREATED));
        }
        assertEquals(pdResult, policy3.add(pathPolicyService, costs));
        vnet.verify();

        // Configure flow conditions.

        // udp_src_50000: Match UDP packet from port 50000.
        String cnameUdp = "udp_src_50000";
        FlowCondition condUdp = new FlowCondition(cnameUdp);
        vnet.getFlowConditions().add(condUdp);
        assertEquals(VtnUpdateType.CREATED,
                     condUdp.update(flowCondService, null, null));
        assertEquals(null, condUdp.update(flowCondService, null, null));
        vnet.verify();

        final int udpSrcPort = 50000;
        VTNUdpMatch udm = new VTNUdpMatch(udpSrcPort, null);
        FlowMatch fm = new FlowMatch(1, null, null, udm);
        assertEquals(VtnUpdateType.CREATED, condUdp.add(flowCondService, fm));
        assertEquals(null, condUdp.add(flowCondService, fm));
        vnet.verify();

        // tcp_dst_23: Match TCP packet destinated to port 23.
        String cnameTcp = "tcp_dst_23";
        FlowCondition condTcp = new FlowCondition(cnameTcp);
        vnet.getFlowConditions().add(condTcp);
        assertEquals(VtnUpdateType.CREATED,
                     condTcp.update(flowCondService,
                                    VtnUpdateOperationType.ADD, null));
        vnet.verify();

        final int tcpDstPort = 23;
        VTNTcpMatch tcm = new VTNTcpMatch(null, tcpDstPort);
        fm = new FlowMatch(2, null, null, tcm);
        assertEquals(VtnUpdateType.CREATED, condTcp.add(flowCondService, fm));
        assertEquals(null, condTcp.add(flowCondService, fm));
        vnet.verify();

        // dscp_10: Match IPv4 packet assigned 10 as DSCP.
        final short dscpValue = 10;
        String cnameDscp = "dscp_10";
        VTNInet4Match im = new VTNInet4Match().setDscp(dscpValue);
        FlowCondition condDscp = new FlowCondition(cnameDscp).
            add(new FlowMatch(3, null, im, null));
        vnet.getFlowConditions().add(condDscp);
        assertEquals(VtnUpdateType.CREATED,
                     condDscp.update(flowCondService,
                                     VtnUpdateOperationType.SET, false));
        vnet.verify();

        // Configure path maps.

        // Map udp_src_50000 packets to path policy 1 using VTN-local path map.
        PathMap vpmap1 = new PathMap(1, cnameUdp, 1);
        assertEquals(VtnUpdateType.CREATED,
                     vpmap1.update(pathMapService, tname));
        assertEquals(null, vpmap1.update(pathMapService, tname));
        VTenantConfig tconf = vnet.getTenant(tname);
        tconf.getPathMaps().add(vpmap1);
        vnet.verify();

        // Map dscp_10 packets to path policy 2 using VTN-local path map.
        int idle2 = 10000;
        int hard2 = 30000;
        PathMap vpmap2 = new PathMap(2, cnameDscp, 2, idle2, hard2);
        assertEquals(VtnUpdateType.CREATED,
                     vpmap2.update(pathMapService, tname));
        assertEquals(null, vpmap2.update(pathMapService, tname));
        tconf.getPathMaps().add(vpmap2);
        vnet.verify();

        // Map tcp_dst_23 packets to path policy 3 using global path map.
        int idle3 = 11111;
        int hard3 = 32000;
        PathMap gpmap = new PathMap(1, cnameTcp, 3, idle3, hard3);
        assertEquals(VtnUpdateType.CREATED, gpmap.update(pathMapService));
        assertEquals(null, gpmap.update(pathMapService));
        vnet.getPathMaps().add(gpmap);
        vnet.verify();

        // Ensure that network topology is established.
        for (Map.Entry<String, String> entry: allLinks.entrySet()) {
            String src = entry.getKey();
            String dst = entry.getValue();
            ofMockService.awaitLink(src, dst, true);
        }

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
            unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2,
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
            unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
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
            unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
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
            unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                        route3);

        // Send TCP packet that should not be matched by any flow condition.
        Tcp4FlowFactory tcpfc3 =
            new Tcp4FlowFactory(ofMockService, (short)333, (short)444);
        tcpfc3.addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.IP_DSCP).
            addMatchType(FlowMatchType.TCP_DST);
        List<UnicastFlow> flows01 =
            unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2,
                        route0);

        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Remove path policy 3.
        // This should remove unicast flows routed by the path policy 3.
        Integer ppId3 = policy3.getId();
        removePathPolicy(pathPolicyService, ppId3);
        vnet.getPathPolicies().remove(ppId3);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, false, true);

        // Restore path policy 3.
        // This should remove all flow entries.
        assertEquals(VtnUpdateType.CREATED,
                     policy3.update(pathPolicyService, null, null));
        vnet.getPathPolicies().add(policy3);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, false, true);

        // Ensure that no flow entry is installed.
        counter.clear().verify();

        // Restore unicast flows in reverse order.
        flows01 = unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                             route3);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route2);
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                             route1);
        flows00 = unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);

        // Set 1 to the default cost for the path policy 1.
        // This should remove unicast flows routed by the path policy 1.
        defCost1 = 1L;
        PathPolicy policy = new PathPolicy(1, defCost1);
        assertEquals(VtnUpdateType.CHANGED,
                     policy.update(pathPolicyService, null, true));
        assertEquals(null, policy.update(pathPolicyService, null, true));
        policy1.setDefaultCost(defCost1);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Now the path policy 1 will choose the shortest path for packets
        // from nodeIds[0] to nodeIds[2].
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);

        // Increase the cost for transmitting packet from nodeIds[3] to
        // nodeIds[2] in the path policy 2.
        // This should remove unicast flows routed by the path policy 2.
        link = getInterSwitchLink(nodeIds[3], nodeIds[2]);
        assertNotNull(link);
        long cost2 = 200000000L;
        pcMode = PATH_COST_MODE_NAME | PATH_COST_MODE_ID;
        PathCost pc2 = newPathCost(link.getSourcePort(), cost2, pcMode);
        assertEquals(VtnUpdateType.CREATED,
                     policy2.add(pathPolicyService, pc2));
        assertEquals(null, policy2.add(pathPolicyService, pc2));
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, true, false);

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
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route2);
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, true, false);

        // Remove the cost for transmitting packet from nodeIds[3] to
        // nodeIds[2] in the path policy 3.
        // This should remove unicast flows routed by the path policy 3.
        assertEquals(VtnUpdateType.REMOVED,
                     policy3.removeCost(pathPolicyService, pc3));
        assertEquals(null, policy3.removeCost(pathPolicyService, pc3));
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, true, false);
        UnicastFlow.verifyFlows(flows3, false, true);

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
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
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
        Integer ppId2 = policy2.getId();
        removePathPolicy(pathPolicyService, ppId2);
        vnet.getPathPolicies().remove(ppId2);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, true, false);
        UnicastFlow.verifyFlows(flows01, true, false);
        UnicastFlow.verifyFlows(flows1, true, false);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, true, false);
        counter.clear().add(flows00).add(flows01).add(flows1).add(flows3).
            verify();

        // The VTN-local path map 2 should be ignored because the path policy
        // 2 is not present. So flows2 should be mapped by the global path 1.
        tcpfc1.clearMatchType().
            addMatchType(FlowMatchType.ETH_TYPE).
            addMatchType(FlowMatchType.IP_PROTO).
            addMatchType(FlowMatchType.TCP_DST).
            setIdleTimeout(idle3).setHardTimeout(hard3);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route3);
        counter.clear().add(flows00).add(flows01).add(flows1).add(flows2).
            add(flows3).verify();

        // Remove flow condition tcp_dst_23 used by the global path map 1.
        // This will remove all flow entries.
        removeFlowCondition(flowCondService, cnameTcp);
        vnet.getFlowConditions().remove(cnameTcp);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        UnicastFlow.verifyFlows(flows2, false, true);
        UnicastFlow.verifyFlows(flows3, false, true);
        counter.clear().verify();

        // Configure flow timeout for the VTN-local path map 1.
        int idle1 = 12345;
        int hard1 = 23456;
        vpmap1.setIdleTimeout(idle1).setHardTimeout(hard1);
        assertEquals(VtnUpdateType.CHANGED,
                     vpmap1.update(pathMapService, tname));
        assertEquals(null, vpmap1.update(pathMapService, tname));
        vnet.verify();
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
        flows00 = unicastTest(arpfc, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);
        flows01 = unicastTest(tcpfc3, bpath, VnodeState.UP, hosts0, hosts2,
                              route0);
        flows1 = unicastTest(udpfc, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);

        // Unicast flows that will be created by tcpfc1 and tcpfc2 should be
        // forwarded by flow entries craeted by tcpfc3.
        tcpfc1.setAlreadyMapped(true);
        tcpfc2.setAlreadyMapped(true);
        flows2 = unicastTest(tcpfc1, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);
        flows3 = unicastTest(tcpfc2, bpath, VnodeState.UP, hosts0, hosts2,
                             route0);
        counter.clear().add(flows00).add(flows01).add(flows1).verify();

        // Remove global path map.
        // This will remove all flow entries.
        Integer gpmIdx = gpmap.getIndex();
        assertEquals(VtnUpdateType.REMOVED,
                     removePathMap(pathMapService, null, gpmIdx));
        assertEquals(null, removePathMap(pathMapService, null, gpmIdx));
        vnet.getPathMaps().remove(gpmIdx);
        vnet.verify();
        UnicastFlow.verifyFlows(flows00, false, true);
        UnicastFlow.verifyFlows(flows01, false, true);
        UnicastFlow.verifyFlows(flows1, false, true);
        counter.clear().verify();

        // Clean up.
        assertEquals(VtnUpdateType.REMOVED,
                     getRpcResult(flowCondService.clearFlowCondition()));
        assertEquals(null, getRpcResult(flowCondService.clearFlowCondition()));
        vnet.getFlowConditions().clear();
        vnet.verify();

        assertEquals(VtnUpdateType.REMOVED,
                     getRpcResult(pathPolicyService.clearPathPolicy()));
        assertEquals(null, getRpcResult(pathPolicyService.clearPathPolicy()));
        vnet.getPathPolicies().clear();
        vnet.verify();

        assertEquals(VtnUpdateType.REMOVED,
                     clearPathMap(pathMapService, tname));
        assertEquals(null, clearPathMap(pathMapService, tname));
        tconf.getPathMaps().clear();
        vnet.verify();

        removeVtn(vtnService, tname);
        vnet.removeTenant(tname).verify();
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

    /**
     * Learn all hosts mapped to the given bridge network.
     *
     * @param bridge  A {@link BridgeNetwork} instance.
     * @throws Exception  An error occurred.
     */
    private void learnHosts(BridgeNetwork bridge) throws Exception {
        VBridgeIdentifier vbrId = bridge.getBridgeId();
        Map<String, List<TestHost>> hostMap = bridge.getTestHosts();
        Map<String, Set<Integer>> allPorts = bridge.getMappedVlans();

        for (List<TestHost> hosts: hostMap.values()) {
            for (TestHost host: hosts) {
                learnHost(vbrId, allPorts, host);
            }
        }
    }

    /**
     * Learn the given host mapped to the given bridge network.
     *
     * @param vbrId     The identifier for the target vBridge.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must
     *                  be associated with the key.
     * @param host      A test host to learn.
     * @throws Exception  An error occurred.
     */
    private void learnHost(VBridgeIdentifier vbrId,
                           Map<String, Set<Integer>> allPorts,
                           TestHost host) throws Exception {
        learnHost(ofMockService, allPorts, host);

        MacAddress mac = host.getEtherAddress().getMacAddress();
        try (ReadOnlyTransaction rtx = newReadOnlyTransaction()) {
            InstanceIdentifier<MacTableEntry> path =
                VBridgeIdentifier.getMacEntryPath(vbrId, mac);
            Optional<MacTableEntry> opt = DataStoreUtils.read(rtx, path);
            assertEquals(true, opt.isPresent());
            assertEquals(host.getMacEntry(), new MacEntry(opt.get()));
        }
    }

    /**
     * Create a new input builder for remove-mac-entry RPC.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @return  A {@link RemoveMacEntryInputBuilder} instance.
     */
    private RemoveMacEntryInputBuilder newRemoveMacEntryInputBuilder(
        VBridgeIdentifier vbrId) {
        return new RemoveMacEntryInputBuilder().
            setTenantName(vbrId.getTenantNameString()).
            setBridgeName(vbrId.getBridgeNameString());
    }

    /**
     * Create a new input for remove-mac-entry RPC.
     *
     * @param vbrId   The identifier for the target vBridge.
     * @param eaddrs  An array of {@link EtherAddress} instances.
     * @return  A {@link RemoveMacEntryInput} instance.
     */
    private RemoveMacEntryInput newRemoveMacEntryInput(
        VBridgeIdentifier vbrId, EtherAddress ... eaddrs) {
        List<MacAddress> addrs;
        if (eaddrs == null) {
            addrs = null;
        } else {
            addrs = new ArrayList<>(eaddrs.length);
            for (EtherAddress eaddr: eaddrs) {
                MacAddress mac = (eaddr == null)
                    ? null : eaddr.getMacAddress();
                addrs.add(mac);
            }
        }

        return newRemoveMacEntryInputBuilder(vbrId).
            setMacAddresses(addrs).
            build();
    }

    /**
     * Remove the given MAC addresses from the MAC address table entry.
     *
     * @param vbrId   The identifier for the target vBridge.
     * @param eaddrs  An array of {@link EtherAddress} instances.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> removeMacEntry(
        VBridgeIdentifier vbrId, EtherAddress ... eaddrs) {
        RemoveMacEntryInput input = newRemoveMacEntryInput(vbrId, eaddrs);
        return removeMacEntry(input);
    }

    /**
     * Remove the given MAC addresses from the MAC address table entry.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @param addrs  A list of {@link MacAddress} instances.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> removeMacEntry(
        VBridgeIdentifier vbrId, List<MacAddress> addrs) {
        RemoveMacEntryInput input = newRemoveMacEntryInputBuilder(vbrId).
            setMacAddresses(addrs).
            build();
        return removeMacEntry(input);
    }

    /**
     * Remove all the MAC addresses in the MAC address table.
     *
     * @param vbrId  The identifier for the target vBridge.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> clearMacEntry(
        VBridgeIdentifier vbrId) {
        RemoveMacEntryInput input = newRemoveMacEntryInputBuilder(vbrId).
            build();
        return removeMacEntry(input);
    }

    /**
     * Remove the MAC addresses specified by the given RPC input.
     *
     * @param input  A {@link RemoveMacEntryInput} instance.
     * @return  A map that contains the output of the remove-mac-entry RPC.
     */
    private Map<MacAddress, VtnUpdateType> removeMacEntry(
        RemoveMacEntryInput input) {
        RemoveMacEntryOutput output =
            getRpcOutput(macTableService.removeMacEntry(input));

        Map<MacAddress, VtnUpdateType> result;
        List<RemoveMacEntryResult> res = output.getRemoveMacEntryResult();
        if (res == null) {
            result = null;
        } else {
            result = new HashMap<>();
            for (RemoveMacEntryResult rmres: res) {
                MacAddress mac = rmres.getMacAddress();
                VtnUpdateType status = rmres.getStatus();
                assertEquals(null, result.put(mac, status));
            }
        }

        return result;
    }

    /**
     * Do the ARP unicast packet forwarding test.
     *
     * @param bridge    A {@link BridgeNetwork} instance.
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
     * @param factory   A {@link UnicastFlowFactory} instance used to
     *                  create unicast flows.
     * @param bridge    A {@link BridgeNetwork} instance.
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
        VBridgeIdentifier vbrId = bridge.getBridgeId();
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
        Set<NodePath> fpaths = new HashSet<>();
        for (Entry<String, List<TestHost>> srcEnt: hostMap.entrySet()) {
            String srcNid = srcEnt.getKey();
            List<TestHost> srcHosts = srcEnt.getValue();
            for (Entry<String, List<TestHost>> dstEnt: hostMap.entrySet()) {
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
                    fpaths.add(new NodePath(srcNid, dstNid));
                    assertEquals(null, route);
                }

                flows.addAll(unicastTest(factory, vbrId, bridgeState, srcHosts,
                                         dstHosts, fpaths, route));
            }
        }

        return flows;
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param vbrId        The identifier for the the test vBridge.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @param srcList      A list of source hosts.
     * @param dstList      A list of destination hosts.
     * @param route        A list of {@link OfMockLink} which represents the
     *                     packet route.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(
        UnicastFlowFactory factory, VBridgeIdentifier vbrId,
        VnodeState bridgeState, List<TestHost> srcList, List<TestHost> dstList,
        List<OfMockLink> route)
        throws Exception {
        return unicastTest(factory, vbrId, bridgeState, srcList, dstList,
                           Collections.<NodePath>emptySet(), route);
    }

    /**
     * Do the unicast packet forwarding test.
     *
     * @param factory      A {@link UnicastFlowFactory} instance used to
     *                     create unicast flows.
     * @param vbrId        The identifier for the the test vBridge.
     * @param bridgeState  Expected vBridge state when all ISL port is up.
     * @param srcList      A list of source hosts.
     * @param dstList      A list of destination hosts.
     * @param fpaths       A set of expected faulted paths.
     * @param route        A list of {@link OfMockLink} which represents the
     *                     packet route.
     * @return  A list of {@link UnicastFlow} instances that represents
     *          established unicast flows.
     * @throws Exception  An error occurred.
     */
    private List<UnicastFlow> unicastTest(
        UnicastFlowFactory factory, VBridgeIdentifier vbrId,
        VnodeState bridgeState, List<TestHost> srcList, List<TestHost> dstList,
        Set<NodePath> fpaths, List<OfMockLink> route) throws Exception {
        List<UnicastFlow> flows = new ArrayList<>();
        boolean reachable = fpaths.isEmpty();
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
                VNodeStateWaiter waiter = new VNodeStateWaiter(ofMockService).
                    set(vbrId, bstate, fpaths);
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
        EtherAddress eaddr = host.getEtherAddress();
        byte[] sha = eaddr.getBytes();
        byte[] spa = host.getRawInetAddress();
        int vid = host.getVlanId();
        EthernetFactory efc = new EthernetFactory(eaddr, MAC_DUMMY).
            setVlanId(vid);
        ArpFactory afc = ArpFactory.newInstance(efc).
            setSenderHardwareAddress(sha).
            setTargetHardwareAddress(MAC_DUMMY.getBytes()).
            setSenderProtocolAddress(spa).
            setTargetProtocolAddress(IPV4_DUMMY.getBytes());
        byte[] payload = efc.create();
        ofMockService.sendPacketIn(ingress, payload);

        // Ensure that a flow entry that drops the packet was installed.
        String nid = OfMockUtils.getNodeIdentifier(ingress);
        int pri = ofMockService.getL2FlowPriority();
        Match match = createMatch(ingress, vid).build();
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
     * Send an ICMPv4 broadcast packet.
     *
     * @param host      Source host.
     * @param dstIp     Destination IP address.
     * @param allPorts  A map that contains all switch ports as map keys.
     *                  A set of VLAN IDs mapped to the given vBridge must be
     *                  associated with the key.
     */
    private void sendBroadcastIcmp(TestHost host, IpNetwork dstIp,
                                   Map<String, Set<Integer>> allPorts)
        throws Exception {
        String pid = host.getPortIdentifier();
        EtherAddress src = host.getEtherAddress();
        IpNetwork srcIp = host.getInetAddress();
        int vid = host.getVlanId();
        byte type = 8;
        byte code = 0;

        EthernetFactory efc = new EthernetFactory(src, EtherAddress.BROADCAST).
            setVlanId(vid);
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

        for (Entry<String, Set<Integer>> entry: allPorts.entrySet()) {
            Set<Integer> vids = entry.getValue();
            if (vids != null) {
                String portId = entry.getKey();
                if (portId.equals(pid)) {
                    // VTN Manager will send an ARP request to probe
                    // IP address.
                    efc.setProbe(srcIp, vid);
                } else {
                    efc.setProbe(null, -1);
                }

                int count = vids.size();
                for (int i = 0; i < count; i++) {
                    byte[] transmitted =
                        ofMockService.awaitTransmittedPacket(portId);
                    vids = efc.verify(ofMockService, transmitted, vids);
                }
                assertTrue(vids.isEmpty());
            }
        }

        for (String p: allPorts.keySet()) {
            assertNull(ofMockService.getTransmittedPacket(p));
        }
    }

    /**
     * Ensure that all flow entries related to the given network have been
     * uninstalled.
     *
     * @param pid    The port identifier of the unmapped network.
     * @param vid    The VLAN ID of the unmapped network.
     * @param flows  A list of {@link UnicastFlow} instances.
     */
    private void verifyFlowUninstalled(String pid, int vid,
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
            int inVid = getVlanMatch(match);
            if (inVid == vid && pid.equals(getInPortMatch(match))) {
                uninstalled.add(unicast);
                it.remove();
                continue;
            }

            // Check egress flow.
            OfMockFlow egress = flowList.get(flowList.size() - 1);
            if (hasOutput(egress.getInstructions(), pid, vid, inVid)) {
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
     * Create a new path cost configuration.
     *
     * @param pid   The MD-SAL switch port identifier.
     * @param cost  The cost of using the specified port.
     * @return  A {@link PathCost} instance.
     */
    private PathCost newPathCost(String pid, Long cost) {
        return newPathCost(pid, cost, PATH_COST_MODE_ID);
    }

    /**
     * Create a new path cost configuration.
     *
     * @param pid   The MD-SAL switch port identifier.
     * @param cost  The cost of using the specified port.
     * @param mode  An integer that specifies how to specify the specified
     *              switch port.
     * @return  A {@link PathCost} instance.
     */
    private PathCost newPathCost(String pid, Long cost, int mode) {
        String node = OfMockUtils.getNodeIdentifier(pid);
        String id = ((mode & PATH_COST_MODE_ID) != 0)
            ? OfMockUtils.getPortId(pid)
            : null;
        String name = ((mode & PATH_COST_MODE_NAME) != 0)
            ? ofMockService.getPortName(pid)
            : null;

        return new PathCost(node, id, name, cost);
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
