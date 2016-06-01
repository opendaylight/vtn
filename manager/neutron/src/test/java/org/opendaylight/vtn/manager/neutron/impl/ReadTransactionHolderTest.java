/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import org.junit.Test;

/**
 * JUnit test for {@link ReadTransactionHolder}.
 */
public class ReadTransactionHolderTest extends TestBase {
    /**
     * Test case for {@link ReadTransactionHolder#get()}.
     */
    @Test
    public void testGet() {
        DataBroker db = mock(DataBroker.class);
        ReadOnlyTransaction rtx = mock(ReadOnlyTransaction.class);
        ReadTransactionHolder holder = new ReadTransactionHolder(db);
        verifyNoMoreInteractions(db, rtx);

        when(db.newReadOnlyTransaction()).thenReturn(rtx);
        for (int i = 0; i < 10; i++) {
            assertSame(rtx, holder.get());
            if (i == 0) {
                verify(db).newReadOnlyTransaction();
            }
            verifyNoMoreInteractions(db, rtx);
        }
    }

    /**
     * Test case for {@link ReadTransactionHolder#close()}.
     */
    @Test
    public void testClose() {
        DataBroker db = mock(DataBroker.class);
        ReadOnlyTransaction rtx = mock(ReadOnlyTransaction.class);
        when(db.newReadOnlyTransaction()).thenReturn(rtx);
        ReadTransactionHolder holder = new ReadTransactionHolder(db);
        verifyNoMoreInteractions(db, rtx);

        holder.close();
        verifyNoMoreInteractions(db, rtx);

        holder = new ReadTransactionHolder(db);
        verifyNoMoreInteractions(db, rtx);
        assertSame(rtx, holder.get());
        verify(db).newReadOnlyTransaction();
        verifyNoMoreInteractions(db, rtx);

        for (int i = 0; i < 10; i++) {
            holder.close();
            if (i == 0) {
                verify(rtx).close();
            }
            verifyNoMoreInteractions(db, rtx);
        }
    }
}
