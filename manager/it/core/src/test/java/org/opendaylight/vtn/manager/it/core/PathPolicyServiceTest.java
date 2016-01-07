/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.core;

import static org.opendaylight.vtn.manager.it.core.VTNManagerIT.LOG;
import static org.opendaylight.vtn.manager.it.ofmock.OfMockService.ID_OPENFLOW;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.opendaylight.vtn.manager.it.util.RpcErrorTag;
import org.opendaylight.vtn.manager.it.util.VirtualNetwork;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathCost;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicy;
import org.opendaylight.vtn.manager.it.util.pathpolicy.PathPolicySet;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * Test case for {@link VtnPathPolicyService}.
 */
public final class PathPolicyServiceTest extends TestMethodBase {
    /**
     * Construct a new instance.
     *
     * @param vit  A {@link VTNManagerIT} instance.
     */
    public PathPolicyServiceTest(VTNManagerIT vit) {
        super(vit);
    }

    // TestMethodBase

    /**
     * Run the test.
     *
     * @throws Exception  An error occurred.
     */
    @Override
    protected void runTest() throws Exception {
        Random rand = new Random(17320508L);
        VTNManagerIT vit = getTest();
        VtnPathPolicyService ppSrv = vit.getPathPolicyService();
        for (int id = PATH_POLICY_ID_MIN; id <= PATH_POLICY_ID_MAX; id++) {
            testPathPolicyService(ppSrv, rand, id);
        }

        // Error tests.

        // Null input.
        checkRpcError(ppSrv.setPathPolicy(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(ppSrv.removePathPolicy(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(ppSrv.setPathCost(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        checkRpcError(ppSrv.removePathCost(null),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No path policy ID.
        String node = ID_OPENFLOW + "1";
        PathCost pc = new PathCost(node, "2", null, 123L);
        PathPolicy tmpp = new PathPolicy(null, 1L);
        tmpp.add(pc);

        SetPathPolicyInput input = tmpp.newInput(null, false);
        checkRpcError(ppSrv.setPathPolicy(input),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        RemovePathPolicyInput rinput = new RemovePathPolicyInputBuilder().
            build();
        checkRpcError(ppSrv.removePathPolicy(rinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        SetPathCostInput cinput = PathPolicy.newSetCostInput(null, pc);
        checkRpcError(ppSrv.setPathCost(cinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        RemovePathCostInput rcinput =
            PathPolicy.newRemoveCostInput(null, pc.getPortDesc());
        checkRpcError(ppSrv.removePathCost(rcinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid path policy ID.
        Integer[] badIds = {
            Integer.MIN_VALUE, -1000, -2, -1,
            0, 4, 5, 100, 9999999, Integer.MAX_VALUE,
        };
        for (Integer id: badIds) {
            cinput = PathPolicy.newSetCostInput(id, pc);
            checkRpcError(ppSrv.setPathCost(cinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rinput = new RemovePathPolicyInputBuilder().setId(id).build();
            checkRpcError(ppSrv.removePathPolicy(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

            rcinput = PathPolicy.newRemoveCostInput(id, pc.getPortDesc());
            checkRpcError(ppSrv.removePathCost(rcinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        // Errors should never affect path policies.
        VirtualNetwork vnet = getVirtualNetwork();
        vnet.verify();

        // Remove path policies.
        PathPolicySet policySet = vnet.getPathPolicies();
        List<PathPolicy> pplist = new ArrayList<>();
        for (int id = PATH_POLICY_ID_MIN; id <= PATH_POLICY_ID_MAX; id++) {
            Integer policy = Integer.valueOf(id);
            pplist.add(policySet.get(policy));
            policySet.remove(policy);
            rinput = new RemovePathPolicyInputBuilder().setId(policy).build();
            removePathPolicy(policy);
            vnet.verify();

            rinput = new RemovePathPolicyInputBuilder().setId(policy).build();
            checkRpcError(ppSrv.removePathPolicy(rinput),
                          RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);
        }

        for (PathPolicy pp: pplist) {
            // Create an empty path policy.
            Integer id = pp.getId();
            tmpp = new PathPolicy(id);
            assertEquals(VtnUpdateType.CREATED,
                         tmpp.update(ppSrv, null, null));
            policySet.add(tmpp);
            vnet.verify();

            // Restore path policy by SET operation.
            assertEquals(VtnUpdateType.CHANGED,
                         pp.update(ppSrv, VtnUpdateOperationType.SET, true));
            policySet.add(pp);
            vnet.verify();

            assertEquals(null,
                         pp.update(ppSrv, VtnUpdateOperationType.SET, true));
            assertEquals(null,
                         pp.update(ppSrv, VtnUpdateOperationType.ADD, true));
            vnet.verify();
        }

        // Remove all the path policies by clear-path-policy.
        assertEquals(VtnUpdateType.REMOVED,
                     getRpcResult(ppSrv.clearPathPolicy()));
        policySet.clear();
        vnet.verify();

        assertEquals(null, getRpcResult(ppSrv.clearPathPolicy()));
        vnet.verify();
    }

    /**
     * Test case for {@link VtnPathPolicyService}.
     *
     * @param ppSrv   vtn-path-policy service.
     * @param rand    A pseudo random generator.
     * @param policy  The target path policy ID.
     * @throws Exception  An error occurred.
     */
    private void testPathPolicyService(VtnPathPolicyService ppSrv, Random rand,
                                       Integer policy) throws Exception {
        LOG.debug("testPathPolicyService(): id={}", policy);

        String[] nodes = {
            ID_OPENFLOW + "1",
            ID_OPENFLOW + "12345",
            ID_OPENFLOW + "9999999",
            ID_OPENFLOW + "18446744073709551615",
            ID_OPENFLOW + "11223344556677",
            ID_OPENFLOW + "7777777777",
        };

        VirtualNetwork vnet = getVirtualNetwork();
        Long def = (policy.intValue() == PATH_POLICY_ID_MIN) ? null : 0L;
        PathPolicy pp = new PathPolicy(policy, def);

        PathCost pc1 = new PathCost(nodes[0], "2", null, null);
        PathCost pc2 = new PathCost(nodes[1], null, "port-3", 1L);
        PathCost pc3 = new PathCost(nodes[2], "15", "port-15", Long.MAX_VALUE);
        PathCost pc4 = new PathCost(nodes[3], "12345", null,
                                    createNaturalLong(rand));
        pp.add(pc1, pc2, pc3, pc4);

        // NOTFOUND tests.
        SetPathPolicyInput input = pp.newInput(null, true);
        checkRpcError(ppSrv.setPathPolicy(input),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        RemovePathPolicyInput rinput = new RemovePathPolicyInputBuilder().
            setId(policy).
            build();
        checkRpcError(ppSrv.removePathPolicy(rinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        SetPathCostInput cinput = PathPolicy.newSetCostInput(policy, pc1, pc2);
        checkRpcError(ppSrv.setPathCost(cinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        RemovePathCostInput rcinput = PathPolicy.newRemoveCostInput(
            policy, pc1.getPortDesc(), pc2.getPortDesc());
        checkRpcError(ppSrv.removePathCost(rcinput),
                      RpcErrorTag.DATA_MISSING, VtnErrorTag.NOTFOUND);

        // Create a path policy.
        Boolean present = (policy.intValue() == PATH_POLICY_ID_MIN)
            ? null : Boolean.FALSE;
        assertEquals(VtnUpdateType.CREATED,
                     pp.update(ppSrv, VtnUpdateOperationType.ADD, present));

        PathPolicySet policySet = vnet.getPathPolicies();
        policySet.add(pp);
        vnet.verify();

        // Retry with the same input.
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            assertEquals(null, pp.update(ppSrv, op, true));
        }
        vnet.verify();

        // Change the default cost.
        def = Long.MAX_VALUE;
        pp.setDefaultCost(def);
        assertEquals(VtnUpdateType.CHANGED,
                     pp.update(ppSrv, VtnUpdateOperationType.SET, false));
        vnet.verify();

        for (int i = 0; i < 5; i++) {
            do {
                def = createPositiveLong(rand);
            } while (def.equals(pp.getDefaultCost()));
            PathPolicy tmp = new PathPolicy(policy, def);
            assertEquals(VtnUpdateType.CHANGED,
                         tmp.update(ppSrv, VtnUpdateOperationType.ADD, true));
            pp.setDefaultCost(def);
            vnet.verify();

            assertEquals(null,
                         tmp.update(ppSrv, VtnUpdateOperationType.ADD, true));
        }

        // Change whole path policy configuration.
        pc1 = new PathCost(nodes[5], null, null, createNaturalLong(rand));
        pc2 = new PathCost(nodes[3], null, "eth9", createNaturalLong(rand));
        pc3 = new PathCost(nodes[4], "1234567", null, createNaturalLong(rand));
        pc4 = new PathCost(nodes[1], "4", "port-4", null);
        PathCost pc5 = new PathCost(nodes[0], null, "port-41", 1L);
        pp.clear().add(pc1, pc2, pc3, pc4, pc5);
        assertEquals(VtnUpdateType.CHANGED,
                     pp.update(ppSrv, VtnUpdateOperationType.SET, present));
        vnet.verify();

        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            assertEquals(null, pp.update(ppSrv, op, true));
        }
        vnet.verify();

        // Set cost for a specific port in the same node.
        PathCost pc6 = new PathCost(nodes[2], null, null,
                                    createNaturalLong(rand));
        assertEquals(VtnUpdateType.CREATED, pp.addCost(ppSrv, pc6));
        vnet.verify();

        String pid1 = "1";
        String pname1 = "port-name-1";
        Map<VtnPortDesc, VtnUpdateType> pcResult = new HashMap<>();
        assertEquals(null, pcResult.put(pc1.getPortDesc(), null));
        assertEquals(null, pcResult.put(pc6.getPortDesc(), null));
        PathCost pc7 = new PathCost(nodes[2], null, pname1, 34455L);
        assertEquals(null,
                     pcResult.put(pc7.getPortDesc(), VtnUpdateType.CREATED));
        assertEquals(pcResult, pp.addCosts(ppSrv, pc1, pc6, pc7));
        vnet.verify();

        pcResult.clear();
        pc7.setCost(34456L);
        assertEquals(null,
                     pcResult.put(pc7.getPortDesc(), VtnUpdateType.CHANGED));
        PathCost pc8 = new PathCost(nodes[2], pid1, null, 1234567890L);
        assertEquals(null,
                     pcResult.put(pc8.getPortDesc(), VtnUpdateType.CREATED));
        PathCost pc9 = new PathCost(nodes[2], pid1, pname1, 7777777777777L);
        assertEquals(null,
                     pcResult.put(pc9.getPortDesc(), VtnUpdateType.CREATED));
        assertEquals(pcResult, pp.addCosts(ppSrv, pc7, pc8, pc9));
        vnet.verify();

        String pid2 = "2";
        String pname2 = "port-name-2";
        pcResult.clear();
        PathCost pc10 = new PathCost(nodes[2], pid2, null,
                                     createNaturalLong(rand));
        PathCost pc11 = new PathCost(nodes[2], null, pname2,
                                     createNaturalLong(rand));
        PathCost pc12 = new PathCost(nodes[2], pid2, pname2,
                                     createNaturalLong(rand));
        assertEquals(null,
                     pcResult.put(pc10.getPortDesc(), VtnUpdateType.CREATED));
        assertEquals(null,
                     pcResult.put(pc11.getPortDesc(), VtnUpdateType.CREATED));
        assertEquals(null,
                     pcResult.put(pc12.getPortDesc(), VtnUpdateType.CREATED));
        assertEquals(pcResult, pp.addCosts(ppSrv, pc10, pc11, pc12));
        vnet.verify();

        // Remove path costs.
        assertEquals(VtnUpdateType.REMOVED, pp.removeCost(ppSrv, pc8));
        vnet.verify();

        pcResult.clear();
        assertEquals(null,
                     pcResult.put(pc4.getPortDesc(), VtnUpdateType.REMOVED));
        assertEquals(null,
                     pcResult.put(pc12.getPortDesc(), VtnUpdateType.REMOVED));
        assertEquals(pcResult, pp.removeCosts(ppSrv, pc4.getPortDesc(),
                                              pc12.getPortDesc()));
        vnet.verify();

        pcResult.clear();
        assertEquals(null, pcResult.put(pc4.getPortDesc(), null));
        assertEquals(null, pcResult.put(pc8.getPortDesc(), null));
        assertEquals(null, pcResult.put(pc12.getPortDesc(), null));
        assertEquals(null,
                     pcResult.put(pc2.getPortDesc(), VtnUpdateType.REMOVED));
        assertEquals(null,
                     pcResult.put(pc9.getPortDesc(), VtnUpdateType.REMOVED));
        assertEquals(null,
                     pcResult.put(pc10.getPortDesc(), VtnUpdateType.REMOVED));

        // Unsupported port descriptors should be ignored.
        VtnPortDesc unsupported1 = new VtnPortDesc("unknown:1,,");
        VtnPortDesc unsupported2 =
            new VtnPortDesc("unknown:2,port-3,port-name-3");
        assertEquals(null, pcResult.put(unsupported1, null));
        assertEquals(null, pcResult.put(unsupported2, null));

        // Duplicates should be eliminated automatically.
        assertEquals(pcResult, pp.removeCosts(ppSrv, pc2.getPortDesc(),
                                              pc4.getPortDesc(),
                                              pc8.getPortDesc(),
                                              pc9.getPortDesc(),
                                              pc10.getPortDesc(),
                                              pc12.getPortDesc(),
                                              pc4.getPortDesc(),
                                              pc9.getPortDesc(),
                                              pc12.getPortDesc(),
                                              unsupported1, unsupported2));
        vnet.verify();

        // Change the default cost only.
        for (int i = 0; i < 5; i++) {
            do {
                def = createPositiveLong(rand);
            } while (def.equals(pp.getDefaultCost()));
            PathPolicy tmp = new PathPolicy(policy, def);
            assertEquals(VtnUpdateType.CHANGED,
                         tmp.update(ppSrv, VtnUpdateOperationType.ADD, true));
            pp.setDefaultCost(def);
            vnet.verify();

            assertEquals(null,
                         tmp.update(ppSrv, VtnUpdateOperationType.ADD, true));
        }

        errorTest(ppSrv, policy);

        // Errors should never affect path policies.
        vnet.verify();
    }

    /**
     * Error test case for RPCs that specifies the path policy ID.
     *
     * @param ppSrv   vtn-path-policy service.
     * @param policy  The target path policy ID.
     * @throws Exception  An error occurred.
     */
    private void errorTest(VtnPathPolicyService ppSrv, Integer policy)
        throws Exception {
        String node = ID_OPENFLOW + "1";
        PathCost pc = new PathCost(node, "2", null, 123L);

        // Null path cost.
        PathPolicy pp = new PathPolicy(policy, 1L).add(pc, null);
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            SetPathPolicyInput input = pp.newInput(op, false);
            checkRpcError(ppSrv.setPathPolicy(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        List<PathCost> costs = new ArrayList<>();
        Collections.addAll(costs, pc, null);
        SetPathCostInput cinput = PathPolicy.newSetCostInput(policy, costs);
        checkRpcError(ppSrv.setPathCost(cinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // Invalid update operation.
        pp.clear();
        SetPathPolicyInput input =
            pp.newInput(VtnUpdateOperationType.REMOVE, false);
        checkRpcError(ppSrv.setPathPolicy(input),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);

        // Null port descriptor.
        List<VtnPortDesc> pdescs = new ArrayList<>();
        Collections.addAll(pdescs, pc.getPortDesc(), null);
        RemovePathCostInput rcinput =
            PathPolicy.newRemoveCostInput(policy, pdescs);
        checkRpcError(ppSrv.removePathCost(rcinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        // No port descriptor is specified
        pp.clear().add(pc, new PathCost(1L));
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            input = pp.newInput(op, false);
            checkRpcError(ppSrv.setPathPolicy(input),
                          RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        cinput = PathPolicy.newSetCostInput(policy, (List<PathCost>)null);
        checkRpcError(ppSrv.setPathCost(cinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        cinput = PathPolicy.newSetCostInput(
            policy, Collections.<PathCost>emptyList());
        checkRpcError(ppSrv.setPathCost(cinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        rcinput = PathPolicy.newRemoveCostInput(
            policy, (List<VtnPortDesc>)null);
        checkRpcError(ppSrv.removePathCost(rcinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);
        rcinput = PathPolicy.newRemoveCostInput(
            policy, Collections.<VtnPortDesc>emptyList());
        checkRpcError(ppSrv.removePathCost(rcinput),
                      RpcErrorTag.MISSING_ELEMENT, VtnErrorTag.BADREQUEST);

        PathCost[] badCosts = {
            // Unsupported node.
            new PathCost("unknown:1", null, null, 1L),
            new PathCost("openflow123:1", "3", "port-3", 1L),

            // Invalid port number.
            new PathCost(node, "bad-port-number", null, 1L),
            new PathCost(node, "-1", null, 1L),
            new PathCost(node, "4294967041", null, 1L),
        };
        for (PathCost pcost: badCosts) {
            pp.clear().add(pc, pcost);
            for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
                input = pp.newInput(op, false);
                checkRpcError(ppSrv.setPathPolicy(input),
                              RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
            }

            cinput = PathPolicy.newSetCostInput(policy, pcost);
            checkRpcError(ppSrv.setPathCost(cinput),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        // Duplicate port descriptor.
        PathCost pc1 = new PathCost(node, "3", null, null);
        PathCost pc2 = new PathCost(
            node, pc.getPortId(), pc.getPortName(), 9L);
        List<VtnPathCost> vcosts = new ArrayList<>();
        Collections.addAll(
            vcosts, pc.toVtnPathCost(), pc1.toVtnPathCost(),
            pc2.toVtnPathCost());
        for (VtnUpdateOperationType op: MODIFY_OPERATIONS) {
            input = new SetPathPolicyInputBuilder().
                setId(policy).
                setDefaultCost(1L).
                setVtnPathCost(vcosts).
                setOperation(op).
                build();
            checkRpcError(ppSrv.setPathPolicy(input),
                          RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
        }

        List<PathCost> pcosts = new ArrayList<>();
        Collections.addAll(pcosts, pc, pc1, pc2);
        cinput = PathPolicy.newSetCostInput(policy, pcosts);
        checkRpcError(ppSrv.setPathCost(cinput),
                      RpcErrorTag.BAD_ELEMENT, VtnErrorTag.BADREQUEST);
    }
}
