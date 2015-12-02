/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;
import static org.mockito.MockitoAnnotations.initMocks;

import static org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContextTest.newStaticEdgePorts;
import static org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContextTest.newStaticSwitchLinks;

import java.io.File;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.slf4j.Logger;

import org.junit.Before;
import org.junit.Test;

import org.mockito.Mock;
import org.mockito.ArgumentCaptor;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.TxQueue;
import org.opendaylight.vtn.manager.internal.TxTask;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticEdgePorts;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLinks;
import org.opendaylight.vtn.manager.internal.util.ChangedData;
import org.opendaylight.vtn.manager.internal.util.CompositeAutoCloseable;
import org.opendaylight.vtn.manager.internal.util.IdentifiedData;
import org.opendaylight.vtn.manager.internal.util.IdentifierTargetComparator;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.concurrent.SettableVTNFuture;
import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticEdgePortsTest;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLinksTest;

import org.opendaylight.controller.md.sal.binding.api.DataBroker;
import org.opendaylight.controller.md.sal.binding.api.DataChangeListener;
import org.opendaylight.controller.md.sal.binding.api.ReadTransaction;
import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataBroker.DataChangeScope;
import org.opendaylight.controller.md.sal.common.api.data.AsyncDataChangeEvent;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.sal.binding.api.RpcProviderRegistry;

import org.opendaylight.yangtools.concepts.ListenerRegistration;
import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePortsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link StaticTopologyManager}.
 */
public class StaticTopologyManagerTest extends TestBase {
    /**
     * Mock-up of {@link VTNManagerProvider}.
     */
    @Mock
    private VTNManagerProvider  vtnProvider;

    /**
     * Mock-up of {@link TxQueue}.
     */
    @Mock
    private TxQueue  txQueue;

    /**
     * Mock-up of {@link DataBroker}.
     */
    @Mock
    private DataBroker  dataBroker;

    /**
     * Mock-up of {@link ListenerRegistration}.
     */
    @Mock
    private ListenerRegistration<DataChangeListener>  registration;

    /**
     * A {@link StaticTopologyManager} instance for test.
     */
    private StaticTopologyManager  testInstance;

    /**
     * Set up test environment.
     */
    @Before
    public void setUp() {
        initMocks(this);

        when(vtnProvider.getDataBroker()).thenReturn(dataBroker);
        when(dataBroker.registerDataChangeListener(
                 any(LogicalDatastoreType.class), any(InstanceIdentifier.class),
                 any(StaticTopologyManager.class), any(DataChangeScope.class))).
            thenReturn(registration);
        testInstance = new StaticTopologyManager(vtnProvider, txQueue);
    }

    /**
     * Test case for {@link StaticTopologyManager#StaticTopologyManager(VTNManagerProvider, TxQueue)}.
     */
    @Test
    public void testConstructor() {
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        DataChangeScope scope = DataChangeScope.SUBTREE;
        InstanceIdentifier<VtnStaticTopology> path = getPath();
        verify(dataBroker).
            registerDataChangeListener(config, path, testInstance, scope);
        verifyZeroInteractions(txQueue, registration);
        assertEquals(VtnStaticTopology.class, testInstance.getTargetType());
    }

    /**
     * Test case for {@link StaticTopologyManager#getComparator()}.
     */
    @Test
    public void testGetComparator() {
        IdentifierTargetComparator comp = testInstance.getComparator();
        assertTrue(comp.getOrder(StaticEdgePort.class).intValue() <
                   comp.getOrder(StaticSwitchLink.class).intValue());
    }

    /**
     * Test case for {@link StaticTopologyManager#getOrder(VtnUpdateType)}.
     */
    @Test
    public void testGetOrder() {
        for (VtnUpdateType type: VtnUpdateType.values()) {
            assertEquals(true, testInstance.getOrder(type));
        }
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#enterEvent(AsyncDataChangeEvent)}.
     */
    @Test
    public void testEnterEvent() {
        AsyncDataChangeEvent<InstanceIdentifier<?>, DataObject> ev = null;
        StaticLinkUpdateTask task = testInstance.enterEvent(ev);
        assertTrue(task instanceof StaticLinkUpdateTask);
        StaticLinkUpdateTask task1 = testInstance.enterEvent(ev);
        assertTrue(task1 instanceof StaticLinkUpdateTask);
        assertNotSame(task, task1);
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#exitEvent(StaticLinkUpdateTask)}.
     */
    @Test
    public void testExitEvent() {
        reset(txQueue);
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);
        testInstance.exitEvent(task);
        verify(txQueue).post(task);
        verifyNoMoreInteractions(txQueue);
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#onCreated(StaticLinkUpdateTask, IdentifiedData)}.
     */
    @Test
    public void testOnCreated() {
        testOnCreatedOrRemoved(true);
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#onUpdated(StaticLinkUpdateTask,ChangedData)}.
     */
    @Test
    public void testOnUpdated() {
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);
        Set<SalPort> expected = Collections.<SalPort>emptySet();

        // In case of unwanted events.
        for (ChangedData<?> data: createUnwantedChangedData()) {
            testInstance.onUpdated(task, data);
            assertEquals(expected, task.getUpdatedPorts());
        }
        verifyZeroInteractions(logger);

        SalPort src = new SalPort(567L, 890L);
        SalPort oldDst = new SalPort(999L, 333L);
        SalPort newDst = new SalPort(10L, 11L);
        NodeConnectorId srcId = src.getNodeConnectorId();
        NodeConnectorId oldDstId = oldDst.getNodeConnectorId();
        NodeConnectorId newDstId = newDst.getNodeConnectorId();
        StaticSwitchLink old = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(oldDstId).build();
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(newDstId).build();
        assertEquals(swlink.getKey(), old.getKey());
        InstanceIdentifier<StaticSwitchLink> path = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            child(StaticSwitchLink.class, swlink.getKey()).
            build();
        ChangedData<StaticSwitchLink> data =
            new ChangedData<>(path, swlink, old);
        expected = new HashSet<>();
        expected.add(src);
        expected.add(oldDst);

        testInstance.onUpdated(task, data);
        assertEquals(expected, task.getUpdatedPorts());
        verifyZeroInteractions(logger);
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#onRemoved(StaticLinkUpdateTask, IdentifiedData)}.
     */
    @Test
    public void testOnRemoved() {
        testOnCreatedOrRemoved(false);
    }

    /**
     * Test case for {@link StaticTopologyManager#getWildcardPath()}.
     */
    @Test
    public void testGetWildcardPath() {
        assertEquals(getPath(), testInstance.getWildcardPath());
    }

    /**
     * Test case for {@link StaticTopologyManager#getLogger()}.
     */
    @Test
    public void testGetLogger() {
        Logger logger = testInstance.getLogger();
        assertEquals(StaticTopologyManager.class.getName(), logger.getName());
    }

    /**
     * Test case for {@link StaticTopologyManager#close()}.
     */
    @Test
    public void testClose() {
        verifyZeroInteractions(registration);
        testInstance.close();
        verify(registration).close();
    }

    /**
     * Test case for {@link StaticTopologyManager#initConfig(boolean)}.
     *
     * <p>
     *   Try to load configuration from files, but no configuration is present.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInitConfigNoConfig() throws Exception {
        XmlConfigFile.init();
        File[] files = {
            XmlStaticSwitchLinksTest.getConfigFile(),
            XmlStaticEdgePortsTest.getConfigFile(),
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        reset(vtnProvider);
        VTNFuture<VtnStaticTopology> future = new SettableVTNFuture<>();
        when(vtnProvider.post(any(TxTask.class))).thenReturn(future);
        assertSame(future, testInstance.initConfig(true));

        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = ArgumentCaptor.forClass(TxTask.class);
        verify(vtnProvider).post(captor.capture());
        verifyNoMoreInteractions(vtnProvider);

        List<TxTask> captured = captor.getAllValues();
        assertEquals(1, captured.size());

        @SuppressWarnings("unchecked")
        TxTask<VtnStaticTopology> task = captured.get(0);

        // Configure configurations into transaction mock.
        // They should be deleted.
        InstanceIdentifier<StaticSwitchLinks> linkPath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            build();
        StaticSwitchLinks swlinks = new StaticSwitchLinksBuilder().build();
        InstanceIdentifier<StaticEdgePorts> edgePath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            build();
        StaticEdgePorts edges = new StaticEdgePortsBuilder().build();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        when(tx.read(config, linkPath)).thenReturn(getReadResult(swlinks));
        when(tx.read(config, edgePath)).thenReturn(getReadResult(edges));

        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);

        // Create dummy configuration files that should be deleted.
        for (File f: files) {
            FileWriter writer = new FileWriter(f);
            writer.write("bad config\n");
            writer.close();
            assertTrue(f.isFile());
        }

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().build();
        assertEquals(vstopo, task.execute(ctx, 0));
        verify(ctx).getReadWriteTransaction();
        verifyNoMoreInteractions(ctx);

        // Broken configuration files should be deleted.
        task.onSuccess(vtnProvider, vstopo);
        for (File f: files) {
            assertFalse(f.exists());
        }

        // Configurations should be deleted from the config DS.
        verify(tx).read(config, linkPath);
        verify(tx).read(config, edgePath);
        verify(tx).delete(config, linkPath);
        verify(tx).delete(config, edgePath);
        verifyNoMoreInteractions(tx);
    }

    /**
     * Test case for {@link StaticTopologyManager#initConfig(boolean)}.
     *
     * <p>
     *   Load configuration from files.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInitConfigLoad() throws Exception {
        XmlConfigFile.init();
        File linkFile = XmlStaticSwitchLinksTest.getConfigFile();
        File edgeFile = XmlStaticEdgePortsTest.getConfigFile();
        File[] files = {
            linkFile,
            edgeFile,
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        reset(vtnProvider);
        VTNFuture<VtnStaticTopology> future = new SettableVTNFuture<>();
        when(vtnProvider.post(any(TxTask.class))).thenReturn(future);
        assertSame(future, testInstance.initConfig(true));

        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = ArgumentCaptor.forClass(TxTask.class);
        verify(vtnProvider).post(captor.capture());
        verifyNoMoreInteractions(vtnProvider);

        List<TxTask> captured = captor.getAllValues();
        assertEquals(1, captured.size());

        @SuppressWarnings("unchecked")
        TxTask<VtnStaticTopology> task = captured.get(0);

        // Create configuration files.
        StaticSwitchLink swlink1 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:1:2")).
            setDestination(new NodeConnectorId("openflow:3:4")).
            build();
        StaticSwitchLink swlink2 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:111:222")).
            setDestination(new NodeConnectorId("openflow:333:444")).
            build();
        StaticSwitchLink swlink3 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:9999:1234")).
            setDestination(new NodeConnectorId("openflow:12345:6789")).
            build();
        StaticSwitchLinks swlinks =
            newStaticSwitchLinks(swlink1, swlink2, swlink3);
        XmlStaticSwitchLinks xswlinks = new XmlStaticSwitchLinks();
        xswlinks.save(swlinks);
        assertTrue(linkFile.isFile());

        SalPort edge1 = new SalPort(1L, 9L);
        SalPort edge2 = new SalPort(123L, 7777L);
        SalPort edge3 = new SalPort(7812394L, 1L);
        SalPort edge4 = new SalPort(98765L, 43210L);
        StaticEdgePorts edgePorts =
            newStaticEdgePorts(edge1, edge2, edge3, edge4);
        XmlStaticEdgePorts xedges = new XmlStaticEdgePorts();
        xedges.save(edgePorts);
        assertTrue(edgeFile.isFile());

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);

        VtnStaticTopology vstopo = task.execute(ctx, 0);
        assertEqualsStaticSwitchLinks(swlinks, vstopo.getStaticSwitchLinks());
        assertEqualsStaticEdgePorts(edgePorts, vstopo.getStaticEdgePorts());
        verify(ctx).getReadWriteTransaction();
        verifyNoMoreInteractions(ctx);

        // Configuration files should be still present.
        task.onSuccess(vtnProvider, vstopo);
        for (File f: files) {
            assertTrue(f.isFile());
        }

        // Configurations should be put into the config DS.
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        InstanceIdentifier<StaticSwitchLinks> linkPath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            build();
        InstanceIdentifier<StaticEdgePorts> edgePath = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            build();

        verify(tx).put(config, linkPath, vstopo.getStaticSwitchLinks(), true);
        verify(tx).put(config, edgePath, vstopo.getStaticEdgePorts(), true);
        verifyNoMoreInteractions(tx);
    }

    /**
     * Test case for {@link StaticTopologyManager#initConfig(boolean)}.
     *
     * <p>
     *   Try to save configuration into files, but no configuration is present.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInitConfigDelete() throws Exception {
        XmlConfigFile.init();
        File[] files = {
            XmlStaticSwitchLinksTest.getConfigFile(),
            XmlStaticEdgePortsTest.getConfigFile(),
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        reset(vtnProvider);
        VTNFuture<VtnStaticTopology> future = new SettableVTNFuture<>();
        when(vtnProvider.post(any(TxTask.class))).thenReturn(future);
        assertSame(future, testInstance.initConfig(false));

        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = ArgumentCaptor.forClass(TxTask.class);
        verify(vtnProvider).post(captor.capture());
        verifyNoMoreInteractions(vtnProvider);

        List<TxTask> captured = captor.getAllValues();
        assertEquals(1, captured.size());

        @SuppressWarnings("unchecked")
        TxTask<VtnStaticTopology> task = captured.get(0);

        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<VtnStaticTopology> path = getPath();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        VtnStaticTopology vstopo = null;
        when(rtx.read(config, path)).thenReturn(getReadResult(vstopo));

        TxContext ctx = mock(TxContext.class);
        when(ctx.getTransaction()).thenReturn(rtx);

        // Create dummy configuration files that should be deleted.
        for (File f: files) {
            FileWriter writer = new FileWriter(f);
            writer.write("bad config\n");
            writer.close();
            assertTrue(f.isFile());
        }

        assertEquals(vstopo, task.execute(ctx, 0));
        verify(ctx).getTransaction();
        verifyNoMoreInteractions(ctx);

        verify(rtx).read(config, path);
        verifyNoMoreInteractions(rtx);

        // Configuration files should be deleted.
        task.onSuccess(vtnProvider, vstopo);
        for (File f: files) {
            assertFalse(f.exists());
        }
    }

    /**
     * Test case for {@link StaticTopologyManager#initConfig(boolean)}.
     *
     * <p>
     *   Save current configurations into files.
     * </p>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testInitConfigSave() throws Exception {
        XmlConfigFile.init();
        File linkFile = XmlStaticSwitchLinksTest.getConfigFile();
        File edgeFile = XmlStaticEdgePortsTest.getConfigFile();
        File[] files = {
            linkFile,
            edgeFile,
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        reset(vtnProvider);
        VTNFuture<VtnStaticTopology> future = new SettableVTNFuture<>();
        when(vtnProvider.post(any(TxTask.class))).thenReturn(future);
        assertSame(future, testInstance.initConfig(false));

        @SuppressWarnings("unchecked")
        ArgumentCaptor<TxTask> captor = ArgumentCaptor.forClass(TxTask.class);
        verify(vtnProvider).post(captor.capture());
        verifyNoMoreInteractions(vtnProvider);

        List<TxTask> captured = captor.getAllValues();
        assertEquals(1, captured.size());

        @SuppressWarnings("unchecked")
        TxTask<VtnStaticTopology> task = captured.get(0);

        // Create configurations to be saved.
        ReadTransaction rtx = mock(ReadTransaction.class);
        InstanceIdentifier<VtnStaticTopology> path = getPath();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;

        StaticSwitchLink swlink1 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:1:2")).
            setDestination(new NodeConnectorId("openflow:3:4")).
            build();
        StaticSwitchLink swlink2 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:5:6")).
            setDestination(new NodeConnectorId("openflow:7:8")).
            build();
        StaticSwitchLink swlink3 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:9999:1000")).
            setDestination(new NodeConnectorId("openflow:12345:6789")).
            build();
        StaticSwitchLinks swlinks =
            newStaticSwitchLinks(swlink1, swlink2, swlink3);

        SalPort edge1 = new SalPort(1L, 9L);
        SalPort edge2 = new SalPort(123L, 45L);
        SalPort edge3 = new SalPort(333L, 444L);
        SalPort edge4 = new SalPort(98765L, 43210L);
        StaticEdgePorts edgePorts =
            newStaticEdgePorts(edge1, edge2, edge3, edge4);

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlinks).
            setStaticEdgePorts(edgePorts).
            build();
        when(rtx.read(config, path)).thenReturn(getReadResult(vstopo));

        TxContext ctx = mock(TxContext.class);
        when(ctx.getTransaction()).thenReturn(rtx);

        assertEquals(vstopo, task.execute(ctx, 0));
        verify(ctx).getTransaction();
        verifyNoMoreInteractions(ctx);

        verify(rtx).read(config, path);
        verifyNoMoreInteractions(rtx);

        // Configurations should be saved into files.
        task.onSuccess(vtnProvider, vstopo);
        for (File f: files) {
            assertTrue(f.isFile());
        }

        XmlConfigFile.Type ctype = XmlConfigFile.Type.TOPOLOGY;
        String key = XmlStaticSwitchLinksTest.CONFIG_KEY;
        XmlStaticSwitchLinks xswlinks = XmlConfigFile.
            load(ctype, key, XmlStaticSwitchLinks.class);
        assertEquals(new XmlStaticSwitchLinks(swlinks), xswlinks);

        key = XmlStaticEdgePortsTest.CONFIG_KEY;
        XmlStaticEdgePorts xedges = XmlConfigFile.
            load(ctype, key, XmlStaticEdgePorts.class);
        assertEquals(new XmlStaticEdgePorts(edgePorts), xedges);
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#initRpcServices(RpcProviderRegistry, CompositeAutoCloseable)}.
     */
    @Test
    public void testInitRpcServices() {
        reset(vtnProvider, txQueue, dataBroker, registration);

        // This should do nothing.
        RpcProviderRegistry rpcReg = mock(RpcProviderRegistry.class);
        CompositeAutoCloseable regs = null;
        testInstance.initRpcServices(null, null);
        verifyZeroInteractions(vtnProvider, txQueue, dataBroker, registration,
                               rpcReg);
    }

    /**
     * Test case for
     * {@link StaticTopologyManager#saveConfig(VtnStaticTopology)}.
     */
    @Test
    public void testSaveConfig() {
        XmlConfigFile.init();
        File linkFile = XmlStaticSwitchLinksTest.getConfigFile();
        File edgeFile = XmlStaticEdgePortsTest.getConfigFile();
        assertFalse(linkFile.exists());
        assertFalse(edgeFile.exists());

        // Create test data.
        List<StaticSwitchLinks> linkList = new ArrayList<>();
        linkList.add(null);
        linkList.add(new StaticSwitchLinksBuilder().build());

        StaticSwitchLink swlink1 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:1:2")).
            setDestination(new NodeConnectorId("openflow:3:4")).
            build();
        StaticSwitchLink swlink2 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:111:222")).
            setDestination(new NodeConnectorId("openflow:333:444")).
            build();
        StaticSwitchLink swlink3 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:9999:1234")).
            setDestination(new NodeConnectorId("openflow:12345:6789")).
            build();
        linkList.add(newStaticSwitchLinks(swlink1, swlink2, swlink3));

        List<StaticEdgePorts> edgeList = new ArrayList<>();
        edgeList.add(null);
        edgeList.add(new StaticEdgePortsBuilder().build());
        SalPort edge1 = new SalPort(1L, 9L);
        SalPort edge2 = new SalPort(123L, 7777L);
        SalPort edge3 = new SalPort(7812394L, 1L);
        SalPort edge4 = new SalPort(98765L, 43210L);
        edgeList.add(newStaticEdgePorts(edge1, edge2, edge3, edge4));

        XmlConfigFile.Type ctype = XmlConfigFile.Type.TOPOLOGY;
        for (StaticSwitchLinks swlinks: linkList) {
            for (StaticEdgePorts edgePorts: edgeList) {
                VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
                    setStaticSwitchLinks(swlinks).
                    setStaticEdgePorts(edgePorts).
                    build();

                StaticTopologyManager.saveConfig(vstopo);
                if (swlinks == null) {
                    assertFalse(linkFile.exists());
                } else {
                    assertTrue(linkFile.isFile());
                    String key = XmlStaticSwitchLinksTest.CONFIG_KEY;
                    XmlStaticSwitchLinks xswlinks = XmlConfigFile.
                        load(ctype, key, XmlStaticSwitchLinks.class);
                    StaticSwitchLinks expected = swlinks;
                    List<StaticSwitchLink> list = swlinks.getStaticSwitchLink();
                    if (list == null || list.isEmpty()) {
                        expected = new StaticSwitchLinksBuilder().build();
                    }
                    assertEqualsStaticSwitchLinks(expected,
                                                  xswlinks.getConfig());
                }

                if (edgePorts == null) {
                    assertFalse(edgeFile.exists());
                } else {
                    assertTrue(edgeFile.isFile());
                    String key = XmlStaticEdgePortsTest.CONFIG_KEY;
                    XmlStaticEdgePorts xedges = XmlConfigFile.
                        load(ctype, key, XmlStaticEdgePorts.class);
                    StaticEdgePorts expected = edgePorts;
                    List<StaticEdgePort> list = edgePorts.getStaticEdgePort();
                    if (list == null || list.isEmpty()) {
                        expected = new StaticEdgePortsBuilder().build();
                    }
                    assertEqualsStaticEdgePorts(expected, xedges.getConfig());
                }
            }
        }

        // This should delete both configuration files.
        StaticTopologyManager.saveConfig((VtnStaticTopology)null);
        assertFalse(linkFile.exists());
        assertFalse(edgeFile.exists());
    }

    /**
     * Return a wildcard path to the MD-SAL data model to listen.
     *
     * @return  A wildcard path to the MD-SAL data model to listen.
     */
    private InstanceIdentifier<VtnStaticTopology> getPath() {
        return InstanceIdentifier.create(VtnStaticTopology.class);
    }

    /**
     * Create a list of {@link IdentifiedData} instances that will be ignored
     * by {@link StaticTopologyManager}.
     *
     * @return  A list of {@link IdentifiedData} instances.
     */
    private List<IdentifiedData<?>> createUnwantedIdentifiedData() {
        InstanceIdentifier<StaticEdgePorts> path1 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            build();
        IdentifiedData<StaticEdgePorts> data1 =
            new IdentifiedData<>(path1, new StaticEdgePortsBuilder().build());

        InstanceIdentifier<StaticSwitchLinks> path2 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            build();
        IdentifiedData<StaticSwitchLinks> data2 =
            new IdentifiedData<>(path2, new StaticSwitchLinksBuilder().build());

        InstanceIdentifier<VtnStaticTopology> path3 = getPath();
        IdentifiedData<VtnStaticTopology> data3 =
            new IdentifiedData<>(path3, new VtnStaticTopologyBuilder().build());

        List<IdentifiedData<?>> list = new ArrayList<>();
        Collections.addAll(list, data1, data2, data3);

        return list;
    }

    /**
     * Create a list of {@link ChangedData} instances that will be ignored
     * by {@link StaticTopologyManager}.
     *
     * @return  A list of {@link ChangedData} instances.
     */
    private List<ChangedData<?>> createUnwantedChangedData() {
        InstanceIdentifier<StaticEdgePorts> path1 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            build();
        ChangedData<StaticEdgePorts> data1 = new ChangedData<>(
            path1, new StaticEdgePortsBuilder().build(),
            new StaticEdgePortsBuilder().build());

        InstanceIdentifier<StaticSwitchLinks> path2 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            build();
        ChangedData<StaticSwitchLinks> data2 = new ChangedData<>(
            path2, new StaticSwitchLinksBuilder().build(),
            new StaticSwitchLinksBuilder().build());

        InstanceIdentifier<VtnStaticTopology> path3 = getPath();
        ChangedData<VtnStaticTopology> data3 = new ChangedData<>(
            path3, new VtnStaticTopologyBuilder().build(),
            new VtnStaticTopologyBuilder().build());

        // StaticEdgePort should not be reported by changed event because
        // it has only one field.
        SalPort sport = new SalPort(3333L, 4444L);
        NodeConnectorId ncId = sport.getNodeConnectorId();
        StaticEdgePort ep = new StaticEdgePortBuilder().
            setPort(ncId).build();
        InstanceIdentifier<StaticEdgePort> path4 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class, ep.getKey()).
            build();
        ChangedData<StaticEdgePort> data4 = new ChangedData<>(path4, ep, ep);

        List<ChangedData<?>> list = new ArrayList<>();
        Collections.addAll(list, data1, data2, data3, data4);

        return list;
    }

    /**
     * Common test for creation and removal event.
     *
     * @param create  {@code true} indicates the test for creation event.
     *                {@code false} indicates the test for removal event.
     */
    private void testOnCreatedOrRemoved(boolean create) {
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);
        Set<SalPort> expected = Collections.<SalPort>emptySet();

        // In case of unwanted events.
        for (IdentifiedData<?> data: createUnwantedIdentifiedData()) {
            if (create) {
                testInstance.onCreated(task, data);
            } else {
                testInstance.onRemoved(task, data);
            }
            assertEquals(expected, task.getUpdatedPorts());
        }
        verifyZeroInteractions(logger);

        // In case of static edge port.
        SalPort sport1 = new SalPort(10L, 20L);
        NodeConnectorId ncId1 = sport1.getNodeConnectorId();
        StaticEdgePort ep = new StaticEdgePortBuilder().
            setPort(ncId1).build();
        InstanceIdentifier<StaticEdgePort> path1 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            child(StaticEdgePort.class, ep.getKey()).
            build();
        IdentifiedData<StaticEdgePort> data1 = new IdentifiedData<>(path1, ep);
        expected = new HashSet<>();
        expected.add(sport1);

        if (create) {
            testInstance.onCreated(task, data1);
        } else {
            testInstance.onRemoved(task, data1);
        }
        assertEquals(expected, task.getUpdatedPorts());
        verifyZeroInteractions(logger);

        // In case of static inter-switch link.
        SalPort sport2 = new SalPort(1L, 2L);
        SalPort sport3 = new SalPort(123L, 456L);
        NodeConnectorId ncId2 = sport2.getNodeConnectorId();
        NodeConnectorId ncId3 = sport3.getNodeConnectorId();
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(ncId2).setDestination(ncId3).build();
        InstanceIdentifier<StaticSwitchLink> path2 = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            child(StaticSwitchLink.class, swlink.getKey()).
            build();
        IdentifiedData<StaticSwitchLink> data2 =
            new IdentifiedData<>(path2, swlink);
        expected.add(sport2);

        if (create) {
            testInstance.onCreated(task, data2);
        } else {
            testInstance.onRemoved(task, data2);
        }
        assertEquals(expected, task.getUpdatedPorts());
        verifyZeroInteractions(logger);
    }
}
