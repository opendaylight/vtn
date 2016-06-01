/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.neutron.impl;

import static org.mockito.Matchers.any;
import static org.mockito.Matchers.anyBoolean;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.util.UUID;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;

import org.junit.Test;
import org.junit.runner.RunWith;

import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.runners.MockitoJUnitRunner;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.ReadOnlyTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.WriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.Networks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.Network;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.networks.rev150712.networks.attributes.networks.NetworkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.neutron.rev150712.Neutron;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev130715.Uuid;

/**
 * JUnit test for {@link MdsalUtils}.
 */
@RunWith(MockitoJUnitRunner.class)
@SuppressWarnings({ "unchecked", "rawtypes" })
public class MdsalUtilsTest extends TestBase {
    /**
     * MdsalUtils mock object.
     */
    @InjectMocks private MdsalUtils mdsalUtils;

    /**
     * DataBroker mock object.
     */
    @Mock private DataBroker dataBroker;

    /**
     * Test method for
     * {@link MdsalUtils#delete(LogicalDatastoreType, InstanceIdentifier)}.
     */
    @Test
    public void testDelete() {
        WriteTransaction writeTransaction = mock(WriteTransaction.class);
        when(dataBroker.newWriteOnlyTransaction()).thenReturn(writeTransaction);
        CheckedFuture<Void, TransactionCommitFailedException> future = mock(CheckedFuture.class);
        when(writeTransaction.submit()).thenReturn(future);

        boolean result = mdsalUtils.delete(LogicalDatastoreType.CONFIGURATION, mock(InstanceIdentifier.class));

        verify(writeTransaction, times(1)).delete(any(LogicalDatastoreType.class), any(InstanceIdentifier.class));
        verify(writeTransaction, times(1)).submit();

        assertTrue("Error, the delete transaction failed", result);
    }

    /**
     * Test method for
     * {@link MdsalUtils#merge(LogicalDatastoreType, InstanceIdentifier, DataObject)}.
     */
    @Test
    public void testMerge() {
        WriteTransaction writeTransaction = mock(WriteTransaction.class);
        when(dataBroker.newWriteOnlyTransaction()).thenReturn(writeTransaction);
        CheckedFuture<Void, TransactionCommitFailedException> future = mock(CheckedFuture.class);
        when(writeTransaction.submit()).thenReturn(future);

        boolean result = mdsalUtils.merge(LogicalDatastoreType.CONFIGURATION, mock(InstanceIdentifier.class), mock(DataObject.class));

        verify(writeTransaction, times(1)).merge(any(LogicalDatastoreType.class), any(InstanceIdentifier.class), any(DataObject.class), anyBoolean());
        verify(writeTransaction, times(1)).submit();

        assertTrue("Error, the merge transaction failed", result);
    }

    /**
     * Test method for
     * {@link MdsalUtils#put(LogicalDatastoreType, InstanceIdentifier, DataObject)}.
     */
    @Test
    public void testPut() {
        WriteTransaction writeTransaction = mock(WriteTransaction.class);
        when(dataBroker.newWriteOnlyTransaction()).thenReturn(writeTransaction);
        CheckedFuture<Void, TransactionCommitFailedException> future = mock(CheckedFuture.class);
        when(writeTransaction.submit()).thenReturn(future);

        boolean result = mdsalUtils.put(LogicalDatastoreType.CONFIGURATION, mock(InstanceIdentifier.class), mock(DataObject.class));

        verify(writeTransaction, times(1)).put(any(LogicalDatastoreType.class), any(InstanceIdentifier.class), any(DataObject.class), anyBoolean());
        verify(writeTransaction, times(1)).submit();

        assertTrue("Error, the put transaction failed", result);
    }

    /**
     * Test case for
     * {@link MdsalUtils#read(LogicalDatastoreType,InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead1() throws Exception {
        // Successful completion.
        UUID uuid = UUID.randomUUID();
        Network nw = createNetwork(uuid);
        InstanceIdentifier<Network> path = getNetworkPath(nw);
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        ReadOnlyTransaction rtx = mock(ReadOnlyTransaction.class);
        when(dataBroker.newReadOnlyTransaction()).thenReturn(rtx);
        when(rtx.read(store, path)).thenReturn(getReadResult(nw));
        Optional<Network> opt = mdsalUtils.read(store, path);
        assertEquals(nw, opt.get());
        verify(dataBroker).newReadOnlyTransaction();
        verify(rtx).read(store, path);
        verify(rtx).close();
        verifyNoMoreInteractions(dataBroker, rtx);

        // Read failed.
        reset(dataBroker);
        uuid = UUID.randomUUID();
        nw = createNetwork(uuid);
        path = getNetworkPath(nw);
        rtx = mock(ReadOnlyTransaction.class);
        when(dataBroker.newReadOnlyTransaction()).thenReturn(rtx);
        IllegalArgumentException cause = new IllegalArgumentException();
        when(rtx.read(store, path)).
            thenReturn(getReadFailure(Network.class, cause));
        opt = mdsalUtils.read(store, path);
        assertEquals(false, opt.isPresent());
        verify(dataBroker).newReadOnlyTransaction();
        verify(rtx).read(store, path);
        verify(rtx).close();
        verifyNoMoreInteractions(dataBroker, rtx);

        // Read timed out.
        reset(dataBroker);
        uuid = UUID.randomUUID();
        nw = createNetwork(uuid);
        path = getNetworkPath(nw);
        rtx = mock(ReadOnlyTransaction.class);
        when(dataBroker.newReadOnlyTransaction()).thenReturn(rtx);
        CheckedFuture<Optional<Network>, ReadFailedException> future =
            getReadTimeoutFailure(Network.class);
        when(rtx.read(store, path)).thenReturn(future);
        opt = mdsalUtils.read(store, path);
        assertEquals(false, opt.isPresent());
        verify(dataBroker).newReadOnlyTransaction();
        verify(rtx).read(store, path);
        verify(rtx).close();
        verifyNoMoreInteractions(dataBroker, rtx);
    }

    /**
     * Test case for
     * {@link MdsalUtils#read(ReadTransaction, LogicalDatastoreType,InstanceIdentifier)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRead2() throws Exception {
        // Successful completion.
        UUID uuid = UUID.randomUUID();
        Network nw = createNetwork(uuid);
        InstanceIdentifier<Network> path = getNetworkPath(nw);
        LogicalDatastoreType store = LogicalDatastoreType.CONFIGURATION;
        ReadTransaction rtx = mock(ReadTransaction.class);
        when(rtx.read(store, path)).thenReturn(getReadResult(nw));
        Optional<Network> opt = MdsalUtils.read(rtx, store, path);
        assertEquals(nw, opt.get());
        verify(rtx).read(store, path);
        verifyNoMoreInteractions(dataBroker, rtx);

        // Read failed.
        uuid = UUID.randomUUID();
        nw = createNetwork(uuid);
        path = getNetworkPath(nw);
        rtx = mock(ReadOnlyTransaction.class);
        IllegalStateException cause = new IllegalStateException();
        when(rtx.read(store, path)).
            thenReturn(getReadFailure(Network.class, cause));
        opt = MdsalUtils.read(rtx, store, path);
        assertEquals(false, opt.isPresent());
        verify(rtx).read(store, path);
        verifyNoMoreInteractions(dataBroker, rtx);

        // Read timed out.
        uuid = UUID.randomUUID();
        nw = createNetwork(uuid);
        path = getNetworkPath(nw);
        rtx = mock(ReadOnlyTransaction.class);
        CheckedFuture<Optional<Network>, ReadFailedException> future =
            getReadTimeoutFailure(Network.class);
        when(rtx.read(store, path)).thenReturn(future);
        opt = MdsalUtils.read(rtx, store, path);
        assertEquals(false, opt.isPresent());
        verify(rtx).read(store, path);
        verifyNoMoreInteractions(dataBroker, rtx);
    }

    /**
     * Create a new neutron network instance.
     *
     * @param id  The UUID for the neutron network.
     * @return  A neutron network instance.
     */
    private Network createNetwork(UUID id) {
        Uuid uuid = new Uuid(id.toString());
        return new NetworkBuilder().setUuid(uuid).build();
    }

    /**
     * Create instance identifier for the specified neutron network.
     *
     * @param nw  A {@link Network} instance.
     * @return  Instance identifier for the specified neutron network.
     */
    private InstanceIdentifier<Network> getNetworkPath(Network nw) {
        return InstanceIdentifier.
            builder(Neutron.class).
            child(Networks.class).
            child(Network.class, nw.getKey()).
            build();
    }
}
