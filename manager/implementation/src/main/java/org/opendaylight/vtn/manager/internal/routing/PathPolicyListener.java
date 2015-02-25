/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.DataStoreUtils;
import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyConfigBuilder;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;
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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

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
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

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
    private class PathPolicyLoadTask extends AbstractTxTask<VtnPathPolicies> {
        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathPolicies execute(TxContext ctx) throws VTNException {
            // Load configuration from file.
            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHPOLICY;
            List<VtnPathPolicy> vlist = new ArrayList<>();
            for (String key: XmlConfigFile.getKeys(ftype)) {
                PathPolicy pp = XmlConfigFile.load(
                    XmlConfigFile.Type.PATHPOLICY, key, PathPolicy.class);
                if (pp != null) {
                    try {
                        VtnPathPolicy vpp = new PathPolicyConfigBuilder.Data().
                            set(pp).getBuilder().build();
                        vlist.add(vpp);
                    } catch (VTNException e) {
                        String msg = MiscUtils.joinColon(
                            "Ignore invalid path policy configuration",
                            pp, e.getMessage());
                        LOG.warn(msg);
                    }
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
                    LOG.info("Path policy was loaded: {}", id);
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
         * A list of {@link PathPolicy} instances to be saved.
         */
        private List<PathPolicy>  saveConfig = new ArrayList<PathPolicy>();

        /**
         * Set {@code true} if the root container has been created.
         */
        private boolean  created;

        /**
         * {@inheritDoc}
         */
        @Override
        public VtnPathPolicies execute(TxContext ctx) throws VTNException {
            saveConfig.clear();
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
            } else {
                List<VtnPathPolicy> vlist = policies.getVtnPathPolicy();
                if (vlist != null) {
                    for (VtnPathPolicy vpp: vlist) {
                        PathPolicy pp = PathPolicyUtils.toPathPolicy(vpp);
                        saveConfig.add(pp);
                    }
                }
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
                    "An empty path policy configuration has been created.");
            }

            Set<String> names = new HashSet<>();
            XmlConfigFile.Type ftype = XmlConfigFile.Type.PATHPOLICY;
            for (PathPolicy pp: saveConfig) {
                // Save configuration into a file.
                String key = pp.getPolicyId().toString();
                XmlConfigFile.save(ftype, key, pp);
                names.add(key);
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
        vtnProvider = provider;
        topology = topo;
        registerListener(provider.getDataBroker(),
                         LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE);
    }

    /**
     * Post a MD-SAL transaction task to initialize configuration.
     *
     * @param master  {@code true} if the local node is the configuration
     *                provider.
     * @return  A {@link VTNFuture} instance.
     */
    VTNFuture<?> initConfig(boolean master) {
        TxTask<?> task = (master)
            ? new PathPolicyLoadTask() : new PathPolicySaveTask();
        return vtnProvider.post(task);
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
        return new PathPolicyChange();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(PathPolicyChange ectx) {
        ectx.apply(vtnProvider, topology, LOG);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(PathPolicyChange ectx,
                             InstanceIdentifier<VtnPathPolicy> key,
                             VtnPathPolicy value) {
        Integer id = getIdentifier(key);
        if (id == null) {
            LOG.warn("Ignore broken creation event: path={}, value={}",
                     key, value);
        } else {
            ectx.addUpdated(id, value);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(PathPolicyChange ectx,
                             InstanceIdentifier<VtnPathPolicy> key,
                             VtnPathPolicy oldValue, VtnPathPolicy newValue) {
        Integer id = getIdentifier(key);
        if (id == null) {
            LOG.warn("Ignore broken update event: path={}, old={}, new={}",
                     key, oldValue, newValue);
        } else {
            ectx.addUpdated(id, newValue);
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(PathPolicyChange ectx,
                             InstanceIdentifier<VtnPathPolicy> key,
                             VtnPathPolicy value) {
        Integer id = getIdentifier(key);
        if (id == null) {
            LOG.warn("Ignore broken removal event: path={}, value={}",
                     key, value);
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

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected Set<VtnUpdateType> getRequiredEvents() {
        return null;
    }
}
