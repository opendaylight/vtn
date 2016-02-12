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

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;

/**
 * {@code VBridgeConfigList} describes configuration about a list of vBridge.
 */
public final class VBridgeConfigList
    extends AbstractConfigList<XmlVBridge, VBridgeConfig,
                               Vbridge, VBridgeConfigList> {
    /**
     * Convert the given instance into a list of vbridge instances.
     *
     * @param list  A {@link VBridgeConfigList} instance.
     * @return  A list of vbridge or {@code null}.
     */
    public static List<Vbridge> toVbridgeList(VBridgeConfigList list) {
        return (list == null)
            ? null
            : list.toDataObjects();
    }

    /**
     * Verify the given vBridge list.
     *
     * @param expected  A {@link VBridgeConfigList} instance that contains
     *                  the expected vBridge configuration.
     * @param xvbrs     A list of {@link XmlVBridge} instances.
     * @param jaxb      {@code true} indicates {@code xvbrs} was deserialized
     *                  from XML.
     */
    public static void verify(VBridgeConfigList expected,
                              List<XmlVBridge> xvbrs, boolean jaxb) {
        if (expected == null) {
            assertEquals(null, xvbrs);
        } else {
            expected.verify(xvbrs, jaxb);
        }
    }

    /**
     * Construct an empty instance.
     */
    public VBridgeConfigList() {
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public VBridgeConfigList(Random rand) {
        super(rand);
    }

    /**
     * Construct a new instance that contains the given number of vBridge
     * configurations using the given random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of vBridges to add.
     */
    public VBridgeConfigList(Random rand, int count) {
        super(rand, count);
    }

    // AbstractConfigList

    /**
     * Set the vBridge configuration in this instance into the specified
     * XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    @Override
    public void setXml(XmlNode xnode) {
        XmlNode xvbrs = new XmlNode("vbridges");
        for (VBridgeConfig vtconf: getConfigMap().values()) {
            xvbrs.add(vtconf.toXmlNode());
        }
        xnode.add(xvbrs);
    }

    /**
     * Ensure that the given {@link XmlVBridge} instances are identical
     * to this instance.
     *
     * @param xvbrs  A list of {@link XmlVBridge} instances.
     * @param jaxb   {@code true} indicates {@code xvifs} was deserialized
     *               from XML.
     */
    @Override
    public void verify(List<XmlVBridge> xvbrs, boolean jaxb) {
        Map<Object, VBridgeConfig> cfMap = getConfigMap();
        if (cfMap.isEmpty()) {
            assertEquals(null, xvbrs);
        } else {
            assertNotNull(xvbrs);
            Iterator<XmlVBridge> it = xvbrs.iterator();
            for (VBridgeConfig vtconf: cfMap.values()) {
                assertEquals(true, it.hasNext());
                vtconf.verify(it.next(), jaxb);
            }
            assertEquals(false, it.hasNext());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VBridgeConfigList getThis() {
        return this;
    }

    /**
     * Construct a new vBridge configuration using the given random number
     * generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  A new vBridge configuration.
     */
    @Override
    protected VBridgeConfig newConfig(Random rand) {
        String name = "vbr_" + rand.nextInt(1000000);
        return new VBridgeConfig(name, rand);
    }

    /**
     * Return the key of the given vBridge configuration.
     *
     * @param conf  An instance that specifies the vBridge configuration.
     * @return  The key of the given configuration.
     */
    @Override
    protected String getKey(VBridgeConfig conf) {
        return conf.getName();
    }

    /**
     * Convert the given configuration into a vbridge instance.
     *
     * @param conf  An instance that specifies the vBridge configuration.
     * @return  A {@link Vbridge} instance.
     */
    @Override
    protected Vbridge toDataObject(VBridgeConfig conf) {
        return conf.toVbridge();
    }
}
