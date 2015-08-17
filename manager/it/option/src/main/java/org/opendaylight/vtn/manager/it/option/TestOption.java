/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.option;

import static org.ops4j.pax.exam.CoreOptions.frameworkProperty;
import static org.ops4j.pax.exam.CoreOptions.systemPackages;
import static org.ops4j.pax.exam.CoreOptions.systemProperty;
import static org.ops4j.pax.exam.CoreOptions.workingDirectory;

import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import org.ops4j.pax.exam.Constants;
import org.ops4j.pax.exam.CoreOptions;
import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.options.DefaultCompositeOption;
import org.ops4j.pax.exam.options.MavenArtifactProvisionOption;
import org.ops4j.pax.exam.util.PathUtils;

import org.opendaylight.controller.test.sal.binding.it.TestHelper;

import org.opendaylight.controller.sal.utils.GlobalConstants;

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
     * Group ID for Apache Commons bundles.
     */
    private static final String  APACHE_COMMON = "org.apache.commons";

    /**
     * Group ID for Apache Felix bundles.
     */
    private static final String  APACHE_FELIX = "org.apache.felix";

    /**
     * Group ID for Equinox SDK bundles.
     */
    private static final String  EQUINOX_SDK = "equinoxSDK381";

    /**
     * Group ID for eclipselink bundles.
     */
    private static final String  ECLIPSELINK = "eclipselink";

    /**
     * Group ID for GeminiWeb bundles.
     */
    private static final String  GEMINIWEB = "geminiweb";

    /**
     * Group ID for Netty bundles.
     */
    private static final String  NETTY = "io.netty";

    /**
     * Group ID for Orbit bundles.
     */
    private static final String  ORBIT = "orbit";

    /**
     * Group ID for Jackson Core bundles.
     */
    private static final String  JACKSON_CORE = "com.fasterxml.jackson.core";

    /**
     * Group ID for Jackson JAX-RS bundles.
     */
    private static final String  JACKSON_JAXRS = "com.fasterxml.jackson.jaxrs";

    /**
     * Group ID for Jackson module bundles.
     */
    private static final String  JACKSON_MODULE =
        "com.fasterxml.jackson.module";

    /**
     * Group ID for Jettison bundles.
     */
    private static final String  JETTISON = "org.codehaus.jettison";

    /**
     * Group ID for Jersey bundles.
     */
    private static final String  JERSEY = "com.sun.jersey";

    /**
     * Group ID for Pax Exam bundles.
     */
    private static final String  PAX_EXAM = "org.ops4j.pax.exam";

    /**
     * Group ID for VTN bundles.
     */
    private static final String  VTN = "org.opendaylight.vtn";

    /**
     * Group ID for MD-SAL application bundles in the controller.
     */
    private static final String  CONTROLLER_MD = TestHelper.CONTROLLER + ".md";

    /**
     * Group ID for third-party bundles in the controller.
     */
    private static final String  CONTROLLER_THIRD_PARTY =
        TestHelper.CONTROLLER + ".thirdparty";

    /**
     * Group ID for openflowplugin.
     */
    private static final String  OPENFLOW = "org.opendaylight.openflowplugin";

    /**
     * Group ID for YANG models provided by openflowplugin.
     */
    private static final String  OPENFLOW_MODEL =
        "org.opendaylight.openflowplugin.model";

    /**
     * Group ID for NSF applications provided by openflowplugin.
     */
    private static final String  OPENFLOW_APPS =
        "org.opendaylight.openflowplugin.applications";

    /**
     * Group ID for legacy applications provided by openflowplugin.
     */
    private static final String  OPENFLOW_LEGACY =
        "org.opendaylight.openflowplugin.legacy";

    /**
     * Group ID for neutron bundles.
     */
    private static final String  NEUTRON = "org.opendaylight.neutron";

    /**
     * Group ID for OVSDB bundles.
     */
    private static final String  OVSDB = "org.opendaylight.ovsdb";

    /**
     *  Group ID for Spring Framework bundles.
     */
    private static final String  SPRING_FRAMEWORK = "org.springframework";

    /**
     *  Group ID for Spring Framework security bundles.
     */
    private static final String  SPRING_FRAMEWORK_SECURITY =
        SPRING_FRAMEWORK + ".security";

    /**
     * The name of the system property that specifies the configuration file
     * for the config subsystem.
     */
    private static final String PROP_FILE_STORAGE =
        "netconf.config.persister.1.properties.fileStorage";

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
        return CoreOptions.mavenBundle(group, name).versionAsInProject();
    }

    /**
     * Return Pax Exam option to load OSGi bundles for AD-SAL core services.
     *
     * @return  A {@link Option} instance.
     */
    public static Option adSalCoreBundles() {
        return new DefaultCompositeOption(
            // AD-SAL core bundles.
            mavenBundle(TestHelper.CONTROLLER, "configuration"),
            mavenBundle(TestHelper.CONTROLLER, "configuration.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "containermanager"),
            mavenBundle(TestHelper.CONTROLLER,
                        "containermanager.it.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "clustering.services"),
            mavenBundle(TestHelper.CONTROLLER,
                        "clustering.services-implementation"),
            mavenBundle(TestHelper.CONTROLLER, "sal"),
            mavenBundle(TestHelper.CONTROLLER, "sal.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "sal.connection"),
            mavenBundle(TestHelper.CONTROLLER, "sal.connection.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "sal.networkconfiguration"),
            mavenBundle(TestHelper.CONTROLLER,
                        "sal.networkconfiguration.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "connectionmanager"),
            mavenBundle(TestHelper.CONTROLLER,
                        "connectionmanager.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "switchmanager"),
            mavenBundle(TestHelper.CONTROLLER, "switchmanager.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "forwardingrulesmanager"),
            mavenBundle(TestHelper.CONTROLLER,
                        "forwardingrulesmanager.implementation"),
            mavenBundle(TestHelper.CONTROLLER,
                        "routing.dijkstra_implementation"),
            mavenBundle(TestHelper.CONTROLLER, "topologymanager"),
            mavenBundle(TestHelper.CONTROLLER, "clustering.stub"),
            mavenBundle(TestHelper.CONTROLLER, "forwarding.staticrouting"),
            mavenBundle(TestHelper.CONTROLLER, "statisticsmanager"),
            mavenBundle(TestHelper.CONTROLLER,
                        "statisticsmanager.implementation"),

            // hosttracker.implementation is not mandatory.
            mavenBundle(TestHelper.CONTROLLER, "hosttracker"),

            // AD-SAL requires MD-SAL compatibility bundle.
            mavenBundle(OPENFLOW_LEGACY, "sal-compatibility"),

            mavenBundle(CONTROLLER_THIRD_PARTY, "net.sf.jung2"),

            // Equinox SDK.
            mavenBundle(EQUINOX_SDK, "org.eclipse.equinox.ds"),
            mavenBundle(EQUINOX_SDK, "org.eclipse.equinox.util"),
            mavenBundle(EQUINOX_SDK, "org.eclipse.osgi.services"),
            mavenBundle(EQUINOX_SDK, "org.apache.felix.gogo.command"),
            mavenBundle(EQUINOX_SDK, "org.apache.felix.gogo.runtime"),
            mavenBundle(EQUINOX_SDK, "org.apache.felix.gogo.shell"),
            mavenBundle(EQUINOX_SDK, "org.eclipse.equinox.cm"),
            mavenBundle(EQUINOX_SDK, "org.eclipse.equinox.console"),
            mavenBundle(EQUINOX_SDK, "org.eclipse.equinox.launcher"),
            mavenBundle(EQUINOX_SDK, "javax.servlet"),

            // Bundles provied by Apache.
            mavenBundle(APACHE_COMMON, "commons-lang3"),
            mavenBundle(APACHE_FELIX, "org.apache.felix.dependencymanager"),
            mavenBundle(APACHE_FELIX,
                        "org.apache.felix.dependencymanager.shell"),
            mavenBundle(APACHE_FELIX, "org.apache.felix.fileinstall"),

            // Jackson bundles.
            mavenBundle(JACKSON_CORE, "jackson-annotations"),
            mavenBundle(JACKSON_CORE, "jackson-core"),
            mavenBundle(JACKSON_CORE, "jackson-databind"),

            // Pax Exam bundles.
            CoreOptions.mavenBundle(PAX_EXAM, "pax-exam-container-native"),
            CoreOptions.mavenBundle(PAX_EXAM, "pax-exam-junit4"),
            CoreOptions.mavenBundle(PAX_EXAM, "pax-exam-link-mvn"),
            CoreOptions.mavenBundle("org.ops4j.pax.url", "pax-url-aether"),

            // Miscellaneous.
            mavenBundle("org.jboss.spec.javax.transaction",
                        "jboss-transaction-api_1.1_spec"),
            mavenBundle(ECLIPSELINK, "javax.resource"),
            mavenBundle("commons-net", "commons-net"),
            mavenBundle("com.google.code.gson", "gson"));
    }

    /**
     * Return Pax Exam option to load OSGi bundles for common integration test.
     *
     * @return  A {@link Option} instance.
     */
    public static Option vtnManagerCommonBundles() {
        // Create configuration directory.
        File startupDir = new File(GlobalConstants.STARTUPHOME.toString());
        if (startupDir.exists()) {
            delete(startupDir);
        }

        File configDir = createPath(startupDir,
                                    GlobalConstants.DEFAULT.toString(),
                                    "vtn", "CONFIG");
        configDir.mkdirs();

        // Install common configuration files to the test environment.
        String vtnConfigXml = "vtn-config.xml";
        File dst = new File(configDir, vtnConfigXml);
        copyFile(getResource(vtnConfigXml), dst);

        String baseDir = PathUtils.getBaseDir();
        String ctlrXml = "controller.xml";
        File tmpDir = createPath(baseDir, "target", "it-tmp");
        dst = new File(tmpDir, ctlrXml);
        copyFile(getResource(ctlrXml), dst);
        String ctlrXmlPath = dst.getAbsolutePath();

        String tmpPath = tmpDir.getAbsolutePath();
        File storageDir = new File(tmpDir, "osgi-storage");
        if (storageDir.exists()) {
            delete(storageDir);
        }
        String storagePath = storageDir.getAbsolutePath();

        File resDir = createPath(baseDir, "src", "test", "resources");
        String logbackXml = new File(resDir, "logback.xml").toURI().toString();
        String serverXml = new File(resDir, "tomcat-server.xml").getPath();

        return new DefaultCompositeOption(
            systemProperty("osgi.console").value("2401"),
            systemProperty("logback.configurationFile").value(logbackXml),
            systemProperty("osgi.bundles.defaultStartLevel").value("4"),
            systemProperty("org.eclipse.gemini.web.tomcat.config.path").value(
                serverXml),

            // Set fail if unresolved bundle present.
            systemProperty("pax.exam.osgi.unresolved.fail").value("true"),

            // Use own working directory in order to prevent from creating
            // temporary files in /tmp.
            workingDirectory(tmpPath),
            frameworkProperty("org.osgi.framework.storage").value(storagePath),

            // Set the systemPackages (used by clustering and config subsystem)
            systemPackages("sun.reflect", "sun.reflect.misc", "sun.misc",
                           "sun.nio.ch"),

            // SLF4J
            mavenBundle(SLF4J, "jcl-over-slf4j"),
            mavenBundle(SLF4J, "slf4j-api"),
            mavenBundle(SLF4J, "log4j-over-slf4j"),

            // Logback
            mavenBundle(LOGBACK, "logback-core"),
            mavenBundle(LOGBACK, "logback-classic"),

            // Load MD-SAL bundles.
            TestHelper.mdSalCoreBundles(),
            TestHelper.bindingAwareSalBundles(),
            TestHelper.configMinumumBundles(),

            // Override the location of the MD-SAL module configuration file.
            systemProperty(PROP_FILE_STORAGE).value(ctlrXmlPath),

            // Load AD-SAL bundles.
            adSalCoreBundles(),

            // Load YANG models.
            TestHelper.baseModelBundles(),
            mavenBundle(TestHelper.CONTROLLER_MODELS, "model-inventory"),
            mavenBundle(TestHelper.CONTROLLER_MODELS, "model-topology"),
            mavenBundle(TestHelper.YANGTOOLS_MODELS, "ietf-topology"),
            mavenBundle(TestHelper.YANGTOOLS_MODELS,
                        "ietf-yang-types-20130715"),
            mavenBundle(OPENFLOW_MODEL, "model-flow-base"),
            mavenBundle(OPENFLOW_MODEL, "model-flow-service"),
            mavenBundle(OPENFLOW_MODEL, "model-flow-statistics"),

            // Load OpenFlow service applications.
            mavenBundle(OPENFLOW, "openflowplugin-common"),
            mavenBundle(OPENFLOW_APPS, "inventory-manager"),
            mavenBundle(OPENFLOW_APPS, "topology-manager"),

            // VTN Manager bundels.
            mavenBundle(VTN, "manager"),
            mavenBundle(VTN, "manager.model"),
            mavenBundle(VTN, "manager.implementation"),
            mavenBundle(VTN, "manager.neutron"),
            mavenBundle(VTN, "manager.it.ofmock"),
            mavenBundle(VTN, "manager.it.util"),

            // Neutron bundles.
            mavenBundle(NEUTRON, "neutron-spi"),
            mavenBundle(NEUTRON, "transcriber"),
            mavenBundle(NEUTRON, "model"),

            // Netty bundles.
            mavenBundle(NETTY, "netty-buffer"),
            mavenBundle(NETTY, "netty-codec"),
            mavenBundle(NETTY, "netty-common"),
            mavenBundle(NETTY, "netty-handler"),
            mavenBundle(NETTY, "netty-transport"),

            // OVSDB bundles.
            mavenBundle(OVSDB, "library"),
            mavenBundle(OVSDB, "plugin"),
            mavenBundle(OVSDB, "ovsdb-plugin-compatibility-layer"),
            mavenBundle(OVSDB, "schema.openvswitch"),
            mavenBundle(OVSDB, "schema.hardwarevtep"),

            // Load JUnit and Mockito bundles.
            TestHelper.junitAndMockitoBundles());
    }

    /**
     * Return Pax Exam option to load OSGi bundles for northbound API tests.
     *
     * @return  A {@link Option} instance.
     */
    public static Option vtnManagerNorthboundBundles() {
        int sysLevel = Constants.START_LEVEL_SYSTEM_BUNDLES;

        return new DefaultCompositeOption(
            vtnManagerCommonBundles(),

            // Required by Tomcat.
            systemProperty("osgi.compatibility.bootdelegation").value("true"),

            // Jackson bundles.
            mavenBundle(JACKSON_JAXRS, "jackson-jaxrs-json-provider"),
            mavenBundle(JACKSON_JAXRS, "jackson-jaxrs-base"),
            mavenBundle(JACKSON_MODULE, "jackson-module-jaxb-annotations"),

            // Jettison bundles.
            mavenBundle(JETTISON, "jettison"),

            // Equinox SDK.
            mavenBundle(EQUINOX_SDK, "javax.servlet.jsp"),

            // GeminiWeb bundles.
            mavenBundle(GEMINIWEB, "org.eclipse.gemini.web.core"),
            mavenBundle(GEMINIWEB, "org.eclipse.gemini.web.extender"),
            mavenBundle(GEMINIWEB, "org.eclipse.gemini.web.tomcat"),
            mavenBundle(GEMINIWEB, "org.eclipse.virgo.util.common"),
            mavenBundle(GEMINIWEB, "org.eclipse.virgo.util.io"),
            mavenBundle(GEMINIWEB, "org.eclipse.virgo.util.math"),
            mavenBundle(GEMINIWEB, "org.eclipse.virgo.util.osgi"),
            mavenBundle(GEMINIWEB, "org.eclipse.virgo.util.osgi.manifest"),
            mavenBundle(GEMINIWEB, "org.eclipse.virgo.util.parser.manifest"),

            // Orbit bundles.
            mavenBundle(ORBIT, "javax.activation"),
            mavenBundle(ORBIT, "javax.annotation"),
            mavenBundle(ORBIT, "javax.ejb"),
            mavenBundle(ORBIT, "javax.el"),
            mavenBundle(ORBIT, "javax.mail.glassfish"),
            mavenBundle(ORBIT, "javax.xml.rpc"),
            mavenBundle(ORBIT, "org.apache.catalina"),

            // These are bundle fragments that can't be started on its own.
            mavenBundle(ORBIT, "org.apache.catalina.ha").noStart(),
            mavenBundle(ORBIT, "org.apache.catalina.tribes").noStart(),
            mavenBundle(ORBIT, "org.apache.coyote").noStart(),
            mavenBundle(ORBIT, "org.apache.jasper").noStart(),

            mavenBundle(ORBIT, "org.apache.el"),
            mavenBundle(ORBIT, "org.apache.juli.extras"),
            mavenBundle(ORBIT, "org.apache.tomcat.api"),
            mavenBundle(ORBIT, "org.apache.tomcat.util").noStart(),
            mavenBundle(ORBIT, "javax.servlet.jsp.jstl"),
            mavenBundle(ORBIT, "javax.servlet.jsp.jstl.impl"),

            // Spring Framework bundles.
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.asm"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.aop"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.context"),
            mavenBundle(SPRING_FRAMEWORK,
                        "org.springframework.context.support"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.core"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.beans"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.expression"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.web"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.web.servlet"),
            mavenBundle(SPRING_FRAMEWORK, "org.springframework.transaction"),
            mavenBundle(SPRING_FRAMEWORK_SECURITY, "spring-security-config"),
            mavenBundle(SPRING_FRAMEWORK_SECURITY, "spring-security-core"),
            mavenBundle(SPRING_FRAMEWORK_SECURITY, "spring-security-web"),
            mavenBundle(SPRING_FRAMEWORK_SECURITY, "spring-security-taglibs"),

            // Miscellaneous.
            mavenBundle("org.ow2.asm", "asm-all"),
            mavenBundle("org.ow2.chameleon.management", "chameleon-mbeans"),
            mavenBundle("org.aopalliance", "com.springsource.org.aopalliance"),
            mavenBundle("commons-io", "commons-io"),
            mavenBundle("commons-fileupload", "commons-fileupload"),
            mavenBundle("commons-codec", "commons-codec"),
            mavenBundle("virgomirror",
                        "org.eclipse.jdt.core.compiler.batch"),
            mavenBundle(ECLIPSELINK, "javax.persistence"),

            // AD-SAL NorthBound API bundles.
            mavenBundle(TestHelper.CONTROLLER, "usermanager"),
            mavenBundle(TestHelper.CONTROLLER, "usermanager.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "security").noStart(),
            mavenBundle(TestHelper.CONTROLLER, "bundlescanner"),
            mavenBundle(TestHelper.CONTROLLER, "bundlescanner.implementation"),
            mavenBundle(TestHelper.CONTROLLER, "commons.northbound"),

            mavenBundle(CONTROLLER_THIRD_PARTY,
                        "com.sun.jersey.jersey-servlet"),

            // VTN Manager bundels.
            mavenBundle(VTN, "manager.northbound"),

            // Jersey needs to be started before the northbound application
            // bundles, using a lower start level.
            mavenBundle(JERSEY, "jersey-client"),
            mavenBundle(JERSEY, "jersey-server").startLevel(sysLevel),
            mavenBundle(JERSEY, "jersey-core").startLevel(sysLevel));
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
     * @param parent  Path to the parent directory.
     * @param comps   Path components in a file path.
     *                {@code null} must not be passed.
     * @return  A {@link File} instance associated with the given path.
     */
    public static File createPath(String parent, String ... comps) {
        return createPath(new File(parent), comps);
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
}
