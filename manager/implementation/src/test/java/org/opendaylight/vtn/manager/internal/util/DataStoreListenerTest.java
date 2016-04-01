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
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import java.util.ArrayList;
import java.util.Collections;
import java.util.EnumSet;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import org.junit.Assert;
import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtils;

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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnNodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNode;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.nodes.VtnNodeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathPolicies;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

/**
 * JUnit test for {@link DataStoreListener}.
 */
public class DataStoreListenerTest extends TestBase {
    /**
     * A class used to keep notified objects.
     */
    private static class NotifiedEvent {
        /**
         * Path to the data object.
         */
        private final InstanceIdentifier<?>  path;

        /**
         * A new object.
         */
        private final DataObject  newObject;

        /**
         * An old object.
         */
        private final DataObject  oldObject;

        /**
         * Construct a new instance.
         *
         * @param p       Path to the data object.
         * @param newObj  A new object.
         * @param oldObj  An old object.
         */
        private NotifiedEvent(InstanceIdentifier<?> p, DataObject newObj,
                              DataObject oldObj) {
            path = p;
            newObject = newObj;
            oldObject = oldObj;
        }

        /**
         * Construct a new instance.
         *
         * @param data  An {@link IdentifiedData} object.
         */
        private NotifiedEvent(IdentifiedData<?> data) {
            this(data.getIdentifier(), data.getValue(), null);
        }

        /**
         * Construct a new instance.
         *
         * @param data  A {@link ChangedData} object.
         */
        private NotifiedEvent(ChangedData<?> data) {
            this(data.getIdentifier(), data.getValue(), data.getOldValue());
        }

        /**
         * Return the path to the data object.
         *
         * @return  Path to the data object.
         */
        private InstanceIdentifier<?> getPath() {
            return path;
        }

        /**
         * Return a new object.
         *
         * @return  A new object.
         */
        private DataObject getNewObject() {
            return newObject;
        }

        /**
         * Return an old object.
         *
         * @return  An old object.
         */
        private DataObject getOldObject() {
            return oldObject;
        }

        /**
         * Determine whether the given object is identical to this object.
         *
         * @param o  An object to be compared.
         * @return   {@code true} if identical. Otherwise {@code false}.
         */
        @Override
        public boolean equals(Object o) {
            if (o == this) {
                return true;
            }
            if (o == null || !getClass().equals(o.getClass())) {
                return false;
            }

            NotifiedEvent nev = (NotifiedEvent)o;
            return (Objects.equals(path, nev.path) &&
                    Objects.equals(newObject, nev.newObject) &&
                    Objects.equals(oldObject, nev.oldObject));
        }

        /**
         * Return the hash code of this object.
         *
         * @return  The hash code.
         */
        @Override
        public int hashCode() {
            return Objects.hash(path, newObject, oldObject);
        }

        /**
         * Return a string representation of this instance.
         *
         * @return  A string representation of this instance.
         */
        @Override
        public String toString() {
            StringBuilder builder = new StringBuilder("NotifiedEvent[path=").
                append(path).append(", new=").append(newObject).
                append(", old=").append(oldObject).append(']');
            return builder.toString();
        }
    }

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
        private final List<NotifiedEvent>  creationEvents = new ArrayList<>();

        /**
         * A list of data update events.
         */
        private final List<NotifiedEvent>  updateEvents = new ArrayList<>();

        /**
         * A list of data removal events.
         */
        private final List<NotifiedEvent>  removalEvents = new ArrayList<>();

        /**
         * Current event context.
         */
        private Object  context;

        /**
         * Data change event currently handled.
         */
        private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> current;

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
         * @param ev   The current data change event.
         * @param ctx  The event context.
         */
        private void setDataChangeEvent(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev,
            Object ctx) {
            current = ev;
            context = ctx;
        }

        /**
         * Return a data change event currently processing.
         *
         * @return  A {@link AsyncDataChangeEvent} or {@code null}.
         */
        private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> getEvent() {
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
        private List<NotifiedEvent> getCreationEvents() {
            return creationEvents;
        }

        /**
         * Return a list of update events.
         *
         * @return  A list of update events.
         */
        private List<NotifiedEvent> getUpdateEvents() {
            return updateEvents;
        }

        /**
         * Return a list of removal events.
         *
         * @return  A list of removal events.
         */
        private List<NotifiedEvent> getRemovalEvents() {
            return removalEvents;
        }

        // DataStoreListener

        /**
         * {@inheritDoc}
         */
        @Override
        protected  Object enterEvent(
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev) {
            Assert.assertEquals(current, ev);
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
        protected void onCreated(Object ectx, IdentifiedData<VtnPort> data) {
            NotifiedEvent nev = new NotifiedEvent(data);
            creationEvents.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onUpdated(Object ectx, ChangedData<VtnPort> data) {
            NotifiedEvent nev = new NotifiedEvent(data);
            updateEvents.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onRemoved(Object ectx, IdentifiedData<VtnPort> data) {
            NotifiedEvent nev = new NotifiedEvent(data);
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
        protected Set<VtnUpdateType> getRequiredEvents() {
            return requiredEvents;
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
     *   <li>{@link AbstractDataChangeListener#registerListener(DataBroker, LogicalDatastoreType, AsyncDataBroker.DataChangeScope, boolean)}</li>
     *   <li>{@link AbstractDataChangeListener#close()}</li>
     * </ul>
     */
    @Test
    public void testRegistration() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        DataBroker broker = mock(DataBroker.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<DataChangeListener> reg =
            mock(ListenerRegistration.class);
        when(broker.registerDataChangeListener(store, path, listener, scope)).
            thenReturn(reg);
        listener.registerListener(broker, store, scope, false);
        verify(broker).
            registerDataChangeListener(store, path, listener, scope);
        verify(reg, never()).close();
        verifyZeroInteractions(logger);

        // Unregister a listener.
        // Registration should be closed only one time.
        for (int i = 0; i < 10; i++) {
            listener.close();
            if (i == 0) {
                verify(reg).close();
            }
            verifyNoMoreInteractions(broker, logger, reg);
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
     *   <li>{@link AbstractDataChangeListener#registerListener(DataBroker, LogicalDatastoreType, AsyncDataBroker.DataChangeScope, boolean)}</li>
     *   <li>{@link AbstractDataChangeListener#close()}</li>
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
        DataChangeScope scope = DataChangeScope.SUBTREE;
        DataBroker broker = mock(DataBroker.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<DataChangeListener> reg =
            mock(ListenerRegistration.class);
        when(broker.registerDataChangeListener(
                 eq(store), eq(path), isA(ClusteredDataChangeListener.class),
                 eq(scope))).
            thenReturn(reg);
        listener.registerListener(broker, store, scope, true);

        ArgumentCaptor<ClusteredDataChangeListener> captor =
            ArgumentCaptor.forClass(ClusteredDataChangeListener.class);
        verify(broker).
            registerDataChangeListener(eq(store), eq(path), captor.capture(),
                                       eq(scope));
        List<ClusteredDataChangeListener> wrappers = captor.getAllValues();
        assertEquals(1, wrappers.size());
        ClusteredDataChangeListener cdcl = wrappers.get(0);
        assertEquals(listener,
                     getFieldValue(cdcl, DataChangeListener.class,
                                   "theListener"));

        verify(reg, never()).close();
        verifyZeroInteractions(logger);

        // ClusteredListener should toss received events to the actual
        // listener.
        Object ctx = new Object();
        List<NotifiedEvent> created =
            Collections.singletonList(newCreationEvent(1L, 1L));
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, null, null);
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());
        assertEquals(Collections.<NotifiedEvent>emptyList(),
                     listener.getCreationEvents());

        cdcl.onDataChanged(ev);
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
            verifyNoMoreInteractions(broker, logger, reg);
        }
    }

    /**
     * Test case for registration failure.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#registerListener(DataBroker, LogicalDatastoreType, AsyncDataBroker.DataChangeScope, boolean)}</li>
     * </ul>
     */
    @Test
    public void testRegistrationError() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        DataBroker broker = mock(DataBroker.class);
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        when(broker.registerDataChangeListener(store, path, listener, scope)).
            thenThrow(iae);

        String msg = null;
        try {
            listener.registerListener(broker, store, scope, false);
            unexpected();
        } catch (IllegalStateException e) {
            msg = "Failed to register data change listener: " +
                VtnPort.class.getName();
            assertEquals(iae, e.getCause());
            assertEquals(msg, e.getMessage());
        }

        verify(broker).
            registerDataChangeListener(store, path, listener, scope);
        verify(logger).error(msg, iae);
        verifyNoMoreInteractions(logger);

        // close() should do nothing.
        for (int i = 0; i < 10; i++) {
            listener.close();
            verify(broker).
                registerDataChangeListener(store, path, listener, scope);
            verifyNoMoreInteractions(logger);
        }
    }

    /**
     * Test case for close error.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#close()}</li>
     * </ul>
     */
    @Test
    public void testCloseError() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<VtnPort> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        DataBroker broker = mock(DataBroker.class);
        @SuppressWarnings("unchecked")
        ListenerRegistration<DataChangeListener> reg =
            mock(ListenerRegistration.class);
        when(broker.registerDataChangeListener(store, path, listener, scope)).
            thenReturn(reg);
        listener.registerListener(broker, store, scope, false);
        verify(broker).
            registerDataChangeListener(store, path, listener, scope);
        verify(reg, never()).close();
        verifyZeroInteractions(logger);

        // Unregister a listener.
        String msg = "Failed to close instance: " + reg;
        IllegalArgumentException iae =
            new IllegalArgumentException("Bad argument");
        doThrow(iae).when(reg).close();

        for (int i = 0; i < 10; i++) {
            listener.close();
            verify(broker).
                registerDataChangeListener(store, path, listener, scope);
            verify(reg).close();
            verify(logger).error(msg, iae);
            verifyNoMoreInteractions(logger);
        }
    }

    /**
     * Test case for null event.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     * </ul>
     */
    @Test
    public void testNullEvent() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        listener.onDataChanged(null);
        verify(logger).warn("Null data change event.");
        assertTrue(listener.getCreationEvents().isEmpty());
        assertTrue(listener.getUpdateEvents().isEmpty());
        assertTrue(listener.getRemovalEvents().isEmpty());
    }

    /**
     * Test case for unexpected exception.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     * </ul>
     */
    @Test
    public void testException() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        IllegalStateException ise =
            new IllegalStateException("Unexpected state");
        @SuppressWarnings("unchecked")
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            mock(AsyncDataChangeEvent.class);
        when(ev.getCreatedData()).thenThrow(ise);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        verify(logger).
            error("Unexpected exception in data change event listener.", ise);
        assertTrue(listener.getCreationEvents().isEmpty());
        assertTrue(listener.getUpdateEvents().isEmpty());
        assertTrue(listener.getRemovalEvents().isEmpty());
    }

    /**
     * Test case for an empty event.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     * </ul>
     */
    @Test
    public void testEmpty() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(null, null, null);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        verifyZeroInteractions(logger);
        assertTrue(listener.getCreationEvents().isEmpty());
        assertTrue(listener.getUpdateEvents().isEmpty());
        assertTrue(listener.getRemovalEvents().isEmpty());
    }

    /**
     * Ensure that broken objects are ignored.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onCreated(Object,Map)}</li>
     *   <li>{@link DataStoreListener#onUpdated(Object,Map,Map)}</li>
     *   <li>{@link DataStoreListener#onRemoved(Object,Set,Map)}</li>
     * </ul>
     */
    @Test
    public void testBroken() {
        VtnPort dummy = new VtnPortBuilder().build();
        List<NotifiedEvent> createdArg = new ArrayList<>();
        List<NotifiedEvent> removedArg = new ArrayList<>();
        List<NotifiedEvent> updatedArg = new ArrayList<>();

        // Wildcard path should be ignored.
        InstanceIdentifier<?> wild1 = InstanceIdentifier.
            builder(VtnNodes.class).child(VtnNode.class).build();
        InstanceIdentifier<?> wild2 = InstanceIdentifier.
            builder(VtnNodes.class).child(VtnNode.class).child(VtnPort.class).
            build();
        InstanceIdentifier<?> wild3 = InstanceIdentifier.
            builder(VtnPathPolicies.class).child(VtnPathPolicy.class).
            build();
        NotifiedEvent nev = new NotifiedEvent(wild1, dummy, null);
        createdArg.add(nev);
        nev = new NotifiedEvent(wild2, dummy, dummy);
        updatedArg.add(nev);
        nev = new NotifiedEvent(wild3, dummy, null);
        removedArg.add(nev);

        // Null identifier should be ignored.
        nev = new NotifiedEvent(null, dummy, dummy);
        createdArg.add(nev);
        updatedArg.add(nev);
        removedArg.add(nev);

        // Invalid type of path should be ignored.
        InstanceIdentifier<?> pp1 = PathPolicyUtils.getIdentifier(1);
        InstanceIdentifier<?> pp2 = PathPolicyUtils.getIdentifier(2);
        InstanceIdentifier<?> pp3 = PathPolicyUtils.
            getIdentifier(3, new VtnPortDesc("openflow:1,,"));
        nev = new NotifiedEvent(pp1, dummy, null);
        createdArg.add(nev);
        nev = new NotifiedEvent(pp2, dummy, dummy);
        updatedArg.add(nev);
        nev = new NotifiedEvent(pp3, dummy, null);
        removedArg.add(nev);

        // Invalid type of object should be ignored.
        long dpid = 1L;
        long port = 1L;
        SalPort sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> badPath1 = sport.getVtnPortIdentifier();
        VtnNode bad1 = new VtnNodeBuilder().setId(sport.getNodeId()).build();
        nev = new NotifiedEvent(badPath1, bad1, null);
        createdArg.add(nev);

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> badPath2 = sport.getVtnPortIdentifier();
        VtnNode bad2 = new VtnNodeBuilder().setId(sport.getNodeId()).build();
        nev = new NotifiedEvent(badPath2, bad2, dummy);
        updatedArg.add(nev);

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> badPath3 = sport.getVtnPortIdentifier();
        VtnNode bad3 = new VtnNodeBuilder().setId(sport.getNodeId()).build();
        nev = new NotifiedEvent(badPath3, dummy, bad3);
        updatedArg.add(nev);

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> badPath4 = sport.getVtnPortIdentifier();
        VtnNode bad4 = new VtnNodeBuilder().setId(sport.getNodeId()).build();
        nev = new NotifiedEvent(badPath4, bad4, null);
        removedArg.add(nev);

        // Null object should be ignored.
        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> null1 = sport.getVtnPortIdentifier();
        nev = new NotifiedEvent(null1, null, null);
        createdArg.add(nev);

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> null2 = sport.getVtnPortIdentifier();
        nev = new NotifiedEvent(null2, null, dummy);
        updatedArg.add(nev);

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> null3 = sport.getVtnPortIdentifier();
        nev = new NotifiedEvent(null3, dummy, null);
        updatedArg.add(nev);

        sport = new SalPort(dpid, port);
        dpid++;
        InstanceIdentifier<?> null4 = sport.getVtnPortIdentifier();
        nev = new NotifiedEvent(null4, null, null);
        removedArg.add(nev);

        List<NotifiedEvent> created = new ArrayList<>();
        nev = newCreationEvent(dpid, port);
        port++;
        createdArg.add(nev);
        created.add(nev);

        List<NotifiedEvent> updated = new ArrayList<>();
        nev = newUpdateEvent(dpid, port);
        port++;
        updatedArg.add(nev);
        updated.add(nev);

        List<NotifiedEvent> removed = new ArrayList<>();
        nev = newRemovalEvent(dpid, port);
        removedArg.add(nev);
        removed.add(nev);

        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(createdArg, updatedArg, removedArg);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev).getUpdatedData();
        verify(ev).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        // Ensure that broken events were ignored.
        assertEquals(created, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());

        // Verify that broken events were logged.
        // Wildcard paths and unwanted type of instance identifiers cannot be
        // detected because they are logged by verbose logger.
        String nullMsg = "{}: Null instance identifier.";
        verify(logger).warn(nullMsg, VtnUpdateType.CREATED);
        verify(logger).warn(nullMsg, VtnUpdateType.CHANGED);
        verify(logger).warn(nullMsg, VtnUpdateType.REMOVED);

        String dataTypeMsg =
            "{}: Unexpected data is associated: path={}, value={}";
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.CREATED, badPath1, bad1);
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.CHANGED, badPath2, bad2);
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.CHANGED, badPath3, bad3);
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.REMOVED, badPath4, bad4);

        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.CREATED, null1, null);
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.CHANGED, null2, null);
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.CHANGED, null3, null);
        verify(logger).
            warn(dataTypeMsg, VtnUpdateType.REMOVED, null4, null);
        verifyNoMoreInteractions(logger);
    }

    /**
     * Ensure that all event types can be listened.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onCreated(Object,Map)}</li>
     *   <li>{@link DataStoreListener#onUpdated(Object,Map,Map)}</li>
     *   <li>{@link DataStoreListener#onRemoved(Object,Set,Map)}</li>
     * </ul>
     */
    @Test
    public void testAll() {
        List<Set<VtnUpdateType>> allSets = new ArrayList<>();
        allSets.add(null);
        allSets.add(EnumSet.allOf(VtnUpdateType.class));
        for (Set<VtnUpdateType> all: allSets) {
            List<NotifiedEvent> created = new ArrayList<>();
            List<NotifiedEvent> removed = new ArrayList<>();
            List<NotifiedEvent> updated = new ArrayList<>();

            long dpid = 1L;
            for (long port = 1L; port <= 10L; port++) {
                created.add(newCreationEvent(dpid, port));
            }

            dpid = 10L;
            for (long port = 1L; port <= 10L; port++) {
                updated.add(newUpdateEvent(dpid, port));
            }

            dpid = 100L;
            for (long port = 1L; port <= 10L; port++) {
                removed.add(newRemovalEvent(dpid, port));
            }

            TestChangeListener listener = new TestChangeListener();
            Logger logger = listener.getLogger();
            AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
                createEvent(created, updated, removed);
            Object ctx = new Object();
            listener.setDataChangeEvent(ev, ctx);
            assertSame(ev, listener.getEvent());
            assertSame(ctx, listener.getContext());

            listener.onDataChanged(ev);
            assertSame(null, listener.getEvent());
            assertSame(null, listener.getContext());

            verify(ev).getCreatedData();
            verify(ev).getOriginalData();
            verify(ev).getUpdatedData();
            verify(ev).getRemovedPaths();
            verify(ev, never()).getOriginalSubtree();
            verify(ev, never()).getUpdatedSubtree();

            assertEquals(created, listener.getCreationEvents());
            assertEquals(updated, listener.getUpdateEvents());
            assertEquals(removed, listener.getRemovalEvents());
            verifyZeroInteractions(logger);
        }
    }

    /**
     * Ensure that only creation event can be listened.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onCreated(Object,Map)}</li>
     * </ul>
     */
    @Test
    public void testAdded() {
        List<NotifiedEvent> created = new ArrayList<>();
        List<NotifiedEvent> removed = new ArrayList<>();
        List<NotifiedEvent> updated = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            created.add(newCreationEvent(dpid, port));
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            updated.add(newUpdateEvent(dpid, port));
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            removed.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required = EnumSet.of(VtnUpdateType.CREATED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, updated, removed);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev, never()).getUpdatedData();
        verify(ev, never()).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        List<NotifiedEvent> empty = Collections.<NotifiedEvent>emptyList();
        assertEquals(created, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only update event can be listened.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onUpdated(Object,Map,Map)}</li>
     * </ul>
     */
    @Test
    public void testUpdated() {
        List<NotifiedEvent> created = new ArrayList<>();
        List<NotifiedEvent> removed = new ArrayList<>();
        List<NotifiedEvent> updated = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            created.add(newCreationEvent(dpid, port));
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            updated.add(newUpdateEvent(dpid, port));
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            removed.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required = EnumSet.of(VtnUpdateType.CHANGED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, updated, removed);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev, never()).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev).getUpdatedData();
        verify(ev, never()).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        List<NotifiedEvent> empty = Collections.<NotifiedEvent>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only removal event can be listened.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onRemoved(Object,Set,Map)}</li>
     * </ul>
     */
    @Test
    public void testRemoved() {
        List<NotifiedEvent> created = new ArrayList<>();
        List<NotifiedEvent> removed = new ArrayList<>();
        List<NotifiedEvent> updated = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            created.add(newCreationEvent(dpid, port));
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            updated.add(newUpdateEvent(dpid, port));
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            removed.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required = EnumSet.of(VtnUpdateType.REMOVED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, updated, removed);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev, never()).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev, never()).getUpdatedData();
        verify(ev).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        List<NotifiedEvent> empty = Collections.<NotifiedEvent>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only creation event can be filtered out.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onUpdated(Object,Map,Map)}</li>
     *   <li>{@link DataStoreListener#onRemoved(Object,Set,Map)}</li>
     * </ul>
     */
    @Test
    public void testIgnoreCreated() {
        List<NotifiedEvent> created = new ArrayList<>();
        List<NotifiedEvent> removed = new ArrayList<>();
        List<NotifiedEvent> updated = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            created.add(newCreationEvent(dpid, port));
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            updated.add(newUpdateEvent(dpid, port));
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            removed.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required =
            EnumSet.of(VtnUpdateType.CHANGED, VtnUpdateType.REMOVED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, updated, removed);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev, never()).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev).getUpdatedData();
        verify(ev).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        List<NotifiedEvent> empty = Collections.<NotifiedEvent>emptyList();
        assertEquals(empty, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only update event can be filtered out.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onCreated(Object,Map)}</li>
     *   <li>{@link DataStoreListener#onRemoved(Object,Set,Map)}</li>
     * </ul>
     */
    @Test
    public void testIgnoreUpdated() {
        List<NotifiedEvent> created = new ArrayList<>();
        List<NotifiedEvent> removed = new ArrayList<>();
        List<NotifiedEvent> updated = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            created.add(newCreationEvent(dpid, port));
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            updated.add(newUpdateEvent(dpid, port));
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            removed.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required =
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.REMOVED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, updated, removed);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev, never()).getUpdatedData();
        verify(ev).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        List<NotifiedEvent> empty = Collections.<NotifiedEvent>emptyList();
        assertEquals(created, listener.getCreationEvents());
        assertEquals(empty, listener.getUpdateEvents());
        assertEquals(removed, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Ensure that only removal event can be filtered out.
     *
     * <ul>
     *   <li>{@link AbstractDataChangeListener#onDataChanged(AsyncDataChangeEvent)}</li>
     *   <li>{@link DataStoreListener#onCreated(Object,Map)}</li>
     *   <li>{@link DataStoreListener#onUpdated(Object,Map,Map)}</li>
     * </ul>
     */
    @Test
    public void testIgnoreRemoved() {
        List<NotifiedEvent> created = new ArrayList<>();
        List<NotifiedEvent> removed = new ArrayList<>();
        List<NotifiedEvent> updated = new ArrayList<>();

        long dpid = 1L;
        for (long port = 1L; port <= 10L; port++) {
            created.add(newCreationEvent(dpid, port));
        }

        dpid = 10L;
        for (long port = 1L; port <= 10L; port++) {
            updated.add(newUpdateEvent(dpid, port));
        }

        dpid = 100L;
        for (long port = 1L; port <= 10L; port++) {
            removed.add(newRemovalEvent(dpid, port));
        }

        Set<VtnUpdateType> required =
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.CHANGED);
        TestChangeListener listener = new TestChangeListener(required);
        Logger logger = listener.getLogger();
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            createEvent(created, updated, removed);
        Object ctx = new Object();
        listener.setDataChangeEvent(ev, ctx);
        assertSame(ev, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataChanged(ev);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        verify(ev).getCreatedData();
        verify(ev).getOriginalData();
        verify(ev).getUpdatedData();
        verify(ev, never()).getRemovedPaths();
        verify(ev, never()).getOriginalSubtree();
        verify(ev, never()).getUpdatedSubtree();

        List<NotifiedEvent> empty = Collections.<NotifiedEvent>emptyList();
        assertEquals(created, listener.getCreationEvents());
        assertEquals(updated, listener.getUpdateEvents());
        assertEquals(empty, listener.getRemovalEvents());
        verifyZeroInteractions(logger);
    }

    /**
     * Create a mock-up of data change event.
     *
     * @param created  A list of created objects.
     * @param updated  A list of updated objects.
     * @param removed  A list of removed objects.
     * @return  A new async data change event.
     */
    private AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> createEvent(
        List<NotifiedEvent> created, List<NotifiedEvent> updated,
        List<NotifiedEvent> removed) {
        Map<InstanceIdentifier<?>, DataObject> createdMap;
        if (created == null) {
            createdMap = null;
        } else {
            createdMap =
                new LinkedHashMap<InstanceIdentifier<?>, DataObject>();
            for (NotifiedEvent nev: created) {
                createdMap.put(nev.getPath(), nev.getNewObject());
            }
        }

        Map<InstanceIdentifier<?>, DataObject> original = null;
        Map<InstanceIdentifier<?>, DataObject> updatedMap;
        if (updated == null) {
            updatedMap = null;
        } else {
            updatedMap =
                new LinkedHashMap<InstanceIdentifier<?>, DataObject>();
            for (NotifiedEvent nev: updated) {
                InstanceIdentifier<?> path = nev.getPath();
                updatedMap.put(path, nev.getNewObject());
                DataObject old = nev.getOldObject();
                if (old != null) {
                    original = putData(original, path, old);
                }
            }
        }

        Set<InstanceIdentifier<?>> removedSet;
        if (removed == null) {
            removedSet = null;
        } else {
            removedSet = new LinkedHashSet<InstanceIdentifier<?>>();
            for (NotifiedEvent nev: removed) {
                InstanceIdentifier<?> path = nev.getPath();
                removedSet.add(path);
                DataObject old = nev.getNewObject();
                if (old != null) {
                    original = putData(original, path, old);
                }
            }
        }

        @SuppressWarnings("unchecked")
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev =
            mock(AsyncDataChangeEvent.class);
        when(ev.getCreatedData()).thenReturn(createdMap);
        when(ev.getUpdatedData()).thenReturn(updatedMap);
        when(ev.getRemovedPaths()).thenReturn(removedSet);
        when(ev.getOriginalData()).thenReturn(original);

        return ev;
    }

    /**
     * Put the given object into the given path.
     *
     * @param map   A map to add the object.
     *              A new map is created if {@code null} is passed.
     * @param path  A path to the data object.
     * @param obj   A data object.
     * @return  {@code map} or a newly created map.
     */
    private Map<InstanceIdentifier<?>, DataObject> putData(
        Map<InstanceIdentifier<?>, DataObject> map, InstanceIdentifier<?> path,
        DataObject obj) {
        Map<InstanceIdentifier<?>, DataObject> ret;
        if (map == null) {
            ret = new HashMap<InstanceIdentifier<?>, DataObject>();
        } else {
            ret = map;
        }
        ret.put(path, obj);

        return ret;
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the creation of
     * {@link VtnPort}.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent newCreationEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort vport = createVtnPortBuilder(sport).build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent(path, vport, null);
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the change of
     * {@link VtnPort}.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent newUpdateEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort newPort = createVtnPortBuilder(sport).build();
        VtnPort oldPort = createVtnPortBuilder(sport, Boolean.FALSE, false).
            build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent(path, newPort, oldPort);
    }

    /**
     * Create a {@link NotifiedEvent} instance which indicates the removal of
     * {@link VtnPort}.
     *
     * @param dpid  A datapath ID of the node.
     * @param port  The number of the physical port number.
     * @return  A {@link NotifiedEvent} instance.
     */
    private NotifiedEvent newRemovalEvent(long dpid, long port) {
        SalPort sport = new SalPort(dpid, port);
        VtnPort vport = createVtnPortBuilder(sport).build();
        InstanceIdentifier<VtnPort> path = sport.getVtnPortIdentifier();
        return new NotifiedEvent(path, vport, null);
    }
}
