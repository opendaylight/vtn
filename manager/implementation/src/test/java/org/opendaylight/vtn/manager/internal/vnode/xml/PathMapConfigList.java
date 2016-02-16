/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathMap;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathmap.rev150328.vtn.path.map.list.VtnPathMap;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMaps;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtnPathMapsBuilder;

/**
 * {@code PathMapConfigList} describes configuration about a list of path maps.
 */
public final class PathMapConfigList
    extends AbstractConfigList<XmlPathMap, PathMapConfig,
                               VtnPathMap, PathMapConfigList> {
    /**
     * Convert the given instance into a vtn-path-maps instance.
     *
     * @param list  A {@link PathMapConfigList} instance.
     * @return  A {@link VtnPathMaps} instance if {@code list} is not
     *          {@code null}. {@code null} if {@code list} is {@code null}.
     */
    public static VtnPathMaps toVtnPathMaps(PathMapConfigList list) {
        VtnPathMaps vpms;
        if (list == null) {
            vpms = null;
        } else {
            vpms = new VtnPathMapsBuilder().
                setVtnPathMap(list.toDataObjects()).
                build();
        }
        return vpms;
    }

    /**
     * Verify the given path map list.
     *
     * @param expected  A {@link PathMapConfigList} instance that contains
     *                  the expected path map configuration.
     * @param xpms      A list of {@link XmlPathMap} instances.
     * @param jaxb      {@code true} indicates {@code xpms} was deserialized
     *                  from XML.
     */
    public static void verify(PathMapConfigList expected,
                              List<XmlPathMap> xpms, boolean jaxb) {
        if (expected == null) {
            assertEquals(null, xpms);
        } else {
            expected.verify(xpms, jaxb);
        }
    }

    /**
     * Construct an empty instance.
     */
    public PathMapConfigList() {
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public PathMapConfigList(Random rand) {
        super(rand);
    }

    /**
     * Construct a new instance that contains the given number of path map
     * configurations using the given random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of path maps to add.
     */
    public PathMapConfigList(Random rand, int count) {
        super(rand, count);
    }

    // AbstractConfigList

    /**
     * Set the path map configuration in this instance into the specified
     * XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    @Override
    public void setXml(XmlNode xnode) {
        XmlNode xpms = new XmlNode("vtn-path-maps");
        for (PathMapConfig pmconf: getConfigMap().values()) {
            xpms.add(pmconf.toXmlNode());
        }
        xnode.add(xpms);
    }

    /**
     * Ensure that the given {@link XmlPathMap} instances are identical
     * to this instance.
     *
     * @param xpms  A list of {@link XmlPathMap} instances.
     * @param jaxb  {@code true} indicates {@code xvifs} was deserialized
     *              from XML.
     */
    @Override
    public void verify(List<XmlPathMap> xpms, boolean jaxb) {
        Map<Object, PathMapConfig> cfMap = getConfigMap();
        if (cfMap.isEmpty()) {
            assertEquals(null, xpms);
        } else {
            assertNotNull(xpms);
            Iterator<XmlPathMap> it = xpms.iterator();
            for (PathMapConfig pmconf: cfMap.values()) {
                assertEquals(true, it.hasNext());
                pmconf.verify(it.next());
            }
            assertEquals(false, it.hasNext());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected PathMapConfigList getThis() {
        return this;
    }

    /**
     * Construct a new path map configuration using the given random number
     * generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  A new path map configuration.
     */
    @Override
    protected PathMapConfig newConfig(Random rand) {
        return new PathMapConfig(rand);
    }

    /**
     * Return the key of the given path map configuration.
     *
     * @param conf  An instance that specifies the path map configuration.
     * @return  The key of the given configuration.
     */
    @Override
    protected Integer getKey(PathMapConfig conf) {
        return conf.getIndex();
    }

    /**
     * Convert the given configuration into a vtn-path-map instance.
     *
     * @param conf  An instance that specifies the path map configuration.
     * @return  A {@link VtnPathMap} instance.
     */
    @Override
    protected VtnPathMap toDataObject(PathMapConfig conf) {
        return conf.toVtnPathMap();
    }
}
