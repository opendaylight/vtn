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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VterminalPathFields;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

/**
 * JUnit test for {@link VTerminalIdentifier}.
 */
public class VTerminalIdentifierTest extends TestBase {
    /**
     * Test case for
     * {@link VTerminalIdentifier#create(String, String, boolean)} and
     * {@link VTerminalIdentifier#create(VterminalPathFields, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        String[] bnames = {
            "vtm", "vtm_1", "vterm_2",
        };
        boolean[] bools = {true, false};

        for (String tname: tnames) {
            for (String bname: bnames) {
                for (boolean find: bools) {
                    VTerminalIdentifier brId =
                        VTerminalIdentifier.create(tname, bname, find);
                    assertEquals(tname, brId.getTenantNameString());
                    assertEquals(bname, brId.getBridgeNameString());

                    VterminalPathFields tpath = mock(VterminalPathFields.class);
                    when(tpath.getTenantName()).thenReturn(tname);
                    when(tpath.getTerminalName()).thenReturn(bname);
                    brId = VTerminalIdentifier.create(tpath, find);
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
                VTerminalIdentifier.create((String)null, "vterm", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }

            VterminalPathFields tpath = mock(VterminalPathFields.class);
            when(tpath.getTenantName()).thenReturn((String)null);
            when(tpath.getTerminalName()).thenReturn("vterm");
            try {
                VTerminalIdentifier.create(tpath, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        msg = "vTerminal name cannot be null";
        for (boolean find: bools) {
            try {
                VTerminalIdentifier.create("vtn", (String)null, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }

            VterminalPathFields tpath = mock(VterminalPathFields.class);
            when(tpath.getTenantName()).thenReturn("vtn");
            when(tpath.getTerminalName()).thenReturn((String)null);
            try {
                VTerminalIdentifier.create(tpath, find);
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
                VTerminalIdentifier.create("", "vterm", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            VterminalPathFields tpath = mock(VterminalPathFields.class);
            when(tpath.getTenantName()).thenReturn("");
            when(tpath.getTerminalName()).thenReturn("vterm");
            try {
                VTerminalIdentifier.create(tpath, find);
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
                VTerminalIdentifier.create(badName, "vterm", find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }

            tpath = mock(VterminalPathFields.class);
            when(tpath.getTenantName()).thenReturn(badName);
            when(tpath.getTerminalName()).thenReturn("vterm");
            try {
                VTerminalIdentifier.create(tpath, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertTrue(e.getCause() instanceof RpcException);
                assertEquals(msg, e.getMessage());
            }
        }

        // Empty vTerminal name (read).
        msg = ": vTerminal does not exist.";
        try {
            VTerminalIdentifier.create("vtn", "", true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        VterminalPathFields tpath = mock(VterminalPathFields.class);
        when(tpath.getTenantName()).thenReturn("vtn");
        when(tpath.getTerminalName()).thenReturn("");
        try {
            VTerminalIdentifier.create(tpath, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Invalid vTerminal name (read).
        msg = badName + ": vTerminal does not exist.";
        try {
            VTerminalIdentifier.create("vtn", badName, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        tpath = mock(VterminalPathFields.class);
        when(tpath.getTenantName()).thenReturn("vtn");
        when(tpath.getTerminalName()).thenReturn(badName);
        try {
            VTerminalIdentifier.create(tpath, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Empty vTerminal name (write).
        msg = "vTerminal name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        vtag = VtnErrorTag.BADREQUEST;
        try {
            VTerminalIdentifier.create("vtn", "", false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        tpath = mock(VterminalPathFields.class);
        when(tpath.getTenantName()).thenReturn("vtn");
        when(tpath.getTerminalName()).thenReturn("");
        try {
            VTerminalIdentifier.create(tpath, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        // Invalid vTerminal name (write).
        msg = "vTerminal name is invalid";
        try {
            VTerminalIdentifier.create("vtn", badName, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof IllegalArgumentException);
            assertEquals(msg, e.getMessage());
        }

        tpath = mock(VterminalPathFields.class);
        when(tpath.getTenantName()).thenReturn("vtn");
        when(tpath.getTerminalName()).thenReturn(badName);
        try {
            VTerminalIdentifier.create(tpath, false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof IllegalArgumentException);
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

        String[] names = {
            null, "vtn", "vtn_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName tname = (name == null) ? null : new VnodeName(name);
            VTerminalIdentifier brId = new VTerminalIdentifier(tname, bname);
            assertEquals(tname, brId.getTenantName());
            assertEquals(name, brId.getTenantNameString());

            brId = new VTerminalIdentifier(brId, bname);
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
            null, "vbr", "vterm_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName bname = (name == null) ? null : new VnodeName(name);
            VTerminalIdentifier brId = new VTerminalIdentifier(tname, bname);
            assertEquals(bname, brId.getBridgeName());
            assertEquals(name, brId.getBridgeNameString());

            brId = new VTerminalIdentifier(brId, bname);
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
        VTerminalIdentifier brId = new VTerminalIdentifier(tname, bname);
        assertEquals(null, brId.getInterfaceName());
        assertEquals(null, brId.getInterfaceNameString());

        brId = new VTerminalIdentifier(brId, bname);
        assertEquals(null, brId.getInterfaceName());
        assertEquals(null, brId.getInterfaceNameString());
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link VTerminalIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtermKey = new VterminalKey(bname);
                InstanceIdentifier<Vterminal> exp = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vterminal.class, vtermKey).
                    build();
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                InstanceIdentifier<Vterminal> path = brId.getIdentifier();
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
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            for (VnodeName bname: bnames) {
                VterminalKey vtermKey = new VterminalKey(bname);
                InstanceIdentifier<BridgeStatus> exp = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(Vterminal.class, vtermKey).
                    child(BridgeStatus.class).
                    build();
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
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
     * {@link VTerminalIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
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
        // vTerminal may contain vTerminal interface.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
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
                    new MacMapIdentifier(tname, bname));
                VTerminalIdentifier brId1 =
                    new VTerminalIdentifier(tname, bname);
                VTerminalIdentifier brId2 =
                    new VTerminalIdentifier(tname, bname);
                assertEquals(true, brId1.contains(brId1));
                assertEquals(true, brId1.contains(brId2));
                assertEquals(true, brId2.contains(brId1));
                assertEquals(true, brId2.contains(brId2));

                List<VNodeIdentifier<?>> trueList = new ArrayList<>();
                for (VnodeName iname: inames) {
                    trueList.add(
                        new VTerminalIfIdentifier(tname, bname, iname));
                    Collections.addAll(
                        falseList,
                        new VBridgeIfIdentifier(tname, bname, iname),
                        new VlanMapIdentifier(tname, bname, iname.getValue()));
                }
                for (MacVlan host: hosts) {
                    falseList.add(
                        new MacMapHostIdentifier(tname, bname, host));
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
        // vTerminal may contain vTerminal interface.
        List<VirtualNodePath> falseList = new ArrayList<>();

        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
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
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(vtnName, brName);
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
                Collections.addAll(falseList, vbpath, mpath);
                trueList.add(vtmpath);

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
                    Collections.addAll(falseList, ipath1, vmpath);
                    trueList.add(ipath2);
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
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                VirtualNodePath vpath = new VirtualNodePathBuilder().
                    setTenantName(tname.getValue()).
                    setTerminalName(bname.getValue()).
                    build();
                assertEquals(vpath, brId.getVirtualNodePath());
            }
        }
    }

    /**
     * Test case for {@link VTerminalIdentifier#childInterface(VnodeName)}.
     */
    @Test
    public void testChildInterface() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("if_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                for (VnodeName iname: inames) {
                    VTerminalIfIdentifier ifId = brId.childInterface(iname);
                    assertEquals(tname, ifId.getTenantName());
                    assertEquals(bname, ifId.getBridgeName());
                    assertEquals(iname, ifId.getInterfaceName());
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIdentifier#getBridgeIdentifier()}.
     */
    @Test
    public void testGetBridgeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    VTerminalIdentifier ident = brId.getBridgeIdentifier();
                    assertSame(brId, ident);
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    assertEquals(VNodeType.VTERMINAL, brId.getType());
                }
            }
        }
    }

    /**
     * Test case for {@link VTerminalIdentifier#getVNodeIdentifier()}.
     */
    @Test
    public void testGetVNodeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                for (int i = 0; i < 5; i++) {
                    VTerminalIdentifier ident = brId.getVNodeIdentifier();
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
        String[] bnames = {null, "vbr", "vterm_1"};
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

                VTerminalIdentifier brId1 =
                    new VTerminalIdentifier(vtnName1, vbrName1);
                VTerminalIdentifier brId2 =
                    new VTerminalIdentifier(vtnName2, vbrName2);
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
        String[] bnames = {null, "vbr", "vterm_1"};
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

                VTerminalIdentifier brId =
                    new VTerminalIdentifier(vtnName, vbrName);
                String expected = joinStrings(
                    "vTerminal:", null, "/", tnameStr, bnameStr);
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
        VnodeName bname = new VnodeName("vterm_1");
        VTerminalIdentifier vtmId = new VTerminalIdentifier(tname, bname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vterminal> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vterminal.class, new VterminalKey(bname)).
            build();
        Vterminal vtm = new VterminalBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtm));
        Optional<Vterminal> opt = vtmId.read(rtx);
        assertEquals(vtm, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vtm = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtm));
        opt = vtmId.read(rtx);
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
        VTerminalIdentifier vtmId = new VTerminalIdentifier(tname, bname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vterminal> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(Vterminal.class, new VterminalKey(bname)).
            build();
        Vterminal vtm = new VterminalBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtm));
        assertEquals(vtm, vtmId.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vtm = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtm));
        try {
            vtmId.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(vtmId.toString() + ": vTerminal does not exist.",
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
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                String msg = brId.toString() + ": vTerminal does not exist.";
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
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vterm_1")};
        for (VnodeName tname: tnames) {
            for (VnodeName bname: bnames) {
                VTerminalIdentifier brId =
                    new VTerminalIdentifier(tname, bname);
                String msg = brId.toString() + ": vTerminal already exists.";
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
     */
    @Test
    public void testGetFlowFilterIdentifier() {
        VnodeName tname = new VnodeName("vtn_1");
        VnodeName bname = new VnodeName("bridge_1");
        VTerminalIdentifier brId = new VTerminalIdentifier(tname, bname);

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = brId.toString() +
            ": vTerminal does not support flow filter.";
        Integer[] indices = {1, 100, 65535};
        boolean[] bools = {true, false};
        for (boolean output: bools) {
            for (Integer index: indices) {
                try {
                    brId.getFlowFilterIdentifier(output, index);
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
        VTerminalIdentifier brId = new VTerminalIdentifier(tname, bname);

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = brId.toString() +
            ": vTerminal does not support flow filter.";
        boolean[] bools = {true, false};
        for (boolean output: bools) {
            try {
                brId.clearFlowFilter(tx, output);
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
