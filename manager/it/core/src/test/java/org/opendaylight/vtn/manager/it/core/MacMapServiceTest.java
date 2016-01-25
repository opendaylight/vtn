/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.core.VTNManagerIT.LOG;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map.Entry;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import com.google.common.collect.ImmutableList;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeConfig;
import org.opendaylight.vtn.manager.it.util.vnode.VBridgeIdentifier;
import org.opendaylight.vtn.manager.it.util.vnode.VTNMacMapConfig;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.GetMacMappedHostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapAclInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.SetMacMapInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * Test case for {@link VtnMacMapService}.
 */
public final class MacMapServiceTest extends TestMethodBase {
    /**
     * A list of host information to be set into denied host set in all
     * MAC mappings.
     */
    private static final List<MacVlan>  DENIED_HOSTS;

    /**
     * A list of invalid MAC addresses.
     */
    private static final List<EtherAddress> INVALID_MAC_ADDRESSES;

    /**
     * A set of hosts mapped by MAC mapping.
     */
    private final Set<MacVlan>  mappedHosts = new HashSet<>();

    /**
     * VLANs mapped by wildcard MAC mapping.
     */
    private final Map<VBridgeIdentifier, Integer>  wildcardMappings =
        new HashMap<>();

    /**
     * Initialize static field.
     */
    static {
        DENIED_HOSTS = ImmutableList.<MacVlan>builder().
            add(new MacVlan(0x001122334455L, 0)).
            add(new MacVlan(0x0aabbccddeefL, 4095)).
            add(new MacVlan(0xa0b0c0d0e0f0L, 10)).
            build();

        INVALID_MAC_ADDRESSES = ImmutableList.<EtherAddress>builder().
            add(EtherAddress.BROADCAST).
            add(new EtherAddress(0x810011223344L)).
            add(new EtherAddress(0L)).
            build();
    }

    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public MacMapServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    /**
     * Create a host information to be set allowed host set in a MAC mapping.
     *
     * @param rand  A pseudo random number generator.
     * @return  A {@link MacVlan} instance.
     */
    private MacVlan newAllowedHost(Random rand) {
        return newAllowedHost(rand, createVlanId(rand));
    }

    /**
     * Create a host information to be set allowed host set in a MAC mapping.
     *
     * @param rand  A pseudo random number generator.
     * @param vid   A VLAN ID for host.
     * @return  A {@link MacVlan} instance.
     */
    private MacVlan newAllowedHost(Random rand, int vid) {
        MacVlan mv;
        do {
            mv = new MacVlan(createEtherAddress(rand), vid);
        } while (DENIED_HOSTS.contains(mv) || !mappedHosts.add(mv));
        return mv;
    }

    /**
     * Ensure that MAC mapping RPCs return NOTFOUND error if the specified
     * vBridge is not present.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrId    The identifier for the target vBridge.
     */
    private void notFoundTest(VtnMacMapService mmapSrv,
                              VBridgeIdentifier vbrId) {
        notFoundTest(mmapSrv, vbrId.getTenantNameString(),
                     vbrId.getBridgeNameString());
    }

    /**
     * Ensure that MAC mapping RPCs return NOTFOUND error if the specified
     * vBridge is not present.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param tname    The name of the VTN.
     * @param bname    The name of the vBridge.
     */
    private void notFoundTest(VtnMacMapService mmapSrv, String tname,
                              String bname) {
        VtnAclType[] aclTypes = VtnAclType.values();
        List<MacAddress> macList = Collections.singletonList(
            new MacAddress("00:aa:bb:cc:dd:ee"));
        for (VtnUpdateOperationType op: VtnUpdateOperationType.values()) {
            SetMacMapInput input = new SetMacMapInputBuilder().
                setTenantName(tname).
                setBridgeName(bname).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMap(input), RpcErrorTag.DATA_MISSING,
                          VtnErrorTag.NOTFOUND);

            GetMacMappedHostInput ginput = new GetMacMappedHostInputBuilder().
                setTenantName(tname).
                setBridgeName(bname).
                setMacAddresses(macList).
                build();
            checkRpcError(mmapSrv.getMacMappedHost(ginput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            for (VtnAclType aclType: aclTypes) {
                SetMacMapAclInput ainput = new SetMacMapAclInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    setOperation(op).
                    setAclType(aclType).
                    build();
                checkRpcError(mmapSrv.setMacMapAcl(ainput),
                              RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
            }
        }
    }

    /**
     * Ensure that no MAC mapping is activated on the specified vBridge.
     *
     * @param mmapSrv     vtn-mac-map service.
     * @param rand        A pseudo random number generator.
     * @param vbrId       The identifier for the target vBridge.
     * @param configured  {@code true} indicates the MAC mapping is configured
     *                    in the specified vBridge.
     */
    private void noMappingTest(VtnMacMapService mmapSrv, Random rand,
                               VBridgeIdentifier vbrId, Boolean configured) {
        VBridgeConfig bconf = getVirtualNetwork().getBridge(vbrId);
        String tname = vbrId.getTenantNameString();
        String bname = vbrId.getBridgeNameString();
        GetMacMappedHostInput input = new GetMacMappedHostInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            build();
        GetMacMappedHostOutput output =
            getRpcOutput(mmapSrv.getMacMappedHost(input));
        assertEquals(null, output.getMacMappedHost());
        assertEquals(configured, output.isConfigured());

        List<MacAddress> macList = Arrays.asList(
            createEtherAddress(rand).getMacAddress(),
            createEtherAddress(rand).getMacAddress(),
            createEtherAddress(rand).getMacAddress());
        input = new GetMacMappedHostInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setMacAddresses(macList).
            build();
        output = getRpcOutput(mmapSrv.getMacMappedHost(input));
        assertEquals(null, output.getMacMappedHost());
        assertEquals(configured, output.isConfigured());
    }

    /**
     * Test case for {@link VtnMacMapService}.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param rand     A pseudo random number generator.
     * @param vbrId    The identifier for the target vBridge.
     * @param wild     A VLAN ID to be mapped by wildcard MAC mapping.
     * @throws Exception  An error occurred.
     */
    private void testMacMapService(VtnMacMapService mmapSrv, Random rand,
                                   VBridgeIdentifier vbrId, int wild)
        throws Exception {
        LOG.debug("testMacMapService: {}", vbrId);

        // Ensure that MAC mapping RPCs return NOTFOUND error if the specified
        // vBridge is not present.
        notFoundTest(mmapSrv, vbrId);

        // Create the target vBridge.
        VirtualNetwork vnet = getVirtualNetwork().
            addBridge(vbrId).
            apply();
        vnet.verify();
        noMappingTest(mmapSrv, rand, vbrId, false);

        // Attempt to configure empty MAC mapping.
        VBridgeConfig bconf = vnet.getBridge(vbrId);
        VTNMacMapConfig mmap = new VTNMacMapConfig();
        assertEquals(null, mmap.update(mmapSrv, vbrId, null));
        vnet.verify();

        // Configure MAC mapping.
        String tname = vbrId.getTenantNameString();
        String bname = vbrId.getBridgeNameString();
        VtnUpdateType utype = VtnUpdateType.CREATED;
        bconf.setMacMap(mmap);
        for (int i = 0; i < 20; i++) {
            MacVlan mv = newAllowedHost(rand);
            VlanHostDesc vhd = mv.getVlanHostDesc();

            // Duplicate host should be ignored.
            List<VlanHostDesc> hostList = Arrays.asList(vhd, vhd, vhd);
            SetMacMapInput input = new SetMacMapInputBuilder().
                setTenantName(tname).
                setBridgeName(bname).
                setAllowedHosts(hostList).
                build();
            assertEquals(utype, getRpcResult(mmapSrv.setMacMap(input)));
            mmap.addAllowed(mv);
            vnet.verify();
            assertEquals(null, getRpcResult(mmapSrv.setMacMap(input)));
            utype = VtnUpdateType.CHANGED;
        }

        MacVlan wildMap1 = new MacVlan(MacVlan.UNDEFINED, wild);
        MacVlan wildMap2 = new MacVlan(MacVlan.UNDEFINED, wild + 100);
        assertEquals(true, mappedHosts.add(wildMap1));
        VTNMacMapConfig mconf = new VTNMacMapConfig().
            addAllowed(wildMap1, wildMap2);
        mmap.addAllowed(wildMap1, wildMap2);
        assertEquals(null, wildcardMappings.put(vbrId, wild));
        VtnUpdateOperationType opType = VtnUpdateOperationType.ADD;
        assertEquals(utype, mconf.update(mmapSrv, vbrId, opType));
        assertEquals(null, mconf.update(mmapSrv, vbrId, opType));
        vnet.verify();

        Set<MacVlan> denied = new HashSet<>(DENIED_HOSTS);
        do {
            MacVlan mv = new MacVlan(
                createEtherAddress(rand), createVlanId(rand));
            denied.add(mv);
        } while (denied.size() < 10);

        // Duplicate host should be ignored.
        List<VlanHostDesc> hostList = new ArrayList<>();
        for (MacVlan mv: denied) {
            VlanHostDesc vhd = mv.getVlanHostDesc();
            hostList.add(vhd);
            hostList.add(vhd);
        }
        SetMacMapInput input = new SetMacMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setDeniedHosts(hostList).
            setOperation(opType).
            build();
        assertEquals(utype, getRpcResult(mmapSrv.setMacMap(input)));
        mmap.setDenied(denied);
        vnet.verify();
        assertEquals(null, getRpcResult(mmapSrv.setMacMap(input)));

        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            assertEquals(null, mmap.update(mmapSrv, vbrId, op));
        }

        // Add 2 more allowed hosts using set-mac-map-acl RPC.
        MacVlan allowed1 = newAllowedHost(rand, 0);
        MacVlan allowed2 = newAllowedHost(rand, 4095);

        // Duplicate host should be ignored.
        hostList = Arrays.asList(
            allowed1.getVlanHostDesc(),
            allowed2.getVlanHostDesc(),
            allowed2.getVlanHostDesc(),
            allowed1.getVlanHostDesc());
        SetMacMapAclInput ainput = new SetMacMapAclInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setHosts(hostList).
            setOperation(opType).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(mmapSrv.setMacMapAcl(ainput)));
        mmap.addAllowed(allowed1, allowed2);
        vnet.verify();
        assertEquals(null, getRpcResult(mmapSrv.setMacMapAcl(ainput)));

        // Add 2 more denied hosts using set-mac-map-acl RPC.
        int count = 0;
        do {
            MacVlan mv = new MacVlan(
                createEtherAddress(rand), createVlanId(rand));
            if (denied.add(mv)) {
                // Duplicate host should be ignored.
                VlanHostDesc vhd = mv.getVlanHostDesc();
                hostList = Arrays.asList(vhd, vhd, vhd);
                ainput = new SetMacMapAclInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    setHosts(hostList).
                    setAclType(VtnAclType.DENY).
                    setOperation(opType).
                    build();
                assertEquals(VtnUpdateType.CHANGED,
                             getRpcResult(mmapSrv.setMacMapAcl(ainput)));
                mmap.addDenied(mv);
                vnet.verify();
                assertEquals(null, getRpcResult(mmapSrv.setMacMapAcl(ainput)));
                count++;
            }
        } while (count < 2);

        // Add more hosts using SET operation.
        mmap.addAllowed(newAllowedHost(rand, 0), newAllowedHost(rand));
        count = denied.size() + 3;
        do {
            MacVlan mv = new MacVlan(
                createEtherAddress(rand), createVlanId(rand));
            if (denied.add(mv)) {
                mmap.addDenied(mv);
            }
        } while (denied.size() < count);

        opType = VtnUpdateOperationType.SET;
        assertEquals(VtnUpdateType.CHANGED,
                     mmap.update(mmapSrv, vbrId, opType));
        vnet.verify();
        assertEquals(null, mmap.update(mmapSrv, vbrId, opType));

        mmap.addAllowed(newAllowedHost(rand, 4095), newAllowedHost(rand));
        assertEquals(VtnUpdateType.CHANGED,
                     mmap.update(mmapSrv, vbrId, VtnAclType.ALLOW, opType));
        vnet.verify();
        assertEquals(null,
                     mmap.update(mmapSrv, vbrId, VtnAclType.ALLOW, opType));

        count = denied.size() + 2;
        do {
            MacVlan mv = new MacVlan(
                createEtherAddress(rand), createVlanId(rand));
            if (denied.add(mv)) {
                mmap.addDenied(mv);
            }
        } while (denied.size() < count);

        assertEquals(VtnUpdateType.CHANGED,
                     mmap.update(mmapSrv, vbrId, VtnAclType.DENY, opType));
        vnet.verify();
        assertEquals(null,
                     mmap.update(mmapSrv, vbrId, VtnAclType.DENY, opType));

        // Remove 2 allowed and denied hosts using set-mac-map RPC.
        count = 0;
        mconf = new VTNMacMapConfig();
        for (MacVlan mv: new HashSet<>(mmap.getAllowed())) {
            if (mv.getAddress() != MacVlan.UNDEFINED) {
                mconf.addAllowed(mv);
                mmap.removeAllowed(mv);
                assertEquals(true, mappedHosts.remove(mv));
                count++;
                if (count >= 2) {
                    break;
                }
            }
        }
        count = 0;
        for (MacVlan mv: new HashSet<>(mmap.getDenied())) {
            mconf.addDenied(mv);
            mmap.removeDenied(mv);
            assertEquals(true, denied.remove(mv));
            count++;
            if (count >= 2) {
                break;
            }
        }

        opType = VtnUpdateOperationType.REMOVE;
        assertEquals(VtnUpdateType.CHANGED,
                     mconf.update(mmapSrv, vbrId, opType));
        vnet.verify();
        assertEquals(null, mconf.update(mmapSrv, vbrId, opType));

        // Remove 2 hosts from denied host set using set-mac-map-acl RPC.
        mconf = new VTNMacMapConfig();
        count = 0;
        for (MacVlan mv: new HashSet<>(mmap.getDenied())) {
            mconf.addDenied(mv);
            mmap.removeDenied(mv);
            assertEquals(true, denied.remove(mv));
            count++;
            if (count >= 2) {
                break;
            }
        }

        assertEquals(VtnUpdateType.CHANGED,
                     mconf.update(mmapSrv, vbrId, VtnAclType.DENY, opType));
        vnet.verify();
        assertEquals(null,
                     mconf.update(mmapSrv, vbrId, VtnAclType.DENY, opType));

        // Remove 1 host and wildcard mapping from allowed host set
        // using set-mac-map-acl RPC.
        mconf = new VTNMacMapConfig();
        count = 0;
        for (MacVlan mv: new HashSet<>(mmap.getAllowed())) {
            if (mv.getAddress() != MacVlan.UNDEFINED) {
                mconf.addAllowed(mv);
                mmap.removeAllowed(mv);
                assertEquals(true, mappedHosts.remove(mv));
                count++;
                if (count >= 1) {
                    break;
                }
            }
        }
        mconf.addAllowed(wildMap1);
        mmap.removeAllowed(wildMap1);

        assertEquals(VtnUpdateType.CHANGED,
                     mconf.update(mmapSrv, vbrId, VtnAclType.ALLOW, opType));
        vnet.verify();
        assertEquals(null,
                     mconf.update(mmapSrv, vbrId, VtnAclType.ALLOW, opType));

        // Install new allowed host set.
        mappedHosts.removeAll(mmap.getAllowed());
        mmap.clearAllowed();
        hostList = new ArrayList<>();
        for (int i = 0; i < 5; i++) {
            MacVlan host = newAllowedHost(rand);
            mmap.addAllowed(host);

            // Duplicate host should be ignored.
            hostList.add(host.getVlanHostDesc());
            hostList.add(host.getVlanHostDesc());
        }

        hostList.add(wildMap1.getVlanHostDesc());
        hostList.add(wildMap2.getVlanHostDesc());
        hostList.add(wildMap1.getVlanHostDesc());
        hostList.add(wildMap2.getVlanHostDesc());
        mmap.addAllowed(wildMap1, wildMap2);

        ainput = new SetMacMapAclInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setHosts(hostList).
            setAclType(VtnAclType.ALLOW).
            setOperation(VtnUpdateOperationType.SET).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(mmapSrv.setMacMapAcl(ainput)));
        vnet.verify();
        assertEquals(null, getRpcResult(mmapSrv.setMacMapAcl(ainput)));

        // Install new denied host set.
        mmap.clearDenied();
        hostList.clear();
        do {
            MacVlan host = new MacVlan(
                createEtherAddress(rand), createVlanId(rand));
            mmap.addDenied(host);

            // Duplicate host should be ignored.
            hostList.add(host.getVlanHostDesc());
            hostList.add(host.getVlanHostDesc());
        } while (mmap.getDenied().size() < 3);

        ainput = new SetMacMapAclInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setHosts(hostList).
            setAclType(VtnAclType.DENY).
            setOperation(VtnUpdateOperationType.SET).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(mmapSrv.setMacMapAcl(ainput)));
        vnet.verify();
        assertEquals(null, getRpcResult(mmapSrv.setMacMapAcl(ainput)));

        // Install new configuration.
        List<VlanHostDesc> allowedList = new ArrayList<>();
        List<VlanHostDesc> deniedList = new ArrayList<>();
        mappedHosts.removeAll(mmap.getAllowed());
        mmap.clearAllowed().addAllowed(wildMap1);
        allowedList.add(wildMap1.getVlanHostDesc());
        mmap.clearDenied();
        for (int i = 0; i < 25; i++) {
            MacVlan host = newAllowedHost(rand);
            mmap.addAllowed(host);

            // Duplicate host should be ignored.
            allowedList.add(host.getVlanHostDesc());
            allowedList.add(host.getVlanHostDesc());
        }

        for (MacVlan mv: DENIED_HOSTS) {
            mmap.addDenied(mv);
            deniedList.add(mv.getVlanHostDesc());
        }
        do {
            MacVlan host = new MacVlan(
                createEtherAddress(rand), createVlanId(rand));
            mmap.addDenied(host);

            // Duplicate host should be ignored.
            deniedList.add(host.getVlanHostDesc());
            deniedList.add(host.getVlanHostDesc());
        } while (mmap.getDenied().size() < 15);

        input = new SetMacMapInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setAllowedHosts(allowedList).
            setDeniedHosts(deniedList).
            setOperation(VtnUpdateOperationType.SET).
            build();
        assertEquals(VtnUpdateType.CHANGED,
                     getRpcResult(mmapSrv.setMacMap(input)));
        vnet.verify();
        assertEquals(null, getRpcResult(mmapSrv.setMacMap(input)));

        // Invalid MAC address.
        VtnUpdateOperationType[] ops = VtnUpdateOperationType.values();
        VtnAclType[] aclTypes = VtnAclType.values();
        for (EtherAddress eaddr: INVALID_MAC_ADDRESSES) {
            VlanHostDesc vhdesc = new VlanHostDesc(eaddr.getText() + "@0");
            hostList = Collections.singletonList(vhdesc);
            for (VtnUpdateOperationType op: ops) {
                input = new SetMacMapInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    setAllowedHosts(hostList).
                    setOperation(op).
                    build();
                checkRpcError(mmapSrv.setMacMap(input),
                              RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

                input = new SetMacMapInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    setDeniedHosts(hostList).
                    setOperation(op).
                    build();
                checkRpcError(mmapSrv.setMacMap(input),
                              RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

                for (VtnAclType aclType: aclTypes) {
                    ainput = new SetMacMapAclInputBuilder().
                        setTenantName(tname).
                        setBridgeName(bname).
                        setHosts(hostList).
                        setAclType(aclType).
                        setOperation(op).
                        build();
                    checkRpcError(mmapSrv.setMacMapAcl(ainput),
                                  RpcErrorTag.BAD_ELEMENT,
                                  VtnErrorTag.BADREQUEST);
                }
            }
        }

        // Null vlan-host-desc.
        List<VlanHostDesc> nullDesc =
            Collections.singletonList((VlanHostDesc)null);
        for (VtnUpdateOperationType op: ops) {
            input = new SetMacMapInputBuilder().
                setTenantName(tname).
                setBridgeName(bname).
                setAllowedHosts(nullDesc).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMap(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            input = new SetMacMapInputBuilder().
                setTenantName(tname).
                setBridgeName(bname).
                setDeniedHosts(nullDesc).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMap(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

            for (VtnAclType aclType: aclTypes) {
                ainput = new SetMacMapAclInputBuilder().
                    setTenantName(tname).
                    setBridgeName(bname).
                    setHosts(nullDesc).
                    setAclType(aclType).
                    setOperation(op).
                    build();
                checkRpcError(mmapSrv.setMacMapAcl(ainput),
                              RpcErrorTag.MISSING_ELEMENT,
                              VtnErrorTag.BADREQUEST);
            }
        }

        EtherAddress dupAddr1;
        do {
            dupAddr1 = createEtherAddress(rand);
        } while (mappedHosts.contains(dupAddr1));
        EtherAddress dupAddr2 = new EtherAddress(
            (dupAddr1.getAddress() + 1) & ~EtherAddress.MASK_MULTICAST);
        VTNMacMapConfig dupConf = new VTNMacMapConfig().addAllowed(
            new MacVlan(dupAddr1, 0),
            new MacVlan(dupAddr2, 1),
            new MacVlan(dupAddr1, 2));
        List<MacVlan> anotherMaps = getAnotherMappings(vbrId, mmap);

        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            // No MAC address in denied host set.
            mconf = new VTNMacMapConfig().addDenied(wildMap1);
            input = mconf.newInputBuilder(vbrId).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            ainput = mconf.newAclInputBuilder(vbrId, VtnAclType.DENY).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMapAcl(ainput),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            // Duplicate MAC address in the allowed host set.
            input = dupConf.newInputBuilder(vbrId).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMap(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            ainput = dupConf.newAclInputBuilder(vbrId, VtnAclType.ALLOW).
                setOperation(op).
                build();
            checkRpcError(mmapSrv.setMacMapAcl(ainput),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

            // Try to map host that is already mapped to another vBridge.
            for (MacVlan mv: anotherMaps) {
                mconf = new VTNMacMapConfig().addAllowed(mv);
                input = mconf.newInputBuilder(vbrId).
                    setOperation(op).
                    build();
                checkRpcError(mmapSrv.setMacMap(input),
                              RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);

                ainput = mconf.newAclInputBuilder(vbrId, VtnAclType.ALLOW).
                    setOperation(op).
                    build();
                checkRpcError(mmapSrv.setMacMapAcl(ainput),
                              RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);
            }
        }

        // Try to MAC address that is already mapped to this vBridge.
        MacVlan mapped = null;
        for (MacVlan mv: mmap.getAllowed()) {
            if (mv.getAddress() != MacVlan.UNDEFINED) {
                mapped = mv;
                break;
            }
        }
        assertNotNull(mapped);
        MacVlan conflict = new MacVlan(
            mapped.getAddress(), (mapped.getVlanId() + 1) & MASK_VLAN_ID);

        mconf = new VTNMacMapConfig().addAllowed(conflict);
        input = mconf.newInputBuilder(vbrId).
            setOperation(VtnUpdateOperationType.ADD).
            build();
        checkRpcError(mmapSrv.setMacMap(input),
                      RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);

        ainput = mconf.newAclInputBuilder(vbrId, VtnAclType.ALLOW).
            setOperation(VtnUpdateOperationType.ADD).
            build();
        checkRpcError(mmapSrv.setMacMapAcl(ainput),
                      RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);

        mconf = new VTNMacMapConfig().clearAllowed().
            addAllowed(mapped, conflict);

        input = mconf.newInputBuilder(vbrId).
            setOperation(VtnUpdateOperationType.SET).
            build();
        checkRpcError(mmapSrv.setMacMap(input),
                      RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);

        ainput = mconf.newAclInputBuilder(vbrId, VtnAclType.ALLOW).
            setOperation(VtnUpdateOperationType.SET).
            build();
        checkRpcError(mmapSrv.setMacMapAcl(ainput),
                      RpcErrorTag.DATA_EXISTS, VtnErrorTag.CONFLICT);

        // Null MAC address in get-mac-mapped-host RPC.
        GetMacMappedHostInput ginput = new GetMacMappedHostInputBuilder().
            setTenantName(tname).
            setBridgeName(bname).
            setMacAddresses(Collections.singletonList((MacAddress)null)).
            build();
        checkRpcError(mmapSrv.getMacMappedHost(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Errors should never affect existing MAC mappings.
        vnet.verify();
        noMappingTest(mmapSrv, rand, vbrId, true);

        // Both ADD and REMOVE operations should do nothing if no host is
        // specified in the RPC input.
        mconf = new VTNMacMapConfig();
        ops = new VtnUpdateOperationType[]{
            VtnUpdateOperationType.ADD,
            VtnUpdateOperationType.REMOVE,
        };
        boolean[] bools = {true, false};
        for (VtnUpdateOperationType op: ops) {
            for (boolean empty: bools) {
                input = mconf.newInputBuilder(vbrId, empty).
                    setOperation(op).
                    build();
                assertEquals(null, getRpcResult(mmapSrv.setMacMap(input)));

                for (VtnAclType aclType: aclTypes) {
                    ainput = mconf.newAclInputBuilder(vbrId, aclType, empty).
                        setOperation(op).
                        build();
                    assertEquals(null,
                                 getRpcResult(mmapSrv.setMacMapAcl(ainput)));
                }
            }
        }
        vnet.verify();
        noMappingTest(mmapSrv, rand, vbrId, true);
    }

    /**
     * Set MAC mapping configurations using set-mac-map RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param macMaps  A map that keeps MAC mapping configurations to be set.
     * @param op       A {@link VtnUpdateOperationType} instance.
     */
    private void restore(VtnMacMapService mmapSrv,
                         Map<VBridgeIdentifier, VTNMacMapConfig> macMaps,
                         VtnUpdateOperationType op) {
        restore(mmapSrv, macMaps, op, VtnUpdateType.CREATED);
    }

    /**
     * Set MAC mapping configurations using set-mac-map RPC.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param macMaps  A map that keeps MAC mapping configurations to be set.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @param utype    The expected update type.
     */
    private void restore(VtnMacMapService mmapSrv,
                         Map<VBridgeIdentifier, VTNMacMapConfig> macMaps,
                         VtnUpdateOperationType op, VtnUpdateType utype) {
        VirtualNetwork vnet = getVirtualNetwork();
        for (Entry<VBridgeIdentifier, VTNMacMapConfig> entry:
                 macMaps.entrySet()) {
            VBridgeIdentifier vbrId = entry.getKey();
            VTNMacMapConfig mmap = entry.getValue();
            assertEquals(utype, mmap.update(mmapSrv, vbrId, op));
            VBridgeConfig bconf = vnet.getBridge(vbrId);
            bconf.setMacMap(mmap.clone());
            vnet.verify();
            assertEquals(null, mmap.update(mmapSrv, vbrId, op));
        }
    }

    /**
     * Clear the specified access control list in all the MAC mappings.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrIds   A list of vBridge identifiers.
     * @param aclType  The type of the access control list.
     */
    private void clearAcl(VtnMacMapService mmapSrv,
                          List<VBridgeIdentifier> vbrIds, VtnAclType aclType) {
        VirtualNetwork vnet = getVirtualNetwork();
        for (VBridgeIdentifier vbrId: vbrIds) {
            VTNMacMapConfig mmap = vnet.getBridge(vbrId).getMacMap();
            VtnUpdateType utype;
            mmap.clear(aclType);
            if (mmap.isEmpty()) {
                utype = VtnUpdateType.REMOVED;
                vnet.getBridge(vbrId).setMacMap(null);
            } else {
                utype = VtnUpdateType.CHANGED;
            }
            assertEquals(utype, removeMacMapAcl(vbrId, aclType));
            vnet.verify();
            assertEquals(null, removeMacMapAcl(vbrId, aclType));
        }
    }

    /**
     * Clear the specified access control list in all the MAC mappings
     * using REMOVE operation.
     *
     * @param mmapSrv  vtn-mac-map service.
     * @param vbrIds   A list of vBridge identifiers.
     * @param aclType  The type of the access control list.
     */
    private void removeAcl(VtnMacMapService mmapSrv,
                           List<VBridgeIdentifier> vbrIds, VtnAclType aclType) {
        VirtualNetwork vnet = getVirtualNetwork();
        for (VBridgeIdentifier vbrId: vbrIds) {
            VTNMacMapConfig mmap = vnet.getBridge(vbrId).getMacMap();
            SetMacMapAclInput input = mmap.newAclInputBuilder(vbrId, aclType).
                setOperation(VtnUpdateOperationType.REMOVE).
                build();
            VtnUpdateType utype;
            mmap.clear(aclType);
            if (mmap.isEmpty()) {
                utype = VtnUpdateType.REMOVED;
                vnet.getBridge(vbrId).setMacMap(null);
            } else {
                utype = VtnUpdateType.CHANGED;
            }
            assertEquals(utype, getRpcResult(mmapSrv.setMacMapAcl(input)));
            vnet.verify();
            assertEquals(null, getRpcResult(mmapSrv.setMacMapAcl(input)));
        }
    }

    /**
     * Create a list of layer 2 hosts that are mapped to another vBridge.
     *
     * @param vbrId  The identifier for the vBridge.
     * @param mmap   The current MAC mapping configuration.
     * @return  A list of {@link MacVlan} instances.
     */
    private List<MacVlan> getAnotherMappings(VBridgeIdentifier vbrId,
                                             VTNMacMapConfig mmap) {
        List<MacVlan> list = new ArrayList<>();
        Set<MacVlan> allowed = mmap.getAllowed();
        for (MacVlan mv: mappedHosts) {
            if (!allowed.contains(mv)) {
                list.add(mv);
                break;
            }
        }

        for (Entry<VBridgeIdentifier, Integer> entry:
                 wildcardMappings.entrySet()) {
            if (!vbrId.equals(entry.getKey())) {
                Integer vid = entry.getValue();
                list.add(new MacVlan(MacVlan.UNDEFINED, vid.intValue()));
                break;
            }
        }

        return list;
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        VTNManagerIT vit = getTest();
        VtnMacMapService mmapSrv = vit.getMacMapService();
        Random rand = new Random(0xaabbccddeeff0011L);
        mappedHosts.clear();
        wildcardMappings.clear();

        // Configure MAC mappings into 3 vBridges.
        String tname1 = "vtn_1";
        String tname2 = "vtn_2";
        String bname1 = "vbr_1";
        String bname2 = "vbr_2";
        List<VBridgeIdentifier> vbrIds = Arrays.asList(
            new VBridgeIdentifier(tname1, bname1),
            new VBridgeIdentifier(tname1, bname2),
            new VBridgeIdentifier(tname2, bname1));
        int wild = 0;
        for (VBridgeIdentifier vbrId: vbrIds) {
            testMacMapService(mmapSrv, rand, vbrId, wild);
            wild++;
        }

        // Error tests.

        // Null input.
        checkRpcError(mmapSrv.setMacMap(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(mmapSrv.setMacMapAcl(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(mmapSrv.getMacMappedHost(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null tenant-name.
        SetMacMapInput input = new SetMacMapInputBuilder().build();
        checkRpcError(mmapSrv.setMacMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        input = new SetMacMapInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(mmapSrv.setMacMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        SetMacMapAclInput ainput = new SetMacMapAclInputBuilder().build();
        checkRpcError(mmapSrv.setMacMapAcl(ainput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        ainput = new SetMacMapAclInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(mmapSrv.setMacMapAcl(ainput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        GetMacMappedHostInput ginput = new GetMacMappedHostInputBuilder().
            build();
        checkRpcError(mmapSrv.getMacMappedHost(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        ginput = new GetMacMappedHostInputBuilder().
            setBridgeName(bname1).
            build();
        checkRpcError(mmapSrv.getMacMappedHost(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null bridge-name.
        input = new SetMacMapInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(mmapSrv.setMacMap(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        ainput = new SetMacMapAclInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(mmapSrv.setMacMapAcl(ainput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        ginput = new GetMacMappedHostInputBuilder().
            setTenantName(tname1).
            build();
        checkRpcError(mmapSrv.getMacMappedHost(ginput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        for (String name: INVALID_VNODE_NAMES) {
            // Invalid tenant-name.
            notFoundTest(mmapSrv, name, bname1);

            // Invalid bridge-name.
            notFoundTest(mmapSrv, tname1, name);
        }

        // Errors should never affect existing MAC mappings.
        VirtualNetwork vnet = getVirtualNetwork().verify();

        // Remove all the MAC mappings.
        Map<VBridgeIdentifier, VTNMacMapConfig> macMaps = new HashMap<>();
        for (VBridgeIdentifier vbrId: vbrIds) {
            VBridgeConfig bconf = vnet.getBridge(vbrId);
            VTNMacMapConfig mmap = bconf.getMacMap();
            assertEquals(null, macMaps.put(vbrId, mmap.clone()));
            assertEquals(VtnUpdateType.REMOVED, removeMacMap(vbrId));
            bconf.setMacMap(null);
            vnet.verify();
            assertEquals(null, removeMacMap(vbrId));
        }

        // Restore all the MAC mappings using ADD operation.
        restore(mmapSrv, macMaps, VtnUpdateOperationType.ADD);

        // Clear allowed host set in all the MAC mappings, and then
        // clear denied host set in order.
        clearAcl(mmapSrv, vbrIds, VtnAclType.ALLOW);
        clearAcl(mmapSrv, vbrIds, VtnAclType.DENY);

        // Restore all the MAC mappings using SET operation.
        restore(mmapSrv, macMaps, VtnUpdateOperationType.SET);

        // Clear denied host set in all the MAC mappings, and then
        // clear allowed host set in order.
        clearAcl(mmapSrv, vbrIds, VtnAclType.DENY);
        clearAcl(mmapSrv, vbrIds, VtnAclType.ALLOW);

        // Restore all the MAC mappings using default operation (ADD).
        restore(mmapSrv, macMaps, null);

        // Remove all the MAC mappings using REMOVE operation.
        VtnUpdateOperationType opType = VtnUpdateOperationType.REMOVE;
        for (VBridgeIdentifier vbrId: vbrIds) {
            VBridgeConfig bconf = vnet.getBridge(vbrId);
            VTNMacMapConfig mmap = bconf.getMacMap();
            assertEquals(VtnUpdateType.REMOVED,
                         mmap.update(mmapSrv, vbrId, opType));
            bconf.setMacMap(null);
            vnet.verify();
            assertEquals(null, mmap.update(mmapSrv, vbrId, opType));
        }

        // Restore all the MAC mappings.
        restore(mmapSrv, macMaps, VtnUpdateOperationType.ADD);

        // Clear allowed host set in all the MAC mappings using REMOVE
        // operation, and then clear denied host set in order.
        removeAcl(mmapSrv, vbrIds, VtnAclType.ALLOW);
        removeAcl(mmapSrv, vbrIds, VtnAclType.DENY);

        // Restore all the MAC mappings.
        restore(mmapSrv, macMaps, VtnUpdateOperationType.SET);

        // Clear denied host set in all the MAC mappings using REMOVE
        // operation, and then clear allowed host set in order.
        removeAcl(mmapSrv, vbrIds, VtnAclType.DENY);
        removeAcl(mmapSrv, vbrIds, VtnAclType.ALLOW);

        // Restore only the denied host set using ADD operation.
        Map<VBridgeIdentifier, VTNMacMapConfig> deniedMaps = new HashMap<>();
        for (Entry<VBridgeIdentifier, VTNMacMapConfig> entry:
                 macMaps.entrySet()) {
            VBridgeIdentifier vbrId = entry.getKey();
            VTNMacMapConfig mmap = entry.getValue().clone().clearAllowed();
            assertEquals(null, deniedMaps.put(vbrId, mmap));
        }
        restore(mmapSrv, deniedMaps, VtnUpdateOperationType.ADD);

        // Restore the allowed host set using ADD operation.
        restore(mmapSrv, macMaps, VtnUpdateOperationType.ADD,
                VtnUpdateType.CHANGED);

        // Remove VTNs.
        String[] tenants = {tname1, tname2};
        for (String tname: tenants) {
            removeVtn(tname);
            vnet.removeTenant(tname).verify();
        }
    }
}
