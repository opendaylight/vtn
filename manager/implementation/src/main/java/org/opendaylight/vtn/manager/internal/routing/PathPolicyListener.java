/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentSkipListSet;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPoliciesBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyKey;

/**
 * A data change listener that listens change of path policy configuration.
 */
final class PathPolicyListener
    extends DataStoreListener<VtnPathPolicy, PathPolicyChange> {
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
     * A set of path policy IDs laoded by {@link PathPolicyLoadTask}.
     */
    private Set<Integer>  loadedPolicies;

    /**
     * MD-SAL transaction task to load path policy configuration.
     *
     * <p>
     *   This task returns current {@link VtnPathPolicies} instance.
     * </p>
     */
    private class PathPolicyLoadTask extends AbstractTxTask<VtnPathPolicies> {
        /**
         * Resume the configuration for the given path policy.
         *
         * @param vlist   A list of {@link VtnPathPolicy} instance to store
         *                resumed configuration.
         * @param loaded  A set of loaded path policy IDs.
         * @param key     A string representation of the policy ID.
         * @param pp      A {@link PathPolicy} instance.
         */
        private void resume(List<VtnPathPolicy> vlist, Set<Integer> loaded,
                            String key, PathPolicy pp) {
            Integer pid = pp.getPolicyId();
            try {
                if (!key.equals(String.valueOf(pid))) {
                    String msg = new StringBuilder("Unexpected ID: ").
                        append(pid).append(": expected=").append(key).
                        toString();
                    throw new IllegalArgumentException(msg);
                }
                VtnPathPolicy vpp = new PathPolicyConfigBuilder.Data().
                    set(pp).getBuilder().build();
                vlist.add(vpp);
                loaded.add(pid);
            } catch (RpcException | RuntimeException e) {
                String msg = MiscUtils.joinColon(
                    "Ignore invalid path policy configuration",
                    pp, e.getMessage());
                LOG.warn(msg, e);
            }
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathPolicies execute(TxContext ctx) throws VTNException {
            loadedPolicies = null;
            Set<Integer> loaded = new ConcurrentSkipListSet<>();

            // Load configuration from file.
            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHPOLICY;
            List<VtnPathPolicy> vlist = new ArrayList<>();
            for (String key: XmlConfigFile.getKeys(ftype)) {
                PathPolicy pp = XmlConfigFile.load(
                    ftype, key, PathPolicy.class);
                if (pp != null) {
                    resume(vlist, loaded, key, pp);
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
            if (!loaded.isEmpty()) {
                loadedPolicies = loaded;
            }

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
                    LOG.info("{}: Path policy has been loaded.", id);
                    topology.updateResolver(id);
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
    private static class PathPolicySaveTask
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
                    PathPolicy pp = PathPolicyUtils.toPathPolicy(vpp);
                    String key = pp.getPolicyId().toString();
                    XmlConfigFile.save(ftype, key, pp);
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
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
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
            ? new PathPolicyLoadTask() : new PathPolicySaveTask();
        return provider.post(task);
    }

    /**
     * Return the path policy identifier in the given instance identifier.
     *
     * @param path  Path to the path policy configuration.
     * @return  An {@link Integer} instance which represents the path policy
     *          identifier. {@code null} on failure.
     */
    private Integer getIdentifier(InstanceIdentifier<VtnPathPolicy> path) {
        VtnPathPolicyKey key =
            path.firstKeyOf(VtnPathPolicy.class, VtnPathPolicyKey.class);
        if (key == null) {
            return null;
        }

        return key.getId();
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected PathPolicyChange enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
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
    protected void onCreated(PathPolicyChange ectx,
                             IdentifiedData<VtnPathPolicy> data) {
        InstanceIdentifier<VtnPathPolicy> path = data.getIdentifier();
        Integer id = getIdentifier(path);
        if (id == null) {
            LOG.warn("Ignore broken creation event: path={}, value={}",
                     path, data.getValue());
        } else {
            // Do nothing if the specified event was caused by the initial
            // setup.
            Set<Integer> loaded = loadedPolicies;
            if (loaded == null || !loaded.remove(id)) {
                VtnPathPolicy vpp = data.getValue();
                ectx.addUpdated(id, PathPolicyUtils.toPathPolicy(vpp));
            } else if (loaded.isEmpty()) {
                LOG.debug("All loaded path policies have been notified.");
                loadedPolicies = null;
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(PathPolicyChange ectx,
                             ChangedData<VtnPathPolicy> data) {
        InstanceIdentifier<VtnPathPolicy> path = data.getIdentifier();
        VtnPathPolicy vpp = data.getValue();
        Integer id = getIdentifier(path);
        if (id == null) {
            LOG.warn("Ignore broken update event: path={}, old={}, new={}",
                     path, vpp, data.getOldValue());
        } else {
            ectx.addUpdated(id, PathPolicyUtils.toPathPolicy(vpp));
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(PathPolicyChange ectx,
                             IdentifiedData<VtnPathPolicy> data) {
        InstanceIdentifier<VtnPathPolicy> path = data.getIdentifier();
        Integer id = getIdentifier(path);
        if (id == null) {
            LOG.warn("Ignore broken removal event: path={}, value={}",
                     path, data.getValue());
        } else {
            ectx.addRemoved(id);
        }
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
