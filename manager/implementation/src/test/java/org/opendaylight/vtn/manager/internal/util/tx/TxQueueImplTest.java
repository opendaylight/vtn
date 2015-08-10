/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.tx;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;

/**
 * JUnit test for {@link TxQueueImpl}
 */
public class TxQueueImplTest extends TestBase {

    /**
     * To test start method in {@link TxQueueImpl}.
     */
    @Test
    public void testStart() {
        try {
            // Set up mock-up of VTNManagerProvider
            VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
            TxQueueImpl impl = new TxQueueImpl("VTN Main" , provider);
            impl.start();
            impl.close();
        } catch (Exception exception) {
            Assert.fail("Exception thrown in TxQueueImpl start method");
        }
    }

    /**
     * To test post method in {@link TxQueueImpl}.
     * @param <T>
     */
    @Test
    public <T> void testPost() {
        try {
            // Set up mock-up of VTNManagerProvider
            VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
            TxQueueImpl impl = new TxQueueImpl("VTN Main" , provider);
            @SuppressWarnings("unchecked")
            TxTask<T> task = Mockito.mock(TxTask.class);
            impl.post(task);
            impl.close();

            // With Transaction Queue as null
            impl.post(task);
            // With Transaction Queue as null
            impl.close();
        } catch (Exception exception) {
            Assert.fail("Exception thrown in TxQueueImpl post method");
        }
    }

    /**
     * To test postFirst method in {@link TxQueueImpl}.
     * @param <T>
     */
    @Test
    public <T> void testPostFirst() {
        try {
            // Set up mock-up of VTNManagerProvider
            VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
            TxQueueImpl impl = new TxQueueImpl("VTN Main" , provider);
            @SuppressWarnings("unchecked")
            TxTask<T> task = Mockito.mock(TxTask.class);
            impl.postFirst(task);
            impl.close();

            // With Transaction Queue as null
            impl.postFirst(task);
        } catch (Exception exception) {
            Assert.fail("Exception thrown in TxQueueImpl postFirst method");
        }
    }
}
