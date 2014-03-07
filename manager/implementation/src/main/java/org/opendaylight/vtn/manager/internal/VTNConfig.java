/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.Field;
import java.util.Properties;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Static configuration for VTN Manager.
 */
public class VTNConfig {
    /**
     * Default value of {@link #nodeEdgeWait}.
     */
    private static final int  DEFAULT_NODE_EDGE_WAIT = 3000;

    /**
     * Default value of {@link #l2FlowPriority}.
     */
    private static final int  DEFAULT_L2FLOW_PRIORITY = 10;

    /**
     * Default value of {@link #flowModTimeout}.
     */
    private static final int  DEFAULT_FLOWMOD_TIMEOUT = 3000;

    /**
     * Default value of {@link #remoteFlowModTimeout}.
     */
    private static final int  DEFAULT_REMOTE_FLOWMOD_TIMEOUT = 5000;

    /**
     * Default value of {@link #remoteBulkFlowModTimeout}.
     */
    private static final int  DEFAULT_REMOTE_BULK_FLOWMOD_TIMEOUT = 15000;

    /**
     * Default value of {@link #cacheInitTimeout}.
     */
    private static final int  DEFAULT_CACHE_INIT_TIMEOUT = 3000;

    /**
     * Default value of {@link #cacheTransactionTimeout}.
     */
    private static final int  DEFAULT_CACHE_TRANSACTION_TIMEOUT = 10000;

    /**
     * Indicates a field keeps an integer value.
     */
    @Retention(RetentionPolicy.RUNTIME)
    @Target(ElementType.FIELD)
    private @interface IntConfig {
        /**
         * Maximum value.
         */
        int max();

        /**
         * Minimum value.
         */
        int min();
    }

    /**
     * Configuration loader.
     */
    private final class ConfigLoader {
        /**
         * Logger instance.
         */
        private final Logger  log = LoggerFactory.getLogger(VTNConfig.class);

        /**
         * The name of the container.
         */
        private final String  containerName;

        /**
         * Construct a new configuration loader.
         *
         * @param containerName  The name of the container.
         */
        private ConfigLoader(String containerName) {
            this.containerName = containerName;
        }

        /**
         * Load configuration.
         *
         * @param dir  A path to directory which contains configuration files.
         */
        private void load(String dir) {
            Properties global = loadProperty(dir, null, null);
            Properties prop = loadProperty(dir, containerName, global);

            // Fetch configurations.
            for (Field field: VTNConfig.class.getDeclaredFields()) {
                IntConfig icfg = field.getAnnotation(IntConfig.class);
                if (icfg != null) {
                    // Fetch an integer value.
                    fetch(prop, field, icfg);
                }
            }
        }

        /**
         * Fetch an integer configuration from the given property.
         *
         * @param prop   A property.
         * @param field  A field object associated with the configuration.
         * @param icfg   Definition of the integer configuration.
         */
        private void fetch(Properties prop, Field field, IntConfig icfg) {
            String name = field.getName();
            String str = prop.getProperty(name);
            int value;
            if (str == null) {
                if (log.isTraceEnabled()) {
                    log.trace("{}: {}: Not defined.", containerName, name);
                }
                return;
            }

            int min = icfg.min();
            int max = icfg.max();
            try {
                Integer v = Integer.decode(str);
                value = v.intValue();
                if (value > max) {
                    log.warn("{}: {}: Must be less than or equal {}",
                             containerName, name, max);
                    return;
                }
                if (value < min) {
                    log.warn("{}: {}: Must be greater than or equal {}",
                             containerName, name, min);
                    return;
                }
            } catch (NumberFormatException e) {
                log.warn("{}: {}: Illegal value: {}",
                         containerName, name, str);
                return;
            }

            if (log.isDebugEnabled()) {
                log.debug("{}: {} = {}", containerName, name, value);
            }

            field.setAccessible(true);

            try {
                field.setInt(VTNConfig.this, value);
            } catch (IllegalAccessException e) {
                log.error("{}: {}: Unable to set value: {}",
                          containerName, name, e);
            }
        }
    }

    /**
     * The number of milliseconds to wait for node edges to be detected.
     */
    @IntConfig(min = 0, max = 600000)
    private int  nodeEdgeWait = DEFAULT_NODE_EDGE_WAIT;

    /**
     * Priority value of layer 2 flow entries.
     */
    @IntConfig(min = 1, max = 999)
    private int  l2FlowPriority = DEFAULT_L2FLOW_PRIORITY;

    /**
     * The number of milliseconds to wait for completion of modification of
     * a single flow entry.
     */
    @IntConfig(min = 100, max = 60000)
    private int  flowModTimeout = DEFAULT_FLOWMOD_TIMEOUT;

    /**
     * The number of milliseconds to wait for remote cluster nodes to finish
     * to modify flow entries in a VTN flow.
     */
    @IntConfig(min = 1000, max = 60000)
    private int  remoteFlowModTimeout = DEFAULT_REMOTE_FLOWMOD_TIMEOUT;

    /**
     * The number of milliseconds to wait for remote cluster nodes to finish
     * modifying bulk flow entries.
     */
    @IntConfig(min = 3000, max = 600000)
    private int  remoteBulkFlowModTimeout =
        DEFAULT_REMOTE_BULK_FLOWMOD_TIMEOUT;

    /**
     * The number of milliseconds to wait for completion of cluster cache
     * initialization by another controller in the cluster.
     */
    @IntConfig(min = 100, max = 600000)
    private int  cacheInitTimeout  = DEFAULT_CACHE_INIT_TIMEOUT;

    /**
     * The number of milliseconds to wait for cluster cache transaction to
     * be established.
     */
    @IntConfig(min = 100, max = 600000)
    private int  cacheTransactionTimeout = DEFAULT_CACHE_TRANSACTION_TIMEOUT;

    /**
     * Construct a new configuration object.
     *
     * @param dir            A path to directory which contains configuartion
     *                       files.
     * @param containerName  The name of the container.
     */
    VTNConfig(String dir, String containerName) {
        ConfigLoader loader = new ConfigLoader(containerName);
        loader.load(dir);
    }

    /**
     * Return the number of milliseconds to wait for node edges to be detected.
     *
     * @return  The number of milliseconds to wait for node edges.
     */
    public int getNodeEdgeWait() {
        return nodeEdgeWait;
    }

    /**
     * Return priority value for layer 2 flow entries.
     *
     * @return  Priority value for layer 2 flow entries.
     */
    public int getL2FlowPriority() {
        return l2FlowPriority;
    }

    /**
     * Return the number of milliseconds to wait for completion of modification
     * of a single flow entry.
     *
     * @return  The number of milliseconds to wait.
     */
    public int getFlowModTimeout() {
        return flowModTimeout;
    }

    /**
     * Return the number of milliseconds to wait for remote cluster nodes to
     * finish modifying flow entries in a VTN flow.
     *
     * @return  The number of milliseconds to wait.
     */
    public int getRemoteFlowModTimeout() {
        return remoteFlowModTimeout;
    }

    /**
     * Return the number of milliseconds to wait for remote cluster nodes to
     * finish modifying bulk flow entries.
     *
     * @return  The number of milliseconds to wait.
     */
    public int getRemoteBulkFlowModTimeout() {
        return remoteBulkFlowModTimeout;
    }

    /**
     * Return the number of milliseconds to wait for completion of cluster
     * cache initialization by another controller in the cluster.
     *
     * @return  The number of milliseconds to wait for completion of cluster
     *          cache initialization.
     */
    public int getCacheInitTimeout() {
        return cacheInitTimeout;
    }

    /**
     * Return the number of milliseconds to wait for cluster cache transaction
     * to be established.
     *
     * @return  The number of milliseconds to wait for cluster cache
     *          transaction to be established.
     */
    public int getCacheTransactionTimeout() {
        return cacheTransactionTimeout;
    }

    /**
     * Load property from the given file.
     *
     * @param dir            A path to directory which contains configuartion
     *                       files.
     * @param containerName  The name of the container.
     * @param defaults       A property which contains default values.
     * @return  A property object which contains properties loaded from the
     *          given file.
     */
    private Properties loadProperty(String dir, String containerName,
                                    Properties defaults) {
        // Construct configuration file path.
        StringBuilder builder = new StringBuilder("vtnmanager");
        if (containerName != null) {
            builder.append('-').append(containerName);
        }
        String fname = builder.append(".ini").toString();
        File file = new File(dir, fname);

        // Load properties.
        FileInputStream fis = null;
        Properties prop = new Properties(defaults);
        try {
            fis = new FileInputStream(file);
            prop.load(fis);
        } catch (IOException e) {
        } finally {
            if (fis != null) {
                try {
                    fis.close();
                } catch (IOException e) {
                }
            }
        }

        return prop;
    }
}
