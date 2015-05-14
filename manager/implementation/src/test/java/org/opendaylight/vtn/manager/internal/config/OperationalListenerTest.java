/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.config;

import org.junit.Test;
import java.util.concurrent.TimeoutException;
import java.util.concurrent.atomic.AtomicReference;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.mockito.Mockito;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.config.rev150209.VtnConfig;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * JUnit test for {@link OperationalListener}.
 */

public class OperationalListenerTest extends TestBase {

     /**
     * Static final Instance of Logger to perform unit testing.
     */
    private static final Logger  LOG = LoggerFactory.getLogger(OperationalListenerTest.class);

    /**
     * Instance of AtomicReference to perform unit testing.
     */
    AtomicReference<VTNConfigImpl> atomicRef = new AtomicReference<VTNConfigImpl>();

    /**
     * Instance of Void to perform unit testing.
     */
    Void vd = null;

    /**
     * Instance of DataBroker to perform unit testing.
     */
    DataBroker dataBroker = Mockito.mock(DataBroker.class);

    /**
     * Instance of VTNConfigImpl to perform unit testing.
     */
    VTNConfigImpl vtnConfigImpl1 = new VTNConfigImpl();

    /**
     * Instance of VTNConfigImpl to perform unit testing.
     */
    VTNConfigImpl vtnConfigImpl2 = new VTNConfigImpl();

    /**
     * Instance of VtnConfig to perform unit testing.
     */
    VtnConfig vtnc1 = vtnConfigImpl1.toVtnConfig();

    /**
     * Instance of VtnConfig to perform unit testing.
     */
    VtnConfig vtnc2 = vtnConfigImpl2.toVtnConfig();

    /**
     * Instance of OperationalListener to perform unit testing.
     */
    OperationalListener opnListener1 = new OperationalListener(dataBroker, atomicRef);

    /**
     * Instance of InstanceIdentifier to perform unit testing.
     */
    InstanceIdentifier<VtnConfig> path = opnListener1.getWildcardPath();

    /**
     * Instance of IdentifiedData to perform unit testing.
     */
    IdentifiedData<VtnConfig> identifiedData;

    /**
     * Instance of ChangedData to perform unit testing.
     */
    ChangedData<VtnConfig> changedData;

    /**
     * Instance of AsyncDataChangeEvent to perform unit testing.
     */
    AsyncDataChangeEvent asyncDataChangeEvent = Mockito.mock(AsyncDataChangeEvent.class);


    /**
     * Test method for
     * {@link OperationalListener#awaitConfig()}.
     */
    @Test
    public void awaitConfigTest() {

        try {
            opnListener1.awaitConfig(10L);

        } catch (InterruptedException e) {

        } catch (TimeoutException e) {

        }

    }

    /**
     * Test method for
     * {@link OperationalListener#enterEvent()}.
     */
    @Test
    public void enterEventTest() {
        assertEquals(null, opnListener1.enterEvent(asyncDataChangeEvent));
    }

    /**
     * Test method for
     * {@link OperationalListener#exitEvent()}.
     */
    @Test
    public void exitEventTest() {
        opnListener1.exitEvent(vd);
    }

    /**
     * Test method for
     * {@link OperationalListener#onCreated()}.
     */
    @Test
    public void onCreatedTest() {
        try {
            identifiedData = new IdentifiedData<VtnConfig>(path, vtnc1);
        } catch (Exception ex) {

        }

        try {
            opnListener1.onCreated(vd, identifiedData);
        } catch (Exception ex) {

        }
    }

    /**
     * Test method for
     * {@link OperationalListener#onUpdated()}.
     */
    @Test
    public void onUpdatedTest() {
        try {
            changedData = new ChangedData<VtnConfig>(path, vtnc1, vtnc2);
        } catch (Exception ex) {

        }

        try {
            opnListener1.onUpdated(vd, changedData);
        } catch (Exception ex) {

        }
    }

    /**
     * Test method for
     * {@link OperationalListener#onRemoved()}.
     */
    @Test
    public void onRemovedTest() {
        try {
            opnListener1.onRemoved(vd, identifiedData);
        } catch (Exception ex) {

        }

    }

    /**
     * Test method for
     * {@link OperationalListener#getWildcardPath()}.
     */
    @Test
    public void getWildcardPathTest() {
        try {
            assertTrue(opnListener1.getWildcardPath() instanceof InstanceIdentifier);
        } catch (Exception ex) {

        }

    }

    /**
     * Test method for
     * {@link OperationalListener#getLogger()}.
     */
    @Test
    public void getLoggerTest() {
        try {

            assertTrue(opnListener1.getLogger() instanceof Logger);
        } catch (Exception ex) {

        }
    }

}
