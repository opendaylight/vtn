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

import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.Mock;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;

import org.opendaylight.vtn.manager.internal.TestBase;

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
                 isA(OperationalListener.class), any(DataChangeScope.class))).
            thenReturn(listenerReg);
        operListener = new OperationalListener(dataBroker, currentConfig);
    }

    /**
     * Test case for
     * {@link OperationalListener#OperationalListener(DataBroker,AtomicReference)}.
     */
    @Test
    public void testConstructor() {
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;

        // Ensure that OperationalListener has been registered as data change
        // listener.
        verify(dataBroker).registerDataChangeListener(
            eq(oper), eq(getPath()), eq(operListener), eq(scope));
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
     * Test case for {@link OperationalListener#awaitConfig(long)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAwaitConfig() throws Exception {
        // In case where the configuration is not initialized.
        Boolean is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(null, is);
        long start = 0;
        long end = 0;
        long timeout = 500;
        try {
            start = System.nanoTime();
            operListener.awaitConfig(timeout);
            unexpected();
        } catch (TimeoutException e) {
            end = System.nanoTime();
        }
        assertTrue(TimeUnit.NANOSECONDS.toMillis(end - start) >= timeout);

        // In case where the configuration is initialized.
        timeout = 10000;
        EtherAddress eaddr = new EtherAddress(0x1122334455L);
        Boolean state = true;
        VtnConfig vcfg = VTNConfigImpl.
            fillDefault(new VtnConfigBuilder(), eaddr).
            setInitState(state).
            build();
        InstanceIdentifier<VtnConfig> path = getPath();
        final IdentifiedData<VtnConfig> data =
            new IdentifiedData<>(path, vcfg);
        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    unexpected();
                }
                operListener.onCreated(null, data);
            }
        };
        t.start();
        operListener.awaitConfig(timeout);
        is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(state, is);
        assertEquals(new VTNConfigImpl(vcfg), currentConfig.get());

        // In case where the configuration is already initialized.
        operListener.awaitConfig(1L);
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
        Boolean is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(null, is);
        assertEquals(new VTNConfigImpl(), currentConfig.get());
    }

    /**
     * Test case for {@link OperationalListener#onCreated(Void,IdentifiedData)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnCreated() throws Exception {
        Boolean is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(null, is);
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

        is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(state, is);
        assertEquals(new VTNConfigImpl(vcfg), currentConfig.get());
    }

    /**
     * Test case for {@link OperationalListener#onUpdated(Void,ChangedData)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testOnUpdated() throws Exception {
        Boolean is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(null, is);
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

        // initState should not be changed to false.
        is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(null, is);
        VTNConfigImpl cur = currentConfig.get();
        assertEquals(new VTNConfigImpl(vcfg), cur);

        state = true;
        old = vcfg;
        vcfg = builder.setInitState(state).build();
        data = new ChangedData<>(path, vcfg, old);
        operListener.onUpdated(null, data);

        is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(state, is);
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
        Boolean is = getFieldValue(operListener, Boolean.class, "initState");
        assertEquals(null, is);
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
