/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathmap;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MAX;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestVnodeName;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.GlobalPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.set.path.map.input.PathMapListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMapKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMapsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link PathMapUtils}.
 */
public class PathMapUtilsTest extends TestBase {
    /**
     * Pseudo random number generator.
     */
    private final Random  random = new Random();

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link PathMapUtils#getNullMapIndexException()}</li>
     *   <li>{@link PathMapUtils#getInvalidMapIndexException(Integer, Throwable)}</li>
     * </ul>
     */
    @Test
    public void testException() {
        RpcException e = PathMapUtils.getNullMapIndexException();
        assertEquals(null, e.getCause());
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals("Path map index cannot be null", e.getMessage());

        for (int i = 0; i <= 10; i++) {
            IllegalArgumentException cause = new IllegalArgumentException();
            e = PathMapUtils.getInvalidMapIndexException(i, cause);
            assertEquals(cause, e.getCause());
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Invalid path map index: " + i, e.getMessage());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link PathMapUtils#toVtnPathMapBuilder(VtnPathMapConfig)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConversion() throws Exception {
        // Valid path map configuration.
        Integer[] indices = {
            1, 2, 333, 65534, 65535,
        };
        String[] conditions = {
            "a",
            "fcond",
            "0123456789012345678901234567890",
        };
        Integer[] timeouts = {
            null, 0, 1, 4000, 65534, 65535,
        };
        List<Integer> policies = new ArrayList<>();
        policies.add(null);
        for (int policy = 0; policy <= PATH_POLICY_MAX; policy++) {
            policies.add(Integer.valueOf(policy));
        }

        for (Integer index: indices) {
            for (String cond: conditions) {
                for (Integer policy: policies) {
                    for (Integer tmout: timeouts) {
                        if (tmout == null) {
                            conversionTest(index, cond, policy, tmout, tmout);
                            continue;
                        }

                        conversionTest(index, cond, policy, tmout, 0);
                        conversionTest(index, cond, policy, 0, tmout);
                        int tm = tmout.intValue();
                        if (tm > 1) {
                            conversionTest(index, cond, policy, tm - 1, tmout);
                        }
                    }
                }
            }
        }

        // Null VtnPathMapConfig.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        VtnPathMapConfig vpmc = null;
        String msg = "Path map configuration cannot be null";
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Map index is null.
        msg = "Path map index cannot be null";
        String condition = "fcond";
        VnodeName vcondition = new VnodeName(condition);
        vpmc = new VtnPathMapBuilder().
            setCondition(vcondition).build();
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Null flow condition.
        msg = "Flow condition name cannot be null";
        vpmc = new VtnPathMapBuilder().setIndex(1).build();
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        vpmc = mock(VtnPathMap.class);
        when(vpmc.getIndex()).thenReturn(1);
        when(vpmc.getCondition()).thenReturn(new TestVnodeName());
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Empty flow condition.
        etag = RpcErrorTag.BAD_ELEMENT;
        msg = "Flow condition name cannot be empty";
        vpmc = mock(VtnPathMap.class);
        when(vpmc.getIndex()).thenReturn(1);
        when(vpmc.getCondition()).thenReturn(new TestVnodeName(""));
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid flow condition name.
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
        msg = "Flow condition name is invalid";
        for (String name: invalidNames) {
            vpmc = mock(VtnPathMap.class);
            when(vpmc.getIndex()).thenReturn(1);
            when(vpmc.getCondition()).thenReturn(new TestVnodeName(name));
            try {
                PathMapUtils.toVtnPathMapBuilder(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(IllegalArgumentException.class,
                             e.getCause().getClass());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid path policy ID.
        Integer[] invalidPolicies = {
            Integer.MIN_VALUE, -9999999, -333333, -2, -1,
            PATH_POLICY_MAX + 1, PATH_POLICY_MAX + 2,
            10000000, 222222222, Integer.MAX_VALUE,
        };
        vcondition = new VnodeName(condition);
        for (Integer policy: invalidPolicies) {
            msg = "Invalid path policy ID: " + policy;
            vpmc = mock(VtnPathMapConfig.class);
            when(vpmc.getIndex()).thenReturn(1);
            when(vpmc.getCondition()).thenReturn(vcondition);
            when(vpmc.getPolicy()).thenReturn(policy);
            try {
                PathMapUtils.toVtnPathMapBuilder(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(IllegalArgumentException.class,
                             e.getCause().getClass());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid idle/hard timeout.
        Integer[] invalidTimeouts = {
            Integer.MIN_VALUE, -99999999, -65535, -3333, -2, -1,
            65536, 65537, 1000000, 33333333, Integer.MAX_VALUE,
        };
        for (Integer timeout: invalidTimeouts) {
            msg = "Invalid idle-timeout: " + timeout;
            vpmc = mock(VtnPathMapConfig.class);
            when(vpmc.getIndex()).thenReturn(1);
            when(vpmc.getCondition()).thenReturn(vcondition);
            when(vpmc.getPolicy()).thenReturn(0);
            when(vpmc.getIdleTimeout()).thenReturn(timeout);
            try {
                PathMapUtils.toVtnPathMapBuilder(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            msg = "Invalid hard-timeout: " + timeout;
            vpmc = mock(VtnPathMapConfig.class);
            when(vpmc.getIndex()).thenReturn(1);
            when(vpmc.getCondition()).thenReturn(vcondition);
            when(vpmc.getPolicy()).thenReturn(0);
            when(vpmc.getHardTimeout()).thenReturn(timeout);
            try {
                PathMapUtils.toVtnPathMapBuilder(vpmc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Inconsistent flow timeouts.
        msg = "idle-timeout must be specified.";
        vpmc = new VtnPathMapBuilder().setIndex(1).setCondition(vcondition).
            setPolicy(0).setHardTimeout(0).build();
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        msg = "hard-timeout must be specified.";
        vpmc = new VtnPathMapBuilder().setIndex(1).setCondition(vcondition).
            setPolicy(0).setIdleTimeout(0).build();
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        timeouts = new Integer[]{
            1, 4000, 65534, 65535,
        };
        msg = "idle-timeout must be less than hard-timeout.";
        List<VtnPathMapConfig> vpmcList = new ArrayList<>();
        for (Integer tmout: timeouts) {
            int timeout = tmout.intValue();
            vpmcList.add(new VtnPathMapBuilder().setIndex(1).
                         setCondition(vcondition).setPolicy(0).
                         setIdleTimeout(tmout).setHardTimeout(tmout).build());

            if (timeout > 1) {
                int hard = timeout - 1;
                vpmcList.add(new VtnPathMapBuilder().setIndex(1).
                             setCondition(vcondition).setPolicy(0).
                             setIdleTimeout(tmout).setHardTimeout(hard).
                             build());
            }
        }

        for (VtnPathMapConfig vpm: vpmcList) {
            try {
                PathMapUtils.toVtnPathMapBuilder(vpm);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link PathMapUtils#verifyMapIndex(Set, Integer)}.
     *
     * @throws Exception  An error occurrred.
     */
    @Test
    public void testVerifyMapIndex() throws Exception {
        Set<Integer> indices = new HashSet<>();
        for (int i = 1; i <= 100; i++) {
            PathMapUtils.verifyMapIndex(indices, i);
        }
        for (int i = 65500; i <= 65535; i++) {
            PathMapUtils.verifyMapIndex(indices, i);
        }

        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        for (int i = 1; i <= 100; i++) {
            String msg = "Duplicate map index: " + i;
            try {
                PathMapUtils.verifyMapIndex(indices, i);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
        for (int i = 65500; i <= 65535; i++) {
            String msg = "Duplicate map index: " + i;
            try {
                PathMapUtils.verifyMapIndex(indices, i);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link PathMapUtils#getIndex(InstanceIdentifier)}</li>
     *   <li>{@link PathMapUtils#getIdentifier(Integer)}</li>
     *   <li>{@link PathMapUtils#getIdentifier(String, Integer)}</li>
     *   <li>{@link PathMapUtils#getIdentifier(VTenantIdentifier, Integer)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetIdentifier() throws Exception {
        Set<Integer> indices = new HashSet<>();
        do {
            int index = random.nextInt(65536);
            if (index != 0) {
                indices.add(index);
            }
        } while (indices.size() < 100);

        String[] tenants = {
            "vtn_1", "vtn_2", "vtn_3",
        };
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "Path map index cannot be null";

        for (Integer index: indices) {
            // Global path map identifier test.
            VtnPathMapKey key = new VtnPathMapKey(index);
            InstanceIdentifier<VtnPathMap> expected = InstanceIdentifier.
                builder(GlobalPathMaps.class).
                child(VtnPathMap.class, key).build();
            assertEquals(expected, PathMapUtils.getIdentifier(index));
            assertEquals(index, PathMapUtils.getIndex(expected));

            try {
                PathMapUtils.getIdentifier((Integer)null);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            // VTN path map identifier test.
            for (String tname: tenants) {
                VnodeName vname = new VnodeName(tname);
                VTenantIdentifier ident = new VTenantIdentifier(vname);
                expected = InstanceIdentifier.builder(Vtns.class).
                    child(Vtn.class, new VtnKey(vname)).
                    child(VtnPathMaps.class).
                    child(VtnPathMap.class, key).build();
                assertEquals(expected,
                             PathMapUtils.getIdentifier(tname, index));
                assertEquals(expected,
                             PathMapUtils.getIdentifier(ident, index));
                assertEquals(index, PathMapUtils.getIndex(expected));

                try {
                    PathMapUtils.getIdentifier(tname, (Integer)null);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }

                try {
                    PathMapUtils.getIdentifier(ident, (Integer)null);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            }
        }

        // getIndex() should return null if the given path does not contain
        // a path map index.
        SalPort sport = new SalPort(1L, 2L);
        List<InstanceIdentifier<?>> list = new ArrayList<>();
        list.add(sport.getNodeIdentifier());
        list.add(sport.getNodeConnectorIdentifier());
        list.add(sport.getVtnNodeIdentifier());
        list.add(sport.getVtnPortIdentifier());
        for (InstanceIdentifier<?> path: list) {
            assertEquals(null, PathMapUtils.getIndex(path));
        }

        // VTN name is null.
        msg = "VTN name cannot be null";
        try {
            PathMapUtils.getIdentifier((String)null, 1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid VTN name.
        String[] invalidNames = {
            "",
            "01234567890123456789012345678901",
            "abcABC_0123_XXXXXXXXXXXXXXXXXXXX",
            "_vtn_1",
            "vtn-1",
            "tenant%1",
            "_",
            " ",
            "\u3042",
        };
        etag = RpcErrorTag.DATA_MISSING;
        vtag = VtnErrorTag.NOTFOUND;
        for (String name: invalidNames) {
            msg = name + ": VTN does not exist.";
            try {
                PathMapUtils.getIdentifier(name, 1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());

                Throwable cause = e.getCause();
                assertTrue(cause instanceof RpcException);
                RpcException re = (RpcException)cause;
                assertEquals(RpcErrorTag.BAD_ELEMENT, re.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, re.getVtnErrorTag());
                cause = re.getCause();
                if (name.isEmpty()) {
                    assertEquals("VTN name cannot be empty",
                                 re.getMessage());
                    assertEquals(null, cause);
                } else {
                    assertEquals("VTN name is invalid", re.getMessage());
                    assertTrue(cause instanceof IllegalArgumentException);
                }
            }
        }
    }

    /**
     * Test case for {@link PathMapUtils#isEmpty(VtnPathMapList)}.
     */
    @Test
    public void testIsEmpty() {
        VtnPathMapList vplist = null;
        assertEquals(true, PathMapUtils.isEmpty(vplist));

        vplist = new GlobalPathMapsBuilder().build();
        assertEquals(true, PathMapUtils.isEmpty(vplist));

        List<VtnPathMap> vlist = new ArrayList<>();
        vplist = new GlobalPathMapsBuilder().setVtnPathMap(vlist).build();
        assertEquals(true, PathMapUtils.isEmpty(vplist));

        vlist.add(new VtnPathMapBuilder().build());
        vplist = new GlobalPathMapsBuilder().setVtnPathMap(vlist).build();
        assertEquals(false, PathMapUtils.isEmpty(vplist));

        vplist = new VtnPathMapsBuilder().build();
        assertEquals(true, PathMapUtils.isEmpty(vplist));

        vlist = new ArrayList<VtnPathMap>();
        vplist = new VtnPathMapsBuilder().setVtnPathMap(vlist).build();
        assertEquals(true, PathMapUtils.isEmpty(vplist));

        vlist.add(new VtnPathMapBuilder().build());
        vplist = new VtnPathMapsBuilder().setVtnPathMap(vlist).build();
        assertEquals(false, PathMapUtils.isEmpty(vplist));
    }

    /**
     * Test case for {@link PathMapUtils#readPathMaps(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadGlobalPathMaps() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<GlobalPathMaps> path = InstanceIdentifier.
            create(GlobalPathMaps.class);

        // Root container is not present.
        GlobalPathMaps root = null;
        List<VtnPathMap> vlist = new ArrayList<>();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(vlist, PathMapUtils.readPathMaps(rtx));
        verify(rtx).read(oper, path);
        reset(rtx);

        // Global path map list is null.
        root = new GlobalPathMapsBuilder().build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(vlist, PathMapUtils.readPathMaps(rtx));
        verify(rtx).read(oper, path);
        reset(rtx);

        // Global path map list is empty.
        root = new GlobalPathMapsBuilder().
            setVtnPathMap(Collections.<VtnPathMap>emptyList()).build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(vlist, PathMapUtils.readPathMaps(rtx));
        verify(rtx).read(oper, path);
        reset(rtx);

        // Global path maps are present.
        Map<Integer, VtnPathMap> maps = new HashMap<>();
        List<VtnPathMap> vpmList = createVtnPathMaps(100, maps);
        root = new GlobalPathMapsBuilder().setVtnPathMap(vpmList).build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        vlist = PathMapUtils.readPathMaps(rtx);
        verify(rtx).read(oper, path);
        assertEquals(maps.size(), vlist.size());

        // Path maps should be sorted by index in ascending order.
        int prev = 0;
        for (VtnPathMap vpm: vlist) {
            Integer idx = vpm.getIndex();
            assertEquals(vpm, maps.get(idx));
            int index = idx.intValue();
            assertTrue(prev < index);
            prev = index;
        }
        assertEquals(65535, prev);
    }

    /**
     * Test case for {@link PathMapUtils#readPathMap(ReadTransaction, Integer)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadGlobalPathMap() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Map index is null.
        try {
            PathMapUtils.readPathMap(rtx, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Path map index cannot be null", e.getMessage());
        }
        verify(rtx, never()).
            read(any(LogicalDatastoreType.class),
                 (InstanceIdentifier<?>)any(InstanceIdentifier.class));

        // Create test data.
        Map<Integer, VtnPathMap> maps = new HashMap<>();
        List<InstanceIdentifier<VtnPathMap>> pathList = new ArrayList<>();
        do {
            int index = random.nextInt(65536);
            if (index == 0) {
                continue;
            }
            Integer idx = Integer.valueOf(index);
            if (maps.containsKey(idx)) {
                continue;
            }

            InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
                builder(GlobalPathMaps.class).
                child(VtnPathMap.class, new VtnPathMapKey(idx)).build();
            pathList.add(path);
            VtnPathMap vpm;
            if ((maps.size() % 3) == 0) {
                // Not present.
                vpm = null;
            } else {
                int policy = random.nextInt(PATH_POLICY_MAX + 1);
                VnodeName vcond = new VnodeName("fcond" + index);
                vpm = new VtnPathMapBuilder().
                    setIndex(idx).setCondition(vcond).setPolicy(policy).
                    build();
            }
            maps.put(idx, vpm);
            when(rtx.read(oper, path)).thenReturn(getReadResult(vpm));
        } while (maps.size() < 100);

        // Run tests.
        for (Integer index: maps.keySet()) {
            VtnPathMap expected = maps.get(index);
            assertEquals(expected, PathMapUtils.readPathMap(rtx, index));
        }
        for (InstanceIdentifier<VtnPathMap> path: pathList) {
            verify(rtx).read(oper, path);
        }
    }

    /**
     * Test case for
     * {@link PathMapUtils#readPathMaps(ReadTransaction, VTenantIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtnPathMaps() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // VTN is not present.
        VnodeName vname = new VnodeName("vtn1");
        VTenantIdentifier ident = new VTenantIdentifier(vname);
        VtnKey vtnKey = new VtnKey(vname);
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).child(Vtn.class, vtnKey).build();
        InstanceIdentifier<VtnPathMaps> path = InstanceIdentifier.
            builder(Vtns.class).child(Vtn.class, vtnKey).
            child(VtnPathMaps.class).build();
        VtnPathMaps root = null;
        Vtn vtn = null;
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
        VtnErrorTag vtag = VtnErrorTag.NOTFOUND;
        String msg = ident + ": VTN does not exist.";
        try {
            PathMapUtils.readPathMaps(rtx, ident);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
        verify(rtx).read(oper, path);
        verify(rtx).read(oper, vtnPath);
        reset(rtx);

        // VTN path map container is not present.
        vtn = new VtnBuilder().build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        assertEquals(Collections.<VtnPathMap>emptyList(),
                     PathMapUtils.readPathMaps(rtx, ident));
        verify(rtx).read(oper, path);
        verify(rtx).read(oper, vtnPath);
        reset(rtx);

        // VTN path map list is null.
        root = new VtnPathMapsBuilder().build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        assertEquals(Collections.<VtnPathMap>emptyList(),
                     PathMapUtils.readPathMaps(rtx, ident));
        verify(rtx).read(oper, path);
        verify(rtx, never()).read(oper, vtnPath);
        reset(rtx);

        // VTN path map list is empty.
        List<VtnPathMap> vlist = new ArrayList<>();
        root = new VtnPathMapsBuilder().setVtnPathMap(vlist).build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        assertEquals(Collections.<VtnPathMap>emptyList(),
                     PathMapUtils.readPathMaps(rtx, ident));
        verify(rtx).read(oper, path);
        verify(rtx, never()).read(oper, vtnPath);
        reset(rtx);

        // VTN path maps are present.
        Map<Integer, VtnPathMap> maps = new HashMap<>();
        List<VtnPathMap> vpmList = createVtnPathMaps(100, maps);
        root = new VtnPathMapsBuilder().setVtnPathMap(vpmList).build();
        when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        vlist = PathMapUtils.readPathMaps(rtx, ident);
        verify(rtx).read(oper, path);
        verify(rtx, never()).read(oper, vtnPath);
        assertEquals(maps.size(), vlist.size());

        // Path maps should be sorted by index in ascending order.
        int prev = 0;
        for (VtnPathMap vpm: vlist) {
            Integer idx = vpm.getIndex();
            assertEquals(vpm, maps.get(idx));
            int index = idx.intValue();
            assertTrue(prev < index);
            prev = index;
        }
        assertEquals(65535, prev);
    }

    /**
     * Test case for
     * {@link PathMapUtils#readPathMap(ReadTransaction, VTenantIdentifier, Integer)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtnPathMap() throws Exception {
        // Set up mock-up of MD-SAL read transaction.
        ReadTransaction rtx = mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Map index is null.
        try {
            VTenantIdentifier ident =
                new VTenantIdentifier(new VnodeName("vtn"));
            PathMapUtils.readPathMap(rtx, ident, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Path map index cannot be null", e.getMessage());
        }
        verify(rtx, never()).
            read(any(LogicalDatastoreType.class),
                 (InstanceIdentifier<?>)any(InstanceIdentifier.class));

        // Create test data.
        Map<Integer, VtnPathMap> maps = new HashMap<>();
        Set<Integer> notPresent = new HashSet<>();
        VnodeName vname1 = new VnodeName("vtn1");
        VnodeName vname2 = new VnodeName("vtn2");
        VTenantIdentifier ident1 = new VTenantIdentifier(vname1);
        VTenantIdentifier ident2 = new VTenantIdentifier(vname2);
        VtnKey vkey1 = new VtnKey(vname1);
        VtnKey vkey2 = new VtnKey(vname2);
        List<InstanceIdentifier<VtnPathMap>> pathList1 = new ArrayList<>();
        List<InstanceIdentifier<VtnPathMap>> pathList2 = new ArrayList<>();
        InstanceIdentifier<Vtn> vtnPath1 = InstanceIdentifier.
            builder(Vtns.class).child(Vtn.class, vkey1).build();
        InstanceIdentifier<Vtn> vtnPath2 = InstanceIdentifier.
            builder(Vtns.class).child(Vtn.class, vkey2).build();
        when(rtx.read(oper, vtnPath1)).
            thenReturn(getReadResult(new VtnBuilder().build()));
        when(rtx.read(oper, vtnPath2)).
            thenReturn(getReadResult((Vtn)null));
        do {
            int index = random.nextInt(65536);
            if (index == 0) {
                continue;
            }
            Integer idx = Integer.valueOf(index);
            if (maps.containsKey(idx)) {
                continue;
            }

            VtnPathMapKey mapKey = new VtnPathMapKey(idx);
            InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
                builder(Vtns.class).child(Vtn.class, vkey1).
                child(VtnPathMaps.class).child(VtnPathMap.class, mapKey).
                build();
            pathList1.add(path);
            VtnPathMap vpm;
            if ((maps.size() % 3) == 0) {
                // Not present.
                notPresent.add(idx);
                vpm = null;
            } else {
                int policy = random.nextInt(PATH_POLICY_MAX + 1);
                VnodeName vcond = new VnodeName("fcond" + index);
                vpm = new VtnPathMapBuilder().
                    setIndex(idx).setCondition(vcond).setPolicy(policy).
                    build();
            }
            maps.put(idx, vpm);
            when(rtx.read(oper, path)).thenReturn(getReadResult(vpm));

            path = InstanceIdentifier.builder(Vtns.class).
                child(Vtn.class, vkey2).child(VtnPathMaps.class).
                child(VtnPathMap.class, mapKey).build();
            pathList2.add(path);
            when(rtx.read(oper, path)).
                thenReturn(getReadResult((VtnPathMap)null));
        } while (maps.size() < 100);

        // Run tests.
        RpcErrorTag etag = RpcErrorTag.DATA_MISSING;
        VtnErrorTag vtag = VtnErrorTag.NOTFOUND;
        String msg = ident2 + ": VTN does not exist.";
        for (Integer index: maps.keySet()) {
            VtnPathMap expected = maps.get(index);
            assertEquals(expected,
                         PathMapUtils.readPathMap(rtx, ident1, index));

            // vtn2 is not present.
            try {
                PathMapUtils.readPathMap(rtx, ident2, index);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
        for (InstanceIdentifier<VtnPathMap> path: pathList1) {
            verify(rtx).read(oper, path);
        }
        for (InstanceIdentifier<VtnPathMap> path: pathList2) {
            verify(rtx).read(oper, path);
        }

        verify(rtx, times(notPresent.size())).
            read(oper, vtnPath1);
        verify(rtx, times(maps.size())).
            read(oper, vtnPath2);
    }

    /**
     * Test case for {@link PathMapUtils#isUpdated(VtnPathMap, VtnPathMap)}.
     */
    @Test
    public void testIsUpdated() {
        VtnPathMapBuilder builder = new VtnPathMapBuilder();
        VtnPathMap old = builder.build();
        VtnPathMap vpm = builder.build();
        assertFalse(PathMapUtils.isUpdated(old, vpm));

        // Change flow condition name.
        String[] conditions = {"cond", "cond_1", "cond_2"};
        for (String cond: conditions) {
            vpm = builder.setCondition(new VnodeName(cond)).build();
            assertTrue(PathMapUtils.isUpdated(old, vpm));
            old = builder.build();
            assertFalse(PathMapUtils.isUpdated(old, vpm));
        }

        // Change path policy ID.
        for (int policy = 0; policy <= PATH_POLICY_MAX; policy++) {
            vpm = builder.setPolicy(policy).build();
            assertTrue(PathMapUtils.isUpdated(old, vpm));
            old = builder.build();
            assertFalse(PathMapUtils.isUpdated(old, vpm));
        }

        // Change idle-timeout.
        int[] idles = {1, 300, 4567, 60000};
        for (int idle: idles) {
            vpm = builder.setIdleTimeout(idle).build();
            assertTrue(PathMapUtils.isUpdated(old, vpm));
            old = builder.build();
            assertFalse(PathMapUtils.isUpdated(old, vpm));
        }

        // Change hard-timeout.
        int[] hards = {2, 123, 6789, 65535};
        for (int hard: hards) {
            vpm = builder.setHardTimeout(hard).build();
            assertTrue(PathMapUtils.isUpdated(old, vpm));
            old = builder.build();
            assertFalse(PathMapUtils.isUpdated(old, vpm));
        }
    }

    /**
     * Test case for {@link PathMapUtils#toString(VtnPathMap)}.
     */
    @Test
    public void testToString() {
        Integer[] indices = {1, 2, 9999};
        String[] conditions = {"cond_1", "cond_2", "condition"};
        Integer[] policies = {0, 1, 2, 3};

        String sep = ", ";
        for (Integer index: indices) {
            VtnPathMapBuilder builder = new VtnPathMapBuilder().
                setIndex(index);
            for (String cond: conditions) {
                builder.setCondition(new VnodeName(cond));
                for (Integer policy: policies) {
                    String vpmStr = "index=" + index + sep +
                        "cond=" + cond + sep +
                        "policy=" + policy;
                    VtnPathMap vpm = builder.setPolicy(policy).build();
                    assertEquals(vpmStr, PathMapUtils.toString(vpm));

                    Integer idle = 12345;
                    vpm = builder.setIdleTimeout(idle).build();
                    String str = vpmStr + sep + "idle=" + idle;
                    assertEquals(str, PathMapUtils.toString(vpm));

                    idle = null;
                    Integer hard = 60000;
                    vpm = builder.setIdleTimeout(idle).
                        setHardTimeout(hard).build();
                    str = vpmStr + sep + "hard=" + hard;
                    assertEquals(str, PathMapUtils.toString(vpm));

                    idle = 3333;
                    hard = 45678;
                    vpm = builder.setIdleTimeout(idle).
                        setHardTimeout(hard).build();
                    str = vpmStr + sep + "idle=" + idle + sep + "hard=" + hard;
                    assertEquals(str, PathMapUtils.toString(vpm));

                    builder.setIdleTimeout(null).setHardTimeout(null);
                }
            }
        }
    }

    /**
     * Test case for
     * {@link PathMapUtils#log(Logger,IdentifiedData,VtnUpdateType)}.
     *
     * <ul>
     *   <li>Global path map has been created.</li>
     * </ul>
     */
    @Test
    public void testLogGlobalCreated() {
        Logger logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);

        // No flow timeout
        Integer index = 123;
        String cond = "cond_1";
        Integer policy = 0;
        VtnPathMap vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            build();
        VtnPathMapKey vpmKey = new VtnPathMapKey(index);
        InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
            builder(GlobalPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        IdentifiedData<?> data = new IdentifiedData<VtnPathMap>(path, vpm);
        String desc = "Global";
        String value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy;
        VtnUpdateType type = VtnUpdateType.CREATED;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "created", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);

        // With flow timeout
        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);
        index = 333;
        cond = "cond_2";
        policy = 3;
        Integer idle = 5000;
        Integer hard = 34567;
        vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            build();
        vpmKey = new VtnPathMapKey(index);
        path = InstanceIdentifier.
            builder(GlobalPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        data = new IdentifiedData<VtnPathMap>(path, vpm);
        value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy + ", idle=" + idle + ", hard=" + hard;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "created", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for
     * {@link PathMapUtils#log(Logger,IdentifiedData,VtnUpdateType)}.
     *
     * <ul>
     *   <li>Global path map has been removed.</li>
     * </ul>
     */
    @Test
    public void testLogGlobalRemoved() {
        Logger logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);

        // No flow timeout
        Integer index = 9999;
        String cond = "cond_3";
        Integer policy = 3;
        VtnPathMap vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            build();
        VtnPathMapKey vpmKey = new VtnPathMapKey(index);
        InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
            builder(GlobalPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        IdentifiedData<?> data = new IdentifiedData<VtnPathMap>(path, vpm);
        String desc = "Global";
        String value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy;
        VtnUpdateType type = VtnUpdateType.REMOVED;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "removed", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);

        // With flow timeout
        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);
        index = 890;
        cond = "cond_4";
        policy = 2;
        Integer idle = 0;
        Integer hard = 0;
        vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            build();
        vpmKey = new VtnPathMapKey(index);
        path = InstanceIdentifier.
            builder(GlobalPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        data = new IdentifiedData<VtnPathMap>(path, vpm);
        value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy + ", idle=" + idle + ", hard=" + hard;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "removed", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for
     * {@link PathMapUtils#log(Logger,IdentifiedData,VtnUpdateType)}.
     *
     * <ul>
     *   <li>VTN path map has been created.</li>
     * </ul>
     */
    @Test
    public void testLogVtnCreated() {
        Logger logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);

        // No flow timeout
        String tname = "vtn_1";
        VtnKey vtnKey = new VtnKey(new VnodeName(tname));
        VTenantIdentifier vtnId = new VTenantIdentifier(vtnKey.getName());
        Integer index = 1230;
        String cond = "cond_10";
        Integer policy = 1;
        VtnPathMap vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            build();
        VtnPathMapKey vpmKey = new VtnPathMapKey(index);
        InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey).
            child(VtnPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        IdentifiedData<?> data = new IdentifiedData<VtnPathMap>(path, vpm);
        String desc = vtnId.toString() + ": VTN";
        String value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy;
        VtnUpdateType type = VtnUpdateType.CREATED;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "created", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);

        // With flow timeout
        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);
        tname = "vtn_2";
        vtnKey = new VtnKey(new VnodeName(tname));
        vtnId = new VTenantIdentifier(vtnKey.getName());
        index = 10203;
        cond = "cond_20";
        policy = 0;
        Integer idle = 65534;
        Integer hard = 65535;
        vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            build();
        vpmKey = new VtnPathMapKey(index);
        path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey).
            child(VtnPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        data = new IdentifiedData<VtnPathMap>(path, vpm);
        desc = vtnId.toString() + ": VTN";
        value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy + ", idle=" + idle + ", hard=" + hard;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "created", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);
    }


    /**
     * Test case for
     * {@link PathMapUtils#log(Logger,IdentifiedData,VtnUpdateType)}.
     *
     * <ul>
     *   <li>VTN path map has been removed.</li>
     * </ul>
     */
    @Test
    public void testLogVtnRemoved() {
        Logger logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);

        // No flow timeout
        String tname = "vtn_3";
        VtnKey vtnKey = new VtnKey(new VnodeName(tname));
        VTenantIdentifier vtnId = new VTenantIdentifier(vtnKey.getName());
        Integer index = 65535;
        String cond = "cond_333";
        Integer policy = 2;
        VtnPathMap vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            build();
        VtnPathMapKey vpmKey = new VtnPathMapKey(index);
        InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey).
            child(VtnPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        IdentifiedData<?> data = new IdentifiedData<VtnPathMap>(path, vpm);
        String desc = vtnId.toString() + ": VTN";
        String value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy;
        VtnUpdateType type = VtnUpdateType.REMOVED;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "removed", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);

        // With flow timeout
        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);
        tname = "vtn_4";
        vtnKey = new VtnKey(new VnodeName(tname));
        vtnId = new VTenantIdentifier(vtnKey.getName());
        index = 6633;
        cond = "cond_4000";
        policy = 1;
        Integer idle = 1234;
        Integer hard = 5678;
        vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            build();
        vpmKey = new VtnPathMapKey(index);
        path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey).
            child(VtnPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        data = new IdentifiedData<VtnPathMap>(path, vpm);
        desc = vtnId.toString() + ": VTN";
        value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy + ", idle=" + idle + ", hard=" + hard;
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been {}: value={{}}",
                            desc, "removed", value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data, type);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for {@link PathMapUtils#log(Logger,ChangedData)}.
     *
     * <ul>
     *   <li>Global path map has been changed.</li>
     * </ul>
     */
    @Test
    public void testLogGlobalChanged() {
        Logger logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);

        Integer index = 9999;
        String ocond = "cond_1";
        Integer opolicy = 1;
        VtnPathMap old = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(ocond)).
            setPolicy(opolicy).
            build();

        String cond = "cond_11";
        Integer policy = 0;
        Integer idle = 7788;
        Integer hard = 12345;
        VtnPathMap vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            build();

        VtnPathMapKey vpmKey = new VtnPathMapKey(index);
        InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
            builder(GlobalPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        ChangedData<?> data = new ChangedData<>(path, vpm, old);
        String desc = "Global";
        String oldValue = "index=" + index + ", cond=" + ocond +
            ", policy=" + opolicy;
        String value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy + ", idle=" + idle + ", hard=" + hard;
        PathMapUtils.log(logger, data);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been changed: old={{}}, new={{}}",
                            desc, oldValue, value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);
    }

    /**
     * Test case for {@link PathMapUtils#log(Logger,ChangedData)}.
     *
     * <ul>
     *   <li>VTN path map has been changed.</li>
     * </ul>
     */
    @Test
    public void testLogVtnChanged() {
        Logger logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(true);

        Integer index = 9999;
        String tname = "vtn_1";
        VtnKey vtnKey = new VtnKey(new VnodeName(tname));
        VTenantIdentifier vtnId = new VTenantIdentifier(vtnKey.getName());
        String ocond = "cond_123";
        Integer opolicy = 2;
        Integer idle = 5544;
        Integer hard = 8765;
        VtnPathMap old = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(ocond)).
            setPolicy(opolicy).
            setIdleTimeout(idle).
            setHardTimeout(hard).
            build();

        String cond = "cond_456";
        Integer policy = 1;
        VtnPathMap vpm = new VtnPathMapBuilder().
            setIndex(index).
            setCondition(new VnodeName(cond)).
            setPolicy(policy).
            build();

        VtnPathMapKey vpmKey = new VtnPathMapKey(index);
        InstanceIdentifier<VtnPathMap> path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey).
            child(VtnPathMaps.class).
            child(VtnPathMap.class, vpmKey).
            build();
        ChangedData<?> data = new ChangedData<>(path, vpm, old);
        String desc = vtnId.toString() + ": VTN";
        String oldValue = "index=" + index + ", cond=" + ocond +
            ", policy=" + opolicy + ", idle=" + idle + ", hard=" + hard;
        String value = "index=" + index + ", cond=" + cond +
            ", policy=" + policy;
        PathMapUtils.log(logger, data);
        verify(logger).isInfoEnabled();
        verify(logger).info("{} path map has been changed: old={{}}, new={{}}",
                            desc, oldValue, value);
        verifyNoMoreInteractions(logger);

        logger = mock(Logger.class);
        when(logger.isInfoEnabled()).thenReturn(false);
        PathMapUtils.log(logger, data);
        verify(logger).isInfoEnabled();
        verifyNoMoreInteractions(logger);
    }

    /**
     * Run data conversion tests.
     *
     * @param index   The path map index.
     * @param cond    The name of the flow condition.
     * @param policy  The path policy identifier.
     * @param idle    The idle-timeout value.
     * @param hard    The hard-timeout value.
     * @throws Exception  An error occurred.
     */
    private void conversionTest(Integer index, String cond, Integer policy,
                                Integer idle, Integer hard) throws Exception {
        VnodeName vcond = new VnodeName(cond);
        PathMapList pml = new PathMapListBuilder().
            setIndex(index).setCondition(vcond).
            setPolicy(policy).setIdleTimeout(idle).setHardTimeout(hard).
            build();
        Integer policyId = policy;
        if (policyId == null) {
            policyId = 0;
        }
        VtnPathMap vpm = PathMapUtils.toVtnPathMapBuilder(pml).build();
        assertEquals(index, vpm.getIndex());
        assertEquals(vcond, vpm.getCondition());
        assertEquals(policyId, vpm.getPolicy());
        assertEquals(idle, vpm.getIdleTimeout());
        assertEquals(hard, vpm.getHardTimeout());
    }

    /**
     * Create {@link VtnPathMap} instances.
     *
     * @param num   The number of {@link VtnPathMap} instances to be created.
     * @param maps  A map to store pairs of map index and {@link VtnPathMap}
     *              instance.
     * @return  A list of {@link VtnPathMap} instances.
     */
    private List<VtnPathMap> createVtnPathMaps(
        int num, Map<Integer, VtnPathMap> maps) {
        int count = num - 1;
        final int maxIndex = 65535;
        do {
            int index = random.nextInt(maxIndex);
            if (index != 0) {
                Integer idx = Integer.valueOf(index);
                int policy = random.nextInt(PATH_POLICY_MAX + 1);
                VnodeName vcond = new VnodeName("fcond" + index);
                VtnPathMap vpm = new VtnPathMapBuilder().
                    setIndex(idx).setCondition(vcond).setPolicy(policy).
                    build();
                maps.put(idx, vpm);
            }
        } while (maps.size() < count);

        // Add the largest index number to the head of the list.
        LinkedList<VtnPathMap> vpmList = new LinkedList<>(maps.values());
        VtnPathMap vpmMax = new VtnPathMapBuilder().
            setIndex(maxIndex).setCondition(new VnodeName("fcond")).build();
        assertEquals(null, maps.put(vpmMax.getIndex(), vpmMax));
        vpmList.addFirst(vpmMax);

        return vpmList;
    }
}
