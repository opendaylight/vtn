/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.map.info.VlanMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;

/**
 * JUnit test for {@link VlanMapIdentifier}.
 */
public class VlanMapIdentifierTest extends TestBase {
    /**
     * Test case for
     * {@link VlanMapIdentifier#create(String, String, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vbr", "vbr_1", "vbridge_2",
        };
        String[] mapIds = {
            "ANY.1", "openflow:3.0", "openflow:12345678.4095",
        };

        for (String tname: tnames) {
            for (String bname: bnames) {
                for (String mapId: mapIds) {
                    VlanMapIdentifier mpath = VlanMapIdentifier.
                        create(tname, bname, mapId);
                    assertEquals(tname, mpath.getTenantNameString());
                    assertEquals(bname, mpath.getBridgeNameString());
                    assertEquals(mapId, mpath.getMapId());
                }
            }
        }

        // Null name.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "VTN name cannot be null";
        try {
            VlanMapIdentifier.create((String)null, "vbr", "ANY.0");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        msg = "vBridge name cannot be null";
        try {
            VlanMapIdentifier.create("vtn", (String)null, "ANY.0");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        msg = "VLAN mapping ID cannot be null";
        try {
            VlanMapIdentifier.create("vtn", "vbr", (String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        // Empty name.
        msg = ": VTN does not exist.";
        etag = RpcErrorTag.DATA_MISSING;
        vtag = VtnErrorTag.NOTFOUND;
        try {
            VlanMapIdentifier.create("", "vbr", "ANY.0");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        msg = ": vBridge does not exist.";
        try {
            VlanMapIdentifier.create("vtn", "", "ANY.0");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Invalid name.
        String badName = "bad name";
        msg = badName + ": VTN does not exist.";
        try {
            VlanMapIdentifier.create(badName, "vbr", "ANY.0");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        msg = badName + ": vBridge does not exist.";
        try {
            VlanMapIdentifier.create("vtn", badName, "ANY.0");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getTenantName()} and
     * {@link VNodeIdentifier#getTenantNameString()}.
     */
    @Test
    public void testGetTenantName() {
        VnodeName bname = new VnodeName("bridge_1");
        String mapId = "ANY.1";

        String[] names = {
            null, "vtn", "vtn_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName tname = (name == null) ? null : new VnodeName(name);
            VlanMapIdentifier mapPath =
                new VlanMapIdentifier(tname, bname, mapId);
            assertEquals(tname, mapPath.getTenantName());
            assertEquals(name, mapPath.getTenantNameString());

            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
            mapPath = new VlanMapIdentifier(vbrId, mapId);
            assertEquals(tname, mapPath.getTenantName());
            assertEquals(name, mapPath.getTenantNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getBridgeName()} and
     * {@link VNodeIdentifier#getBridgeNameString()}.
     */
    @Test
    public void testGetBridgeName() {
        VnodeName tname = new VnodeName("vtn_1");
        String mapId = "ANY.1";

        String[] names = {
            null, "vbr", "vbridge_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName bname = (name == null) ? null : new VnodeName(name);
            VlanMapIdentifier mapPath =
                new VlanMapIdentifier(tname, bname, mapId);
            assertEquals(bname, mapPath.getBridgeName());
            assertEquals(name, mapPath.getBridgeNameString());

            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
            mapPath = new VlanMapIdentifier(vbrId, mapId);
            assertEquals(bname, mapPath.getBridgeName());
            assertEquals(name, mapPath.getBridgeNameString());
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
        String mapId = "ANY.1";
        VlanMapIdentifier mapPath = new VlanMapIdentifier(tname, bname, mapId);
        assertEquals(null, mapPath.getInterfaceName());
        assertEquals(null, mapPath.getInterfaceNameString());

        VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
        mapPath = new VlanMapIdentifier(vbrId, mapId);
        assertEquals(null, mapPath.getInterfaceName());
        assertEquals(null, mapPath.getInterfaceNameString());
    }

    /**
     * Test case for {@link VlanMapIdentifier#getMapId()}.
     */
    @Test
    public void testGetMapId() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");
        String[] mapIds = {
            null, "ANY.1", "openflow:3.0", "openflow:12345678.4095",
        };
        for (String mapId: mapIds) {
            VlanMapIdentifier mapPath =
                new VlanMapIdentifier(tname, bname, mapId);
            assertEquals(mapId, mapPath.getMapId());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link VlanMapIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                for (String mapId: mapIds) {
                    VlanMapKey mapKey = new VlanMapKey(mapId);
                    InstanceIdentifier<VlanMap> exp = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(VlanMap.class, mapKey).
                        build();
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    InstanceIdentifier<VlanMap> path = mapPath.getIdentifier();
                    assertEquals(exp, path);

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(path, mapPath.getIdentifier());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMapIdentifier#getStatusPath()}.
     */
    @Test
    public void testGetStatusPath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                for (String mapId: mapIds) {
                    VlanMapKey mapKey = new VlanMapKey(mapId);
                    InstanceIdentifier<VlanMapStatus> exp = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(VlanMap.class, mapKey).
                        child(VlanMapStatus.class).
                        build();
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    InstanceIdentifier<VlanMapStatus> path =
                        mapPath.getStatusPath();
                    assertEquals(exp, path);

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(path, mapPath.getStatusPath());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getComponents()} and
     * {@link VlanMapIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    List<String> comps = mapPath.getComponents();
                    assertEquals(3, comps.size());
                    assertEquals(tname.getValue(), comps.get(0));
                    assertEquals(bname.getValue(), comps.get(1));
                    assertEquals(mapId, comps.get(2));

                    // Result should be cached.
                    for (int i = 0; i < 5; i++) {
                        assertSame(comps, mapPath.getComponents());
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
        // VlanMapIdentifier.contains(VNodeIdentifier) returns true only
        // if the given instance is equal to the instance.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();
        falseList.add(null);

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName tname: tnames) {
            falseList.add(new VTenantIdentifier(tname));
            for (VnodeName bname: bnames) {
                Collections.addAll(
                    falseList,
                    new VBridgeIdentifier(tname, bname),
                    new VTerminalIdentifier(tname, bname),
                    new MacMapIdentifier(tname, bname));
                for (MacVlan host: hosts) {
                    falseList.add(
                        new MacMapHostIdentifier(tname, bname, host));
                }
                for (VnodeName iname: inames) {
                    Collections.addAll(
                        falseList,
                        new VBridgeIfIdentifier(tname, bname, iname),
                        new VTerminalIfIdentifier(tname, bname, iname));
                }
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath1 =
                        new VlanMapIdentifier(tname, bname, mapId);
                    VlanMapIdentifier mapPath2 =
                        new VlanMapIdentifier(tname, bname, mapId);
                    assertEquals(true, mapPath1.contains(mapPath1));
                    assertEquals(true, mapPath1.contains(mapPath2));
                    assertEquals(true, mapPath2.contains(mapPath1));
                    assertEquals(true, mapPath2.contains(mapPath2));

                    for (VNodeIdentifier<?> ident: falseList) {
                        assertEquals(false, mapPath1.contains(ident));
                        assertEquals(false, mapPath2.contains(ident));
                    }
                    falseList.add(mapPath1);
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VirtualNodePath)}.
     */
    @Test
    public void testContainsVirtualNodePath() {
        // VlanMapIdentifier.contains(VirtualNodePath) returns true only
        // if the given instance specifies the same VLAN mapping.
        List<VirtualNodePath> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        String[] inames = {"if", "vif_1"};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
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
                    Collections.addAll(falseList, ipath1, ipath2);
                }
                for (MacVlan host: hosts) {
                    bmi = new BridgeMapInfoBuilder().
                        setMacMappedHost(host.getEncodedValue()).build();
                    VirtualNodePath mhpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    falseList.add(mhpath);
                }
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(vtnName, brName, mapId);
                    bmi = new BridgeMapInfoBuilder().
                        setVlanMapId(mapId).build();
                    VirtualNodePath vmpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    assertEquals(true, mapPath.contains(vmpath));

                    for (VirtualNodePath vpath: falseList) {
                        assertEquals(false, mapPath.contains(vpath));
                    }
                    falseList.add(vmpath);
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
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    BridgeMapInfo bmi = new BridgeMapInfoBuilder().
                        setVlanMapId(mapId).build();
                    VirtualNodePath vpath = new VirtualNodePathBuilder().
                        setTenantName(tname.getValue()).
                        setBridgeName(bname.getValue()).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    assertEquals(vpath, mapPath.getVirtualNodePath());
                }
            }
        }
    }

    /**
     * Test case for {@link VlanMapIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    for (int i = 0; i < 5; i++) {
                        assertEquals(VNodeType.VLANMAP, mapPath.getType());
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
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    VBridgeIdentifier ident = mapPath.getBridgeIdentifier();
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
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    VBridgeIdentifier ident = mapPath.getVNodeIdentifier();
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
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (String tname: tnames) {
            VnodeName vtnName1;
            VnodeName vtnName2;
            if (tname == null) {
                vtnName1 = null;
                vtnName2 = null;
            } else {
                vtnName1 = new VnodeName(tname);
                vtnName2 = new VnodeName(copy(tname));
            }
            for (String bname: bnames) {
                VnodeName vbrName1;
                VnodeName vbrName2;
                if (bname == null) {
                    vbrName1 = null;
                    vbrName2 = null;
                } else {
                    vbrName1 = new VnodeName(bname);
                    vbrName2 = new VnodeName(copy(bname));
                }

                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath1 =
                        new VlanMapIdentifier(vtnName1, vbrName1, mapId);
                    VlanMapIdentifier mapPath2 =
                        new VlanMapIdentifier(vtnName2, vbrName2, copy(mapId));
                    testEquals(set, mapPath1, mapPath2);
                }
            }
        }

        int expected = tnames.length * bnames.length * mapIds.length;
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
        String[] mapIds = {
            null, "ANY.1", "openflow:3.0", "openflow:12345678.4095",
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
                for (String mapId: mapIds) {
                    String idStr = (mapId == null) ? "<null>" : mapId;
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(vtnName, vbrName, mapId);
                    String expected = joinStrings(
                        "VLAN-map:", null, "/", tnameStr, bnameStr, idStr);
                    assertEquals(expected, mapPath.toString());

                    // VNodeIdentifier.create() works only if path components
                    // are valid.
                    boolean valid = (tname != null && bname != null &&
                                     mapId != null);
                    try {
                        VNodeIdentifier<?> ident =
                            VNodeIdentifier.create(expected);
                        assertTrue(valid);
                        assertEquals(mapPath, ident);

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
        VnodeName bname = new VnodeName("vterm_1");
        String mapId = "openflow:1.0";
        VlanMapIdentifier mapPath = new VlanMapIdentifier(tname, bname, mapId);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<VlanMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            child(VlanMap.class, new VlanMapKey(mapId)).
            build();
        VlanMap vmap = new VlanMapBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vmap));
        Optional<VlanMap> opt = mapPath.read(rtx);
        assertEquals(vmap, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vmap = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vmap));
        opt = mapPath.read(rtx);
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
        VnodeName bname = new VnodeName("vterm_1");
        String mapId = "openflow:1.0";
        VlanMapIdentifier mapPath = new VlanMapIdentifier(tname, bname, mapId);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<VlanMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            child(VlanMap.class, new VlanMapKey(mapId)).
            build();
        VlanMap vmap = new VlanMapBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vmap));
        assertEquals(vmap, mapPath.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vmap = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vmap));
        try {
            mapPath.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(mapPath.toString() + ": VLAN mapping does not exist.",
                         e.getMessage());
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
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    String msg = mapPath.toString() +
                        ": VLAN mapping does not exist.";
                    RpcException e = mapPath.getNotFoundException();
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
        String[] mapIds = {"ANY.1", "openflow:3.0", "openflow:12345678.4095"};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                for (String mapId: mapIds) {
                    VlanMapIdentifier mapPath =
                        new VlanMapIdentifier(tname, bname, mapId);
                    String msg = mapPath.toString() +
                        ": VLAN mapping already exists.";
                    RpcException e = mapPath.getDataExistsException();
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
        String mapId = "ANY.1";
        VlanMapIdentifier mapPath = new VlanMapIdentifier(tname, bname, mapId);

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = mapPath.toString() +
            ": VLAN mapping does not support flow filter.";
        Integer[] indices = {1, 100, 65535};
        boolean[] bools = {true, false};
        for (boolean output: bools) {
            for (Integer index: indices) {
                try {
                    mapPath.getFlowFilterIdentifier(output, index);
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
        String mapId = "ANY.1";
        VlanMapIdentifier mapPath = new VlanMapIdentifier(tname, bname, mapId);

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = mapPath.toString() +
            ": VLAN mapping does not support flow filter.";
        boolean[] bools = {true, false};
        for (boolean output: bools) {
            try {
                mapPath.clearFlowFilter(tx, output);
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
