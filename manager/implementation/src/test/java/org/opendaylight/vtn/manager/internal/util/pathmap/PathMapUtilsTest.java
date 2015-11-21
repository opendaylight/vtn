/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathmap;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.PathMap;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.vnode.VTenantIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest;

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
     *   <li>{@link PathMapUtils#getInvalidMapIndexException(Integer)}</li>
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
            e = PathMapUtils.getInvalidMapIndexException(i);
            assertEquals(null, e.getCause());
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Invalid path map index: " + i, e.getMessage());
        }
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link PathMapUtils#toVtnPathMapBuilder(PathMap)}</li>
     *   <li>{@link PathMapUtils#toVtnPathMapBuilder(VtnPathMapConfig)}</li>
     *   <li>{@link PathMapUtils#toPathMapListBuilder(Integer, PathMap)}</li>
     *   <li>{@link PathMapUtils#toPathMap(VtnPathMapConfig)}</li>
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
        for (int policy = 0; policy <= PathPolicyUtilsTest.PATH_POLICY_MAX;
             policy++) {
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

        // Null PathMap.
        PathMap pmap = null;
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "PathMap cannot be null";
        try {
            PathMapUtils.toVtnPathMapBuilder(pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            PathMapUtils.toPathMapListBuilder(1, pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Null VtnPathMapConfig.
        VtnPathMapConfig vpmc = null;
        msg = "Path map configuration cannot be null";
        try {
            PathMapUtils.toVtnPathMapBuilder(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        assertEquals(null, PathMapUtils.toPathMap(vpmc));

        // Map index is null.
        String condition = "fcond";
        VnodeName vcondition = new VnodeName(condition);
        pmap = new PathMap(condition, 1);
        msg = "Path map index cannot be null";
        try {
            PathMapUtils.toVtnPathMapBuilder(pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            PathMapUtils.toPathMapListBuilder(null, pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

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

        try {
            PathMapUtils.toPathMap(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // Invalid map index.
        int[] invalidIndices = {
            Integer.MIN_VALUE, -20000, -2, -1, 0,
            65536, 65537, 100000, 3000000, Integer.MAX_VALUE,
        };
        etag = RpcErrorTag.BAD_ELEMENT;
        for (int index: invalidIndices) {
            msg = "Invalid path map index: " + index;
            pmap = new PathMap(index, condition, 0);
            try {
                PathMapUtils.toVtnPathMapBuilder(pmap);
                unexpected();
            } catch (RpcException e) {
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            pmap = new PathMap(1, condition, 0);
            try {
                PathMapUtils.toPathMapListBuilder(index, pmap);
                unexpected();
            } catch (RpcException e) {
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Null flow condition.
        etag = RpcErrorTag.MISSING_ELEMENT;
        msg = "Flow condition name cannot be null";
        pmap = new PathMap(1, null, 0);
        try {
            PathMapUtils.toVtnPathMapBuilder(pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            PathMapUtils.toPathMapListBuilder(1, pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

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

        try {
            PathMapUtils.toPathMap(vpmc);
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
        pmap = new PathMap(1, "", 0);
        try {
            PathMapUtils.toVtnPathMapBuilder(pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            PathMapUtils.toPathMapListBuilder(1, pmap);
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
            pmap = new PathMap(1, name, 0);
            try {
                PathMapUtils.toVtnPathMapBuilder(pmap);
                unexpected();
            } catch (RpcException e) {
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            try {
                PathMapUtils.toPathMapListBuilder(1, pmap);
                unexpected();
            } catch (RpcException e) {
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid path policy ID.
        int[] invalidPolicies = {
            Integer.MIN_VALUE, -9999999, -333333, -2, -1,
            PathPolicyUtilsTest.PATH_POLICY_MAX + 1,
            PathPolicyUtilsTest.PATH_POLICY_MAX + 2,
            10000000, 222222222, Integer.MAX_VALUE,
        };
        for (int policy: invalidPolicies) {
            msg = "Invalid path policy ID: " + policy;
            pmap = new PathMap(1, condition, policy);
            try {
                PathMapUtils.toVtnPathMapBuilder(pmap);
                unexpected();
            } catch (RpcException e) {
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            try {
                PathMapUtils.toPathMapListBuilder(null, pmap);
                unexpected();
            } catch (RpcException e) {
                assertTrue(e.getCause() instanceof IllegalArgumentException);
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid idle/hard timeout.
        int[] invalidTimeouts = {
            Integer.MIN_VALUE, -99999999, -65535, -3333, -2, -1,
            65536, 65537, 1000000, 33333333, Integer.MAX_VALUE,
        };
        for (int timeout: invalidTimeouts) {
            msg = "Invalid idle-timeout: " + timeout;
            pmap = new PathMap(1, condition, 0, timeout, 0);
            try {
                PathMapUtils.toVtnPathMapBuilder(pmap);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            try {
                PathMapUtils.toPathMapListBuilder(1, pmap);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            msg = "Invalid hard-timeout: " + timeout;
            pmap = new PathMap(1, condition, 0, 0, timeout);
            try {
                PathMapUtils.toVtnPathMapBuilder(pmap);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            try {
                PathMapUtils.toPathMapListBuilder(1, pmap);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Inconsistent flow timeouts.
        Unmarshaller um = createUnmarshaller(PathMap.class);
        String xml = new XmlNode("pathmap").
            setAttribute("index", 1).setAttribute("condition", condition).
            setAttribute("policy", 0).setAttribute("hardTimeout", 0).
            toString();
        pmap = unmarshal(um, xml, PathMap.class);
        msg = "idle-timeout must be specified.";
        try {
            PathMapUtils.toVtnPathMapBuilder(pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            PathMapUtils.toPathMapListBuilder(1, pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

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

        try {
            PathMapUtils.toPathMap(vpmc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        xml = new XmlNode("pathmap").
            setAttribute("index", 1).setAttribute("condition", condition).
            setAttribute("policy", 0).setAttribute("idleTimeout", 0).
            toString();
        pmap = unmarshal(um, xml, PathMap.class);
        msg = "hard-timeout must be specified.";
        try {
            PathMapUtils.toVtnPathMapBuilder(pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        try {
            PathMapUtils.toPathMapListBuilder(1, pmap);
            unexpected();
        } catch (RpcException e) {
            assertEquals(null, e.getCause());
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

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

        try {
            PathMapUtils.toPathMap(vpmc);
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
        List<PathMap> pmapList = new ArrayList<>();
        List<VtnPathMapConfig> vpmcList = new ArrayList<>();
        for (Integer tmout: timeouts) {
            int timeout = tmout.intValue();
            pmapList.add(new PathMap(1, condition, 0, timeout, timeout));
            vpmcList.add(new VtnPathMapBuilder().setIndex(1).
                         setCondition(vcondition).setPolicy(0).
                         setIdleTimeout(tmout).setHardTimeout(tmout).build());

            if (timeout > 1) {
                int hard = timeout - 1;
                pmapList.add(new PathMap(1, condition, 0, timeout, hard));
                vpmcList.add(new VtnPathMapBuilder().setIndex(1).
                             setCondition(vcondition).setPolicy(0).
                             setIdleTimeout(tmout).setHardTimeout(hard)
                             .build());
            }
        }

        for (PathMap pm: pmapList) {
            try {
                PathMapUtils.toVtnPathMapBuilder(pm);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }

            try {
                PathMapUtils.toPathMapListBuilder(1, pm);
                unexpected();
            } catch (RpcException e) {
                assertEquals(null, e.getCause());
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
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

            try {
                PathMapUtils.toPathMap(vpm);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<GlobalPathMaps> path = InstanceIdentifier.
            create(GlobalPathMaps.class);

        // Root container is not present.
        GlobalPathMaps root = null;
        List<VtnPathMap> vlist = new ArrayList<>();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(vlist, PathMapUtils.readPathMaps(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);

        // Global path map list is null.
        root = new GlobalPathMapsBuilder().build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(vlist, PathMapUtils.readPathMaps(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);

        // Global path map list is empty.
        root = new GlobalPathMapsBuilder().
            setVtnPathMap(Collections.<VtnPathMap>emptyList()).build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        assertEquals(vlist, PathMapUtils.readPathMaps(rtx));
        Mockito.verify(rtx).read(oper, path);
        Mockito.reset(rtx);

        // Global path maps are present.
        Map<Integer, VtnPathMap> maps = new HashMap<>();
        List<VtnPathMap> vpmList = createVtnPathMaps(100, maps);
        root = new GlobalPathMapsBuilder().setVtnPathMap(vpmList).build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        vlist = PathMapUtils.readPathMaps(rtx);
        Mockito.verify(rtx).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
        Mockito.verify(rtx, Mockito.never()).
            read(Mockito.any(LogicalDatastoreType.class),
                 (InstanceIdentifier<?>)Mockito.any(InstanceIdentifier.class));

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
                int policy =
                    random.nextInt(PathPolicyUtilsTest.PATH_POLICY_MAX + 1);
                VnodeName vcond = new VnodeName("fcond" + index);
                vpm = new VtnPathMapBuilder().
                    setIndex(idx).setCondition(vcond).setPolicy(policy).
                    build();
            }
            maps.put(idx, vpm);
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vpm));
        } while (maps.size() < 100);

        // Run tests.
        for (Integer index: maps.keySet()) {
            VtnPathMap expected = maps.get(index);
            assertEquals(expected, PathMapUtils.readPathMap(rtx, index));
        }
        for (InstanceIdentifier<VtnPathMap> path: pathList) {
            Mockito.verify(rtx).read(oper, path);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        Mockito.when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
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
        Mockito.verify(rtx).read(oper, path);
        Mockito.verify(rtx).read(oper, vtnPath);
        Mockito.reset(rtx);

        // VTN path map container is not present.
        vtn = new VtnBuilder().build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        Mockito.when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        assertEquals(Collections.<VtnPathMap>emptyList(),
                     PathMapUtils.readPathMaps(rtx, ident));
        Mockito.verify(rtx).read(oper, path);
        Mockito.verify(rtx).read(oper, vtnPath);
        Mockito.reset(rtx);

        // VTN path map list is null.
        root = new VtnPathMapsBuilder().build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        Mockito.when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        assertEquals(Collections.<VtnPathMap>emptyList(),
                     PathMapUtils.readPathMaps(rtx, ident));
        Mockito.verify(rtx).read(oper, path);
        Mockito.verify(rtx, Mockito.never()).read(oper, vtnPath);
        Mockito.reset(rtx);

        // VTN path map list is empty.
        List<VtnPathMap> vlist = new ArrayList<>();
        root = new VtnPathMapsBuilder().setVtnPathMap(vlist).build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        Mockito.when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        assertEquals(Collections.<VtnPathMap>emptyList(),
                     PathMapUtils.readPathMaps(rtx, ident));
        Mockito.verify(rtx).read(oper, path);
        Mockito.verify(rtx, Mockito.never()).read(oper, vtnPath);
        Mockito.reset(rtx);

        // VTN path maps are present.
        Map<Integer, VtnPathMap> maps = new HashMap<>();
        List<VtnPathMap> vpmList = createVtnPathMaps(100, maps);
        root = new VtnPathMapsBuilder().setVtnPathMap(vpmList).build();
        Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(root));
        Mockito.when(rtx.read(oper, vtnPath)).thenReturn(getReadResult(vtn));
        vlist = PathMapUtils.readPathMaps(rtx, ident);
        Mockito.verify(rtx).read(oper, path);
        Mockito.verify(rtx, Mockito.never()).read(oper, vtnPath);
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
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
        Mockito.verify(rtx, Mockito.never()).
            read(Mockito.any(LogicalDatastoreType.class),
                 (InstanceIdentifier<?>)Mockito.any(InstanceIdentifier.class));

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
        Mockito.when(rtx.read(oper, vtnPath1)).
            thenReturn(getReadResult(new VtnBuilder().build()));
        Mockito.when(rtx.read(oper, vtnPath2)).
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
                int policy =
                    random.nextInt(PathPolicyUtilsTest.PATH_POLICY_MAX + 1);
                VnodeName vcond = new VnodeName("fcond" + index);
                vpm = new VtnPathMapBuilder().
                    setIndex(idx).setCondition(vcond).setPolicy(policy).
                    build();
            }
            maps.put(idx, vpm);
            Mockito.when(rtx.read(oper, path)).thenReturn(getReadResult(vpm));

            path = InstanceIdentifier.builder(Vtns.class).
                child(Vtn.class, vkey2).child(VtnPathMaps.class).
                child(VtnPathMap.class, mapKey).build();
            pathList2.add(path);
            Mockito.when(rtx.read(oper, path)).
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
            Mockito.verify(rtx).read(oper, path);
        }
        for (InstanceIdentifier<VtnPathMap> path: pathList2) {
            Mockito.verify(rtx).read(oper, path);
        }

        Mockito.verify(rtx, Mockito.times(notPresent.size())).
            read(oper, vtnPath1);
        Mockito.verify(rtx, Mockito.times(maps.size())).
            read(oper, vtnPath2);
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
        PathMap pmap = PathMapUtils.toPathMap(pml);
        assertEquals(index, pmap.getIndex());
        assertEquals(cond, pmap.getFlowConditionName());
        int pid = (policy == null) ? 0 : policy.intValue();
        assertEquals(pid, pmap.getPathPolicyId());
        assertEquals(idle, pmap.getIdleTimeout());
        assertEquals(hard, pmap.getHardTimeout());

        Integer policyId = policy;
        if (policyId == null) {
            policyId = Integer.valueOf(pid);
        }
        VtnPathMap vpm = PathMapUtils.toVtnPathMapBuilder(pmap).build();
        assertEquals(index, vpm.getIndex());
        assertEquals(vcond, vpm.getCondition());
        assertEquals(policyId, vpm.getPolicy());
        assertEquals(idle, vpm.getIdleTimeout());
        assertEquals(hard, vpm.getHardTimeout());

        int idx = index.intValue() + 1;
        if (idx > 65535) {
            idx = 1;
        }

        Integer anotherIndex = Integer.valueOf(idx);
        pml = PathMapUtils.toPathMapListBuilder(anotherIndex, pmap).build();
        assertEquals(anotherIndex, pml.getIndex());
        assertEquals(vcond, pml.getCondition());
        assertEquals(policyId, pml.getPolicy());
        assertEquals(idle, pml.getIdleTimeout());
        assertEquals(hard, pml.getHardTimeout());

        vpm = PathMapUtils.toVtnPathMapBuilder(pml).build();
        assertEquals(anotherIndex, vpm.getIndex());
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
                int policy =
                    random.nextInt(PathPolicyUtilsTest.PATH_POLICY_MAX + 1);
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
