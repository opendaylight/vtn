/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import javax.annotation.Nonnull;

import org.opendaylight.vtn.manager.internal.util.DataStoreListener;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;

/**
 * The base class for data tree change listeners that listens vtn-config.
 */
public abstract class VtnConfigListener
    extends DataStoreListener<VtnConfig, Void> {
    /**
     * Consturct a new instance.
     */
    protected VtnConfigListener() {
        super(VtnConfig.class);
    }

    // AbstractDataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected final Void enterEvent() {
        return null;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final void exitEvent(Void ectx) {
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final InstanceIdentifier<VtnConfig> getWildcardPath() {
        return VTNConfigManager.CONFIG_IDENT;
    }

    // DataStoreListener

    /**
     * {@inheritDoc}
     */
    @Override
    protected final boolean isUpdated(@Nonnull VtnConfig before,
                                      @Nonnull VtnConfig after) {
        return !before.equals(after);
    }
}
