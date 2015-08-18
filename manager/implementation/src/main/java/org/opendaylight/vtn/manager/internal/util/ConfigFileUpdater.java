/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.HashSet;
import java.util.Set;

/**
 * {@code ConfigFileUpdater} describes changes to be applied to the
 * configuration file.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 *
 * @param <K>  The type of the identifier which identifies data.
 * @param <V>  The type of the data.
 */
public abstract class ConfigFileUpdater<K, V>
    extends AbstractConfigFileUpdater<K, V> {
    /**
     * A set of identifiers for created data.
     */
    private final Set<K> createdData = new HashSet<>();

    /**
     * Construct a new instance.
     *
     * @param ftype  The type of the XML configuration file.
     * @param desc   A brief description about the target data.
     */
    protected ConfigFileUpdater(XmlConfigFile.Type ftype, String desc) {
        super(ftype, desc);
    }

    /**
     * Add the created or updated data to be saved into the configuration file.
     *
     * @param key      The identifier of the given data.
     * @param value    The value to be saved into the configuration file.
     * @param created  {@code true} means that the given data has been newly
     *                 created.
     * @return  {@code true} only if the given data was actually added.
     */
    public final boolean addUpdated(K key, V value, boolean created) {
        boolean added = addUpdated(key, value);
        if (added && created) {
            createdData.add(key);
        }
        return added;
    }

    // AbstractConfigFileUpdater

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean addRemoved(K key) {
        boolean added = super.addRemoved(key);
        if (added) {
            createdData.remove(added);
        }
        return added;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected final boolean onUpdated(K key, V value) {
        return createdData.contains(key);
    }

    /**
     * {@inheritDoc}
     */
    protected final void onRemoved(K key) {
    }
}
