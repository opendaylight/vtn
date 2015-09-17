/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditions;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatchKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowConditionKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link FlowCondUtils}.
 */
public class FlowCondUtilsTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowCondUtils#getNotFoundException(String)}</li>
     *   <li>{@link FlowCondUtils#getNotFoundException(String, Throwable)}</li>
     *   <li>{@link FlowCondUtils#getMatchIndexMissingException()}</li>
     * </ul>
     */
    @Test
    public void testException() {
        String[] names = {
            "flow_cond",
            "fcond",
            "fc",
        };

        Throwable cause = new IllegalStateException();
        for (String name: names) {
            String msg = name + ": Flow condition does not exist.";
            RpcException e = FlowCondUtils.getNotFoundException(name);
            assertEquals(null, e.getCause());
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());

            e = FlowCondUtils.getNotFoundException(name, cause);
            assertSame(cause, e.getCause());
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        String msg = "Match index cannot be null";
        RpcException e = FlowCondUtils.getMatchIndexMissingException();
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals(msg, e.getMessage());
        assertEquals(null, e.getCause());
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowCondUtils#checkName(String)}</li>
     *   <li>{@link FlowCondUtils#checkName(VnodeName)}</li>
     *   <li>{@link FlowCondUtils#getVnodeName(String)}</li>
     *   <li>{@link FlowCondUtils#getIdentifier(String)}</li>
     *   <li>{@link FlowCondUtils#getIdentifier(VnodeName)}</li>
     *   <li>{@link FlowCondUtils#getIdentifier(String, Integer)}</li>
     *   <li>{@link FlowCondUtils#getIdentifier(VnodeName, Integer)}</li>
     *   <li>{@link FlowCondUtils#getName(InstanceIdentifier)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testName() throws Exception {
        // Valid name.
        String[] names = {
            "0",
            "01",
            "012",
            "0123",
            "012345678901234567890123456789",
            "0123456789012345678901234567890",
            "a",
            "ab",
            "abc",
            "abcABC",
            "abcABC_",
            "abcABC_0123",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXX",
        };
        Integer[] indices = {
            1, 2, 3, 444, 1000, 2000, 3333, 44444, 55555, 65534, 65535,
        };

        for (String name: names) {
            VnodeName vname = new VnodeName(name);
            assertEquals(vname, FlowCondUtils.checkName(name));
            assertEquals(name, FlowCondUtils.checkName(vname));
            assertEquals(vname, FlowCondUtils.getVnodeName(name));

            VtnFlowConditionKey key = new VtnFlowConditionKey(vname);
            InstanceIdentifier<VtnFlowCondition> expected = InstanceIdentifier.
                builder(VtnFlowConditions.class).
                child(VtnFlowCondition.class, key).build();
            assertEquals(expected, FlowCondUtils.getIdentifier(name));
            assertEquals(expected, FlowCondUtils.getIdentifier(vname));
            assertEquals(name, FlowCondUtils.getName(expected));

            for (Integer idx: indices) {
                VtnFlowMatchKey mkey = new VtnFlowMatchKey(idx);
                InstanceIdentifier<VtnFlowMatch> ex = InstanceIdentifier.
                    builder(VtnFlowConditions.class).
                    child(VtnFlowCondition.class, key).
                    child(VtnFlowMatch.class, mkey).build();
                assertEquals(ex, FlowCondUtils.getIdentifier(name, idx));
                assertEquals(ex, FlowCondUtils.getIdentifier(vname, idx));
                assertEquals(name, FlowCondUtils.getName(ex));
            }

            String msg = "Match index cannot be null";
            try {
                FlowCondUtils.getIdentifier(name, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }

            try {
                FlowCondUtils.getIdentifier(vname, null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }
        }

        // Null name.
        String msg = "Flow condition name cannot be null";
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        try {
            FlowCondUtils.checkName((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        try {
            FlowCondUtils.checkName((VnodeName)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        try {
            FlowCondUtils.getVnodeName(null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        try {
            FlowCondUtils.getIdentifier((String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        try {
            FlowCondUtils.getIdentifier((String)null, Integer.valueOf(1));
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        // Empty name.
        msg = "Flow condition name cannot be empty";
        etag = RpcErrorTag.BAD_ELEMENT;
        try {
            FlowCondUtils.checkName("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        try {
            FlowCondUtils.getVnodeName("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(": Flow condition does not exist.", e.getMessage());

            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        try {
            FlowCondUtils.getIdentifier("");
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(": Flow condition does not exist.", e.getMessage());

            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        try {
            FlowCondUtils.getIdentifier("", Integer.valueOf(1));
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(": Flow condition does not exist.", e.getMessage());

            Throwable cause = e.getCause();
            assertTrue(cause instanceof RpcException);
            RpcException re = (RpcException)cause;
            assertEquals(etag, re.getErrorTag());
            assertEquals(vtag, re.getVtnErrorTag());
            assertEquals(msg, re.getMessage());
            assertEquals(null, re.getCause());
        }

        // Invalid name.
        msg = "Flow condition name is invalid";
        String[] invalidNames = {
            "01234567890123456789012345678901",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXXX",
            "_flow_cond",
            "flow-cond",
            "flow%cond",
            "_",
            " ",
            "\u3042",
        };
        for (String name: invalidNames) {
            try {
                FlowCondUtils.checkName(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                Throwable cause = e.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }

            try {
                FlowCondUtils.getVnodeName(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(name + ": Flow condition does not exist.",
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                cause = re.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }

            try {
                FlowCondUtils.getIdentifier(name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(name + ": Flow condition does not exist.",
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                cause = re.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }

            try {
                FlowCondUtils.getIdentifier(name, Integer.valueOf(1));
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(name + ": Flow condition does not exist.",
                             e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(etag, re.getErrorTag());
                assertEquals(vtag, re.getVtnErrorTag());
                assertEquals(msg, re.getMessage());
                cause = re.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }
        }

        // getName() should return null if the given path does not contain a
        // flow condition key.
        SalPort sport = new SalPort(1L, 2L);
        List<InstanceIdentifier<?>> list = new ArrayList<>();
        list.add(sport.getNodeIdentifier());
        list.add(sport.getNodeConnectorIdentifier());
        list.add(sport.getVtnNodeIdentifier());
        list.add(sport.getVtnPortIdentifier());
        for (InstanceIdentifier<?> path: list) {
            assertEquals(null, FlowCondUtils.getName(path));
        }

        VtnFlowConditionKey nullKey = new VtnFlowConditionKey((VnodeName)null);
        InstanceIdentifier<VtnFlowCondition> path = InstanceIdentifier.
            builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class, nullKey).build();
        assertEquals(null, FlowCondUtils.getName(path));

        InstanceIdentifier<VtnFlowMatch> mpath = InstanceIdentifier.
            builder(VtnFlowConditions.class).
            child(VtnFlowCondition.class, nullKey).
            child(VtnFlowMatch.class, new VtnFlowMatchKey(1)).build();
        assertEquals(null, FlowCondUtils.getName(mpath));
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link FlowCondUtils#verifyMatchIndex(Set, Integer)}</li>
     *   <li>{@link FlowCondUtils#verifyMatchIndex(Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatchIndex() throws Exception {
        Integer[] indices = {
            1, 2, 3, 444, 1000, 2000, 3333, 44444, 55555, 65534, 65535,
        };

        Set<Integer> idxSet = new HashSet<>();
        for (Integer index: indices) {
            FlowCondUtils.verifyMatchIndex(index);
            FlowCondUtils.verifyMatchIndex(idxSet, index);
        }

        // Index is null.
        String msg = "Match index cannot be null";
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        try {
            FlowCondUtils.verifyMatchIndex(null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        // Invalid index.
        Integer[] badIndices = {
            Integer.MIN_VALUE, -10000000, -3000, -200, -3, -2, -1, 0,
            65536, 65537, 70000, 1000000, Integer.MAX_VALUE,
        };
        etag = RpcErrorTag.BAD_ELEMENT;
        for (Integer index: badIndices) {
            msg = "Invalid match index: " + index;
            try {
                FlowCondUtils.verifyMatchIndex(index);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof IllegalArgumentException);
            }
        }

        // Duplicate index.
        for (Integer index: indices) {
            msg = "Duplicate match index: " + index;
            try {
                FlowCondUtils.verifyMatchIndex(idxSet, index);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }
        }
    }

    /**
     * Test case for {@link FlowCondUtils#isEmpty(VtnFlowConditions)}.
     */
    @Test
    public void testIsEmpty() {
        VtnFlowConditions root = null;
        assertEquals(true, FlowCondUtils.isEmpty(root));

        root = new VtnFlowConditionsBuilder().build();
        assertEquals(true, FlowCondUtils.isEmpty(root));

        List<VtnFlowCondition> vlist = new ArrayList<>();
        root = new VtnFlowConditionsBuilder().
            setVtnFlowCondition(vlist).build();
        assertEquals(true, FlowCondUtils.isEmpty(root));

        vlist.add(new VtnFlowConditionBuilder().build());
        assertEquals(false, FlowCondUtils.isEmpty(root));
    }

    /**
     * Test case for
     * {@link FlowCondUtils#checkPresent(ReadTransaction, VnodeName)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckPresent() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        List<VnodeName> names = new ArrayList<>();
        List<InstanceIdentifier<VtnFlowCondition>> paths = new ArrayList<>();
        Set<VnodeName> present = new HashSet<>();

        for (int i = 0; i < 10; i++) {
            String name = "present" + i;
            VnodeName vname = new VnodeName(name);
            names.add(vname);
            assertEquals(true, present.add(vname));
            VtnFlowCondition vfc = new VtnFlowConditionBuilder().
                setName(vname).build();
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(vname);
            paths.add(path);
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfc));
        }

        VtnFlowCondition vfcNull = null;
        for (int i = 0; i < 10; i++) {
            String name = "notfound" + i;
            VnodeName vname = new VnodeName(name);
            names.add(vname);
            assertEquals(false, present.contains(vname));
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(vname);
            paths.add(path);
            Mockito.when(rtx.read(oper, path)).
                thenReturn(getReadResult(vfcNull));
        }

        for (VnodeName vname: names) {
            try {
                FlowCondUtils.checkPresent(rtx, vname);
                assertEquals(true, present.contains(vname));
            } catch (RpcException e) {
                assertEquals(false, present.contains(vname));
                assertEquals(null, e.getCause());
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                String msg = vname.getValue() +
                    ": Flow condition does not exist.";
                assertEquals(msg, e.getMessage());
            }
        }

        for (InstanceIdentifier<VtnFlowCondition> path: paths) {
            Mockito.verify(rtx).read(oper, path);
        }
    }

    /**
     * Test case for {@link FlowCondUtils#readFlowConditions(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadConditions() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnFlowConditions> path =
            InstanceIdentifier.create(VtnFlowConditions.class);

        // Root container is not present.
        VtnFlowConditions root = null;
        List<VTNFlowCondition> expected = new ArrayList<>();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(expected, FlowCondUtils.readFlowConditions(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);

        // Flow condition list is null.
        root = new VtnFlowConditionsBuilder().build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(expected, FlowCondUtils.readFlowConditions(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);

        // Flow condition list is empty.
        List<VtnFlowCondition> vlist = new ArrayList<>();
        root = new VtnFlowConditionsBuilder().
            setVtnFlowCondition(vlist).build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(expected, FlowCondUtils.readFlowConditions(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);

        // Flow conditions are present.
        Map<FlowCondParams, FlowCondParams> cases =
            FlowCondParams.createFlowConditions();
        for (FlowCondParams params: cases.values()) {
            vlist.add(params.toVtnFlowConditionBuilder().build());
            expected.add(params.toVTNFlowCondition());
        }
        root = new VtnFlowConditionsBuilder().
            setVtnFlowCondition(vlist).build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(expected, FlowCondUtils.readFlowConditions(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);
    }

    /**
     * Test case for
     * {@link FlowCondUtils#readFlowCondition(ReadTransaction, String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadCondition() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Null name.
        try {
            FlowCondUtils.readFlowCondition(rtx, (String)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Flow condition name cannot be null", e.getMessage());
        }

        // Invalid name should be treated as if the specified flow condition
        // is not present.
        String[] invalidNames = {
            "",
            "01234567890123456789012345678901",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXXX",
            "_flow_cond",
            "flow-cond",
            "flow%cond",
            "_",
            " ",
            "\u3042",
        };
        for (String name: invalidNames) {
            try {
                FlowCondUtils.readFlowCondition(rtx, name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                String msg = name + ": Flow condition does not exist.";
                assertEquals(msg, e.getMessage());
            }
        }
        Mockito.verify(rtx, Mockito.never()).
            read(Mockito.any(LogicalDatastoreType.class),
                 (InstanceIdentifier<?>)Mockito.any(InstanceIdentifier.class));

        // Flow condition is not present.
        VtnFlowCondition vfc = null;
        Map<FlowCondParams, FlowCondParams> cases =
            FlowCondParams.createFlowConditions();
        for (FlowCondParams params: cases.values()) {
            String name = params.getName();
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(name);
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfc));
            try {
                FlowCondUtils.readFlowCondition(rtx, name);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                String msg = name + ": Flow condition does not exist.";
                assertEquals(msg, e.getMessage());
            }
            Mockito.verify(rtx).read(oper, path);
        }

        // Flow condition is present.
        for (FlowCondParams params: cases.values()) {
            String name = params.getName();
            InstanceIdentifier<VtnFlowCondition> path =
                FlowCondUtils.getIdentifier(name);
            vfc = params.toVtnFlowConditionBuilder().build();
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfc));
            assertEquals(params.toVTNFlowCondition(),
                         FlowCondUtils.readFlowCondition(rtx, name));
            Mockito.verify(rtx, Mockito.times(2)).read(oper, path);
        }
    }

    /**
     * Test case for
     * {@link FlowCondUtils#readFlowMatch(ReadTransaction, String, int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadMatch() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Null name.
        try {
            FlowCondUtils.readFlowMatch(rtx, (String)null, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Flow condition name cannot be null", e.getMessage());
        }

        // Invalid name should be treated as if the specified flow condition
        // is not present.
        String[] invalidNames = {
            "",
            "01234567890123456789012345678901",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXXX",
            "_flow_cond",
            "flow-cond",
            "flow%cond",
            "_",
            " ",
            "\u3042",
        };
        for (String name: invalidNames) {
            try {
                FlowCondUtils.readFlowMatch(rtx, name, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                String msg = name + ": Flow condition does not exist.";
                assertEquals(msg, e.getMessage());
            }
        }
        Mockito.verify(rtx, Mockito.never()).
            read(Mockito.any(LogicalDatastoreType.class),
                 (InstanceIdentifier<?>)Mockito.any(InstanceIdentifier.class));

        VtnFlowCondition vfc = new VtnFlowConditionBuilder().build();
        VtnFlowCondition vfcNull = null;
        VtnFlowMatch vfm = null;
        Map<FlowMatchParams, FlowMatchParams> cases =
            FlowMatchParams.createFlowMatches();
        String name = "fcond";
        InstanceIdentifier<VtnFlowCondition> fcPath =
            FlowCondUtils.getIdentifier(name);
        String msg = name + ": Flow condition does not exist.";
        int fcCount = 0;
        for (FlowMatchParams params: cases.values()) {
            // Flow match is not present.
            Integer index = params.getIndex();
            InstanceIdentifier<VtnFlowMatch> path =
                FlowCondUtils.getIdentifier(name, index);
            Mockito.when(rtx.read(oper, fcPath)).thenReturn(getReadResult(vfc));
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfm));
            assertEquals(null, FlowCondUtils.readFlowMatch(rtx, name, index));
            fcCount++;
            Mockito.verify(rtx, Mockito.times(fcCount)).read(oper, fcPath);
            Mockito.verify(rtx).read(oper, path);

            // Flow condition is not present.
            Mockito.when(rtx.read(oper, fcPath)).
                thenReturn(getReadResult(vfcNull));
            try {
                FlowCondUtils.readFlowMatch(rtx, name, index);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
                assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            fcCount++;
            Mockito.verify(rtx, Mockito.times(fcCount)).read(oper, fcPath);
            Mockito.verify(rtx, Mockito.times(2)).read(oper, path);
        }

        // Flow match is present.
        for (FlowMatchParams params: cases.values()) {
            Integer index = params.getIndex();
            InstanceIdentifier<VtnFlowMatch> path =
                FlowCondUtils.getIdentifier(name, index);
            vfm = params.toVtnFlowMatchBuilder().build();
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vfm));
            assertEquals(params.toVTNFlowMatch(),
                         FlowCondUtils.readFlowMatch(rtx, name, index));
            Mockito.verify(rtx, Mockito.times(fcCount)).read(oper, fcPath);
            Mockito.verify(rtx, Mockito.times(3)).read(oper, path);
        }
    }
}
