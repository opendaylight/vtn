/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.verifyZeroInteractions;
import static org.mockito.Mockito.when;

import static org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContextTest.newStaticEdgePorts;
import static org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContextTest.newStaticSwitchLinks;

import java.io.File;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import com.google.common.base.Optional;

import org.slf4j.Logger;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.TxContext;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticEdgePorts;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLinks;
import org.opendaylight.vtn.manager.internal.util.VTNEntityType;
import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.LinkUpdateContext;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticEdgePortsTest;
import org.opendaylight.vtn.manager.internal.inventory.xml.XmlStaticSwitchLinksTest;
import org.opendaylight.vtn.manager.internal.util.inventory.InterSwitchLink;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;
import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.IgnoredLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.ignored.links.IgnoredLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.topology.rev150209.vtn.topology.VtnLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link StaticLinkUpdateTask}.
 */
public class StaticLinkUpdateTaskTest extends TestBase {
    /**
     * Test case for {@link StaticLinkUpdateTask#StaticLinkUpdateTask(Logger)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);

        assertEquals(Collections.<SalPort>emptySet(), task.getUpdatedPorts());
        assertEquals(logger, getFieldValue(task, Logger.class, "logger"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        verifyZeroInteractions(logger);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#addUpdated(StaticSwitchLink)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddUpdatedStaticSwitchLink1() throws Exception {
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);

        // Unsupported port should be ignored.
        Set<SalPort> expected = new HashSet<>();
        NodeConnectorId srcId = new NodeConnectorId("unknown:1:2");
        NodeConnectorId dstId = new NodeConnectorId("openflow:22:33");
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(dstId).build();
        task.addUpdated(swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        // Broken configuration should be ignored.
        swlink = new StaticSwitchLinkBuilder().build();
        task.addUpdated(swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        SalPort src = new SalPort(123L, 456L);
        srcId = src.getNodeConnectorId();
        swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(dstId).build();
        assertTrue(expected.add(src));
        task.addUpdated(swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        // Same port will be ignored.
        dstId = new NodeConnectorId("openflow:10:20");
        swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(dstId).build();
        task.addUpdated(swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        src = new SalPort(12345L, 6789L);
        srcId = src.getNodeConnectorId();
        swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(dstId).build();
        assertTrue(expected.add(src));
        task.addUpdated(swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        verifyZeroInteractions(logger);
    }

    /**
     * Test case for
     * {@link StaticLinkUpdateTask#addUpdated(StaticSwitchLink, StaticSwitchLink)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddUpdatedStaticSwitchLink2() throws Exception {
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);

        // Unsupported port should be ignored.
        Set<SalPort> expected = new HashSet<>();
        NodeConnectorId srcId = new NodeConnectorId("unknown:1:2");
        NodeConnectorId newDstId = new NodeConnectorId("openflow:22:33");
        NodeConnectorId oldDstId = new NodeConnectorId("unknown:3:4");
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(newDstId).build();
        StaticSwitchLink old = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(oldDstId).build();
        task.addUpdated(old, swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        // Broken configuration should be ignored.
        swlink = new StaticSwitchLinkBuilder().build();
        task.addUpdated(swlink, swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        SalPort src = new SalPort(123L, 456L);
        SalPort oldDst = new SalPort(111L, 222L);
        srcId = src.getNodeConnectorId();
        oldDstId = oldDst.getNodeConnectorId();
        swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(newDstId).build();
        old = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(oldDstId).build();
        Collections.addAll(expected, src, oldDst);
        task.addUpdated(old, swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        // Same port will be ignored.
        newDstId = new NodeConnectorId("openflow:10:20");
        swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(newDstId).build();
        task.addUpdated(old, swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        src = new SalPort(12345L, 6789L);
        oldDst = new SalPort(333L, 444L);
        srcId = src.getNodeConnectorId();
        oldDstId = oldDst.getNodeConnectorId();
        swlink = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(newDstId).build();
        old = new StaticSwitchLinkBuilder().
            setSource(srcId).setDestination(oldDstId).build();
        Collections.addAll(expected, src, oldDst);
        task.addUpdated(old, swlink);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        verifyZeroInteractions(logger);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#addUpdated(StaticEdgePort)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testAddUpdatedStaticEdgePort() throws Exception {
        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);

        // Unsupported port should be ignored.
        Set<SalPort> expected = new HashSet<>();
        StaticEdgePort ep = new StaticEdgePortBuilder().
            setPort(new NodeConnectorId("unknown:1:2")).build();
        task.addUpdated(ep);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        // Broken configuration should be ignored.
        ep = new StaticEdgePortBuilder().build();
        task.addUpdated(ep);
        assertEquals(expected, task.getUpdatedPorts());
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        SalPort[] ports = {
            new SalPort(123L, 456L),
            new SalPort(11L, 22L),
            new SalPort(11L, 23L),
        };
        for (SalPort sport: ports) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            ep = new StaticEdgePortBuilder().setPort(ncId).build();
            assertTrue(expected.add(sport));
            task.addUpdated(ep);
            assertEquals(expected, task.getUpdatedPorts());
            assertEquals(Boolean.FALSE,
                         getFieldValue(task, Boolean.class, "saveLinks"));
            assertEquals(Boolean.TRUE,
                         getFieldValue(task, Boolean.class, "saveEdges"));
            assertEquals(null,
                         getFieldValue(task, Optional.class, "staticTopology"));
        }

        // Same port will be ignored.
        for (SalPort sport: ports) {
            NodeConnectorId ncId = sport.getNodeConnectorId();
            ep = new StaticEdgePortBuilder().setPort(ncId).build();
            task.addUpdated(ep);
            assertEquals(expected, task.getUpdatedPorts());
            assertEquals(Boolean.FALSE,
                         getFieldValue(task, Boolean.class, "saveLinks"));
            assertEquals(Boolean.TRUE,
                         getFieldValue(task, Boolean.class, "saveEdges"));
            assertEquals(null,
                         getFieldValue(task, Optional.class, "staticTopology"));
        }

        verifyZeroInteractions(logger);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onSuccess(VTNManagerProvider, LinkUpdateContext)}.
     *
     * <ul>
     *   <li>
     *     The process is the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     No configuration is present.
     *   </li>
     *   <li>
     *     No updated ports.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteNoConfig() throws Exception {
        testExecuteNoConfig(true);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onSuccess(VTNManagerProvider, LinkUpdateContext)}.
     *
     * <ul>
     *   <li>
     *     The process is not the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     No configuration is present.
     *   </li>
     *   <li>
     *     No updated ports.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteNoConfigNotOwner() throws Exception {
        testExecuteNoConfig(false);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onSuccess(VTNManagerProvider, LinkUpdateContext)}.
     *
     * <ul>
     *   <li>
     *     The process is the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     Only static link configuration is updated.
     *   </li>
     *   <li>
     *     None of the updated ports are present.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteNoPort() throws Exception {
        testExecuteNoPort(true);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onSuccess(VTNManagerProvider, LinkUpdateContext)}.
     *
     * <ul>
     *   <li>
     *     The process is not the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     Only static link configuration is updated.
     *   </li>
     *   <li>
     *     None of the updated ports are present.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteNoPortNotOwner() throws Exception {
        testExecuteNoPort(false);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onSuccess(VTNManagerProvider, LinkUpdateContext)}.
     *
     * <ul>
     *   <li>
     *     The process is the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     Static inter-switch link is updated, and new static link is
     *     established.
     *   </li>
     *   <li>
     *     Static edge port is updated, and that port is configured as an
     *     edge port.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteUpdated() throws Exception {
        testExecuteUpdated(true);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onSuccess(VTNManagerProvider, LinkUpdateContext)}.
     *
     * <ul>
     *   <li>
     *     The process is not the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     Static inter-switch link is updated, and new static link is
     *     established.
     *   </li>
     *   <li>
     *     Static edge port is updated, and that port is configured as an
     *     edge port.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteUpdatedNotOwner() throws Exception {
        testExecuteUpdated(false);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onFailure(VTNManagerProvider, Throwable)}.
     *
     * <ul>
     *   <li>
     *     Both static inter-switch link and static edge are updated.
     *   </li>
     *   <li>
     *     Failed to read the static network topology from the config DS.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecuteConfigReadFailed() throws Exception {
        XmlConfigFile.init();
        File[] files = {
            XmlStaticSwitchLinksTest.getConfigFile(),
            XmlStaticEdgePortsTest.getConfigFile(),
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);

        // Throw an exception on configuration read operation.
        IllegalStateException cause = new IllegalStateException();
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        InstanceIdentifier<VtnStaticTopology> path =
            InstanceIdentifier.create(VtnStaticTopology.class);
        when(tx.read(config, path)).
            thenReturn(getReadFailure(VtnStaticTopology.class, cause));

        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);

        SalPort src = new SalPort(12L, 34L);
        SalPort newDst = new SalPort(56L, 78L);
        SalPort oldDst = new SalPort(90L, 12L);
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(src.getNodeConnectorId()).
            setDestination(newDst.getNodeConnectorId()).
            build();
        StaticSwitchLink old = new StaticSwitchLinkBuilder().
            setSource(src.getNodeConnectorId()).
            setDestination(oldDst.getNodeConnectorId()).
            build();

        Set<SalPort> updatedPorts = new HashSet<>();
        Collections.addAll(updatedPorts, src, oldDst);
        task.addUpdated(old, swlink);

        SalPort[] edges = {
            new SalPort(1L, 2L),
            new SalPort(3L, 4L),
        };
        for (SalPort edge: edges) {
            StaticEdgePort ep = new StaticEdgePortBuilder().
                setPort(edge.getNodeConnectorId()).build();
            task.addUpdated(ep);
            updatedPorts.add(edge);
        }
        assertEquals(updatedPorts, task.getUpdatedPorts());

        Throwable caught = null;
        try {
            task.execute(ctx);
            unexpected();
        } catch (VTNException e) {
            caught = e;
            Throwable t = e.getCause();
            assertTrue(t instanceof ReadFailedException);
            assertEquals(cause, t.getCause());
        }

        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(null,
                     getFieldValue(task, Optional.class, "staticTopology"));

        verify(ctx).getReadWriteTransaction();
        verify(tx).read(config, path);
        verifyNoMoreInteractions(ctx, tx);

        task.onFailure(null, caught);
        verify(logger).
            error("Failed to update VTN static network topology.", caught);
        verifyNoMoreInteractions(logger);

        // Configuration should not be saved.
        for (File f: files) {
            assertFalse(f.exists());
        }
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onFailure(VTNManagerProvider, Throwable)}.
     *
     * <ul>
     *   <li>
     *     The process is the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     Only static edge port configuration is updated.
     *   </li>
     *   <li>
     *     Failed to read the updated VTN port from the operational DS.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecutePortReadFailed() throws Exception {
        testExecutePortReadFailed(true);
    }

    /**
     * Test case for {@link StaticLinkUpdateTask#execute(TxContext)} and
     * {@link StaticLinkUpdateTask#onFailure(VTNManagerProvider, Throwable)}.
     *
     * <ul>
     *   <li>
     *     The process is not the owner of the VTN inventory.
     *   </li>
     *   <li>
     *     Only static edge port configuration is updated.
     *   </li>
     *   <li>
     *     Failed to read the updated VTN port from the operational DS.
     *   </li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testExecutePortReadFailedNotOwner() throws Exception {
        testExecutePortReadFailed(false);
    }

    /**
     * Common test for {@link #testExecuteNoConfig()} and
     * {@link #testExecuteNoConfigNotOwner()}.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testExecuteNoConfig(boolean owner) throws Exception {
        XmlConfigFile.init();
        File[] files = {
            XmlStaticSwitchLinksTest.getConfigFile(),
            XmlStaticEdgePortsTest.getConfigFile(),
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        when(ctx.getReadSpecific(InventoryReader.class)).thenReturn(reader);

        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        when(ctx.getProvider()).thenReturn(provider);

        // No configuration is present in the config DS.
        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        InstanceIdentifier<VtnStaticTopology> path =
            InstanceIdentifier.create(VtnStaticTopology.class);
        VtnStaticTopology vstopo = null;
        when(tx.read(config, path)).thenReturn(getReadResult(vstopo));

        // No ignored links.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        IgnoredLinks ignored = null;
        when(tx.read(oper, igPath)).thenReturn(getReadResult(ignored));

        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);
        when(logger.isDebugEnabled()).thenReturn(true);

        LinkUpdateContext luctx = task.execute(ctx);
        if (owner) {
            assertNotNull(luctx);
        } else {
            assertEquals(null, luctx);
        }

        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(Optional.fromNullable(vstopo),
                     getFieldValue(task, Optional.class, "staticTopology"));

        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            verify(ctx).getReadSpecific(InventoryReader.class);
            verify(tx).read(oper, igPath);
        }
        verify(tx).read(config, path);
        verifyNoMoreInteractions(provider, ctx, tx);

        // Create dummy configuration files that should be deleted.
        for (File f: files) {
            FileWriter writer = new FileWriter(f);
            writer.write("bad config\n");
            writer.close();
            assertTrue(f.isFile());
        }

        task.onSuccess(null, luctx);
        if (owner) {
            verify(logger).isDebugEnabled();
        }
        verifyNoMoreInteractions(logger);

        for (File f: files) {
            assertFalse(f.exists());
        }
    }

    /**
     * Common test for {@link #testExecuteNoPort()} and
     * {@link #testExecuteNoPortNotOwner()}.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testExecuteNoPort(boolean owner) throws Exception {
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

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        when(ctx.getReadSpecific(InventoryReader.class)).thenReturn(reader);

        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        when(ctx.getProvider()).thenReturn(provider);

        // Create configurations to be saved.
        SalPort src = new SalPort(12L, 34L);
        SalPort newDst = new SalPort(56L, 78L);
        SalPort oldDst = new SalPort(90L, 12L);
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(src.getNodeConnectorId()).
            setDestination(newDst.getNodeConnectorId()).
            build();
        StaticSwitchLink old = new StaticSwitchLinkBuilder().
            setSource(src.getNodeConnectorId()).
            setDestination(oldDst.getNodeConnectorId()).
            build();

        StaticSwitchLink swlink1 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:1:2")).
            setDestination(new NodeConnectorId("openflow:3:4")).
            build();
        StaticSwitchLink swlink2 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:5:6")).
            setDestination(new NodeConnectorId("openflow:7:8")).
            build();
        StaticSwitchLinks swlinks =
            newStaticSwitchLinks(swlink, swlink1, swlink2);

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

        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        InstanceIdentifier<VtnStaticTopology> path =
            InstanceIdentifier.create(VtnStaticTopology.class);
        when(tx.read(config, path)).thenReturn(getReadResult(vstopo));

        // No ignored links.
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        IgnoredLinks ignored = null;
        when(tx.read(oper, igPath)).thenReturn(getReadResult(ignored));

        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);
        when(logger.isDebugEnabled()).thenReturn(true);

        Set<SalPort> updatedPorts = new HashSet<>();
        Collections.addAll(updatedPorts, src, oldDst);
        task.addUpdated(old, swlink);
        assertEquals(updatedPorts, task.getUpdatedPorts());

        // None of the updated ports are present.
        reader.prefetch(src, (VtnPort)null);
        reader.prefetch(oldDst, (VtnPort)null);

        LinkUpdateContext luctx = task.execute(ctx);
        if (owner) {
            assertNotNull(luctx);
        } else {
            assertEquals(null, luctx);
        }

        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(Optional.fromNullable(vstopo),
                     getFieldValue(task, Optional.class, "staticTopology"));

        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.INVENTORY);
        if (owner) {
            verify(ctx).getReadSpecific(InventoryReader.class);
            verify(tx).read(oper, igPath);
        }
        verify(tx).read(config, path);
        verifyNoMoreInteractions(ctx, tx);

        task.onSuccess(null, luctx);
        if (owner) {
            verify(logger).isDebugEnabled();
        }
        verifyNoMoreInteractions(logger);

        // Only static links should be saved.
        assertFalse(edgeFile.exists());
        assertTrue(linkFile.isFile());

        XmlConfigFile.Type ctype = XmlConfigFile.Type.TOPOLOGY;
        String key = XmlStaticSwitchLinksTest.CONFIG_KEY;
        XmlStaticSwitchLinks xswlinks = XmlConfigFile.
            load(ctype, key, XmlStaticSwitchLinks.class);
        assertEquals(new XmlStaticSwitchLinks(swlinks), xswlinks);
    }

    /**
     * Common test for {@link #testExecuteUpdated()} and
     * {@link #testExecuteUpdatedNotOwner()}.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testExecuteUpdated(boolean owner) throws Exception {
        XmlConfigFile.init();
        File[] files = {
            XmlStaticSwitchLinksTest.getConfigFile(),
            XmlStaticEdgePortsTest.getConfigFile(),
        };
        for (File f: files) {
            assertFalse(f.exists());
        }

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        when(ctx.getReadSpecific(InventoryReader.class)).thenReturn(reader);

        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        when(ctx.getProvider()).thenReturn(provider);

        // sport1 is configured as an static edge port.
        SalPort sport1 = new SalPort(11L, 22L);
        SalPort peer1 = new SalPort(33L, 44L);
        SalPort src1 = new SalPort(55L, 66L);

        // Below links will be removed.
        InterSwitchLink isl11 = new InterSwitchLink(sport1, peer1);
        InterSwitchLink isl12 = new InterSwitchLink(peer1, sport1);
        InterSwitchLink isl13 = new InterSwitchLink(src1, sport1, true);
        List<PortLink> plinks = new ArrayList<>();
        Collections.addAll(plinks, isl11.getSourceLink(),
                           isl12.getDestinationLink(),
                           isl13.getDestinationLink());
        VtnPort vport1 = new VtnPortBuilder().
            setId(sport1.getNodeConnectorId()).
            setEnabled(true).setPortLink(plinks).build();

        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnLink> lpath11 = isl11.getVtnLinkPath();
        InstanceIdentifier<VtnLink> lpath12 = isl12.getVtnLinkPath();
        InstanceIdentifier<VtnLink> lpath13 = isl13.getVtnLinkPath();
        when(tx.read(oper, lpath11)).
            thenReturn(getReadResult(isl11.getVtnLink()));
        when(tx.read(oper, lpath12)).
            thenReturn(getReadResult(isl12.getVtnLink()));
        when(tx.read(oper, lpath13)).
            thenReturn(getReadResult(isl13.getVtnLink()));

        InstanceIdentifier<PortLink> spath11 = isl11.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath12 = isl12.getSourceLinkPath();
        InstanceIdentifier<PortLink> spath13 = isl13.getSourceLinkPath();
        when(tx.read(oper, spath12)).
            thenReturn(getReadResult(isl12.getSourceLink()));
        when(tx.read(oper, spath13)).
            thenReturn(getReadResult(isl13.getSourceLink()));

        InstanceIdentifier<PortLink> dpath11 = isl11.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath12 = isl12.getDestinationLinkPath();
        InstanceIdentifier<PortLink> dpath13 = isl13.getDestinationLinkPath();
        when(tx.read(oper, dpath11)).
            thenReturn(getReadResult(isl11.getDestinationLink()));

        // A static link will be configured on sport2.
        SalPort sport2 = new SalPort(123L, 456L);
        SalPort peer2 = new SalPort(789L, 112233L);

        InterSwitchLink isl21 = new InterSwitchLink(sport2, peer2, true);
        VtnPort vport2 = new VtnPortBuilder().
            setId(sport2.getNodeConnectorId()).
            setEnabled(true).build();
        VtnPort vpeer2 = new VtnPortBuilder().
            setId(peer2.getNodeConnectorId()).
            setEnabled(true).build();

        InstanceIdentifier<VtnLink> lpath21 = isl21.getVtnLinkPath();
        when(tx.read(oper, lpath21)).
            thenReturn(getReadResult((VtnLink)null));

        // Create configurations to be saved.
        StaticSwitchLink swlink = new StaticSwitchLinkBuilder().
            setSource(sport2.getNodeConnectorId()).
            setDestination(peer2.getNodeConnectorId()).
            build();

        StaticSwitchLink swlink1 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:1:2")).
            setDestination(new NodeConnectorId("openflow:3:4")).
            build();
        StaticSwitchLink swlink2 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:5:6")).
            setDestination(new NodeConnectorId("openflow:7:8")).
            build();
        StaticSwitchLinks swlinks =
            newStaticSwitchLinks(swlink, swlink1, swlink2);

        SalPort edge1 = new SalPort(1L, 9L);
        SalPort edge2 = new SalPort(123L, 45L);
        StaticEdgePorts edgePorts = newStaticEdgePorts(sport1, edge1, edge2);
        StaticEdgePort ep = new StaticEdgePortBuilder().
            setPort(sport1.getNodeConnectorId()).build();

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlinks).
            setStaticEdgePorts(edgePorts).
            build();

        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        InstanceIdentifier<VtnStaticTopology> path =
            InstanceIdentifier.create(VtnStaticTopology.class);
        when(tx.read(config, path)).thenReturn(getReadResult(vstopo));

        // isl11 and isl12 will be moved to ignored-links container.
        InstanceIdentifier<IgnoredLinks> igPath =
            InstanceIdentifier.create(IgnoredLinks.class);
        List<IgnoredLink> igList = new ArrayList<>();
        Collections.addAll(igList, isl11.getIgnoredLink(),
                           isl12.getIgnoredLink());
        IgnoredLinks ignored = new IgnoredLinksBuilder().
            setIgnoredLink(igList).build();
        when(tx.read(oper, igPath)).thenReturn(getReadResult(ignored));

        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);
        when(logger.isDebugEnabled()).thenReturn(true);

        Set<SalPort> updatedPorts = new HashSet<>();
        Collections.addAll(updatedPorts, sport1, sport2);
        task.addUpdated(swlink);
        task.addUpdated(ep);
        assertEquals(updatedPorts, task.getUpdatedPorts());

        reader.prefetch(sport1, vport1);
        reader.prefetch(sport2, vport2);
        reader.prefetch(peer2, vpeer2);

        LinkUpdateContext luctx = task.execute(ctx);
        if (owner) {
            assertNotNull(luctx);
        } else {
            assertEquals(null, luctx);
        }

        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(Optional.fromNullable(vstopo),
                     getFieldValue(task, Optional.class, "staticTopology"));

        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.INVENTORY);
        verify(tx).read(config, path);

        if (owner) {
            verify(ctx).getReadSpecific(InventoryReader.class);

            // Links on sport1 will be removed.
            verify(tx).read(oper, lpath11);
            verify(tx).read(oper, lpath12);
            verify(tx).read(oper, lpath13);
            verify(tx).delete(oper, lpath11);
            verify(tx).delete(oper, lpath12);
            verify(tx).delete(oper, lpath13);

            verify(tx).read(oper, spath12);
            verify(tx).read(oper, spath13);
            verify(tx).delete(oper, spath11);
            verify(tx).delete(oper, spath12);
            verify(tx).delete(oper, spath13);

            verify(tx).read(oper, dpath11);
            verify(tx).delete(oper, dpath11);
            verify(tx).delete(oper, dpath12);
            verify(tx).delete(oper, dpath13);

            verify(tx).
                put(oper, isl11.getIgnoredLinkPath(), isl11.getIgnoredLink(),
                    true);
            verify(tx).
                put(oper, isl12.getIgnoredLinkPath(), isl12.getIgnoredLink(),
                    true);

            // isl21 will be established.
            InstanceIdentifier<PortLink> spath21 = isl21.getSourceLinkPath();
            InstanceIdentifier<PortLink> dpath21 =
                isl21.getDestinationLinkPath();

            verify(tx).read(oper, lpath21);
            verify(tx).put(oper, lpath21, isl21.getVtnLink(), true);
            verify(tx).put(oper, spath21, isl21.getSourceLink(), true);
            verify(tx).put(oper, dpath21, isl21.getDestinationLink(), true);

            // LinkUpdateContext.resolveIgnoredLinks() will found isl11 and
            // isl12, but they should not be resolved because sport1 is
            // configured as a  static edge port.
            verify(tx).read(oper, igPath);
        }

        verifyNoMoreInteractions(ctx, tx);

        task.onSuccess(null, luctx);

        if (owner) {
            verify(logger).isDebugEnabled();

            String msg =
                "Inter-switch link has been superseded by static topology";
            VtnLink vlink11 = isl11.getVtnLink();
            VtnLink vlink12 = isl12.getVtnLink();
            verify(logger).
                info("{}: {}: {} -> {}", msg,
                     vlink11.getLinkId().getValue(),
                     vlink11.getSource().getValue(),
                     vlink11.getDestination().getValue());
            verify(logger).
                info("{}: {}: {} -> {}", msg,
                     vlink12.getLinkId().getValue(),
                     vlink12.getSource().getValue(),
                     vlink12.getDestination().getValue());
            verify(logger).
                info("{}: Port has been updated as a static edge port.",
                     sport1);

            msg = "Skip ignored-link";
            verify(logger).
                debug("{}: {}: {} -> {}: cause={}", msg,
                      vlink11.getLinkId().getValue(),
                      vlink11.getSource().getValue(),
                      vlink11.getDestination().getValue(),
                      "Source port is configured as static edge port.");
            verify(logger).
                debug("{}: {}: {} -> {}: cause={}", msg,
                      vlink12.getLinkId().getValue(),
                      vlink12.getSource().getValue(),
                      vlink12.getDestination().getValue(),
                      "Destination port is configured as static edge port.");
        }
        verifyNoMoreInteractions(logger);

        // Static network topology should be saved.
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
     * Common test for {@link #testExecutePortReadFailed()} and
     * {@link #testExecutePortReadFailedNotOwner()}.
     *
     * @param owner  {@code true} indicates the process is the owner of the
     *               VTN inventory.
     * @throws Exception  An error occurred.
     */
    private void testExecutePortReadFailed(boolean owner) throws Exception {
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

        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InventoryReader reader = new InventoryReader(tx);
        TxContext ctx = mock(TxContext.class);
        when(ctx.getReadWriteTransaction()).thenReturn(tx);
        when(ctx.getReadSpecific(InventoryReader.class)).thenReturn(reader);

        VTNManagerProvider provider = mock(VTNManagerProvider.class);
        when(provider.isOwner(VTNEntityType.INVENTORY)).thenReturn(owner);
        when(ctx.getProvider()).thenReturn(provider);

        // Create configurations to be saved.
        StaticSwitchLink swlink1 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:1:2")).
            setDestination(new NodeConnectorId("openflow:3:4")).
            build();
        StaticSwitchLink swlink2 = new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId("openflow:5:6")).
            setDestination(new NodeConnectorId("openflow:7:8")).
            build();
        StaticSwitchLinks swlinks = newStaticSwitchLinks(swlink1, swlink2);

        SalPort edge = new SalPort(10L, 20L);
        SalPort edge1 = new SalPort(1L, 9L);
        SalPort edge2 = new SalPort(123L, 45L);
        SalPort edge3 = new SalPort(333L, 444L);
        SalPort edge4 = new SalPort(98765L, 43210L);
        StaticEdgePorts edgePorts =
            newStaticEdgePorts(edge, edge1, edge2, edge3, edge4);

        VtnStaticTopology vstopo = new VtnStaticTopologyBuilder().
            setStaticSwitchLinks(swlinks).
            setStaticEdgePorts(edgePorts).
            build();

        LogicalDatastoreType config = LogicalDatastoreType.CONFIGURATION;
        InstanceIdentifier<VtnStaticTopology> path =
            InstanceIdentifier.create(VtnStaticTopology.class);
        when(tx.read(config, path)).thenReturn(getReadResult(vstopo));

        Logger logger = mock(Logger.class);
        StaticLinkUpdateTask task = new StaticLinkUpdateTask(logger);

        Set<SalPort> updatedPorts = new HashSet<>();
        updatedPorts.add(edge);
        StaticEdgePort ep = new StaticEdgePortBuilder().
            setPort(edge.getNodeConnectorId()).build();
        task.addUpdated(ep);
        assertEquals(updatedPorts, task.getUpdatedPorts());

        // Throw an exception on VTN port read operation.
        IllegalStateException cause = new IllegalStateException();
        LogicalDatastoreType oper = LogicalDatastoreType.OPERATIONAL;
        InstanceIdentifier<VtnPort> portPath = edge.getVtnPortIdentifier();
        when(tx.read(oper, portPath)).
            thenReturn(getReadFailure(VtnPort.class, cause));

        Throwable caught = null;
        try {
            task.execute(ctx);
            if (owner) {
                unexpected();
            }
        } catch (VTNException e) {
            if (owner) {
                caught = e;
                Throwable t = e.getCause();
                assertTrue(t instanceof ReadFailedException);
                assertEquals(cause, t.getCause());
            } else {
                unexpected(e);
            }
        }

        assertEquals(Boolean.FALSE,
                     getFieldValue(task, Boolean.class, "saveLinks"));
        assertEquals(Boolean.TRUE,
                     getFieldValue(task, Boolean.class, "saveEdges"));
        assertEquals(Optional.fromNullable(vstopo),
                     getFieldValue(task, Optional.class, "staticTopology"));

        verify(ctx).getReadWriteTransaction();
        verify(ctx).getProvider();
        verify(provider).isOwner(VTNEntityType.INVENTORY);
        verify(tx).read(config, path);

        if (owner) {
            verify(ctx).getReadSpecific(InventoryReader.class);
            verify(tx).read(oper, portPath);
        }
        verifyNoMoreInteractions(ctx, tx);

        task.onFailure(null, caught);
        verify(logger).
            error("Failed to update VTN static network topology.", caught);
        verifyNoMoreInteractions(logger);

        // Only static edge ports should be saved.
        assertFalse(linkFile.exists());
        assertTrue(edgeFile.isFile());

        XmlConfigFile.Type ctype = XmlConfigFile.Type.TOPOLOGY;
        String key = XmlStaticEdgePortsTest.CONFIG_KEY;
        XmlStaticEdgePorts xedges = XmlConfigFile.
            load(ctype, key, XmlStaticEdgePorts.class);
        assertEquals(new XmlStaticEdgePorts(edgePorts), xedges);
    }
}
