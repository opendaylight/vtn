/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.pathpolicy;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcResult;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathCostOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.SetPathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPortDescResult;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code PathPolicy} describes the configuration of path policy.
 */
public final class PathPolicy {
    /**
     * The default value of default-cost.
     */
    private static final Long  DEFAULT_DEF_COST = 0L;

    /**
     * The identifier for the path policy.
     */
    private final Integer  policyId;

    /**
     * The default cost for unspecified link.
     */
    private Long  defaultCost;

    /**
     * A map that keeps path costs.
     */
    private final Map<VtnPortDesc, PathCost>  pathCosts = new HashMap<>();

    /**
     * Create a new input for set-path-cost RPC.
     *
     * @param policy  The path policy ID.
     * @param costs   An array of path costs to be added.
     * @return  A {@link SetPathCostInput} instance.
     */
    public static SetPathCostInput newSetCostInput(
        Integer policy, PathCost ... costs) {
        List<PathCostList> pclist = (costs == null)
            ? null : PathCost.toPathCostList(costs);
        return new SetPathCostInputBuilder().
            setId(policy).
            setPathCostList(pclist).
            build();
    }

    /**
     * Create a new input for set-path-cost RPC.
     *
     * @param policy  The path policy ID.
     * @param costs   A collection of path costs to be added.
     * @return  A {@link SetPathCostInput} instance.
     */
    public static SetPathCostInput newSetCostInput(
        Integer policy, Collection<PathCost> costs) {
        List<PathCostList> pclist = (costs == null)
            ? null : PathCost.toPathCostList(costs);
        return new SetPathCostInputBuilder().
            setId(policy).
            setPathCostList(pclist).
            build();
    }

    /**
     * Create a new input for remove-path-cost RPC.
     *
     * @param policy  The path policy ID.
     * @param pdescs  An array of vtn-port-desc instances to be removed.
     * @return  A {@link RemovePathCostInput} instance.
     */
    public static RemovePathCostInput newRemoveCostInput(
        Integer policy, VtnPortDesc ... pdescs) {
        List<VtnPortDesc> pdlist = (pdescs == null)
            ? null : Arrays.asList(pdescs);
        return newRemoveCostInput(policy, pdlist);
    }

    /**
     * Create a new input for remove-path-cost RPC.
     *
     * @param policy  The path policy ID.
     * @param pdescs  A list of vtn-port-desc instances to be removed.
     * @return  A {@link RemovePathCostInput} instance.
     */
    public static RemovePathCostInput newRemoveCostInput(
        Integer policy, List<VtnPortDesc> pdescs) {
        return new RemovePathCostInputBuilder().
            setId(policy).
            setPortDesc(pdescs).
            build();
    }

    /**
     * Create a map that indicates the given result of path cost RPC.
     *
     * @param list  A list of {@link VtnPortDescResult} instances.
     * @param <T>   The type of the RPC result.
     * @return  A map that indicates the given result of path cost RPC.
     *          Note that {@code null} is returned if the given list is
     *          {@code null}.
     */
    private static <T extends VtnPortDescResult> Map<VtnPortDesc, VtnUpdateType> getResultMap(
        List<T> list) {
        Map<VtnPortDesc, VtnUpdateType> result;
        if (list == null) {
            result = null;
        } else {
            result = new HashMap<>();
            for (VtnPortDescResult pdres: list) {
                VtnPortDesc pdesc = pdres.getPortDesc();
                VtnUpdateType status = pdres.getStatus();
                assertEquals(null, result.put(pdesc, status));
            }
        }

        return result;
    }

    /**
     * Construct a new instance.
     *
     * @param id  The identifier for the path policy.
     */
    public PathPolicy(Integer id) {
        this(id, null);
    }

    /**
     * Construct a new instance.
     *
     * @param id   The identifier for the path policy.
     * @param def  The default cost for unspecified link.
     */
    public PathPolicy(Integer id, Long def) {
        policyId = id;
        defaultCost = def;
    }

    /**
     * Return the identifier for the path policy.
     *
     * @return  The identifier for the path policy.
     */
    public Integer getId() {
        return policyId;
    }

    /**
     * Return the default cost.
     *
     * @return  The default cost for unspecified link.
     */
    public Long getDefaultCost() {
        return defaultCost;
    }

    /**
     * Set the default cost.
     *
     * @param def  The default cost for unspecified link.
     * @return  This instance.
     */
    public PathPolicy setDefaultCost(Long def) {
        defaultCost = def;
        return this;
    }

    /**
     * Add the given path costs.
     *
     * @param costs  An array of path costs to be added.
     * @return  This instance.
     */
    public PathPolicy add(PathCost ... costs) {
        for (PathCost pc: costs) {
            VtnPortDesc pdesc = (pc == null) ? null : pc.getPortDesc();
            pathCosts.put(pdesc, pc);
        }

        return this;
    }

    /**
     * Remove the given path costs.
     *
     * @param costs  An array of path costs to be removed.
     * @return  This instance.
     */
    public PathPolicy remove(PathCost ... costs) {
        for (PathCost pc: costs) {
            VtnPortDesc pdesc = (pc == null) ? null : pc.getPortDesc();
            pathCosts.remove(pdesc);
        }

        return this;
    }

    /**
     * Remove the path costs specified by the given port descriptors.
     *
     * @param pdescs  An array of port descriptors.
     * @return  This instance.
     */
    public PathPolicy remove(VtnPortDesc ... pdescs) {
        for (VtnPortDesc pdesc: pdescs) {
            pathCosts.remove(pdesc);
        }

        return this;
    }

    /**
     * Clear the set of path costs.
     *
     * @return  This instance.
     */
    public PathPolicy clear() {
        pathCosts.clear();
        return this;
    }

    /**
     * Return the path cost specified by the given port descriptor.
     *
     * @param pdesc  The port descriptor.
     * @return  A {@link PathCost} instance if found.
     *          {@code null} if not found.
     */
    public PathCost getCost(VtnPortDesc pdesc) {
        return pathCosts.get(pdesc);
    }

    /**
     * Create a new input for set-path-policy RPC.
     *
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @param present  If {@code true}, the operation will fail unless the
     *                 specified path policy is present.
     * @return  A {@link SetPathPolicyInput} instance.
     */
    public SetPathPolicyInput newInput(VtnUpdateOperationType op,
                                       Boolean present) {
        List<VtnPathCost> costs;
        if (pathCosts.isEmpty()) {
            costs = null;
        } else {
            costs = new ArrayList<>(pathCosts.size());
            for (PathCost pc: pathCosts.values()) {
                VtnPathCost vpc = (pc == null) ? null : pc.toVtnPathCost();
                costs.add(vpc);
            }
        }

        return new SetPathPolicyInputBuilder().
            setId(policyId).
            setDefaultCost(defaultCost).
            setVtnPathCost(costs).
            setOperation(op).
            setPresent(present).
            build();
    }

    /**
     * Update the path policy.
     *
     * @param service  The vtn-path-policy service.
     * @param op       A {@link VtnUpdateOperationType} instance.
     * @param present  If {@code true}, the operation will fail unless the
     *                 specified path policy is present.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType update(VtnPathPolicyService service,
                                VtnUpdateOperationType op, Boolean present) {
        SetPathPolicyInput input = newInput(op, present);
        return getRpcResult(service.setPathPolicy(input));
    }

    /**
     * Add the given path cost to the path policy using set-path-cost RPC,
     * and then add it to this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param pc       A {@link PathCost} instance to be added.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType addCost(VtnPathPolicyService service, PathCost pc) {
        Map<VtnPortDesc, VtnUpdateType> result = addCosts(service, pc);
        VtnPortDesc pdesc = pc.getPortDesc();
        assertEquals(Collections.singleton(pdesc), result.keySet());
        return result.get(pdesc);
    }

    /**
     * Add the given path costs to the path policy using set-path-cost RPC,
     * and then add them to this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param costs    An array of path costs to be added.
     * @return  A map that contains the RPC result.
     */
    public Map<VtnPortDesc, VtnUpdateType> addCosts(
        VtnPathPolicyService service, PathCost ... costs) {
        List<PathCost> pclist = (costs == null) ? null : Arrays.asList(costs);
        return addCosts(service, pclist);
    }

    /**
     * Add the given path costs to the path policy using set-path-cost RPC,
     * and then add them to this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param costs    A collection of path costs to be added.
     * @return  A map that contains the RPC result.
     */
    public Map<VtnPortDesc, VtnUpdateType> addCosts(
        VtnPathPolicyService service, Collection<PathCost> costs) {
        SetPathCostInput input = newSetCostInput(policyId, costs);
        SetPathCostOutput output = getRpcOutput(service.setPathCost(input));
        Map<VtnPortDesc, VtnUpdateType> result =
            getResultMap(output.getSetPathCostResult());

        if (costs != null) {
            for (PathCost pc: costs) {
                VtnPortDesc pdesc = pc.getPortDesc();
                pathCosts.put(pdesc, pc);
            }
        }

        return result;
    }

    /**
     * Remove the given path cost form the path policy using remove-path-cost
     * RPC, and then remove it from this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param pc       A {@link PathCost} instance to be removed.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType removeCost(VtnPathPolicyService service, PathCost pc) {
        return removeCost(service, pc.getPortDesc());
    }

    /**
     * Remove the given path cost form the path policy using remove-path-cost
     * RPC, and then remove it from this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param pdesc f   The port descriptor to be removed.
     * @return  A {@link VtnUpdateType} instance returned by the RPC.
     */
    public VtnUpdateType removeCost(VtnPathPolicyService service,
                                    VtnPortDesc pdesc) {
        Map<VtnPortDesc, VtnUpdateType> result = removeCosts(service, pdesc);
        assertEquals(Collections.singleton(pdesc), result.keySet());
        return result.get(pdesc);
    }

    /**
     * Remove the given path costs form the path policy using remove-path-cost
     * RPC, and then remove them from this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param pdescs   An array of vtn-port-desc instances to be removed.
     * @return  A map that contains the RPC result.
     */
    public Map<VtnPortDesc, VtnUpdateType> removeCosts(
        VtnPathPolicyService service, VtnPortDesc ... pdescs) {
        List<VtnPortDesc> pdlist = (pdescs == null)
            ? null : Arrays.asList(pdescs);
        return removeCosts(service, pdlist);
    }

    /**
     * Remove the given path costs form the path policy using remove-path-cost
     * RPC, and then remove them from this instance.
     *
     * @param service  The vtn-path-policy service.
     * @param pdescs   A list of vtn-port-desc instances to be removed.
     * @return  A map that contains the RPC result.
     */
    public Map<VtnPortDesc, VtnUpdateType> removeCosts(
        VtnPathPolicyService service, List<VtnPortDesc> pdescs) {
        RemovePathCostInput input = newRemoveCostInput(policyId, pdescs);
        RemovePathCostOutput output =
            getRpcOutput(service.removePathCost(input));
        Map<VtnPortDesc, VtnUpdateType> result =
            getResultMap(output.getRemovePathCostResult());

        if (pdescs != null) {
            for (VtnPortDesc pdesc: pdescs) {
                pathCosts.remove(pdesc);
            }
        }

        return result;
    }

    /**
     * Verify the given path policy.
     *
     * @param vpp      A {@link VtnPathPolicy} instance.
     */
    public void verify(VtnPathPolicy vpp) {
        assertEquals(policyId, vpp.getId());

        Long def = defaultCost;
        if (def == null) {
            def = DEFAULT_DEF_COST;
        }
        assertEquals(def, vpp.getDefaultCost());

        List<VtnPathCost> vpcs = vpp.getVtnPathCost();
        if (!pathCosts.isEmpty()) {
            assertNotNull(vpcs);
            Set<VtnPortDesc> checked = new HashSet<>();
            for (VtnPathCost vpc: vpcs) {
                VtnPortDesc pdesc = vpc.getPortDesc();
                PathCost pc = pathCosts.get(pdesc);
                assertNotNull(pc);
                pc.verify(vpc);
                assertEquals(true, checked.add(pdesc));
            }
            assertEquals(checked, pathCosts.keySet());
        } else if (vpcs != null) {
            assertEquals(Collections.<VtnPathCost>emptyList(), vpcs);
        }
    }
}
