/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * JUnit test for {@link PathPolicyConfigBuilder}
 */
public class PathPolicyConfigBuilderTest extends TestBase {
    /**
     * The minimum value of the path policy identifier.
     */
    private static final int  PATH_POLICY_MIN = 1;

    /**
     * The maximum value of the path policy identifier.
     */
    private static final int  PATH_POLICY_MAX = 3;

    /**
     * Test case for all setter methods.
     *
     * <ul>
     *   <li>{@link PathPolicyConfigBuilder#set(PathPolicy)}</li>
     *   <li>{@link PathPolicyConfigBuilder#set(PathPolicy, Integer)}</li>
     *   <li>{@link PathPolicyConfigBuilder#set(VtnPathPolicyConfig)}</li>
     *   <li>{@link PathPolicyConfigBuilder#setId(Integer)}</li>
     *   <li>{@link PathPolicyConfigBuilder#setDefaultCost(Long)}</li>
     *   <li>{@link PathPolicyConfigBuilder#setVtnPathCost(List)}</li>
     *   <li>{@link PathPolicyConfigBuilder#getBuilder()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetter() throws Exception {
        // Create a list of VtnPathCost.
        List<VtnPathCost> vpcomplete = new ArrayList<>();
        List<VtnPathCost> vpcosts = new ArrayList<>();
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

                    if (cost == 0) {
                        vpc = new VtnPathCostBuilder().setPortDesc(vdesc).
                            setCost(Long.valueOf(1L)).build();
                    }
                    vpcomplete.add(vpc);

                    PathCost pc = new PathCost(ploc, Math.max(cost, 1L));
                    pcosts.add(pc);
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
            null, Long.valueOf(1L), Long.valueOf(777L), Long.valueOf(3333L),
            Long.valueOf(9999999L), Long.valueOf(Long.MAX_VALUE),
        };

        for (Integer id: ids) {
            for (Long defCost: defCosts) {
                long defc = (defCost == null)
                    ? PathPolicy.COST_UNDEF : defCost.longValue();
                Long dc = Long.valueOf(defc);

                PathPolicy pp = new PathPolicy(id, defc, null);
                VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(id).
                    setDefaultCost(dc).build();
                VtnPathPolicy vpp1 = new PathPolicyConfigBuilder.Data().
                    set(pp).getBuilder().build();
                assertEquals(vpp, vpp1);
                SetPathPolicyInput input = new SetPathPolicyInputBuilder().
                    setId(id).setDefaultCost(dc).build();
                SetPathPolicyInput input1 = new PathPolicyConfigBuilder.Rpc().
                    set(pp).getBuilder().build();
                assertEquals(input, input1);

                pp = new PathPolicy(id, defc, pcosts);
                VtnPathPolicyConfig vpcfg = new VtnPathPolicyBuilder().
                    setId(id).setDefaultCost(dc).setVtnPathCost(vpcosts).
                    build();
                vpp = new VtnPathPolicyBuilder().setId(id).setDefaultCost(dc).
                    setVtnPathCost(vpcomplete).build();
                vpp1 = new PathPolicyConfigBuilder.Data().set(pp).
                    getBuilder().build();
                assertEquals(vpp, vpp1);
                vpp1 = new PathPolicyConfigBuilder.Data().set(vpcfg).
                    getBuilder().build();
                assertEquals(vpp, vpp1);

                input = new SetPathPolicyInputBuilder().setId(id).
                    setDefaultCost(dc).setVtnPathCost(vpcomplete).build();
                input1 = new PathPolicyConfigBuilder.Rpc().set(pp).
                    getBuilder().build();
                assertEquals(input, input1);
                input1 = new PathPolicyConfigBuilder.Rpc().set(vpcfg).
                    getBuilder().build();
                assertEquals(input, input1);

                vpp1 = new PathPolicyConfigBuilder.Data().
                    setDefaultCost(defCost).getBuilder().build();
                assertEquals(defCost, vpp1.getDefaultCost());
                input1 = new PathPolicyConfigBuilder.Rpc().
                    setDefaultCost(defCost).getBuilder().build();
                assertEquals(defCost, input1.getDefaultCost());
            }
        }

        PathPolicyConfigBuilder[] ppcBuilders = {
            new PathPolicyConfigBuilder.Data(),
            new PathPolicyConfigBuilder.Rpc(),
        };

        // Null PathPolicy test.
        for (PathPolicyConfigBuilder ppcb: ppcBuilders) {
            PathPolicy pp = null;
            try {
                ppcb.set(pp);
                unexpected();
            } catch (RpcException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("PathPolicy cannot be null", st.getDescription());
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
            }
        }

        // Invalid default cost test.
        Long[] badDefCosts = {
            Long.valueOf(Long.MIN_VALUE),
            Long.valueOf(Long.MIN_VALUE + 1),
            Long.valueOf(-1000000000000L),
            Long.valueOf(-99999999L),
            Long.valueOf(-10L),
            Long.valueOf(-5L),
            Long.valueOf(-1L),
        };
        for (Long defCost: badDefCosts) {
            PathPolicy pp = new PathPolicy(1, defCost.longValue(), null);
            for (PathPolicyConfigBuilder ppcb: ppcBuilders) {
                try {
                    ppcb.set(pp);
                    unexpected();
                } catch (RpcException e) {
                    String msg = "Invalid default cost: " + defCost;
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals(msg, st.getDescription());
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    Throwable t = e.getCause();
                    assertTrue("Unexpected cause: " + t,
                               t instanceof IllegalArgumentException);
                }

                try {
                    ppcb.setDefaultCost(defCost);
                    unexpected();
                } catch (RpcException e) {
                    String msg = "Invalid default cost: " + defCost;
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals(msg, st.getDescription());
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    Throwable t = e.getCause();
                    assertTrue("Unexpected cause: " + t,
                               t instanceof IllegalArgumentException);
                }
            }
        }

        // Invalid path policy ID test.
        Integer validId = Integer.valueOf(PATH_POLICY_MAX);
        long defc = 0L;
        for (Integer badId: badIds) {
            PathPolicy pp = new PathPolicy(badId, defc, null);
            for (PathPolicyConfigBuilder ppcb: ppcBuilders) {
                try {
                    ppcb.set(pp);
                    unexpected();
                } catch (RpcException e) {
                    Throwable t = e.getCause();
                    if (badId == null) {
                        assertEquals(null, t);
                    } else {
                        assertTrue("Unexpected cause: " + t,
                                   t instanceof IllegalArgumentException);
                    }

                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    if (badId == null) {
                        assertEquals("Path policy ID cannot be null",
                                     st.getDescription());
                        assertEquals(RpcErrorTag.MISSING_ELEMENT,
                                     e.getErrorTag());
                    } else {
                        assertEquals("Invalid path policy ID: " + badId,
                                     st.getDescription());
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    }
                }

                try {
                    ppcb.setId(badId);
                    unexpected();
                } catch (RpcException e) {
                    Throwable t = e.getCause();
                    if (badId == null) {
                        assertEquals(null, t);
                    } else {
                        assertTrue("Unexpected cause: " + t,
                                   t instanceof IllegalArgumentException);
                    }

                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    if (badId == null) {
                        assertEquals("Path policy ID cannot be null",
                                     st.getDescription());
                        assertEquals(RpcErrorTag.MISSING_ELEMENT,
                                     e.getErrorTag());
                    } else {
                        assertEquals("Invalid path policy ID: " + badId,
                                     st.getDescription());
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    }
                }
            }

            // Bad path policy ID in PathPolicy should be ignored if a valid
            // path policy ID is specified explicitly.
            VtnPathPolicy vpp = new VtnPathPolicyBuilder().
                setId(validId).setDefaultCost(defc).build();
            VtnPathPolicy vpp1 = new PathPolicyConfigBuilder.Data().
                set(pp, validId).getBuilder().build();
            assertEquals(vpp, vpp1);
            SetPathPolicyInput input = new SetPathPolicyInputBuilder().
                setId(validId).setDefaultCost(defc).build();
            SetPathPolicyInput input1 = new PathPolicyConfigBuilder.Rpc().
                set(pp, validId).getBuilder().build();
            assertEquals(input, input1);
        }

        // Dupliacte vtn-port-desc test.
        for (PathCost pc: pcosts) {
            List<PathCost> badCosts = new ArrayList<>(pcosts);
            PortLocation ploc = pc.getLocation();
            pc = new PathCost(ploc, 10L);
            badCosts.add(pc);
            PathPolicy pp = new PathPolicy(1, 10L, badCosts);
            for (PathPolicyConfigBuilder ppcb: ppcBuilders) {
                try {
                    ppcb.set(pp);
                    unexpected();
                } catch (RpcException e) {
                    String msg = "Duplicate port descriptor: " + ploc;
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals(msg, st.getDescription());
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(null, e.getCause());
                }
            }
        }

        // Bad PathCost tests.
        for (PathPolicyConfigBuilder ppcb: ppcBuilders) {
            List<PathCost> badCosts = new ArrayList<>(pcosts);
            badCosts.add(null);
            PathPolicy pp = new PathPolicy(1, 10L, badCosts);
            try {
                ppcb.set(pp);
                unexpected();
            } catch (RpcException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("PathCost cannot be null", st.getDescription());
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
            }

            badCosts = new ArrayList<>(pcosts);
            badCosts.add(new PathCost(null, 5L));
            pp = new PathPolicy(1, 10L, badCosts);
            try {
                ppcb.set(pp);
                unexpected();
            } catch (RpcException e) {
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals("Port location cannot be null",
                             st.getDescription());
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
            }
        }
    }
}
