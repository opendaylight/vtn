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
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link OperationalListener}.
 */
public class OperationalListenerTest extends TestBase {
    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Registration to be associated with {@link OperationalListener}.
     */
    @Mock
    private ListenerRegistration<DataTreeChangeListener<VtnConfig>>  listenerReg;

    /**
     * Reference to current configuration.
     */
    private AtomicReference<VTNConfigImpl>  currentConfig =
        new AtomicReference<>(new VTNConfigImpl());

    /**
     * An {@link OperationalListener} instance for test.
     */
    private OperationalListener  operListener;

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
        operListener = new OperationalListener(dataBroker, currentConfig);
    }

    /**
     * Test case for
     * {@link OperationalListener#OperationalListener(DataBroker,AtomicReference)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        DataTreeIdentifier<VtnConfig> ident = new DataTreeIdentifier<>(
            LogicalDatastoreType.OPERATIONAL, getPath());

        // Ensure that OperationalListener has been registered as data change
        // listener.
        ArgumentCaptor<DataTreeChangeListener> captor =
            ArgumentCaptor.forClass(DataTreeChangeListener.class);
        verify(dataBroker).registerDataTreeChangeListener(
            eq(ident), (DataTreeChangeListener<VtnConfig>)captor.capture());
        List<DataTreeChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        DataTreeChangeListener<?> cdcl = wrappers.get(0);
        assertTrue(cdcl instanceof ClusteredDataTreeChangeListener);
        assertEquals(operListener,
                     getFieldValue(cdcl, DataTreeChangeListener.class,
                                   "theListener"));

        verifyZeroInteractions(listenerReg);
    }

    /**
     * Test case for {@link OperationalListener#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(listenerReg);

        // Close the listener.
        operListener.close();
        verify(listenerReg).close();

        // Listener registrations should never be closed twice.
        operListener.close();
        verify(listenerReg).close();
    }

    /**
     * Test case for {@link OperationalListener#awaitConfig(boolean, long)}.
     *
     * <p>
     *   Do nothing if VTN configuration is already initialized.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAwaitConfig1() throws Exception {
        operListener.awaitConfig(true, 1L);
        assertEquals(null, getFieldValue(operListener, SettableVTNFuture.class,
                                         "initFuture"));

        // Should do nothing if already called.
        operListener.awaitConfig(false, 1L);
    }

    /**
     * Test case for {@link OperationalListener#awaitConfig(boolean, long)}.
     *
     * <p>
     *   VTN configuration is initialized successfully.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAwaitConfig2() throws Exception {
        @SuppressWarnings("unchecked")
        SettableVTNFuture<Void> future = (SettableVTNFuture<Void>)
            getFieldValue(operListener, SettableVTNFuture.class, "initFuture");
        assertNotNull(future);
        future.set(null);
        operListener.awaitConfig(false, 1L);
        assertEquals(null, getFieldValue(operListener, SettableVTNFuture.class,
                                         "initFuture"));

        // Should do nothing if already called.
        operListener.awaitConfig(false, 1L);
    }

    /**
     * Test case for {@link OperationalListener#awaitConfig(boolean, long)}.
     *
     * <p>
     *   Operation timed out.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAwaitConfig3() throws Exception {
        try {
            operListener.awaitConfig(false, 100L);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
        }
        assertEquals(null, getFieldValue(operListener, SettableVTNFuture.class,
                                         "initFuture"));

        // Should do nothing if already called.
        operListener.awaitConfig(false, 1L);
    }

    /**
     * Test case for
     * {@link OperationalListener#enterEvent()}.
     */
    @Test
    public void testEnterEvent() {
        assertEquals(null, operListener.enterEvent());
    }

    /**
     * Test case for {@link OperationalListener#exitEvent(Void)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExitEvent() throws Exception {
        // This should do nothing.
        operListener.exitEvent(null);
        @SuppressWarnings("unchecked")
        SettableVTNFuture<Void> future = (SettableVTNFuture<Void>)
            getFieldValue(operListener, SettableVTNFuture.class, "initFuture");
        assertNotNull(future);
        assertEquals(new VTNConfigImpl(), currentConfig.get());
    }

    /**
     * Test case for {@link OperationalListener#onCreated(Void,IdentifiedData)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnCreated() throws Exception {
        @SuppressWarnings("unchecked")
        SettableVTNFuture<Void> future = (SettableVTNFuture<Void>)
            getFieldValue(operListener, SettableVTNFuture.class, "initFuture");
        assertNotNull(future);
        assertEquals(new VTNConfigImpl(), currentConfig.get());

        EtherAddress eaddr = new EtherAddress(0x123456789abL);
        Boolean state = true;
        VtnConfig vcfg = VTNConfigImpl.
            fillDefault(new VtnConfigBuilder(), eaddr).
            setInitTimeout(5000).
            setInitState(state).
            build();
        InstanceIdentifier<VtnConfig> path = getPath();
        IdentifiedData<VtnConfig> data = new IdentifiedData<>(path, vcfg);
        operListener.onCreated(null, data);

        assertEquals(true, future.isDone());
        assertEquals(null, future.get());
        operListener.awaitConfig(false, 1L);
        assertEquals(new VTNConfigImpl(vcfg), currentConfig.get());
    }

    /**
     * Test case for {@link OperationalListener#onUpdated(Void,ChangedData)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnUpdated() throws Exception {
        @SuppressWarnings("unchecked")
        SettableVTNFuture<Void> future = (SettableVTNFuture<Void>)
            getFieldValue(operListener, SettableVTNFuture.class, "initFuture");
        assertNotNull(future);
        assertEquals(new VTNConfigImpl(), currentConfig.get());

        EtherAddress eaddr = new EtherAddress(0xaabbccddeeL);
        Boolean state = false;
        VtnConfigBuilder builder = VTNConfigImpl.
            fillDefault(new VtnConfigBuilder(), eaddr);
        VtnConfig old = builder.build();
        VtnConfig vcfg = builder.
            setInitState(state).
            setFlowModTimeout(6000).
            build();
        InstanceIdentifier<VtnConfig> path = getPath();
        ChangedData<VtnConfig> data = new ChangedData<>(path, vcfg, old);
        operListener.onUpdated(null, data);

        // initFuture should be still active.
        assertEquals(false, future.isDone());
        VTNConfigImpl cur = currentConfig.get();
        assertEquals(new VTNConfigImpl(vcfg), cur);

        state = true;
        old = vcfg;
        vcfg = builder.setInitState(state).build();
        data = new ChangedData<>(path, vcfg, old);
        operListener.onUpdated(null, data);

        assertEquals(null, future.get());
        operListener.awaitConfig(false, 1L);
        assertSame(cur, currentConfig.get());
    }

    /**
     * Test case for {@link OperationalListener#onRemoved(Void,IdentifiedData)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnRemoved() throws Exception {
        // This should do nothing.
        operListener.onRemoved(null, null);
        @SuppressWarnings("unchecked")
        SettableVTNFuture<Void> future = (SettableVTNFuture<Void>)
            getFieldValue(operListener, SettableVTNFuture.class, "initFuture");
        assertNotNull(future);
        assertEquals(new VTNConfigImpl(), currentConfig.get());
    }

    /**
     * Test case for {@link OperationalListener#isUpdated(VtnConfig,VtnConfig)}.
     */
    @Test
    public void testIsUpdated() {
        VtnConfigBuilder builder = new VtnConfigBuilder();
        VtnConfig old = builder.build();
        VtnConfig vcfg = builder.build();
        assertEquals(false, operListener.isUpdated(old, vcfg));

        // Change topology-wait.
        Integer[] topoWaits = {1, 34, 567, 999};
        for (Integer value: topoWaits) {
            vcfg = builder.setTopologyWait(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }

        // Change l2-flow-priority.
        Integer[] priorities = {1, 55, 678, 999};
        for (Integer value: priorities) {
            vcfg = builder.setL2FlowPriority(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }

        // Change flow-mod-timeout.
        Integer[] flowModTimeouts = {100, 345, 19384, 34913, 60000};
        for (Integer value: flowModTimeouts) {
            vcfg = builder.setFlowModTimeout(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }

        // Change bulk-flow-mod-timeout.
        Integer[] bulkFlowModTimeouts = {3000, 10000, 56789, 123456, 600000};
        for (Integer value: bulkFlowModTimeouts) {
            vcfg = builder.setBulkFlowModTimeout(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }

        // Change init-timeout.
        Integer[] initTimeouts = {100, 3456, 78901, 600000};
        for (Integer value: initTimeouts) {
            vcfg = builder.setInitTimeout(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }

        // Change max-redirections.
        Integer[] maxRedirections = {10, 222, 34567, 100000};
        for (Integer value: maxRedirections) {
            vcfg = builder.setMaxRedirections(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
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
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }

        // Change host-tracking.
        Boolean[] bools = {Boolean.TRUE, Boolean.FALSE};
        for (Boolean value: bools) {
            vcfg = builder.setHostTracking(value).build();
            assertEquals(true, operListener.isUpdated(old, vcfg));
            old = builder.build();
            assertEquals(false, operListener.isUpdated(old, vcfg));
        }
    }

    /**
     * Test case for
     * {@link OperationalListener#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        assertEquals(getPath(), operListener.getWildcardPath());
    }

    /**
     * Test case for {@link OperationalListener#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Logger logger = operListener.getLogger();
        assertEquals(OperationalListener.class.getName(), logger.getName());
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
