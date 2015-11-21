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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilterKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.result.FlowFilterResultBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VbridgePathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.MacTables;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.mac.tables.TenantMacTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTable;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.list.MacAddressTableKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeOutputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link VBridgeIdentifier}.
 */
public class VBridgeIdentifierTest extends TestBase {
    /**
     * Test case for
     * {@link VBridgeIdentifier#create(String, String, boolean)} and
     * {@link VBridgeIdentifier#create(VbridgePathFields, boolean)}.
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
        boolean[] bools = {true, false};

        for (String tname: tnames) {
            for (String bname: bnames) {
                for (boolean find: bools) {
                    VBridgeIdentifier brId =
                        VBridgeIdentifier.create(tname, bname, find);
                    assertEquals(tname, brId.getTenantNameString());
                    assertEquals(bname, brId.getBridgeNameString());

                    VbridgePathFields bpath = mock(VbridgePathFields.class);
                    when(bpath.getTenantName()).thenReturn(tname);
                    when(bpath.getBridgeName()).thenReturn(bname);
                    brId = VBridgeIdentifier.create(bpath, find);
                    assertEquals(tname, brId.getTenantNameString());
                    assertEquals(bname, brId.getBridgeNameString());
                }
            }
        }

        // Null name.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "VTN name cannot be null";
        for (boolean find: bools) {
            try {
                VBridgeIdentifier.create((String)null, "vbr", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }

            VbridgePathFields bpath = mock(VbridgePathFields.class);
            when(bpath.getTenantName()).thenReturn((String)null);
            when(bpath.getBridgeName()).thenReturn("vbr");
            try {
                VBridgeIdentifier.create(bpath, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        msg = "vBridge name cannot be null";
        for (boolean find: bools) {
            try {
                VBridgeIdentifier.create("vtn", (String)null, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }

            VbridgePathFields bpath = mock(VbridgePathFields.class);
            when(bpath.getTenantName()).thenReturn("vtn");
            when(bpath.getBridgeName()).thenReturn((String)null);
            try {
                VBridgeIdentifier.create(bpath, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        String badName = "bad name";
        etag = RpcErrorTag.DATA_MISSING;
        vtag = VtnErrorTag.NOTFOUND;
        for (boolean find: bools) {
            // Empty VTN name.
            msg = ": VTN does not exist.";
            try {
                VBridgeIdentifier.create("", "vbr", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            VbridgePathFields bpath = mock(VbridgePathFields.class);
            when(bpath.getTenantName()).thenReturn("");
            when(bpath.getBridgeName()).thenReturn("vbr");
            try {
                VBridgeIdentifier.create(bpath, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            // Invalid VTN name.
            msg = badName + ": VTN does not exist.";
            try {
                VBridgeIdentifier.create(badName, "vbr", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            bpath = mock(VbridgePathFields.class);
            when(bpath.getTenantName()).thenReturn(badName);
            when(bpath.getBridgeName()).thenReturn("vbr");
            try {
                VBridgeIdentifier.create(bpath, true);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }
        }

        // Empty vBridge name (read).
        msg = ": vBridge does not exist.";
        try {
            VBridgeIdentifier.create("vtn", "", true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        VbridgePathFields bpath = mock(VbridgePathFields.class);
        when(bpath.getTenantName()).thenReturn("vtn");
        when(bpath.getBridgeName()).thenReturn("");
        try {
            VBridgeIdentifier.create(bpath, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        msg = badName + ": vBridge does not exist.";
        try {
            VBridgeIdentifier.create("vtn", badName, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        bpath = mock(VbridgePathFields.class);
        when(bpath.getTenantName()).thenReturn("vtn");
        when(bpath.getBridgeName()).thenReturn(badName);
        try {
            VBridgeIdentifier.create(bpath, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Empty vBridge name (write).
        msg = "vBridge name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        vtag = VtnErrorTag.BADREQUEST;
        try {
            VBridgeIdentifier.create("vtn", "", false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        bpath = mock(VbridgePathFields.class);
        when(bpath.getTenantName()).thenReturn("vtn");
        when(bpath.getBridgeName()).thenReturn("");
        try {
            VBridgeIdentifier.create(bpath, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        // Invalid vBridge name (write).
        msg = "vBridge name is invalid";
        try {
            VBridgeIdentifier.create("vtn", badName, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof IllegalArgumentException);
            assertEquals(msg, e.getMessage());
        }

        bpath = mock(VbridgePathFields.class);
        when(bpath.getTenantName()).thenReturn("vtn");
        when(bpath.getBridgeName()).thenReturn(badName);
        try {
            VBridgeIdentifier.create(bpath, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof IllegalArgumentException);
            assertEquals(msg, e.getMessage());
        }
    }

    /**
     * Test case for
     * {@link VBridgeIdentifier#getTenantMacTablePath(VNodeIdentifier)}.
     */
    @Test
    public void testGetTenantMacTablePath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        for (VnodeName tname: tnames) {
            VNodeIdentifier<?> ident = new VTenantIdentifier(tname);
            TenantMacTableKey key = new TenantMacTableKey(tname.getValue());
            InstanceIdentifier<TenantMacTable> expected = InstanceIdentifier.
                builder(MacTables.class).
                child(TenantMacTable.class, key).
                build();
            assertEquals(expected,
                         VBridgeIdentifier.getTenantMacTablePath(ident));
        }
    }

    /**
     * Test case for
     * {@link VBridgeIdentifier#getMacTablePath(BridgeIdentifier)}.
     */
    @Test
    public void testGetMacTablePath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            TenantMacTableKey tkey = new TenantMacTableKey(tname.getValue());
            for (VnodeName bname: bnames) {
                MacAddressTableKey bkey =
                    new MacAddressTableKey(bname.getValue());
                BridgeIdentifier<Vbridge> brId =
                    new VBridgeIdentifier(tname, bname);
                InstanceIdentifier<MacAddressTable> expected =
                    InstanceIdentifier.builder(MacTables.class).
                    child(TenantMacTable.class, tkey).
                    child(MacAddressTable.class, bkey).
                    build();
                assertEquals(expected,
                             VBridgeIdentifier.getMacTablePath(brId));
            }
        }
    }

    /**
     * Test case for
     * {@link VBridgeIdentifier#getMacEntryPath(BridgeIdentifier, MacAddress)}.
     */
    @Test
    public void testGetMacEntryPath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        MacAddress[] addrs = {
            new MacAddress("00:11:22:33:44:55"),
            new MacAddress("a0:b0:c0:d0:e0:f0"),
            new MacAddress("01:23:45:ab:cd:ef"),
        };
        for (VnodeName tname: tnames) {
            TenantMacTableKey tkey = new TenantMacTableKey(tname.getValue());
            for (VnodeName bname: bnames) {
                MacAddressTableKey bkey =
                    new MacAddressTableKey(bname.getValue());
                BridgeIdentifier<Vbridge> brId =
                    new VBridgeIdentifier(tname, bname);
                for (MacAddress mac: addrs) {
                    MacTableEntryKey entKey = new MacTableEntryKey(mac);
                    InstanceIdentifier<MacTableEntry> expected =
                        InstanceIdentifier.builder(MacTables.class).
                        child(TenantMacTable.class, tkey).
                        child(MacAddressTable.class, bkey).
                        child(MacTableEntry.class, entKey).
                        build();
                    assertEquals(expected,
                                 VBridgeIdentifier.getMacEntryPath(brId, mac));
                }
            }
        }
    }

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
            VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
            assertEquals(tname, brId.getTenantName());
            assertEquals(name, brId.getTenantNameString());

            brId = new VBridgeIdentifier(brId, bname);
            assertEquals(tname, brId.getTenantName());
            assertEquals(name, brId.getTenantNameString());
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
            VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
            assertEquals(bname, brId.getBridgeName());
            assertEquals(name, brId.getBridgeNameString());

            brId = new VBridgeIdentifier(brId, bname);
            assertEquals(bname, brId.getBridgeName());
            assertEquals(name, brId.getBridgeNameString());
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
        VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
        assertEquals(null, brId.getInterfaceName());
        assertEquals(null, brId.getInterfaceNameString());

        brId = new VBridgeIdentifier(brId, bname);
        assertEquals(null, brId.getInterfaceName());
        assertEquals(null, brId.getInterfaceNameString());
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link VBridgeIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                InstanceIdentifier<Vbridge> exp = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    build();
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                InstanceIdentifier<Vbridge> path = brId.getIdentifier();
                assertEquals(exp, path);

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(path, brId.getIdentifier());
                }
            }
        }
    }

    /**
     * Test case for {@link BridgeIdentifier#getStatusPath()}.
     */
    @Test
    public void testGetStatusPath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                InstanceIdentifier<BridgeStatus> exp = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(BridgeStatus.class).
                    build();
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                InstanceIdentifier<BridgeStatus> path = brId.getStatusPath();
                assertEquals(exp, path);

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(path, brId.getStatusPath());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getComponents()} and
     * {@link VBridgeIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                List<String> comps = brId.getComponents();
                assertEquals(2, comps.size());
                assertEquals(tname.getValue(), comps.get(0));
                assertEquals(bname.getValue(), comps.get(1));

                // Result should be cached.
                for (int i = 0; i < 5; i++) {
                    assertSame(comps, brId.getComponents());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VNodeIdentifier)}.
     */
    @Test
    public void testContains() {
        // vBridge may contain vBridge interface, VLAN mapping, MAC mapping,
        // and MAC mapped hosts.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName tname: tnames) {
            falseList.add(new VTenantIdentifier(tname));
            for (VnodeName bname: bnames) {
                falseList.add(new VTerminalIdentifier(tname, bname));
                VBridgeIdentifier brId1 = new VBridgeIdentifier(tname, bname);
                VBridgeIdentifier brId2 = new VBridgeIdentifier(tname, bname);
                assertEquals(true, brId1.contains(brId1));
                assertEquals(true, brId1.contains(brId2));
                assertEquals(true, brId2.contains(brId1));
                assertEquals(true, brId2.contains(brId2));

                List<VNodeIdentifier<?>> trueList = new ArrayList<>();
                trueList.add(new MacMapIdentifier(tname, bname));
                for (MacVlan host: hosts) {
                    trueList.add(new MacMapHostIdentifier(tname, bname, host));
                }
                for (VnodeName iname: inames) {
                    Collections.addAll(
                        trueList,
                        new VBridgeIfIdentifier(tname, bname, iname),
                        new VlanMapIdentifier(tname, bname, iname.getValue()));
                    falseList.add(
                        new VTerminalIfIdentifier(tname, bname, iname));
                }

                for (VNodeIdentifier<?> ident: falseList) {
                    assertEquals(false, brId1.contains(ident));
                    assertEquals(false, brId2.contains(ident));
                }
                for (VNodeIdentifier<?> ident: trueList) {
                    assertEquals(true, brId1.contains(ident));
                    assertEquals(true, brId2.contains(ident));
                }

                falseList.addAll(trueList);
                falseList.add(brId1);
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VirtualNodePath)}.
     */
    @Test
    public void testContainsVirtualNodePath() {
        // vBridge may contain vBridge interface, VLAN mapping, MAC mapping,
        // and MAC mapped hosts.
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
                VBridgeIdentifier brId =
                    new VBridgeIdentifier(vtnName, brName);
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
                List<VirtualNodePath> trueList = new ArrayList<>();
                Collections.addAll(trueList, vbpath, mpath);
                falseList.add(vtmpath);

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
                    Collections.addAll(trueList, ipath1, vmpath);
                    falseList.add(ipath2);
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

                for (VirtualNodePath vpath: falseList) {
                    assertEquals(false, brId.contains(vpath));
                }
                for (VirtualNodePath vpath: trueList) {
                    assertEquals(true, brId.contains(vpath));
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
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                VirtualNodePath vpath = new VirtualNodePathBuilder().
                    setTenantName(tname.getValue()).
                    setBridgeName(bname.getValue()).
                    build();
                assertEquals(vpath, brId.getVirtualNodePath());
            }
        }
    }

    /**
     * Test case for {@link VBridgeIdentifier#childInterface(VnodeName)}.
     */
    @Test
    public void testChildInterface() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("if_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                for (VnodeName iname: inames) {
                    VBridgeIfIdentifier ifId = brId.childInterface(iname);
                    assertEquals(tname, ifId.getTenantName());
                    assertEquals(bname, ifId.getBridgeName());
                    assertEquals(iname, ifId.getInterfaceName());
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeIdentifier#getBridgeIdentifier()}.
     */
    @Test
    public void testGetBridgeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    VBridgeIdentifier ident = brId.getBridgeIdentifier();
                    assertSame(brId, ident);
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    assertEquals(VNodeType.VBRIDGE, brId.getType());
                }
            }
        }
    }

    /**
     * Test case for {@link VBridgeIdentifier#getVNodeIdentifier()}.
     */
    @Test
    public void testGetVNodeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    VBridgeIdentifier ident = brId.getVNodeIdentifier();
                    assertSame(brId, ident);
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

                VBridgeIdentifier brId1 =
                    new VBridgeIdentifier(vtnName1, vbrName1);
                VBridgeIdentifier brId2 =
                    new VBridgeIdentifier(vtnName2, vbrName2);
                testEquals(set, brId1, brId2);
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

                VBridgeIdentifier brId =
                    new VBridgeIdentifier(vtnName, vbrName);
                String expected = joinStrings(
                    "vBridge:", null, "/", tnameStr, bnameStr);
                assertEquals(expected, brId.toString());

                // VNodeIdentifier.create() works only if path components
                // are valid.
                boolean valid = (tname != null && bname != null);
                try {
                    VNodeIdentifier<?> ident =
                        VNodeIdentifier.create(expected);
                    assertTrue(valid);
                    assertEquals(brId, ident);

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
        VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vbridge> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            build();
        Vbridge vbr = new VbridgeBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vbr));
        Optional<Vbridge> opt = vbrId.read(rtx);
        assertEquals(vbr, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vbr = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vbr));
        opt = vbrId.read(rtx);
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
        VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vbridge> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vbridge.class, new VbridgeKey(bname)).
            build();
        Vbridge vbr = new VbridgeBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vbr));
        assertEquals(vbr, vbrId.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vbr = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vbr));
        try {
            vbrId.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(vbrId.toString() + ": vBridge does not exist.",
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
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                String msg = brId.toString() + ": vBridge does not exist.";
                RpcException e = brId.getNotFoundException();
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
                VBridgeIdentifier brId = new VBridgeIdentifier(tname, bname);
                String msg = brId.toString() + ": vBridge already exists.";
                RpcException e = brId.getDataExistsException();
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
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetFlowFilterIdentifier() throws Exception {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        Integer[] indices = {1, 100, 65535};
        InstanceIdentifier<VtnFlowFilter> expected;
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);
                for (Integer index: indices) {
                    VtnFlowFilterKey filterKey = new VtnFlowFilterKey(index);
                    expected = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(VbridgeInputFilter.class).
                        child(VtnFlowFilter.class, filterKey).
                        build();
                    assertEquals(expected,
                                 vbrId.getFlowFilterIdentifier(false, index));

                    expected = InstanceIdentifier.
                        builder(Vtns.class).
                        child(Vtn.class, vtnKey).
                        child(Vbridge.class, vbrKey).
                        child(VbridgeOutputFilter.class).
                        child(VtnFlowFilter.class, filterKey).
                        build();
                    assertEquals(expected,
                                 vbrId.getFlowFilterIdentifier(true, index));
                }
            }
        }
    }

    /**
     * Test case for
     * {@link VBridgeIdentifier#clearFlowFilter(ReadWriteTransaction, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClearFlowFilter() throws Exception {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        List<VtnFlowFilter> vfilters = new ArrayList<>();
        List<FlowFilterResult> results = new ArrayList<>();
        for (int i = 1; i <= 4; i++) {
            VtnFlowFilter vff = new VtnFlowFilterBuilder().
                setIndex(i).build();
            FlowFilterResult res = new FlowFilterResultBuilder().
                setIndex(i).setStatus(VtnUpdateType.REMOVED).build();
            vfilters.add(vff);
            results.add(res);
        }

        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VbridgeKey vbrKey = new VbridgeKey(bname);
                VBridgeIdentifier vbrId = new VBridgeIdentifier(tname, bname);

                InstanceIdentifier<VbridgeInputFilter> ipath =
                    InstanceIdentifier.builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(VbridgeInputFilter.class).
                    build();

                // INPUT: The target list is missing.
                ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
                VbridgeInputFilter ilist = null;
                when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                assertEquals(null, vbrId.clearFlowFilter(tx, false));
                verify(tx).read(oper, ipath);
                verifyNoMoreInteractions(tx);

                // INPUT: Flow filter list is null.
                List<VtnFlowFilter> filters = null;
                ilist = mock(VbridgeInputFilter.class);
                when(ilist.getVtnFlowFilter()).thenReturn(filters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                assertEquals(null, vbrId.clearFlowFilter(tx, false));
                verify(tx).read(oper, ipath);
                verify(tx).delete(oper, ipath);
                verifyNoMoreInteractions(tx);

                // INPUT: Flow filter list is empty.
                filters = Collections.<VtnFlowFilter>emptyList();
                ilist = mock(VbridgeInputFilter.class);
                when(ilist.getVtnFlowFilter()).thenReturn(filters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                assertEquals(null, vbrId.clearFlowFilter(tx, false));
                verify(tx).read(oper, ipath);
                verify(tx).delete(oper, ipath);
                verifyNoMoreInteractions(tx);

                // INPUT: Flow filter list is not empty.
                ilist = mock(VbridgeInputFilter.class);
                when(ilist.getVtnFlowFilter()).thenReturn(vfilters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, ipath)).thenReturn(getReadResult(ilist));
                assertEquals(results, vbrId.clearFlowFilter(tx, false));
                verify(tx).read(oper, ipath);
                verify(tx).delete(oper, ipath);
                verifyNoMoreInteractions(tx);

                InstanceIdentifier<VbridgeOutputFilter> opath =
                    InstanceIdentifier.builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vbridge.class, vbrKey).
                    child(VbridgeOutputFilter.class).
                    build();

                // OUTPUT: The target list is missing.
                tx = mock(ReadWriteTransaction.class);
                VbridgeOutputFilter olist = null;
                when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                assertEquals(null, vbrId.clearFlowFilter(tx, true));
                verify(tx).read(oper, opath);
                verifyNoMoreInteractions(tx);

                // OUTPUT: Flow filter list is null.
                filters = null;
                olist = mock(VbridgeOutputFilter.class);
                when(olist.getVtnFlowFilter()).thenReturn(filters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                assertEquals(null, vbrId.clearFlowFilter(tx, true));
                verify(tx).read(oper, opath);
                verify(tx).delete(oper, opath);
                verifyNoMoreInteractions(tx);

                // OUTPUT: Flow filter list is empty.
                filters = Collections.<VtnFlowFilter>emptyList();
                olist = mock(VbridgeOutputFilter.class);
                when(olist.getVtnFlowFilter()).thenReturn(filters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                assertEquals(null, vbrId.clearFlowFilter(tx, true));
                verify(tx).read(oper, opath);
                verify(tx).delete(oper, opath);
                verifyNoMoreInteractions(tx);

                // OUTPUT: Flow filter list is not empty.
                olist = mock(VbridgeOutputFilter.class);
                when(olist.getVtnFlowFilter()).thenReturn(vfilters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, opath)).thenReturn(getReadResult(olist));
                assertEquals(results, vbrId.clearFlowFilter(tx, true));
                verify(tx).read(oper, opath);
                verify(tx).delete(oper, opath);
                verifyNoMoreInteractions(tx);
            }
        }
    }
}
