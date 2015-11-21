/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.google.common.base.Optional;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;

/**
 * JUnit test for {@link MacMapHostIdentifier}.
 */
public class MacMapHostIdentifierTest extends TestBase {
    /**
     * Test case for
     * {@link MacMapHostIdentifier#getMappedHost(VirtualNodePath)}.
     */
    @Test
    public void testGetMappedHostStatic() {
        VirtualNodePath vpath = null;
        assertEquals(null, MacMapHostIdentifier.getMappedHost(vpath));

        vpath = new VirtualNodePathBuilder().build();
        assertEquals(null, MacMapHostIdentifier.getMappedHost(vpath));

        BridgeMapInfo bmi = new BridgeMapInfoBuilder().build();
        vpath = new VirtualNodePathBuilder().
            addAugmentation(BridgeMapInfo.class, bmi).build();
        assertEquals(null, MacMapHostIdentifier.getMappedHost(vpath));

        MacVlan mv = new MacVlan(0xfe0123abcdefL, 4095);
        bmi = new BridgeMapInfoBuilder().
            setMacMappedHost(mv.getEncodedValue()).build();
        vpath = new VirtualNodePathBuilder().
            addAugmentation(BridgeMapInfo.class, bmi).build();
        assertEquals(mv, MacMapHostIdentifier.getMappedHost(vpath));
    }

    /**
     * Test case for {@link VNodeIdentifier#getTenantName()} and
     * {@link VNodeIdentifier#getTenantNameString()}.
     */
    @Test
    public void testGetTenantName() {
        VnodeName bname = new VnodeName("bridge_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);

        String[] names = {
            null, "vtn", "vtn_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName tname = (name == null) ? null : new VnodeName(name);
            MacMapHostIdentifier hostPath =
                new MacMapHostIdentifier(tname, bname, host);
            assertEquals(tname, hostPath.getTenantName());
            assertEquals(name, hostPath.getTenantNameString());

            MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
            hostPath = new MacMapHostIdentifier(mapPath, host);
            assertEquals(tname, hostPath.getTenantName());
            assertEquals(name, hostPath.getTenantNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getBridgeName()} and
     * {@link VNodeIdentifier#getBridgeNameString()}.
     */
    @Test
    public void testGetBridgeName() {
        VnodeName tname = new VnodeName("vtn_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);

        String[] names = {
            null, "vbr", "vbridge_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName bname = (name == null) ? null : new VnodeName(name);
            MacMapHostIdentifier hostPath =
                new MacMapHostIdentifier(tname, bname, host);
            assertEquals(bname, hostPath.getBridgeName());
            assertEquals(name, hostPath.getBridgeNameString());

            MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
            hostPath = new MacMapHostIdentifier(mapPath, host);
            assertEquals(bname, hostPath.getBridgeName());
            assertEquals(name, hostPath.getBridgeNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getInterfaceName()} and
     * {@link VNodeIdentifier#getInterfaceNameString()}.
     */
    @Test
    public void testGetInterfaceName() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);
        MacMapHostIdentifier hostPath =
            new MacMapHostIdentifier(tname, bname, host);
        assertEquals(null, hostPath.getInterfaceName());
        assertEquals(null, hostPath.getInterfaceNameString());

        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
        hostPath = new MacMapHostIdentifier(mapPath, host);
        assertEquals(null, hostPath.getInterfaceName());
        assertEquals(null, hostPath.getInterfaceNameString());
    }

    /**
     * Test case for {@link MacMapHostIdentifier#getMappedHost()}.
     */
    @Test
    public void testGetMappedHost() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");
        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
        MacVlan[] hosts = {
            null,
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (MacVlan host: hosts) {
            MacMapHostIdentifier hostPath =
                new MacMapHostIdentifier(tname, bname, host);
            assertEquals(host, hostPath.getMappedHost());

            hostPath = new MacMapHostIdentifier(mapPath, host);
            assertEquals(host, hostPath.getMappedHost());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link MacMapHostIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                for (MacVlan host: hosts) {
                    VlanHostDescListKey hostKey =
                        new VlanHostDescListKey(host.getVlanHostDesc());
                    InstanceIdentifier<VlanHostDescList> exp =
                        InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(MacMap.class).
                        child(MacMapConfig.class).
                        child(AllowedHosts.class).
                        child(VlanHostDescList.class, hostKey).
                        build();
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    InstanceIdentifier<VlanHostDescList> path =
                        hostPath.getIdentifier();
                    assertEquals(exp, path);

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(path, hostPath.getIdentifier());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getComponents()} and
     * {@link MacMapHostIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    List<String> comps = hostPath.getComponents();
                    assertEquals(3, comps.size());
                    assertEquals(tname.getValue(), comps.get(0));
                    assertEquals(bname.getValue(), comps.get(1));
                    assertEquals(Long.toString(host.getEncodedValue()),
                                 comps.get(2));

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(comps, hostPath.getComponents());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VNodeIdentifier)}.
     */
    @Test
    public void testContains() {
        // MacMapHostIdentifier.contains(VNodeIdentifier) returns true only
        // if the given instance is equal to the instance.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        VnodeName iname = new VnodeName("if");
        for (VnodeName tname: tnames) {
            falseList.add(new VTenantIdentifier(tname));
            for (VnodeName bname: bnames) {
                Collections.addAll(
                    falseList,
                    new VBridgeIdentifier(tname, bname),
                    new VTerminalIdentifier(tname, bname),
                    new MacMapIdentifier(tname, bname));
                for (VnodeName imame: inames) {
                    Collections.addAll(
                        falseList,
                        new VBridgeIfIdentifier(tname, bname, iname),
                        new VTerminalIfIdentifier(tname, bname, iname));
                }

                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath1 =
                        new MacMapHostIdentifier(tname, bname, host);
                    MacMapHostIdentifier hostPath2 =
                        new MacMapHostIdentifier(tname, bname, host);
                    assertEquals(true, hostPath1.contains(hostPath1));
                    assertEquals(true, hostPath1.contains(hostPath2));
                    assertEquals(true, hostPath2.contains(hostPath1));
                    assertEquals(true, hostPath2.contains(hostPath2));
                    falseList.add(
                        new VlanMapIdentifier(tname, bname, host.toString()));

                    for (VNodeIdentifier<?> ident: falseList) {
                        assertEquals(false, hostPath1.contains(ident));
                        assertEquals(false, hostPath2.contains(ident));
                    }
                    falseList.add(hostPath1);
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VirtualNodePath)}.
     */
    @Test
    public void testContainsVirtualNodePath() {
        // MacMapHostIdentifier.contains(VirtualNodePath) returns true only
        // if the given instance specifies the same MAC mapped host.
        List<VirtualNodePath> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        String[] inames = {"if", "vif_1"};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName vtnName: tnames) {
            String tname = vtnName.getValue();
            VirtualNodePath vtpath = new VirtualNodePathBuilder().
                setTenantName(tname).build();
            falseList.add(vtpath);

            for (VnodeName brName: bnames) {
                String bname = brName.getValue();
                VirtualNodePath vbpath = new VirtualNodePathBuilder().
                    setTenantName(tname).setBridgeName(bname).build();
                VirtualNodePath vtmpath = new VirtualNodePathBuilder().
                    setTenantName(tname).setTerminalName(bname).build();
                BridgeMapInfo bmi = new BridgeMapInfoBuilder().
                    setMacMappedHost(-1L).build();
                VirtualNodePath mpath = new VirtualNodePathBuilder().
                    setTenantName(tname).setBridgeName(bname).
                    addAugmentation(BridgeMapInfo.class, bmi).
                    build();
                Collections.addAll(falseList, vbpath, vtmpath, mpath);

                for (String iname: inames) {
                    VirtualNodePath ipath1 = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        setInterfaceName(iname).build();
                    VirtualNodePath ipath2 = new VirtualNodePathBuilder().
                        setTenantName(tname).setTerminalName(bname).
                        setInterfaceName(iname).build();
                    bmi = new BridgeMapInfoBuilder().
                        setVlanMapId(iname).build();
                    VirtualNodePath vmpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    Collections.addAll(falseList, ipath1, ipath2, vmpath);
                }
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(vtnName, brName, host);
                    bmi = new BridgeMapInfoBuilder().
                        setMacMappedHost(host.getEncodedValue()).build();
                    VirtualNodePath mhpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    assertEquals(true, hostPath.contains(mhpath));

                    for (VirtualNodePath vpath: falseList) {
                        assertEquals(false, hostPath.contains(vpath));
                    }
                    falseList.add(mhpath);
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getVirtualNodePath()}.
     */
    @Test
    public void testGetVirtualNodePath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    BridgeMapInfo bmi = new BridgeMapInfoBuilder().
                        setMacMappedHost(host.getEncodedValue()).build();
                    VirtualNodePath vpath = new VirtualNodePathBuilder().
                        setTenantName(tname.getValue()).
                        setBridgeName(bname.getValue()).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    assertEquals(vpath, hostPath.getVirtualNodePath());
                }
            }
        }
    }

    /**
     * Test case for {@link MacMapHostIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    for (int i = 0; i < 5; i++) {
                        assertEquals(VNodeType.MACMAP_HOST, hostPath.getType());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeMapIdentifier#getBridgeIdentifier()}.
     */
    @Test
    public void testGetBridgeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    VBridgeIdentifier ident = hostPath.getBridgeIdentifier();
                    assertEquals(expected, ident);
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeMapIdentifier#getVNodeIdentifier()}.
     */
    @Test
    public void testGetVNodeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    VBridgeIdentifier ident = hostPath.getVNodeIdentifier();
                    assertEquals(expected, ident);
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#equals(Object)} and
     * {@link VNodeIdentifier#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();

        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {null, "vbr", "vbridge_1"};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (String tname: tnames) {
            VnodeName vtnName1;
            VnodeName vtnName2;
            if (tname == null) {
                vtnName1 = null;
                vtnName2 = null;
            } else {
                vtnName1 = new VnodeName(tname);
                vtnName2 = copy(vtnName1);
            }
            for (String bname: bnames) {
                VnodeName vbrName1;
                VnodeName vbrName2;
                if (bname == null) {
                    vbrName1 = null;
                    vbrName2 = null;
                } else {
                    vbrName1 = new VnodeName(bname);
                    vbrName2 = copy(vbrName1);
                }
                for (MacVlan host1: hosts) {
                    MacVlan host2 = new MacVlan(host1.getEncodedValue());
                    MacMapHostIdentifier hostPath1 =
                        new MacMapHostIdentifier(vtnName1, vbrName1, host1);
                    MacMapHostIdentifier hostPath2 =
                        new MacMapHostIdentifier(vtnName2, vbrName2, host2);
                    testEquals(set, hostPath1, hostPath2);
                }
            }
        }

        int expected = tnames.length * bnames.length * hosts.length;
        assertEquals(expected, set.size());
    }

    /**
     * Test case for {@link VNodeIdentifier#toString()} and
     * {@link VNodeIdentifier#create(String)}.
     */
    @Test
    public void testToString() {
        String[] tnames = {null, "vtn", "vtn_1"};
        String[] bnames = {null, "vbr", "vbridge_1"};
        MacVlan[] hosts = {
            null,
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (String tname: tnames) {
            VnodeName vtnName;
            String tnameStr;
            if (tname == null) {
                vtnName = null;
                tnameStr = "<null>";
            } else {
                vtnName = new VnodeName(tname);
                tnameStr = tname;
            }
            for (String bname: bnames) {
                VnodeName vbrName;
                String bnameStr;
                if (bname == null) {
                    vbrName = null;
                    bnameStr = "<null>";
                } else {
                    vbrName = new VnodeName(bname);
                    bnameStr = bname;
                }
                for (MacVlan host: hosts) {
                    String hostStr = (host == null)
                        ? "<null>"
                        : Long.toString(host.getEncodedValue());

                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(vtnName, vbrName, host);
                    String expected = joinStrings(
                        "MAC-map-host:", null, "/", tnameStr, bnameStr,
                        hostStr);
                    assertEquals(expected, hostPath.toString());

                    // VNodeIdentifier.create() works only if path components
                    // are valid.
                    boolean valid = (tname != null && bname != null
                                     && host != null);
                    try {
                        VNodeIdentifier<?> ident =
                            VNodeIdentifier.create(expected);
                        assertTrue(valid);
                        assertEquals(hostPath, ident);

                        // The given string should be cached.
                        assertSame(expected, ident.toString());
                    } catch (IllegalArgumentException e) {
                        assertFalse(valid);
                        assertEquals("Invalid identifier format: " + expected,
                                     e.getMessage());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#read(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("vbridge_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);
        MacMapHostIdentifier hostPath =
            new MacMapHostIdentifier(tname, bname, host);
        ReadTransaction rtx = mock(ReadTransaction.class);
        VlanHostDescListKey hostKey =
            new VlanHostDescListKey(host.getVlanHostDesc());
        InstanceIdentifier<VlanHostDescList> path =
            InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            child(MacMap.class).
            child(MacMapConfig.class).
            child(AllowedHosts.class).
            child(VlanHostDescList.class, hostKey).
            build();
        VlanHostDescList vhdl = new VlanHostDescListBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vhdl));
        Optional<VlanHostDescList> opt = hostPath.read(rtx);
        assertEquals(vhdl, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vhdl = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vhdl));
        opt = hostPath.read(rtx);
        assertEquals(false, opt.isPresent());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link VNodeIdentifier#fetch(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testFetch() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("vbridge_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);
        MacMapHostIdentifier hostPath =
            new MacMapHostIdentifier(tname, bname, host);
        ReadTransaction rtx = mock(ReadTransaction.class);
        VlanHostDescListKey hostKey =
            new VlanHostDescListKey(host.getVlanHostDesc());
        InstanceIdentifier<VlanHostDescList> path =
            InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            child(MacMap.class).
            child(MacMapConfig.class).
            child(AllowedHosts.class).
            child(VlanHostDescList.class, hostKey).
            build();
        VlanHostDescList vhdl = new VlanHostDescListBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vhdl));
        assertEquals(vhdl, hostPath.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vhdl = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vhdl));
        try {
            hostPath.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            String msg = hostPath.toString() +
                ": MAC mapped host does not exist.";
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link VNodeIdentifier#getNotFoundException()}.
     */
    @Test
    public void testGetNotFoundException() {
        RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
        VtnErrorTag vtag = VtnErrorTag.NOTFOUND;

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    String msg = hostPath.toString() +
                        ": MAC mapped host does not exist.";
                    RpcException e = hostPath.getNotFoundException();
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getDataExistsException()}.
     */
    @Test
    public void testGetDataExistsException() {
        RpcErrorTag etag = RpcErrorTag.DATA_EXISTS;
        VtnErrorTag vtag = VtnErrorTag.CONFLICT;

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacVlan[] hosts = {
            new MacVlan(0x001122334455L, (short)0),
            new MacVlan(0xa0b0c0d0e0f0L, (short)4095),
        };
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (MacVlan host: hosts) {
                    MacMapHostIdentifier hostPath =
                        new MacMapHostIdentifier(tname, bname, host);
                    String msg = hostPath.toString() +
                        ": MAC mapped host already exists.";
                    RpcException e = hostPath.getDataExistsException();
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VNodeIdentifier#getFlowFilterIdentifier(boolean, Integer)}.
     */
    @Test
    public void testGetFlowFilterIdentifier() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);
        MacMapHostIdentifier hostPath =
            new MacMapHostIdentifier(tname, bname, host);

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = hostPath.toString() +
            ": MAC mapped host does not support flow filter.";
        Integer[] indices = {1, 100, 65535};
        boolean[] bools = {true, false};
        for (boolean output: bools) {
            for (Integer index: indices) {
                try {
                    hostPath.getFlowFilterIdentifier(output, index);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(msg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VNodeIdentifier#clearFlowFilter(ReadWriteTransaction, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClearFlowFilter() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");
        MacVlan host = new MacVlan(0x001122334455L, (short)1);
        MacMapHostIdentifier hostPath =
            new MacMapHostIdentifier(tname, bname, host);

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = hostPath.toString() +
            ": MAC mapped host does not support flow filter.";
        boolean[] bools = {true, false};
        for (boolean output: bools) {
            try {
                hostPath.clearFlowFilter(tx, output);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }
        verifyZeroInteractions(tx);
    }
}
