/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.any;
import static org.mockito.Matchers.anyBoolean;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.runners.MockitoJUnitRunner;
import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.WriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import com.google.common.util.concurrent.CheckedFuture;
/**
 * Unit test for class {@link MdsalUtils}
 *
 */
@RunWith(MockitoJUnitRunner.class)
@SuppressWarnings({ "unchecked", "rawtypes" })
public class MdsalUtilsTest {

    /**
     * MdsalUtils mock object.
     */
    @InjectMocks private MdsalUtils mdsalUtils;

    /**
     * DataBroker mock object.
     */
    @Mock private DataBroker databroker;

    /**
     * Test method for
     * {@link MdsalUtils#delete(LogicalDatastoreType, InstanceIdentifier<D>)}.
     */
    @Test
    public void testDelete() {
        WriteTransaction writeTransaction = mock(WriteTransaction.class);
        when(databroker.newWriteOnlyTransaction()).thenReturn(writeTransaction);
        CheckedFuture<Void, TransactionCommitFailedException> future = mock(CheckedFuture.class);
        when(writeTransaction.submit()).thenReturn(future);

        boolean result = mdsalUtils.delete(LogicalDatastoreType.CONFIGURATION, mock(InstanceIdentifier.class));

        verify(writeTransaction, times(1)).delete(any(LogicalDatastoreType.class), any(InstanceIdentifier.class));
        verify(writeTransaction, times(1)).submit();

        assertTrue("Error, the delete transaction failed", result);
    }

    /**
     * Test method for
     * {@link MdsalUtils#merge(LogicalDatastoreType, InstanceIdentifier<D>, D)}.
     */
    @Test
    public void testMerge() {
        WriteTransaction writeTransaction = mock(WriteTransaction.class);
        when(databroker.newWriteOnlyTransaction()).thenReturn(writeTransaction);
        CheckedFuture<Void, TransactionCommitFailedException> future = mock(CheckedFuture.class);
        when(writeTransaction.submit()).thenReturn(future);

        boolean result = mdsalUtils.merge(LogicalDatastoreType.CONFIGURATION, mock(InstanceIdentifier.class), mock(DataObject.class));

        verify(writeTransaction, times(1)).merge(any(LogicalDatastoreType.class), any(InstanceIdentifier.class), any(DataObject.class), anyBoolean());
        verify(writeTransaction, times(1)).submit();

        assertTrue("Error, the merge transaction failed", result);
    }

    /**
     * Test method for
     * {@link MdsalUtils#put(LogicalDatastoreType, InstanceIdentifier<D>, D)}.
     */
    @Test
    public void testPut() {
        WriteTransaction writeTransaction = mock(WriteTransaction.class);
        when(databroker.newWriteOnlyTransaction()).thenReturn(writeTransaction);
        CheckedFuture<Void, TransactionCommitFailedException> future = mock(CheckedFuture.class);
        when(writeTransaction.submit()).thenReturn(future);

        boolean result = mdsalUtils.put(LogicalDatastoreType.CONFIGURATION, mock(InstanceIdentifier.class), mock(DataObject.class));

        verify(writeTransaction, times(1)).put(any(LogicalDatastoreType.class), any(InstanceIdentifier.class), any(DataObject.class), anyBoolean());
        verify(writeTransaction, times(1)).submit();

        assertTrue("Error, the put transaction failed", result);
    }
}
