/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;
import static org.opendaylight.vtn.manager.it.util.TestBase.RANDOM_ADD_MAX;
import static org.opendaylight.vtn.manager.it.util.TestBase.createVnodeName;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;
import org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.RemoveVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.UpdateVtnInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnVtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VTenantConfig} describes the configuration of a VTN.
 */
public final class VTenantConfig extends VNodeConfig<VTenantConfig>
    implements FlowFilterNode {
    /**
     * The default value of idle-timeout.
     */
    public static final Integer  DEFAULT_IDLE_TIMEOUT = 300;

    /**
     * The default value of hard-timeout.
     */
    public static final Integer  DEFAULT_HARD_TIMEOUT = 0;

    /**
     * A map that keeps vBridge configurations.
     */
    private final Map<String, VBridgeConfig>  vBridges = new HashMap<>();

    /**
     * A map that keeps vTerminal configurations.
     */
    private final Map<String, VTerminalConfig>  vTerminals = new HashMap<>();

    /**
     * A set of path map configurations configured in the VTN.
     */
    private final PathMapSet  pathMaps = new PathMapSet();

    /**
     * The value of idle-timeout.
     */
    private Integer  idleTimeout;

    /**
     * The value of hard-timeout.
     */
    private Integer  hardTimeout;

    /**
     * A list of input flow filter.
     */
    private final FlowFilterList  inputFilter = new FlowFilterList();

    /**
     * Remove the specified VTN.
     *
     * @param service  The vtn RPC service.
     * @param name     The name of the VTN.
     */
    public static void removeVtn(VtnService service, String name) {
        RemoveVtnInput input = new RemoveVtnInputBuilder().
            setTenantName(name).
            build();

        assertEquals(null, getRpcOutput(service.removeVtn(input), true));
    }

    /**
     * Construct a new instance with default values.
     */
    public VTenantConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The description about the VTN.
     * @param idle  The value of idle-timeout.
     * @param hard  The value of hard-timeout.
     */
    public VTenantConfig(String desc, Integer idle, Integer hard) {
        super(desc);
        idleTimeout = idle;
        hardTimeout = hard;
    }

    /**
     * Construct a new instance that contains the given VTN configuration.
     *
     * @param vtnc  A VTN configuration.
     */
    public VTenantConfig(VtnVtenantConfig vtnc) {
        super(vtnc.getDescription());
        idleTimeout = vtnc.getIdleTimeout();
        hardTimeout = vtnc.getHardTimeout();
    }

    /**
     * Create a copy of this instance, and complete missing parameters.
     *
     * @return  A copied {@link VTenantConfig} instance.
     */
    public VTenantConfig complete() {
        Integer idle = idleTimeout;
        if (idle == null) {
            idle = DEFAULT_IDLE_TIMEOUT;
        }

        Integer hard = hardTimeout;
        if (hard == null) {
            hard = DEFAULT_HARD_TIMEOUT;
        }

        return new VTenantConfig(getDescription(), idle, hard);
    }

    /**
     * Return the value of idle-timeout.
     *
     * @return  The value of idle-timeout.
     */
    public Integer getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Set the value of idle-timeout.
     *
     * @param idle  The value of idle-timeout.
     * @return  This instance.
     */
    public VTenantConfig setIdleTimeout(Integer idle) {
        idleTimeout = idle;
        return this;
    }

    /**
     * Return the value of hard-timeout.
     *
     * @return  The value of hard-timeout.
     */
    public Integer getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Set the value of hard-timeout.
     *
     * @param hard  The value of hard-timeout.
     * @return  This instance.
     */
    public VTenantConfig setHardTimeout(Integer hard) {
        hardTimeout = hard;
        return this;
    }

    /**
     * Return the path map configuration.
     *
     * @return  A {@link PathMapSet} instance.
     */
    public PathMapSet getPathMaps() {
        return pathMaps;
    }

    /**
     * Add the given vBridge configuration.
     *
     * @param name   The name of the vBridge.
     * @param bconf  The vBridge configuration.
     * @return  This instance.
     */
    public VTenantConfig addBridge(String name, VBridgeConfig bconf) {
        vBridges.put(name, bconf);
        return this;
    }

    /**
     * Remove the vBridge configuration specified by the name.
     *
     * @param name  The name of the vBridge.
     * @return  This instance.
     */
    public VTenantConfig removeBridge(String name) {
        vBridges.remove(name);
        return this;
    }

    /**
     * Return the configuration of the vBridge specified by the name.
     *
     * @param name  The name of the vBridge.
     * @return  A {@link VBridgeConfig} instance if found.
     *          {@code null} if not found.
     */
    public VBridgeConfig getBridge(String name) {
        return vBridges.get(name);
    }

    /**
     * Return an unmodifiable map that contains all the vBridges in this VTN.
     *
     * @return  An unmodifiable map that contains all the vBridges.
     */
    public Map<String, VBridgeConfig> getBridges() {
        return Collections.unmodifiableMap(vBridges);
    }

    /**
     * Add the given vTerminal configuration.
     *
     * @param name    The name of the vTerminal.
     * @param tmconf  The vTerminal configuration.
     */
    public void addTerminal(String name, VTerminalConfig tmconf) {
        vTerminals.put(name, tmconf);
    }

    /**
     * Remove the vTerminal configuration specified by the name.
     *
     * @param name  The name of the vTerminal.
     * @return  This instance.
     */
    public VTenantConfig removeTerminal(String name) {
        vTerminals.remove(name);
        return this;
    }

    /**
     * Return the configuration of the vTerminal.
     *
     * @param name  The name of the vTerminal.
     * @return  A {@link VTerminalConfig} instance if found.
     *          {@code null} if not found.
     */
    public VTerminalConfig getTerminal(String name) {
        return vTerminals.get(name);
    }

    /**
     * Return an unmodifiable map that contains all the vTerminals in this VTN.
     *
     * @return  An unmodifiable map that contains all the vTerminals.
     */
    public Map<String, VTerminalConfig> getTerminals() {
        return Collections.unmodifiableMap(vTerminals);
    }

    /**
     * Create a new input builder for update-vtn RPC.
     *
     * @return  An {@link UpdateVtnInputBuilder} instance.
     */
    public UpdateVtnInputBuilder newInputBuilder() {
        return new UpdateVtnInputBuilder().
            setDescription(getDescription()).
            setIdleTimeout(idleTimeout).
            setHardTimeout(hardTimeout);
    }

    /**
     * Add random configuration to this VTN configuration using the given
     * random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public VTenantConfig add(Random rand) {
        int count = rand.nextInt(RANDOM_ADD_MAX);
        for (int i = 0; i <= count; i++) {
            addBridge(createVnodeName("vbr_", rand), new VBridgeConfig());
        }

        count = rand.nextInt(RANDOM_ADD_MAX);
        for (int i = 0; i <= count; i++) {
            addTerminal(createVnodeName("vterm_", rand),
                        new VTerminalConfig());
        }

        pathMaps.add(rand);
        inputFilter.add(rand);
        return this;
    }

    /**
     * Update the specified VTN.
     *
     * @param service  The vtn RPC service.
     * @param name     The name of the VTN.
     * @param mode     A {@link VnodeUpdateMode} instance.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnService service, String name,
                                VnodeUpdateMode mode,
                                VtnUpdateOperationType op) {
        UpdateVtnInput input = newInputBuilder().
            setTenantName(name).
            setUpdateMode(mode).
            setOperation(op).
            build();
        return getRpcResult(service.updateVtn(input));
    }

    /**
     * Verify the given VTN.
     *
     * @param rtx      A read-only MD-SAL datastore transaction.
     * @param name     The name of the VTN.
     * @param vtn      The VTN to be verified.
     * @param tmtable  The MAC address tables associated with the specified
     *                 VTN.
     */
    public void verify(ReadTransaction rtx, String name, Vtn vtn,
                       TenantMacTable tmtable) {
        VnodeName vname = new VnodeName(name);
        assertEquals(vname, vtn.getName());

        // Verify the VTN configuration.
        VtenantConfig vtc = vtn.getVtenantConfig();
        assertEquals(getDescription(), vtc.getDescription());
        verifyFlowTimeout(vtc.getIdleTimeout(), idleTimeout,
                          DEFAULT_IDLE_TIMEOUT);
        verifyFlowTimeout(vtc.getHardTimeout(), hardTimeout,
                          DEFAULT_HARD_TIMEOUT);

        // Verify the vBridges and MAC address tables.
        List<Vbridge> vbrList = vtn.getVbridge();
        List<MacAddressTable> mtables = tmtable.getMacAddressTable();
        if (!vBridges.isEmpty()) {
            assertNotNull(vbrList);
            Set<String> checked = new HashSet<>();
            for (Vbridge vbr: vbrList) {
                VnodeName vbrName = vbr.getName();
                String bname = vbrName.getValue();
                VBridgeConfig bconf = vBridges.get(bname);
                assertNotNull(bconf);
                VBridgeIdentifier ident =
                    new VBridgeIdentifier(vname, vbrName);
                bconf.verify(rtx, ident, vbr);
                assertEquals(true, checked.add(bname));
            }
            assertEquals(checked, vBridges.keySet());

            for (MacAddressTable mtable: mtables) {
                String bname = mtable.getName();
                assertEquals(true, checked.remove(bname));
            }
            assertEquals(Collections.<String>emptySet(), checked);
        } else if (vbrList != null) {
            assertEquals(Collections.<Vbridge>emptyList(), vbrList);
            assertEquals(Collections.<TenantMacTable>emptyList(), mtables);
        }

        // Verify the vTerminals.
        List<Vterminal> vtermList = vtn.getVterminal();
        if (!vTerminals.isEmpty()) {
            assertNotNull(vtermList);
            Set<String> checked = new HashSet<>();
            for (Vterminal vterm: vtermList) {
                VnodeName vtermName = vterm.getName();
                String bname = vtermName.getValue();
                VTerminalConfig vtconf = vTerminals.get(bname);
                assertNotNull(vtconf);
                VTerminalIdentifier ident =
                    new VTerminalIdentifier(vname, vtermName);
                vtconf.verify(rtx, ident, vterm);
                assertEquals(true, checked.add(bname));
            }
            assertEquals(checked, vTerminals.keySet());
        } else if (vtermList != null) {
            assertEquals(Collections.<Vterminal>emptyList(), vtermList);
        }

        // Verify path maps.
        pathMaps.verify(vtn.getVtnPathMaps());

        // Verify the input flow filters.
        inputFilter.verify(vtn.getVtnInputFilter());
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param current  The current VTN.
     * @param name     The name of the VTN.
     */
    public void apply(VTNServices service, String name, Vtn current) {
        // Create the VTN if not present.
        UpdateVtnInput input = newInputBuilder().
            setTenantName(name).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            setOperation(VtnUpdateOperationType.SET).
            build();
        getRpcOutput(service.getVtnService().updateVtn(input));

        VTenantIdentifier vtnId = new VTenantIdentifier(name);
        Vtn vtn = (current == null) ? new VtnBuilder().build() : current;

        // Apply the vBridge configuration.
        Map<String, Vbridge> vbrPresent = removeUnwanted(
            service.getVbridgeService(), name, vtn);
        for (Entry<String, VBridgeConfig> entry: vBridges.entrySet()) {
            String bname = entry.getKey();
            VBridgeConfig bconf = entry.getValue();
            VBridgeIdentifier vbrId =
                new VBridgeIdentifier(vtnId, new VnodeName(bname));
            bconf.apply(service, vbrId, vbrPresent.get(bname));
        }

        // Apply the vTerminal configuration.
        Map<String, Vterminal> vtermPresent = removeUnwanted(
            service.getVterminalService(), name, vtn);
        for (Entry<String, VTerminalConfig> entry: vTerminals.entrySet()) {
            String bname = entry.getKey();
            VTerminalConfig vtconf = entry.getValue();
            VTerminalIdentifier vtermId =
                new VTerminalIdentifier(vtnId, new VnodeName(bname));
            vtconf.apply(service, vtermId, vtermPresent.get(bname));
        }

        // Apply the path map configurations.
        pathMaps.apply(service, name, vtn.getVtnPathMaps());

        // Apply the flow filter configurations.
        inputFilter.apply(service, vtnId, false, vtn.getVtnInputFilter());
    }

    /**
     * Verify the flow timeout configuration.
     *
     * @param value  The value of the flow timeout.
     * @param cfg    The value of the flow timeout configured in this instance.
     * @param def    The default value of the flow timeout.
     */
    private void verifyFlowTimeout(Integer value, Integer cfg, Integer def) {
        if (cfg == null) {
            assertEquals(def, value);
        } else {
            assertEquals(cfg, value);
        }
    }

    /**
     * Remove all the vBridges that are not associated with the configuration
     * in this instance.
     *
     * @param service  The vtn-vbridge RPC service.
     * @param name     The name of the VTN.
     * @param current  The current VTN.
     * @return  A map that contains vBridges present in the target VTN.
     */
    private Map<String, Vbridge> removeUnwanted(
        VtnVbridgeService service, String name, Vtn current) {
        List<Vbridge> vbrList = current.getVbridge();
        Map<String, Vbridge> present = new HashMap<>();
        if (vbrList != null) {
            for (Vbridge vbr: vbrList) {
                String bname = vbr.getName().getValue();
                assertEquals(null, present.put(bname, vbr));
                if (!vBridges.containsKey(bname)) {
                    VBridgeIdentifier vbrId =
                        new VBridgeIdentifier(name, bname);
                    VBridgeConfig.removeVbridge(service, vbrId);
                }
            }
        }

        return present;
    }

    /**
     * Remove all the vTerminals that are not associated with the configuration
     * in this instance.
     *
     * @param service  The vtn-vterminal RPC service.
     * @param name     The name of the VTN.
     * @param current  The current VTN.
     * @return  A map that contains vTerminals present in the target VTN.
     */
    private Map<String, Vterminal> removeUnwanted(
        VtnVterminalService service, String name, Vtn current) {
        List<String> unwanted = new ArrayList<>();
        List<Vterminal> vtermList = current.getVterminal();
        Map<String, Vterminal> present = new HashMap<>();
        if (vtermList != null) {
            for (Vterminal vterm: vtermList) {
                String bname = vterm.getName().getValue();
                assertEquals(null, present.put(bname, vterm));
                if (!vTerminals.containsKey(bname)) {
                    unwanted.add(bname);
                }
            }
        }

        if (!unwanted.isEmpty()) {
            for (String bname: unwanted) {
                VTerminalIdentifier vtermId =
                    new VTerminalIdentifier(name, bname);
                VTerminalConfig.removeVterminal(service, vtermId);
            }
        }

        return present;
    }

    // FlowFilterNode

    /**
     * {@inheritDoc}
     */
    @Override
    public FlowFilterList getInputFilter() {
        return inputFilter;
    }

    /**
     * This method returns the flow filter list for incoming packets because
     * VTN does not have output filter.
     *
     * @return  A {@link FlowFilterList} instance that contains flow filters
     *          for incoming packets.
     */
    @Override
    public FlowFilterList getOutputFilter() {
        return inputFilter;
    }
}
