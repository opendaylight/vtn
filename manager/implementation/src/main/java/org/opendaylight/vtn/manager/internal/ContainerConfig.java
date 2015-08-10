/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.io.Serializable;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.IObjectReader;
import org.opendaylight.controller.sal.utils.ObjectReader;
import org.opendaylight.controller.sal.utils.ObjectWriter;
import org.opendaylight.controller.sal.utils.Status;

/**
 * {@code ContainerConfig} class provides utilities to save non-volatile
 * configurations in a container into files.
 *
 * <p>
 *   The configuration should be represented by a map. The contents of a map
 *   are saved into separate directory, and each map entry in a map is saved
 *   into a separate file in the directory. The map key must be a string
 *   because it is used to determine configuration file name.
 * </p>
 */
public final class ContainerConfig implements IObjectReader {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(ContainerConfig.class);

    /**
     * The name of the base directory.
     */
    private static final String  BASE_DIRECTORY = "vtn";

    /**
     * Suffix of the name of the map entry file.
     */
    private static final String  FILE_SUFFIX = ".conf";

    /**
     * Specify the type of configuration.
     */
    public static enum Type {
        /**
         * Indicates the configuration of the VTN.
         */
        TENANT;
    }

    /**
     * The name of the container.
     */
    private final String  containerName;

    /**
     * Construct a new instance.
     *
     * @param container  The name of the container.
     */
    public ContainerConfig(String container) {
        containerName = container;
    }

    /**
     * Initialize the configuration directory for the VTN Manager.
     */
    public void init() {
        File parent = getDirectory();
        for (Type type: Type.values()) {
            File dir = new File(parent, type.toString());
            if (dir.isDirectory()) {
                continue;
            }

            if (delete(dir)) {
                LOG.warn("Deleted unexpected configuration file: {}", dir);
            }

            if (dir.mkdirs()) {
                LOG.info("Created configuration directory: {}", dir);
            } else {
                LOG.error("Failed to create configuration directory: {}", dir);
            }
        }
    }

    /**
     * Remove all configuration files for the specified container.
     */
    public void cleanUp() {
        File dir = getDirectory();
        LOG.info("Delete configuration directory: {}", dir);
        delete(dir);
    }

    /**
     * Write a map entry to the file.
     *
     * @param type   The type of the configuration.
     * @param key    A key associated with the map entry.
     * @param value  A value to be saved into the file.
     * @return  A {@link Status} instance which indicates the result.
     */
    public Status save(Type type, String key, Serializable value) {
        String path = getPath(type, key).getPath();
        ObjectWriter wrt = new ObjectWriter();
        return wrt.write(value, path);
    }

    /**
     * Load a map entry from the file.
     *
     * @param type   The type of the configuration.
     * @param key    A key associated with the map entry.
     * @return  A map entry loaded from the file.
     *          {@code null} is returned on failure.
     *          The specified file will be deleted if this method returns
     *          {@code null}.
     */
    public Object load(Type type, String key) {
        File file = getPath(type, key);
        ObjectReader rdr = new ObjectReader();
        Object obj = rdr.read(this, file.getPath());
        if (obj == null && delete(file)) {
            LOG.warn("Delete broken configuration file: {}", file);
        }
        return obj;
    }

    /**
     * Delete the configuration file for the specified configuration.
     *
     * @param type   The type of the configuration.
     * @param key    A key associated with the map entry.
     * @return  {@code true} if the specified configuration file was deleted
     *          successfully. Otherwise {@code false}.
     */
    public boolean delete(Type type, String key) {
        File file = getPath(type, key);
        return deleteFile(file);
    }

    /**
     * Delete all the configuration files for the specified configuration.
     *
     * @param type    The type of the configuration.
     * @param retain  A set of map keys to be retained.
     * @return  {@code true} if the operation was successfully completed.
     *          {@code false} if at least one error occurred.
     */
    public boolean deleteAll(Type type, Set<String> retain) {
        File dir = getDirectory(type);
        File[] files = dir.listFiles();
        if (files == null) {
            return false;
        }

        boolean result = true;
        if (retain == null) {
            // Delete all files unconditionally.
            for (File f: files) {
                if (!delete(f)) {
                    result = false;
                }
            }
        } else {
            for (File f: files) {
                String key = getMapKey(f);
                if (key == null) {
                    // Delete unexpected file.
                    if (delete(f)) {
                        LOG.warn("Deleted unexpected file: {}", f);
                    } else {
                        result = false;
                    }
                    continue;
                }

                if (retain.contains(key)) {
                    continue;
                }

                if (!deleteFile(f)) {
                    result = false;
                }
            }
        }

        return result;
    }

    /**
     * Return a list of valid map keys saved in the configuration directory.
     *
     * @param type    The type of the configuration.
     * @return  A list of valid map keys.
     */
    public List<String> getKeys(Type type) {
        List<String> list = new ArrayList<String>();
        File dir = getDirectory(type);
        File[] files = dir.listFiles();
        if (files == null) {
            return list;
        }

        for (File f: files) {
            String key = getMapKey(f);
            if (key != null) {
                list.add(key);
            }
        }

        return list;
    }

    /**
     * Return the configuration directory for the container.
     *
     * @return  A {@link File} instance which represents the configuration
     *          directory.
     */
    private File getDirectory() {
        String base = GlobalConstants.STARTUPHOME.toString();
        File parent = new File(base, containerName);
        return new File(parent, BASE_DIRECTORY);
    }

    /**
     * Return the configuration directory for the specified configuration type.
     *
     * @param type  The type of the configuration.
     * @return  A {@link File} instance which represents the configuration
     *          directory for the specified configuration type.
     */
    private File getDirectory(Type type) {
        File parent = getDirectory();
        return new File(parent, type.toString());
    }

    /**
     * Return the path to the configuration file.
     *
     * @param type  The type of the configuration.
     * @param key   A map key associated with the value to be saved.
     * @return  A {@link File} instance which represents the specified
     *          configuration file path.
     */
    private File getPath(Type type, String key) {
        File parent = getDirectory(type);
        String fname = key + FILE_SUFFIX;
        return new File(parent, fname);
    }

    /**
     * Delete the specified file or directory.
     *
     * <p>
     *   If a directory is specified all files under the specified directory
     *   are removed.
     * </p>
     *
     * @param file  A {@link File} instance which specifies a file or directory
     *              to be removed.
     * @return  {@code true} if the specified file or directory was deleted
     *          successfully. Otherwise {@code false}.
     */
    private boolean delete(File file) {
        if (!file.exists()) {
            return false;
        }

        File[] files = file.listFiles();
        if (files == null) {
            // Delete the specified file.
            boolean ret = file.delete();
            if (!ret) {
                LOG.error("Failed to delete file: {}", file);
            }

            return ret;
        }

        // Make the specified directory empty.
        for (File f: files) {
            delete(f);
        }

        // Delete the specified directory.
        boolean ret = file.delete();
        if (!ret) {
            LOG.error("Failed to delete directory: {}", file);
        }

        return ret;
    }

    /**
     * Delete the specified configuration file.
     *
     * @param file  A {@link File} instance which specifies the configuration
     *              file path.
     * @return  {@code true} if the specified file was deleted successfully.
     *          Otherwise {@code false}.
     */
    private boolean deleteFile(File file) {
        boolean ret = file.delete();
        if (ret) {
            LOG.debug("Deleted configuration file: {}", file);
        } else if (file.exists()) {
            LOG.error("Failed to delete configuration file: {}", file);
        }

        return ret;
    }

    /**
     * Return the map key embedded in the configuration file name.
     *
     * @param file  A {@link File} which indicates the configuration file path.
     * @return  The name of the map key.
     *          {@code null} is returned if an unexpected path is specified.
     */
    private String getMapKey(File file) {
        String name = file.getName();
        if (name.endsWith(FILE_SUFFIX)) {
            int slen = FILE_SUFFIX.length();
            int len = name.length();
            if (len > slen) {
                return name.substring(0, len - slen);
            }
        }

        return null;
    }

    // IObjectReader

    /**
     * Read an object from the given input stream.
     *
     * @param in  Input stream.
     * @return    An object.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @Override
    public Object readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        return in.readObject();
    }
}
