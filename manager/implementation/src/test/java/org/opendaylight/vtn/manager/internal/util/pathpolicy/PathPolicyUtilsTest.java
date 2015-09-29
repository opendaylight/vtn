/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.IdentifiableItem;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.Item;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

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
     *   <li>{@link PathPolicyUtils#getInvalidPolicyIdException(Integer)}</li>
     *   <li>{@link PathPolicyUtils#getInvalidDefaultCostException(Long)}</li>
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

        // getInvalidPolicyIdException(int)
        for (int i = PATH_POLICY_MAX + 1; i < PATH_POLICY_MAX + 10; i++) {
            RpcException e = PathPolicyUtils.getInvalidPolicyIdException(i);
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            String msg = "Invalid path policy ID: " + i;
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }

        // getInvalidDefaultCostException(Long)
        for (long c = -5L; c < 0; c++) {
            RpcException e = PathPolicyUtils.getInvalidDefaultCostException(c);
            assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
            assertEquals(null, e.getCause());
            String msg = "Invalid default cost: " + c;
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
        SalPort sport = new SalPort(1L, 2L);
        Object[] locs = {
            new PortLocation(sport.getAdNodeConnector(), null),
            "openflow:1,,,,",
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
     * Test case for {@link PathPolicyUtils#getIdentifier(org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig)}
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
     * Test case for {@link PathPolicyUtils#getIdentifier(int, org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig)}
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
     * Test case for {@link PathPolicyUtils#toPathCost(VtnPathCost)}.
     */
    @Test
    public void testToPathCost() {
        VtnPortDesc[] invalidDescs = {
            null,
            new VtnPortDesc("unknown,,"),
            new VtnPortDesc("openflow:bad,1,?"),
        };
        for (VtnPortDesc vdesc: invalidDescs) {
            VtnPathCost vpc = new VtnPathCostBuilder().setPortDesc(vdesc).
                build();
            assertEquals(null, PathPolicyUtils.toPathCost(vpc));
        }

        Long[] costs = {
            null, Long.valueOf(1L), Long.valueOf(2L), Long.valueOf(100L),
            Long.valueOf(12345678L), Long.valueOf(Long.MAX_VALUE),
        };

        long[] dpids = {
            Long.MIN_VALUE, -0x123456789abcdefL, -1L,
            0L, 1L, 10L, 12345L, 0xabcdef1234567L, Long.MAX_VALUE,
        };
        String[] ports = {
            null, "1", "2", "3",
        };
        String[] names = {
            null, "port-1", "port-2", "port-3",
        };

        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            Node node = snode.getAdNode();
            for (String port: ports) {
                String p;
                String type;
                if (port == null) {
                    p = "";
                    type = null;
                } else {
                    p = port;
                    type = NodeConnectorIDType.OPENFLOW;
                }
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    String pd = joinStrings(null, null, ",", snode, p, n);
                    VtnPortDesc vdesc = new VtnPortDesc(pd);
                    SwitchPort swport = (port == null && name == null)
                        ? null
                        : new SwitchPort(name, type, port);
                    PortLocation ploc = new PortLocation(node, swport);
                    for (Long cost: costs) {
                        VtnPathCost vpc = new VtnPathCostBuilder().
                            setPortDesc(vdesc).setCost(cost).build();
                        long c = (cost == null) ? 1L : cost.longValue();
                        PathCost expected = new PathCost(ploc, c);
                        PathCost pc = PathPolicyUtils.toPathCost(vpc);
                        assertEquals(expected, pc);
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link PathPolicyUtils#toPathPolicy(VtnPathPolicy)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToPathPolicy() throws Exception {
        // Create a list of VtnPathCost.
        List<VtnPathCost> vpcosts = new ArrayList<>();
        List<VtnPathCost> vpcosts1 = new ArrayList<>();
        List<PathCost> pcosts = new ArrayList<>();
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

        // This VtnPathCost should be ignored.
        vpcosts.add(new VtnPathCostBuilder().setCost(Long.valueOf(1L)).
                    build());

        long cost = 0;
        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            Node node = snode.getAdNode();
            for (String port: ports) {
                String p;
                String type;
                if (port == null) {
                    p = "";
                    type = null;
                } else {
                    p = port;
                    type = NodeConnectorIDType.OPENFLOW;
                }
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    String pd = joinStrings(null, null, ",", snode, p, n);
                    VtnPortDesc vdesc = new VtnPortDesc(pd);
                    SwitchPort swport = (port == null && name == null)
                        ? null
                        : new SwitchPort(name, type, port);
                    PortLocation ploc = new PortLocation(node, swport);
                    Long c = (cost == 0) ? null : Long.valueOf(cost);
                    VtnPathCost vpc = new VtnPathCostBuilder().
                        setPortDesc(vdesc).setCost(c).build();
                    vpcosts.add(vpc);
                    vpcosts1.add(vpc);

                    PathCost pc = new PathCost(ploc, Math.max(cost, 1L));
                    pcosts.add(pc);
                    cost++;
                }
            }
        }

        List<Integer> ids = new ArrayList<>();
        ids.add(null);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            ids.add(Integer.valueOf(id));
        }

        Long[] defCosts = {
            null, Long.valueOf(1L), Long.valueOf(777L), Long.valueOf(3333L),
            Long.valueOf(9999999L), Long.valueOf(Long.MAX_VALUE),
        };

        for (Integer id: ids) {
            for (Long defCost: defCosts) {
                long defc = (defCost == null)
                    ? PathPolicy.COST_UNDEF : defCost.longValue();

                VtnPathPolicy vpp = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).build();
                PathPolicy expected = new PathPolicy(id, defc, null);
                assertEquals(expected, PathPolicyUtils.toPathPolicy(vpp));

                vpp = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).
                    setVtnPathCost(new ArrayList<VtnPathCost>()).build();
                assertEquals(expected, PathPolicyUtils.toPathPolicy(vpp));

                vpp = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(defCost).setVtnPathCost(vpcosts1)
                    .build();
                expected = new PathPolicy(id, defc, pcosts);
                assertEquals(expected, PathPolicyUtils.toPathPolicy(vpp));
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
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        Mockito.when(rtx.read(store, path)).
            thenReturn(getReadResult(policies));
        assertTrue(PathPolicyUtils.readVtnPathPolicies(rtx).isEmpty());
        Mockito.verify(rtx, Mockito.times(1)).read(store, path);

        // No path policy is defined.
        VtnPathPoliciesBuilder builder = new VtnPathPoliciesBuilder();
        policies = builder.build();
        rtx = Mockito.mock(ReadTransaction.class);
        Mockito.when(rtx.read(store, path)).
            thenReturn(getReadResult(policies));
        assertTrue(PathPolicyUtils.readVtnPathPolicies(rtx).isEmpty());
        Mockito.verify(rtx, Mockito.times(1)).read(store, path);

        policies = builder.setVtnPathPolicy(new ArrayList<VtnPathPolicy>()).
            build();
        rtx = Mockito.mock(ReadTransaction.class);
        Mockito.when(rtx.read(store, path)).
            thenReturn(getReadResult(policies));
        assertTrue(PathPolicyUtils.readVtnPathPolicies(rtx).isEmpty());
        Mockito.verify(rtx, Mockito.times(1)).read(store, path);

        // Only one path policy is defined.
        VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(1).build();
        List<VtnPathPolicy> vpplist = new ArrayList<>();
        vpplist.add(vpp);
        policies = builder.setVtnPathPolicy(vpplist).build();
        rtx = Mockito.mock(ReadTransaction.class);
        Mockito.when(rtx.read(store, path)).
            thenReturn(getReadResult(policies));
        assertEquals(vpplist, PathPolicyUtils.readVtnPathPolicies(rtx));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path);

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
        Mockito.when(rtx.read(store, path)).
            thenReturn(getReadResult(policies));
        assertEquals(vpplist, PathPolicyUtils.readVtnPathPolicies(rtx));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path);
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
        Mockito.when(rtx.read(store, path1)).thenReturn(getReadResult(vpp1));
        Mockito.when(rtx.read(store, path2)).thenReturn(getReadResult(vpp2));
        Mockito.when(rtx.read(store, path3)).thenReturn(getReadResult(vpp3));

        assertEquals(vpp1, PathPolicyUtils.readVtnPathPolicy(rtx, 1));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.never()).read(store, path2);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);

        assertEquals(vpp2, PathPolicyUtils.readVtnPathPolicy(rtx, 2));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path2);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);

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

        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path3);
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
        Mockito.when(rtx.read(store, path1)).thenReturn(getReadResult(vpp1));
        Mockito.when(rtx.read(store, cpath11)).
            thenReturn(getReadResult(vpc11));
        Mockito.when(rtx.read(store, cpath12)).
            thenReturn(getReadResult(vpc12));
        Mockito.when(rtx.read(store, cpath13)).
            thenReturn(getReadResult(vpc13));

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
        Mockito.when(rtx.read(store, path2)).thenReturn(getReadResult(vpp2));
        Mockito.when(rtx.read(store, cpath21)).
            thenReturn(getReadResult(vpc21));
        Mockito.when(rtx.read(store, cpath22)).
            thenReturn(getReadResult(vpc22));
        Mockito.when(rtx.read(store, cpath23)).
            thenReturn(getReadResult(vpc23));

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
        Mockito.when(rtx.read(store, path3)).thenReturn(getReadResult(vpp3));
        Mockito.when(rtx.read(store, cpath31)).
            thenReturn(getReadResult(vpc31));
        Mockito.when(rtx.read(store, cpath32)).
            thenReturn(getReadResult(vpc32));
        Mockito.when(rtx.read(store, cpath33)).
            thenReturn(getReadResult(vpc33));

        assertEquals(vpc11, PathPolicyUtils.readVtnPathCost(rtx, 1, vdesc1));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath12);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath13);
        Mockito.verify(rtx, Mockito.never()).read(store, path2);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath21);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath22);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath23);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc12, PathPolicyUtils.readVtnPathCost(rtx, 1, vdesc2));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath13);
        Mockito.verify(rtx, Mockito.never()).read(store, path2);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath21);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath22);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath23);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc13, PathPolicyUtils.readVtnPathCost(rtx, 1, vdesc3));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.never()).read(store, path2);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath21);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath22);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath23);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc21, PathPolicyUtils.readVtnPathCost(rtx, 2, vdesc1));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.never()).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath21);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath22);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath23);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc22, PathPolicyUtils.readVtnPathCost(rtx, 2, vdesc2));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.never()).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath21);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath22);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath23);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

        assertEquals(vpc23, PathPolicyUtils.readVtnPathCost(rtx, 2, vdesc3));
        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath21);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath22);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath23);
        Mockito.verify(rtx, Mockito.never()).read(store, path3);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

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

        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath21);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath22);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath23);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path3);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath31);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

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

        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath21);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath22);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath23);
        Mockito.verify(rtx, Mockito.times(2)).read(store, path3);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath31);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath32);
        Mockito.verify(rtx, Mockito.never()).read(store, cpath33);

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

        Mockito.verify(rtx, Mockito.times(1)).read(store, path1);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath11);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath12);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath13);
        Mockito.verify(rtx, Mockito.times(1)).read(store, path2);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath21);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath22);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath23);
        Mockito.verify(rtx, Mockito.times(3)).read(store, path3);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath31);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath32);
        Mockito.verify(rtx, Mockito.times(1)).read(store, cpath33);
    }

    /**
     * Test case for {@link PathPolicyUtils#createRpcInput(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateRpcInput() throws Exception {
        final int delta = 5;
        for (int i = PATH_POLICY_MIN - delta; i <= PATH_POLICY_MAX + delta;
             i++) {
            if (i >= PATH_POLICY_MIN && i <= PATH_POLICY_MAX) {
                // Valid path policy ID.
                PathPolicyConfigBuilder.Rpc input =
                    PathPolicyUtils.createRpcInput(i);
                SetPathPolicyInputBuilder builder = input.getBuilder();
                assertEquals(Integer.valueOf(i), builder.getId());
                assertEquals(null, builder.getDefaultCost());
                assertEquals(null, builder.getVtnPathCost());
                assertEquals(VtnUpdateOperationType.ADD,
                             builder.getOperation());
                assertEquals(Boolean.TRUE, builder.isPresent());
            } else {
                // Invalid path policy ID.
                try {
                    PathPolicyUtils.createRpcInput(i);
                    unexpected();
                } catch (RpcException e) {
                    // This error should be treated as if the target path
                    // policy is not present.
                    String msg = i + ": Path policy does not exist.";
                    assertEquals(VtnErrorTag.NOTFOUND, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                    assertEquals(RpcErrorTag.DATA_MISSING, e.getErrorTag());

                    Throwable t = e.getCause();
                    assertTrue(t instanceof RpcException);
                    RpcException cause = (RpcException)t;
                    assertEquals(RpcErrorTag.BAD_ELEMENT, cause.getErrorTag());
                    msg = "Invalid path policy ID: " + i;
                    assertEquals(VtnErrorTag.BADREQUEST,
                                 cause.getVtnErrorTag());
                    assertEquals(msg, cause.getMessage());

                    t = cause.getCause();
                    assertTrue("Unexpected cause: " + t,
                               t instanceof IllegalArgumentException);
                }
            }
        }
    }
}
