/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestVtnPortDesc;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.IdentifiableItem;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.Item;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * JUnit test for {@link PathPolicyUtils}.
 */
public class PathPolicyUtilsTest extends TestBase {
    /**
     * The minimum value of the path policy identifier.
     */
    public static final int  PATH_POLICY_MIN = 1;

    /**
     * The maximum value of the path policy identifier.
     */
    public static final int  PATH_POLICY_MAX = 3;

    /**
     * Test case for utility methods that return an exception.
     *
     * <ul>
     *   <li>{@link PathPolicyUtils#getNotFoundException(int)}</li>
     *   <li>{@link PathPolicyUtils#getNotFoundException(int, Throwable)}</li>
     *   <li>{@link PathPolicyUtils#getInvalidPolicyIdException(Integer, Throwable)}</li>
     *   <li>{@link PathPolicyUtils#getInvalidDefaultCostException(Long, Throwable)}</li>
     *   <li>{@link PathPolicyUtils#getInvalidCostException(Long, Throwable)}</li>
     *   <li>{@link PathPolicyUtils#getNullPolicyIdException()}</li>
     *   <li>{@link PathPolicyUtils#getNullPathCostException()}</li>
     *   <li>{@link PathPolicyUtils#getDuplicatePortException(Object)}</li>
     * </ul>
     */
    @Test
    public void testGetException() {
        IllegalArgumentException cause = new IllegalArgumentException();
        for (int i = PATH_POLICY_MIN; i <= PATH_POLICY_MAX; i++) {
            // getNotFoundException(int)
            RpcException e = PathPolicyUtils.getNotFoundException(i);
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(null, e.getCause());
            String msg = i + ": Path policy does not exist.";
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());

            // getNotFoundException(int, Throwable)
            e = PathPolicyUtils.getNotFoundException(i, cause);
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertSame(cause, e.getCause());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // getInvalidPolicyIdException(int, Throwable)
        for (int i = PATH_POLICY_MAX + 1; i < PATH_POLICY_MAX + 10; i++) {
            RpcException e =
                PathPolicyUtils.getInvalidPolicyIdException(i, cause);
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(cause, e.getCause());
            String msg = "Invalid path policy ID: " + i;
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        for (long c = -5L; c < 0; c++) {
            // getInvalidDefaultCostException(Long, Throwable)
            RpcException e =
                PathPolicyUtils.getInvalidDefaultCostException(c, cause);
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(cause, e.getCause());
            String msg = "Invalid default cost: " + c;
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());

            // getInvalidCostException(Long, Throwable)
            e = PathPolicyUtils.getInvalidCostException(c, cause);
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(cause, e.getCause());
            msg = "Invalid cost value: " + c;
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // getNullPolicyIdException()
        RpcException e = PathPolicyUtils.getNullPolicyIdException();
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(null, e.getCause());
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals("Path policy ID cannot be null", e.getMessage());

        // getNullPathCostException()
        e = PathPolicyUtils.getNullPathCostException();
        assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
        assertEquals(null, e.getCause());
        assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
        assertEquals("Path cost cannot be null", e.getMessage());

        // getDuplicatePortException()
        Object[] locs = {
            "openflow:1,,,,",
            "openflow:123,4,port-4",
        };
        for (Object loc: locs) {
            e = PathPolicyUtils.getDuplicatePortException(loc);
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("Duplicate port descriptor: " + loc, e.getMessage());
        }
    }

    /**
     * Test case for {@link PathPolicyUtils#getIdentifier(VtnPathPolicyConfig)}
     * and {@link PathPolicyUtils#getIdentifier(Integer)}.
     */
    @Test
    public void testGetVtnPathPolicyIdentifier() {
        for (int i = PATH_POLICY_MIN; i <= PATH_POLICY_MAX; i++) {
            Integer id = Integer.valueOf(i);
            InstanceIdentifier<VtnPathPolicy> path =
                PathPolicyUtils.getIdentifier(id);
            assertEquals(VtnPathPolicy.class, path.getTargetType());
            assertEquals(false, path.isWildcarded());

            Iterator<PathArgument> it = path.getPathArguments().iterator();
            assertTrue(it.hasNext());
            PathArgument pa = it.next();
            assertEquals(VtnPathPolicies.class, pa.getType());
            assertTrue(pa instanceof Item);

            assertTrue(it.hasNext());
            pa = it.next();
            assertEquals(VtnPathPolicy.class, pa.getType());
            assertTrue(pa instanceof IdentifiableItem);
            IdentifiableItem<?, ?> item = (IdentifiableItem<?, ?>)pa;
            VtnPathPolicyKey key = new VtnPathPolicyKey(id);
            assertEquals(key, item.getKey());

            assertFalse(it.hasNext());

            VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(id).build();
            assertEquals(path, PathPolicyUtils.getIdentifier(vpp));
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#getIdentifier(int, VtnPathCostConfig)}
     * and {@link PathPolicyUtils#getIdentifier(Integer, VtnPortDesc)}.
     */
    @Test
    public void testGetVtnPathCostIdentifier() {
        VtnPortDesc[] vdescs = {
            new VtnPortDesc("openflow:1,,"),
            new VtnPortDesc("openflow:1,2,"),
            new VtnPortDesc("openflow:1,2,s1-eth2"),
        };

        for (int i = PATH_POLICY_MIN; i <= PATH_POLICY_MAX; i++) {
            Integer id = Integer.valueOf(i);
            for (VtnPortDesc vdesc: vdescs) {
                InstanceIdentifier<VtnPathCost> path =
                    PathPolicyUtils.getIdentifier(id, vdesc);
                assertEquals(VtnPathCost.class, path.getTargetType());
                assertEquals(false, path.isWildcarded());

                Iterator<PathArgument> it = path.getPathArguments().iterator();
                assertTrue(it.hasNext());
                PathArgument pa = it.next();
                assertEquals(VtnPathPolicies.class, pa.getType());
                assertTrue(pa instanceof Item);

                assertTrue(it.hasNext());
                pa = it.next();
                assertEquals(VtnPathPolicy.class, pa.getType());
                assertTrue(pa instanceof IdentifiableItem);
                IdentifiableItem<?, ?> item = (IdentifiableItem<?, ?>)pa;
                VtnPathPolicyKey key = new VtnPathPolicyKey(id);
                assertEquals(key, item.getKey());

                assertTrue(it.hasNext());
                pa = it.next();
                assertEquals(VtnPathCost.class, pa.getType());
                assertTrue(pa instanceof IdentifiableItem);
                item = (IdentifiableItem<?, ?>)pa;
                VtnPathCostKey ckey = new VtnPathCostKey(vdesc);
                assertEquals(ckey, item.getKey());

                assertFalse(it.hasNext());

                VtnPathCost vpc = new VtnPathCostBuilder().setPortDesc(vdesc).
                    build();
                assertEquals(path, PathPolicyUtils.getIdentifier(i, vpc));
            }
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#setId(VtnPathPolicyBuilder, Integer)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetId() throws Exception {
        VtnPathPolicyBuilder builder = new VtnPathPolicyBuilder();
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            PathPolicyUtils.setId(builder, id);
            assertEquals(id, builder.getId().intValue());
        }

        int[] invalid = {
            Integer.MIN_VALUE, -1000, -1, 0,
            PATH_POLICY_MAX + 1, PATH_POLICY_MAX + 2,
            PATH_POLICY_MAX + 1000, Integer.MAX_VALUE,
        };
        for (int id: invalid) {
            try {
                PathPolicyUtils.setId(builder, id);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                Throwable t = e.getCause();
                assertTrue("Unexpected cause: " + t,
                           t instanceof IllegalArgumentException);
                String msg = "Invalid path policy ID: " + id;
                assertEquals(msg, e.getMessage());
            }
        }

        try {
            PathPolicyUtils.setId(builder, (Integer)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(null, e.getCause());
            assertEquals("Path policy ID cannot be null", e.getMessage());
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#setDefaultCost(VtnPathPolicyBuilder, Long)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetDefaultCost() throws Exception {
        Long[] defCosts = {
            null, 0L, 1L, 12345L, 34567L, 99999999L, Long.MAX_VALUE,
        };

        VtnPathPolicyBuilder builder = new VtnPathPolicyBuilder();
        for (Long cost: defCosts) {
            PathPolicyUtils.setDefaultCost(builder, cost);
            assertEquals(cost, builder.getDefaultCost());
        }

        long[] invalid = {
            Long.MIN_VALUE, -100000L, -2L, -1L,
        };
        for (long cost: invalid) {
            try {
                PathPolicyUtils.setDefaultCost(builder, cost);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                String msg = "Invalid default cost: " + cost;
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#setCost(VtnPathCostBuilder, Long)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetCost() throws Exception {
        Long[] costs = {
            1L, 2L, 3124L, 9876543L, Long.MAX_VALUE,
        };

        VtnPathCostBuilder builder = new VtnPathCostBuilder();
        for (Long cost: costs) {
            PathPolicyUtils.setCost(builder, cost);
            assertEquals(cost, builder.getCost());
        }

        // null should be treated as a default value.
        PathPolicyUtils.setCost(builder, (Long)null);
        assertEquals(1L, builder.getCost().longValue());

        long[] invalid = {
            Long.MIN_VALUE, -100000L, -3333L, -2L, -1L, 0L,
        };
        for (long cost: invalid) {
            try {
                PathPolicyUtils.setCost(builder, cost);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                String msg = "Invalid cost value: " + cost;
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#setPortDesc(VtnPathCostBuilder, VtnPortDesc)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetPortDesc() throws Exception {
        VtnPortDesc[] vdescs = {
            new VtnPortDesc("openflow:1,,"),
            new VtnPortDesc("openflow:23,45,port-45"),
            new VtnPortDesc("openflow:18446744073709551615,,port-1"),
        };

        VtnPathCostBuilder builder = new VtnPathCostBuilder();
        for (VtnPortDesc vdesc: vdescs) {
            PathPolicyUtils.setPortDesc(builder, vdesc);
            assertEquals(vdesc, builder.getPortDesc());
        }

        // Empty vtn-port-desc.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vtn-port-desc cannot be null";
        VtnPortDesc[] empty = {
            null,
            new TestVtnPortDesc(),
        };

        for (VtnPortDesc vdesc: empty) {
            try {
                PathPolicyUtils.setPortDesc(builder, vdesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid node-id.
        String[] badNodes = {
            "",
            "openflow:1:2",
            "openflow:18446744073709551616",
            "unknown:1",
        };
        etag = RpcErrorTag.BAD_ELEMENT;
        for (String node: badNodes) {
            VtnPortDesc vdesc = new TestVtnPortDesc(node + ",,");
            try {
                PathPolicyUtils.setPortDesc(builder, vdesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                msg = "Invalid vtn-port-desc: " + vdesc.getValue() +
                    ": Invalid node ID: " + node;
                assertEquals(msg, e.getMessage());
                Throwable t = e.getCause();
                assertTrue("Unexpected cause: " + t,
                           t instanceof RpcException);
            }
        }

        // Invalid format.
        VtnPortDesc[] badFormats = {
            new TestVtnPortDesc(",2"),
            new TestVtnPortDesc("unknown:1,"),
            new TestVtnPortDesc("openflow:1"),
            new TestVtnPortDesc("openflow:1,2"),
        };
        for (VtnPortDesc vdesc: badFormats) {
            try {
                PathPolicyUtils.setPortDesc(builder, vdesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                msg = "Invalid vtn-port-desc format: " + vdesc.getValue();
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#newBuilder(VtnPathPolicyConfig)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNewPolicyBuilder() throws Exception {
        // Create a list of VtnPathCost.
        List<VtnPathCost> vpcomplete = new ArrayList<>();
        List<VtnPathCost> vpcosts = new ArrayList<>();
        long[] dpids = {
            Long.MIN_VALUE, -1234567L,  -1L,
            1L, 0xabcdef1234567L, Long.MAX_VALUE,
        };
        String[] ports = {
            null, "1", "10",
        };
        String[] names = {
            null, "port-1", "port-10",
        };

        long cost = 0;
        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            for (String port: ports) {
                String p = (port == null) ? "" : port;
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    String pd = joinStrings(null, null, ",", snode, p, n);
                    VtnPortDesc vdesc = new VtnPortDesc(pd);
                    Long c = (cost == 0) ? null : Long.valueOf(cost);
                    VtnPathCost vpc = new VtnPathCostBuilder().
                        setPortDesc(vdesc).setCost(c).build();
                    vpcosts.add(vpc);

                    if (cost == 0) {
                        vpc = new VtnPathCostBuilder().setPortDesc(vdesc).
                            setCost(1L).build();
                    }
                    vpcomplete.add(vpc);
                    cost++;
                }
            }
        }

        List<Integer> ids = new ArrayList<>();
        List<Integer> badIds = new ArrayList<>();
        badIds.add(null);
        final int delta = 5;
        for (int i = PATH_POLICY_MIN - delta; i <= PATH_POLICY_MAX + delta;
             i++) {
            Integer id = Integer.valueOf(i);
            if (i >= PATH_POLICY_MIN && i <= PATH_POLICY_MAX) {
                ids.add(id);
            } else {
                badIds.add(id);
            }
        }

        Long[] defCosts = {
            null, 1L, 777L, 3333L, 9999999L, Long.MAX_VALUE,
        };

        for (Integer id: ids) {
            for (Long defCost: defCosts) {
                VtnPathPolicyConfig vppc = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).setVtnPathCost(vpcosts).
                    build();
                VtnPathPolicy vpp = PathPolicyUtils.newBuilder(vppc).build();
                VtnPathPolicy expected = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).
                    setVtnPathCost(vpcomplete).build();
                assertEquals(expected, vpp);

                // Null vtn-path-cost list.
                vppc = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).build();
                vpp = PathPolicyUtils.newBuilder(vppc).build();
                expected = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).build();
                assertEquals(expected, vpp);

                // Empty vtn-path-cost-list.
                vppc = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).
                    setVtnPathCost(Collections.<VtnPathCost>emptyList()).
                    build();
                vpp = PathPolicyUtils.newBuilder(vppc).build();
                assertEquals(expected, vpp);
            }
        }

        // Invalid default cost test.
        Long[] badDefCosts = {
            Long.MIN_VALUE, Long.MIN_VALUE + 1, -1000000000000L, -99999999L,
            -10L, -5L, -2L, -1L,
        };
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;

        for (Long defCost: badDefCosts) {
            VtnPathPolicyConfig vppc = mock(VtnPathPolicyConfig.class);
            when(vppc.getId()).thenReturn(1);
            when(vppc.getDefaultCost()).thenReturn(defCost);
            when(vppc.getVtnPathCost()).thenReturn((List<VtnPathCost>)null);
            try {
                PathPolicyUtils.newBuilder(vppc).build();
                unexpected();
            } catch (RpcException e) {
                String msg = "Invalid default cost: " + defCost;
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                Throwable t = e.getCause();
                assertTrue("Unexpected cause: " + t,
                           t instanceof IllegalArgumentException);
            }
        }

        // Invalid path policy ID test.
        Integer validId = Integer.valueOf(PATH_POLICY_MAX);
        long defc = 0L;
        for (Integer badId: badIds) {
            VtnPathPolicyConfig vppc = mock(VtnPathPolicyConfig.class);
            when(vppc.getId()).thenReturn(badId);
            when(vppc.getDefaultCost()).thenReturn(0L);
            when(vppc.getVtnPathCost()).thenReturn((List<VtnPathCost>)null);
            try {
                PathPolicyUtils.newBuilder(vppc).build();
                unexpected();
            } catch (RpcException e) {
                Throwable t = e.getCause();
                if (badId == null) {
                    assertEquals(null, t);
                    assertEquals("Path policy ID cannot be null",
                                 e.getMessage());
                    assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                } else {
                    assertTrue("Unexpected cause: " + t,
                               t instanceof IllegalArgumentException);
                    assertEquals("Invalid path policy ID: " + badId,
                                 e.getMessage());
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                }

                assertEquals(vtag, e.getVtnErrorTag());
            }
        }

        // Dupliacte vtn-port-desc test.
        for (VtnPathCost vpc: vpcosts) {
            List<VtnPathCost> badCosts = new ArrayList<>(vpcosts);
            VtnPortDesc vdesc = vpc.getPortDesc();
            VtnPathCost dup = new VtnPathCostBuilder().
                setPortDesc(vdesc).setCost(12345678L).build();
            badCosts.add(dup);

            VtnPathPolicyConfig vppc = mock(VtnPathPolicyConfig.class);
            when(vppc.getId()).thenReturn(1);
            when(vppc.getDefaultCost()).thenReturn(0L);
            when(vppc.getVtnPathCost()).thenReturn(badCosts);
            try {
                PathPolicyUtils.newBuilder(vppc).build();
                unexpected();
            } catch (RpcException e) {
                String msg = "Duplicate port descriptor: " + vdesc.getValue();
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
                assertEquals(null, e.getCause());
            }
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#newBuilder(VtnPathCostConfig)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNewCostBuilder() throws Exception {
        long[] dpids = {
            Long.MIN_VALUE, -1234567L,  -1L,
            1L, 0xabcdef1234567L, Long.MAX_VALUE,
        };
        String[] ports = {
            null, "1", "10",
        };
        String[] names = {
            null, "port-1", "port-10",
        };

        long cost = 0;
        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            for (String port: ports) {
                String p = (port == null) ? "" : port;
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    String pd = joinStrings(null, null, ",", snode, p, n);
                    VtnPortDesc vdesc = new VtnPortDesc(pd);
                    Long c = (cost == 0) ? null : Long.valueOf(cost);
                    VtnPathCost expected = new VtnPathCostBuilder().
                        setPortDesc(vdesc).setCost(c).build();
                    VtnPathCostConfig vpcc = expected;
                    if (cost == 0) {
                        expected = new VtnPathCostBuilder(expected).
                            setCost(1L).build();
                    }

                    VtnPathCost vpc = PathPolicyUtils.newBuilder(vpcc).build();
                    assertEquals(expected, vpc);
                    cost++;
                }
            }
        }

        // Null configuration.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "Path cost cannot be null";
        VtnPathCostConfig vpcc = null;
        try {
            PathPolicyUtils.newBuilder(vpcc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
            assertEquals(null, e.getCause());
        }

        // Invalid cost.
        etag = RpcErrorTag.BAD_ELEMENT;
        Long[] invalid = {
            Long.MIN_VALUE, -99999L, -222L, -2L, -1L, 0L,
        };
        for (Long c: invalid) {
            VtnPortDesc vdesc = new VtnPortDesc("openflow:1,2,");
            vpcc = mock(VtnPathCostConfig.class);
            when(vpcc.getCost()).thenReturn(c);
            when(vpcc.getPortDesc()).thenReturn(vdesc);
            try {
                PathPolicyUtils.newBuilder(vpcc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                msg = "Invalid cost value: " + c;
                assertEquals(msg, e.getMessage());
            }
        }

        // Invalid vtn-port-desc format.
        VtnPortDesc[] badFormats = {
            new TestVtnPortDesc(",2"),
            new TestVtnPortDesc("unknown:1,"),
            new TestVtnPortDesc("openflow:1"),
            new TestVtnPortDesc("openflow:1,2"),
        };
        for (VtnPortDesc vdesc: badFormats) {
            vpcc = mock(VtnPathCostConfig.class);
            when(vpcc.getCost()).thenReturn(1L);
            when(vpcc.getPortDesc()).thenReturn(vdesc);
            try {
                PathPolicyUtils.newBuilder(vpcc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                msg = "Invalid vtn-port-desc format: " + vdesc.getValue();
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#readVtnPathPolicies(ReadTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtnPathPolicies() throws Exception {
        InstanceIdentifier<VtnPathPolicies> path =
            InstanceIdentifier.create(VtnPathPolicies.class);
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;

        // Root container does not exist.
        VtnPathPolicies policies = null;
        ReadTransaction rtx = mock(ReadTransaction.class);
        when(rtx.read(store, path)).thenReturn(getReadResult(policies));
        assertTrue(PathPolicyUtils.readVtnPathPolicies(rtx).isEmpty());
        verify(rtx).read(store, path);

        // No path policy is defined.
        VtnPathPoliciesBuilder builder = new VtnPathPoliciesBuilder();
        policies = builder.build();
        rtx = Mockito.mock(ReadTransaction.class);
        when(rtx.read(store, path)).thenReturn(getReadResult(policies));
        assertTrue(PathPolicyUtils.readVtnPathPolicies(rtx).isEmpty());
        verify(rtx).read(store, path);

        policies = builder.setVtnPathPolicy(new ArrayList<VtnPathPolicy>()).
            build();
        rtx = Mockito.mock(ReadTransaction.class);
        when(rtx.read(store, path)).thenReturn(getReadResult(policies));
        assertTrue(PathPolicyUtils.readVtnPathPolicies(rtx).isEmpty());
        verify(rtx).read(store, path);

        // Only one path policy is defined.
        VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(1).build();
        List<VtnPathPolicy> vpplist = new ArrayList<>();
        vpplist.add(vpp);
        policies = builder.setVtnPathPolicy(vpplist).build();
        rtx = Mockito.mock(ReadTransaction.class);
        when(rtx.read(store, path)).thenReturn(getReadResult(policies));
        assertEquals(vpplist, PathPolicyUtils.readVtnPathPolicies(rtx));
        verify(rtx).read(store, path);

        // 3 path policies are defined.
        VtnPortDesc vdesc = new VtnPortDesc("openflow:1,,");
        VtnPathCost vpc = new VtnPathCostBuilder().setPortDesc(vdesc).
            setCost(10L).build();
        List<VtnPathCost> vpclist = new ArrayList<>();
        vpclist.add(vpc);
        vpp = new VtnPathPolicyBuilder().setId(2).setVtnPathCost(vpclist).
            build();
        vpplist.add(vpp);
        vdesc = new VtnPortDesc("openflow:2,,port-2");
        vpc = new VtnPathCostBuilder().setPortDesc(vdesc).setCost(3L).build();
        vpclist = new ArrayList<VtnPathCost>(vpclist);
        vpclist.add(vpc);
        vdesc = new VtnPortDesc("openflow:2,3,");
        vpc = new VtnPathCostBuilder().setPortDesc(vdesc).setCost(100L).
            build();
        vpclist.add(vpc);
        vpp = new VtnPathPolicyBuilder().setId(3).setVtnPathCost(vpclist).
            build();
        vpplist.add(vpp);

        policies = builder.setVtnPathPolicy(vpplist).build();
        rtx = Mockito.mock(ReadTransaction.class);
        when(rtx.read(store, path)).thenReturn(getReadResult(policies));
        assertEquals(vpplist, PathPolicyUtils.readVtnPathPolicies(rtx));
        verify(rtx).read(store, path);
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#readVtnPathPolicy(ReadTransaction, int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtnPathPolicy() throws Exception {
        // Define 2 path policies.
        InstanceIdentifier<VtnPathPolicy> path1 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, new VtnPathPolicyKey(1)).build();
        VtnPathPolicy vpp1 = new VtnPathPolicyBuilder().setId(1).
            setDefaultCost(100L).build();
        List<VtnPathCost> vpclist = new ArrayList<VtnPathCost>();
        VtnPortDesc vdesc = new VtnPortDesc("openflow:1,,");
        vpclist.add(new VtnPathCostBuilder().setPortDesc(vdesc).
                    setCost(10L).build());
        vdesc = new VtnPortDesc("openflow:3,2,s3-eth2");
        vpclist.add(new VtnPathCostBuilder().setPortDesc(vdesc).
                    setCost(40L).build());
        InstanceIdentifier<VtnPathPolicy> path2 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, new VtnPathPolicyKey(2)).build();
        VtnPathPolicy vpp2 = new VtnPathPolicyBuilder().setId(2).
            setDefaultCost(10L).setVtnPathCost(vpclist).build();

        InstanceIdentifier<VtnPathPolicy> path3 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, new VtnPathPolicyKey(3)).build();
        VtnPathPolicy vpp3 = null;

        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        when(rtx.read(store, path1)).thenReturn(getReadResult(vpp1));
        when(rtx.read(store, path2)).thenReturn(getReadResult(vpp2));
        when(rtx.read(store, path3)).thenReturn(getReadResult(vpp3));

        assertEquals(vpp1, PathPolicyUtils.readVtnPathPolicy(rtx, 1));
        verify(rtx).read(store, path1);
        verify(rtx, Mockito.never()).read(store, path2);
        verify(rtx, Mockito.never()).read(store, path3);

        assertEquals(vpp2, PathPolicyUtils.readVtnPathPolicy(rtx, 2));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, path2);
        verify(rtx, Mockito.never()).read(store, path3);

        try {
            PathPolicyUtils.readVtnPathPolicy(rtx, 3);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            String msg = "3: Path policy does not exist.";
            assertEquals(msg, e.getMessage());
        }

        verify(rtx).read(store, path1);
        verify(rtx).read(store, path2);
        verify(rtx).read(store, path3);
    }

    /**
     * Test case for
     * {@link PathPolicyUtils#readVtnPathCost(ReadTransaction, int, VtnPortDesc)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadVtnPathCost() throws Exception {
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        VtnPortDesc vdesc1 = new VtnPortDesc("openflow:1,,");
        VtnPortDesc vdesc2 = new VtnPortDesc("openflow:2,3,s2-eth3");
        VtnPortDesc vdesc3 = new VtnPortDesc("openflow:3,4,");
        VtnPathCostKey ckey1 = new VtnPathCostKey(vdesc1);
        VtnPathCostKey ckey2 = new VtnPathCostKey(vdesc2);
        VtnPathCostKey ckey3 = new VtnPathCostKey(vdesc3);

        VtnPathPolicyKey vppkey1 = new VtnPathPolicyKey(1);
        InstanceIdentifier<VtnPathPolicy> path1 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey1).build();
        InstanceIdentifier<VtnPathCost> cpath11 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey1).
            child(VtnPathCost.class, ckey1).build();
        InstanceIdentifier<VtnPathCost> cpath12 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey1).
            child(VtnPathCost.class, ckey2).build();
        InstanceIdentifier<VtnPathCost> cpath13 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey1).
            child(VtnPathCost.class, ckey3).build();
        VtnPathCost vpc11 = null;
        VtnPathCost vpc12 = new VtnPathCostBuilder().setPortDesc(vdesc2).
            setCost(100L).build();
        VtnPathCost vpc13 = new VtnPathCostBuilder().setPortDesc(vdesc3).
            setCost(123L).build();
        List<VtnPathCost> vpclist1 = new ArrayList<>();
        vpclist1.add(vpc12);
        vpclist1.add(vpc13);
        VtnPathPolicy vpp1 = new VtnPathPolicyBuilder().setId(1).
            setVtnPathCost(vpclist1).setDefaultCost(100L).build();
        when(rtx.read(store, path1)).thenReturn(getReadResult(vpp1));
        when(rtx.read(store, cpath11)).thenReturn(getReadResult(vpc11));
        when(rtx.read(store, cpath12)).thenReturn(getReadResult(vpc12));
        when(rtx.read(store, cpath13)).thenReturn(getReadResult(vpc13));

        VtnPathPolicyKey vppkey2 = new VtnPathPolicyKey(2);
        InstanceIdentifier<VtnPathPolicy> path2 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey2).build();
        InstanceIdentifier<VtnPathCost> cpath21 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey2).
            child(VtnPathCost.class, ckey1).build();
        InstanceIdentifier<VtnPathCost> cpath22 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey2).
            child(VtnPathCost.class, ckey2).build();
        InstanceIdentifier<VtnPathCost> cpath23 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey2).
            child(VtnPathCost.class, ckey3).build();
        List<VtnPathCost> vpclist2 = new ArrayList<>();
        VtnPathCost vpc21 = new VtnPathCostBuilder().setPortDesc(vdesc1).
            setCost(5L).build();
        VtnPathCost vpc22 = new VtnPathCostBuilder().setPortDesc(vdesc2).
            setCost(100L).build();
        VtnPathCost vpc23 = null;
        vpclist1.add(vpc21);
        vpclist1.add(vpc22);
        VtnPathPolicy vpp2 = new VtnPathPolicyBuilder().setId(2).
            setVtnPathCost(vpclist2).setDefaultCost(50L).build();
        when(rtx.read(store, path2)).thenReturn(getReadResult(vpp2));
        when(rtx.read(store, cpath21)).thenReturn(getReadResult(vpc21));
        when(rtx.read(store, cpath22)).thenReturn(getReadResult(vpc22));
        when(rtx.read(store, cpath23)).thenReturn(getReadResult(vpc23));

        VtnPathPolicyKey vppkey3 = new VtnPathPolicyKey(3);
        InstanceIdentifier<VtnPathPolicy> path3 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey3).build();
        InstanceIdentifier<VtnPathCost> cpath31 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey3).
            child(VtnPathCost.class, ckey1).build();
        InstanceIdentifier<VtnPathCost> cpath32 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey3).
            child(VtnPathCost.class, ckey2).build();
        InstanceIdentifier<VtnPathCost> cpath33 = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, vppkey3).
            child(VtnPathCost.class, ckey3).build();
        VtnPathCost vpc31 = null;
        VtnPathCost vpc32 = null;
        VtnPathCost vpc33 = null;
        VtnPathPolicy vpp3 = null;
        when(rtx.read(store, path3)).thenReturn(getReadResult(vpp3));
        when(rtx.read(store, cpath31)).thenReturn(getReadResult(vpc31));
        when(rtx.read(store, cpath32)).thenReturn(getReadResult(vpc32));
        when(rtx.read(store, cpath33)).thenReturn(getReadResult(vpc33));

        assertEquals(vpc11, PathPolicyUtils.readVtnPathCost(rtx, 1, vdesc1));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx, Mockito.never()).read(store, cpath12);
        verify(rtx, Mockito.never()).read(store, cpath13);
        verify(rtx, Mockito.never()).read(store, path2);
        verify(rtx, Mockito.never()).read(store, cpath21);
        verify(rtx, Mockito.never()).read(store, cpath22);
        verify(rtx, Mockito.never()).read(store, cpath23);
        verify(rtx, Mockito.never()).read(store, path3);
        verify(rtx, Mockito.never()).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc12, PathPolicyUtils.readVtnPathCost(rtx, 1, vdesc2));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx, Mockito.never()).read(store, cpath13);
        verify(rtx, Mockito.never()).read(store, path2);
        verify(rtx, Mockito.never()).read(store, cpath21);
        verify(rtx, Mockito.never()).read(store, cpath22);
        verify(rtx, Mockito.never()).read(store, cpath23);
        verify(rtx, Mockito.never()).read(store, path3);
        verify(rtx, Mockito.never()).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc13, PathPolicyUtils.readVtnPathCost(rtx, 1, vdesc3));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx, Mockito.never()).read(store, path2);
        verify(rtx, Mockito.never()).read(store, cpath21);
        verify(rtx, Mockito.never()).read(store, cpath22);
        verify(rtx, Mockito.never()).read(store, cpath23);
        verify(rtx, Mockito.never()).read(store, path3);
        verify(rtx, Mockito.never()).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc21, PathPolicyUtils.readVtnPathCost(rtx, 2, vdesc1));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx, Mockito.never()).read(store, path2);
        verify(rtx).read(store, cpath21);
        verify(rtx, Mockito.never()).read(store, cpath22);
        verify(rtx, Mockito.never()).read(store, cpath23);
        verify(rtx, Mockito.never()).read(store, path3);
        verify(rtx, Mockito.never()).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc22, PathPolicyUtils.readVtnPathCost(rtx, 2, vdesc2));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx, Mockito.never()).read(store, path2);
        verify(rtx).read(store, cpath21);
        verify(rtx).read(store, cpath22);
        verify(rtx, Mockito.never()).read(store, cpath23);
        verify(rtx, Mockito.never()).read(store, path3);
        verify(rtx, Mockito.never()).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc23, PathPolicyUtils.readVtnPathCost(rtx, 2, vdesc3));
        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx).read(store, path2);
        verify(rtx).read(store, cpath21);
        verify(rtx).read(store, cpath22);
        verify(rtx).read(store, cpath23);
        verify(rtx, Mockito.never()).read(store, path3);
        verify(rtx, Mockito.never()).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        try {
            PathPolicyUtils.readVtnPathCost(rtx, 3, vdesc1);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            String msg = "3: Path policy does not exist.";
            assertEquals(msg, e.getMessage());
        }

        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx).read(store, path2);
        verify(rtx).read(store, cpath21);
        verify(rtx).read(store, cpath22);
        verify(rtx).read(store, cpath23);
        verify(rtx).read(store, path3);
        verify(rtx).read(store, cpath31);
        verify(rtx, Mockito.never()).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        try {
            PathPolicyUtils.readVtnPathCost(rtx, 3, vdesc2);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            String msg = "3: Path policy does not exist.";
            assertEquals(msg, e.getMessage());
        }

        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx).read(store, path2);
        verify(rtx).read(store, cpath21);
        verify(rtx).read(store, cpath22);
        verify(rtx).read(store, cpath23);
        verify(rtx, times(2)).read(store, path3);
        verify(rtx).read(store, cpath31);
        verify(rtx).read(store, cpath32);
        verify(rtx, Mockito.never()).read(store, cpath33);

        try {
            PathPolicyUtils.readVtnPathCost(rtx, 3, vdesc3);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());
            assertEquals(null, e.getCause());
            assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
            String msg = "3: Path policy does not exist.";
            assertEquals(msg, e.getMessage());
        }

        verify(rtx).read(store, path1);
        verify(rtx).read(store, cpath11);
        verify(rtx).read(store, cpath12);
        verify(rtx).read(store, cpath13);
        verify(rtx).read(store, path2);
        verify(rtx).read(store, cpath21);
        verify(rtx).read(store, cpath22);
        verify(rtx).read(store, cpath23);
        verify(rtx, times(3)).read(store, path3);
        verify(rtx).read(store, cpath31);
        verify(rtx).read(store, cpath32);
        verify(rtx).read(store, cpath33);
    }
}
