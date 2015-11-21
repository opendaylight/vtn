/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.slf4j.Logger;

/**
 * {@code AbstractConfigFileUpdater} describes changes to be applied to the
 * configuration file.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 *
 * @param <K>  The type of the identifier which identifies data.
 * @param <V>  The type of the data.
 */
public abstract class AbstractConfigFileUpdater<K, V> {
    /**
     * A map that keeps updated data.
     */
    private final Map<K, V> updatedData = new HashMap<>();

    /**
     * A set of identifiers for removed data.
     */
    private final Set<K> removedData = new HashSet<>();

    /**
     * The type of the XML configuration file.
     */
    private final XmlConfigFile.Type  fileType;

    /**
     * A brief description about the target data.
     */
    private final String  description;

    /**
     * Construct a new instance.
     *
     * @param ftype  The type of the XML configuration file.
     * @param desc   A brief description about the target data.
     */
    protected AbstractConfigFileUpdater(XmlConfigFile.Type ftype,
                                        String desc) {
        fileType = ftype;
        description = desc;
    }

    /**
     * Add the updated data to be saved into the configuration file.
     *
     * @param key    The identifier of the given data.
     * @param value  The value to be saved into the configuration file.
     * @return  {@code true} only if the given data was actually added.
     */
    public final boolean addUpdated(K key, V value) {
        boolean added = !removedData.contains(key);
        if (added) {
            updatedData.put(key, value);
        }
        return added;
    }

    /**
     * Add the removed data to be removed from the configuration file.
     *
     * @param key  The identifier of the removed data.
     * @return  {@code true} only if the given data was actually added.
     */
    public boolean addRemoved(K key) {
        boolean added = removedData.add(key);
        if (added) {
            updatedData.remove(key);
        }
        return added;
    }

    /**
     * Determine whether the data specified by the given key has been removed
     * or not.
     *
     * @param key  The identifier of the data.
     * @return  {@code true} only if the specified data has been removed.
     */
    public boolean isRemoved(K key) {
        return removedData.contains(key);
    }

    /**
     * Apply changes to the configuration file.
     *
     * @param logger  A {@link Logger} instance.
     */
    public final void apply(Logger logger) {
        fixUp(logger);

        // Save updated data into configuration files.
        for (Map.Entry<K, V> entry: updatedData.entrySet()) {
            K key = entry.getKey();
            V value = entry.getValue();
            boolean created = onUpdated(key, value);
            String strKey = String.valueOf(key);

            logUpdated(logger, strKey, created);
            if (!XmlConfigFile.save(fileType, strKey, value)) {
                logger.warn("{}: {} could not be saved.", strKey, description);
            } else if (logger.isTraceEnabled()) {
                logger.trace("{}: {} has been saved.", strKey, description);
            }
        }

        // Remove configuration files associated with removed data.
        for (K key: removedData) {
            onRemoved(key);
            String strKey = String.valueOf(key);

            logRemoved(logger, strKey);
            if (!XmlConfigFile.delete(fileType, strKey)) {
                logger.warn("{}: {} configuration could not be deleted.",
                            strKey, description);
            } else if (logger.isTraceEnabled()) {
                logger.trace("{}: {} configuration has been deleted.",
                             strKey, description);
            }
        }
    }

    /**
     * Fix up changes before applying.
     *
     * <p>
     *   This method in this class does nothing. Subclass can override this
     *   method to insert hook just before applying changes.
     * </p>
     *
     * @param logger  A {@link Logger} instance.
     */
    protected void fixUp(Logger logger) {
    }

    /**
     * Log a created or updated data.
     *
     * @param logger   A {@link Logger} instance.
     * @param key      A string which specifies the given data.
     * @param created  {@code true} means that the given data has been created.
     *                 {@code false} means that the given data has been
     *                 updated.
     */
    protected void logUpdated(Logger logger, String key, boolean created) {
        logger.info("{}: {} has been {}.", key, description,
                    (created) ? "created" : "updated");
    }

    /**
     * Log a removed data.
     *
     * @param logger  A {@link Logger} instance.
     * @param key     A string which specifies the given data.
     */
    protected void logRemoved(Logger logger, String key) {
        logger.info("{}: {} has been removed.", key, description);
    }

    /**
     * Invoked when the given data is saved into the configuration file.
     *
     * @param key    The identifier of the given data.
     * @param value  The value to be saved into the configuration file.
     * @return  {@code true} if the given data is newly created data.
     *          Otherwise {@code false}.
     */
    protected abstract boolean onUpdated(K key, V value);

    /**
     * Invoked when the configuration file associated with the given data
     * is removed.
     *
     * @param key  The identifier of the removed data.
     */
    protected abstract void onRemoved(K key);
}
