/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.io.File;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;
import javax.xml.bind.ValidationEvent;
import javax.xml.bind.ValidationEventHandler;
import javax.xml.bind.ValidationEventLocator;

import org.w3c.dom.Node;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.util.log.FixedLogger;
import org.opendaylight.vtn.manager.internal.util.log.VTNLogLevel;

import org.opendaylight.controller.sal.utils.GlobalConstants;

/**
 * Helper class used to write configurations into a file in XML format.
 */
public final class XmlConfigFile {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(XmlConfigFile.class);

    /**
     * A {@link File} instance which represents the configuration file
     * directory.
     */
    private static final File  BASE_DIRECTORY;

    /**
     * Suffix of the name of the map entry file.
     */
    private static final String  FILE_SUFFIX = ".xml";

    /**
     * Specify the type of configuration.
     */
    public static enum Type {
        /**
         * Indicates the global configuration.
         */
        CONFIG,

        /**
         * Indicates the configuration of the VTN.
         */
        VTN,

        /**
         * Indicates the configuration for the path policy.
         */
        PATHPOLICY,

        /**
         * Indicates the configuration of the global path map list.
         */
        PATHMAP,

        /**
         * Indicates the configuration for the flow condition.
         */
        FLOWCOND,

        /**
         * Indicates the configuratoin for the static network topology.
         */
        TOPOLOGY;
    }

    /**
     * Initialize the configuration file directory.
     */
    static {
        String base = GlobalConstants.STARTUPHOME.toString();
        File parent = new File(base, GlobalConstants.DEFAULT.toString());
        BASE_DIRECTORY = new File(parent, "vtn");
    }

    /**
     * JAXB validation event handler used to load or save configuration file.
     *
     * <p>
     *   This handler terminates the current operation when an error is
     *   detected.
     * </p>
     */
    private static final class JaxbEventHandler
        implements ValidationEventHandler {
        /**
         * A separator in log message.
         */
        private static final String  LOG_SEPARATOR = ": ";

        /**
         * A separator for a string that indicates the location.
         */
        private static final String  LOCATION_SEPARATOR = ", ";

        /**
         * Invoked when a validation warning or error is notified.
         *
         * @param event  A {@link ValidationEvent} instance.
         * @return  {@code true} if the JAXB provider should attempt to
         *          continue the current operation.
         *          {@code false} if the JAXB provider should terminate the
         *          current operation.
         * @throws IllegalArgumentException
         *    {@code event} is {@code null}.
         */
        @Override
        public boolean handleEvent(ValidationEvent event) {
            if (event == null) {
                throw new IllegalArgumentException();
            }

            StringBuilder builder = new StringBuilder();
            FixedLogger logger;
            boolean ret;
            int severity = event.getSeverity();
            if (severity == ValidationEvent.WARNING) {
                // Continue operation.
                ret = true;
                logger = new FixedLogger(LOG, VTNLogLevel.WARN);
                builder.append("Warning on XML validation: ");
            } else {
                ret = false;
                logger = new FixedLogger(LOG, VTNLogLevel.ERROR);
                if (severity == ValidationEvent.ERROR) {
                    builder.append("[ERROR]");
                } else if (severity == ValidationEvent.FATAL_ERROR) {
                    builder.append("[FATAL_ERROR]");
                } else {
                    // This should never happen.
                    builder.append("[UNKNOWN:").append(severity).append(']');
                }
                builder.append(LOG_SEPARATOR);
            }

            setLocation(builder, event);
            String msg = event.getMessage();
            if (msg != null) {
                builder.append(msg);
            }

            logger.log(builder.toString(), event.getLinkedException());

            return ret;
        }

        /**
         * Append a string that indicates the location of a validation event
         * to the given string builder.
         *
         * @param builder  A {@link StringBuilder} instance.
         * @param event    A {@link ValidationEvent} instance.
         */
        private void setLocation(StringBuilder builder,
                                 ValidationEvent event) {
            ValidationEventLocator locator = event.getLocator();
            if (locator != null) {
                URL url = locator.getURL();
                int line = locator.getLineNumber();
                Node node = locator.getNode();
                Object o = locator.getObject();

                String sep = "";
                if (url != null) {
                    builder.append(url);
                    sep = LOCATION_SEPARATOR;
                }
                if (line >= 0) {
                    builder.append(sep).append("line=").append(line);
                    sep = LOCATION_SEPARATOR;
                }
                if (node != null) {
                    builder.append(sep).append("node=").append(node);
                    sep = LOCATION_SEPARATOR;
                }
                if (o != null) {
                    builder.append(sep).append("object=").append(o);
                    sep = LOCATION_SEPARATOR;
                }

                if (sep.length() != 0) {
                    builder.append(LOG_SEPARATOR);
                }
            }
        }
    }

    /**
     * Private constructor that protects this class from instantiating.
     */
    private XmlConfigFile() {}

    /**
     * Initialize the configuration directory for the VTN Manager.
     */
    public static void init() {
        for (Type type: Type.values()) {
            File dir = new File(BASE_DIRECTORY, type.toString());
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
     * Write a map entry to the XML file.
     *
     * @param type   The type of the configuration.
     * @param key    A key associated with the map entry.
     * @param value  A value to be saved into XML file.
     *               An object bound to XML root element needs to be specified.
     * @return  {@code true} on success. {@code false} on failure.
     */
    public static boolean save(Type type, String key, Object value) {
        File file = getPath(type, key);

        try {
            Marshaller marshaller = createMarshaller(value.getClass());
            marshaller.marshal(value, file);
            return true;
        } catch (Exception e) {
            LOG.error("Failed to save configuration for " +
                      value.getClass().getName(), e);
            return false;
        }
    }

    /**
     * Load a map entry from the file.
     *
     * @param type   The type of the configuration.
     * @param key    A key associated with the map entry.
     * @param cls    The type of object to be bound to XML element.
     * @param <T>  The type of object to be bound to XML element.
     * @return  A map entry loaded from the file.
     *          {@code null} is returned on failure.
     *          The specified file will be deleted if this method returns
     *          {@code null}.
     */
    public static <T> T load(Type type, String key, Class<T> cls) {
        File file = getPath(type, key);
        if (!file.isFile()) {
            return null;
        }

        try {
            Unmarshaller unmarshaller = createUnmarshaller(cls);
            Object o = unmarshaller.unmarshal(file);
            if (cls.isInstance(o)) {
                return cls.cast(o);
            }

            throw new IllegalStateException("Unexpected object: " + o);
        } catch (Exception e) {
            LOG.error("Failed to load configuration for " + cls.getName(), e);
            if (delete(file)) {
                LOG.warn("Delete broken configuration file: {}", file);
            }
            return null;
        }
    }

    /**
     * Delete the configuration file for the specified configuration.
     *
     * @param type   The type of the configuration.
     * @param key    A key associated with the map entry.
     * @return  {@code true} if the specified configuration file was deleted
     *          successfully. Otherwise {@code false}.
     */
    public static boolean delete(Type type, String key) {
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
    public static boolean deleteAll(Type type, Set<String> retain) {
        File dir = getDirectory(type);
        File[] files = dir.listFiles();
        boolean result;
        if (files == null) {
            result = false;
        } else {
            result = true;
            Set<String> rset = retain;
            if (rset == null) {
                // Delete all files unconditionally.
                rset = Collections.<String>emptySet();
            }
            for (File f: files) {
                String key = getMapKey(f);
                if (key == null) {
                    // Delete unexpected file.
                    if (delete(f)) {
                        LOG.warn("Deleted unexpected file: {}", f);
                    } else {
                        result = false;
                    }
                } else if (!rset.contains(key) && !deleteFile(f)) {
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
    public static List<String> getKeys(Type type) {
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
     * Return the configuration directory for the specified configuration type.
     *
     * @param type  The type of the configuration.
     * @return  A {@link File} instance which represents the configuration
     *          directory for the specified configuration type.
     */
    private static File getDirectory(Type type) {
        return new File(BASE_DIRECTORY, type.toString());
    }

    /**
     * Return the path to the configuration file.
     *
     * @param type  The type of the configuration.
     * @param key   A map key associated with the value to be saved.
     * @return  A {@link File} instance which represents the specified
     *          configuration file path.
     */
    private static File getPath(Type type, String key) {
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
    private static boolean delete(File file) {
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
    private static boolean deleteFile(File file) {
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
    private static String getMapKey(File file) {
        String name = file.getName();
        if (name.endsWith(FILE_SUFFIX) && file.isFile()) {
            int slen = FILE_SUFFIX.length();
            int len = name.length();
            if (len > slen) {
                return name.substring(0, len - slen);
            }
        }

        return null;
    }

    /**
     * Create JAXB marshaller.
     *
     * @param cls  A class to be bound to XML.
     * @return  A {@link Marshaller} instance.
     * @throws JAXBException  Failed to create JAXB marshaller.
     */
    private static Marshaller createMarshaller(Class<?> cls)
        throws JAXBException {
        JAXBContext jc = JAXBContext.newInstance(cls);
        Marshaller m = jc.createMarshaller();
        m.setProperty(Marshaller.JAXB_FORMATTED_OUTPUT, Boolean.TRUE);
        m.setEventHandler(new JaxbEventHandler());
        return m;
    }

    /**
     * Create JAXB unmarshaller.
     *
     * @param cls  A class to be bound to XML.
     * @return  A {@link Unmarshaller} instance.
     * @throws JAXBException  Failed to create JAXB unmarshaller.
     */
    private static Unmarshaller createUnmarshaller(Class<?> cls)
        throws JAXBException {
        JAXBContext jc = JAXBContext.newInstance(cls);
        Unmarshaller um = jc.createUnmarshaller();
        um.setEventHandler(new JaxbEventHandler());
        return um;
    }
}
