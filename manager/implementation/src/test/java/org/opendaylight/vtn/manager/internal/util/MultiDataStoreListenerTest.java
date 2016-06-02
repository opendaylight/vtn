/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.junit.Assert.assertEquals;

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
import java.util.Deque;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.annotation.Nonnull;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.ImmutableSet;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.slf4j.Logger;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.vnode.BridgeStatusBuildHelper;
import org.opendaylight.vtn.manager.internal.vnode.VbridgeBuildHelper;
import org.opendaylight.vtn.manager.internal.vnode.VinterfaceBuildHelper;
import org.opendaylight.vtn.manager.internal.vnode.VterminalBuildHelper;
import org.opendaylight.vtn.manager.internal.vnode.VtnBuildHelper;
import org.opendaylight.vtn.manager.internal.vnode.VtnTreeBuilder;

import org.opendaylight.controller.md.sal.binding.api.ClusteredDataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification.ModificationType;
import org.opendaylight.controller.md.sal.binding.api.DataObjectModification;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeListener;
import org.opendaylight.controller.md.sal.binding.api.DataTreeChangeService;
import org.opendaylight.controller.md.sal.binding.api.DataTreeIdentifier;
import org.opendaylight.controller.md.sal.binding.api.DataTreeModification;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.IdentifiableItem;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.Item;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.PathArgument;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.Vtns;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.Vtn;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtns.VtnKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPathsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.port.mappable.bridge.BridgeStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.VinterfaceKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalKey;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

/**
 * JUnit test for {@link MultiDataStoreListener}.
 */
public class MultiDataStoreListenerTest extends TestBase {
    /**
     * Stub data change listener for test.
     */
    private static class TestChangeListener
        extends MultiDataStoreListener<Vtn, Object> {
        /**
         * Logger instance.
         */
        private Logger  logger;

        /**
         * A wildcard path.
         */
        private final InstanceIdentifier<Vtn>  wildcardPath =
            InstanceIdentifier.builder(Vtns.class).child(Vtn.class).
            build();

        /**
         * Event attribute that determines behavior of
         * {@link MultiDataStoreListener}.
         */
        private final MultiEventAttr  attributes;

        /**
         * A list of notified events.
         */
        private final List<NotifiedEvent<?>>  events = new ArrayList<>();

        /**
         * Current event context.
         */
        private Object  context;

        /**
         * Data tree change event currently handled.
         */
        private Collection<DataTreeModification<Vtn>>  current;

        /**
         * The event type to be handled by depth mode, which notifies children
         * before parent.
         */
        private VtnUpdateType  depthType;

        /**
         * Construct a new instance.
         */
        private TestChangeListener() {
            this(null);
        }

        /**
         * Construct a new instance.
         *
         * @param attr  A {@link MultiEventAttr} instance.
         */
        private TestChangeListener(MultiEventAttr attr) {
            super(Vtn.class);
            attributes = (attr == null) ? new MultiEventAttr() : attr;
        }

        /**
         * Set the current data change event.
         *
         * @param changes  The current data change events.
         * @param ctx      The event context.
         */
        private void setDataChangeEvent(
            Collection<DataTreeModification<Vtn>> changes, Object ctx) {
            current = changes;
            context = ctx;
        }

        /**
         * Return a data tree change events currently processing.
         *
         * @return  A collection of data tree modification or {@code null}.
         */
        private Collection<DataTreeModification<Vtn>> getEvent() {
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
         * Return a list of notified events.
         *
         * @return  A list of notified events.
         */
        private List<NotifiedEvent<?>> getNotifiedEvents() {
            return events;
        }

        // MultiDataStoreListener

        /**
         * Determine whether the specified type of the tree node should be
         * treated as a leaf node.
         *
         * @param type  A class that specifies the type of the tree node.
         * @return  {@code true} if the specified type of the tree node should
         *          be treated as a leaf node. {@code false} otherwise.
         */
        @Override
        protected boolean isLeafNode(@Nonnull Class<?> type) {
            return attributes.isLeafNode(type);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isDepth(@Nonnull VtnUpdateType type) {
            return attributes.isDepth(type);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isRequiredType(@Nonnull Class<?> type) {
            return attributes.isRequiredType(type);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected boolean isUpdated(Object ectx, ChangedData<?> data) {
            return attributes.isUpdated(data);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onCreated(Object ectx, IdentifiedData<?> data) {
            NotifiedEvent<?> nev =
                new NotifiedEvent<>(data, VtnUpdateType.CREATED);
            events.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onUpdated(Object ectx, ChangedData<?> data) {
            NotifiedEvent<?> nev = new NotifiedEvent<>(data);
            events.add(nev);
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void onRemoved(Object ectx, IdentifiedData<?> data) {
            NotifiedEvent<?> nev =
                new NotifiedEvent<>(data, VtnUpdateType.REMOVED);
            events.add(nev);
        }

        // AbstractDataChangeListener

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
            assertEquals(context, ectx);
            current = null;
            context = null;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected InstanceIdentifier<Vtn> getWildcardPath() {
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
            return attributes.isRequiredEvent(type);
        }
    }

    /**
     * Describes a hook to check test data.
     */
    private abstract static class EventChecker {
        /**
         * Reset the internal state.
         */
        protected void reset() {
        }

        /**
         * Verify generated test data.
         *
         * @param nev  A {@link NotifiedEvent} instance that represents an
         *             event to be notified.
         * @return  {@code true} if the specified event should be notified.
         *          {@code true} if the specified event should never be
         *          notified.
         */
        protected abstract boolean check(NotifiedEvent<?> nev);
    }

    /**
     * The base class for a hook that checks the order of events.
     */
    private abstract static class EventOrderChecker extends EventChecker {
        /**
         * A set of paths already notified.
         */
        private final Set<InstanceIdentifier<?>>  notifiedPaths =
            new LinkedHashSet<>();

        /**
         * The path notified by the last event.
         */
        private InstanceIdentifier<?>  lastPath;

        /**
         * Return the path in the given event.
         *
         * @param nev  A {@link NotifiedEvent} instance that represents an
         *             event to be notified.
         * @return  The path in the given event.
         */
        protected InstanceIdentifier<?> getPath(NotifiedEvent<?> nev) {
            InstanceIdentifier<?> path = nev.getPath();
            lastPath = path;
            assertEquals(true, notifiedPaths.add(path));
            return path;
        }

        /**
         * Return a set of notified paths.
         *
         * @return  A set of notified paths.
         */
        protected Set<InstanceIdentifier<?>> getNotifiedPaths() {
            return notifiedPaths;
        }

        /**
         * Return the path to the parent node of the node specified by the
         * given instance identifier.
         *
         * @param path  The path to the tree node.
         * @return  The path to the parent node inside the VTN.
         *          {@code null} if {@code path} points the VTN.
         */
        protected final InstanceIdentifier<?> getParent(
            InstanceIdentifier<?> path) {
            InstanceIdentifier<?> ret;
            if (Vtn.class.equals(path.getTargetType())) {
                ret = null;
            } else {
                Deque<PathArgument> args = TestBase.getPathArguments(path);
                args.removeLast();
                ret = InstanceIdentifier.create(args);
            }

            return ret;
        }

        /**
         * Return the path notified by the last event.
         *
         * @return  The path notified by the last event.
         */
        protected InstanceIdentifier<?> getLastPath() {
            return lastPath;
        }

        // EventChecker

        /**
         * {@inheritDoc}
         */
        @Override
        protected void reset() {
            notifiedPaths.clear();
            lastPath = null;
        }
    }

    /**
     * An implementation of {@link EventChecker} to ensure that each tree
     * node's children are notified after the tree node itself.
     */
    private static final class DepthFirstEventChecker
        extends EventOrderChecker {
        // EventChecker

        /**
         * Ensure that each tree node's children are notified after the
         * tree node itself.
         *
         * @param nev  A {@link NotifiedEvent} instance that represents an
         *             event to be notified.
         * @return  {@code true} if the specified event should be notified.
         *          {@code true} if the specified event should never be
         *          notified.
         */
        @Override
        protected boolean check(NotifiedEvent<?> nev) {
            Set<InstanceIdentifier<?>> notified = getNotifiedPaths();
            boolean first = notified.isEmpty();
            InstanceIdentifier<?> path = getPath(nev);
            if (first) {
                // The first node should be a VTN.
                assertEquals(Vtn.class, path.getTargetType());
            }

            InstanceIdentifier<?> parent = getParent(path);
            if (parent != null) {
                // The parent should be notified before children.
                assertEquals(true, notified.contains(parent));
            }

            return true;
        }
    }

    /**
     * An implementation of {@link EventChecker} to ensure that each tree
     * node's children are notified before the tree node itself.
     */
    private static final class DepthEventChecker extends EventOrderChecker {
        // EventChecker

        /**
         * Ensure that each tree node's children are notified before the
         * tree node itself.
         *
         * @param nev  A {@link NotifiedEvent} instance that represents an
         *             event to be notified.
         * @return  {@code true} if the specified event should be notified.
         *          {@code true} if the specified event should never be
         *          notified.
         */
        @Override
        protected boolean check(NotifiedEvent<?> nev) {
            InstanceIdentifier<?> path = getPath(nev);
            for (InstanceIdentifier<?> p: getNotifiedPaths()) {
                if (!p.equals(path)) {
                    assertEquals(false, p.contains(path));
                }
            }

            return true;
        }
    }

    /**
     * An implementation of {@link EventChecker} to ensure that CHANGED
     * events are notified correctly.
     */
    private static final class ChangedEventChecker extends EventChecker {
        /**
         * A set of paths to be expected to be notified by CHANGED events.
         */
        private final Set<InstanceIdentifier<?>>  changedPaths;

        /**
         * A set of target types to be ignored.
         */
        private final Set<Class<?>>  ignoredTypes;

        /**
         * A set of paths that are not yet notified.
         */
        private Set<InstanceIdentifier<?>>  remains;

        /**
         * Construct a new instance.
         *
         * @param changed  A set of paths to be expected to be notified by
         *                 CHANGED events.
         */
        private ChangedEventChecker(Set<InstanceIdentifier<?>> changed) {
            this(changed, Collections.<Class<?>>emptySet());
        }

        /**
         * Construct a new instance.
         *
         * @param changed  A set of paths to be expected to be notified by
         *                 CHANGED events.
         * @param ignored  A set of target types to be ignored.
         */
        private ChangedEventChecker(Set<InstanceIdentifier<?>> changed,
                                    Set<Class<?>> ignored) {
            changedPaths = ImmutableSet.copyOf(changed);
            ignoredTypes = ImmutableSet.copyOf(ignored);
        }

        /**
         * Return a set of paths that are not yet notified.
         *
         * @return  A set if {@link InstanceIdentifier} instances that are
         *          not yet notified.
         */
        private Set<InstanceIdentifier<?>> getRemains() {
            return remains;
        }

        // EventChecker

        /**
         * Ensure that CHANGED events are notified correctly.
         *
         * @param nev  A {@link NotifiedEvent} instance that represents an
         *             event to be notified.
         * @return  {@code true} if the specified event should be notified.
         *          {@code true} if the specified event should never be
         *          notified.
         */
        @Override
        public boolean check(NotifiedEvent<?> nev) {
            boolean ret = true;
            if (nev.getUpdateType() == VtnUpdateType.CHANGED) {
                InstanceIdentifier<?> path = nev.getPath();
                if (ignoredTypes.contains(path.getTargetType())) {
                    ret = false;
                } else {
                    assertEquals(true, remains.remove(path));
                }
            }

            return ret;
        }

        /**
         * {@inheritDoc}
         */
        @Override
        protected void reset() {
            remains = new HashSet<>(changedPaths);
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
     *   <li>{@link MultiDataStoreListener#registerListener(DataTreeChangeService, LogicalDatastoreType, boolean)}</li>
     *   <li>{@link MultiDataStoreListener#close()}</li>
     * </ul>
     */
    @Test
    public void testRegistration() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<Vtn> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<Vtn> ident = new DataTreeIdentifier<>(store, path);
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
     *   <li>{@link MultiDataStoreListener#registerListener(DataTreeChangeService, LogicalDatastoreType, boolean)}</li>
     *   <li>{@link MultiDataStoreListener#close()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testRegistrationCluster() throws Exception {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<Vtn> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<Vtn> ident = new DataTreeIdentifier<>(store, path);
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
        VtnTreeBuilder tree = new VtnTreeBuilder();
        VtnTreeBuilder oldTree = new VtnTreeBuilder();
        VtnBuildHelper vbh = tree.createVtn("vtn_1");
        tree.freeze();
        oldTree.freeze();
        Collection<DataTreeModification<Vtn>> changes =
            tree.createEvent(oldTree, true);
        Vtn vtn = vbh.build();
        InstanceIdentifier<Vtn> vpath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtn.getKey()).
            build();
        NotifiedEvent<Vtn> nev =
            new NotifiedEvent<>(vpath, vtn, null, VtnUpdateType.CREATED);
        List<NotifiedEvent<Vtn>> created = Collections.singletonList(nev);
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());
        assertEquals(Collections.<NotifiedEvent<Vtn>>emptyList(),
                     listener.getNotifiedEvents());

        cdcl.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        assertEquals(created, listener.getNotifiedEvents());

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
     *   <li>{@link MultiDataStoreListener#registerListener(DataTreeChangeService, LogicalDatastoreType, boolean)}</li>
     * </ul>
     */
    @Test
    public void testRegistrationError() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<Vtn> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<Vtn> ident = new DataTreeIdentifier<>(store, path);
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
                Vtn.class.getName() + ", path=" + path;
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
     *   <li>{@link MultiDataStoreListener#close()}</li>
     * </ul>
     */
    @Test
    public void testCloseError() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        InstanceIdentifier<Vtn> path = listener.getWildcardPath();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        DataTreeIdentifier<Vtn> ident = new DataTreeIdentifier<>(store, path);
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
     * Test case for an empty event.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     * </ul>
     */
    @Test
    public void testEmpty() {
        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        Collection<DataTreeModification<Vtn>> changes = new ArrayList<>();
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());
        verifyZeroInteractions(logger);
        List<NotifiedEvent<?>> empty =
            Collections.<NotifiedEvent<?>>emptyList();
        assertEquals(empty, listener.getNotifiedEvents());
    }

    /**
     * Ensure that broken objects are ignored.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    // @Test
    public void testBroken() {
        List<DataTreeModification<Vtn>> changes = new ArrayList<>();
        List<NotifiedEvent<?>> notified = new ArrayList<>();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;

        // Wildcard path should be ignored.
        InstanceIdentifier<Vtn> wild1 = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class).
            build();
        VnodeName tname = new VnodeName("vtn1");
        Vtn vtn = new VtnBuilder().setName(tname).build();
        DataObjectModification<Vtn> mod = newDataModification(
            Vtn.class, ModificationType.WRITE, new Item(Vtn.class), null,
            vtn, null);
        changes.add(newTreeModification(wild1, oper, mod));

        tname = new VnodeName("vtn2");
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            build();
        Deque<PathArgument> args = getPathArguments(vtnPath);
        args.addLast(new Item<Vbridge>(Vbridge.class));
        InstanceIdentifier<?> wild2 = InstanceIdentifier.create(args);
        args.removeLast();
        args.addLast(new Item<Vterminal>(Vterminal.class));
        InstanceIdentifier<?> wild3 = InstanceIdentifier.create(args);

        VbridgeBuildHelper newVbr = new VbridgeBuildHelper("vbr1").
            setConfig(null, 5000);
        VbridgeBuildHelper oldVbr = new VbridgeBuildHelper("vbr1").
            setConfig(null, 5001);
        VterminalBuildHelper vterm = new VterminalBuildHelper("vterm");
        newVbr.freeze();
        oldVbr.freeze();
        vterm.freeze();

        Vtn oldVtn = new VtnBuilder().
            setName(tname).
            setVbridge(Collections.singletonList(oldVbr.build())).
            setVterminal(Collections.singletonList(vterm.build())).
            build();
        Vtn newVtn = new VtnBuilder().
            setName(tname).
            setVbridge(Collections.singletonList(newVbr.build())).
            build();
        List<DataObjectModification<?>> children = new ArrayList<>();
        DataObjectModification<Vbridge> vbrMod = newDataModification(
            Vbridge.class, ModificationType.SUBTREE_MODIFIED,
            new Item<Vbridge>(Vbridge.class), oldVbr.build(), newVbr.build(),
            null);
        DataObjectModification<Vterminal> vtermMod = newDataModification(
            Vterminal.class, ModificationType.DELETE,
            new Item<Vterminal>(Vterminal.class), vterm.build(), null, null);
        Collections.addAll(children, vbrMod, vtermMod);

        mod = newKeyedModification(oldVtn, newVtn, children);
        NotifiedEvent<?> nev = new NotifiedEvent<Vtn>(
            vtnPath, newVtn, oldVtn, VtnUpdateType.CHANGED);
        notified.add(nev);
        changes.add(newTreeModification(vtnPath, oper, mod));

        // Null object should be ignored.
        tname = new VnodeName("vtn3");
        VtnKey vtnKey = new VtnKey(tname);
        IdentifiableItem<Vtn, VtnKey> vtnItem =
            new IdentifiableItem<>(Vtn.class, vtnKey);
        InstanceIdentifier<Vtn> null1 = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, vtnKey).
            build();
        mod = newDataModification(
            Vtn.class, ModificationType.WRITE, vtnItem, null, null, null);
        changes.add(newTreeModification(null1, oper, mod));

        tname = new VnodeName("vtn4");
        vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            build();

        VbridgeKey vbrKey = new VbridgeKey(new VnodeName("vbr1"));
        InstanceIdentifier<Vbridge> null2 = vtnPath.
            child(Vbridge.class, vbrKey);
        VterminalKey vtermKey = new VterminalKey(new VnodeName("vterm"));
        InstanceIdentifier<Vterminal> null3 = vtnPath.
            child(Vterminal.class, vtermKey);

        newVbr = new VbridgeBuildHelper(vbrKey.getName()).
            setConfig("vBridge", 5000);
        oldVbr = new VbridgeBuildHelper(vbrKey.getName()).
            setConfig("Old vBridge", 5000);
        vterm = new VterminalBuildHelper(vtermKey.getName());
        newVbr.freeze();
        oldVbr.freeze();
        vterm.freeze();

        oldVtn = new VtnBuilder().
            setName(tname).
            setVbridge(Collections.singletonList(oldVbr.build())).
            setVterminal(Collections.singletonList(vterm.build())).
            build();
        newVtn = new VtnBuilder().
            setName(tname).
            setVbridge(Collections.singletonList(newVbr.build())).
            build();
        children = new ArrayList<>();
        IdentifiableItem<Vbridge, VbridgeKey> vbrItem =
            new IdentifiableItem<>(Vbridge.class, vbrKey);
        vbrMod = newDataModification(
            Vbridge.class, ModificationType.SUBTREE_MODIFIED, vbrItem,
            oldVbr.build(), null, null);
        IdentifiableItem<Vterminal, VterminalKey> vtermItem =
            new IdentifiableItem<>(Vterminal.class, vtermKey);
        vtermMod = newDataModification(
            Vterminal.class, ModificationType.DELETE, vtermItem, null, null,
            null);
        Collections.addAll(children, vbrMod, vtermMod);

        mod = newKeyedModification(oldVtn, newVtn, children);
        nev = new NotifiedEvent<Vtn>(
            vtnPath, newVtn, oldVtn, VtnUpdateType.CHANGED);
        notified.add(nev);
        changes.add(newTreeModification(vtnPath, oper, mod));

        // Invalid target type should be ignored.
        tname = new VnodeName("vtn5");
        vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname)).
            build();

        VbridgeKey vbrKey2 = new VbridgeKey(new VnodeName("vbridge_2"));
        args = getPathArguments(vtnPath);
        PathArgument badArg1 = new IdentifiableItem<Vinterface, VinterfaceKey>(
            Vinterface.class, new VinterfaceKey(new VnodeName("if_bad_1")));
        PathArgument badArg2 = new IdentifiableItem<Vinterface, VinterfaceKey>(
            Vinterface.class, new VinterfaceKey(new VnodeName("if_bad_2")));
        PathArgument badArg3 = new IdentifiableItem<Vinterface, VinterfaceKey>(
            Vinterface.class, new VinterfaceKey(new VnodeName("if_bad_3")));
        args.addLast(badArg1);
        InstanceIdentifier<?> bad1 = InstanceIdentifier.create(args);
        args.removeLast();
        args.addLast(badArg2);
        InstanceIdentifier<?> bad2 = InstanceIdentifier.create(args);
        args.removeLast();
        args.addLast(badArg3);
        InstanceIdentifier<?> bad3 = InstanceIdentifier.create(args);

        newVbr = new VbridgeBuildHelper(vbrKey.getName()).
            setConfig("vBridge", 3333);
        oldVbr = new VbridgeBuildHelper(vbrKey.getName()).
            setConfig("Old vBridge", 3333);
        VbridgeBuildHelper newVbr2 = new VbridgeBuildHelper(vbrKey2.getName()).
            setConfig(null, 100);
        vterm = new VterminalBuildHelper(vtermKey.getName());
        newVbr.freeze();
        oldVbr.freeze();
        newVbr2.freeze();
        vterm.freeze();

        oldVtn = new VtnBuilder().
            setName(tname).
            setVbridge(Collections.singletonList(oldVbr.build())).
            setVterminal(Collections.singletonList(vterm.build())).
            build();
        List<Vbridge> bridges = new ArrayList<>();
        Collections.addAll(bridges, newVbr.build(), newVbr2.build());
        newVtn = new VtnBuilder().
            setName(tname).
            setVbridge(bridges).
            build();
        children = new ArrayList<>();
        vbrMod = newDataModification(
            Vbridge.class, ModificationType.SUBTREE_MODIFIED, badArg1,
            oldVbr.build(), newVbr.build(), null);
        DataObjectModification<Vbridge> vbrMod2 = newDataModification(
            Vbridge.class, ModificationType.WRITE, badArg2, null,
            newVbr2.build(), null);
        vtermMod = newDataModification(
            Vterminal.class, ModificationType.DELETE, badArg3, vterm.build(),
            null, null);
        Collections.addAll(children, vbrMod, vbrMod2, vtermMod);

        mod = newKeyedModification(oldVtn, newVtn, children);
        nev = new NotifiedEvent<Vtn>(
            vtnPath, newVtn, oldVtn, VtnUpdateType.CHANGED);
        notified.add(nev);
        changes.add(newTreeModification(vtnPath, oper, mod));

        TestChangeListener listener = new TestChangeListener();
        Logger logger = listener.getLogger();
        LogicalDatastoreType store = LogicalDatastoreType.OPERATIONAL;
        Object ctx = new Object();
        listener.setDataChangeEvent(changes, ctx);
        assertSame(changes, listener.getEvent());
        assertSame(ctx, listener.getContext());

        listener.onDataTreeChanged(changes);
        assertSame(null, listener.getEvent());
        assertSame(null, listener.getContext());

        // Ensure that broken events were ignored.
        assertEquals(notified, listener.getNotifiedEvents());

        // Verify that broken events were logged.
        String wildMsg = "{}: Ignore wildcard path: {}";
        verify(logger).warn(wildMsg, VtnUpdateType.CREATED, wild1);
        verify(logger).warn(wildMsg, VtnUpdateType.CHANGED, wild2);
        verify(logger).warn(wildMsg, VtnUpdateType.REMOVED, wild3);

        String nullValueMsg = "{}: Null value is notified: path={}";
        verify(logger).warn(nullValueMsg, "handleTreeNode", null1);
        verify(logger).warn(nullValueMsg, "handleTreeNode", null2);
        verify(logger).warn(nullValueMsg, VtnUpdateType.REMOVED, null3);

        String badPathMsg =
            "{}: Unexpected instance identifier type: path={}, expected={}";
        verify(logger).warn(badPathMsg, VtnUpdateType.CHANGED, bad1,
                            Vbridge.class);
        verify(logger).warn(badPathMsg, VtnUpdateType.CREATED, bad2,
                            Vbridge.class);
        verify(logger).warn(badPathMsg, VtnUpdateType.REMOVED, bad3,
                            Vterminal.class);

        verifyNoMoreInteractions(logger);
    }

    /**
     * Ensure that all event types can be listened.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     * </ul>
     */
    @Test
    public void testAll() {
        runTest(new MultiEventAttr());
    }

    /**
     * Ensure that the order of notifications can be controlled.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     *   <li>{@link MultiDataStoreListener#isDepth(VtnUpdateType)}</li>
     * </ul>
     */
    @Test
    public void testDepth() {
        List<Set<VtnUpdateType>> cases = new ArrayList<>();
        Collections.addAll(
            cases,
            EnumSet.of(VtnUpdateType.CREATED),
            EnumSet.of(VtnUpdateType.CHANGED),
            EnumSet.of(VtnUpdateType.REMOVED),
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.CHANGED),
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.REMOVED),
            EnumSet.of(VtnUpdateType.CHANGED, VtnUpdateType.REMOVED));

        for (Set<VtnUpdateType> depth: cases) {
            MultiEventAttr attr = new MultiEventAttr().
                setDepthModes(depth);
            runTest(attr);
        }

        // Ensure that all events are ordered in depth-first mode.
        DepthFirstEventChecker dfec = new DepthFirstEventChecker();
        MultiEventAttr attr = new MultiEventAttr();
        runTest(attr, dfec);

        // Ensure that all events are ordered in depth mode.
        Set<VtnUpdateType> depth = Collections.<VtnUpdateType>emptySet();
        attr = new MultiEventAttr().setDepthModes(depth);
        DepthEventChecker dec = new DepthEventChecker();
        runTest(attr, dec);
        assertEquals(Vtn.class, dec.getLastPath().getTargetType());
    }

    /**
     * Ensure that events can be filtered out by event type.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     *   <li>{@link MultiDataStoreListener#isRequiredEvent(VtnUpdateType)}</li>
     * </ul>
     */
    @Test
    public void testEventType() {
        List<Set<VtnUpdateType>> cases = new ArrayList<>();
        Collections.addAll(
            cases,
            EnumSet.of(VtnUpdateType.CREATED),
            EnumSet.of(VtnUpdateType.CHANGED),
            EnumSet.of(VtnUpdateType.REMOVED),
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.CHANGED),
            EnumSet.of(VtnUpdateType.CREATED, VtnUpdateType.REMOVED),
            EnumSet.of(VtnUpdateType.CHANGED, VtnUpdateType.REMOVED),
            EnumSet.allOf(VtnUpdateType.class));

        for (final Set<VtnUpdateType> utypes: cases) {
            EventChecker checker = new EventChecker() {
                @Override
                public boolean check(NotifiedEvent<?> nev) {
                    assertEquals(true, utypes.contains(nev.getUpdateType()));
                    return true;
                }
            };

            MultiEventAttr attr = new MultiEventAttr().
                setRequiredEvents(utypes);
            runTest(attr, checker);
        }
    }

    /**
     * Ensure that events can be filtered out by data type.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     *   <li>{@link MultiDataStoreListener#isRequiredType(Class)}</li>
     * </ul>
     */
    @Test
    public void testDataType() {
        List<Set<Class<?>>> cases = new ArrayList<>();
        Collections.addAll(
            cases,
            ImmutableSet.of(Vtn.class),
            ImmutableSet.of(Vtn.class, Vinterface.class),
            ImmutableSet.of(Vbridge.class, Vterminal.class),
            ImmutableSet.of(VbridgeConfig.class, BridgeStatus.class,
                            VterminalConfig.class, Vinterface.class,
                            VinterfaceConfig.class, VinterfaceStatus.class),
            ImmutableSet.of(VbridgeConfig.class, BridgeStatus.class,
                            VtenantConfig.class, Vinterface.class,
                            VinterfaceStatus.class, FaultedPaths.class));

        for (final Set<Class<?>> types: cases) {
            EventChecker checker = new EventChecker() {
                @Override
                public boolean check(NotifiedEvent<?> nev) {
                    Class<?> type = nev.getPath().getTargetType();
                    assertEquals(true, types.contains(type));
                    return true;
                }
            };

            MultiEventAttr attr = new MultiEventAttr().setRequiredTypes(types);
            runTest(attr, checker);
        }
    }

    /**
     * Ensure that events can be filtered out by specifying leaf node.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     *   <li>{@link MultiDataStoreListener#isLeafNode(Class)}</li>
     * </ul>
     */
    @Test
    public void testLeafNode() {
        // Treat vtn as leaf node.
        Set<Class<?>> leaves = ImmutableSet.of(Vtn.class);
        Set<Class<?>> expected = ImmutableSet.of(Vtn.class);
        runLeafTest(leaves, expected);

        // Treat vbridge and vterminal as leaf node.
        leaves = ImmutableSet.of(Vbridge.class, Vterminal.class);
        expected = ImmutableSet.of(Vtn.class, VtenantConfig.class,
                                   Vbridge.class, Vterminal.class);
        runLeafTest(leaves, expected);

        // Treat vinterface and bridge-status as leaf node.
        leaves = ImmutableSet.of(Vinterface.class, BridgeStatus.class);
        expected = ImmutableSet.of(Vtn.class, VtenantConfig.class,
                                   Vbridge.class, VbridgeConfig.class,
                                   Vterminal.class, VterminalConfig.class,
                                   BridgeStatus.class, Vinterface.class);
        runLeafTest(leaves, expected);
    }

    /**
     * Ensure that changed events can be controlled by
     * {@link MultiDataStoreListener#isUpdated(Object, ChangedData)}.
     *
     * <ul>
     *   <li>{@link MultiDataStoreListener#onDataTreeChanged(Collection)}</li>
     *   <li>{@link MultiDataStoreListener#handleTree(TreeChangeContext,Object,DataTreeModification)}</li>
     *   <li>{@link MultiDataStoreListener#isUpdated(Object, ChangedData)}</li>
     * </ul>
     */
    @Test
    public void testIsUpdated() {
        // Ensure that only changed tree nodes are notified.
        Set<InstanceIdentifier<?>> changedPaths = new HashSet<>();
        InstanceIdentifier<Vtn> vtnPath = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(new VnodeName("vtn_3"))).
            build();
        assertEquals(true, changedPaths.add(vtnPath));
        InstanceIdentifier<?> path = vtnPath.child(VtenantConfig.class);
        assertEquals(true, changedPaths.add(path));

        InstanceIdentifier<Vbridge> vbrPath = vtnPath.
            child(Vbridge.class, new VbridgeKey(new VnodeName("vbridge_2")));
        assertEquals(true, changedPaths.add(vbrPath));
        path = vbrPath.child(VbridgeConfig.class);
        assertEquals(true, changedPaths.add(path));
        path = vbrPath.child(BridgeStatus.class);
        assertEquals(true, changedPaths.add(path));

        VnodeName iname1 = new VnodeName("vif_1");
        InstanceIdentifier<Vinterface> ifPath = vbrPath.
            child(Vinterface.class, new VinterfaceKey(iname1));
        assertEquals(true, changedPaths.add(ifPath));
        path = ifPath.child(VinterfaceConfig.class);
        assertEquals(true, changedPaths.add(path));
        path = ifPath.child(VinterfaceStatus.class);
        assertEquals(true, changedPaths.add(path));

        InstanceIdentifier<Vterminal> vtermPath = vtnPath.
            child(Vterminal.class,
                  new VterminalKey(new VnodeName("vterminal_1")));
        assertEquals(true, changedPaths.add(vtermPath));

        VnodeName iname2 = new VnodeName("vif_2");
        ifPath = vtermPath.child(Vinterface.class, new VinterfaceKey(iname2));
        assertEquals(true, changedPaths.add(ifPath));
        path = ifPath.child(VinterfaceStatus.class);
        assertEquals(true, changedPaths.add(path));

        vtermPath = vtnPath.
            child(Vterminal.class,
                  new VterminalKey(new VnodeName("vterminal_2")));
        assertEquals(true, changedPaths.add(vtermPath));

        ifPath = vtermPath.child(Vinterface.class, new VinterfaceKey(iname2));
        assertEquals(true, changedPaths.add(ifPath));
        path = ifPath.child(VinterfaceConfig.class);
        assertEquals(true, changedPaths.add(path));
        path = ifPath.child(VinterfaceStatus.class);
        assertEquals(true, changedPaths.add(path));

        ChangedEventChecker checker = new ChangedEventChecker(changedPaths);
        MultiEventAttr attr = new MultiEventAttr();
        runTest(attr, checker);
        Set<InstanceIdentifier<?>> empty =
            Collections.<InstanceIdentifier<?>>emptySet();
        assertEquals(empty, checker.getRemains());

        // Ensure that all CHANGED events can be filtered out.
        EventChecker noChanged = new EventChecker() {
            @Override
            public boolean check(NotifiedEvent<?> nev) {
                return (nev.getUpdateType() != VtnUpdateType.CHANGED);
            }
        };

        attr = new MultiEventAttr().setUpdated(null);
        runTest(attr, noChanged);

        // Ignore CHANGED events on vbridge, vinterface, and vinterface-status.
        Map<Class<?>, Boolean> ignored = ImmutableMap.of(
            Vbridge.class, false, Vinterface.class, false,
            VinterfaceStatus.class, false);
        for (Iterator<InstanceIdentifier<?>> it = changedPaths.iterator();
             it.hasNext();) {
            path = it.next();
            if (ignored.containsKey(path.getTargetType())) {
                it.remove();
            }
        }

        checker = new ChangedEventChecker(changedPaths, ignored.keySet());
        attr = new MultiEventAttr().setUpdated(ignored);
        runTest(attr, checker);
        assertEquals(empty, checker.getRemains());
    }

    /**
     * Run event notification tests.
     *
     * @param attr     A {@link MultiEventAttr} instance that determines
     *                 behavior of {@link MultiDataStoreListener} instance.
     */
    private void runTest(MultiEventAttr attr) {
        runTest(attr, null);
    }

    /**
     * Run event notification tests.
     *
     * @param attr     A {@link MultiEventAttr} instance that determines
     *                 behavior of {@link MultiDataStoreListener} instance.
     * @param checker  An {@link EventChecker} instance that verifies test
     *                 data.
     */
    private void runTest(MultiEventAttr attr, EventChecker checker) {
        boolean[] bools = {true, false};

        for (boolean merge: bools) {
            TestChangeListener listener = new TestChangeListener(attr);
            Logger logger = listener.getLogger();
            MultiEventCollector collector = new MultiEventCollector(attr);
            Collection<DataTreeModification<Vtn>> changes =
                createEvent(collector, false);

            List<NotifiedEvent<?>> events = collector.getEvents();
            if (checker != null) {
                // Verify test data.
                List<NotifiedEvent<?>> list = new ArrayList<>();
                checker.reset();
                for (NotifiedEvent<?> nev: collector.getEvents()) {
                    if (checker.check(nev)) {
                        list.add(nev);
                    }
                }

                events = list;
            }

            Object ctx = new Object();
            listener.setDataChangeEvent(changes, ctx);
            assertSame(changes, listener.getEvent());
            assertSame(ctx, listener.getContext());

            listener.onDataTreeChanged(changes);
            assertSame(null, listener.getEvent());
            assertSame(null, listener.getContext());

            assertEquals(events, listener.getNotifiedEvents());
            verifyZeroInteractions(logger);
        }
    }

    /**
     * Run event notification tests with specying leaf nodes.
     *
     * @param leaves    A set of data types to be treated as leaf node.
     * @param expected  A set of data types to be expected to be notified.
     */
    private void runLeafTest(Set<Class<?>> leaves,
                             final Set<Class<?>> expected) {
        EventChecker checker = new EventChecker() {
            @Override
            public boolean check(NotifiedEvent<?> nev) {
                Class<?> type = nev.getPath().getTargetType();
                assertEquals(true, expected.contains(type));
                return true;
            }
        };

        MultiEventAttr attr = new MultiEventAttr().setLeafTypes(leaves);
        runTest(attr, checker);
    }

    /**
     * Create data tree modifications for test.
     *
     * @param collector  A {@link MultiEventCollector} instance.
     * @param merge      Use merge operation if {@code true}.
     *                   Use put operation if {@code false}.
     * @return  A colleciton of data tree modifications.
     */
    private Collection<DataTreeModification<Vtn>> createEvent(
        MultiEventCollector collector, boolean merge) {
        VtnTreeBuilder tree = new VtnTreeBuilder();
        VtnTreeBuilder oldTree = new VtnTreeBuilder();

        // Create a new VTN.
        //
        // vtn_1
        //   config
        //   vbridge_1
        //     config
        //     status
        //     vif_1
        //       config
        //       status
        //     vif_2
        //       config
        //       status
        //   vbridge_2
        //     config
        //     status
        //       faulted-path openflow:1 -> openflow:11
        //       faulted-path openflow:2 -> openflow:22
        //   vterminal_1
        //     config
        //     status
        //     vif_1
        //       config
        //       status
        //     vif_2
        //       config
        //       status
        VnodeName tname1 = new VnodeName("vtn_1");
        VtnBuildHelper vtn1 = tree.createVtn(tname1).
            setConfig("VTN 1", null, null);
        InstanceIdentifier<Vtn> vtn1path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname1)).
            build();
        collector.addUpdated(vtn1path, VtnUpdateType.CREATED);

        VbridgeBuildHelper vbr11 = vtn1.createBridge("vbridge_1").
            setConfig("vBridge 1", 1000);
        vbr11.setStatus(VnodeState.UP, 0);
        vbr11.createInterface("vif_1").
            setConfig("Interface 1", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(10L, 1L));
        vbr11.createInterface("vif_2").
            setConfig("Interface 2", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(20L, 2L));

        VbridgeBuildHelper vbr12 = vtn1.createBridge("vbridge_2").
            setConfig("vBridge 2", 600);
        BridgeStatusBuildHelper vbr12st = vbr12.setStatus(VnodeState.DOWN, 2);
        vbr12st.createFaultedPaths(new SalNode(1L), new SalNode(11L));
        vbr12st.createFaultedPaths(new SalNode(2L), new SalNode(22L));

        VterminalBuildHelper vterm11 = vtn1.createTerminal("vterminal_1").
            setConfig("vTerminal 1");
        vterm11.setStatus(VnodeState.UNKNOWN, 0);
        VinterfaceBuildHelper tif111 = vterm11.createInterface("vif_1").
            setConfig(null, false).
            setStatus(VnodeState.DOWN, VnodeState.UNKNOWN, null);
        VinterfaceBuildHelper tif112 = vterm11.createInterface("vif_2").
            setConfig("vTerminal interface 2", true).
            setStatus(VnodeState.UNKNOWN, VnodeState.UNKNOWN, null);

        // Remove a VTN.
        //
        // vtn_2
        //   config
        //   vbridge_1
        //     config
        //     status
        //     vif_1
        //       config
        //       status
        //   vterminal_1
        //     config
        //     status
        //       faulted-path openflow:3 -> openflow:33
        //     vif_1
        //       config
        //       status
        //     vif_2
        //       config
        //       status
        //     vif_3
        //       config
        //       status
        VnodeName tname2 = new VnodeName("vtn_2");
        VtnBuildHelper vtn2 = oldTree.createVtn(tname2).
            setConfig("VTN 2", 3000, 10000);
        InstanceIdentifier<Vtn> vtn2path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname2)).
            build();
        collector.addUpdated(vtn2path, VtnUpdateType.REMOVED);

        VbridgeBuildHelper vbr21 = vtn2.createBridge("vbridge_1").
            setConfig(null, 600);
        vbr21.setStatus(VnodeState.DOWN, 0);
        vbr21.createInterface("vif_1").
            setConfig(null, false).
            setStatus(VnodeState.DOWN, VnodeState.UNKNOWN, null);

        VterminalBuildHelper vterm21 = vtn2.createTerminal("vterminal_1").
            setConfig(null);
        BridgeStatusBuildHelper vterm21st =
            vterm21.setStatus(VnodeState.DOWN, 1);
        vterm21st.createFaultedPaths(new SalNode(3L), new SalNode(33L));

        vterm21.createInterface("vif_1").
            setConfig(null, true).
            setStatus(VnodeState.UP, VnodeState.DOWN, new SalPort(30L, 3L));
        vterm21.createInterface("vif_2").
            setConfig("vTerminal-IF 2", false).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(40L, 4L));
        vterm21.createInterface("vif_3").
            setConfig("vTerminal-IF 3", false).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(50L, 5L));

        // Update a VTN.
        //
        // vtn_3
        //   config  (CHANGED)
        //   vbridge_1
        //     config
        //     status
        //   vbridge_2
        //     config (CHANGED)
        //     status
        //       faulted-path openflow:4 -> openflow:44 (CREATED)
        //       faulted-path openflow:5 -> openflow:55 (REMOVED)
        //       faulted-path openflow:6 -> openflow:66
        //     vif_1
        //       config (CHANGED)
        //       status (CHANGED)
        //     vif_2
        //       config
        //       status
        //   vterminal_1
        //     config
        //     status
        //     vif_1 (CREATED)
        //       config
        //       status
        //     vif_2
        //       config
        //       status (CHANGED)
        //   vterminal_2
        //     config
        //     status
        //     vif_1 (REMOVED)
        //       config
        //       status
        //     vif_2
        //       config (CHANGED)
        //       status (CHANGED)
        VnodeName tname3 = new VnodeName("vtn_3");
        VtnBuildHelper vtn3 = tree.createVtn(tname3).
            setConfig("VTN 3 (CHANGED)", 0, 0);
        VtnBuildHelper oldVtn3 = oldTree.createVtn(tname3).
            setConfig("VTN 3", 6000, 0);
        InstanceIdentifier<Vtn> vtn3path = InstanceIdentifier.
            builder(Vtns.class).
            child(Vtn.class, new VtnKey(tname3)).
            build();
        InstanceIdentifier<VtenantConfig> vtn3cfpath = vtn3path.
            child(VtenantConfig.class);
        collector.addUpdated(vtn3cfpath, VtnUpdateType.CHANGED);

        vtn3.createBridge("vbridge_1").setConfig("vBridge 1", 600);
        oldVtn3.createBridge("vbridge_1").setConfig("vBridge 1", 600);

        VnodeName bname2 = new VnodeName("vbridge_2");
        VbridgeBuildHelper vbr32 = vtn3.createBridge(bname2).
            setConfig("vBridge 2 (CHANGED)", 1000);
        VbridgeBuildHelper oldVbr32 = oldVtn3.createBridge(bname2).
            setConfig("vBridge 2", 300);
        InstanceIdentifier<Vbridge> vbr32path = vtn3path.
            child(Vbridge.class, new VbridgeKey(bname2));
        InstanceIdentifier<VbridgeConfig> vbr32cfpath = vbr32path.
            child(VbridgeConfig.class);
        collector.addUpdated(vbr32cfpath, VtnUpdateType.CHANGED);

        BridgeStatusBuildHelper vbr32st = vbr32.setStatus(VnodeState.DOWN, 2);
        BridgeStatusBuildHelper oldVbr32st =
            oldVbr32.setStatus(VnodeState.DOWN, 2);
        InstanceIdentifier<BridgeStatus> vbr32stpath = vbr32path.
            child(BridgeStatus.class);

        FaultedPathsKey fpkey4 = new FaultedPathsKey(
            new NodeId("openflow:44"), new NodeId("openflow:4"));
        FaultedPathsKey fpkey5 = new FaultedPathsKey(
            new NodeId("openflow:55"), new NodeId("openflow:5"));
        FaultedPathsKey fpkey6 = new FaultedPathsKey(
            new NodeId("openflow:66"), new NodeId("openflow:6"));
        vbr32st.createFaultedPaths(fpkey4);
        vbr32st.createFaultedPaths(fpkey6);
        oldVbr32st.createFaultedPaths(fpkey5);
        oldVbr32st.createFaultedPaths(fpkey6);
        InstanceIdentifier<FaultedPaths> fpath4 = vbr32stpath.
            child(FaultedPaths.class, fpkey4);
        InstanceIdentifier<FaultedPaths> fpath5 = vbr32stpath.
            child(FaultedPaths.class, fpkey5);
        collector.addUpdated(fpath4, VtnUpdateType.CREATED);
        collector.addUpdated(fpath5, VtnUpdateType.REMOVED);

        VnodeName iname1 = new VnodeName("vif_1");
        vbr32.createInterface(iname1).
            setConfig(null, true).
            setStatus(VnodeState.UNKNOWN, VnodeState.UNKNOWN, null);
        oldVbr32.createInterface(iname1).
            setConfig(null, false).
            setStatus(VnodeState.DOWN, VnodeState.UNKNOWN, null);
        InstanceIdentifier<Vinterface> bif321path = vbr32path.
            child(Vinterface.class, new VinterfaceKey(iname1));
        InstanceIdentifier<VinterfaceConfig> bif321cfpath = bif321path.
            child(VinterfaceConfig.class);
        InstanceIdentifier<VinterfaceStatus> bif321stpath = bif321path.
            child(VinterfaceStatus.class);
        collector.addUpdated(bif321cfpath, VtnUpdateType.CHANGED);
        collector.addUpdated(bif321stpath, VtnUpdateType.CHANGED);

        VnodeName iname2 = new VnodeName("vif_2");
        vbr32.createInterface(iname2).
            setConfig("vInterface 2", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(60L, 6L));
        oldVbr32.createInterface(iname2).
            setConfig("vInterface 2", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(60L, 6L));

        VnodeName vtname1 = new VnodeName("vterminal_1");
        VterminalBuildHelper vterm31 = vtn3.createTerminal(vtname1).
            setConfig("vTerminal 1");
        vterm31.setStatus(VnodeState.UP, 0);
        VterminalBuildHelper oldVterm31 = oldVtn3.createTerminal(vtname1).
            setConfig("vTerminal 1");
        oldVterm31.setStatus(VnodeState.UP, 0);
        InstanceIdentifier<Vterminal> vterm31path = vtn3path.
            child(Vterminal.class, new VterminalKey(vtname1));

        vterm31.createInterface(iname1).
            setConfig("vInterface 1", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(70L, 7L));
        InstanceIdentifier<Vinterface> tif311path = vterm31path.
            child(Vinterface.class, new VinterfaceKey(iname1));
        collector.addUpdated(tif311path, VtnUpdateType.CREATED);

        vterm31.createInterface(iname2).
            setConfig("vInterface 2", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(80L, 8L));
        oldVterm31.createInterface(iname2).
            setConfig("vInterface 2", true).
            setStatus(VnodeState.DOWN, VnodeState.DOWN, new SalPort(80L, 8L));
        InstanceIdentifier<VinterfaceStatus> tif312stpath = vterm31path.
            builder().
            child(Vinterface.class, new VinterfaceKey(iname2)).
            child(VinterfaceStatus.class).
            build();
        collector.addUpdated(tif312stpath, VtnUpdateType.CHANGED);

        VnodeName vtname2 = new VnodeName("vterminal_2");
        VterminalBuildHelper vterm32 = vtn3.createTerminal(vtname2).
            setConfig("vTerminal 2");
        vterm32.setStatus(VnodeState.UP, 0);
        VterminalBuildHelper oldVterm32 = oldVtn3.createTerminal(vtname2).
            setConfig("vTerminal 2");
        oldVterm32.setStatus(VnodeState.UP, 0);
        InstanceIdentifier<Vterminal> vterm32path = vtn3path.
            child(Vterminal.class, new VterminalKey(vtname2));

        oldVterm32.createInterface(iname1).
            setConfig("vInterface 1", true).
            setStatus(VnodeState.DOWN, VnodeState.DOWN, new SalPort(90L, 9L));
        InstanceIdentifier<Vinterface> tif321path = vterm32path.
            child(Vinterface.class, new VinterfaceKey(iname1));
        collector.addUpdated(tif321path, VtnUpdateType.REMOVED);

        vterm32.createInterface(iname2).
            setConfig("vInterface 2", true).
            setStatus(VnodeState.UP, VnodeState.UP, new SalPort(100L, 10L));
        oldVterm32.createInterface(iname2).
            setConfig("vInterface 2", false).
            setStatus(VnodeState.DOWN, VnodeState.UNKNOWN, null);
        InstanceIdentifier<Vinterface> tif322path = vterm32path.
            child(Vinterface.class, new VinterfaceKey(iname2));
        InstanceIdentifier<VinterfaceConfig> tif322cfpath = tif322path.
            child(VinterfaceConfig.class);
        InstanceIdentifier<VinterfaceStatus> tif322stpath = tif322path.
            child(VinterfaceStatus.class);
        collector.addUpdated(tif322cfpath, VtnUpdateType.CHANGED);
        collector.addUpdated(tif322stpath, VtnUpdateType.CHANGED);

        tree.freeze();
        oldTree.freeze();
        tree.collectEvents(collector, oldTree);

        return tree.createEvent(oldTree, merge);
    }
}
