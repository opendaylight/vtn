/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.tx.AbstractTxTask;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;

/**
 * VTN configuration listener for configuration view.
 */
public final class ConfigListener extends DataStoreListener<VtnConfig, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(ConfigListener.class);

    /**
     * The transaction submit queue for the VTN configuration data models.
     */
    private final TxQueue  txQueue;

    /**
     * MAC address of the local node.
     */
    private final EtherAddress localMacAddress;

    /**
     * MD-SAL transaction task to be invoked when a configuration has been
     * created or updated.
     */
    private static final class ConfigUpdateTask extends AbstractTxTask<Void> {
        /**
         * A {@link VtnConfig} instance to be applied.
         */
        private final VtnConfig  vtnConfig;

        /**
         * MAC address of the local node.
         */
        private final EtherAddress  macAddress;

        /**
         * Construct a new task.
         *
         * @param vcfg  A {@link VtnConfig} instance.
         * @param mac   MAC address of the local node.
         */
        private ConfigUpdateTask(VtnConfig vcfg, EtherAddress mac) {
            vtnConfig = vcfg;
            macAddress = mac;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) {
            if (ctx.getProvider().isOwner(VTNEntityType.CONFIG)) {
                // Apply new configuration to operational view.
                LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                VtnConfig vcfg = VTNConfigImpl.builder(vtnConfig, macAddress).
                    build();
                LOG.trace("Updating vtn-config in operational DS: {}", vcfg);
                tx.merge(oper, VTNConfigManager.CONFIG_IDENT, vcfg,
                         true);
            }

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Void result) {
            // Save current configuration into file.
            VTNConfigImpl vconf = new VTNConfigImpl(vtnConfig);
            XmlConfigFile.save(XmlConfigFile.Type.CONFIG,
                               VTNConfigManager.KEY_VTN_CONFIG, vconf);
        }
    }

    /**
     * MD-SAL transaction task to be invoked when a configuration has been
     * removed.
     */
    private static final class ConfigRemoveTask extends AbstractTxTask<Void> {
        /**
         * MAC address of the local node.
         */
        private final EtherAddress  macAddress;

        /**
         * Construct a new instance.
         *
         * @param mac   MAC address of the local node.
         */
        private ConfigRemoveTask(EtherAddress mac) {
            macAddress = mac;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public Void execute(TxContext ctx) throws VTNException {
            if (ctx.getProvider().isOwner(VTNEntityType.CONFIG)) {
                // Reset operational view to default.
                VtnConfig vcfg = VTNConfigImpl.builder(macAddress).build();
                ReadWriteTransaction tx = ctx.getReadWriteTransaction();
                LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
                LOG.trace("Resetting vtn-config in operational DS: {}", vcfg);
                tx.merge(oper, VTNConfigManager.CONFIG_IDENT, vcfg,
                         true);
            }

            return null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        public void onSuccess(VTNManagerProvider provider, Void result) {
            // Delete configuration file.
            XmlConfigFile.delete(XmlConfigFile.Type.CONFIG,
                                 VTNConfigManager.KEY_VTN_CONFIG);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param queue   A {@link TxQueue} instance used to update the
     *                VTN inventory.
     * @param broker  A {@link DataBroker} service instance.
     * @param mac     MAC address of the local node.
     */
    public ConfigListener(TxQueue queue, DataBroker broker, EtherAddress mac) {
        super(VtnConfig.class);
        txQueue = queue;
        localMacAddress = mac;
        registerListener(broker, LogicalDatastoreType.CONFIGURATION,
                         DataChangeScope.SUBTREE, true);
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected Void enterEvent(
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void exitEvent(Void ectx) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onCreated(Void ectx, IdentifiedData<VtnConfig> data) {
        VtnConfig vcfg = data.getValue();
        ConfigUpdateTask task = new ConfigUpdateTask(vcfg, localMacAddress);
        txQueue.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx, ChangedData<VtnConfig> data) {
        onCreated(ectx, data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx, IdentifiedData<VtnConfig> data) {
        ConfigRemoveTask task = new ConfigRemoveTask(localMacAddress);
        txQueue.post(task);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnConfig> getWildcardPath() {
        return VTNConfigManager.CONFIG_IDENT;
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
