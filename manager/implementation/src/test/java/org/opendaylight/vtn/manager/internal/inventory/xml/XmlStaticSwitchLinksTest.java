/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
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

import org.opendaylight.controller.sal.utils.GlobalConstants;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopology;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.VtnStaticTopologyBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinksBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLinkBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;

/**
 * JUnit test for {@link XmlStaticSwitchLinks}.
 */
public class XmlStaticSwitchLinksTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlStaticSwitchLinks}
     * class.
     */
    private static final String  XML_ROOT = "static-switch-links";

    /**
     * The key associated with the configuration file.
     */
    public static final String  CONFIG_KEY = "static-link";

    /**
     * Return a path to the configuration file for static inter-switch links.
     *
     * @return  A {@link File} instance associated with the static inter-switch
     *          link configuration file.
     */
    public static File getConfigFile() {
        File baseDir = new File(GlobalConstants.STARTUPHOME.toString(),
                                GlobalConstants.DEFAULT.toString());
        File vtnDir = new File(baseDir, "vtn");
        File confDir =
            new File(vtnDir, XmlConfigFile.Type.TOPOLOGY.toString());

        return new File(confDir, CONFIG_KEY + ".xml");
    }

    /**
     * Test case for {@link XmlStaticSwitchLinks#XmlStaticSwitchLinks()}.
     */
    @Test
    public void testDefaultConstructor() {
        XmlStaticSwitchLinks xswlinks = new XmlStaticSwitchLinks();
        assertEquals(null, xswlinks.getXmlStaticSwitchLink());
    }

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link XmlStaticSwitchLinks#XmlStaticSwitchLinks(StaticSwitchLinks)}</li>
     *   <li>{@link XmlStaticSwitchLinks#getConfig()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        // Empty static inter-switch link configuration.
        StaticSwitchLinks swlinks = new StaticSwitchLinksBuilder().build();
        XmlStaticSwitchLinks xswlinks = new XmlStaticSwitchLinks(swlinks);
        assertEquals(swlinks, xswlinks.getConfig());

        swlinks = new StaticSwitchLinksBuilder().
            setStaticSwitchLink(new ArrayList<StaticSwitchLink>()).
            build();
        xswlinks = new XmlStaticSwitchLinks(swlinks);
        swlinks = xswlinks.getConfig();
        assertEquals(null, swlinks.getStaticSwitchLink());

        for (List<StaticSwitchLink> links: createStaticSwitchLinks(true)) {
            swlinks = new StaticSwitchLinksBuilder().
                setStaticSwitchLink(links).
                build();
            xswlinks = new XmlStaticSwitchLinks(swlinks);
            assertEqualsStaticSwitchLinks(swlinks, xswlinks.getConfig());
        }

        // Ensure that invalid static links are eliminated.
        List<StaticSwitchLink> links = new ArrayList<>();
        List<StaticSwitchLink> badLinks = new ArrayList<>();
        Set<StaticSwitchLink> goodLinks = new HashSet<>();
        StaticSwitchLink swlink =
            newStaticSwitchLink("openflow:1:2", "openflow:2:3");
        goodLinks.add(swlink);
        links.add(swlink);

        // Source port is empty.
        swlink = newStaticSwitchLink("", "openflow:10:20");
        badLinks.add(swlink);
        links.add(swlink);

        swlink = newStaticSwitchLink("openflow:3:4", "openflow:5:6");
        goodLinks.add(swlink);
        links.add(swlink);

        // Destination port is empty.
        swlink = newStaticSwitchLink("openflow:9:10", "");
        badLinks.add(swlink);
        links.add(swlink);

        swlink = newStaticSwitchLink("openflow:11:12", "openflow:13:14");
        goodLinks.add(swlink);
        links.add(swlink);

        String src = "openflow:99:1";
        swlink = newStaticSwitchLink(src, "openflow:10:11");
        goodLinks.add(swlink);
        links.add(swlink);

        swlink = newStaticSwitchLink("unknown:1:2", "openflow:1:99");
        goodLinks.add(swlink);
        links.add(swlink);

        // The destination port is the same as the source.
        swlink = newStaticSwitchLink("openflow:34:35", "openflow:34:35");
        badLinks.add(swlink);
        links.add(swlink);

        // Duplicate source port "openflow:99:1".
        swlink = newStaticSwitchLink(src, "openflow:888:999");
        links.add(swlink);

        swlinks = new StaticSwitchLinksBuilder().
            setStaticSwitchLink(badLinks).
            build();
        xswlinks = new XmlStaticSwitchLinks(swlinks);
        StaticSwitchLinks swlinks1 = xswlinks.getConfig();
        assertEquals(null, swlinks1.getStaticSwitchLink());

        swlinks = new StaticSwitchLinksBuilder().
            setStaticSwitchLink(links).
            build();
        xswlinks = new XmlStaticSwitchLinks(swlinks);
        swlinks1 = xswlinks.getConfig();
        for (StaticSwitchLink swl: swlinks1.getStaticSwitchLink()) {
            assertTrue(goodLinks.remove(swl));
        }
        assertTrue(goodLinks.isEmpty());
    }

    /**
     * Test case for {@link XmlStaticSwitchLinks#equals(Object)} and
     * {@link XmlStaticSwitchLinks#hashCode()}.
     */
    @Test
    public void testEquals() {
        Set<Object> set = new HashSet<>();

        List<List<StaticSwitchLink>> swLinks = createStaticSwitchLinks(false);
        for (List<StaticSwitchLink> links: swLinks) {
            StaticSwitchLinks swl1 = new StaticSwitchLinksBuilder().
                setStaticSwitchLink(links).
                build();
            StaticSwitchLinks swl2 = new StaticSwitchLinksBuilder().
                setStaticSwitchLink(new ArrayList<StaticSwitchLink>(links)).
                build();
            XmlStaticSwitchLinks xswl1 = new XmlStaticSwitchLinks(swl1);
            XmlStaticSwitchLinks xswl2 = new XmlStaticSwitchLinks(swl2);
            testEquals(set, xswl1, xswl2);
        }

        assertEquals(swLinks.size(), set.size());
    }

    /**
     * Test case for JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlStaticSwitchLinks> type = XmlStaticSwitchLinks.class;
        for (List<StaticSwitchLink> links: createStaticSwitchLinks(true)) {
            StaticSwitchLinks swlinks = new StaticSwitchLinksBuilder().
                setStaticSwitchLink(links).
                build();
            XmlStaticSwitchLinks xswlinks = new XmlStaticSwitchLinks(swlinks);
            jaxbTest(xswlinks, type, XML_ROOT);
        }

        // Ensure that invalid static links are eliminated.
        XmlNode xlinks = new XmlNode(XML_ROOT);
        XmlNode badLinks = new XmlNode(XML_ROOT);
        Set<StaticSwitchLink> goodLinks = new HashSet<>();

        String src = "openflow:1:2";
        String dst = "openflow:2:2";
        XmlNode xlink = createXmlLink(src, dst);
        goodLinks.add(newStaticSwitchLink(src, dst));
        xlinks.add(xlink);

        // Empty link.
        xlink = createXmlLink(null, null);
        badLinks.add(xlink);
        xlinks.add(xlink);

        src = "openflow:1:3";
        dst = "openflow:2:1";
        xlink = createXmlLink(src, dst);
        goodLinks.add(newStaticSwitchLink(src, dst));
        xlinks.add(xlink);

        // Source port is null.
        xlink = createXmlLink(null, "openflow:1:10");
        badLinks.add(xlink);
        xlinks.add(xlink);

        // Source port is empty.
        xlink = createXmlLink("", "openflow:1:10");
        badLinks.add(xlink);
        xlinks.add(xlink);

        src = "openflow:100:200";
        dst = "openflow:101:201";
        xlink = createXmlLink(src, dst);
        goodLinks.add(newStaticSwitchLink(src, dst));
        xlinks.add(xlink);

        src = "openflow:100:201";
        dst = "openflow:102:201";
        xlink = createXmlLink(src, dst);
        goodLinks.add(newStaticSwitchLink(src, dst));
        xlinks.add(xlink);

        // Destination port is null.
        xlink = createXmlLink(src, null);
        badLinks.add(xlink);
        xlinks.add(xlink);

        // Destination port is empty.
        xlink = createXmlLink(src, "");
        badLinks.add(xlink);
        xlinks.add(xlink);

        src = "unknown:1:2";
        dst = "unknown:2:3";
        xlink = createXmlLink(src, dst);
        goodLinks.add(newStaticSwitchLink(src, dst));
        xlinks.add(xlink);

        String dupSrc = "openflow:123:456";
        dst = "openflow:111:222";
        xlink = createXmlLink(dupSrc, dst);
        goodLinks.add(newStaticSwitchLink(dupSrc, dst));
        xlinks.add(xlink);

        src = "unknown:1:3";
        dst = "unknown:4:3";
        xlink = createXmlLink(src, dst);
        goodLinks.add(newStaticSwitchLink(src, dst));
        xlinks.add(xlink);

        // The destination port is the same as the source.
        String[] ports = {
            src, dupSrc, "openflow:1:2", "openflow:1:3", "openflow:999:999",
            "unknown:1:2", "proto:10:20",
        };
        for (String port: ports) {
            xlink = createXmlLink(port, port);
            badLinks.add(xlink);
            xlinks.add(xlink);

            // Duplicate source port "openflow:123:456" will be ignored.
            xlink = createXmlLink(dupSrc, port);
            xlinks.add(xlink);
        }

        // Duplicate source port "openflow:123:456" will be ignored.
        xlink = createXmlLink(dupSrc, "openflow:555:678");
        xlinks.add(xlink);

        for (int i = 1; i <= 5; i++) {
            src = "openflow:12:" + i;
            dst = "openflow:56:" + i;
            xlink = createXmlLink(src, dst);
            goodLinks.add(newStaticSwitchLink(src, dst));
            xlinks.add(xlink);
        }

        Unmarshaller um = createUnmarshaller(type);
        XmlStaticSwitchLinks xswlinks =
            unmarshal(um, badLinks.toString(), type);
        StaticSwitchLinks swlinks = xswlinks.getConfig();
        assertEquals(null, swlinks.getStaticSwitchLink());

        xswlinks = unmarshal(um, xlinks.toString(), type);
        swlinks = xswlinks.getConfig();
        for (StaticSwitchLink swl: swlinks.getStaticSwitchLink()) {
            assertEquals(true, goodLinks.remove(swl));
        }
        assertTrue(goodLinks.toString(), goodLinks.isEmpty());
    }

    /**
     * Test case for {@link XmlStaticSwitchLinks#getContainerType()}.
     */
    @Test
    public void testGetContainerType() {
        XmlStaticSwitchLinks base = new XmlStaticSwitchLinks();
        for (int i = 0; i < 10; i++) {
            assertEquals(StaticSwitchLinks.class, base.getContainerType());
        }
    }

    /**
     * Test case for {@link XmlStaticSwitchLinks#getXmlType()}.
     */
    @Test
    public void testGetXmlType() {
        XmlStaticSwitchLinks base = new XmlStaticSwitchLinks();
        for (int i = 0; i < 10; i++) {
            assertEquals(XmlStaticSwitchLinks.class, base.getXmlType());
        }
    }

    /**
     * Test case for {@link XmlStaticSwitchLinks#getXmlConfigKey()}.
     */
    @Test
    public void testGetXmlConfigKey() {
        XmlStaticSwitchLinks base = new XmlStaticSwitchLinks();
        for (int i = 0; i < 10; i++) {
            assertEquals(CONFIG_KEY, base.getXmlConfigKey());
        }
    }

    /**
     * Test case for
     * {@link XmlStaticSwitchLinks#setConfig(VtnStaticTopologyBuilder, StaticSwitchLinks)}.
     */
    @Test
    public void testSetConfig() {
        XmlStaticSwitchLinks base = new XmlStaticSwitchLinks();
        StaticSwitchLinks swlinks = new StaticSwitchLinksBuilder().build();
        VtnStaticTopologyBuilder builder = new VtnStaticTopologyBuilder();
        base.setConfig(builder, swlinks);
        assertSame(swlinks, builder.getStaticSwitchLinks());
        assertEquals(null, builder.getStaticEdgePorts());
    }

    /**
     * Test case for
     * {@link XmlStaticSwitchLinks#newInstance(StaticSwitchLinks)}.
     */
    @Test
    public void testNewInstance() {
        XmlStaticSwitchLinks base = new XmlStaticSwitchLinks();
        StaticSwitchLinks swlinks = new StaticSwitchLinksBuilder().build();
        XmlStaticSwitchLinks xswlinks = base.newInstance(swlinks);
        assertNotSame(base, xswlinks);
        assertEquals(swlinks, xswlinks.getConfig());

        swlinks = new StaticSwitchLinksBuilder().
            setStaticSwitchLink(new ArrayList<StaticSwitchLink>()).
            build();
        xswlinks = base.newInstance(swlinks);
        assertNotSame(base, xswlinks);
        swlinks = xswlinks.getConfig();
        assertEquals(null, swlinks.getStaticSwitchLink());

        for (List<StaticSwitchLink> links: createStaticSwitchLinks(true)) {
            swlinks = new StaticSwitchLinksBuilder().
                setStaticSwitchLink(links).
                build();
            xswlinks = base.newInstance(swlinks);
            assertNotSame(base, xswlinks);
            assertEqualsStaticSwitchLinks(swlinks, xswlinks.getConfig());
        }

        // newInstance() should never affect base instance.
        assertEquals(null, base.getXmlStaticSwitchLink());
    }

    /**
     * Test case for {@link XmlStaticSwitchLinks#save(org.opendaylight.yangtools.yang.binding.ChildOf)} and
     * {@link XmlStaticSwitchLinks#load(VtnStaticTopologyBuilder, ReadWriteTransaction)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSaveLoad() throws Exception {
        XmlConfigFile.init();
        XmlStaticSwitchLinks base = new XmlStaticSwitchLinks();
        File confFile = getConfigFile();

        // In case where the configuration file is not present.
        // In this case the configuration should be removed from the config DS.
        assertFalse(confFile.exists());
        VtnStaticTopologyBuilder builder = new VtnStaticTopologyBuilder();
        ReadWriteTransaction tx = mock(ReadWriteTransaction.class);
        InstanceIdentifier<StaticSwitchLinks> path = InstanceIdentifier.
            builder(VtnStaticTopology.class).
            child(StaticSwitchLinks.class).
            build();
        LogicalDatastoreType cstore = LogicalDatastoreType.CONFIGURATION;
        StaticSwitchLinks swlinks = new StaticSwitchLinksBuilder().build();
        builder.setStaticSwitchLinks(swlinks);
        when(tx.read(cstore, path)).thenReturn(getReadResult(swlinks));
        base.load(builder, tx);

        verify(tx).read(cstore, path);
        verify(tx).delete(cstore, path);
        verifyNoMoreInteractions(tx);
        assertEquals(null, builder.getStaticSwitchLinks());
        assertEquals(null, builder.getStaticEdgePorts());

        for (List<StaticSwitchLink> links: createStaticSwitchLinks(false)) {
            // Save the configuration.
            swlinks = new StaticSwitchLinksBuilder().
                setStaticSwitchLink(links).
                build();
            base.save(swlinks);
            assertTrue(confFile.isFile());

            // Load the configuration.
            // This should install the loaded configuration into the config DS.
            tx = mock(ReadWriteTransaction.class);
            base.load(builder, tx);

            ArgumentCaptor<StaticSwitchLinks> captor =
                ArgumentCaptor.forClass(StaticSwitchLinks.class);
            verify(tx).put(eq(cstore), eq(path), captor.capture(), eq(true));
            verifyNoMoreInteractions(tx);

            List<StaticSwitchLinks> captured = captor.getAllValues();
            assertEquals(1, captured.size());
            StaticSwitchLinks loaded = captured.get(0);
            assertEquals(loaded, builder.getStaticSwitchLinks());
            assertEqualsStaticSwitchLinks(swlinks, loaded);
            assertEquals(null, builder.getStaticEdgePorts());

            // Base instance should never be affected.
            assertEquals(null, base.getXmlStaticSwitchLink());
        }

        // This should delete the configuration file.
        assertTrue(confFile.isFile());
        swlinks = null;
        base.save(swlinks);
        assertFalse(confFile.exists());
    }

    /**
     * Create a list of {@link StaticSwitchLink} lists for test.
     *
     * @param addNull  Add a {@code null} if {@code true}.
     * @return  A list of {@link StaticSwitchLink} lists.
     */
    private List<List<StaticSwitchLink>> createStaticSwitchLinks(
        boolean addNull) {
        List<List<StaticSwitchLink>> lists = new ArrayList<>();
        if (addNull) {
            lists.add(null);
        }
        lists.add(new ArrayList<StaticSwitchLink>());

        List<StaticSwitchLink> links = new ArrayList<>();
        links.add(newStaticSwitchLink("openflow:1:1", "openflow:2:1"));
        lists.add(new ArrayList<StaticSwitchLink>(links));
        links.add(newStaticSwitchLink("openflow:2:2", "openflow:3:2"));
        lists.add(new ArrayList<StaticSwitchLink>(links));
        links.add(newStaticSwitchLink("unknown:1:10", "openflow:100:9"));
        lists.add(new ArrayList<StaticSwitchLink>(links));
        links.add(newStaticSwitchLink("unknown:123:4", "unknown:456:7"));
        lists.add(new ArrayList<StaticSwitchLink>(links));

        links.clear();
        links.add(newStaticSwitchLink("openflow:12:1", "openflow:34:1"));
        links.add(newStaticSwitchLink("openflow:56:1", "openflow:78:3"));
        links.add(newStaticSwitchLink("openflow:111:5", "openflow:222:6"));
        links.add(newStaticSwitchLink("openflow:333:3", "openflow:444:9"));
        links.add(newStaticSwitchLink("openflow:555:9", "openflow:666:6"));
        links.add(newStaticSwitchLink("openflow:555:8", "openflow:888:9"));
        lists.add(links);

        return lists;
    }

    /**
     * Create a new {@link StaticSwitchLink} instance.
     *
     * @param src  The source switch port of the link.
     * @param dst  The destination switch port of the link.
     */
    private StaticSwitchLink newStaticSwitchLink(String src, String dst) {
        return new StaticSwitchLinkBuilder().
            setSource(new NodeConnectorId(src)).
            setDestination(new NodeConnectorId(dst)).
            build();
    }

    /**
     * Create a {@link XmlNode} instance which represents an instance of
     * {@link StaticSwitchLink}.
     *
     * @param src   The source switch port of the link.
     * @param dst   The destination switch port of the link.
     * @return  A {@link XmlNode} instance.
     */
    private XmlNode createXmlLink(String src, String dst) {
        return XmlStaticSwitchLinkTest.
            createXmlNode("static-switch-link", src, dst);
    }
}
