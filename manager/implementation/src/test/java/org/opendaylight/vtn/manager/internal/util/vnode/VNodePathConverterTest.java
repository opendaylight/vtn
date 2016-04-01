/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifierTest.ALL_TYPES;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link VNodePathConverter}.
 */
public class VNodePathConverterTest extends TestBase {
    /**
     * Test case for VTN identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testVTenantIdentifier() {
        VnodeName[] tnames = {
            new VnodeName("vtn"),
            new VnodeName("vtn_1"),
            new VnodeName("Tenant1"),
        };

        VtnPathMapKey pmapKey = new VtnPathMapKey(1);
        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(1);
        for (VnodeName tname: tnames) {
            InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, new VtnKey(tname)).
                build();
            InstanceIdentifier<VtnPathMaps> pmapsPath =
                vtnPath.child(VtnPathMaps.class);
            InstanceIdentifier<VtnInputFilter> ifilterPath =
                vtnPath.child(VtnInputFilter.class);
            List<InstanceIdentifier<?>> pathList = new ArrayList<>();
            Collections.addAll(
                pathList, vtnPath,
                vtnPath.child(VtenantConfig.class),
                pmapsPath,
                pmapsPath.child(VtnPathMap.class, pmapKey),
                ifilterPath,
                ifilterPath.child(VtnFlowFilter.class, filterKey));
            VTenantIdentifier expected = new VTenantIdentifier(tname);
            check(pathList, expected, VTenantIdentifier.class);
        }
    }

    /**
     * Test case for vBridge identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testVBridgeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};

        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(1);
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                InstanceIdentifier<Vbridge> vbrPath = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, new VbridgeKey(bname)).
                    build();
                InstanceIdentifier<VbridgeInputFilter> ifilterPath =
                    vbrPath.child(VbridgeInputFilter.class);
                InstanceIdentifier<VbridgeOutputFilter> ofilterPath =
                    vbrPath.child(VbridgeOutputFilter.class);
                List<InstanceIdentifier<?>> pathList = new ArrayList<>();
                Collections.addAll(
                    pathList, vbrPath,
                    vbrPath.child(VbridgeConfig.class),
                    ifilterPath,
                    ifilterPath.child(VtnFlowFilter.class, filterKey),
                    ofilterPath,
                    ofilterPath.child(VtnFlowFilter.class, filterKey));
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                check(pathList, expected, VBridgeIdentifier.class);
            }
        }
    }

    /**
     * Test case for vTerminal identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testVTerminalIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vtm"), new VnodeName("vterm_1")};

        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                InstanceIdentifier<Vterminal> vtmPath = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vterminal.class, new VterminalKey(bname)).
                    build();
                List<InstanceIdentifier<?>> pathList = new ArrayList<>();
                Collections.addAll(
                    pathList, vtmPath,
                    vtmPath.child(VterminalConfig.class));
                VTerminalIdentifier expected =
                    new VTerminalIdentifier(tname, bname);
                check(pathList, expected, VTerminalIdentifier.class);
            }
        }
    }

    /**
     * Test case for vBridge interface identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testVBridgeIfIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("if_1")};

        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(1);
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                for (VnodeName iname: inames) {
                    InstanceIdentifier<Vinterface> ifPath = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(Vinterface.class, new VinterfaceKey(iname)).
                        build();
                    InstanceIdentifier<VinterfaceInputFilter> ifilterPath =
                        ifPath.child(VinterfaceInputFilter.class);
                    InstanceIdentifier<VinterfaceOutputFilter> ofilterPath =
                        ifPath.child(VinterfaceOutputFilter.class);
                    List<InstanceIdentifier<?>> pathList = new ArrayList<>();
                    Collections.addAll(
                        pathList, ifPath,
                        ifPath.child(VinterfaceConfig.class),
                        ifilterPath,
                        ifilterPath.child(VtnFlowFilter.class, filterKey),
                        ofilterPath,
                        ofilterPath.child(VtnFlowFilter.class, filterKey));
                    VBridgeIfIdentifier expected =
                        new VBridgeIfIdentifier(tname, bname, iname);
                    check(pathList, expected, VBridgeIfIdentifier.class);
                }
            }
        }
    }

    /**
     * Test case for vTerminal interface identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testVTerminalIfIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vtm"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("if_1")};

        VtnFlowFilterKey filterKey = new VtnFlowFilterKey(1);
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtmKey = new VterminalKey(bname);
                for (VnodeName iname: inames) {
                    InstanceIdentifier<Vinterface> ifPath = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vterminal.class, vtmKey).
                        child(Vinterface.class, new VinterfaceKey(iname)).
                        build();
                    InstanceIdentifier<VinterfaceInputFilter> ifilterPath =
                        ifPath.child(VinterfaceInputFilter.class);
                    InstanceIdentifier<VinterfaceOutputFilter> ofilterPath =
                        ifPath.child(VinterfaceOutputFilter.class);
                    List<InstanceIdentifier<?>> pathList = new ArrayList<>();
                    Collections.addAll(
                        pathList, ifPath,
                        ifPath.child(VinterfaceConfig.class),
                        ifilterPath,
                        ifilterPath.child(VtnFlowFilter.class, filterKey),
                        ofilterPath,
                        ofilterPath.child(VtnFlowFilter.class, filterKey));
                    VTerminalIfIdentifier expected =
                        new VTerminalIfIdentifier(tname, bname, iname);
                    check(pathList, expected, VTerminalIfIdentifier.class);
                }
            }
        }
    }

    /**
     * Test case for VLAN mapping identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testVlanMapIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vtm"), new VnodeName("vterm_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};

        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                for (String mapId: mapIds) {
                    InstanceIdentifier<VlanMap> mapPath = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(VlanMap.class, new VlanMapKey(mapId)).
                        build();
                    List<InstanceIdentifier<?>> pathList = new ArrayList<>();
                    Collections.addAll(
                        pathList, mapPath,
                        mapPath.child(VlanMapConfig.class),
                        mapPath.child(VlanMapStatus.class));
                    VlanMapIdentifier expected =
                        new VlanMapIdentifier(tname, bname, mapId);
                    check(pathList, expected, VlanMapIdentifier.class);
                }
            }
        }
    }

    /**
     * Test case for MAC mapping identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testMacMapIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};

        VlanHostDesc vhd = new VlanHostDesc("a0:b0:c0:d0:e0:f0@0");
        MacAddress addr = new MacAddress("00:11:22:aa:bb:cc");
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                InstanceIdentifier<MacMap> mapPath = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, new VbridgeKey(bname)).
                    child(MacMap.class).
                    build();
                InstanceIdentifier<MacMapConfig> confPath =
                    mapPath.child(MacMapConfig.class);
                InstanceIdentifier<AllowedHosts> allowedPath =
                    confPath.child(AllowedHosts.class);
                InstanceIdentifier<DeniedHosts> deniedPath =
                    confPath.child(DeniedHosts.class);
                InstanceIdentifier<MacMapStatus> statPath =
                    mapPath.child(MacMapStatus.class);
                List<InstanceIdentifier<?>> pathList = new ArrayList<>();
                Collections.addAll(
                    pathList, mapPath, confPath, allowedPath, deniedPath,
                    deniedPath.child(VlanHostDescList.class,
                                     new VlanHostDescListKey(vhd)),
                    statPath,
                    statPath.child(MappedHost.class,
                                   new MappedHostKey(addr)));
                MacMapIdentifier expected = new MacMapIdentifier(tname, bname);
                check(pathList, expected, MacMapIdentifier.class);
            }
        }
    }

    /**
     * Test case for MAC mapped host identifier.
     *
     * <ul>
     *   <li>
     *     {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier()}
     *   </li>
     *   <li>
     *     {@link VNodePathConverter#getIdentifier(Class)}
     *   </li>
     * </ul>
     */
    @Test
    public void testMacMapHostIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
            new MacVlan(0xf0aabbcc9988L, (short)4095),
        };

        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                for (MacVlan host: hosts) {
                    VlanHostDescListKey hostKey =
                        new VlanHostDescListKey(host.getVlanHostDesc());
                    InstanceIdentifier<VlanHostDescList> mapPath =
                        InstanceIdentifier.builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(MacMap.class).
                        child(MacMapConfig.class).
                        child(AllowedHosts.class).
                        child(VlanHostDescList.class, hostKey).
                        build();
                    List<InstanceIdentifier<?>> pathList = Collections.
                        <InstanceIdentifier<?>>singletonList(mapPath);
                    MacMapHostIdentifier expected =
                        new MacMapHostIdentifier(tname, bname, host);
                    check(pathList, expected, MacMapHostIdentifier.class);
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VNodePathConverter#VNodePathConverter(InstanceIdentifier)}.
     */
    @Test
    public void testConstructor() {
        try {
            new VNodePathConverter(null);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Invalid instance identifier: " + null,
                         e.getMessage());
            Throwable cause = e.getCause();
            assertTrue(cause instanceof NullPointerException);
        }

        // Invalid MAC address in MAC mapped host path.
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        VtnKey vtnKey = new VtnKey(new VnodeName("vtn"));
        VbridgeKey vbrKey = new VbridgeKey(new VnodeName("vbridge"));

        Map<String, String> cases = new HashMap<>();
        cases.put("00:00:00:00:00:00", "Zero MAC address");
        cases.put("ff:ff:ff:ff:ff:ff", "Broadcast address");
        cases.put("01:00:00:00:00:00", "Multicast address");
        for (Map.Entry<String, String> entry: cases.entrySet()) {
            String mac = entry.getKey();
            String edesc = entry.getValue();
            VlanHostDesc vhd = new VlanHostDesc(mac + "@0");
            VlanHostDescListKey hostKey = new VlanHostDescListKey(vhd);
            InstanceIdentifier<VlanHostDescList> mapPath =
                InstanceIdentifier.builder(Vtns.class).
                child(Vtn.class, vtnKey).
                child(Vbridge.class, vbrKey).
                child(MacMap.class).
                child(MacMapConfig.class).
                child(AllowedHosts.class).
                child(VlanHostDescList.class, hostKey).
                build();
            try {
                new VNodePathConverter(mapPath);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid instance identifier: " + mapPath,
                             e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);

                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(edesc + " cannot be specified: " + mac,
                             re.getMessage());
                assertEquals(vtag, re.getVtnErrorTag());
            }
        }

        // Invalid VLAN ID in MAC mapped host path.
        long[] badVids = {4096, 4097, 10000, Long.MAX_VALUE};
        for (long vid: badVids) {
            String desc = "00:11:22:33:44:55@" + vid;
            VlanHostDesc vhd = new VlanHostDesc(desc);
            VlanHostDescListKey hostKey = new VlanHostDescListKey(vhd);
            InstanceIdentifier<VlanHostDescList> mapPath =
                InstanceIdentifier.builder(Vtns.class).
                child(Vtn.class, vtnKey).
                child(Vbridge.class, vbrKey).
                child(MacMap.class).
                child(MacMapConfig.class).
                child(AllowedHosts.class).
                child(VlanHostDescList.class, hostKey).
                build();
            try {
                new VNodePathConverter(mapPath);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid instance identifier: " + mapPath,
                             e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);

                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals("Invalid VLAN ID in vlan-host-desc: " + desc,
                             re.getMessage());
                assertEquals(vtag, re.getVtnErrorTag());
            }
        }

        // Unsupported paths.
        List<InstanceIdentifier<?>> badPaths = new ArrayList<>();
        Collections.addAll(
            badPaths,
            InstanceIdentifier.create(Vtns.class),
            InstanceIdentifier.create(GlobalPathMaps.class),
            InstanceIdentifier.builder(GlobalPathMaps.class).child(
                VtnPathMap.class, new VtnPathMapKey(1)).build(),
            InstanceIdentifier.builder(VtnFlowConditions.class).child(
                VtnFlowCondition.class,
                new VtnFlowConditionKey(new VnodeName("cond_1"))).build());
        for (InstanceIdentifier<?> path: badPaths) {
            try {
                new VNodePathConverter(path);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Unsupported instance identifier: " + path,
                             e.getMessage());
            }
        }
    }

    /**
     * Check the conversion.
     *
     * @param pathList  A list of instance identifiers.
     * @param expected  A {@link VNodeIdentifier} instance expected to be
     *                  returned by {@link VNodePathConverter#getIdentifier()}.
     * @param type      Expected type of VNode identifier.
     * @param <T>       The type of expected VNode identifier.
     */
    private <T extends VNodeIdentifier<?>> void check(
        List<InstanceIdentifier<?>> pathList, T expected, Class<T> type) {
        for (InstanceIdentifier<?> path: pathList) {
            VNodePathConverter conv = new VNodePathConverter(path);
            assertEquals(expected, conv.getIdentifier());
            T ident = conv.getIdentifier(type);
            assertEquals(expected, ident);

            for (Class<? extends VNodeIdentifier<?>> cls: ALL_TYPES) {
                if (cls.equals(type)) {
                    continue;
                }

                String msg = "Unexpected VNode identifier type: " + expected;
                try {
                    conv.getIdentifier(cls);
                    unexpected();
                } catch (IllegalStateException e) {
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }
}
