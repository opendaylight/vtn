/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import javax.annotation.Nonnull;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathPolicy;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.MultiDataStoreListener;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * A data change listener that listens change of path policy configuration.
 */
final class PathPolicyListener
    extends MultiDataStoreListener<VtnPathPolicy, PathPolicyChange> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(PathPolicyListener.class);

    /**
     * A graph that keeps network topology.
     */
    private final TopologyGraph  topology;

    /**
     * MD-SAL transaction task to load path policy configuration.
     *
     * <p>
     *   This task returns current {@link VtnPathPolicies} instance.
     * </p>
     */
    private static final class PathPolicyLoadTask
        extends AbstractTxTask<VtnPathPolicies> {
        /**
         * A graph that keeps network topology.
         */
        private final TopologyGraph  topoGraph;

        /**
         * Construct a new instance.
         *
         * @param topo  A {@link TopologyGraph} instance.
         */
        private PathPolicyLoadTask(TopologyGraph topo) {
            topoGraph = topo;
        }

        /**
         * Resume the configuration for the given path policy.
         *
         * @param ctx     MD-SAL datastore transaction context.
         * @param vlist   A list of {@link VtnPathPolicy} instance to store
         *                resumed configuration.
         * @param key     A string representation of the policy ID.
         * @param xpp     A {@link XmlPathPolicy} instance.
         */
        private void resume(TxContext ctx, List<VtnPathPolicy> vlist,
                            String key, XmlPathPolicy xpp) {
            Integer pid = xpp.getId();
            try {
                if (!key.equals(String.valueOf(pid))) {
                    String msg = new StringBuilder("Unexpected ID: ").
                        append(pid).append(": expected=").append(key).
                        toString();
                    throw new IllegalArgumentException(msg);
                }
                VtnPathPolicy vpp = PathPolicyUtils.newBuilder(xpp).build();
                vlist.add(vpp);
            } catch (RpcException | RuntimeException e) {
                ctx.log(LOG, VTNLogLevel.WARN, e,
                        "Ignore invalid path policy configuration: %s", e);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathPolicies execute(TxContext ctx) throws VTNException {
            // Load configuration from file.
            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHPOLICY;
            List<VtnPathPolicy> vlist = new ArrayList<>();
            for (String key: XmlConfigFile.getKeys(ftype)) {
                XmlPathPolicy xpp = XmlConfigFile.load(
                    ftype, key, XmlPathPolicy.class);
                if (xpp != null) {
                    resume(ctx, vlist, key, xpp);
                }
            }

            VtnPathPoliciesBuilder builder = new VtnPathPoliciesBuilder();
            if (!vlist.isEmpty()) {
                builder.setVtnPathPolicy(vlist);
            }
            InstanceIdentifier<VtnPathPolicies> path =
                InstanceIdentifier.create(VtnPathPolicies.class);

            // Remove old configuration, and install loaded configuration.
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            DataStoreUtils.delete(tx, oper, path);

            VtnPathPolicies policies = builder.build();
            tx.put(oper, path, policies, true);

            return policies;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              VtnPathPolicies result) {
            List<VtnPathPolicy> vlist = result.getVtnPathPolicy();
            if (vlist != null) {
                // Create route resolvers for path policies.
                for (VtnPathPolicy vpp: vlist) {
                    Integer id = vpp.getId();
                    LOG.debug("{}: Path policy has been loaded.", id);
                    topoGraph.updateResolver(id);
                }
            }
        }
    }

    /**
     * MD-SAL transaction task to save current path policy configuration.
     *
     * <p>
     *   This task returns current {@link VtnPathPolicies} instance.
     * </p>
     */
    private static final class PathPolicySaveTask
        extends AbstractTxTask<VtnPathPolicies> {
        /**
         * Set {@code true} if the root container has been created.
         */
        private boolean  created;

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathPolicies execute(TxContext ctx) throws VTNException {
            created = false;

            // Load current configuration.
            InstanceIdentifier<VtnPathPolicies> path =
                InstanceIdentifier.create(VtnPathPolicies.class);
            LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
            ReadWriteTransaction tx = ctx.getReadWriteTransaction();
            VtnPathPolicies policies =
                DataStoreUtils.read(tx, oper, path).orNull();
            if (policies == null) {
                // Initialize the path policy container.
                VtnPathPoliciesBuilder builder = new VtnPathPoliciesBuilder();
                policies = builder.build();
                tx.put(oper, path, builder.build(), true);
                created = true;
            }

            return policies;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider,
                              VtnPathPolicies result) {
            if (created) {
                LOG.info(
                    "An empty path policy container has been created.");
            }

            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHPOLICY;
            Set<String> names = new HashSet<>();
            List<VtnPathPolicy> vlist = result.getVtnPathPolicy();
            if (vlist != null) {
                for (VtnPathPolicy vpp: vlist) {
                    // Save configuration into a file.
                    XmlPathPolicy xpp = new XmlPathPolicy(vpp);
                    String key = xpp.getId().toString();
                    XmlConfigFile.save(ftype, key, xpp);
                    names.add(key);
                }
            }

            // Remove obsolete configuration files.
            XmlConfigFile.deleteAll(ftype, names);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param provider  VTN Manager provider service.
     * @param topo      A {@link TopologyGraph} instance.
     */
    PathPolicyListener(VTNManagerProvider provider, TopologyGraph topo) {
        super(VtnPathPolicy.class);
        topology = topo;
        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL, true);
    }

    /**
     * Post a MD-SAL transaction task to initialize configuration.
     *
     * @param provider  VTN Manager provider service.
     * @param master    {@code true} if the local node is the configuration
     *                  provider.
     * @return  A {@link VTNFuture} instance.
     */
    VTNFuture<?> initConfig(VTNManagerProvider provider, boolean master) {
        TxTask<?> task = (master)
            ? new PathPolicyLoadTask(topology) : new PathPolicySaveTask();
        return provider.post(task);
    }

    /**
     * Return the path policy identifier in the given instance identifier.
     *
     * @param path  Path to the path policy configuration.
     * @return  An {@link Integer} instance which represents the path policy
     *          identifier. {@code null} on failure.
     */
    private Integer getIdentifier(InstanceIdentifier<?> path) {
        VtnPathPolicyKey key = path.firstKeyOf(VtnPathPolicy.class);
        return (key == null) ? null : key.getId();
    }

    /**
     * Handle creation or removal event for a path cost.
     *
     * @param ectx  A {@link PathPolicyChange} instance which keeps changes to
     *              the configuration.
     * @param data  An {@link IdentifiedData} instance that contains a path
     *              cost.
     * @param type  {@link VtnUpdateType#CREATED} on added,
     *              {@link VtnUpdateType#REMOVED} on removed.
     */
    private void onPathCostChanged(PathPolicyChange ectx,
                                   IdentifiedData<?> data,
                                   VtnUpdateType type) {
        IdentifiedData<VtnPathCost> cdata = data.checkType(VtnPathCost.class);
        if (cdata != null) {
            Integer id = getIdentifier(cdata.getIdentifier());
            VtnPathCost vpc = cdata.getValue();
            LOG.info("{}: Path cost has been {}: desc=\"{}\", cost={}",
                     id, MiscUtils.toLowerCase(type),
                     vpc.getPortDesc().getValue(), vpc.getCost());

            // Mark the path policy as changed.
            ectx.setChanged(id);
        } else {
            // This should never happen.
            data.unexpected(LOG, type);
        }
    }

    /**
     * Handle update event for a path cost.
     *
     * @param ectx  A {@link PathPolicyChange} instance which keeps changes to
     *              the configuration.
     * @param data  A {@link ChangedData} instance that contains a path cost.
     */
    private void onPathCostChanged(PathPolicyChange ectx,
                                   ChangedData<?> data) {
        ChangedData<VtnPathCost> cdata = data.checkType(VtnPathCost.class);
        if (cdata != null) {
            Integer id = getIdentifier(cdata.getIdentifier());
            VtnPathCost vpc = cdata.getValue();
            VtnPathCost old = cdata.getOldValue();
            LOG.info("{}: Path cost has been changed: desc=\"{}\", " +
                     "cost={{} -> {}}", id, vpc.getPortDesc().getValue(),
                     old.getCost(), vpc.getCost());

            // Mark the path policy as changed.
            ectx.setChanged(id);
        } else {
            // This should never happen.
            data.unexpected(LOG, VtnUpdateType.CHANGED);
        }
    }

    // MultiDataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isDepth(VtnUpdateType type) {
        // Creation events should be processed from outer to inner.
        // Other events should be processed from inner to outer.
        return (type == VtnUpdateType.CREATED);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isRequiredType(@Nonnull Class<?> type) {
        return (VtnPathPolicy.class.equals(type) ||
                VtnPathCost.class.equals(type));
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean isUpdated(PathPolicyChange ectx, ChangedData<?> data) {
        boolean changed;
        ChangedData<VtnPathPolicy> pdata = data.checkType(VtnPathPolicy.class);
        if (pdata != null) {
            // Return true if vtn-path-cost list is updated.
            VtnPathPolicy vpp = pdata.getValue();
            changed = ectx.isChanged(vpp.getId());
            if (!changed) {
                // Check to see if default-cost is changed.
                VtnPathPolicy old = pdata.getOldValue();
                changed = !Objects.equals(
                    old.getDefaultCost(), vpp.getDefaultCost());
            }
        } else {
            ChangedData<VtnPathCost> cdata = data.checkType(VtnPathCost.class);
            if (cdata != null) {
                // Check to see if cost is changed.
                VtnPathCost old = cdata.getOldValue();
                VtnPathCost vpc = cdata.getValue();
                changed = !Objects.equals(old.getCost(), vpc.getCost());
            } else {
                // This should never happen.
                data.unexpected(LOG, VtnUpdateType.CHANGED);
                changed = false;
            }
        }

        return changed;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(PathPolicyChange ectx, IdentifiedData<?> data) {
        IdentifiedData<VtnPathPolicy> pdata =
            data.checkType(VtnPathPolicy.class);
        if (pdata != null) {
            VtnPathPolicy vpp = pdata.getValue();
            Integer id = vpp.getId();
            if (id == null) {
                LOG.warn("Ignore broken creation event: path={}, value={}",
                         pdata.getIdentifier(), vpp);
            } else {
                ectx.addUpdated(id, new XmlPathPolicy(vpp));
                LOG.info("Path policy has been created: id={}, default={}",
                         id, vpp.getDefaultCost());
            }
        } else {
            onPathCostChanged(ectx, data, VtnUpdateType.CREATED);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(PathPolicyChange ectx, ChangedData<?> data) {
        ChangedData<VtnPathPolicy> pdata = data.checkType(VtnPathPolicy.class);
        if (pdata != null) {
            VtnPathPolicy vpp = pdata.getValue();
            VtnPathPolicy old = pdata.getOldValue();
            Integer id = vpp.getId();
            if (id == null) {
                LOG.warn("Ignore broken update event: path={}, old={}, new={}",
                         pdata.getIdentifier(), old, vpp);
            } else {
                ectx.addUpdated(id, new XmlPathPolicy(vpp));
                Long def = vpp.getDefaultCost();
                Long odef = old.getDefaultCost();
                if (!Objects.equals(def, odef)) {
                    LOG.info("Default cost for path policy has been changed:" +
                             " id={}, default={{} -> {}}", id, odef, def);
                }
            }
        } else {
            onPathCostChanged(ectx, data);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(PathPolicyChange ectx, IdentifiedData<?> data) {
        IdentifiedData<VtnPathPolicy> pdata =
            data.checkType(VtnPathPolicy.class);
        if (pdata != null) {
            VtnPathPolicy vpp = pdata.getValue();
            Integer id = vpp.getId();
            if (id == null) {
                LOG.warn("Ignore broken removal event: path={}, value={}",
                         pdata.getIdentifier(), vpp);
            } else {
                ectx.addRemoved(id);
                LOG.info("Path policy has been removed: id={}, default={}",
                         id, vpp.getDefaultCost());
            }
        } else {
            onPathCostChanged(ectx, data, VtnUpdateType.REMOVED);
        }
    }

    // AbstractDataChangeListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected PathPolicyChange enterEvent() {
        return new PathPolicyChange(topology);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(PathPolicyChange ectx) {
        ectx.apply(LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnPathPolicy> getWildcardPath() {
        return InstanceIdentifier.builder(VtnPathPolicies.class).
            child(VtnPathPolicy.class).build();
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }
}
