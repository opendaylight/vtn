/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.NavigableSet;
import java.util.Set;
import java.util.TreeSet;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlanTest;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHostsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHostsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnAclType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;

/**
 * JUnit test for {@link VTNMacMapConfig}.
 */
public class VTNMacMapConfigTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNMacMapConfig} class.
     */
    private static final String  XML_ROOT = "vtn-mac-map-config";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNMacMapConfig} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        String[] allowedPath = XmlDataType.addPath(
            "allowed-hosts", XmlDataType.addPath(name, parent));
        String[] deniedPath = XmlDataType.addPath(
            "denied-hosts", XmlDataType.addPath(name, parent));

        dlist.addAll(MacVlanTest.getXmlDataTypes("host", allowedPath));
        dlist.addAll(MacVlanTest.getXmlDataTypes("host", deniedPath));
        return dlist;
    }

    /**
     * Test case for
     * {@link VTNMacMapConfig#getDuplicateMacAddressException(long)}.
     */
    @Test
    public void testGetDuplicateMacAddressException() {
        EtherAddress[] eaddrs = {
            new EtherAddress("00:00:00:00:00:01"),
            new EtherAddress("00:AA:BB:CC:DD:EE"),
            new EtherAddress("12:34:56:78:9a:BC"),
        };

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        for (EtherAddress eaddr: eaddrs) {
            String msg = "Duplicate MAC address in allowed set: " +
                eaddr.getText();
            RpcException e = VTNMacMapConfig.
                getDuplicateMacAddressException(eaddr.getAddress());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VTNMacMapConfig#getAlreadyMappedException(long)}.
     */
    @Test
    public void testGetAlreadyMappedException() {
        EtherAddress[] eaddrs = {
            new EtherAddress("00:00:00:00:00:01"),
            new EtherAddress("00:AA:BB:CC:DD:EE"),
            new EtherAddress("12:34:56:78:9a:BC"),
        };

        RpcErrorTag etag = RpcErrorTag.DATA_EXISTS;
        VtnErrorTag vtag = VtnErrorTag.CONFLICT;
        for (EtherAddress eaddr: eaddrs) {
            String msg = "Already mapped to this vBridge: " + eaddr.getText();
            RpcException e = VTNMacMapConfig.
                getAlreadyMappedException(eaddr.getAddress());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VTNMacMapConfig#verifyDeniedHost(VlanHostDesc)} and
     * {@link VTNMacMapConfig#verifyDeniedHost(MacVlan)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testVerifyDeniedHost() throws Exception {
        EtherAddress[] eaddrs = {
            new EtherAddress("00:00:00:00:00:01"),
            new EtherAddress("00:AA:BB:CC:DD:EE"),
            new EtherAddress("12:34:56:78:9a:BC"),
        };
        int[] vids = {0, 1, 234, 1024, 3000, 4094, 4095};

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;

        for (int vid: vids) {
            for (EtherAddress eaddr: eaddrs) {
                String desc = eaddr.getText() + "@" + vid;
                MacVlan mv = new MacVlan(eaddr.getAddress(), vid);
                VlanHostDesc vhd = new VlanHostDesc(desc);
                assertEquals(mv, VTNMacMapConfig.verifyDeniedHost(vhd));
                assertSame(mv, VTNMacMapConfig.verifyDeniedHost(mv));
            }

            // MAC address must be specified in the denied host set.
            String msg =
                "MAC address cannot be null in denied host set: vlan=" + vid;
            VlanHostDesc vhd = new VlanHostDesc("@" + vid);
            try {
                VTNMacMapConfig.verifyDeniedHost(vhd);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            MacVlan mv = new MacVlan((byte[])null, vid);
            try {
                VTNMacMapConfig.verifyDeniedHost(mv);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for
     * {@link VTNMacMapConfig#containsMacAddress(NavigableSet, long)}.
     */
    @Test
    public void testContainsMacAddress() {
        EtherAddress[] eaddrs = {
            new EtherAddress("00:00:00:00:00:00"),
            new EtherAddress("00:00:00:00:00:01"),
            new EtherAddress("00:AA:BB:CC:DD:EE"),
            new EtherAddress("12:34:56:78:9a:BC"),
            new EtherAddress("ff:ff:ff:ff:ff:ff"),
        };
        EtherAddress[] eaddrs1 = {
            new EtherAddress("00:00:00:00:00:02"),
            new EtherAddress("00:AA:BB:CC:DD:EF"),
            new EtherAddress("01:23:45:67:90:ab"),
            new EtherAddress("ff:ff:ff:ff:ff:fe"),
        };
        int[] vids = {0, 1, 234, 1024, 3000, 4094, 4095};

        for (int vid: vids) {
            NavigableSet<MacVlan> set = new TreeSet<>();
            for (EtherAddress eaddr: eaddrs) {
                long mac = eaddr.getAddress();
                assertFalse(VTNMacMapConfig.containsMacAddress(set, mac));
                MacVlan mv = new MacVlan(eaddr.getAddress(), vid);
                assertTrue(set.add(mv));
                assertTrue(VTNMacMapConfig.containsMacAddress(set, mac));
            }

            for (EtherAddress eaddr: eaddrs) {
                long mac = eaddr.getAddress();
                assertTrue(VTNMacMapConfig.containsMacAddress(set, mac));
            }
            for (EtherAddress eaddr: eaddrs1) {
                long mac = eaddr.getAddress();
                assertFalse(VTNMacMapConfig.containsMacAddress(set, mac));
            }
        }
    }

    /**
     * Test case for {@link VTNMacMapConfig#VTNMacMapConfig()}.
     */
    @Test
    public void testConstructor1() {
        VTNMacMapConfig mmc = new VTNMacMapConfig();
        assertTrue(mmc.getAllowedHosts().isEmpty());
        assertTrue(mmc.getDeniedHosts().isEmpty());
        assertTrue(mmc.isEmpty());
    }

    /**
     * Test case for {@link VTNMacMapConfig#VTNMacMapConfig(VtnMacMapConfig)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor2() throws Exception {
        Set<MacVlan> hostSet = Collections.<MacVlan>emptySet();
        Map<AllowedHosts, Set<MacVlan>> allowedCases = new HashMap<>();
        Map<DeniedHosts, Set<MacVlan>> deniedCases = new HashMap<>();
        allowedCases.put(null, hostSet);
        deniedCases.put(null, hostSet);

        AllowedHosts allowed = new AllowedHostsBuilder().build();
        DeniedHosts denied = new DeniedHostsBuilder().build();
        allowedCases.put(allowed, hostSet);
        deniedCases.put(denied, hostSet);

        List<VlanHostDescList> vhdescs = new ArrayList<>();
        allowed = new AllowedHostsBuilder().
            setVlanHostDescList(vhdescs).build();
        denied = new DeniedHostsBuilder().
            setVlanHostDescList(vhdescs).build();
        allowedCases.put(allowed, hostSet);
        deniedCases.put(denied, hostSet);

        MacVlan[][] allowedHosts = {
            new MacVlan[] {new MacVlan(0x001122334455L, (short)0)},
            new MacVlan[] {
                new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
                new MacVlan(0xf8ffff998877L, (short)1),
                new MacVlan(0x3abcdef12345L, (short)100),
            },
            new MacVlan[] {
                new MacVlan(MacVlan.UNDEFINED, (short)123),
                new MacVlan(MacVlan.UNDEFINED, (short)456),
                new MacVlan(0x00abcdef0000L, (short)3),
                new MacVlan(0x00abcdef0001L, (short)4),
                new MacVlan(0x00abcdef0002L, (short)5),
                new MacVlan(0x00abcdef0003L, (short)6),
            },
        };

        MacVlan[][] deniedHosts = {
            new MacVlan[] {new MacVlan(0x003344556677L, (short)10)},
            new MacVlan[] {
                new MacVlan(0xa876543210ffL, (short)999),
                new MacVlan(0xa00000000001L, (short)1000),
            },
            new MacVlan[] {
                new MacVlan(0x000000000011L, (short)0),
                new MacVlan(0x000000000012L, (short)0),
                new MacVlan(0x000000000013L, (short)0),
                new MacVlan(0xa00000000013L, (short)0),
                new MacVlan(0xa00000000014L, (short)0),
            },
        };

        for (MacVlan[] ahs: allowedHosts) {
            vhdescs = new ArrayList<>();
            hostSet = new HashSet<>();
            for (MacVlan mv: ahs) {
                vhdescs.add(mv.getVlanHostDescList());
                hostSet.add(mv);
            }
            allowed = new AllowedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            allowedCases.put(allowed, hostSet);
        }

        for (MacVlan[] dhs: deniedHosts) {
            vhdescs = new ArrayList<>();
            hostSet = new HashSet<>();
            for (MacVlan mv: dhs) {
                vhdescs.add(mv.getVlanHostDescList());
                hostSet.add(mv);
            }
            denied = new DeniedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            deniedCases.put(denied, hostSet);
        }

        for (Map.Entry<AllowedHosts, Set<MacVlan>> entry:
                 allowedCases.entrySet()) {
            allowed = entry.getKey();
            Set<MacVlan> allowedSet = entry.getValue();
            for (Map.Entry<DeniedHosts, Set<MacVlan>> entry1:
                     deniedCases.entrySet()) {
                denied = entry1.getKey();
                Set<MacVlan> deniedSet = entry1.getValue();
                VtnMacMapConfig vmmc = new MacMapConfigBuilder().
                    setAllowedHosts(allowed).
                    setDeniedHosts(denied).
                    build();

                VTNMacMapConfig mmc = new VTNMacMapConfig(vmmc);
                assertEquals(allowedSet, mmc.getAllowedHosts());
                assertEquals(deniedSet, mmc.getDeniedHosts());
                assertEquals(allowedSet.isEmpty() && deniedSet.isEmpty(),
                             mmc.isEmpty());
            }
        }
    }

    /**
     * Test case for {@link VTNMacMapConfig#toMacMapConfig()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToMacMapConfig() throws Exception {
        List<AllowedHosts> allowedCases = new ArrayList<>();
        List<DeniedHosts> deniedCases = new ArrayList<>();
        allowedCases.add(null);
        deniedCases.add(null);

        AllowedHosts allowed = new AllowedHostsBuilder().build();
        DeniedHosts denied = new DeniedHostsBuilder().build();
        allowedCases.add(allowed);
        deniedCases.add(denied);

        List<VlanHostDescList> vhdescs = new ArrayList<>();
        allowed = new AllowedHostsBuilder().
            setVlanHostDescList(vhdescs).build();
        denied = new DeniedHostsBuilder().
            setVlanHostDescList(vhdescs).build();
        allowedCases.add(allowed);
        deniedCases.add(denied);

        MacVlan[][] allowedHosts = {
            new MacVlan[] {new MacVlan(0x001122334455L, (short)0)},
            new MacVlan[] {
                new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
                new MacVlan(0xf8ffff998877L, (short)1),
                new MacVlan(0x3abcdef12345L, (short)100),
            },
            new MacVlan[] {
                new MacVlan(MacVlan.UNDEFINED, (short)123),
                new MacVlan(MacVlan.UNDEFINED, (short)456),
                new MacVlan(0x00abcdef0000L, (short)3),
                new MacVlan(0x00abcdef0001L, (short)4),
                new MacVlan(0x00abcdef0002L, (short)5),
                new MacVlan(0x00abcdef0003L, (short)6),
            },
        };

        MacVlan[][] deniedHosts = {
            new MacVlan[] {new MacVlan(0x003344556677L, (short)10)},
            new MacVlan[] {
                new MacVlan(0xa876543210ffL, (short)999),
                new MacVlan(0xa00000000001L, (short)1000),
            },
            new MacVlan[] {
                new MacVlan(0x000000000011L, (short)0),
                new MacVlan(0x000000000012L, (short)0),
                new MacVlan(0x000000000013L, (short)0),
                new MacVlan(0xa00000000013L, (short)0),
                new MacVlan(0xa00000000014L, (short)0),
            },
        };

        for (MacVlan[] ahs: allowedHosts) {
            vhdescs = new ArrayList<>();
            for (MacVlan mv: ahs) {
                vhdescs.add(mv.getVlanHostDescList());
            }
            allowed = new AllowedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            allowedCases.add(allowed);
        }

        for (MacVlan[] dhs: deniedHosts) {
            vhdescs = new ArrayList<>();
            for (MacVlan mv: dhs) {
                vhdescs.add(mv.getVlanHostDescList());
            }
            denied = new DeniedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            deniedCases.add(denied);
        }

        for (AllowedHosts ah: allowedCases) {
            AllowedHosts exAllowed = null;
            if (ah != null) {
                List<VlanHostDescList> hosts = ah.getVlanHostDescList();
                if (hosts != null && !hosts.isEmpty()) {
                    exAllowed = ah;
                }
            }

            for (DeniedHosts dh: deniedCases) {
                DeniedHosts exDenied = null;
                if (dh != null) {
                    List<VlanHostDescList> hosts = dh.getVlanHostDescList();
                    if (hosts != null && !hosts.isEmpty()) {
                        exDenied = dh;
                    }
                }

                MacMapConfig mconf = new MacMapConfigBuilder().
                    setAllowedHosts(ah).
                    setDeniedHosts(dh).
                    build();

                VTNMacMapConfig mmc = new VTNMacMapConfig(mconf);
                MacMapConfig expected = new MacMapConfigBuilder().
                    setAllowedHosts(exAllowed).
                    setDeniedHosts(exDenied).
                    build();
                assertEqualsMacMapConfig(expected, mmc.toMacMapConfig());
            }
        }
    }

    /**
     * Test case for
     * {@link VTNMacMapConfig#update(VtnUpdateOperationType, List, List)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdate() throws Exception {
        VTNMacMapConfig mmc = new VTNMacMapConfig();
        Set<MacVlan> allowedSet = new HashSet<>();
        Set<MacVlan> deniedSet = new HashSet<>();

        // Remove hosts from empty configuration.
        List<VlanHostDesc> allowed = new ArrayList<>();
        List<VlanHostDesc> denied = new ArrayList<>();
        Collections.addAll(
            allowed,
            new VlanHostDesc("00:aa:bb:cc:dd:ee@0"),
            new VlanHostDesc("10:20:30:a0:b0:c0@1000"),
            new VlanHostDesc("a0:b0:c0:d0:e0:f0@123"),
            new VlanHostDesc("@4095"));
        Collections.addAll(
            denied,
            new VlanHostDesc("f0:f1:f2:f3:f4:f5@3"),
            new VlanHostDesc("12:34:56:78:aa:bb@128"));
        VtnUpdateOperationType op = VtnUpdateOperationType.REMOVE;
        assertEquals(null, mmc.update(op, allowed, denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(true, mmc.isEmpty());

        // Specify empty configuration.
        VtnUpdateOperationType[] ops = {
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.REMOVE,
            VtnUpdateOperationType.SET,
        };
        List<VlanHostDesc> nullDesc = null;
        List<VlanHostDesc> emptyDesc = Collections.<VlanHostDesc>emptyList();
        List<List<VlanHostDesc>> emptyHosts = new ArrayList<>();
        Collections.addAll(emptyHosts, nullDesc, emptyDesc);

        for (VtnUpdateOperationType opType: ops) {
            for (List<VlanHostDesc> alist: emptyHosts) {
                for (List<VlanHostDesc> dlist: emptyHosts) {
                    assertEquals(null, mmc.update(opType, alist, dlist));
                    assertEquals(allowedSet, mmc.getAllowedHosts());
                    assertEquals(deniedSet, mmc.getDeniedHosts());
                    assertEquals(true, mmc.isEmpty());
                }
            }
        }

        // Add hosts to empty configuration.
        Set<MacVlan> allowAdded = toHostSet(allowed);
        Set<MacVlan> denyAdded = toHostSet(denied);
        Set<MacVlan> allowRemoved = new HashSet<>();
        Set<MacVlan> denyRemoved = new HashSet<>();
        allowedSet.addAll(allowAdded);
        deniedSet.addAll(denyAdded);

        // Duplicate should be ignored.
        op = VtnUpdateOperationType.ADD;
        MacMapChange change =
            mmc.update(op, duplicate(allowed), duplicate(denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Add the same configuration.
        op = null;
        assertEquals(null, mmc.update(op, allowed, denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        // Add/Remove empty configuration.
        ops = new VtnUpdateOperationType[]{
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.REMOVE,
        };
        for (VtnUpdateOperationType opType: ops) {
            for (List<VlanHostDesc> alist: emptyHosts) {
                for (List<VlanHostDesc> dlist: emptyHosts) {
                    assertEquals(null, mmc.update(opType, alist, dlist));
                    assertEquals(allowedSet, mmc.getAllowedHosts());
                    assertEquals(deniedSet, mmc.getDeniedHosts());
                    assertEquals(false, mmc.isEmpty());
                }
            }
        }

        // Change configuration by SET operation.
        allowed.clear();
        Collections.addAll(
            allowed,
            new VlanHostDesc("00:93:fa:bf:31:96@1234"),
            new VlanHostDesc("00:aa:bb:cc:dd:ee@1"),
            new VlanHostDesc("10:20:30:a0:b0:c0@1000"),
            new VlanHostDesc("@1000"),
            new VlanHostDesc("@4094"),
            new VlanHostDesc("@4095"));
        allowAdded.clear();
        Collections.addAll(
            allowAdded,
            new MacVlan(0x0093fabf3196L, 1234),
            new MacVlan(0x00aabbccddeeL, 1),
            new MacVlan(MacVlan.UNDEFINED, 1000),
            new MacVlan(MacVlan.UNDEFINED, 4094));
        allowRemoved.clear();
        Collections.addAll(
            allowRemoved,
            new MacVlan(0x00aabbccddeeL, 0),
            new MacVlan(0xa0b0c0d0e0f0L, 123));
        allowedSet = toHostSet(allowed);

        denied.clear();
        Collections.addAll(
            denied,
            new VlanHostDesc("00:a3:98:3c:d1:96@3"),
            new VlanHostDesc("00:00:00:11:22:33@4"),
            new VlanHostDesc("00:00:00:11:22:33@5"),
            new VlanHostDesc("12:34:56:78:aa:bb@128"));
        denyAdded.clear();
        Collections.addAll(
            denyAdded,
            new MacVlan(0x00a3983cd196L, 3),
            new MacVlan(0x000000112233L, 4),
            new MacVlan(0x000000112233L, 5));
        denyRemoved.clear();
        Collections.addAll(
            denyRemoved,
            new MacVlan(0xf0f1f2f3f4f5L, 3));
        deniedSet = toHostSet(denied);

        // Duplicate should be ignored.
        op = VtnUpdateOperationType.SET;
        change = mmc.update(op, duplicate(allowed), duplicate(denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Set the same configuration.
        assertEquals(null, mmc.update(op, allowed, denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        // Remove the configuration by SET operation.
        allowAdded.clear();
        allowRemoved.clear();
        allowRemoved.addAll(allowedSet);
        denyAdded.clear();
        denyRemoved.clear();
        denyRemoved.addAll(deniedSet);
        change = mmc.update(op, nullDesc, nullDesc);
        assertEquals(true, mmc.getAllowedHosts().isEmpty());
        assertEquals(true, mmc.getDeniedHosts().isEmpty());
        assertEquals(true, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(true, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Restore the configuration by SET operation.
        allowAdded.clear();
        allowAdded.addAll(allowedSet);
        allowRemoved.clear();
        denyAdded.clear();
        denyAdded.addAll(deniedSet);
        denyRemoved.clear();
        change = mmc.update(op, allowed, denied);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Set null host.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vlan-host-desc cannot be null";
        VlanHostDesc vhd = null;
        List<VlanHostDesc> hosts = Collections.singletonList(vhd);
        ops = new VtnUpdateOperationType[]{
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.REMOVE,
            VtnUpdateOperationType.SET,
        };
        for (VtnUpdateOperationType opType: ops) {
            try {
                mmc.update(opType, hosts, nullDesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            try {
                mmc.update(opType, nullDesc, hosts);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        Map<VlanHostDesc, String> cases = new HashMap<>();
        etag = RpcErrorTag.BAD_ELEMENT;

        // Broadcast address.
        EtherAddress eaddr = EtherAddress.BROADCAST;
        MacVlan mvlan = new MacVlan(eaddr.getAddress(), 1);
        vhd = mvlan.getVlanHostDesc();
        msg = "Broadcast address cannot be specified: " + eaddr.getText();
        assertEquals(null, cases.put(vhd, msg));

        // Multicast address.
        eaddr = new EtherAddress(0x0123456789abL);
        mvlan = new MacVlan(eaddr.getAddress(), 1);
        vhd = mvlan.getVlanHostDesc();
        msg = "Multicast address cannot be specified: " + eaddr.getText();
        assertEquals(null, cases.put(vhd, msg));

        // Zero MAC address.
        eaddr = new EtherAddress(0L);
        vhd = new VlanHostDesc("00:00:00:00:00:00@1");
        msg = "Zero MAC address cannot be specified: " + eaddr.getText();
        assertEquals(null, cases.put(vhd, msg));

        for (Map.Entry<VlanHostDesc, String> entry: cases.entrySet()) {
            hosts = Collections.singletonList(entry.getKey());
            msg = entry.getValue();
            for (VtnUpdateOperationType opType: ops) {
                try {
                    mmc.update(opType, hosts, emptyDesc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }

                try {
                    mmc.update(opType, emptyDesc, hosts);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            }
        }

        // Null MAC address in the denied host set.
        mvlan = new MacVlan(MacVlan.UNDEFINED, 1);
        vhd = mvlan.getVlanHostDesc();
        hosts = Collections.singletonList(vhd);
        ops = new VtnUpdateOperationType[]{
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.SET,
        };
        msg = "MAC address cannot be null in denied host set: vlan=1";

        for (VtnUpdateOperationType opType: ops) {
            try {
                mmc.update(opType, nullDesc, hosts);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Restore the configuration by SET operation.
        op = VtnUpdateOperationType.SET;
        mmc.update(op, allowed, denied);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        // Duplicate MAC address in the given allowed host set.
        eaddr = new EtherAddress(0x0091cf66da91L);
        String mac = eaddr.getText();
        hosts = Arrays.asList(new VlanHostDesc(mac + "@0"),
                              new VlanHostDesc(mac + "@4095"));
        msg = "Duplicate MAC address in allowed set: " + mac;
        for (VtnUpdateOperationType opType: ops) {
            try {
                mmc.update(opType, hosts, nullDesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Try to map the same MAC address with different VLAN ID.
        eaddr = new EtherAddress(0x102030a0b0c0L);
        mvlan = new MacVlan(eaddr.getAddress(), 0);
        vhd = mvlan.getVlanHostDesc();
        hosts = Collections.singletonList(vhd);
        msg = "Already mapped to this vBridge: " + eaddr.getText();
        etag = RpcErrorTag.DATA_EXISTS;
        vtag = VtnErrorTag.CONFLICT;
        op = VtnUpdateOperationType.ADD;
        try {
            mmc.update(op, hosts, emptyDesc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        op = VtnUpdateOperationType.SET;
        hosts = Arrays.asList(vhd, new VlanHostDesc("10:20:30:a0:b0:c0@1000"));
        try {
            mmc.update(op, hosts, emptyDesc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Remove the configuration by REMOVE operation.
        allowedSet = new HashSet<>(mmc.getAllowedHosts());
        deniedSet = new HashSet<>(mmc.getDeniedHosts());
        op = VtnUpdateOperationType.REMOVE;
        allowAdded = Collections.<MacVlan>emptySet();
        denyAdded = allowAdded;
        denyRemoved = allowAdded;
        for (Iterator<MacVlan> it = allowedSet.iterator(); it.hasNext();) {
            mvlan = it.next();
            allowRemoved = Collections.singleton(mvlan);
            it.remove();
            hosts = Collections.singletonList(mvlan.getVlanHostDesc());
            change = mmc.update(op, hosts, emptyDesc);
            assertEquals(allowedSet, mmc.getAllowedHosts());
            assertEquals(deniedSet, mmc.getDeniedHosts());
            assertEquals(false, mmc.isEmpty());
            assertEquals(allowAdded, change.getAllowAddedSet());
            assertEquals(allowRemoved, change.getAllowRemovedSet());
            assertEquals(denyAdded, change.getDenyAddedSet());
            assertEquals(denyRemoved, change.getDenyRemovedSet());
            assertEquals(false, change.isRemoving());
            assertEquals(false, change.dontPurge());
        }

        allowRemoved = denyAdded;
        for (Iterator<MacVlan> it = deniedSet.iterator(); it.hasNext();) {
            mvlan = it.next();
            denyRemoved = Collections.singleton(mvlan);
            it.remove();
            hosts = Collections.singletonList(mvlan.getVlanHostDesc());
            boolean removing = deniedSet.isEmpty();

            change = mmc.update(op, emptyDesc, hosts);
            assertEquals(allowedSet, mmc.getAllowedHosts());
            assertEquals(deniedSet, mmc.getDeniedHosts());
            assertEquals(removing, mmc.isEmpty());
            assertEquals(allowAdded, change.getAllowAddedSet());
            assertEquals(allowRemoved, change.getAllowRemovedSet());
            assertEquals(denyAdded, change.getDenyAddedSet());
            assertEquals(denyRemoved, change.getDenyRemovedSet());
            assertEquals(removing, change.isRemoving());
            assertEquals(false, change.dontPurge());
        }
    }

    /**
     * Test case for
     * {@link VTNMacMapConfig#update(VtnUpdateOperationType, VtnAclType, List)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testUpdateAcl() throws Exception {
        VTNMacMapConfig mmc = new VTNMacMapConfig();
        Set<MacVlan> allowedSet = new HashSet<>();
        Set<MacVlan> deniedSet = new HashSet<>();

        // Remove hosts from empty configuration.
        List<VlanHostDesc> nullDesc = null;
        List<VlanHostDesc> allowed = new ArrayList<>();
        List<VlanHostDesc> denied = new ArrayList<>();
        Collections.addAll(
            allowed,
            new VlanHostDesc("00:aa:bb:cc:dd:ee@0"),
            new VlanHostDesc("10:20:30:a0:b0:c0@1000"),
            new VlanHostDesc("a0:b0:c0:d0:e0:f0@123"),
            new VlanHostDesc("@4095"));
        Collections.addAll(
            denied,
            new VlanHostDesc("f0:f1:f2:f3:f4:f5@3"),
            new VlanHostDesc("12:34:56:78:aa:bb@128"));
        VtnUpdateOperationType op = VtnUpdateOperationType.REMOVE;
        VtnAclType acl = null;
        assertEquals(null, mmc.update(op, acl, allowed));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(true, mmc.isEmpty());

        acl = VtnAclType.ALLOW;
        assertEquals(null, mmc.update(op, acl, allowed));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(true, mmc.isEmpty());

        acl = VtnAclType.DENY;
        assertEquals(null, mmc.update(op, acl, denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(true, mmc.isEmpty());

        // Specify empty configuration.
        VtnUpdateOperationType[] ops = {
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.REMOVE,
            VtnUpdateOperationType.SET,
        };
        VtnAclType[] acls = {null, VtnAclType.ALLOW, VtnAclType.DENY};
        List<VlanHostDesc> emptyDesc = Collections.<VlanHostDesc>emptyList();
        List<List<VlanHostDesc>> emptyHosts = new ArrayList<>();
        Collections.addAll(emptyHosts, nullDesc, emptyDesc);

        for (VtnUpdateOperationType opType: ops) {
            for (VtnAclType aclType: acls) {
                for (List<VlanHostDesc> list: emptyHosts) {
                    assertEquals(null, mmc.update(opType, aclType, list));
                    assertEquals(allowedSet, mmc.getAllowedHosts());
                    assertEquals(deniedSet, mmc.getDeniedHosts());
                    assertEquals(true, mmc.isEmpty());
                }
            }
        }

        // Add hosts to empty configuration.
        Set<MacVlan> allowAdded = Collections.<MacVlan>emptySet();
        Set<MacVlan> denyAdded = toHostSet(denied);
        Set<MacVlan> allowRemoved = new HashSet<>();
        Set<MacVlan> denyRemoved = new HashSet<>();
        deniedSet.addAll(denyAdded);

        // Duplicate should be ignored.
        op = VtnUpdateOperationType.ADD;
        acl = VtnAclType.DENY;
        MacMapChange change = mmc.update(op, acl, duplicate(denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        allowAdded = toHostSet(allowed);
        allowedSet.addAll(allowAdded);
        denyAdded.clear();
        acl = VtnAclType.ALLOW;
        change = mmc.update(op, acl, duplicate(allowed));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Add the same configuration.
        op = null;
        assertEquals(null, mmc.update(op, acl, allowed));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        acl = VtnAclType.DENY;
        assertEquals(null, mmc.update(op, acl, denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        // Add/Remove empty configuration.
        ops = new VtnUpdateOperationType[]{
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.REMOVE,
        };
        for (VtnUpdateOperationType opType: ops) {
            for (VtnAclType aclType: acls) {
                for (List<VlanHostDesc> list: emptyHosts) {
                    assertEquals(null, mmc.update(opType, aclType, list));
                    assertEquals(allowedSet, mmc.getAllowedHosts());
                    assertEquals(deniedSet, mmc.getDeniedHosts());
                    assertEquals(false, mmc.isEmpty());
                }
            }
        }

        // Change configuration by SET operation.
        allowed.clear();
        Collections.addAll(
            allowed,
            new VlanHostDesc("00:93:fa:bf:31:96@1234"),
            new VlanHostDesc("00:aa:bb:cc:dd:ee@1"),
            new VlanHostDesc("10:20:30:a0:b0:c0@1000"),
            new VlanHostDesc("@1000"),
            new VlanHostDesc("@4094"),
            new VlanHostDesc("@4095"));
        allowAdded.clear();
        Collections.addAll(
            allowAdded,
            new MacVlan(0x0093fabf3196L, 1234),
            new MacVlan(0x00aabbccddeeL, 1),
            new MacVlan(MacVlan.UNDEFINED, 1000),
            new MacVlan(MacVlan.UNDEFINED, 4094));
        allowRemoved.clear();
        Collections.addAll(
            allowRemoved,
            new MacVlan(0x00aabbccddeeL, 0),
            new MacVlan(0xa0b0c0d0e0f0L, 123));
        allowedSet = toHostSet(allowed);
        denyAdded.clear();
        denyRemoved.clear();

        op = VtnUpdateOperationType.SET;
        acl = VtnAclType.ALLOW;
        change = mmc.update(op, acl, allowed);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        denied.clear();
        Collections.addAll(
            denied,
            new VlanHostDesc("00:a3:98:3c:d1:96@3"),
            new VlanHostDesc("00:00:00:11:22:33@4"),
            new VlanHostDesc("00:00:00:11:22:33@5"),
            new VlanHostDesc("12:34:56:78:aa:bb@128"));
        Collections.addAll(
            denyAdded,
            new MacVlan(0x00a3983cd196L, 3),
            new MacVlan(0x000000112233L, 4),
            new MacVlan(0x000000112233L, 5));
        Collections.addAll(
            denyRemoved,
            new MacVlan(0xf0f1f2f3f4f5L, 3));
        deniedSet = toHostSet(denied);
        allowAdded.clear();
        allowRemoved.clear();

        acl = VtnAclType.DENY;
        change = mmc.update(op, acl, denied);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Set the same configuration.
        acl = VtnAclType.ALLOW;
        assertEquals(null, mmc.update(op, acl, allowed));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        acl = VtnAclType.DENY;
        assertEquals(null, mmc.update(op, acl, denied));
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        // Remove the configuration by SET operation.
        allowAdded.clear();
        allowRemoved.clear();
        allowRemoved.addAll(allowedSet);
        denyAdded.clear();
        denyRemoved.clear();
        acl = VtnAclType.ALLOW;
        change = mmc.update(op, acl, nullDesc);
        assertEquals(true, mmc.getAllowedHosts().isEmpty());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        allowRemoved.clear();
        denyRemoved.addAll(deniedSet);
        acl = VtnAclType.DENY;
        change = mmc.update(op, acl, nullDesc);
        assertEquals(true, mmc.getAllowedHosts().isEmpty());
        assertEquals(true, mmc.getDeniedHosts().isEmpty());
        assertEquals(true, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(true, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Restore the configuration by SET operation.
        allowAdded.clear();
        allowAdded.addAll(allowedSet);
        allowRemoved.clear();
        denyAdded.clear();
        denyRemoved.clear();
        acl = VtnAclType.ALLOW;
        change = mmc.update(op, acl, allowed);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(true, mmc.getDeniedHosts().isEmpty());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        allowAdded.clear();
        denyAdded.addAll(deniedSet);
        acl = VtnAclType.DENY;
        change = mmc.update(op, acl, denied);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());
        assertEquals(allowAdded, change.getAllowAddedSet());
        assertEquals(allowRemoved, change.getAllowRemovedSet());
        assertEquals(denyAdded, change.getDenyAddedSet());
        assertEquals(denyRemoved, change.getDenyRemovedSet());
        assertEquals(false, change.isRemoving());
        assertEquals(false, change.dontPurge());

        // Set null host.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vlan-host-desc cannot be null";
        VlanHostDesc vhd = null;
        List<VlanHostDesc> hosts = Collections.singletonList(vhd);
        ops = new VtnUpdateOperationType[]{
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.REMOVE,
            VtnUpdateOperationType.SET,
        };
        for (VtnUpdateOperationType opType: ops) {
            for (VtnAclType aclType: acls) {
                try {
                    mmc.update(opType, aclType, hosts);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            }
        }

        Map<VlanHostDesc, String> cases = new HashMap<>();
        etag = RpcErrorTag.BAD_ELEMENT;

        // Broadcast address.
        EtherAddress eaddr = EtherAddress.BROADCAST;
        MacVlan mvlan = new MacVlan(eaddr.getAddress(), 1);
        vhd = mvlan.getVlanHostDesc();
        msg = "Broadcast address cannot be specified: " + eaddr.getText();
        assertEquals(null, cases.put(vhd, msg));

        // Multicast address.
        eaddr = new EtherAddress(0x0123456789abL);
        mvlan = new MacVlan(eaddr.getAddress(), 1);
        vhd = mvlan.getVlanHostDesc();
        msg = "Multicast address cannot be specified: " + eaddr.getText();
        assertEquals(null, cases.put(vhd, msg));

        // Zero MAC address.
        eaddr = new EtherAddress(0L);
        vhd = new VlanHostDesc("00:00:00:00:00:00@1");
        msg = "Zero MAC address cannot be specified: " + eaddr.getText();
        assertEquals(null, cases.put(vhd, msg));

        for (Map.Entry<VlanHostDesc, String> entry: cases.entrySet()) {
            hosts = Collections.singletonList(entry.getKey());
            msg = entry.getValue();
            for (VtnUpdateOperationType opType: ops) {
                for (VtnAclType aclType: acls) {
                    try {
                        mmc.update(opType, aclType, hosts);
                        unexpected();
                    } catch (RpcException e) {
                        assertEquals(etag, e.getErrorTag());
                        assertEquals(vtag, e.getVtnErrorTag());
                        assertEquals(msg, e.getMessage());
                    }
                }
            }
        }

        // Null MAC address in the denied host set.
        mvlan = new MacVlan(MacVlan.UNDEFINED, 1);
        vhd = mvlan.getVlanHostDesc();
        hosts = Collections.singletonList(vhd);
        ops = new VtnUpdateOperationType[]{
            null, VtnUpdateOperationType.ADD, VtnUpdateOperationType.SET,
        };
        msg = "MAC address cannot be null in denied host set: vlan=1";

        acl = VtnAclType.DENY;
        for (VtnUpdateOperationType opType: ops) {
            try {
                mmc.update(opType, acl, hosts);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Restore the configuration by SET operation.
        op = VtnUpdateOperationType.SET;
        mmc.update(op, VtnAclType.ALLOW, allowed);
        mmc.update(op, VtnAclType.DENY, denied);
        assertEquals(allowedSet, mmc.getAllowedHosts());
        assertEquals(deniedSet, mmc.getDeniedHosts());
        assertEquals(false, mmc.isEmpty());

        // Duplicate MAC address in the given allowed host set.
        eaddr = new EtherAddress(0x0091cf66da91L);
        String mac = eaddr.getText();
        hosts = Arrays.asList(new VlanHostDesc(mac + "@0"),
                              new VlanHostDesc(mac + "@4095"));
        msg = "Duplicate MAC address in allowed set: " + mac;
        acl = VtnAclType.ALLOW;
        for (VtnUpdateOperationType opType: ops) {
            try {
                mmc.update(opType, acl, hosts);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Try to map the same MAC address with different VLAN ID.
        eaddr = new EtherAddress(0x102030a0b0c0L);
        mvlan = new MacVlan(eaddr.getAddress(), 0);
        vhd = mvlan.getVlanHostDesc();
        hosts = Collections.singletonList(vhd);
        msg = "Already mapped to this vBridge: " + eaddr.getText();
        etag = RpcErrorTag.DATA_EXISTS;
        vtag = VtnErrorTag.CONFLICT;
        op = VtnUpdateOperationType.ADD;
        try {
            mmc.update(op, acl, hosts);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        op = VtnUpdateOperationType.SET;
        hosts = Arrays.asList(vhd, new VlanHostDesc("10:20:30:a0:b0:c0@1000"));
        try {
            mmc.update(op, acl, hosts);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Remove the configuration by REMOVE operation.
        allowedSet = new HashSet<>(mmc.getAllowedHosts());
        deniedSet = new HashSet<>(mmc.getDeniedHosts());
        op = VtnUpdateOperationType.REMOVE;
        acl = VtnAclType.DENY;
        allowAdded = Collections.<MacVlan>emptySet();
        allowRemoved = allowAdded;
        denyAdded = allowAdded;
        for (Iterator<MacVlan> it = deniedSet.iterator(); it.hasNext();) {
            mvlan = it.next();
            denyRemoved = Collections.singleton(mvlan);
            it.remove();
            hosts = Collections.singletonList(mvlan.getVlanHostDesc());
            change = mmc.update(op, acl, hosts);
            assertEquals(allowedSet, mmc.getAllowedHosts());
            assertEquals(deniedSet, mmc.getDeniedHosts());
            assertEquals(false, mmc.isEmpty());
            assertEquals(allowAdded, change.getAllowAddedSet());
            assertEquals(allowRemoved, change.getAllowRemovedSet());
            assertEquals(denyAdded, change.getDenyAddedSet());
            assertEquals(denyRemoved, change.getDenyRemovedSet());
            assertEquals(false, change.isRemoving());
            assertEquals(false, change.dontPurge());
        }

        acl = VtnAclType.ALLOW;
        denyRemoved = allowAdded;
        for (Iterator<MacVlan> it = allowedSet.iterator(); it.hasNext();) {
            mvlan = it.next();
            allowRemoved = Collections.singleton(mvlan);
            it.remove();
            hosts = Collections.singletonList(mvlan.getVlanHostDesc());
            boolean removing = allowedSet.isEmpty();

            change = mmc.update(op, hosts, emptyDesc);
            assertEquals(allowedSet, mmc.getAllowedHosts());
            assertEquals(deniedSet, mmc.getDeniedHosts());
            assertEquals(removing, mmc.isEmpty());
            assertEquals(allowAdded, change.getAllowAddedSet());
            assertEquals(allowRemoved, change.getAllowRemovedSet());
            assertEquals(denyAdded, change.getDenyAddedSet());
            assertEquals(denyRemoved, change.getDenyRemovedSet());
            assertEquals(removing, change.isRemoving());
            assertEquals(false, change.dontPurge());
        }
    }

    /**
     * Test case for {@link VTNMacMapConfig#equals(Object)} and
     * {@link VTNMacMapConfig#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        List<AllowedHosts> allowedCases = new ArrayList<>();
        List<DeniedHosts> deniedCases = new ArrayList<>();
        allowedCases.add(null);
        deniedCases.add(null);

        MacVlan[][] allowedHosts = {
            new MacVlan[] {new MacVlan(0x001122334455L, (short)0)},
            new MacVlan[] {
                new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
                new MacVlan(0xf8ffff998877L, (short)1),
                new MacVlan(0x3abcdef12345L, (short)100),
            },
            new MacVlan[] {
                new MacVlan(MacVlan.UNDEFINED, (short)123),
                new MacVlan(MacVlan.UNDEFINED, (short)456),
                new MacVlan(0x00abcdef0000L, (short)3),
                new MacVlan(0x00abcdef0001L, (short)4),
                new MacVlan(0x00abcdef0002L, (short)5),
                new MacVlan(0x00abcdef0003L, (short)6),
            },
        };

        MacVlan[][] deniedHosts = {
            new MacVlan[] {new MacVlan(0x003344556677L, (short)10)},
            new MacVlan[] {
                new MacVlan(0xa876543210ffL, (short)999),
                new MacVlan(0xa00000000001L, (short)1000),
            },
            new MacVlan[] {
                new MacVlan(0x000000000011L, (short)0),
                new MacVlan(0x000000000012L, (short)0),
                new MacVlan(0x000000000013L, (short)0),
                new MacVlan(0xa00000000013L, (short)0),
                new MacVlan(0xa00000000014L, (short)0),
            },
        };

        for (MacVlan[] ahs: allowedHosts) {
            List<VlanHostDescList> vhdescs = new ArrayList<>();
            for (MacVlan mv: ahs) {
                vhdescs.add(mv.getVlanHostDescList());
            }
            AllowedHosts allowed = new AllowedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            allowedCases.add(allowed);
        }

        for (MacVlan[] dhs: deniedHosts) {
            List<VlanHostDescList> vhdescs = new ArrayList<>();
            for (MacVlan mv: dhs) {
                vhdescs.add(mv.getVlanHostDescList());
            }
            DeniedHosts denied = new DeniedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            deniedCases.add(denied);
        }

        for (AllowedHosts ah: allowedCases) {
            for (DeniedHosts dh: deniedCases) {
                MacMapConfig mconf = new MacMapConfigBuilder().
                    setAllowedHosts(ah).
                    setDeniedHosts(dh).
                    build();

                VTNMacMapConfig mmc1 = new VTNMacMapConfig(mconf);
                VTNMacMapConfig mmc2 = new VTNMacMapConfig(mconf);
                testEquals(set, mmc1, mmc2);
            }
        }

        assertEquals(allowedCases.size() * deniedCases.size(),
                     set.size());
    }

    /**
     * Ensure that {@link VTNMacMapConfig} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<VTNMacMapConfig> type = VTNMacMapConfig.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);

        List<AllowedHosts> allowedCases = new ArrayList<>();
        List<DeniedHosts> deniedCases = new ArrayList<>();
        allowedCases.add(null);
        deniedCases.add(null);

        MacVlan[][] allowedHosts = {
            new MacVlan[] {new MacVlan(0x001122334455L, (short)0)},
            new MacVlan[] {
                new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
                new MacVlan(0xf8ffff998877L, (short)1),
                new MacVlan(0x3abcdef12345L, (short)100),
            },
            new MacVlan[] {
                new MacVlan(MacVlan.UNDEFINED, (short)123),
                new MacVlan(MacVlan.UNDEFINED, (short)456),
                new MacVlan(0x00abcdef0000L, (short)3),
                new MacVlan(0x00abcdef0001L, (short)4),
                new MacVlan(0x00abcdef0002L, (short)5),
                new MacVlan(0x00abcdef0003L, (short)6),
            },
        };

        MacVlan[][] deniedHosts = {
            new MacVlan[] {new MacVlan(0x003344556677L, (short)10)},
            new MacVlan[] {
                new MacVlan(0xa876543210ffL, (short)999),
                new MacVlan(0xa00000000001L, (short)1000),
            },
            new MacVlan[] {
                new MacVlan(0x000000000011L, (short)0),
                new MacVlan(0x000000000012L, (short)0),
                new MacVlan(0x000000000013L, (short)0),
                new MacVlan(0xa00000000013L, (short)0),
                new MacVlan(0xa00000000014L, (short)0),
            },
        };

        for (MacVlan[] ahs: allowedHosts) {
            List<VlanHostDescList> vhdescs = new ArrayList<>();
            for (MacVlan mv: ahs) {
                vhdescs.add(mv.getVlanHostDescList());
            }
            AllowedHosts allowed = new AllowedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            allowedCases.add(allowed);
        }

        for (MacVlan[] dhs: deniedHosts) {
            List<VlanHostDescList> vhdescs = new ArrayList<>();
            for (MacVlan mv: dhs) {
                vhdescs.add(mv.getVlanHostDescList());
            }
            DeniedHosts denied = new DeniedHostsBuilder().
                setVlanHostDescList(vhdescs).build();
            deniedCases.add(denied);
        }

        for (AllowedHosts ah: allowedCases) {
            for (DeniedHosts dh: deniedCases) {
                MacMapConfig mconf = new MacMapConfigBuilder().
                    setAllowedHosts(ah).
                    setDeniedHosts(dh).
                    build();

                VTNMacMapConfig mmc = new VTNMacMapConfig(mconf);
                VTNMacMapConfig mmc1 =  jaxbTest(mmc, type, m, um, XML_ROOT);
                mmc1.verify();
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        Map<String, String> broken = new HashMap<>();

        // Broadcast MAC address.
        String mac = "ff:ff:ff:ff:ff:ff";
        String xml = new XmlNode(XML_ROOT).
            add(new XmlNode("allowed-hosts").
                add(new XmlNode("host").
                    add(new XmlNode("address", "00:11:22:33:44:55")).
                    add(new XmlNode("vlan-id", 0))).
                add(new XmlNode("host").
                    add(new XmlNode("address", mac)).
                    add(new XmlNode("vlan-id", 1)))).
            toString();
        broken.put(xml, "Broadcast address cannot be specified: " + mac);

        // Multicast address.
        String[] multicasts = {
            "01:00:00:00:00:00",
            "01:11:22:33:44:55",
            "7f:ff:ff:ff:ff:ff",
        };
        for (String multi: multicasts) {
            xml = new XmlNode(XML_ROOT).
                add(new XmlNode("allowed-hosts").
                    add(new XmlNode("host").
                        add(new XmlNode("address", "00:11:22:33:44:55")).
                        add(new XmlNode("vlan-id", 0))).
                    add(new XmlNode("host").
                        add(new XmlNode("address", multi)).
                        add(new XmlNode("vlan-id", 1)))).
                toString();
            broken.put(xml, "Multicast address cannot be specified: " + multi);
        }

        // Duplicate MAC address in allowed-hosts.
        mac = "a0:b0:c0:d0:e0:f0";
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("allowed-hosts").
                add(new XmlNode("host").
                    add(new XmlNode("address", "00:11:22:33:44:55")).
                    add(new XmlNode("vlan-id", 0))).
                add(new XmlNode("host").
                    add(new XmlNode("address", mac)).
                    add(new XmlNode("vlan-id", 0))).
                add(new XmlNode("host").
                    add(new XmlNode("address", "88:99:aa:bb:cc:dd")).
                    add(new XmlNode("vlan-id", 1))).
                add(new XmlNode("host").
                    add(new XmlNode("address", mac)).
                    add(new XmlNode("vlan-id", 1)))).
            toString();
        broken.put(xml, "Duplicate MAC address in allowed set: " + mac);

        // No MAC address in denied-hosts.
        int vlan = 1;
        xml = new XmlNode(XML_ROOT).
            add(new XmlNode("allowed-hosts").
                add(new XmlNode("host").
                    add(new XmlNode("address", "00:11:22:33:44:55")).
                    add(new XmlNode("vlan-id", 0))).
                add(new XmlNode("host").
                    add(new XmlNode("address", "88:99:aa:bb:cc:dd")).
                    add(new XmlNode("vlan-id", 1)))).
            add(new XmlNode("denied-hosts").
                add(new XmlNode("host").
                    add(new XmlNode("address", "00:11:11:11:11:11")).
                    add(new XmlNode("vlan-id", 4095))).
                add(new XmlNode("host").
                    add(new XmlNode("vlan-id", vlan))).
                add(new XmlNode("host").
                    add(new XmlNode("address", "00:22:22:22:22:22")).
                    add(new XmlNode("vlan-id", 4095)))).
            toString();
        broken.put(xml,
                   "MAC address cannot be null in denied host set: vlan=" +
                   vlan);

        for (Map.Entry<String, String> entry: broken.entrySet()) {
            VTNMacMapConfig mmc = unmarshal(um, entry.getKey(), type);
            try {
                mmc.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }
    }

    /**
     * Convert the given list of {@link VlanHostDesc} instances into a set of
     * {@link MacVlan} instances.
     *
     * @param hosts  A list of {@link VlanHostDesc} instances.
     * @return  A set of {@link MacVlan} instances.
     * @throws Exception  An error occurred.
     */
    private Set<MacVlan> toHostSet(List<VlanHostDesc> hosts) throws Exception {
        Set<MacVlan> set;
        if (hosts == null) {
            set = Collections.<MacVlan>emptySet();
        } else {
            set = new HashSet<>();
            for (VlanHostDesc host: hosts) {
                set.add(new MacVlan(host));
            }
        }

        return set;
    }

    /**
     * Duplicate hosts in the specified list.
     *
     * @param hosts  A list of {@link VlanHostDesc} instances.
     * @return  A new list of {@link VlanHostDesc} instances.
     */
    private List<VlanHostDesc> duplicate(List<VlanHostDesc> hosts) {
        List<VlanHostDesc> list = new ArrayList<>(hosts.size() << 1);
        for (VlanHostDesc vhd: hosts) {
            list.add(vhd);
        }
        for (VlanHostDesc vhd: hosts) {
            list.add(vhd);
        }
        return list;
    }
}
