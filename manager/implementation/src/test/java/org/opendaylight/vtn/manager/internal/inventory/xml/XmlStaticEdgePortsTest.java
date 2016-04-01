/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.inventory.xml;

import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import java.io.File;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.mockito.ArgumentCaptor;

import org.opendaylight.vtn.manager.internal.util.XmlConfigFile;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.controller.md.sal.binding.api.ReadWriteTransaction;
import org.opendaylight.controller.md.sal.common.api.data.LogicalDatastoreType;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePortsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePortBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link XmlStaticEdgePorts}.
 */
public class XmlStaticEdgePortsTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlStaticEdgePorts} class.
     */
    private static final String  XML_ROOT = "static-edge-ports";

    /**
     * The key associated with the configuration file.
     */
    public static final String  CONFIG_KEY = "static-edge";

    /**
     * Return a path to the configuration file for static edge ports.
     *
     * @return  A {@link File} instance associated with the static edge port
     *          configuration file.
     */
    public static File getConfigFile() {
        File confDir = new File(
            getConfigDir(), XmlConfigFile.Type.TOPOLOGY.toString());
        return new File(confDir, CONFIG_KEY + ".xml");
    }

    /**
     * Test case for {@link XmlStaticEdgePorts#XmlStaticEdgePorts()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testDefaultConstructor() throws Exception {
        XmlStaticEdgePorts xedges = new XmlStaticEdgePorts();
        assertEquals(null, getFieldValue(xedges, Map.class, "edgePorts"));
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link XmlStaticEdgePorts#XmlStaticEdgePorts(StaticEdgePorts)}</li>
     *   <li>{@link XmlStaticEdgePorts#getConfig()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        // Empty static edge port configuration.
        StaticEdgePorts edges = new StaticEdgePortsBuilder().build();
        XmlStaticEdgePorts xedges = new XmlStaticEdgePorts(edges);
        assertEquals(edges, xedges.getConfig());

        edges = new StaticEdgePortsBuilder().
            setStaticEdgePort(new ArrayList<StaticEdgePort>()).
            build();
        xedges = new XmlStaticEdgePorts(edges);
        edges = xedges.getConfig();
        assertEquals(null, edges.getStaticEdgePort());

        for (List<StaticEdgePort> eplist: createStaticEdgePorts(true)) {
            edges = new StaticEdgePortsBuilder().
                setStaticEdgePort(eplist).
                build();
            xedges = new XmlStaticEdgePorts(edges);
            assertEqualsStaticEdgePorts(edges, xedges.getConfig());
        }

        // Ensure that invalid static edge ports are eliminated.
        List<StaticEdgePort> eplist = new ArrayList<>();
        List<StaticEdgePort> badEdges = new ArrayList<>();
        Set<StaticEdgePort> goodEdges = new HashSet<>();
        StaticEdgePort edge = newStaticEdgePort("openflow:1:2");
        goodEdges.add(edge);
        eplist.add(edge);

        edge = newStaticEdgePort("openflow:1:3");
        goodEdges.add(edge);
        eplist.add(edge);

        // Empty port ID.
        edge = newStaticEdgePort("");
        badEdges.add(edge);
        eplist.add(edge);

        edge = newStaticEdgePort("unknown:1:2");
        goodEdges.add(edge);
        eplist.add(edge);

        // Duplicate port.
        edge = newStaticEdgePort("openflow:1:3");
        eplist.add(edge);

        edge = newStaticEdgePort("openflow:10:20");
        goodEdges.add(edge);
        eplist.add(edge);

        // No port ID.
        edge = new StaticEdgePortBuilder().build();
        badEdges.add(edge);
        eplist.add(edge);

        edge = newStaticEdgePort("unknown:3:4");
        goodEdges.add(edge);
        eplist.add(edge);

        edges = new StaticEdgePortsBuilder().
            setStaticEdgePort(badEdges).build();
        xedges = new XmlStaticEdgePorts(edges);
        StaticEdgePorts edges1 = xedges.getConfig();
        assertEquals(null, edges1.getStaticEdgePort());

        edges = new StaticEdgePortsBuilder().
            setStaticEdgePort(eplist).build();
        xedges = new XmlStaticEdgePorts(edges);
        edges1 = xedges.getConfig();
        for (StaticEdgePort ep: edges1.getStaticEdgePort()) {
            assertEquals(true, goodEdges.remove(ep));
        }
        assertTrue(goodEdges.isEmpty());
    }

    /**
     * Test case for {@link XmlStaticEdgePorts#equals(Object)} and
     * {@link XmlStaticEdgePorts#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();

        List<List<StaticEdgePort>> edgePorts = createStaticEdgePorts(false);
        for (List<StaticEdgePort> eplist: edgePorts) {
            StaticEdgePorts edges1 = new StaticEdgePortsBuilder().
                setStaticEdgePort(eplist).
                build();
            StaticEdgePorts edges2 = new StaticEdgePortsBuilder().
                setStaticEdgePort(new ArrayList<StaticEdgePort>(eplist)).
                build();
            XmlStaticEdgePorts xedges1 = new XmlStaticEdgePorts(edges1);
            XmlStaticEdgePorts xedges2 = new XmlStaticEdgePorts(edges2);
            testEquals(set, xedges1, xedges2);
        }

        assertEquals(edgePorts.size(), set.size());
    }

    /**
     * Test case for JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlStaticEdgePorts> type = XmlStaticEdgePorts.class;
        for (List<StaticEdgePort> eplist: createStaticEdgePorts(true)) {
            StaticEdgePorts edges = new StaticEdgePortsBuilder().
                setStaticEdgePort(eplist).
                build();
            XmlStaticEdgePorts xedges = new XmlStaticEdgePorts(edges);
            jaxbTest(xedges, type, XML_ROOT);
        }

        // Ensure that invalid static edge ports are eliminated.
        XmlNode xedges = new XmlNode(XML_ROOT);
        XmlNode badEdges = new XmlNode(XML_ROOT);
        Set<StaticEdgePort> goodEdges = new HashSet<>();

        for (int i = 5; i <= 10; i++) {
            String pid = "openflow:1:" + i;
            xedges.add(createXmlEdge(pid));
            goodEdges.add(newStaticEdgePort(pid));
        }

        // Empty edge port.
        XmlNode xedge = createXmlEdge(null);
        badEdges.add(xedge);
        xedges.add(xedge);

        for (int i = 1; i <= 10; i++) {
            String pid = "unknown:" + i + ":" + i;
            goodEdges.add(newStaticEdgePort(pid));
            xedges.add(createXmlEdge(pid));
        }

        xedge = createXmlEdge("");
        badEdges.add(xedge);
        xedges.add(xedge);

        String pid = "openflow:999:888";
        goodEdges.add(newStaticEdgePort(pid));
        xedges.add(createXmlEdge(pid));

        pid = "openflow:123:4";
        goodEdges.add(newStaticEdgePort(pid));
        xedges.add(createXmlEdge(pid));

        Unmarshaller um = createUnmarshaller(type);
        XmlStaticEdgePorts xedgePorts =
            unmarshal(um, badEdges.toString(), type);
        StaticEdgePorts edgePorts = xedgePorts.getConfig();
        assertEquals(null, edgePorts.getStaticEdgePort());

        xedgePorts = unmarshal(um, xedges.toString(), type);
        edgePorts = xedgePorts.getConfig();
        for (StaticEdgePort ep: edgePorts.getStaticEdgePort()) {
            assertEquals(true, goodEdges.remove(ep));
        }
        assertTrue(goodEdges.isEmpty());
    }

    /**
     * Test case for {@link XmlStaticEdgePorts#getContainerType()}.
     */
    @Test
    public void testGetContainerType() {
        XmlStaticEdgePorts base = new XmlStaticEdgePorts();
        for (int i = 0; i < 10; i++) {
            assertEquals(StaticEdgePorts.class, base.getContainerType());
        }
    }

    /**
     * Test case for {@link XmlStaticEdgePorts#getXmlType()}.
     */
    @Test
    public void testGetXmlType() {
        XmlStaticEdgePorts base = new XmlStaticEdgePorts();
        for (int i = 0; i < 10; i++) {
            assertEquals(XmlStaticEdgePorts.class, base.getXmlType());
        }
    }

    /**
     * Test case for {@link XmlStaticEdgePorts#getXmlConfigKey()}.
     */
    @Test
    public void testGetXmlConfigKey() {
        XmlStaticEdgePorts base = new XmlStaticEdgePorts();
        for (int i = 0; i < 10; i++) {
            assertEquals(CONFIG_KEY, base.getXmlConfigKey());
        }
    }

    /**
     * Test case for
     * {@link XmlStaticEdgePorts#setConfig(VtnStaticTopologyBuilder, StaticEdgePorts)}.
     */
    @Test
    public void testSetConfig() {
        XmlStaticEdgePorts base = new XmlStaticEdgePorts();
        StaticEdgePorts edges = new StaticEdgePortsBuilder().build();
        VtnStaticTopologyBuilder builder = new VtnStaticTopologyBuilder();
        base.setConfig(builder, edges);
        assertSame(edges, builder.getStaticEdgePorts());
        assertEquals(null, builder.getStaticSwitchLinks());
    }

    /**
     * Test case for
     * {@link XmlStaticEdgePorts#newInstance(StaticEdgePorts)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNewInstance() throws Exception {
        XmlStaticEdgePorts base = new XmlStaticEdgePorts();
        StaticEdgePorts edges = new StaticEdgePortsBuilder().build();
        XmlStaticEdgePorts xedges = base.newInstance(edges);
        assertNotSame(base, xedges);
        assertEquals(edges, xedges.getConfig());

        edges = new StaticEdgePortsBuilder().
            setStaticEdgePort(new ArrayList<StaticEdgePort>()).
            build();
        xedges = base.newInstance(edges);
        assertNotSame(base, xedges);
        edges = xedges.getConfig();
        assertEquals(null, edges.getStaticEdgePort());

        for (List<StaticEdgePort> eplist: createStaticEdgePorts(true)) {
            edges = new StaticEdgePortsBuilder().
                setStaticEdgePort(eplist).
                build();
            xedges = base.newInstance(edges);
            assertNotSame(base, xedges);
            assertEqualsStaticEdgePorts(edges, xedges.getConfig());
        }

        // newInstance() should never affect base instance.
        assertEquals(null, getFieldValue(base, Map.class, "edgePorts"));
    }

    /**
     * Test case for {@link XmlStaticEdgePorts#save(org.opendaylight.yangtools.yang.binding.ChildOf)} and
     * {@link XmlStaticEdgePorts#load(VtnStaticTopologyBuilder, ReadWriteTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSaveLoad() throws Exception {
        XmlConfigFile.init();
        XmlStaticEdgePorts base = new XmlStaticEdgePorts();
        File confFile = getConfigFile();

        // In case where the configuration file is not present.
        // In this case the configuration should be removed from the config DS.
        assertFalse(confFile.exists());
        VtnStaticTopologyBuilder builder = new VtnStaticTopologyBuilder();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InstanceIdentifier<StaticEdgePorts> path = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticEdgePorts.class).
            build();
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        StaticEdgePorts edges = new StaticEdgePortsBuilder().build();
        builder.setStaticEdgePorts(edges);
        when(tx.read(cstore, path)).thenReturn(getReadResult(edges));
        base.load(builder, tx);

        verify(tx).read(cstore, path);
        verify(tx).delete(cstore, path);
        verifyNoMoreInteractions(tx);
        assertEquals(null, builder.getStaticEdgePorts());
        assertEquals(null, builder.getStaticSwitchLinks());

        for (List<StaticEdgePort> eplist: createStaticEdgePorts(false)) {
            // Save the configuration.
            edges = new StaticEdgePortsBuilder().
                setStaticEdgePort(eplist).
                build();
            base.save(edges);
            assertTrue(confFile.isFile());

            // Load the configuration.
            // This should install the loaded configuration into the config DS.
            tx = mock(ReadWriteTransaction.class);
            base.load(builder, tx);

            ArgumentCaptor<StaticEdgePorts> captor =
                ArgumentCaptor.forClass(StaticEdgePorts.class);
            verify(tx).put(eq(cstore), eq(path), captor.capture(), eq(true));
            verifyNoMoreInteractions(tx);

            List<StaticEdgePorts> captured = captor.getAllValues();
            assertEquals(1, captured.size());
            StaticEdgePorts loaded = captured.get(0);
            assertEquals(loaded, builder.getStaticEdgePorts());
            assertEqualsStaticEdgePorts(edges, loaded);
            assertEquals(null, builder.getStaticSwitchLinks());

            // Base instance should never be affected.
            assertEquals(null, getFieldValue(base, Map.class, "edgePorts"));
        }

        // This should delete the configuration file.
        assertTrue(confFile.isFile());
        edges = null;
        base.save(edges);
        assertFalse(confFile.exists());
    }

    /**
     * Create a list of {@link StaticEdgePort} lists for test.
     *
     * @param addNull  Add a {@code null} if {@code true}.
     * @return  A list of {@link StaticEdgePort} lists.
     */
    private List<List<StaticEdgePort>> createStaticEdgePorts(boolean addNull) {
        List<List<StaticEdgePort>> lists = new ArrayList<>();
        if (addNull) {
            lists.add(null);
        }
        lists.add(new ArrayList<StaticEdgePort>());

        List<StaticEdgePort> edges = new ArrayList<>();
        for (int i = 1; i <= 10; i++) {
            edges.add(newStaticEdgePort("openflow:" + i + ":" + i));
            lists.add(new ArrayList<StaticEdgePort>(edges));
        }

        edges.clear();
        edges.add(newStaticEdgePort("openflow:111:1"));
        edges.add(newStaticEdgePort("openflow:333:3"));
        edges.add(newStaticEdgePort("openflow:555:5"));
        for (int i = 1; i <= 5; i++) {
            edges.add(newStaticEdgePort("unknown:1:" + i));
        }

        lists.add(edges);

        return lists;
    }

    /**
     * Create a new {@link StaticEdgePort} instance.
     *
     * @param edge  The switch port to be treated as edge port.
     * @return  A {@link StaticEdgePort} instance.
     */
    private StaticEdgePort newStaticEdgePort(String edge) {
        return new StaticEdgePortBuilder().
            setPort(new NodeConnectorId(edge)).
            build();
    }

    /**
     * Create a {@link XmlNode} instance which represents an instance of
     * {@link StaticEdgePort}.
     *
     * @param port  The port identifier to be treated as edge port.
     * @return  A {@link XmlNode} instance.
     */
    private XmlNode createXmlEdge(String port) {
        return new XmlNode("static-edge-port", port);
    }
}
