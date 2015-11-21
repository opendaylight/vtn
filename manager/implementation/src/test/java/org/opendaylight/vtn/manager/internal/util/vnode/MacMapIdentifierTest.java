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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.mappable.MacMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;

/**
 * JUnit test for {@link MacMapIdentifier}.
 */
public class MacMapIdentifierTest extends TestBase {
    /**
     * Test case for {@link VNodeIdentifier#getTenantName()} and
     * {@link VNodeIdentifier#getTenantNameString()}.
     */
    @Test
    public void testGetTenantName() {
        VnodeName bname = new VnodeName("bridge_1");

        String[] names = {
            null, "vtn", "vtn_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName tname = (name == null) ? null : new VnodeName(name);
            MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
            assertEquals(tname, mapPath.getTenantName());
            assertEquals(name, mapPath.getTenantNameString());

            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
            mapPath = new MacMapIdentifier(vbrId);
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

        String[] names = {
            null, "vbr", "vbridge_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName bname = (name == null) ? null : new VnodeName(name);
            MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
            assertEquals(bname, mapPath.getBridgeName());
            assertEquals(name, mapPath.getBridgeNameString());

            VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
            mapPath = new MacMapIdentifier(vbrId);
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
        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
        assertEquals(null, mapPath.getInterfaceName());
        assertEquals(null, mapPath.getInterfaceNameString());

        VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
        mapPath = new MacMapIdentifier(vbrId);
        assertEquals(null, mapPath.getInterfaceName());
        assertEquals(null, mapPath.getInterfaceNameString());
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link MacMapIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                InstanceIdentifier<MacMap> exp = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(MacMap.class).
                    build();
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                InstanceIdentifier<MacMap> path = mapPath.getIdentifier();
                assertEquals(exp, path);

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(path, mapPath.getIdentifier());
                }
            }
        }
    }

    /**
     * Test case for {@link MacMapIdentifier#getStatusPath()}.
     */
    @Test
    public void testGetStatusPath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                InstanceIdentifier<MacMapStatus> exp = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(MacMap.class).
                    child(MacMapStatus.class).
                    build();
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                InstanceIdentifier<MacMapStatus> path =
                    mapPath.getStatusPath();
                assertEquals(exp, path);

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(path, mapPath.getStatusPath());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getComponents()} and
     * {@link MacMapIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                List<String> comps = mapPath.getComponents();
                assertEquals(2, comps.size());
                assertEquals(tname.getValue(), comps.get(0));
                assertEquals(bname.getValue(), comps.get(1));

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(comps, mapPath.getComponents());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VNodeIdentifier)}.
     */
    @Test
    public void testContains() {
        // MAC mapping may contain MAC mapped hosts.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
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
                    new VBridgeIfIdentifier(tname, bname, iname),
                    new VTerminalIfIdentifier(tname, bname, iname));
                MacMapIdentifier mapPath1 = new MacMapIdentifier(tname, bname);
                MacMapIdentifier mapPath2 = new MacMapIdentifier(tname, bname);
                assertEquals(true, mapPath1.contains(mapPath1));
                assertEquals(true, mapPath1.contains(mapPath2));
                assertEquals(true, mapPath2.contains(mapPath1));
                assertEquals(true, mapPath2.contains(mapPath2));

                List<VNodeIdentifier<?>> trueList = new ArrayList<>();
                for (MacVlan host: hosts) {
                    trueList.add(new MacMapHostIdentifier(tname, bname, host));
                    falseList.add(
                        new VlanMapIdentifier(tname, bname, host.toString()));
                }

                for (VNodeIdentifier<?> ident: falseList) {
                    assertEquals(false, mapPath1.contains(ident));
                    assertEquals(false, mapPath2.contains(ident));
                }
                for (VNodeIdentifier<?> ident: trueList) {
                    assertEquals(true, mapPath1.contains(ident));
                    assertEquals(true, mapPath2.contains(ident));
                }

                falseList.addAll(trueList);
                falseList.add(mapPath1);
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VirtualNodePath)}.
     */
    @Test
    public void testContainsVirtualNodePath() {
        // MAC mapping may contain MAC mapped hosts.
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
                Collections.addAll(falseList, vbpath, vtmpath);
                List<VirtualNodePath> trueList = new ArrayList<>();
                trueList.add(mpath);

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
                    bmi = new BridgeMapInfoBuilder().
                        setMacMappedHost(host.getEncodedValue()).build();
                    VirtualNodePath mhpath = new VirtualNodePathBuilder().
                        setTenantName(tname).setBridgeName(bname).
                        addAugmentation(BridgeMapInfo.class, bmi).
                        build();
                    trueList.add(mhpath);
                }

                MacMapIdentifier mapPath =
                    new MacMapIdentifier(vtnName, brName);
                for (VirtualNodePath vpath: falseList) {
                    assertEquals(false, mapPath.contains(vpath));
                }
                for (VirtualNodePath vpath: trueList) {
                    assertEquals(true, mapPath.contains(vpath));
                }

                falseList.addAll(trueList);
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
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                BridgeMapInfo bmi = new BridgeMapInfoBuilder().
                    setMacMappedHost(-1L).build();
                VirtualNodePath vpath = new VirtualNodePathBuilder().
                    setTenantName(tname.getValue()).
                    setBridgeName(bname.getValue()).
                    addAugmentation(BridgeMapInfo.class, bmi).
                    build();
                assertEquals(vpath, mapPath.getVirtualNodePath());
            }
        }
    }

    /**
     * Test case for {@link MacMapIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    assertEquals(VNodeType.MACMAP, mapPath.getType());
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
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                VBridgeIdentifier ident = mapPath.getBridgeIdentifier();
                assertEquals(expected, ident);
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
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier expected =
                    new VBridgeIdentifier(tname, bname);
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                VBridgeIdentifier ident = mapPath.getVNodeIdentifier();
                assertEquals(expected, ident);
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

                MacMapIdentifier mapPath1 =
                    new MacMapIdentifier(vtnName1, vbrName1);
                MacMapIdentifier mapPath2 =
                    new MacMapIdentifier(vtnName2, vbrName2);
                testEquals(set, mapPath1, mapPath2);
            }
        }

        int expected = tnames.length * bnames.length;
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

                MacMapIdentifier mapPath =
                    new MacMapIdentifier(vtnName, vbrName);
                String expected = joinStrings(
                    "MAC-map:", null, "/", tnameStr, bnameStr);
                assertEquals(expected, mapPath.toString());

                // VNodeIdentifier.create() works only if path components
                // are valid.
                boolean valid = (tname != null && bname != null);
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

    /**
     * Test case for {@link VNodeIdentifier#read(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("vbridge_1");
        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<MacMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            child(MacMap.class).
            build();
        MacMap mmap = new MacMapBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(mmap));
        Optional<MacMap> opt = mapPath.read(rtx);
        assertEquals(mmap, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        mmap = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(mmap));
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
        VnodeName bname = new VnodeName("vbridge_1");
        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<MacMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            child(MacMap.class).
            build();
        MacMap mmap = new MacMapBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(mmap));
        assertEquals(mmap, mapPath.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        mmap = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(mmap));
        try {
            mapPath.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(mapPath.toString() + ": MAC mapping does not exist.",
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
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                String msg = mapPath.toString() +
                    ": MAC mapping does not exist.";
                RpcException e = mapPath.getNotFoundException();
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
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
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);
                String msg = mapPath.toString() +
                    ": MAC mapping already exists.";
                RpcException e = mapPath.getDataExistsException();
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
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
        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = mapPath.toString() +
            ": MAC mapping does not support flow filter.";
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
        MacMapIdentifier mapPath = new MacMapIdentifier(tname, bname);

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = mapPath.toString() +
            ": MAC mapping does not support flow filter.";
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
