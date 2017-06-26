/*
 * Copyright (c) 2016, 2017 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.base;

import static org.junit.Assert.fail;

import static org.ops4j.pax.exam.CoreOptions.options;
import static org.ops4j.pax.exam.karaf.options.KarafDistributionOption.editConfigurationFilePut;

import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getRpcOutput;
import static org.opendaylight.vtn.manager.it.util.ModelDrivenTestBase.getVtns;
import static org.opendaylight.vtn.manager.it.util.pathmap.PathMapSet.clearPathMap;
import static org.opendaylight.vtn.manager.it.util.vnode.VTenantConfig.removeVtn;

import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Properties;

import javax.inject.Inject;

import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.rules.TestWatcher;
import org.junit.runner.Description;

import org.sonar.java.jacoco.JUnitListener;

import org.ops4j.pax.exam.Option;
import org.ops4j.pax.exam.options.BootDelegationOption;
import org.ops4j.pax.exam.options.DefaultCompositeOption;
import org.ops4j.pax.exam.options.extra.VMOption;
import org.ops4j.pax.exam.util.Filter;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.it.ofmock.DataStoreUtils;
import org.opendaylight.vtn.manager.it.util.VTNServices;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.mdsal.it.base.AbstractMdsalTestBase;
import org.opendaylight.controller.sal.binding.api.BindingAwareBroker.ProviderContext;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowConditionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.VtnFlowFilterService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.VtnMacMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.port.rev150907.VtnPortMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.VtnVlanMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.VtnPathMapService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicyService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.VtnService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacTableService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.VtnVbridgeService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.version.rev150901.VtnVersionService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.VtnVinterfaceService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalService;

/**
 * Base class for integration test using Karaf container.
 */
public abstract class KarafTestBase extends AbstractMdsalTestBase
    implements VTNServices {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(KarafTestBase.class);

    /**
     * The number of milliseconds to wait for OSGi service.
     */
    public static final long  OSGI_TIMEOUT = 600000L;

    /**
     * Group ID for VTN bundles.
     */
    public static final String  VTN = "org.opendaylight.vtn";

    /**
     * The classifier of the feature repository.
     */
    public static final String  FEATURE_CLASSIFIER = "features";

    /**
     * The type of the feature repository.
     */
    public static final String  FEATURE_TYPE = "xml";

    /**
     * Prefix for the logging property in the logging configuration file.
     */
    private static final String  LOG_PROP_PREFIX = "log4j.logger.";

    /**
     * The name of the resource that contains logging configuration.
     */
    private static final String  RES_LOG_CONFIG = "/log.properties";

    /**
     * The name of the property associated with the root logger's level in the
     * logging configuration.
     */
    private static final String  LOG_CFG_ROOT = "root";

    /**
     * The interval, in milliseconds, between VTN configuration pollings.
     */
    private static final long  CONFIG_POLL_INTERVAL = 500;

    /**
     * The upper limit of the number of VTN configuration pollings.
     */
    private static final int  CONFIG_POLL_MAX = 60;

    /**
     * JUnit rule to watch tests.
     */
    @Rule
    public final JacocoListener  jacocoListener = new JacocoListener();

    /**
     * Data broker service.
     */
    @Inject
    @Filter(timeout = OSGI_TIMEOUT)
    private DataBroker  dataBroker;

    /**
     * vtn RPC service.
     */
    private VtnService  vtnService;

    /**
     * vtn-vbridge RPC service.
     */
    private VtnVbridgeService  vbridgeService;

    /**
     * vtn-vterminal RPC service.
     */
    private VtnVterminalService  vterminalService;

    /**
     * vtn-vinterface RPC service.
     */
    private VtnVinterfaceService  vinterfaceService;

    /**
     * vtn-mac-table RPC service.
     */
    private VtnMacTableService  macTableService;

    /**
     * vtn-vlan-map RPC service.
     */
    private VtnVlanMapService  vlanMapService;

    /**
     * vtn-mac-map RPC service.
     */
    private VtnMacMapService  macMapService;

    /**
     * vtn-port-map RPC service.
     */
    private VtnPortMapService  portMapService;

    /**
     * vtn-flow-filter RPC service.
     */
    private VtnFlowFilterService  flowFilterService;

    /**
     * vtn-flow-condition RPC service.
     */
    private VtnFlowConditionService  flowCondService;

    /**
     * vtn-path-policy RPC service.
     */
    private VtnPathPolicyService  pathPolicyService;

    /**
     * vtn-path-map RPC service.
     */
    private VtnPathMapService  pathMapService;

    /**
     * vtn-version RPC service.
     */
    private VtnVersionService  versionService;

    /**
     * JUnit test watcher to invoke JUnit listener provided by
     * sonar-jacoco-listeners.
     */
    private static final class JacocoListener extends TestWatcher {
        /**
         * JUnit listener provided by sonar-jacoco-listeners.
         */
        private final JUnitListener  junitListener = new JUnitListener();

        /**
         * Construct a new instance.
         */
        private JacocoListener() {
        }

        /**
         * Invoked when the test is about to start.
         *
         * @param desc  The description about the test.
         */
        @Override
        public void starting(Description desc) {
            junitListener.testStarted(desc);
        }

        /**
         * Invoked when the test has finished.
         *
         * @param desc  The description about the test.
         */
        @Override
        public void finished(Description desc) {
            junitListener.testFinished(desc);
        }
    }

    /**
     * Called when a test suite quits.
     *
     * @throws Exception  An error occurred.
     */
    @After
    public void tearDown() throws Exception {
        // Remove all global path maps.
        clearPathMap(pathMapService, null);

        // Remove all flow conditions.
        getRpcOutput(flowCondService.clearFlowCondition());

        // Remove all path policies.
        getRpcOutput(pathPolicyService.clearPathPolicy());

        // Remove all VTNs.
        for (Vtn vtn: getVtns(this)) {
            removeVtn(vtnService, vtn.getName().getValue());
        }
    }

    /**
     * Return the data broker service.
     *
     * @return  The data broker service.
     */
    public final DataBroker getDataBroker() {
        return dataBroker;
    }

    /**
     * Return the property name that specifies the logging level.
     *
     * @param name  The name of the logger.
     * @return  The property name that specifies the logging level for the
     *          specified logger.
     */
    private String getLogPropertyName(String name) {
        return LOG_PROP_PREFIX + name;
    }

    /**
     * Append the VM option to the given Pax Exam option.
     *
     * @param opts  A {@link DefaultCompositeOption} instance.
     * @param name  The name of the system property that specifies the
     *              VM option.
     */
    private void addVmOption(DefaultCompositeOption opts, String name) {
        String arg = System.getProperty(name);
        if (arg != null && !arg.isEmpty()) {
            opts.add(new VMOption(arg));
        }
    }

    /**
     * Load the specified resource as {@link Properties}.
     *
     * @param name  The name of the resource.
     * @return  A {@link Properties} loaded from the specified resource.
     *          An empty {@link Properties} if the specified resource is not
     *          available.
     */
    private Properties loadProperties(String name) {
        Properties props = new Properties();
        try (InputStream in = getClass().getResourceAsStream(RES_LOG_CONFIG)) {
            if (in != null) {
                props.load(in);
            }
        } catch (IOException e) {
            // Ignore any error.
        }

        return props;
    }

    /**
     * Append the logging level options to the given Pax Exam option.
     *
     * @param opts  A {@link DefaultCompositeOption} instance.
     */
    private void addLogLevelOptions(DefaultCompositeOption opts) {
        Properties props = loadProperties(RES_LOG_CONFIG);

        // Configure level for the root logger.
        String root = props.getProperty(LOG_CFG_ROOT);
        if (root != null) {
            props.remove(LOG_CFG_ROOT);
            opts.add(editConfigurationFilePut(
                         ORG_OPS4J_PAX_LOGGING_CFG, "log4j.rootLogger",
                         root + ", stdout"));
        }

        // Configure level for specific logger.
        for (Iterator<Entry<Object, Object>> it = props.entrySet().iterator();
             it.hasNext();) {
            Entry<Object, Object> entry = it.next();
            String key = getLogPropertyName(entry.getKey().toString());
            Object value = entry.getValue();
            opts.add(editConfigurationFilePut(
                         ORG_OPS4J_PAX_LOGGING_CFG, key, value.toString()));
        }
    }

    /**
     * Initialize the vtn-config container.
     *
     * @throws InterruptedException  The calling thread was interrupted.
     */
    private void initVtnConfig() throws InterruptedException {
        ReadWriteTransaction tx = newReadWriteTransaction();
        InstanceIdentifier<VtnConfig> path =
            InstanceIdentifier.create(VtnConfig.class);
        boolean submitted = false;
        Integer tpwait = Integer.valueOf(0);
        Boolean ht = Boolean.FALSE;
        try {
            // - Disable topology detection wait.
            // - Disable host tracking.
            VtnConfig vcfg = new VtnConfigBuilder().
                setTopologyWait(tpwait).
                setHostTracking(ht).
                build();

            tx.merge(LogicalDatastoreType.CONFIGURATION, path, vcfg, true);
            LOG.debug("Submitting vtn-config.");
            DataStoreUtils.submit(tx);
            submitted = true;
        } finally {
            if (!submitted) {
                tx.cancel();
            }
        }

        verifyVtnConfig(path, tpwait, ht);
    }

    /**
     * Wait for the VTN configuration to be applied to the operational DS.
     *
     * @param path    Path to the vtn-config container.
     * @param tpwait  Expected value of vtn-config.topology-wait.
     * @param ht      Expected value of vtn-config.host-tracking.
     * @throws InterruptedException  The calling thread was interrupted.
     */
    private void verifyVtnConfig(InstanceIdentifier<VtnConfig> path,
                                 Integer tpwait, Boolean ht)
        throws InterruptedException {

        // Wait for the configuration to be appied to operational DS.
        VtnConfig vcfg = null;
        for (int i = 1; i <= CONFIG_POLL_MAX; i++) {
            try (ReadOnlyTransaction rtx = newReadOnlyTransaction()) {
                vcfg = DataStoreUtils.read(rtx, path).orNull();
                if (vcfg != null && tpwait.equals(vcfg.getTopologyWait()) &&
                    ht.equals(vcfg.isHostTracking())) {
                    LOG.debug("vtn-config has been initialized: count={}", i);
                    return;
                }
            }

            Thread.sleep(CONFIG_POLL_INTERVAL);
        }

        fail("vtn-config was not applied to the operational DS: " + vcfg);
    }

    // AbstractConfigTestBase

    /**
     * Return extra Pax Exam options.
     *
     * @return  An array of additional Pax Exam options.
     */
    @Override
    protected final Option[] getAdditionalOptions() {
        DefaultCompositeOption opts = new DefaultCompositeOption(
            // Share JaCoCo agent classes with the boot class loader.
            new BootDelegationOption("org.jacoco.agent.*"));

        // Add Java VM options.
        addVmOption(opts, "vtn.vm.maxHeap");
        addVmOption(opts, "vtn.vm.agent");

        // Add logging level options.
        addLogLevelOptions(opts);

        return options(opts);
    }

    // AbstractMdsalTestBase

    /**
     * Set up the test container.
     *
     * @throws Exception  An error occurred.
     */
    @Before
    @Override
    public void setup() throws Exception {
        super.setup();

        // Get VTN RPC services.
        ProviderContext sess = getSession();
        vtnService = sess.getRpcService(VtnService.class);
        vbridgeService = sess.getRpcService(VtnVbridgeService.class);
        vterminalService = sess.getRpcService(VtnVterminalService.class);
        vinterfaceService = sess.getRpcService(VtnVinterfaceService.class);
        macTableService = sess.getRpcService(VtnMacTableService.class);
        vlanMapService = sess.getRpcService(VtnVlanMapService.class);
        macMapService = sess.getRpcService(VtnMacMapService.class);
        portMapService = sess.getRpcService(VtnPortMapService.class);
        flowFilterService = sess.getRpcService(VtnFlowFilterService.class);
        flowCondService = sess.getRpcService(VtnFlowConditionService.class);
        pathPolicyService = sess.getRpcService(VtnPathPolicyService.class);
        pathMapService = sess.getRpcService(VtnPathMapService.class);
        versionService = sess.getRpcService(VtnVersionService.class);

        initVtnConfig();
    }

    // VTNServices

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnService getVtnService() {
        return vtnService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnVbridgeService getVbridgeService() {
        return vbridgeService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnVterminalService getVterminalService() {
        return vterminalService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnVinterfaceService getVinterfaceService() {
        return vinterfaceService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnMacTableService getMacTableService() {
        return macTableService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnVlanMapService getVlanMapService() {
        return vlanMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnMacMapService getMacMapService() {
        return macMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnPortMapService getPortMapService() {
        return portMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnFlowFilterService getFlowFilterService() {
        return flowFilterService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnFlowConditionService getFlowConditionService() {
        return flowCondService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnPathPolicyService getPathPolicyService() {
        return pathPolicyService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnPathMapService getPathMapService() {
        return pathMapService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final VtnVersionService getVersionService() {
        return versionService;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final ReadOnlyTransaction newReadOnlyTransaction() {
        return dataBroker.newReadOnlyTransaction();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final ReadWriteTransaction newReadWriteTransaction() {
        return dataBroker.newReadWriteTransaction();
    }
}
