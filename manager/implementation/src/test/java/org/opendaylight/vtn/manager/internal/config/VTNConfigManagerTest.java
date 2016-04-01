/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isA;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;

import com.google.common.base.Optional;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import org.mockito.Mock;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.tx.TxQueueImpl;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.clustering.Entity;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipChange;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListener;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipListenerRegistration;
import org.opendaylight.controller.md.sal.common.api.clustering.EntityOwnershipState;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link VTNConfigManager}.
 */
public class VTNConfigManagerTest extends TestBase {
    /**
     * The number of milliseconds to wait for completion of background tasks.
     */
    private static final long  TASK_TIMEOUT = 5000;

    /**
     * Default value for "topology-wait".
     */
    private static final int  DEFAULT_TOPOLOGY_WAIT = 3000;

    /**
     * Default value for "l2-flow-priority".
     */
    private static final int  DEFAULT_L2_FLOW_PRIORITY = 10;

    /**
     * Default value for "flow-mod-timeout".
     */
    private static final int  DEFAULT_FLOW_MOD_TIMEOUT = 3000;

    /**
     * Default value for "bulk-flow-mod-timeout".
     */
    private static final int  DEFAULT_BULK_FLOW_MOD_TIMEOUT = 10000;

    /**
     * Default value for "init-timeout".
     */
    private static final int  DEFAULT_INIT_TIMEOUT = 10000;

    /**
     * Default value for "max-redirections".
     */
    private static final int  DEFAULT_MAX_REDIRECTIONS = 100;

    /**
     * Mock-up of {@link VTNManagerProvider}.
     */
    @Mock
    private VTNManagerProvider  vtnProvider;

    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Registration to be associated with {@link ConfigListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  cfgListenerReg;

    /**
     * Registration to be associated with {@link OperationalListener}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  operListenerReg;

    /**
     * Registration to be associated with {@link EntityOwnershipListener}.
     */
    @Mock
    private EntityOwnershipListenerRegistration  ownerListenerReg;

    /**
     * A {@link VTNConfigManager} instance for test.
     */
    private VTNConfigManager  configManager;

    /**
     * A {@link OperationalListener} instance created by the config manager.
     */
    private OperationalListener  operListener;

    /**
     * Reference to current configuration.
     */
    private AtomicReference<VTNConfigImpl>  currentConfig;

    /**
     * Settings to be set to the current configuration.
     */
    private VTNConfigImpl  newCurrentConfig;

    /**
     * Set true if the VTN configuration has submitted.
     */
    private boolean  configSubmitted;

    /**
     * The default value for the controller's MAC address.
     */
    private EtherAddress  defaultMacAddress;

    /**
     * An entity ownership change to be notified.
     */
    private EntityOwnershipChange  ownerChange;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);

        InstanceIdentifier<VtnConfig> path = getPath();
        when(vtnProvider.getDataBroker()).thenReturn(dataBroker);
        when(dataBroker.registerDataChangeListener(
                 eq(LogicalDatastoreType.CONFIGURATION), eq(path),
                 isA(ClusteredDataChangeListener.class),
                 any(DataChangeScope.class))).
            thenReturn(cfgListenerReg);

        Answer ans = new Answer() {
            @Override
            public Object answer(InvocationOnMock inv) {
                Object[] args = inv.getArguments();
                Object cdcl = args[2];
                assertTrue(cdcl instanceof ClusteredDataChangeListener);
                try {
                    DataChangeListener dcl = getFieldValue(
                        cdcl, DataChangeListener.class, "theListener");
                    assertTrue(dcl instanceof OperationalListener);
                    setOperationalListener((OperationalListener)dcl);
                    return operListenerReg;
                } catch (Exception e) {
                    unexpected(e);
                }
                return null;
            }
        };
        when(dataBroker.registerDataChangeListener(
                 eq(LogicalDatastoreType.OPERATIONAL), eq(path),
                 isA(ClusteredDataChangeListener.class),
                 any(DataChangeScope.class))).
            thenAnswer(ans);

        Answer ownerAns = new Answer() {
            @Override
            public Object answer(InvocationOnMock inv) {
                Object[] args = inv.getArguments();
                Object type = args[0];
                Object eol = args[1];
                if (VTNEntityType.CONFIG.equals(type) &&
                    eol instanceof EntityOwnershipListener) {
                    setOwnerListener((EntityOwnershipListener)eol);
                    return ownerListenerReg;
                }

                unexpected();
                return null;
            }
        };
        when(vtnProvider.registerListener(
                 eq(VTNEntityType.CONFIG),
                 any(EntityOwnershipListener.class))).
            thenAnswer(ownerAns);
    }

    /**
     * Clean up test environment.
     */
    @After
    public void cleanUp() {
        if (configManager != null) {
            configManager.close();
        }
    }

    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   VTN configuration is not present and saved.
     * </p>
     */
    @Test
    public void testConstructor1() {
        initProvider();
        assertEquals(true, configManager.isConfigProvider());
    }

    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   VTN configuration is not present, but saved into a file.
     * </p>
     */
    @Test
    public void testConstructor2() {
        EtherAddress ea = new EtherAddress(0xdeadbeef123L);
        int nodeEdgeWait = 5000;
        int l2Priority = 100;
        int flowModTimeout = 5000;
        int bulkFlowModTimeout = 20000;
        int initTimeout = 6000;
        int maxRedirections = 1000;
        VtnConfig vcfg = new VtnConfigBuilder().
            setTopologyWait(nodeEdgeWait).
            setL2FlowPriority(l2Priority).
            setFlowModTimeout(flowModTimeout).
            setBulkFlowModTimeout(bulkFlowModTimeout).
            setInitTimeout(initTimeout).
            setMaxRedirections(maxRedirections).
            setControllerMacAddress(ea.getMacAddress()).
            build();
        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        initProvider(vconf, new EntityOwnershipState(false, false));
        assertEquals(true, configManager.isConfigProvider());

        assertEquals(nodeEdgeWait, configManager.getTopologyWait());
        assertEquals(l2Priority, configManager.getL2FlowPriority());
        assertEquals(flowModTimeout, configManager.getFlowModTimeout());
        assertEquals(bulkFlowModTimeout,
                     configManager.getBulkFlowModTimeout());
        assertEquals(initTimeout, configManager.getInitTimeout());
        assertEquals(maxRedirections, configManager.getMaxRedirections());
        assertEquals(ea, configManager.getControllerMacAddress());
    }

    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   VTN configuration is initialized by another controller in the cluster.
     * </p>
     */
    @Test
    public void testConstructor3() {
        int flowModTimeout = 4000;
        int initTimeout = 10000;
        VtnConfig vcfg = new VtnConfigBuilder().
            setFlowModTimeout(flowModTimeout).
            setInitTimeout(initTimeout).
            build();
        initConsumer(vcfg, true);
        assertEquals(false, configManager.isConfigProvider());

        assertEquals(DEFAULT_TOPOLOGY_WAIT, configManager.getTopologyWait());
        assertEquals(DEFAULT_L2_FLOW_PRIORITY,
                     configManager.getL2FlowPriority());
        assertEquals(flowModTimeout, configManager.getFlowModTimeout());
        assertEquals(DEFAULT_BULK_FLOW_MOD_TIMEOUT,
                     configManager.getBulkFlowModTimeout());
        assertEquals(initTimeout, configManager.getInitTimeout());
        assertEquals(DEFAULT_MAX_REDIRECTIONS,
                     configManager.getMaxRedirections());
        assertEquals(getDefaultMacAddress(),
                     configManager.getControllerMacAddress());
    }

    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   VTN configuration is initialized by another controller in the cluster,
     *   and the MAC address of the controller is configured.
     * </p>
     */
    @Test
    public void testConstructor4() {
        EtherAddress ea = new EtherAddress(0x987654321L);
        int maxRedirections = 9999;
        VtnConfig vcfg = new VtnConfigBuilder().
            setControllerMacAddress(ea.getMacAddress()).
            setMaxRedirections(maxRedirections).
            build();
        initConsumer(vcfg, true, new EntityOwnershipState(false, false));
        assertEquals(false, configManager.isConfigProvider());

        assertEquals(DEFAULT_TOPOLOGY_WAIT, configManager.getTopologyWait());
        assertEquals(DEFAULT_L2_FLOW_PRIORITY,
                     configManager.getL2FlowPriority());
        assertEquals(DEFAULT_FLOW_MOD_TIMEOUT,
                     configManager.getFlowModTimeout());
        assertEquals(DEFAULT_BULK_FLOW_MOD_TIMEOUT,
                     configManager.getBulkFlowModTimeout());
        assertEquals(DEFAULT_INIT_TIMEOUT, configManager.getInitTimeout());
        assertEquals(maxRedirections, configManager.getMaxRedirections());
        assertEquals(ea, configManager.getControllerMacAddress());
    }

    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   Failed to submit MD-SAL datastore transaction.
     * </p>
     */
    @Test
    public void testConstructorError1() {
        initOwner(true);

        EtherAddress ea = getDefaultMacAddress();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(dataBroker.newReadWriteTransaction()).thenReturn(tx);

        InstanceIdentifier<VtnConfig> path = getPath();
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VtnConfig vcfg = null;
        when(tx.read(cfg, path)).thenReturn(getReadResult(vcfg));
        when(tx.read(oper, path)).thenReturn(getReadResult(vcfg));
        IllegalStateException cause = new IllegalStateException("Bad state");
        when(tx.submit()).thenReturn(getSubmitFuture(cause));

        Entity ent = VTNEntityType.getGlobalEntity(VTNEntityType.CONFIG);
        EntityOwnershipState estate = new EntityOwnershipState(false, false);
        when(vtnProvider.getOwnershipState(ent)).
            thenReturn(Optional.fromNullable(estate));

        try {
            new VTNConfigManager(vtnProvider);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals("Failed to initialize global configuration.",
                         e.getMessage());
            Throwable t = e.getCause();
            assertTrue(t instanceof VTNException);
            t = t.getCause();
            assertTrue(t instanceof TransactionCommitFailedException);
            assertEquals(cause, t.getCause());
        }
    }


    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   The call of {@link OperationalListener#awaitConfig(boolean, long)} was
     *   interrupted.
     * </p>
     */
    @Test
    public void testConstructorError2() {
        initOwner(false);

        VtnConfig current = new VtnConfigBuilder().
            setFlowModTimeout(3000).
            setInitTimeout(20000).
            build();
        boolean istate = false;

        EtherAddress ea = getDefaultMacAddress();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(dataBroker.newReadWriteTransaction()).thenReturn(tx);

        InstanceIdentifier<VtnConfig> path = getPath();
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(tx.read(cfg, path)).thenReturn(getReadResult(current));
        final VtnConfig ovcfg = VTNConfigImpl.
            builder(current, ea).
            setInitState(istate).
            build();
        when(tx.read(oper, path)).thenReturn(getReadResult(ovcfg));
        setSubmitFuture(tx);

        // Start a thread to interrupt the calling thread.
        final Thread curThread = Thread.currentThread();
        Thread thr = new Thread() {
            @Override
            public void run() {
                awaitConfigSaved();
                try {
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    // Ignore interruption.
                }

                curThread.interrupt();
            }
        };
        thr.start();

        Entity ent = VTNEntityType.getGlobalEntity(VTNEntityType.CONFIG);
        EntityOwnershipState estate = null;
        when(vtnProvider.getOwnershipState(ent)).
            thenReturn(Optional.fromNullable(estate));

        try {
            new VTNConfigManager(vtnProvider);
            unexpected();
        } catch (IllegalStateException e) {
            assertEquals("Failed to synchronize the initialization.",
                         e.getMessage());
        }
    }

    /**
     * Test case for
     * {@link VTNConfigManager#VTNConfigManager(VTNManagerProvider)}.
     *
     * <p>
     *   The initialization of the VTN configuration did not complete.
     * </p>
     */
    @Test
    public void testConstructorError3() {
        VtnConfig vcfg = new VtnConfigBuilder().
            setFlowModTimeout(4000).
            setInitTimeout(100).
            build();
        initConsumer(vcfg, false, new EntityOwnershipState(false, true));

        assertEquals(false, configManager.isConfigProvider());
    }

    /**
     * Test case for {@link VTNConfigManager#close()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testClose() throws Exception {
        initProvider(null, new EntityOwnershipState(true, true));

        @SuppressWarnings("unchecked")
        AtomicReference<TxQueueImpl> qref = (AtomicReference<TxQueueImpl>)
            getFieldValue(configManager, AtomicReference.class, "configQueue");
        assertNotNull(qref);
        TxQueueImpl txq = qref.get();
        Thread txThread = getFieldValue(txq, Thread.class, "runnerThread");
        assertEquals(true, txThread.isAlive());
        verifyZeroInteractions(cfgListenerReg);
        verifyZeroInteractions(operListenerReg);

        configManager.close();
        assertEquals(null, qref.get());
        assertEquals(false, txThread.isAlive());
        verify(cfgListenerReg).close();
        verify(operListenerReg).close();

        // Do nothing if already closed.
        configManager.close();
        assertEquals(null, qref.get());
        verify(cfgListenerReg).close();
        verify(operListenerReg).close();

        configManager = null;
    }

    /**
     * Test case for {@link VTNConfigManager#initDone()}.
     *
     * <p>
     *   In case of connfiguration provider.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInitDoneProvider() throws Exception {
        initProvider();

        reset(dataBroker);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(dataBroker.newReadWriteTransaction()).thenReturn(tx);

        final CountDownLatch latch = new CountDownLatch(1);
        Answer ans = new Answer() {
            @Override
            public Object answer(InvocationOnMock inv) {
                latch.countDown();
                return getSubmitFuture();
            }
        };
        when(tx.submit()).thenAnswer(ans);

        configManager.initDone();
        assertEquals(true, latch.await(TASK_TIMEOUT, TimeUnit.MILLISECONDS));

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnConfig> path = getPath();
        VtnConfig vcfg = new VtnConfigBuilder().setInitState(true).build();
        verify(tx).merge(oper, path, vcfg, true);
    }

    /**
     * Test case for {@link VTNConfigManager#initDone()}.
     *
     * <p>
     *   In case of connfiguration consumer.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInitDoneConsumer() throws Exception {
        VtnConfig vcfg = new VtnConfigBuilder().build();
        initConsumer(vcfg, true);
        assertEquals(false, configManager.isConfigProvider());

        reset(dataBroker);
        configManager.initDone();
        sleep(100);
        verifyZeroInteractions(dataBroker);
    }

    /**
     * Test case for {@link VTNConfigManager#getTopologyWait()}.
     */
    @Test
    public void testGetTopologyWait() {
        initProvider(null, new EntityOwnershipState(false, false));
        assertEquals(DEFAULT_TOPOLOGY_WAIT, configManager.getTopologyWait());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        int[] values = {0, 10, 3000, 5000, 100000, 600000};
        for (int v: values) {
            VtnConfig vcfg = builder.setTopologyWait(v).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(v, configManager.getTopologyWait());
        }
    }

    /**
     * Test case for {@link VTNConfigManager#getL2FlowPriority()}.
     */
    @Test
    public void testGetL2FlowPriority() {
        initProvider(null, new EntityOwnershipState(true, true));
        assertEquals(DEFAULT_L2_FLOW_PRIORITY,
                     configManager.getL2FlowPriority());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        int[] values = {1, 9, 10, 300, 555, 999};
        for (int v: values) {
            VtnConfig vcfg = builder.setL2FlowPriority(v).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(v, configManager.getL2FlowPriority());
        }
    }

    /**
     * Test case for {@link VTNConfigManager#getFlowModTimeout()}.
     */
    @Test
    public void testGetFlowModTimeout() {
        initProvider(null, new EntityOwnershipState(false, false));
        assertEquals(DEFAULT_FLOW_MOD_TIMEOUT,
                     configManager.getFlowModTimeout());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        int[] values = {100, 200, 1000, 3000, 50000, 60000};
        for (int v: values) {
            VtnConfig vcfg = builder.setFlowModTimeout(v).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(v, configManager.getFlowModTimeout());
        }
    }

    /**
     * Test case for {@link VTNConfigManager#getBulkFlowModTimeout()}.
     */
    @Test
    public void testGetBulkFlowModTimeout() {
        initProvider();
        assertEquals(DEFAULT_BULK_FLOW_MOD_TIMEOUT,
                     configManager.getBulkFlowModTimeout());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        int[] values = {3000, 4000, 33333, 100000, 555555, 600000};
        for (int v: values) {
            VtnConfig vcfg = builder.setBulkFlowModTimeout(v).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(v, configManager.getBulkFlowModTimeout());
        }
    }

    /**
     * Test case for {@link VTNConfigManager#getInitTimeout()}.
     */
    @Test
    public void testGetInitTimeout() {
        initProvider(null, new EntityOwnershipState(true, true));
        assertEquals(DEFAULT_INIT_TIMEOUT, configManager.getInitTimeout());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        int[] values = {100, 200, 1000, 44444, 500000, 600000};
        for (int v: values) {
            VtnConfig vcfg = builder.setInitTimeout(v).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(v, configManager.getInitTimeout());
        }
    }

    /**
     * Test case for {@link VTNConfigManager#getMaxRedirections()}.
     */
    @Test
    public void testGetMaxRedirections() {
        initProvider(null, new EntityOwnershipState(false, false));
        assertEquals(DEFAULT_MAX_REDIRECTIONS,
                     configManager.getMaxRedirections());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        int[] values = {10, 20, 3333, 10000, 44444, 99999, 100000};
        for (int v: values) {
            VtnConfig vcfg = builder.setMaxRedirections(v).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(v, configManager.getMaxRedirections());
        }
    }

    /**
     * Test case for {@link VTNConfigManager#getControllerMacAddress()}.
     */
    @Test
    public void testGetControllerMacAddress() {
        initProvider();
        assertEquals(getDefaultMacAddress(),
                     configManager.getControllerMacAddress());

        getOperationalListener();
        VTNConfigImpl vconf = currentConfig.get();
        VtnConfigBuilder builder = new VtnConfigBuilder(vconf.toVtnConfig());
        EtherAddress[] values = {
            new EtherAddress(1L),
            new EtherAddress(0x1234567890L),
            new EtherAddress(0x00aabbccddeeL),
        };
        for (EtherAddress eaddr: values) {
            MacAddress mac = eaddr.getMacAddress();
            VtnConfig vcfg = builder.setControllerMacAddress(mac).build();
            vconf = new VTNConfigImpl(vcfg);
            currentConfig.set(vconf);
            assertEquals(eaddr, configManager.getControllerMacAddress());
        }
    }

    /**
     * Set the entity ownership listener associated with the VTN configuration.
     *
     * @param eol  An {@link EntityOwnershipListener} instance.
     */
    private void setOwnerListener(final EntityOwnershipListener eol) {
        // Start a thread to notify entity ownership change.
        Thread thr = new Thread() {
            @Override
            public void run() {
                verifyZeroInteractions(ownerListenerReg);

                // Send a dummy ownership event.
                eol.ownershipChanged(newOwnerChange(false, false, false));
                verifyZeroInteractions(ownerListenerReg);

                eol.ownershipChanged(ownerChange);
            }
        };
        thr.start();
    }

    /**
     * Construct a new entity ownership change.
     *
     * @param was  The previous ownership status.
     * @param is   The current ownership status.
     * @param has  A boolean value where the VTN configuration has owner.
     * @return  An {@link EntityOwnershipChange} instance.
     */
    private EntityOwnershipChange newOwnerChange(boolean was, boolean is,
                                                 boolean has) {
        Entity configEntity =
            VTNEntityType.getGlobalEntity(VTNEntityType.CONFIG);
        return new EntityOwnershipChange(configEntity, was, is, has);
    }

    /**
     * Initialize the entity ownership.
     *
     * @param owner  {@code true} indicates that this process is the owner
     *               of the VTN configuration.
     */
    private void initOwner(boolean owner) {
        ownerChange = newOwnerChange(false, owner, true);
    }

    /**
     * Create a {@link VTNConfigManager} instance as the configuration
     * provider.
     */
    private void initProvider() {
        initProvider(null);
    }

    /**
     * Create a {@link VTNConfigManager} instance as the configuration
     * provider.
     *
     * @param vconf  A {@link VTNConfigImpl} to be loaded.
     */
    private void initProvider(VTNConfigImpl vconf) {
        initProvider(vconf, null);
    }

    /**
     * Create a {@link VTNConfigManager} instance as the configuration
     * provider.
     *
     * @param vconf   A {@link VTNConfigImpl} to be loaded.
     * @param estate  An {@link EntityOwnershipState} for the VTN
     *                configuration.
     */
    private void initProvider(VTNConfigImpl vconf,
                              EntityOwnershipState estate) {
        initOwner(true);

        EtherAddress ea = getDefaultMacAddress();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(dataBroker.newReadWriteTransaction()).thenReturn(tx);

        InstanceIdentifier<VtnConfig> path = getPath();
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        VtnConfig vcfg = null;
        when(tx.read(cfg, path)).thenReturn(getReadResult(vcfg));
        when(tx.read(oper, path)).thenReturn(getReadResult(vcfg));
        setSubmitFuture(tx);

        VtnConfigBuilder builder;
        VtnConfig saved;
        if (vconf == null) {
            builder = VTNConfigImpl.fillDefault(new VtnConfigBuilder(), ea);
            saved = null;
        } else {
            XmlConfigFile.init();
            XmlConfigFile.save(
                XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
                vconf);
            saved = vconf.toVtnConfig();
            builder = VTNConfigImpl.builder(saved, ea);
        }
        vcfg = builder.setInitState(false).build();
        newCurrentConfig = new VTNConfigImpl(vcfg);

        Entity ent = VTNEntityType.getGlobalEntity(VTNEntityType.CONFIG);
        when(vtnProvider.getOwnershipState(ent)).
            thenReturn(Optional.fromNullable(estate));

        configManager = new VTNConfigManager(vtnProvider);
        verify(ownerListenerReg).close();

        if (vconf == null) {
            verify(tx).read(cfg, path);
        } else {
            verify(tx).put(cfg, path, saved, true);
        }

        verify(tx).put(oper, path, vcfg, true);
        verify(tx).submit();

        VTNConfigImpl loaded = XmlConfigFile.load(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
            VTNConfigImpl.class);
        assertEquals(vconf, loaded);
    }

    /**
     * Create a {@link VTNConfigManager} instance as the configuration
     * consumer.
     *
     * @param current  A {@link VtnConfig} instance which contains settings
     *                 in the config datastore.
     * @param istate   Init state to be set.
     */
    private void initConsumer(VtnConfig current, boolean istate) {
        initConsumer(current, istate, null);
    }

    /**
     * Create a {@link VTNConfigManager} instance as the configuration
     * consumer.
     *
     * @param current  A {@link VtnConfig} instance which contains settings
     *                 in the config datastore.
     * @param istate   Init state to be set.
     * @param estate   An {@link EntityOwnershipState} for the VTN
     *                 configuration.
     */
    private void initConsumer(VtnConfig current, boolean istate,
                              EntityOwnershipState estate) {
        initOwner(false);

        EtherAddress ea = getDefaultMacAddress();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(dataBroker.newReadWriteTransaction()).thenReturn(tx);

        InstanceIdentifier<VtnConfig> path = getPath();
        LogicalDatastoreType cfg = LogicalDatastoreType.CONFIGURATION;
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        when(tx.read(cfg, path)).thenReturn(getReadResult(current));
        final VtnConfig ovcfg = VTNConfigImpl.
            builder(current, ea).
            setInitState(istate).
            build();
        when(tx.read(oper, path)).thenReturn(getReadResult(ovcfg));
        setSubmitFuture(tx);
        newCurrentConfig = new VTNConfigImpl(ovcfg);

        // Start a thread to notify settings to the operational datastore
        // listener.
        Thread thr = new Thread() {
            @Override
            public void run() {
                setInitState(ovcfg);
            }
        };
        thr.start();

        Entity ent = VTNEntityType.getGlobalEntity(VTNEntityType.CONFIG);
        when(vtnProvider.getOwnershipState(ent)).
            thenReturn(Optional.fromNullable(estate));

        configManager = new VTNConfigManager(vtnProvider);
        verify(ownerListenerReg).close();

        verify(tx).read(cfg, path);
        verify(tx).read(oper, path);
        verify(tx, never()).
            put(any(LogicalDatastoreType.class), eq(path),
                any(VtnConfig.class), any(Boolean.class));
        verify(tx).submit();

        VTNConfigImpl loaded = awaitConfigSaved();
        assertEquals(new VTNConfigImpl(current), loaded);
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<VtnConfig> getPath() {
        return InstanceIdentifier.create(VtnConfig.class);
    }

    /**
     * Return the default value for the controller's MAC address.
     *
     * @return  An {@link EtherAddress} instance.
     */
    private EtherAddress getDefaultMacAddress() {
        EtherAddress eaddr = defaultMacAddress;
        if (eaddr == null) {
            eaddr = VTNConfigManager.getLocalMacAddress();
            if (eaddr == null) {
                eaddr = new VTNConfigImpl().getControllerMacAddress();
            }
            defaultMacAddress = eaddr;
        }

        return eaddr;
    }

    /**
     * Configure the submit procedure of the MD-SAL transaction.
     *
     * @param tx  The mock-up of {@link ReadWriteTransaction}.
     */
    private void setSubmitFuture(ReadWriteTransaction tx) {
        Answer ans = new Answer() {
            @Override
            public Object answer(InvocationOnMock inv) {
                setConfigSubmitted();
                return getSubmitFuture();
            }
        };

        when(tx.submit()).thenAnswer(ans);
    }

    /**
     * Invoked when the VTN configuration has been submitted.
     */
    private synchronized void setConfigSubmitted() {
        configSubmitted = true;
        notifyAll();
    }


    /**
     * Wait fot the VTN configuration to be submitted.
     */
    private synchronized void awaitConfigSubmitted() {
        if (!configSubmitted) {
            long timeout = TASK_TIMEOUT;
            long expire = System.currentTimeMillis() + timeout;
            do {
                try {
                    wait(timeout);
                } catch (InterruptedException e) {
                    unexpected(e);
                }

                timeout = expire - System.currentTimeMillis();
            } while (!configSubmitted && timeout > 0);

            assertTrue(configSubmitted);
        }
    }

    /**
     * Set operational datastore listener.
     *
     * @param opl  An {@link OperationalListener} instance.
     */
    private synchronized void setOperationalListener(OperationalListener opl) {
        operListener = opl;

        try {
            @SuppressWarnings("unchecked")
            AtomicReference<VTNConfigImpl> ref = (AtomicReference<VTNConfigImpl>)
                getFieldValue(opl, AtomicReference.class, "current");
            assertNotNull(ref);
            currentConfig = ref;
            if (newCurrentConfig != null) {
                ref.set(newCurrentConfig);
            }
        } catch (Exception e) {
            unexpected(e);
        }

        notifyAll();
    }

    /**
     * Get operational datastore listener.
     *
     * @return  An {@link OperationalListener} instance.
     */
    private synchronized OperationalListener getOperationalListener() {
        if (operListener == null) {
            long timeout = TASK_TIMEOUT;
            long expire = System.currentTimeMillis() + timeout;
            do {
                try {
                    wait(timeout);
                } catch (InterruptedException e) {
                    unexpected(e);
                }

                timeout = expire - System.currentTimeMillis();
            } while (operListener == null && timeout > 0);

            assertNotNull(operListener);
        }

        return operListener;
    }

    /**
     * Notify settings to the operational datastore listener.
     *
     * @param vcfg  A {@link VtnConfig} instance which contains settings
     *              in the operational datastore.
     */
    private void setInitState(VtnConfig vcfg) {
        OperationalListener opl = getOperationalListener();
        awaitConfigSubmitted();

        InstanceIdentifier<VtnConfig> path = getPath();
        IdentifiedData<VtnConfig> data = new IdentifiedData<>(path, vcfg);
        opl.onCreated(null, data);
    }

    /**
     * Wait for the settings to be saved into file.
     *
     * @return  A {@link VTNConfigImpl} instance constructed from the saved
     *          file.
     */
    private VTNConfigImpl awaitConfigSaved() {
        for (int i = 0; i < 30; i++) {
            VTNConfigImpl vconf = XmlConfigFile.load(
                XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
                VTNConfigImpl.class);
            if (vconf != null) {
                return vconf;
            }

            sleep(100);
        }

        return XmlConfigFile.load(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
            VTNConfigImpl.class);
    }
}
