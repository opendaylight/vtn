/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.opendaylight.vtn.manager.it.util.VTNServices;
import org.opendaylight.vtn.manager.it.util.flow.filter.FlowFilterList;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeUpdateMode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.RemoveVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.UpdateVbridgeInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;

/**
 * {@code VBridgeConfig} describes the configuration of a vBridge.
 */
public final class VBridgeConfig extends BridgeConfig<VBridgeConfig, Vbridge>
    implements FlowFilterNode {
    /**
     * The default value of age-interval.
     */
    public static final Integer  DEFAULT_AGE_INTERVAL = 600;

    /**
     * The value of age-interval.
     */
    private Integer  ageInterval;

    /**
     * VLAN mappings configured in this vBridge.
     */
    private final Map<String, VTNVlanMapConfig>  vlanMaps = new HashMap<>();

    /**
     * The configuration of the MAC mapping.
     */
    private VTNMacMapConfig  macMap;

    /**
     * A list of input flow filter.
     */
    private final FlowFilterList  inputFilter = new FlowFilterList();

    /**
     * A list of output flow filter.
     */
    private final FlowFilterList  outputFilter = new FlowFilterList();

    /**
     * Remove the specified vBridge.
     *
     * @param service  The vtn-vbridge RPC service.
     * @param ident    The identifier for the vBridge.
     */
    public static void removeVbridge(
        VtnVbridgeService service, VBridgeIdentifier ident) {
        RemoveVbridgeInput input = new RemoveVbridgeInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            build();

        assertEquals(null, getRpcOutput(service.removeVbridge(input), true));
    }

    /**
     * Construct a new instance with default values.
     */
    public VBridgeConfig() {
    }

    /**
     * Construct a new instance.
     *
     * @param desc  The description about the vBridge.
     * @param age   The value of age-interval.
     */
    public VBridgeConfig(String desc, Integer age) {
        super(desc);
        ageInterval = age;
    }

    /**
     * Return the value of age-interval.
     *
     * @return  The value of age-interval.
     */
    public Integer getAgeInterval() {
        return ageInterval;
    }

    /**
     * Set the value of age-interval.
     *
     * @param age  The value of age-interval.
     * @return  This instance.
     */
    public VBridgeConfig setAgeInterval(Integer age) {
        ageInterval = age;
        return this;
    }

    /**
     * Add the given VLAN mapping configuration.
     *
     * @param vmconf  The VLAN mapping configuration.
     * @return  This instance.
     */
    public VBridgeConfig addVlanMap(VTNVlanMapConfig vmconf) {
        vlanMaps.put(vmconf.getMapId(), vmconf);
        return this;
    }

    /**
     * Remove the specified VLAN mapping configuration.
     *
     * @param vmconf  The VLAN mapping configuration.
     * @return  This instance.
     */
    public VBridgeConfig removeVlanMap(VTNVlanMapConfig vmconf) {
        return removeVlanMap(vmconf.getMapId());
    }

    /**
     * Remove the VLAN mapping configuration specified by the mapping ID.
     *
     * @param id  The identifier for the VLAN mapping.
     * @return  This instance.
     */
    public VBridgeConfig removeVlanMap(String id) {
        vlanMaps.remove(id);
        return this;
    }

    /**
     * Remove all the VLAN mapping configurations in this vBridge
     * configuration.
     *
     * @return  This instance.
     */
    public VBridgeConfig clearVlanMap() {
        vlanMaps.clear();
        return this;
    }

    /**
     * Return the configuration of the VLAN mapping specified by the mapping
     * ID.
     *
     * @param id  The identifier for the VLAN mapping.
     * @return  A {@link VTNVlanMapConfig} instance if found.
     *          {@code null} if not found.
     */
    public VTNVlanMapConfig getVlanMap(String id) {
        return vlanMaps.get(id);
    }

    /**
     * Return the MAC mapping configuration.
     *
     * @return  A {@link VTNMacMapConfig} instance if the MAC mapping is
     *          configured. {@code null} if not configured.
     */
    public VTNMacMapConfig getMacMap() {
        return macMap;
    }

    /**
     * Set the MAC mapping configuration.
     *
     * @param mmconf  A {@link VTNMacMapConfig} instance.
     *                The MAC mapping is removed if {@code null} is specified.
     * @return  This instance.
     */
    public VBridgeConfig setMacMap(VTNMacMapConfig mmconf) {
        macMap = mmconf;
        return this;
    }

    /**
     * Create a new input builder for update-vbridge RPC.
     *
     * @return  An {@link UpdateVbridgeInputBuilder} instance.
     */
    public UpdateVbridgeInputBuilder newInputBuilder() {
        return new UpdateVbridgeInputBuilder().
            setDescription(getDescription()).
            setAgeInterval(ageInterval);
    }

    /**
     * Add random configuration to this vBridge configuration using the given
     * random generator.
     *
     * @param rand  A pseudo random generator.
     * @return  This instance.
     */
    public VBridgeConfig add(Random rand) {
        int count = rand.nextInt(RANDOM_ADD_MAX);
        for (int i = 0; i <= count; i++) {
            addInterface(createVnodeName("if_", rand), new VInterfaceConfig());
        }

        inputFilter.add(rand);
        outputFilter.add(rand);
        return this;
    }

    /**
     * Update the specified vBridge.
     *
     * @param service  The vtn-vbridge service.
     * @param ident    The identifier for the vBridge.
     * @param mode     A {@link VnodeUpdateMode} instance.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnVbridgeService service,
                                VBridgeIdentifier ident,
                                VnodeUpdateMode mode,
                                VtnUpdateOperationType op) {
        UpdateVbridgeInput input = newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            setUpdateMode(mode).
            setOperation(op).
            build();
        return getRpcResult(service.updateVbridge(input));
    }

    /**
     * Verify the given vBridge.
     *
     * @param rtx    A read-only MD-SAL datastore transaction.
     * @param ident  The identifier for the vBridge.
     * @param vbr    The vBridge to be verified.
     */
    public void verify(ReadTransaction rtx, VBridgeIdentifier ident,
                       Vbridge vbr) {
        // Verify the configuration.
        VbridgeConfig vbrc = vbr.getVbridgeConfig();
        assertEquals(getDescription(), vbrc.getDescription());
        Integer age = (ageInterval == null)
            ? DEFAULT_AGE_INTERVAL : ageInterval;
        assertEquals(age, vbrc.getAgeInterval());

        // Verify the VLAN mappings.
        VnodeState bstate = VnodeState.UNKNOWN;
        List<VlanMap> vlmaps = vbr.getVlanMap();
        if (!vlanMaps.isEmpty()) {
            assertNotNull(vlmaps);
            Set<String> checked = new HashSet<>();
            for (VlanMap vlmap: vlmaps) {
                String mapId = vlmap.getMapId();
                VTNVlanMapConfig vmconf = vlanMaps.get(mapId);
                assertNotNull(vmconf);
                VnodeState state = vmconf.verify(rtx, vlmap);
                assertEquals(true, checked.add(mapId));

                if (bstate != VnodeState.DOWN) {
                    bstate = state;
                }
            }
            assertEquals(checked, vlanMaps.keySet());
        } else if (vlmaps != null) {
            assertEquals(Collections.<VlanMap>emptyList(), vlmaps);
        }

        // Verify the MAC mapping.
        MacMap mmap = vbr.getMacMap();
        if (macMap == null) {
            assertEquals(null, mmap);
        } else {
            assertNotNull(mmap);
            VnodeState state = macMap.verify(mmap);
            if (bstate != VnodeState.DOWN) {
                bstate = state;
            }
        }

        // Verify the virtual interfaces and the bridge status.
        verify(rtx, ident, vbr, bstate);

        // Verify the flow filters.
        inputFilter.verify(vbr.getVbridgeInputFilter());
        outputFilter.verify(vbr.getVbridgeOutputFilter());
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param ident    The identifier for the vBridge.
     * @param current  The current vBridge.
     */
    public void apply(VTNServices service, VBridgeIdentifier ident,
                      Vbridge current) {
        // Create the vBridge if not present.
        UpdateVbridgeInput input = newInputBuilder().
            setTenantName(ident.getTenantNameString()).
            setBridgeName(ident.getBridgeNameString()).
            setUpdateMode(VnodeUpdateMode.UPDATE).
            setOperation(VtnUpdateOperationType.SET).
            build();
        getRpcOutput(service.getVbridgeService().updateVbridge(input));

        Vbridge vbr = (current == null)
            ? new VbridgeBuilder().build()
            : current;

        // Apply the virtual interface configuration.
        applyInterfaces(service, ident, vbr);

        // Apply the MAC mapping configuration.
        if (macMap == null) {
            VtnUpdateType expecetd = (vbr.getMacMap() == null)
                ? null
                : VtnUpdateType.REMOVED;
            VtnUpdateType result = VTNMacMapConfig.removeMacMap(
                service.getMacMapService(), ident);
            assertEquals(expecetd, result);
        } else {
            macMap.apply(service, ident);
        }

        // Apply the VLAN mapping configuration.
        Map<String, VlanMap> present =
            removeUnwanted(service.getVlanMapService(), ident, vbr);
        for (VTNVlanMapConfig vmconf: vlanMaps.values()) {
            if (!present.containsKey(vmconf.getMapId())) {
                vmconf.apply(service, ident);
            }
        }

        // Apply the flow filter configurations.
        inputFilter.apply(service, ident, false, vbr.getVbridgeInputFilter());
        outputFilter.apply(service, ident, true, vbr.getVbridgeOutputFilter());
    }

    /**
     * Remove all the VLAN mappings that are not associated with the
     * configuration in this instance.
     *
     * @param service  The vtn-vlan-map RPC service.
     * @param ident    The identifier for the vBridge.
     * @param vbr      The current vBridge.
     * @return  A map that contains VLAN mappings present in the target
     *          vBridge.
     */
    private Map<String, VlanMap> removeUnwanted(
        VtnVlanMapService service, VBridgeIdentifier ident, Vbridge vbr) {
        Map<String, VlanMap> present = new HashMap<>();
        List<String> unwanted = new ArrayList<>();
        List<VlanMap> vlmaps = vbr.getVlanMap();
        if (vlmaps != null) {
            for (VlanMap vlmap: vlmaps) {
                String mapId = vlmap.getMapId();
                assertEquals(null, present.put(mapId, vlmap));
                if (!vlanMaps.containsKey(mapId)) {
                    unwanted.add(mapId);
                }
            }
        }

        if (!unwanted.isEmpty()) {
            Map<String, VtnUpdateType> result = (vlanMaps.isEmpty())
                ? VTNVlanMapConfig.removeVlanMap(service, ident)
                : VTNVlanMapConfig.removeVlanMap(service, ident, unwanted);
            assertEquals(unwanted.size(), result.size());
            for (String mapId: unwanted) {
                assertEquals(VtnUpdateType.REMOVED, result.get(mapId));
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
     * {@inheritDoc}
     */
    @Override
    public FlowFilterList getOutputFilter() {
        return outputFilter;
    }
}
