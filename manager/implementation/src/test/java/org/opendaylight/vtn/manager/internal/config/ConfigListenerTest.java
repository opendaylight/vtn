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
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.List;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;

/**
 * JUnit test for {@link ConfigListener}.
 */
public class ConfigListenerTest extends TestBase {
    /**
     * MAC address of the controller.
     */
    private static final EtherAddress  CONTROLLER_MAC =
        new EtherAddress(0xaabbccdd876L);

    /**
     * Mock-up of {@link TxQueue}.
     */
    @Mock
    private TxQueue  txQueue;

    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Registration to be associated with {@link ConfigListener}.
     */
    @Mock
    private ListenerRegistration<DataTreeChangeListener<VtnConfig>>  listenerReg;

    /**
     * A {@link ConfigListener} instance for test.
     */
    private ConfigListener  cfgListener;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);

        Class<DataTreeIdentifier> idtype = DataTreeIdentifier.class;
        Class<ClusteredDataTreeChangeListener> cltype =
            ClusteredDataTreeChangeListener.class;
        when(dataBroker.registerDataTreeChangeListener(
                 (DataTreeIdentifier<VtnConfig>)any(idtype),
                 (DataTreeChangeListener<VtnConfig>)isA(cltype))).
            thenReturn(listenerReg);
        cfgListener = new ConfigListener(txQueue, dataBroker, CONTROLLER_MAC);
    }

    /**
     * Test case for
     * {@link ConfigListener#ConfigListener(TxQueue,DataBroker,EtherAddress)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        DataTreeIdentifier<VtnConfig> ident = new DataTreeIdentifier<>(
            LogicalDatastoreType.CONFIGURATION, getPath());

        // Ensure that ConfigListener has been registered as data tree change
        // listener.
        ArgumentCaptor<DataTreeChangeListener> captor = ArgumentCaptor.
            forClass(DataTreeChangeListener.class);
        verify(dataBroker).registerDataTreeChangeListener(
            eq(ident), (DataTreeChangeListener<VtnConfig>)captor.capture());
        List<DataTreeChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        DataTreeChangeListener<?> cdcl = wrappers.get(0);
        assertTrue(cdcl instanceof ClusteredDataTreeChangeListener);
        assertEquals(cfgListener,
                     getFieldValue(cdcl, DataTreeChangeListener.class,
                                   "theListener"));
        verifyZeroInteractions(listenerReg);
    }

    /**
     * Test case for {@link ConfigListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);

        // Close the listener.
        cfgListener.close();
        verify(listenerReg).close();

        // Listener registrations should never be closed twice.
        cfgListener.close();
        verify(listenerReg).close();
    }

    /**
     * Test case for
     * {@link ConfigListener#enterEvent()}.
     */
    @Test
    public void testEnterEvent() {
        assertEquals(null, cfgListener.enterEvent());
    }

    /**
     * Test case for {@link ConfigListener#exitEvent(Void)}.
     */
    @Test
    public void testExitEvent() {
        // This should do nothing.
        cfgListener.exitEvent(null);
    }

    /**
     * Test case for {@link ConfigListener#onCreated(Void,IdentifiedData)}.
     *
     * <p>
     *   In case where the process is the owner of VTN configuration.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnCreated() throws Exception {
        XmlConfigFile.init();
        EtherAddress eaddr = new EtherAddress(0x123456789abL);
        VtnConfigBuilder builder = new VtnConfigBuilder().
            setMaxRedirections(1000).
            setL2FlowPriority(200).
            setControllerMacAddress(eaddr.getMacAddress());
        VtnConfig vcfg = builder.build();
        InstanceIdentifier<VtnConfig> path = getPath();
        IdentifiedData<VtnConfig> data = new IdentifiedData<>(path, vcfg);
        cfgListener.onCreated(null, data);

        // Verify posted MD-SAL transaction task.
        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = (ArgumentCaptor<TxTask>)
            ArgumentCaptor.forClass(TxTask.class);
        verify(txQueue).post(captor.capture());
        List<TxTask> posted = captor.getAllValues();
        assertEquals(1, posted.size());
        TxTask task = posted.get(0);
        String taskName = ConfigListener.class.getName() + ".ConfigUpdateTask";
        assertEquals(taskName, task.getClass().getCanonicalName());

        // Execute the task.
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.CONFIG)).thenReturn(true);
        when(ctx.getProvider()).thenReturn(provider);
        assertEquals(null, task.execute(ctx, 0));

        VtnConfig ovcfg = VTNConfigImpl.fillDefault(builder, null).build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        verify(tx).merge(oper, path, ovcfg, true);
        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.CONFIG);
        verifyNoMoreInteractions(ctx, provider, tx);

        // Configuration should be saved into file.
        task.onSuccess(null, null);
        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        VTNConfigImpl loaded = XmlConfigFile.load(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
            VTNConfigImpl.class);
        assertEquals(vconf, loaded);
    }

    /**
     * Test case for {@link ConfigListener#onCreated(Void,IdentifiedData)}.
     *
     * <p>
     *   In case where the process is not the owner of VTN configuration.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnCreatedNotOwner() throws Exception {
        XmlConfigFile.init();
        XmlConfigFile.Type cfType = XmlConfigFile.Type.CONFIG;
        String cfKey = VTNConfigManager.KEY_VTN_CONFIG;
        int timeout = 120;
        VtnConfigBuilder builder = new VtnConfigBuilder().
            setMaxRedirections(777).
            setL2FlowPriority(150).
            setInitTimeout(timeout);
        timeout++;
        VtnConfig vcfg = builder.build();
        InstanceIdentifier<VtnConfig> path = getPath();
        IdentifiedData<VtnConfig> data = new IdentifiedData<>(path, vcfg);
        cfgListener.onCreated(null, data);

        // Verify posted MD-SAL transaction task.
        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = (ArgumentCaptor<TxTask>)
            ArgumentCaptor.forClass(TxTask.class);
        verify(txQueue).post(captor.capture());
        List<TxTask> posted = captor.getAllValues();
        assertEquals(1, posted.size());
        TxTask task = posted.get(0);
        String taskName = ConfigListener.class.getName() + ".ConfigUpdateTask";
        assertEquals(taskName, task.getClass().getCanonicalName());
        reset(txQueue);

        // Execute the task.
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.CONFIG)).thenReturn(false);
        when(ctx.getProvider()).thenReturn(provider);
        assertEquals(null, task.execute(ctx, 0));

        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.CONFIG);
        verifyNoMoreInteractions(ctx, provider, tx);

        // Configuration should be saved into file.
        task.onSuccess(null, null);
        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        VTNConfigImpl loaded = XmlConfigFile.load(
            cfType, cfKey, VTNConfigImpl.class);
        assertEquals(vconf, loaded);
        assertEquals(true, XmlConfigFile.delete(cfType, cfKey));
    }

    /**
     * Test case for {@link ConfigListener#onUpdated(Void,ChangedData)}.
     *
     * <p>
     *   In case where the process is the owner of VTN configuration.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnUpdated() throws Exception {
        XmlConfigFile.init();
        VtnConfigBuilder builder = new VtnConfigBuilder();
        VtnConfig old = builder.build();
        builder.setFlowModTimeout(10000).
            setL2FlowPriority(33);
        VtnConfig vcfg = builder.build();
        InstanceIdentifier<VtnConfig> path = getPath();
        ChangedData<VtnConfig> data = new ChangedData<>(path, vcfg, old);
        cfgListener.onUpdated(null, data);

        // Verify posted MD-SAL transaction task.
        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = (ArgumentCaptor<TxTask>)
            ArgumentCaptor.forClass(TxTask.class);
        verify(txQueue).post(captor.capture());
        List<TxTask> posted = captor.getAllValues();
        assertEquals(1, posted.size());
        TxTask task = posted.get(0);
        String taskName = ConfigListener.class.getName() + ".ConfigUpdateTask";
        assertEquals(taskName, task.getClass().getCanonicalName());

        // Execute the task.
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.CONFIG)).thenReturn(true);
        when(ctx.getProvider()).thenReturn(provider);
        assertEquals(null, task.execute(ctx, 0));

        VtnConfig ovcfg = VTNConfigImpl.fillDefault(builder, CONTROLLER_MAC).
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        verify(tx).merge(oper, path, ovcfg, true);
        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.CONFIG);
        verifyNoMoreInteractions(ctx, provider, tx);

        // Configuration should be saved into file.
        task.onSuccess(null, null);
        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        VTNConfigImpl loaded = XmlConfigFile.load(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
            VTNConfigImpl.class);
        assertEquals(vconf, loaded);
    }

    /**
     * Test case for {@link ConfigListener#onUpdated(Void,ChangedData)}.
     *
     * <p>
     *   In case where the process is not the owner of VTN configuration.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnUpdatedNotOwner() throws Exception {
        XmlConfigFile.init();
        XmlConfigFile.Type cfType = XmlConfigFile.Type.CONFIG;
        String cfKey = VTNConfigManager.KEY_VTN_CONFIG;
        int timeout = 4444;
        VtnConfigBuilder builder = new VtnConfigBuilder();
        VtnConfig old = builder.build();
        builder.setFlowModTimeout(10000).
            setFlowModTimeout(timeout).
            setL2FlowPriority(33);
        timeout++;
        VtnConfig vcfg = builder.build();
        InstanceIdentifier<VtnConfig> path = getPath();
        ChangedData<VtnConfig> data = new ChangedData<>(path, vcfg, old);
        cfgListener.onUpdated(null, data);

        // Verify posted MD-SAL transaction task.
        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor =
            ArgumentCaptor.forClass(TxTask.class);
        verify(txQueue).post(captor.capture());
        List<TxTask> posted = captor.getAllValues();
        assertEquals(1, posted.size());
        TxTask task = posted.get(0);
        String taskName = ConfigListener.class.getName() + ".ConfigUpdateTask";
        assertEquals(taskName, task.getClass().getCanonicalName());
        reset(txQueue);

        // Execute the task.
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.CONFIG)).thenReturn(false);
        when(ctx.getProvider()).thenReturn(provider);
        assertEquals(null, task.execute(ctx, 0));

        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.CONFIG);
        verifyNoMoreInteractions(ctx, provider, tx);

        // Configuration should be saved into file.
        task.onSuccess(null, null);
        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        VTNConfigImpl loaded = XmlConfigFile.load(
            cfType, cfKey, VTNConfigImpl.class);
        assertEquals(vconf, loaded);
        assertEquals(true, XmlConfigFile.delete(cfType, cfKey));
    }

    /**
     * Test case for {@link ConfigListener#onRemoved(Void,IdentifiedData)}.
     *
     * <p>
     *   In case where the process is the owner of VTN configuration.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnRemoved() throws Exception {
        XmlConfigFile.init();
        VtnConfigBuilder builder = new VtnConfigBuilder().
            setL2FlowPriority(111);
        VtnConfig vcfg = builder.build();
        InstanceIdentifier<VtnConfig> path = getPath();
        IdentifiedData<VtnConfig> data = new IdentifiedData<>(path, vcfg);
        cfgListener.onRemoved(null, data);

        // Verify posted MD-SAL transaction task.
        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = ArgumentCaptor.forClass(TxTask.class);
        verify(txQueue).post(captor.capture());
        List<TxTask> posted = captor.getAllValues();
        assertEquals(1, posted.size());
        TxTask task = posted.get(0);
        String taskName = ConfigListener.class.getName() + ".ConfigRemoveTask";
        assertEquals(taskName, task.getClass().getCanonicalName());

        // Execute the task.
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.CONFIG)).thenReturn(true);
        when(ctx.getProvider()).thenReturn(provider);
        assertEquals(null, task.execute(ctx, 0));

        VtnConfig ovcfg = VTNConfigImpl.
            fillDefault(new VtnConfigBuilder(), CONTROLLER_MAC).
            build();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        verify(tx).merge(oper, path, ovcfg, true);
        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.CONFIG);
        verifyNoMoreInteractions(ctx, provider, tx);

        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        XmlConfigFile.save(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG, vconf);
        VTNConfigImpl loaded = XmlConfigFile.load(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
            VTNConfigImpl.class);
        assertEquals(loaded, vconf);

        // Configuration file should be removed.
        task.onSuccess(null, null);
        loaded = XmlConfigFile.load(
            XmlConfigFile.Type.CONFIG, VTNConfigManager.KEY_VTN_CONFIG,
            VTNConfigImpl.class);
        assertEquals(null, loaded);
    }

    /**
     * Test case for {@link ConfigListener#onRemoved(Void,IdentifiedData)}.
     *
     * <p>
     *   In case where the process is not the owner of VTN configuration.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnRemovedNotOwner() throws Exception {
        XmlConfigFile.init();
        XmlConfigFile.Type cfType = XmlConfigFile.Type.CONFIG;
        String cfKey = VTNConfigManager.KEY_VTN_CONFIG;
        int timeout = 9999;
        VtnConfigBuilder builder = new VtnConfigBuilder().
            setBulkFlowModTimeout(timeout);
        timeout++;
        VtnConfig vcfg = builder.build();
        InstanceIdentifier<VtnConfig> path = getPath();
        IdentifiedData<VtnConfig> data = new IdentifiedData<>(path, vcfg);
        cfgListener.onRemoved(null, data);

        // Verify posted MD-SAL transaction task.
        @SuppressWarnings("unchecked")
            ArgumentCaptor<TxTask> captor = (ArgumentCaptor<TxTask>)
            ArgumentCaptor.forClass(TxTask.class);
        verify(txQueue).post(captor.capture());
        List<TxTask> posted = captor.getAllValues();
        assertEquals(1, posted.size());
        TxTask task = posted.get(0);
        String taskName = ConfigListener.class.getName() + ".ConfigRemoveTask";
        assertEquals(taskName, task.getClass().getCanonicalName());
        reset(txQueue);

        // Execute the task.
        TxContext ctx = mock(TxContext.class);
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.CONFIG)).thenReturn(false);
        when(ctx.getProvider()).thenReturn(provider);
        assertEquals(null, task.execute(ctx, 0));

        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.CONFIG);
        verifyNoMoreInteractions(ctx, provider, tx);

        VTNConfigImpl vconf = new VTNConfigImpl(vcfg);
        XmlConfigFile.save(cfType, cfKey, vconf);
        VTNConfigImpl loaded = XmlConfigFile.load(
            cfType, cfKey, VTNConfigImpl.class);
        assertEquals(loaded, vconf);

        // Configuration file should be removed.
        task.onSuccess(null, null);
        loaded = XmlConfigFile.load(cfType, cfKey, VTNConfigImpl.class);
        assertEquals(null, loaded);
    }

    /**
     * Test case for {@link ConfigListener#isUpdated(VtnConfig,VtnConfig)}.
     */
    @Test
    public void testIsUpdated() {
        VtnConfigBuilder builder = new VtnConfigBuilder();
        VtnConfig old = builder.build();
        VtnConfig vcfg = builder.build();
        assertEquals(false, cfgListener.isUpdated(old, vcfg));

        // Change topology-wait.
        Integer[] topoWaits = {1, 34, 567, 999};
        for (Integer value: topoWaits) {
            vcfg = builder.setTopologyWait(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change l2-flow-priority.
        Integer[] priorities = {1, 55, 678, 999};
        for (Integer value: priorities) {
            vcfg = builder.setL2FlowPriority(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change flow-mod-timeout.
        Integer[] flowModTimeouts = {100, 345, 19384, 34913, 60000};
        for (Integer value: flowModTimeouts) {
            vcfg = builder.setFlowModTimeout(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change bulk-flow-mod-timeout.
        Integer[] bulkFlowModTimeouts = {3000, 10000, 56789, 123456, 600000};
        for (Integer value: bulkFlowModTimeouts) {
            vcfg = builder.setBulkFlowModTimeout(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change init-timeout.
        Integer[] initTimeouts = {100, 3456, 78901, 600000};
        for (Integer value: initTimeouts) {
            vcfg = builder.setInitTimeout(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change max-redirections.
        Integer[] maxRedirections = {10, 222, 34567, 100000};
        for (Integer value: maxRedirections) {
            vcfg = builder.setMaxRedirections(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change controller-mac-address.
        EtherAddress[] macAddrs = {
            new EtherAddress(0x001122334455L),
            new EtherAddress(0xa0b0c0d0e0f0L),
            new EtherAddress(0x0c123abcdef9L),
        };
        for (EtherAddress eaddr: macAddrs) {
            vcfg = builder.setControllerMacAddress(eaddr.getMacAddress()).
                build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }

        // Change host-tracking.
        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};
        for (Boolean value: bools) {
            vcfg = builder.setHostTracking(value).build();
            assertEquals(true, cfgListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, cfgListener.isUpdated(old, vcfg));
        }
    }

    /**
     * Test case for
     * {@link ConfigListener#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        assertEquals(getPath(), cfgListener.getWildcardPath());
    }

    /**
     * Test case for {@link ConfigListener#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Logger logger = cfgListener.getLogger();
        assertEquals(ConfigListener.class.getName(), logger.getName());
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<VtnConfig> getPath() {
        return InstanceIdentifier.create(VtnConfig.class);
    }
}
