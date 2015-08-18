/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcOutputGenerator;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutput;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.ClearPathPolicyOutputBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code ClearPathPolicyTask} describes the MD-SAL datastore transaction task
 * that clears the path policy container.
 */
public final class ClearPathPolicyTask extends AbstractTxTask<VtnUpdateType>
    implements RpcOutputGenerator<VtnUpdateType, ClearPathPolicyOutput> {
    /**
     * A set of removed path policy identifiers.
     */
    private final Set<Integer>  removedPolicies = new HashSet<>();

    // AbstractTxTask

    /**
     * Clear the root container of the path policy.
     *
     * @param ctx  A runtime context for transaction task.
     * @return  {@link VtnUpdateType#REMOVED} if at least one path policy has
     *          been removed. {@code null} is returned if no path policy is
     *          present.
     * @throws VTNException  An error occurred.
     */
    @Override
    protected VtnUpdateType execute(TxContext ctx) throws VTNException {
        // Collect path policy identifiers.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnPathPolicies> path =
            InstanceIdentifier.create(VtnPathPolicies.class);
        ReadWriteTransaction tx = ctx.getReadWriteTransaction();
        Optional<VtnPathPolicies> opt = DataStoreUtils.read(tx, oper, path);
        if (opt.isPresent()) {
            List<VtnPathPolicy> vlist = opt.get().getVtnPathPolicy();
            if (vlist != null) {
                for (VtnPathPolicy vpp: vlist) {
                    removedPolicies.add(vpp.getId());
                }
            }
        }

        // Put an empty root container.
        VtnPathPolicies root = new VtnPathPoliciesBuilder().build();
        tx.put(oper, path, root, true);

        return (removedPolicies.isEmpty()) ? null : VtnUpdateType.REMOVED;
    }

    // TxTask

    /**
     * {@inheritDoc}
     */
    @Override
    public void onSuccess(VTNManagerProvider provider, VtnUpdateType result) {
        // Remove all flow entries affected by removed path policies.
        if (result != null) {
            PathPolicyFlowRemover remover =
                new PathPolicyFlowRemover(removedPolicies);
            addBackgroundTask(provider.removeFlows(remover));
        }
    }

    // RpcOutputGenerator

    /**
     * {@inheritDoc}
     */
    @Override
    public Class<ClearPathPolicyOutput> getOutputType() {
        return ClearPathPolicyOutput.class;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public ClearPathPolicyOutput createOutput(VtnUpdateType result) {
        return new ClearPathPolicyOutputBuilder().setStatus(result).build();
    }
}
