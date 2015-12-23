/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
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
    private ListenerRegistration<DataChangeListener>  listenerReg;

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

        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 isA(ClusteredDataChangeListener.class),
                 any(DataChangeScope.class))).
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
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;

        // Ensure that OperationalListener has been registered as data change
        // listener.
        ArgumentCaptor<ClusteredDataChangeListener> captor =
            ArgumentCaptor.forClass(ClusteredDataChangeListener.class);
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(getPath()), captor.capture(), eq(scope));
        List<ClusteredDataChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        ClusteredDataChangeListener cdcl = wrappers.get(0);
        assertEquals(operListener,
                     getFieldValue(cdcl, DataChangeListener.class,
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
     * {@link OperationalListener#enterEvent(AsyncDataChangeEvent)}.
     */
    @Test
    public void testEnterEvent() {
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev = null;
        assertEquals(null, operListener.enterEvent(ev));
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
