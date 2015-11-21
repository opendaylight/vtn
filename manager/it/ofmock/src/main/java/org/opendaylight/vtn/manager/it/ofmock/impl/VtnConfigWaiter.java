/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.google.common.base.Optional;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * {@code VtnConfigWaiter} observes the vtn-config container, and wait for
 * the configuration to be initialized.
 */
public final class VtnConfigWaiter extends DataStoreListener<VtnConfig, Void> {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VtnConfigWaiter.class);

    /**
     * Instance identifier of the VTN global configuration.
     */
    static final InstanceIdentifier<VtnConfig> CONFIG_IDENT =
        InstanceIdentifier.create(VtnConfig.class);

    /**
     * Set {@code true} when the VTN configuration has been initialized.
     */
    private boolean  initialized;

    /**
     * Construct a new instance.
     *
     * @param ofmock  ofmock provider service.
     */
    VtnConfigWaiter(OfMockProvider ofmock) {
        super(VtnConfig.class);

        DataBroker broker = ofmock.getDataBroker();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        registerListener(broker, oper, DataChangeScope.SUBTREE);

        // Read the current value.
        try (ReadOnlyTransaction rtx = broker.newReadOnlyTransaction()) {
            Optional<VtnConfig> opt =
                DataStoreUtils.read(rtx, oper, CONFIG_IDENT);
            if (opt.isPresent()) {
                update(opt.get());
            }
        }
    }

    /**
     * Wait for the VTN configuration to be initialized.
     *
     * @param timeout  The number of milliseconds to wait.
     * @return  {@code true} if the VTN configuration has been initialized.
     *          {@code false} otherwise.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     */
    public synchronized boolean await(long timeout)
        throws InterruptedException {
        boolean result = initialized;
        if (!result) {
            long to = timeout;
            long limit = System.currentTimeMillis() + to;

            do {
                wait(to);
                result = initialized;
                if (result) {
                    break;
                }

                to = limit - System.currentTimeMillis();
            } while (to > 0);
        }

        return result;
    }

    /**
     * Update the waiter status.
     *
     * @param vcfg  A {@link VtnConfig} instance.
     */
    private synchronized void update(VtnConfig vcfg) {
        if (Boolean.TRUE.equals(vcfg.isInitState())) {
            initialized = true;
            notifyAll();
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
    protected void onCreated(Void ectx, InstanceIdentifier<VtnConfig> path,
                             VtnConfig data) {
        update(data);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onUpdated(Void ectx, InstanceIdentifier<VtnConfig> path,
                             VtnConfig oldData, VtnConfig newData) {
        update(newData);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected void onRemoved(Void ectx, InstanceIdentifier<VtnConfig> path,
                             VtnConfig data) {
        // This should never happen.
        LOG.error("VTN configuration has been removed unexpectedly: {}", data);
        throw new IllegalStateException();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected InstanceIdentifier<VtnConfig> getWildcardPath() {
        return CONFIG_IDENT;
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
