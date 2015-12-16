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

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.RemovePathPolicyInputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code PathPolicySet} describes a set of path policy configurations.
 */
public final class PathPolicySet {
    /**
     * A map that keeps path policy configurations.
     */
    private final Map<Integer, PathPolicy>  pathPolicies = new HashMap<>();

    /**
     * Remove the specified path policy.
     *
     * @param service  The vtn-path-policy RPC service.
     * @param id       The identifier for the path policy to be removed.
     */
    public static void removePathPolicy(VtnPathPolicyService service,
                                        Integer id) {
        RemovePathPolicyInput input = new RemovePathPolicyInputBuilder().
            setId(id).
            build();
        getRpcOutput(service.removePathPolicy(input), true);
    }

    /**
     * Add the given path policies.
     *
     * @param pps  An array of path policies to be added.
     * @return  This instance.
     */
    public PathPolicySet add(PathPolicy ... pps) {
        for (PathPolicy pp: pps) {
            Integer id = pp.getId();
            pathPolicies.put(id, pp);
        }

        return this;
    }

    /**
     * Remove the path policies specified by the given IDs.
     *
     * @param ids  An array of path policy IDs to be removed.
     * @return  This instance.
     */
    public PathPolicySet remove(Integer ... ids) {
        for (Integer id: ids) {
            pathPolicies.remove(id);
        }

        return this;
    }

    /**
     * Remove all the path policy configurations.
     *
     * @return  This instance.
     */
    public PathPolicySet clear() {
        pathPolicies.clear();
        return this;
    }

    /**
     * Return the path policy configuration specified by the given ID.
     *
     * @param id  The identifier for the path policy.
     * @return  A {@link PathPolicy} instance if found.
     *          {@code null} if not found.
     */
    public PathPolicy get(Integer id) {
        return pathPolicies.get(id);
    }

    /**
     * Verify the vtn-path-policies container.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     */
    public void verify(ReadTransaction rtx) {
        List<VtnPathPolicy> vpps = readPathPolicies(rtx).getVtnPathPolicy();
        if (!pathPolicies.isEmpty()) {
            assertNotNull(vpps);
            Set<Integer> checked = new HashSet<>();
            for (VtnPathPolicy vpp: vpps) {
                Integer id = vpp.getId();
                PathPolicy pp = pathPolicies.get(id);
                assertNotNull(pp);
                pp.verify(vpp);
                assertEquals(true, checked.add(id));
            }
            assertEquals(checked, pathPolicies.keySet());
        } else if (vpps != null) {
            assertEquals(Collections.<VtnPathPolicy>emptyList(), vpps);
        }
    }

    /**
     * Apply all the configurations in this instance.
     *
     * @param service  A {@link VTNServices} instance.
     * @param rtx      A read-only MD-SAL datastore transaction.
     */
    public void apply(VTNServices service, ReadTransaction rtx) {
        VtnPathPolicyService vppSrv = service.getPathPolicyService();
        removeUnwanted(vppSrv, rtx);

        for (PathPolicy pp: pathPolicies.values()) {
            pp.update(vppSrv, VtnUpdateOperationType.SET, false);
        }
    }

    /**
     * Read the vtn-path-policies container.
     *
     * @param rtx  A read-only MD-SAL datastore transaction.
     * @return  A {@link VtnPathPolicies} instance.
     */
    private VtnPathPolicies readPathPolicies(ReadTransaction rtx) {
        InstanceIdentifier<VtnPathPolicies> path = InstanceIdentifier.
            create(VtnPathPolicies.class);
        Optional<VtnPathPolicies> opt = DataStoreUtils.read(rtx, path);
        assertEquals(true, opt.isPresent());

        return opt.get();
    }

    /**
     * Remove all the path policies that are not assocaited with the
     * configuration in this instance.
     *
     * @param service  The vtn-path-policy RPC service.
     * @param rtx      A read-only MD-SAL datastore transaction.
     */
    private void removeUnwanted(
        VtnPathPolicyService service, ReadTransaction rtx) {
        List<VtnPathPolicy> vpps = readPathPolicies(rtx).getVtnPathPolicy();
        if (vpps != null) {
            if (pathPolicies.isEmpty()) {
                assertEquals(VtnUpdateType.REMOVED,
                             getRpcResult(service.clearPathPolicy()));
            } else {
                for (VtnPathPolicy vpp: vpps) {
                    Integer id = vpp.getId();
                    if (!pathPolicies.containsKey(id)) {
                        removePathPolicy(service, id);
                    }
                }
            }
        }
    }
}
