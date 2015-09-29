/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateOperationType;

/**
 * {@code PathPolicyUtils} class is a collection of utility class methods
 * for path policy.
 */
public final class PathPolicyUtils {
    /**
     * The path policy ID associated with the system default routing policy.
     */
    public static final int  DEFAULT_POLICY = 0;

    /**
     * Default link cost used when the path policy does not exist.
     */
    public static final Long  DEFAULT_LINK_COST = Long.valueOf(1L);

    /**
     * The pseudo link cost value which represents the cost is not defined.
     */
    public static final long  COST_UNDEF = 0L;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private PathPolicyUtils() {}

    /**
     * Return a new {@link RpcException} that indicates the specified path
     * policy is not present.
     *
     * @param id  The identifier of the path policy.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(int id) {
        return getNotFoundException(id, null);
    }

    /**
     * Return a new {@link RpcException} that indicates the specified path
     * policy is not present.
     *
     * @param id     The identifier of the path policy.
     * @param cause  A {@link Throwable} which indicates the cause of error.
     * @return  An {@link RpcException}.
     */
    public static RpcException getNotFoundException(int id, Throwable cause) {
        String msg = MiscUtils.joinColon(id, "Path policy does not exist.");
        return RpcException.getNotFoundException(msg, cause);
    }

    /**
     * Return a new {@link RpcException} that indicates the given path policy
     * ID is invalid.
     *
     * @param id  The identifier of the path policy.
     * @return  An {@link RpcException}.
     */
    public static RpcException getInvalidPolicyIdException(Integer id) {
        String msg = MiscUtils.joinColon("Invalid path policy ID", id);
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Return a new {@link RpcException} that indicates the given default cost
     * valud is invalid.
     *
     * @param cost  Value for default cost.
     * @return  An {@link RpcException}.
     */
    public static RpcException getInvalidDefaultCostException(Long cost) {
        String msg = MiscUtils.joinColon("Invalid default cost", cost);
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Return a new {@link RpcException} that indicates the given link cost
     * valud is invalid.
     *
     * @param cost  Link cost.
     * @return  An {@link RpcException}.
     */
    public static RpcException getInvalidCostException(Long cost) {
        String msg = MiscUtils.joinColon("Invalid cost value", cost);
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Return a new {@link RpcException} that indicates the path policy ID
     * is missing.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullPolicyIdException() {
        return RpcException.getNullArgumentException("Path policy ID");
    }

    /**
     * Return a new {@link RpcException} that indicates the path cost
     * configuration is null.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNullPathCostException() {
        return RpcException.getNullArgumentException("Path cost");
    }

    /**
     * Return a new {@link RpcException} that indicates duplicate port
     * descriptor is detected.
     *
     * @param loc  An object that represents the switch port location.
     * @return  An {@link RpcException}.
     */
    public static RpcException getDuplicatePortException(Object loc) {
        String msg = "Duplicate port descriptor: " + loc;
        return RpcException.getBadArgumentException(msg);
    }

    /**
     * Return a new {@link RpcException} that indicates no switch port is
     * specified.
     *
     * @return  An {@link RpcException}.
     */
    public static RpcException getNoSwitchPortException() {
        return RpcException.getMissingArgumentException(
            "At least one switch port must be specified.");
    }

    /**
     * Create the instance identifier for the specified path policy.
     *
     * @param id  The identifier of the path policy.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnPathPolicy> getIdentifier(Integer id) {
        VtnPathPolicyKey key = new VtnPathPolicyKey(id);
        return InstanceIdentifier.builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, key).build();
    }

    /**
     * Create the instance identifier for the given {@link VtnPathPolicyConfig}
     * instance.
     *
     * @param vpp  A {@link VtnPathPolicyConfig} instance.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnPathPolicy> getIdentifier(
        VtnPathPolicyConfig vpp) {
        return getIdentifier(vpp.getId());
    }

    /**
     * Create the instance identifier for the specified path cost
     * configuration.
     *
     * @param id     The identifier of the path policy.
     * @param vdesc  A {@link VtnPortDesc} instance.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnPathCost> getIdentifier(
        Integer id, VtnPortDesc vdesc) {
        VtnPathPolicyKey key = new VtnPathPolicyKey(id);
        VtnPathCostKey ckey = new VtnPathCostKey(vdesc);
        return InstanceIdentifier.builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, key).
            child(VtnPathCost.class, ckey).build();
    }

    /**
     * Create the instance identifier for the specified path cost information.
     *
     * @param id   The identifier of the path policy.
     * @param vpc  A {@link VtnPathCostConfig} instance.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<VtnPathCost> getIdentifier(
        int id, VtnPathCostConfig vpc) {
        return getIdentifier(Integer.valueOf(id), vpc.getPortDesc());
    }

    /**
     * Create a {@link PathCost} instance which describes the given
     * {@link VtnPathCost} instance.
     *
     * @param vpc  A {@link VtnPathCost} instance.
     * @return  A {@link PathCost} instance or {@code null}.
     */
    public static PathCost toPathCost(VtnPathCost vpc) {
        Long c = vpc.getCost();
        if (c == null) {
            c = DEFAULT_LINK_COST;
        }

        PortLocation ploc = NodeUtils.toPortLocation(vpc.getPortDesc());
        if (ploc == null) {
            return null;
        }

        return new PathCost(ploc, c.longValue());
    }

    /**
     * Create a {@link PathPolicy} instance which describes the given
     * {@link VtnPathPolicy} instance.
     *
     * @param vpp  A {@link VtnPathPolicy} instance.
     * @return  A {@link PathPolicy} instance.
     */
    public static PathPolicy toPathPolicy(VtnPathPolicy vpp) {
        Long c = vpp.getDefaultCost();
        long defc = (c == null) ? COST_UNDEF : c.longValue();
        List<PathCost> costs = new ArrayList<>();
        List<VtnPathCost> vlist = vpp.getVtnPathCost();
        if (vlist != null) {
            for (VtnPathCost vpc: vlist) {
                PathCost pc = toPathCost(vpc);
                if (pc != null) {
                    costs.add(pc);
                }
            }
        }

        return new PathPolicy(vpp.getId(), defc, costs);
    }

    /**
     * Read all the path policies from the MD-SAL datastore.
     *
     * @param rtx  A {@link ReadTransaction} instance.
     * @return  A list of {@link VtnPathPolicy} instances.
     * @throws VTNException  An error occurred.
     */
    public static List<VtnPathPolicy> readVtnPathPolicies(ReadTransaction rtx)
        throws VTNException {
        InstanceIdentifier<VtnPathPolicies> path =
            InstanceIdentifier.create(VtnPathPolicies.class);
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        VtnPathPolicies policies = DataStoreUtils.read(rtx, store, path).
            orNull();
        List<VtnPathPolicy> vlist = null;
        if (policies != null) {
            vlist = policies.getVtnPathPolicy();
        }
        if (vlist == null) {
            vlist = Collections.<VtnPathPolicy>emptyList();
        }

        return vlist;
    }

    /**
     * Read path policy configuration specified by the given ID.
     *
     * @param rtx  A {@link ReadTransaction} instance.
     * @param id   The path policy identifier.
     * @return  A {@link VtnPathPolicy} instance.
     * @throws VTNException  An error occurred.
     */
    public static VtnPathPolicy readVtnPathPolicy(ReadTransaction rtx, int id)
        throws VTNException {
        InstanceIdentifier<VtnPathPolicy> path =
            getIdentifier(Integer.valueOf(id));
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        VtnPathPolicy vpp = DataStoreUtils.read(rtx, store, path).orNull();
        if (vpp == null) {
            throw getNotFoundException(id);
        }

        return vpp;
    }

    /**
     * Read the cost information associated with the given port location
     * in the given path policy.
     *
     * @param rtx    A {@link ReadTransaction} instance.
     * @param id     The path policy identifier.
     * @param vdesc  A {@link VtnPortDesc} instance if found.
     *               {@code null} if not found.
     * @return  A {@link VtnPathCost} instance.
     * @throws VTNException  An error occurred.
     */
    public static VtnPathCost readVtnPathCost(ReadTransaction rtx, int id,
                                              VtnPortDesc vdesc)
        throws VTNException {
        VtnPathCost vpc = null;
        if (vdesc != null) {
            InstanceIdentifier<VtnPathCost> path =
                getIdentifier(Integer.valueOf(id), vdesc);
            LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
            vpc = DataStoreUtils.read(rtx, store, path).orNull();
        }
        if (vpc == null) {
            // Check to see if the path policy is present.
            readVtnPathPolicy(rtx, id);
        }

        return vpc;
    }

    /**
     * Create an RPC input builder used to update existing path policy
     * configuration.
     *
     * @param id  The path policy ID.
     * @return  Builder instance for RPC input.
     * @throws RpcException
     *    The given path policy ID is invalid.
     */
    public static PathPolicyConfigBuilder.Rpc createRpcInput(int id)
        throws RpcException {
        PathPolicyConfigBuilder.Rpc builder =
            new PathPolicyConfigBuilder.Rpc();
        Integer pid = Integer.valueOf(id);

        try {
            builder.setId(pid);
        } catch (VTNException e) {
            // Invalid path policy ID is specified.
            throw getNotFoundException(id, e);
        }

        builder.getBuilder().setOperation(VtnUpdateOperationType.ADD).
            setPresent(Boolean.TRUE);

        return builder;
    }
}
