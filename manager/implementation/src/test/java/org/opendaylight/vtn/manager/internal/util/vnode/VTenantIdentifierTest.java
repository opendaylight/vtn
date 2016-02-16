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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnInputFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link VTenantIdentifier}.
 */
public class VTenantIdentifierTest extends TestBase {
    /**
     * Test case for {@link VTenantIdentifier#create(String, boolean)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        String[] tnames = {
            "vtn", "vtn_1", "vtn_2",
        };
        boolean[] bools = {true, false};

        for (String tname: tnames) {
            for (boolean find: bools) {
                VTenantIdentifier vtnId =
                    VTenantIdentifier.create(tname, find);
                assertEquals(tname, vtnId.getTenantNameString());
            }
        }

        // Null name.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "VTN name cannot be null";
        for (boolean find: bools) {
            try {
                VTenantIdentifier.create((String)null, find);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(msg, e.getMessage());
            }
        }

        // Empty name (read).
        msg = ": VTN does not exist.";
        etag = RpcErrorTag.DATA_MISSING;
        vtag = VtnErrorTag.NOTFOUND;
        try {
            VTenantIdentifier.create("", true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Invalid name (read).
        String badName = "bad name";
        msg = badName + ": VTN does not exist.";
        try {
            VTenantIdentifier.create(badName, true);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertTrue(e.getCause() instanceof RpcException);
            assertEquals(msg, e.getMessage());
        }

        // Empty name (write).
        msg = "VTN name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        vtag = VtnErrorTag.BADREQUEST;
        try {
            VTenantIdentifier.create("", false);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        // Invalid name (write).
        msg = "VTN name is invalid";
        try {
            VTenantIdentifier.create(badName, false);
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
        String[] names = {
            null, "vtn", "vtn_1", "0123456789012345678901234567890",
        };
        for (String name: names) {
            VnodeName tname = (name == null) ? null : new VnodeName(name);
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            assertEquals(tname, vtnId.getTenantName());
            assertEquals(name, vtnId.getTenantNameString());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getBridgeName()} and
     * {@link VNodeIdentifier#getBridgeNameString()}.
     */
    @Test
    public void testGetBridgeName() {
        VnodeName tname = new VnodeName("vtn_1");
        VTenantIdentifier vtnId = new VTenantIdentifier(tname);
        assertEquals(null, vtnId.getBridgeName());
        assertEquals(null, vtnId.getBridgeNameString());
    }

    /**
     * Test case for {@link VNodeIdentifier#getInterfaceName()} and
     * {@link VNodeIdentifier#getInterfaceNameString()}.
     */
    @Test
    public void testGetInterfaceName() {
        VnodeName tname = new VnodeName("vtn_1");
        VTenantIdentifier vtnId = new VTenantIdentifier(tname);
        assertEquals(null, vtnId.getInterfaceName());
        assertEquals(null, vtnId.getInterfaceNameString());
    }

    /**
     * Test case for {@link VNodeIdentifier#getIdentifier()} and
     * {@link VTenantIdentifier#getIdentifierBuilder()}.
     */
    @Test
    public void testGetIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            InstanceIdentifier<Vtn> exp = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, vtnKey).
                build();
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            InstanceIdentifier<Vtn> path = vtnId.getIdentifier();
            assertEquals(exp, path);

            // Result should be cached.
            for (int i = 0; i < 5; i++) {
                assertSame(path, vtnId.getIdentifier());
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getComponents()} and
     * {@link VTenantIdentifier#newComponents()}.
     */
    @Test
    public void testGetComponents() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            List<String> comps = vtnId.getComponents();
            assertEquals(1, comps.size());
            assertEquals(tname.getValue(), comps.get(0));

            // Result should be cached.
            for (int i = 0; i < 5; i++) {
                assertSame(comps, vtnId.getComponents());
            }
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VNodeIdentifier)}.
     */
    @Test
    public void testContains() {
        // VTN may contains all types of virtual nodes.
        List<VNodeIdentifier<?>> falseList = new ArrayList<>();
        falseList.add(null);

        VnodeName[] tnames = {
            new VnodeName("vtn"),
            new VnodeName("vtn_1"),
            new VnodeName("vtn_2"),
        };
        VnodeName[] bnames = {new VnodeName("vbr"), new VnodeName("vbridge_1")};
        VnodeName[] inames = {new VnodeName("if"), new VnodeName("vif_1")};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId1 = new VTenantIdentifier(tname);
            VTenantIdentifier vtnId2 = new VTenantIdentifier(tname);
            assertEquals(true, vtnId1.contains(vtnId1));
            assertEquals(true, vtnId1.contains(vtnId2));
            assertEquals(true, vtnId2.contains(vtnId1));
            assertEquals(true, vtnId2.contains(vtnId2));

            List<VNodeIdentifier<?>> trueList = new ArrayList<>();
            for (VnodeName bname: bnames) {
                Collections.addAll(
                    trueList,
                    new VBridgeIdentifier(tname, bname),
                    new VTerminalIdentifier(tname, bname),
                    new MacMapIdentifier(tname, bname));

                for (VnodeName iname: inames) {
                    Collections.addAll(
                        trueList,
                        new VBridgeIfIdentifier(tname, bname, iname),
                        new VTerminalIfIdentifier(tname, bname, iname),
                        new VlanMapIdentifier(tname, bname, iname.getValue()));
                }
                for (MacVlan host: hosts) {
                    trueList.add(new MacMapHostIdentifier(tname, bname, host));
                }
            }

            for (VNodeIdentifier<?> ident: falseList) {
                assertEquals(false, vtnId1.contains(ident));
                assertEquals(false, vtnId2.contains(ident));
            }
            for (VNodeIdentifier<?> ident: trueList) {
                assertEquals(true, vtnId1.contains(ident));
                assertEquals(true, vtnId2.contains(ident));
            }

            falseList.addAll(trueList);
            falseList.add(vtnId1);
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#contains(VirtualNodePath)}.
     */
    @Test
    public void testContainsVirtualNodePath() {
        // VTN may contains all types of virtual nodes.
        List<VirtualNodePath> falseList = new ArrayList<>();

        VnodeName[] tnames = {
            new VnodeName("vtn"),
            new VnodeName("vtn_1"),
            new VnodeName("vtn_2"),
        };
        String[] bnames = {"vbr", "vbridge_1"};
        String[] inames = {"if", "vif_1"};
        MacVlan[] hosts = {
            new MacVlan(0xa0b0c0d0e0f0L, (short)0),
            new MacVlan(0x001122334455L, (short)1024),
        };
        for (VnodeName vtnName: tnames) {
            String tname = vtnName.getValue();
            List<VirtualNodePath> trueList = new ArrayList<>();
            VTenantIdentifier vtnId = new VTenantIdentifier(vtnName);
            VirtualNodePath vtpath = new VirtualNodePathBuilder().
                setTenantName(tname).build();
            trueList.add(vtpath);

            for (String bname: bnames) {
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
                Collections.addAll(trueList, vbpath, vtmpath, mpath);

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
                    Collections.addAll(trueList, ipath1, ipath2, vmpath);
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
            }

            for (VirtualNodePath vpath: falseList) {
                assertEquals(false, vtnId.contains(vpath));
            }
            for (VirtualNodePath vpath: trueList) {
                assertEquals(true, vtnId.contains(vpath));
            }

            falseList.addAll(trueList);
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#getVirtualNodePath()}.
     */
    @Test
    public void testGetVirtualNodePath() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            VirtualNodePath vpath = new VirtualNodePathBuilder().
                setTenantName(tname.getValue()).build();
            assertEquals(vpath, vtnId.getVirtualNodePath());
        }
    }

    /**
     * Test case for {@link VNodeIdentifier#readVtenantConfig(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtenantConfig() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VTenantIdentifier vtnId = new VTenantIdentifier(tname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<VtenantConfig> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            child(VtenantConfig.class).
            build();
        VtenantConfig vtnc = new VtenantConfigBuilder().
            setIdleTimeout(3000).
            setHardTimeout(60000).
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtnc));
        Optional<VtenantConfig> opt = vtnId.readVtenantConfig(rtx);
        assertEquals(vtnc, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vtnc = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtnc));
        opt = vtnId.readVtenantConfig(rtx);
        assertEquals(false, opt.isPresent());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
    }

    /**
     * Test case for {@link VNodeIdentifier#read(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead() throws Exception {
        VnodeName tname = new VnodeName("vtn_1");
        VTenantIdentifier vtnId = new VTenantIdentifier(tname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vtn> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            build();
        Vtn vtn = new VtnBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
        Optional<Vtn> opt = vtnId.read(rtx);
        assertEquals(vtn, opt.get());
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vtn = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
        opt = vtnId.read(rtx);
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
        VTenantIdentifier vtnId = new VTenantIdentifier(tname);
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<Vtn> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            build();
        Vtn vtn = new VtnBuilder().build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
        assertEquals(vtn, vtnId.fetch(rtx));
        verify(rtx).read(oper, path);
        verifyNoMoreInteractions(rtx);
        reset(rtx);

        vtn = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(vtn));
        try {
            vtnId.fetch(rtx);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(vtnId.toString() + ": VTN does not exist.",
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
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            String msg = vtnId.toString() + ": VTN does not exist.";
            RpcException e = vtnId.getNotFoundException();
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
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
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            String msg = vtnId.toString() + ": VTN already exists.";
            RpcException e = vtnId.getDataExistsException();
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(msg, e.getMessage());
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
        Integer[] indices = {1, 100, 65535};
        boolean[] bools = {true, false};
        for (VnodeName tname: tnames) {
            VtnKey vtnKey = new VtnKey(tname);
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            for (Integer index: indices) {
                InstanceIdentifier<VtnFlowFilter> expected = InstanceIdentifier.
                    builder(Vtns.class).
                    child(Vtn.class, vtnKey).
                    child(VtnInputFilter.class).
                    child(VtnFlowFilter.class, new VtnFlowFilterKey(index)).
                    build();
                for (boolean output: bools) {
                    assertEquals(expected,
                                 vtnId.getFlowFilterIdentifier(output, index));
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
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        boolean[] bools = {true, false};

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
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            InstanceIdentifier<VtnInputFilter> path = InstanceIdentifier.
                builder(Vtns.class).
                child(Vtn.class, vtnKey).
                child(VtnInputFilter.class).
                build();

            for (boolean output: bools) {
                // The target list is missing.
                ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
                VtnInputFilter list = null;
                when(tx.read(oper, path)).thenReturn(getReadResult(list));
                assertEquals(null, vtnId.clearFlowFilter(tx, output));
                verify(tx).read(oper, path);
                verifyNoMoreInteractions(tx);

                // Flow filter list is null.
                List<VtnFlowFilter> filters = null;
                list = mock(VtnInputFilter.class);
                when(list.getVtnFlowFilter()).thenReturn(filters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, path)).thenReturn(getReadResult(list));
                assertEquals(null, vtnId.clearFlowFilter(tx, output));
                verify(tx).read(oper, path);
                verify(tx).delete(oper, path);
                verifyNoMoreInteractions(tx);

                // Flow filter list is empty.
                filters = Collections.<VtnFlowFilter>emptyList();
                list = mock(VtnInputFilter.class);
                when(list.getVtnFlowFilter()).thenReturn(filters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, path)).thenReturn(getReadResult(list));
                assertEquals(null, vtnId.clearFlowFilter(tx, output));
                verify(tx).read(oper, path);
                verify(tx).delete(oper, path);
                verifyNoMoreInteractions(tx);

                // Flow filter list is not empty.
                list = mock(VtnInputFilter.class);
                when(list.getVtnFlowFilter()).thenReturn(vfilters);
                tx = mock(ReadWriteTransaction.class);
                when(tx.read(oper, path)).thenReturn(getReadResult(list));
                assertEquals(results, vtnId.clearFlowFilter(tx, output));
                verify(tx).read(oper, path);
                verify(tx).delete(oper, path);
                verifyNoMoreInteractions(tx);
            }
        }
    }

    /**
     * Test case for {@link VTenantIdentifier#getType()}.
     */
    @Test
    public void testGetType() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            for (int i = 0; i < 5; i++) {
                assertEquals(VNodeType.VTN, vtnId.getType());
            }
        }
    }

    /**
     * Test case for {@link VTenantIdentifier#getVNodeIdentifier()}.
     */
    @Test
    public void testGetVNodeIdentifier() {
        VnodeName[] tnames = {new VnodeName("vtn"), new VnodeName("vtn_1")};
        for (VnodeName tname: tnames) {
            VTenantIdentifier vtnId = new VTenantIdentifier(tname);
            for (int i = 0; i < 5; i++) {
                VTenantIdentifier ident = vtnId.getVNodeIdentifier();
                assertSame(vtnId, ident);
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

        String[] tnames = {null, "vtn", "vtn_1", "vtn_2"};
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

            VTenantIdentifier vtnId1 = new VTenantIdentifier(vtnName1);
            VTenantIdentifier vtnId2 = new VTenantIdentifier(vtnName2);
            testEquals(set, vtnId1, vtnId2);
        }

        assertEquals(tnames.length, set.size());
    }

    /**
     * Test case for {@link VNodeIdentifier#toString()} and
     * {@link VNodeIdentifier#create(String)}.
     */
    @Test
    public void testToString() {
        String[] tnames = {null, "vtn", "vtn_1", "vtn_2"};
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

            VTenantIdentifier vtnId = new VTenantIdentifier(vtnName);
            String expected = "VTN:" + tnameStr;
            assertEquals(expected, vtnId.toString());

            // VNodeIdentifier.create() works only if path components are
            // valid.
            boolean valid = (tname != null);
            try {
                VNodeIdentifier<?> ident = VNodeIdentifier.create(expected);
                assertTrue(valid);
                assertEquals(vtnId, ident);

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
