/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.isA;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.EnumSet;
import java.util.List;
import java.util.Set;

import javax.annotation.Nonnull;

import org.junit.Assert;
import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeService;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link DataStoreListener}.
 */
public class DataStoreListenerTest extends TestBase {
    /**
     * Stub data change listener for test.
     */
    private static class TestChangeListener
        extends DataStoreListener<VtnPort, Object> {
        /**
         * Logger instance.
         */
        private Logger  logger;

        /**
         * A wildcard path.
         */
        private final InstanceIdentifier<VtnPort>  wildcardPath =
            InstanceIdentifier.builder(VtnNodes.class).child(VtnNode.class).
            child(VtnPort.class).build();

        /**
         * A set of required event types.
         */
        private final Set<VtnUpdateType>  requiredEvents;

        /**
         * A list of data creation events.
         */
        private final List<NotifiedEvent<VtnPort>>  creationEvents =
            new ArrayList<>();

        /**
         * A list of data update events.
         */
        private final List<NotifiedEvent<VtnPort>>  updateEvents =
            new ArrayList<>();

        /**
         * A list of data removal events.
         */
        private final List<NotifiedEvent<VtnPort>>  removalEvents =
            new ArrayList<>();

        /**
         * Current event context.
         */
        private Object  context;

        /**
         * Data tree change event currently handled.
         */
        private Collection<DataTreeModification<VtnPort>>  current;

        /**
         * Construct a new instance.
         */
        private TestChangeListener() {
            this(null);
        }

        /**
         * Construct a new instance.
         *
         * @param events  A set of required event types.
         */
        private TestChangeListener(Set<VtnUpdateType> events) {
            super(VtnPort.class);
            requiredEvents = events;
        }

        /**
         * Set the current data change event.
         *
         * @param changes  The current data change events.
         * @param ctx      The event context.
         */
        private void setDataChangeEvent(
            Collection<DataTreeModification<VtnPort>> changes, Object ctx) {
            current = changes;
            context = ctx;
        }

        /**
         * Return a data tree change events currently processing.
         *
         * @return  A collection of data tree modification or {@code null}.
         */
        private Collection<DataTreeModification<VtnPort>> getEvent() {
            return current;
        }

        /**
         * Return the current event context.
         *
         * @return  The current event context or {@code null}.
         */
        private Object getContext() {
            return context;
        }

        /**
         * Return a list of creation events.
         *
         * @return  A list of creation events.
         */
        private List<NotifiedEvent<VtnPort>> getCreationEvents() {
            return creationEvents;
        }

        /**
         * Return a list of update events.
         *
         * @return  A list of update events.
         */
        private List<NotifiedEvent<VtnPort>> getUpdateEvents() {
            return updateEvents;
        }

        /**
         * Return a list of removal events.
         *
         * @return  A list of removal events.
         */
        private List<NotifiedEvent<VtnPort>> getRemovalEvents() {
            return removalEvents;
        }

        // DataStoreListener

        /**
         * {@inheritDoc}
         */
        @Override
        protected  Object enterEvent() {
            return context;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void exitEvent(Object ectx) {
            Assert.assertEquals(context, ectx);
            current = null;
            context = null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isUpdated(@Nonnull VtnPort before,
                                    @Nonnull VtnPort after) {
            return !before.equals(after);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onCreated(Object ectx, IdentifiedData<VtnPort> data) {
            NotifiedEvent<VtnPort> nev =
                new NotifiedEvent<>(data, VtnUpdateType.CREATED);
            creationEvents.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onUpdated(Object ectx, ChangedData<VtnPort> data) {
            NotifiedEvent<VtnPort> nev = new NotifiedEvent<>(data);
            updateEvents.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onRemoved(Object ectx, IdentifiedData<VtnPort> data) {
            NotifiedEvent<VtnPort> nev =
                new NotifiedEvent<>(data, VtnUpdateType.REMOVED);
            removalEvents.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected InstanceIdentifier<VtnPort> getWildcardPath() {
            return wildcardPath;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected Logger getLogger() {
            Logger log = logger;
            if (log == null) {
                log = mock(Logger.class);
                logger = log;
            }
            return log;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isRequiredEvent(@Nonnull VtnUpdateType type) {
            return (requiredEvents == null || requiredEvents.contains(type));
        }
    }

    /**
     * Test case for registration and unregistration.
     *
     * <p>
     *   Register non-clustered listener.
     * </p>
     *
     * <ul>
     *   <li>{@link DataStoreListener#registerListener(DataTreeChangeService, LogicalDatastoreType, boolean)}</li>
     *   <li>{@link DataStoreListener#close()}</li>
     * </ul>
     */
    @Test
    public void testRegistration() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<VtnPort> ident =
            new DataTreeIdentifier<>(store, path);
        DataTreeChangeService service = mock(DataTreeChangeService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<TestChangeListener> reg =
            mock(ListenerRegistration.class);
        when(service.registerDataTreeChangeListener(ident, listener)).
            thenReturn(reg);
        listener.registerListener(service, store, false);
        verify(service).registerDataTreeChangeListener(ident, listener);
        verifyNoMoreInteractions(service);
        verifyZeroInteractions(reg, logger);

        // Unregister a listener.
        // Registration should be closed only one time.
        for (int i = 0; i < 10; i++) {
            listener.close();
            if (i == 0) {
                verify(reg).close();
            }
            verifyNoMoreInteractions(service, logger, reg);
        }
    }

    /**
     * Test case for registration and unregistration.
     *
     * <p>
     *   Register clustered listener.
     * </p>
     *
     * <ul>
     *   <li>{@link DataStoreListener#registerListener(DataTreeChangeService, LogicalDatastoreType, boolean)}</li>
     *   <li>{@link DataStoreListener#close()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRegistrationCluster() throws Exception {
        TestChangeListener listener =
            new TestChangeListener(EnumSet.of(VtnUpdateType.CREATED));
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<VtnPort> ident =
            new DataTreeIdentifier<>(store, path);
        DataTreeChangeService service = mock(DataTreeChangeService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<ClusteredDataTreeChangeListener> reg =
            mock(ListenerRegistration.class);
        when(service.registerDataTreeChangeListener(
                 eq(ident), isA(ClusteredDataTreeChangeListener.class))).
            thenReturn(reg);
        listener.registerListener(service, store, true);

        ArgumentCaptor<ClusteredDataTreeChangeListener> captor =
            ArgumentCaptor.forClass(ClusteredDataTreeChangeListener.class);
        verify(service).
            registerDataTreeChangeListener(eq(ident), captor.capture());
        List<ClusteredDataTreeChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        ClusteredDataTreeChangeListener cdcl = wrappers.get(0);
        assertEquals(listener,
                     getFieldValue(cdcl, DataTreeChangeListener.class,
                                   "theListener"));
        verifyZeroInteractions(reg, logger);

        // ClusteredListener should toss received events to the actual
        // listener.
        Object ctx = new Object();
        List<NotifiedEvent<VtnPort>> created =
            Collections.singletonList(newCreationEvent(1L, 1L));
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, created);
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());
        assertEquals(Collections.<NotifiedEvent<VtnPort>>emptyList(),
                     listener.getCreationEvents());

        cdcl.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        assertEquals(created, listener.getCreationEvents());

        // Unregister a listener.
        // Registration should be closed only one time.
        for (int i = 0; i < 10; i++) {
            listener.close();
            if (i == 0) {
                verify(reg).close();
            }
            verifyNoMoreInteractions(service, logger, reg);
        }
    }


    /**
     * Test case for registration failure.
     *
     * <ul>
     *   <li>{@link DataStoreListener#registerListener(DataTreeChangeService, LogicalDatastoreType, boolean)}</li>
     * </ul>
     */
    @Test
    public void testRegistrationError() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<VtnPort> ident =
            new DataTreeIdentifier<>(store, path);
        DataTreeChangeService service = mock(DataTreeChangeService.class);
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        when(service.registerDataTreeChangeListener(ident, listener)).
            thenThrow(iae);

        String msg = null;
        try {
            listener.registerListener(service, store, false);
            unexpected();
        } catch (IllegalStateException e) {
            msg = "Failed to register data tree change listener: type=" +
                VtnPort.class.getName() + ", path=" + path;
            assertEquals(iae, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        verify(service).registerDataTreeChangeListener(ident, listener);
        verify(logger).error(msg, iae);
        verifyNoMoreInteractions(service, logger);

        // close() should do nothing.
        for (int i = 0; i < 10; i++) {
            listener.close();
            verifyNoMoreInteractions(service, logger);
        }
    }

    /**
     * Test case for close error.
     *
     * <ul>
     *   <li>{@link DataStoreListener#close()}</li>
     * </ul>
     */
    @Test
    public void testCloseError() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<VtnPort> ident =
            new DataTreeIdentifier<>(store, path);
        DataTreeChangeService service = mock(DataTreeChangeService.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<TestChangeListener> reg =
            mock(ListenerRegistration.class);
        when(service.registerDataTreeChangeListener(ident, listener)).
            thenReturn(reg);
        listener.registerListener(service, store, false);
        verify(service).
            registerDataTreeChangeListener(ident, listener);
        verifyZeroInteractions(reg, logger);

        // Unregister a listener.
        String msg = "Failed to close instance: " + reg;
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        doThrow(iae).when(reg).close();

        for (int i = 0; i < 10; i++) {
            listener.close();
            verify(reg).close();
            verify(logger).error(msg, iae);
            verifyNoMoreInteractions(service, reg, logger);
        }
    }

    /**
     * Test case for unexpected exception.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     * </ul>
     */
    @Test
    public void testException() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        IllegalStateException ise =
            new IllegalStateException("Unexpected state");
        @SuppressWarnings("unchecked")
        DataTreeModification<VtnPort> change =
            mock(DataTreeModification.class);
        Collection<DataTreeModification<VtnPort>> changes =
            Collections.singletonList(change);
        when(change.getRootPath()).thenThrow(ise);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        verify(logger).error(
            "Unexpected exception in data tree change event listener.", ise);
        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
    }

    /**
     * Test case for an empty event.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     * </ul>
     */
    @Test
    public void testEmpty() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        Collection<DataTreeModification<VtnPort>> changes =
            new ArrayList<>();
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        verifyZeroInteractions(logger);
        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
    }

    /**
     * Ensure that broken objects are ignored.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testBroken() {
        VtnPort dummy = new VtnPortBuilder().build();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        // Wildcard path should be ignored.
        InstanceIdentifier<VtnPort> wild1 = InstanceIdentifier.
            builder(VtnNodes.class).child(VtnNode.class).child(VtnPort.class).
            build();
        SalNode snode = new SalNode(1L);
        InstanceIdentifier<VtnPort> wild2 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class, snode.getVtnNodeKey()).child(VtnPort.class).
            build();
        SalPort sport = new SalPort(1L, 2L);
        InstanceIdentifier<VtnPort> wild3 = InstanceIdentifier.
            builder(VtnNodes.class).
            child(VtnNode.class).
            child(VtnPort.class, sport.getVtnPortKey()).
            build();
        Collections.addAll(
            args,
            new NotifiedEvent<>(wild1, dummy, null, VtnUpdateType.CREATED),
            new NotifiedEvent<>(wild2, dummy, dummy, VtnUpdateType.CHANGED),
            new NotifiedEvent<>(wild3, dummy, null, VtnUpdateType.REMOVED));

        // Null object should be ignored.
        long dpid = 1L;
        long port = 10L;
        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<VtnPort> null1 = sport.getVtnPortIdentifier();
        args.add(
            new NotifiedEvent<>(null1, null, null, VtnUpdateType.CREATED));

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<VtnPort> null2 = sport.getVtnPortIdentifier();
        args.add(
            new NotifiedEvent<>(null2, null, dummy, VtnUpdateType.CHANGED));

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<VtnPort> null3 = sport.getVtnPortIdentifier();
        args.add(
            new NotifiedEvent<>(null3, null, null, VtnUpdateType.REMOVED));

        List<NotifiedEvent<VtnPort>> created = new ArrayList<>();
        port++;
        NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
        args.add(nev);
        created.add(nev);

        List<NotifiedEvent<VtnPort>> updated = new ArrayList<>();
        port++;
        nev = newUpdateEvent(dpid, port);
        args.add(nev);
        updated.add(nev);

        List<NotifiedEvent<VtnPort>> removed = new ArrayList<>();
        nev = newRemovalEvent(dpid, port);
        args.add(nev);
        removed.add(nev);

        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        // Ensure that broken events were ignored.
        assertEquals(created, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());

        // Verify that broken events were logged.
        String wildMsg = "{}: Ignore wildcard path: {}";
        verify(logger).warn(wildMsg, VtnUpdateType.CREATED, wild1);
        verify(logger).warn(wildMsg, VtnUpdateType.CHANGED, wild2);
        verify(logger).warn(wildMsg, VtnUpdateType.REMOVED, wild3);

        String nullValueMsg = "{}: Null value is notified: path={}";
        verify(logger).warn(nullValueMsg, "handleTree", null1);
        verify(logger).warn(nullValueMsg, "handleTree", null2);
        verify(logger).warn(nullValueMsg, VtnUpdateType.REMOVED, null3);

        verifyNoMoreInteractions(logger);
    }

    /**
     * Ensure that all event types can be listened.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testAll() {
        List<Set<VtnUpdateType>> allSets = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> created = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> removed = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> updated = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            created.add(nev);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev;
            if ((port & 1) == 0) {
                nev = newUpdateEvent(dpid, port);
                updated.add(nev);
            } else {
                // This event must be ignored because value is not changed.
                nev = newWriteEvent(dpid, port);
            }
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newRemovalEvent(dpid, port);
            removed.add(nev);
            args.add(nev);
        }

        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        assertEquals(created, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only creation event can be listened.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testAdded() {
        List<NotifiedEvent<VtnPort>> created = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            created.add(nev);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = ((port & 1) == 0)
                ? newUpdateEvent(dpid, port)
                : newWriteEvent(dpid, port);
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            args.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required = EnumSet.of(VtnUpdateType.CREATED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(created, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only update event can be listened.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testUpdated() {
        List<NotifiedEvent<VtnPort>> updated = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev;
            if ((port & 1) == 0) {
                // This event must be ignored because value is not changed.
                nev = newWriteEvent(dpid, port);
            } else {
                nev = newUpdateEvent(dpid, port);
                updated.add(nev);
            }
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newRemovalEvent(dpid, port);
            args.add(nev);
        }

        Set<VtnUpdateType> required = EnumSet.of(VtnUpdateType.CHANGED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only removal event can be listened.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testRemoved() {
        List<NotifiedEvent<VtnPort>> removed = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = ((port & 1) == 0)
                ? newUpdateEvent(dpid, port)
                : newWriteEvent(dpid, port);
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newRemovalEvent(dpid, port);
            removed.add(nev);
            args.add(nev);
        }

        Set<VtnUpdateType> required = EnumSet.of(VtnUpdateType.REMOVED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }


    /**
     * Ensure that only creation event can be filtered out.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testIgnoreCreated() {
        List<NotifiedEvent<VtnPort>> removed = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> updated = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev;
            if ((port & 1) == 0) {
                nev = newUpdateEvent(dpid, port);
                updated.add(nev);
            } else {
                // This event must be ignored because value is not changed.
                nev = newWriteEvent(dpid, port);
            }
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newRemovalEvent(dpid, port);
            removed.add(nev);
            args.add(nev);
        }

        Set<VtnUpdateType> required =
            EnumSet.of(VtnUpdateType.CHANGED, VtnUpdateType.REMOVED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }


    /**
     * Ensure that only update event can be filtered out.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testIgnoreUpdated() {
        List<NotifiedEvent<VtnPort>> created = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> removed = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            created.add(nev);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = ((port & 1) == 0)
                ? newUpdateEvent(dpid, port)
                : newWriteEvent(dpid, port);
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newRemovalEvent(dpid, port);
            removed.add(nev);
            args.add(nev);
        }

        Set<VtnUpdateType> required =
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.REMOVED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(created, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only removal event can be filtered out.
     *
     * <ul>
     *   <li>{@link DataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link DataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testIgnoreRemoved() {
        List<NotifiedEvent<VtnPort>> created = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> updated = new ArrayList<>();
        List<NotifiedEvent<VtnPort>> args = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev = newCreationEvent(dpid, port);
            created.add(nev);
            args.add(nev);
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            NotifiedEvent<VtnPort> nev;
            if ((port & 1) == 0) {
                // This event must be ignored because value is not changed.
                nev = newWriteEvent(dpid, port);
            } else {
                nev = newUpdateEvent(dpid, port);
                updated.add(nev);
            }
            args.add(nev);
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            args.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required =
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.CHANGED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Collection<DataTreeModification<VtnPort>> changes =
            createEvent(store, args);
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        List<NotifiedEvent<VtnPort>> empty =
            Collections.<NotifiedEvent<VtnPort>>emptyList();
        assertEquals(created, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Create a mock-up of data change event.
     *
     * @param store  The type of the datastore.
     * @param args   A list of objects to be notified.
     * @return  A list of new data tree change events.
     */
    private Collection<DataTreeModification<VtnPort>> createEvent(
        LogicalDatastoreType store, List<NotifiedEvent<VtnPort>> args) {
        List<DataTreeModification<VtnPort>> changes = new ArrayList<>();
        if (args != null) {
            for (NotifiedEvent<VtnPort> nev: args) {
                InstanceIdentifier<VtnPort> path = nev.getPath();
                VtnUpdateType utype = nev.getUpdateType();
                DataObjectModification<VtnPort> mod;
                if (utype == VtnUpdateType.CREATED) {
                    mod = newDataModification(
                        VtnPort.class, ModificationType.WRITE,
                        getLastPathArgument(path), null, nev.getNewObject(),
                        null);
                } else if (utype == VtnUpdateType.CHANGED) {
                    mod = newDataModification(
                        VtnPort.class, ModificationType.WRITE,
                        getLastPathArgument(path), nev.getOldObject(),
                        nev.getNewObject(), null);
                } else {
                    mod = newDataModification(
                        VtnPort.class, ModificationType.DELETE,
                        getLastPathArgument(path), nev.getNewObject(),
                        null, null);
                }
                changes.add(newTreeModification(path, store, mod));
            }
        }

        return changes;
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the creation of
     * {@link VtnPort}.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent<VtnPort> newCreationEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort vport = createVtnPortBuilder(sport).build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent<VtnPort>(path, vport, null,
                                          VtnUpdateType.CREATED);
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the change of
     * {@link VtnPort}.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent<VtnPort> newUpdateEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort newPort = createVtnPortBuilder(sport).build();
        VtnPort oldPort = createVtnPortBuilder(sport, Boolean.FALSE, false).
            build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent<VtnPort>(path, newPort, oldPort,
                                          VtnUpdateType.CHANGED);
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the specified
     * {@link VtnPort} is not modified.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent<VtnPort> newWriteEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort newPort = createVtnPortBuilder(sport, Boolean.TRUE, true).
            build();
        VtnPort oldPort = createVtnPortBuilder(sport, Boolean.TRUE, true).
            build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent<VtnPort>(path, newPort, oldPort,
                                          VtnUpdateType.CHANGED);
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the removal of
     * {@link VtnPort}.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent<VtnPort> newRemovalEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort vport = createVtnPortBuilder(sport).build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent<VtnPort>(path, vport, null,
                                          VtnUpdateType.REMOVED);
    }
}
