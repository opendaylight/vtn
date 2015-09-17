/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import org.junit.Test;

import org.mockito.Mockito;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.cond.config.VtnFlowMatch;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.flow.conditions.VtnFlowCondition;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

/**
 * JUnit test for {@link DataStoreUtils}.
 */
public class DataStoreUtilsTest extends TestBase {
    /**
     * Datastore read timeout in seconds.
     */
    private static final long  READ_TIMEOUT = 5;

    /**
     * Test case for {@link DataStoreUtils#read(ReadTransaction, LogicalDatastoreType, InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead() throws Exception {
        SalPort sport = new SalPort(1L, 2L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // In case of successful completion.
        ReadTransaction rtx = Mockito.mock(ReadTransaction.class);
        CheckedFuture<Optional<VtnPort>, ReadFailedException> f =
            doRead(vport);
        Mockito.when(rtx.read(oper, path)).thenReturn(f);
        Optional<VtnPort> res = DataStoreUtils.read(rtx, oper, path);
        assertTrue(res.isPresent());
        assertEquals(vport, res.orNull());
        Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        verifyFutureMock(f);

        rtx = Mockito.mock(ReadTransaction.class);
        f = doRead((VtnPort)null);
        Mockito.when(rtx.read(oper, path)).thenReturn(f);
        res = DataStoreUtils.read(rtx, oper, path);
        assertFalse(res.isPresent());
        assertEquals(null, res.orNull());
        Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        verifyFutureMock(f);

        // In case of read failure.
        ReadFailedException rfe = new ReadFailedException("Read failed");
        rtx = Mockito.mock(ReadTransaction.class);
        f = doRead(rfe);
        Mockito.when(rtx.read(oper, path)).thenReturn(f);
        try {
            DataStoreUtils.read(rtx, oper, path);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(rfe, e.getCause());
        }
        Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        verifyFutureMock(f);

        // In case of timeout.
        TimeoutException te = new TimeoutException("Timed out");
        rtx = Mockito.mock(ReadTransaction.class);
        f = doRead(te);
        Mockito.when(rtx.read(oper, path)).thenReturn(f);
        try {
            DataStoreUtils.read(rtx, oper, path);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
            assertEquals(te, e.getCause());
        }
        Mockito.verify(rtx, Mockito.times(1)).read(oper, path);
        verifyFutureMock(f);
    }

    /**
     * Test case for {@link DataStoreUtils#read(CheckedFuture)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testReadFuture() throws Exception {
        SalPort sport = new SalPort(10L, 20L);
        VtnPort vport = createVtnPortBuilder(sport).build();

        // In case of successful completion.
        CheckedFuture<Optional<VtnPort>, ReadFailedException> f =
            doRead(vport);
        Optional<VtnPort> res = DataStoreUtils.read(f);
        assertTrue(res.isPresent());
        assertEquals(vport, res.orNull());
        verifyFutureMock(f);

        f = doRead((VtnPort)null);
        res = DataStoreUtils.read(f);
        assertFalse(res.isPresent());
        assertEquals(null, res.orNull());
        verifyFutureMock(f);

        // In case of read failure.
        ReadFailedException rfe = new ReadFailedException("Read failed");
        f = doRead(rfe);
        try {
            DataStoreUtils.read(f);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(rfe, e.getCause());
        }
        verifyFutureMock(f);

        // In case of timeout.
        TimeoutException te = new TimeoutException("Timed out");
        f = doRead(te);
        try {
            DataStoreUtils.read(f);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
            assertEquals(te, e.getCause());
        }
        verifyFutureMock(f);
    }

    /**
     * Test case for {@link DataStoreUtils#delete(ReadWriteTransaction, LogicalDatastoreType, InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDelete() throws Exception {
        SalPort sport = new SalPort(100L, 200L);
        VtnPort vport = createVtnPortBuilder(sport).build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // The target object is present.
        ReadWriteTransaction tx = Mockito.mock(ReadWriteTransaction.class);
        CheckedFuture<Optional<VtnPort>, ReadFailedException> f =
            doRead(vport);
        Mockito.when(tx.read(oper, path)).thenReturn(f);
        assertEquals(true, DataStoreUtils.delete(tx, oper, path));
        Mockito.verify(tx, Mockito.times(1)).read(oper, path);
        Mockito.verify(tx, Mockito.times(1)).delete(oper, path);
        verifyFutureMock(f);

        // The target object is not present.
        tx = Mockito.mock(ReadWriteTransaction.class);
        f = doRead((VtnPort)null);
        Mockito.when(tx.read(oper, path)).thenReturn(f);
        assertEquals(false, DataStoreUtils.delete(tx, oper, path));
        Mockito.verify(tx, Mockito.times(1)).read(oper, path);
        Mockito.verify(tx, Mockito.never()).delete(oper, path);
        verifyFutureMock(f);

        // In case of read failure.
        ReadFailedException rfe = new ReadFailedException("Read failed");
        tx = Mockito.mock(ReadWriteTransaction.class);
        f = doRead(rfe);
        Mockito.when(tx.read(oper, path)).thenReturn(f);
        try {
            DataStoreUtils.delete(tx, oper, path);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.INTERNALERROR, e.getVtnErrorTag());
            assertEquals(rfe, e.getCause());
        }
        Mockito.verify(tx, Mockito.times(1)).read(oper, path);
        Mockito.verify(tx, Mockito.never()).delete(oper, path);
        verifyFutureMock(f);

        // In case of timeout.
        TimeoutException te = new TimeoutException("Timed out");
        tx = Mockito.mock(ReadWriteTransaction.class);
        f = doRead(te);
        Mockito.when(tx.read(oper, path)).thenReturn(f);
        try {
            DataStoreUtils.delete(tx, oper, path);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
            assertEquals(te, e.getCause());
        }
        Mockito.verify(tx, Mockito.times(1)).read(oper, path);
        Mockito.verify(tx, Mockito.never()).delete(oper, path);
        verifyFutureMock(f);
    }

    /**
     * Test case for {@link DataStoreUtils#cast(Class,InstanceIdentifier)}.
     */
    @Test
    public void testCast() {
        SalPort sport = new SalPort(1L, 10L);
        InstanceIdentifier<?> path = sport.getVtnPortIdentifier();
        InstanceIdentifier<VtnPort> portPath =
            DataStoreUtils.cast(VtnPort.class, path);
        assertEquals(path, portPath);

        List<Class<? extends DataObject>> bad = new ArrayList<>();
        Collections.addAll(bad, VtnFlowMatch.class, VtnFlowCondition.class,
                           VtnNode.class, VtnPathMap.class);
        for (Class<? extends DataObject> cls: bad) {
            assertEquals(null, DataStoreUtils.cast(cls, path));
        }
    }

    /**
     * Return a {@link CheckedFuture} instance which represents a MD-SAL
     * datastore read operation.
     *
     * @param vport  A {@link VtnPort} instance to be returned.
     * @return  A {@link CheckedFuture} instance.
     * @throws Exception  An error occurred.
     */
    private CheckedFuture<Optional<VtnPort>, ReadFailedException> doRead(
        VtnPort vport) throws Exception {
        @SuppressWarnings("unchecked")
        CheckedFuture<Optional<VtnPort>, ReadFailedException> f =
            Mockito.mock(CheckedFuture.class);
        Optional<VtnPort> opt = Optional.fromNullable(vport);
        Mockito.when(f.checkedGet(READ_TIMEOUT, TimeUnit.SECONDS)).
            thenReturn(opt);

        return f;
    }

    /**
     * Return a {@link CheckedFuture} instance which represents an aborted
     * MD-SAL datastore read operation.
     *
     * @param e  An exception to be thrown.
     * @return  A {@link CheckedFuture} instance.
     * @throws Exception  An error occurred.
     */
    private CheckedFuture<Optional<VtnPort>, ReadFailedException> doRead(
        Exception e) throws Exception {
        @SuppressWarnings("unchecked")
        CheckedFuture<Optional<VtnPort>, ReadFailedException> f =
            Mockito.mock(CheckedFuture.class);
        Mockito.doThrow(e).when(f).checkedGet(READ_TIMEOUT, TimeUnit.SECONDS);

        return f;
    }

    /**
     * Ensure that only {@link CheckedFuture#checkedGet(long, TimeUnit)} was
     * called.
     *
     * @param f  A mock-up of {@link CheckedFuture} to be tested.
     * @throws Exception  An error occurred.
     */
    private void verifyFutureMock(CheckedFuture<?, ?> f) throws Exception {
        Mockito.verify(f, Mockito.times(1)).
            checkedGet(READ_TIMEOUT, TimeUnit.SECONDS);
        Mockito.verify(f, Mockito.never()).checkedGet();
        Mockito.verify(f, Mockito.never()).get();
        Mockito.verify(f, Mockito.never()).
            get(Mockito.anyLong(), Mockito.any(TimeUnit.class));
    }
}
