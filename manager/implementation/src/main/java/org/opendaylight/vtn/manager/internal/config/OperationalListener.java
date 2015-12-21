/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.DataStoreListener;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;

/**
 * VTN configuration listener for operational view.
 */
public final class OperationalListener
    extends DataStoreListener<VtnConfig, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(OperationalListener.class);

    /**
     * The current configuration.
     */
    private final AtomicReference<VTNConfigImpl>  current;

    /**
     * The future associated with the initialization of vtn-config in
     * operational datastore.
     */
    private SettableVTNFuture<Void>  initFuture = new SettableVTNFuture<>();

    /**
     * Consturct a new instance.
     *
     * @param broker  A {@link DataBroker} service.
     * @param cfg     An {@link AtomicReference} instance to keep current
     *                configuration.
     */
    public OperationalListener(DataBroker broker,
                               AtomicReference<VTNConfigImpl> cfg) {
        super(VtnConfig.class);
        current = cfg;
        registerListener(broker, LogicalDatastoreType.OPERATIONAL,
                         DataChangeScope.SUBTREE, true);
    }

    /**
     * Wait for another contoller in the cluster to complete initialization.
     *
     * @param state   The init-state value obtained from the operational DS.
     * @param millis  The number of milliseconds to wait.
     * @throws VTNException  An error occurred.
     */
    public void awaitConfig(boolean state, long millis) throws VTNException {
        SettableVTNFuture<Void> f = initFuture;

        try {
            if (!state && f != null) {
                LOG.debug("Synchronizing VTN configuration.");
                f.checkedGet(millis, TimeUnit.MILLISECONDS);
                LOG.debug("VTN configuration has been synchronized.");
            }
        } finally {
            initFuture = null;
        }
    }

    /**
     * Update the current configuration.
     *
     * @param data  An {@link IdentifiedData} instance which contains the
     *              configuration to be applied.
     */
    private void updateConfig(IdentifiedData<VtnConfig> data) {
        VtnConfig vcfg = data.getValue();
        VTNConfigImpl cur = current.get();
        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        String diff = VTNConfigImpl.diff(cur, vconf);
        if (diff != null) {
            current.set(vconf);
            LOG.info("Configuration has been changed: {}", diff);
        }

        Boolean state = vcfg.isInitState();
        if (Boolean.TRUE.equals(state)) {
            // Complete the vtn-config initialization.
            SettableVTNFuture<Void> f = initFuture;
            if (f != null && f.set(null)) {
                LOG.trace("Wake up awaitConfig().");
            }
        }
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
        updateConfig(data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx, ChangedData<VtnConfig> data) {
        updateConfig(data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx, IdentifiedData<VtnConfig> data) {
        LOG.warn("Global configuration has been removed unexpectedly.");
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
