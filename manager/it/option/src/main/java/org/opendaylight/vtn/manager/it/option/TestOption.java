/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.option;

import static org.ops4j.pax.exam.CoreOptions.frameworkProperty;
import static org.ops4j.pax.exam.CoreOptions.junitBundles;
import static org.ops4j.pax.exam.CoreOptions.systemPackages;
import static org.ops4j.pax.exam.CoreOptions.systemProperty;
import static org.ops4j.pax.exam.CoreOptions.systemTimeout;
import static org.ops4j.pax.exam.CoreOptions.workingDirectory;

import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.ops4j.pax.exam.CoreOptions;
import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.options.DefaultCompositeOption;
import org.ops4j.pax.exam.options.MavenArtifactProvisionOption;
import org.ops4j.pax.exam.util.PathUtils;

/**
 * Helper class which provides Pax Exam configurations.
 *
 * <p>
 *   Note that this class is never loaded into the PaxExam test container.
 *   All methods provided by this class can be used in a method that returns
 *   PaxExam configuration.
 * </p>
 */
public final class TestOption {
    /**
     * Logger instance.
     */
    private static final Logger LOG = LoggerFactory.
        getLogger(TestOption.class);

    /**
     * A relative path to the base configuration file directory.
     */
    public static final String CONFIG_DIR_BASE = "configuration";

    /**
     * The number of milliseconds to wait for the framework to start.
     */
    private static final int  SYSTEM_TIMEOUT = 600000;

    /**
     * The size of internal buffer used by
     * {@link #copyFile(InputStream, File)}.
     */
    private static final int  IO_BUFSIZE = 8192;

    /**
     * Group ID for SLF4J bundles.
     */
    private static final String  SLF4J = "org.slf4j";

    /**
     * Group ID for Logback bundles.
     */
    private static final String  LOGBACK = "ch.qos.logback";

    /**
     * Group ID for Pax Exam bundles.
     */
    private static final String  PAX_EXAM = "org.ops4j.pax.exam";

    /**
     * Group ID for VTN bundles.
     */
    private static final String  VTN = "org.opendaylight.vtn";

    /**
     * Group ID for controller bundles.
     */
    private static final String  CONTROLLER = "org.opendaylight.controller";

    /**
     * Group ID for YANG modules provided by controller.
     */
    private static final String CONTROLLER_MODEL = CONTROLLER + ".model";

    /**
     * Group ID for third-party bundles in the controller.
     */
    private static final String  CONTROLLER_THIRD_PARTY =
        CONTROLLER + ".thirdparty";

    /**
     * GroupID for YANG tools.
     */
    private static final String  YANGTOOLS = "org.opendaylight.yangtools";

    /**
     * Group ID for MD-SAL bundles.
     */
    private static final String  MDSAL = "org.opendaylight.mdsal";

    /**
     * Group ID for YANG modules provided by MD-SAL.
     */
    private static final String  MDSAL_MODEL = MDSAL + ".model";

    /**
     * Group ID for YANG models provided by openflowplugin.
     */
    private static final String  OPENFLOW_MODEL =
        "org.opendaylight.openflowplugin.model";

    /**
     * The name of the system property that specifies whether the config
     * persister is active or not.
     */
    private static final String PROP_PERSISTER_ACTIVE =
        "netconf.config.persister.active";

    /**
     * The name of the system property that specifies the storage adapter
     * class for the config persister.
     */
    private static final String PROP_PERSISTER_ADAPTER =
        "netconf.config.persister.1.storageAdapterClass";

    /**
     * The name of the system property that specifies the configuration file
     * for the config subsystem.
     */
    private static final String PROP_FILE_STORAGE =
        "netconf.config.persister.1.properties.fileStorage";

    /**
     * The name of the system property that specifies the number of config
     * backups.
     */
    private static final String PROP_NUM_BACKUPS =
        "netconf.config.persister.1.properties.numberOfBackups";

    /**
     * The name of the storage adapter class for the config persister.
     */
    private static final String  STORAGE_ADAPTER =
        "org.opendaylight.controller.config.persist.storage.file.xml." +
        "XmlFileStorageAdapter";

    /**
     * The name of the framework property that specifies the storage path.
     */
    private static final String  FPROP_STORAGE = "org.osgi.framework.storage";

    /**
     * The name of the system property that specifies the temporary directory
     * for integration test.
     */
    private static final String VTN_PROP_TMPDIR = "vtn.it.tmpdir";

    /**
     * Path to the temporary directory.
     */
    private static File  tmpDirectory;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private TestOption() {}

    /**
     * Return Pax Exam option to load the given OSGi bundle.
     *
     * @param group  Group ID for the target bundle.
     * @param name   Artifact ID for the target bundle.
     * @return  A {@link Option} instance.
     */
    public static MavenArtifactProvisionOption mavenBundle(String group,
                                                           String name) {
        try {
            return CoreOptions.mavenBundle(group, name).versionAsInProject();
        } catch (RuntimeException e) {
            String msg = "Could not load bundle: group=" + group +
                ", name=" + name;
            LOG.error(msg, e);
            throw new IllegalArgumentException(msg);
        }
    }

    /**
     * Return Pax Exam option to be shared with all the integration tests.
     *
     * @return  An {@link Option} instance.
     */
    public static Option commonOption() {
        // Create configuration directory.
        File configBase = new File(CONFIG_DIR_BASE);
        if (configBase.exists()) {
            delete(configBase);
        }

        File configDir = createPath(configBase, "startup", "vtn", "CONFIG");
        configDir.mkdirs();

        // Install common configuration files to the test environment.
        String vtnConfigXml = "vtn-config.xml";
        File dst = new File(configDir, vtnConfigXml);
        copyFile(getResource(vtnConfigXml), dst);

        File tmpDir = getTmpDirectory();
        String tmpPath = tmpDir.getAbsolutePath();
        File storageDir = new File(tmpDir, "osgi-storage");
        if (storageDir.exists()) {
            delete(storageDir);
        }
        String storagePath = storageDir.getAbsolutePath();

        return new DefaultCompositeOption(
            // Set system timeout.
            systemTimeout(SYSTEM_TIMEOUT),

            // Use own working directory in order to prevent from creating
            // temporary files in /tmp.
            workingDirectory(tmpPath),
            frameworkProperty(FPROP_STORAGE).value(storagePath));
    }

    /**
     * Return Pax Exam option to load OSGi bundles for common integration test.
     *
     * @return  An {@link Option} instance.
     */
    public static Option vtnManagerCommonBundles() {
        // Initialize the test environment.
        Option common = commonOption();

        // Install the configuration for config subsystem.
        File tmpDir = getTmpDirectory();
        String ctlrXml = "controller.xml";
        File dst = new File(tmpDir, ctlrXml);
        copyFile(getResource(ctlrXml), dst);
        String ctlrXmlPath = dst.getAbsolutePath();

        // Determine configuration file paths.
        File baseDir = new File(PathUtils.getBaseDir());
        File resDir = createPath(baseDir, "src", "test", "resources");
        String logbackXml = new File(resDir, "logback.xml").toURI().toString();

        return new DefaultCompositeOption(
            common,
            systemProperty("osgi.console").value("2401"),
            systemProperty("logback.configurationFile").value(logbackXml),
            systemProperty("osgi.bundles.defaultStartLevel").value("4"),

            // Set fail if unresolved bundle present.
            systemProperty("pax.exam.osgi.unresolved.fail").value("true"),

            // SLF4J
            mavenBundle(SLF4J, "jcl-over-slf4j"),
            mavenBundle(SLF4J, "slf4j-api"),
            mavenBundle(SLF4J, "log4j-over-slf4j"),

            // Logback
            mavenBundle(LOGBACK, "logback-core"),
            mavenBundle(LOGBACK, "logback-classic"),

            // Load MD-SAL bundles.
            mdSalBundles(),

            // Load Pax Exam bundles.
            CoreOptions.mavenBundle(PAX_EXAM, "pax-exam-container-native"),
            CoreOptions.mavenBundle(PAX_EXAM, "pax-exam-junit4"),
            CoreOptions.mavenBundle(PAX_EXAM, "pax-exam-link-mvn"),
            CoreOptions.mavenBundle("org.ops4j.pax.url", "pax-url-aether"),

            // Configure the config persister in the config subsystem.
            systemProperty(PROP_PERSISTER_ACTIVE).value("1"),
            systemProperty(PROP_PERSISTER_ADAPTER).value(STORAGE_ADAPTER),
            systemProperty(PROP_NUM_BACKUPS).value("1"),

            // Specify the location of the MD-SAL module configuration file.
            systemProperty(PROP_FILE_STORAGE).value(ctlrXmlPath),

            // Load YANG models.
            mavenBundle(MDSAL_MODEL, "yang-ext"),
            mavenBundle(MDSAL_MODEL, "ietf-type-util"),
            mavenBundle(MDSAL_MODEL, "ietf-yang-types"),
            mavenBundle(MDSAL_MODEL, "ietf-yang-types-20130715"),
            mavenBundle(MDSAL_MODEL, "ietf-inet-types"),
            mavenBundle(MDSAL_MODEL, "ietf-topology"),
            mavenBundle(MDSAL_MODEL, "opendaylight-l2-types"),
            mavenBundle(CONTROLLER_MODEL, "model-inventory"),
            mavenBundle(CONTROLLER_MODEL, "model-topology"),
            mavenBundle(OPENFLOW_MODEL, "model-flow-base"),
            mavenBundle(OPENFLOW_MODEL, "model-flow-service"),
            mavenBundle(OPENFLOW_MODEL, "model-flow-statistics"),

            // Load OpenDaylight third-party bundles.
            mavenBundle(CONTROLLER_THIRD_PARTY, "net.sf.jung2"),

            // VTN Manager bundels.
            mavenBundle(VTN, "manager"),
            mavenBundle(VTN, "manager.model"),
            mavenBundle(VTN, "manager.implementation"),
            mavenBundle(VTN, "manager.it.ofmock"),
            mavenBundle(VTN, "manager.it.ownermock"),
            mavenBundle(VTN, "manager.it.util"),

            // Load JUnit and Mockito bundles.
            junitBundles(),
            mavenBundle("org.mockito", "mockito-core"),
            mavenBundle("org.objenesis", "objenesis"),
            frameworkProperty("felix.bootdelegation.implicit").value("false"));
    }

    /**
     * Detele the specified file.
     *
     * @param file  A {@link File} to be deleted.
     */
    public static void delete(File file) {
        File[] list = file.listFiles();
        if (list != null) {
            for (File f: list) {
                delete(f);
            }
        }
        file.delete();
    }

    /**
     * Create a {@link File} instance from the given path components.
     *
     * @param parent  A {@link File} instance which represents the parent
     *                directory path.
     * @param comps   Path components in a file path.
     *                {@code null} must not be passed.
     * @return  A {@link File} instance associated with the given path.
     */
    public static File createPath(File parent, String ... comps) {
        File f = parent;
        for (String c: comps) {
            f = new File(f, c);
        }

        return f;
    }

    /**
     * Read data from the given input stream, and write to the given file.
     *
     * @param in   An input stream.
     * @param dst  Destination file path.
     */
    public static void copyFile(InputStream in, File dst) {
        byte[] buffer = new byte[IO_BUFSIZE];
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(dst);
            for (;;) {
                int nbytes = in.read(buffer);
                if (nbytes < 0) {
                    break;
                }

                out.write(buffer, 0, nbytes);
            }
        } catch (IOException e) {
            String msg = "Unable to copy resource to " + dst;
            throw new IllegalStateException(msg, e);
        } finally {
            close(in);
            close(out);
        }
    }

    /**
     * Return an input stream associated with the given resource in the
     * manager.it.util module.
     *
     * @param name  The name of the resource.
     * @return  An {@link InputStream} instance associated with the given
     *          resource.
     */
    public static InputStream getResource(String name) {
        return getResource(TestOption.class, name);
    }

    /**
     * Return an input stream associated with the given resource.
     *
     * @param cls   A {@link Class} instance used to load resource.
     * @param name  The name of the resource.
     * @return  An {@link InputStream} instance associated with the given
     *          resource.
     */
    public static InputStream getResource(Class<?> cls, String name) {
        InputStream in = cls.getResourceAsStream("/" + name);
        if (in == null) {
            String msg = "Unable to load resource: " + name;
            throw new IllegalStateException(msg);
        }

        return in;
    }

    /**
     * Close the given closeable.
     *
     * @param c  A {@link Closeable} to be closed.
     */
    private static void close(Closeable c) {
        if (c != null) {
            try {
                c.close();
            } catch (IOException e) {
                throw new IllegalStateException("close() failed: " + c, e);
            }
        }
    }

    /**
     * Return Pax Exam option that loads the MD-SAL bundles.
     *
     * @return  An {@link Option} instance that loads the MD-SAL bundles.
     */
    private static Option mdSalBundles() {
        return new DefaultCompositeOption(
            // YANG tools
            mavenBundle(YANGTOOLS, "concepts"),
            mavenBundle(YANGTOOLS, "util"),
            mavenBundle(YANGTOOLS, "yang-common"),
            mavenBundle(YANGTOOLS, "object-cache-api"),
            mavenBundle(YANGTOOLS, "object-cache-guava"),
            mavenBundle(YANGTOOLS, "yang-data-api"),
            mavenBundle(YANGTOOLS, "yang-data-util"),
            mavenBundle(YANGTOOLS, "yang-data-impl"),
            mavenBundle(YANGTOOLS, "yang-model-api"),
            mavenBundle(YANGTOOLS, "yang-model-util"),
            mavenBundle(YANGTOOLS, "yang-parser-api"),
            mavenBundle(YANGTOOLS, "yang-parser-impl"),

            // MD-SAL
            mavenBundle(MDSAL, "yang-binding"),
            mavenBundle(MDSAL, "mdsal-binding-generator-api"),
            mavenBundle(MDSAL, "mdsal-binding-generator-util"),
            mavenBundle(MDSAL, "mdsal-binding-generator-impl"),
            mavenBundle(MDSAL, "mdsal-binding-dom-codec"),

            // Controller
            mavenBundle(CONTROLLER, "sal-common-api"),
            mavenBundle(CONTROLLER, "sal-common-impl"),
            mavenBundle(CONTROLLER, "sal-binding-util"),
            mavenBundle(CONTROLLER, "sal-common-util"),
            mavenBundle(CONTROLLER, "sal-core-api").update(),
            mavenBundle(CONTROLLER, "sal-binding-api"),
            mavenBundle(CONTROLLER, "sal-broker-impl"),
            mavenBundle(CONTROLLER, "sal-dom-config"),
            mavenBundle(CONTROLLER, "sal-inmemory-datastore"),
            mavenBundle(CONTROLLER, "sal-dom-broker-config"),
            mavenBundle(CONTROLLER, "sal-core-spi").update(),
            mavenBundle(CONTROLLER, "sal-binding-broker-impl"),
            mavenBundle(CONTROLLER, "sal-binding-config"),
            mavenBundle(CONTROLLER, "config-api"),
            mavenBundle(CONTROLLER, "config-manager"),
            mavenBundle(CONTROLLER, "config-util"),
            mavenBundle(CONTROLLER, "config-manager-facade-xml"),
            mavenBundle(CONTROLLER, "yang-jmx-generator"),
            mavenBundle(CONTROLLER, "logback-config"),
            mavenBundle(CONTROLLER, "config-persister-api"),
            mavenBundle(CONTROLLER, "config-persister-impl"),
            mavenBundle(CONTROLLER,
                        "config-persister-file-xml-adapter").noStart(),

            // Requisites
            systemPackages("sun.nio.ch", "sun.misc"),
            mavenBundle("org.apache.commons", "commons-lang3"),
            mavenBundle("com.google.guava", "guava"),
            mavenBundle("com.github.romix", "java-concurrent-hash-trie-map"),
            mavenBundle("org.javassist", "javassist"),
            mavenBundle("org.antlr", "antlr4-runtime"),
            mavenBundle("com.lmax", "disruptor"),
            mavenBundle("commons-codec", "commons-codec"),
            mavenBundle("commons-io", "commons-io"),
            mavenBundle("org.eclipse.persistence",
                        "org.eclipse.persistence.core"),
            mavenBundle("org.eclipse.persistence",
                        "org.eclipse.persistence.moxy"));
    }

    /**
     * Return path to the temporary directory.
     *
     * @return  Path to the temporary directory.
     */
    private static File getTmpDirectory() {
        File dir = tmpDirectory;
        if (dir == null) {
            // Determine the temporary directory.
            String path = System.getProperty(VTN_PROP_TMPDIR);
            if (path == null || path.isEmpty()) {
                String msg = "Temporary directory is not specified.";
                LOG.error(msg);
                throw new IllegalStateException(msg);
            }

            dir = new File(path);
            tmpDirectory = dir;
        }

        return dir;
    }
}
