/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.commons.collections15.Transformer;

import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.LinkEdge;
import org.opendaylight.vtn.manager.internal.util.NodeUtils;
import org.opendaylight.vtn.manager.internal.util.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * An implementation of {@link Transformer} that transforms link edge into
 * the cost of the link.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
final class PathPolicyTransformer implements Transformer<LinkEdge, Long> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PathPolicyTransformer.class);

    /**
     * A data broker service.
     */
    private final DataBroker  dataBroker;

    /**
     * The index of the path policy.
     */
    private final int  policyId;

    /**
     * VTN inventory reader.
     */
    private InventoryReader  reader;

    /**
     * Construct a new instance.
     *
     * @param broker  A {@link DataBroker} service.
     * @param id      The index of the path policy associated with this
     *                instance.
     */
    PathPolicyTransformer(DataBroker broker, int id) {
        dataBroker = broker;
        policyId = id;
    }

    /**
     * Set inventory reader that contains active transaction for the MD-SAL
     * datastore.
     *
     * @param rdr  An {@link InventoryReader} instance.
     * @return  An {@link InventoryReader} currently configured is returned.
     */
    InventoryReader setReader(InventoryReader rdr) {
        InventoryReader old = reader;
        reader = rdr;
        return old;
    }

    /**
     * Return the path cost associated with the given switch port.
     *
     * @param rdr    A {@link InventoryReader} instance.
     * @param sport  A {@link SalPort} instance.
     * @return  A {@link Long} instance which represents the cost.
     *          {@code null} is returned if the default cost should be used.
     * @throws VTNException  An error occurred.
     */
    private Long getCost(InventoryReader rdr, SalPort sport)
        throws VTNException {
        VtnPort vport = rdr.get(sport);
        if (vport == null) {
            // This should never happen.
            LOG.warn("{}: Unknown port: {}", policyId, sport);
            return Long.valueOf(Long.MAX_VALUE);
        }

        // Read path policy configuration.
        VtnPathPolicyKey key = new VtnPathPolicyKey(Integer.valueOf(policyId));
        InstanceIdentifier<VtnPathPolicy> path = InstanceIdentifier.
            builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class, key).build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        ReadTransaction rtx = rdr.getReadTransaction();
        VtnPathPolicy vpp = DataStoreUtils.read(rtx, oper, path).orNull();
        if (vpp == null) {
            LOG.debug("{}: Path policy not found: port={}", policyId, sport);
            return null;
        }

        List<VtnPathCost> vcosts = vpp.getVtnPathCost();
        if (vcosts != null && !vcosts.isEmpty()) {
            VtnPortDesc[] descriptors =
                NodeUtils.createPortDescArray(sport, vport);
            for (VtnPortDesc vpdesc: descriptors) {
                VtnPathCostKey ckey = new VtnPathCostKey(vpdesc);
                InstanceIdentifier<VtnPathCost> cpath = InstanceIdentifier.
                    builder(VtnPathPolicies.class).
                    child(VtnPathPolicy.class, key).
                    child(VtnPathCost.class, ckey).build();
                VtnPathCost vpc = DataStoreUtils.read(rtx, oper, cpath).
                    orNull();
                if (vpc != null) {
                    Long cost = vpc.getCost();
                    LOG.trace("{}: Path cost was found: port={}, desc={}, " +
                              "cost={}", policyId, sport, vpdesc.getValue(),
                              cost);
                    return cost;
                }
            }
        }

        // Use default cost.
        Long cost = vpp.getDefaultCost();
        if (cost == null || cost.longValue() == PathPolicy.COST_UNDEF) {
            cost = vport.getCost();
            LOG.trace("{}: Use link cost in VTN port: port={}, cost={}",
                      policyId, sport, cost);
        } else {
            LOG.trace("{}: Use default cost: port={}, cost={}", policyId,
                      sport, cost);
        }

        return cost;
    }

    // Transformer

    /**
     * Return the cost associated with the given link edge.
     *
     * @param le  A {@link LinkEdge} instance.
     * @return  A {@link Long} instance which represents the cost.
     */
    @Override
    public Long transform(LinkEdge le) {
        if (le == null) {
            LOG.warn("{}: Link edge is null.", policyId);
            return Long.valueOf(Long.MAX_VALUE);
        }

        SalPort sport = le.getSourcePort();
        if (sport == null) {
            LOG.warn("{}: Switch port is null: {}", policyId, le);
            return Long.valueOf(Long.MAX_VALUE);
        }

        // Prepare MD-SAL datastore transaction.
        InventoryReader rdr = reader;
        ReadOnlyTransaction localTx;
        if (rdr == null) {
            localTx = dataBroker.newReadOnlyTransaction();
            rdr = new InventoryReader(localTx);
        } else {
            localTx = null;
        }

        try {
            Long c = getCost(rdr, sport);
            return (c == null) ? PathPolicyUtils.DEFAULT_LINK_COST : c;
        } catch (Exception e) {
            StringBuilder builder = new StringBuilder();
            builder.append(policyId).
                append(": Failed to determine path cost for ").
                append(le).append('.');
            LOG.warn(builder.toString(), e);
            return Long.valueOf(Long.MAX_VALUE);
        } finally {
            if (localTx != null) {
                localTx.close();
            }
        }
    }
}
