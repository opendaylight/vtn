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

import static org.opendaylight.vtn.manager.internal.TestBase.createVlanId;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.vnode.VTNVlanMapConfig;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.vlan.rev150907.vtn.vlan.mappable.VlanMap;

/**
 * {@code XmlVlanMapConfigList} describes a list of VLAN mapping configuration
 * in a vBridge.
 */
public final class XmlVlanMapConfigList
    extends AbstractConfigList<VTNVlanMapConfig, XmlVlanMapConfig,
                               VlanMap, XmlVlanMapConfigList> {
    /**
     * Verify the given VLAN mapping list.
     *
     * @param expected  A {@link XmlVlanMapConfigList} instance that contains
     *                  the expected VLAN mapping configuration.
     * @param vmaps     A list of {@link VTNVlanMapConfig} instances to be
     *                  verified.
     */
    public static void verify(XmlVlanMapConfigList expected,
                              List<VTNVlanMapConfig> vmaps) {
        if (expected == null) {
            assertEquals(null, vmaps);
        } else {
            expected.verify(vmaps);
        }
    }

    /**
     * Construct an empty instance.
     */
    public XmlVlanMapConfigList() {
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public XmlVlanMapConfigList(Random rand) {
        super(rand);
    }

    /**
     * Construct a new instance that contains the given number of VLAN mapping
     * configuratiosn using the given random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of VLAN mapping configurations to add.
     */
    public XmlVlanMapConfigList(Random rand, int count) {
        super(rand, count);
    }

    // AbstractConfigList

    /**
     * Set the VLAN mapping configuration in this instance into the specified
     * XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    @Override
    public void setXml(XmlNode xnode) {
        XmlNode xvmaps = new XmlNode("vlan-maps");
        for (XmlVlanMapConfig xvmc: getConfigMap().values()) {
            xvmaps.add(xvmc.toXmlNode());
        }
        xnode.add(xvmaps);
    }

    /**
     * Ensure that the given {@link VTNVlanMapConfig} instances are identical
     * to this instance.
     *
     * @param vmaps  A list of {@link VTNVlanMapConfig} instances.
     * @param jaxb   {@code true} indicates {@code vmaps} was deserialized from
     *               XML.
     */
    @Override
    public void verify(List<VTNVlanMapConfig> vmaps, boolean jaxb) {
        Map<Object, XmlVlanMapConfig> cfMap = getConfigMap();
        if (cfMap.isEmpty()) {
            assertEquals(null, vmaps);
        } else {
            assertNotNull(vmaps);
            Iterator<VTNVlanMapConfig> it = vmaps.iterator();
            for (XmlVlanMapConfig xvmc: cfMap.values()) {
                assertEquals(true, it.hasNext());
                xvmc.verify(it.next());
            }
            assertEquals(false, it.hasNext());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected XmlVlanMapConfigList getThis() {
        return this;
    }

    /**
     * Construct a new VLAN mapping configuration using the given random
     * number generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  A new VLAN mapping configuration.
     */
    @Override
    protected XmlVlanMapConfig newConfig(Random rand) {
        SalNode snode;
        if (rand.nextBoolean()) {
            // Specify the target node.
            snode = new SalNode(rand.nextLong());
        } else {
            snode = null;
        }

        Integer vid;
        int n = rand.nextInt(5);
        if (n < 2) {
            // Use default VLAN ID.
            vid = null;
        } else {
            // Specify the VLAN ID.
            vid = createVlanId(rand);
        }

        return new XmlVlanMapConfig(snode, vid);
    }

    /**
     * Return the key of the given VLAN mapping configuration.
     *
     * @param conf  An instance that specifies the VLAN mapping configuration.
     * @return  The key of the given configuration.
     */
    @Override
    protected String getKey(XmlVlanMapConfig conf) {
        return conf.getMapId();
    }

    /**
     * Convert the given configuration into a VLAN mapping.
     *
     * @param conf  An instance that specifies the VLAN mapping configuration.
     * @return  A {@link VlanMap} instance.
     */
    @Override
    protected VlanMap toDataObject(XmlVlanMapConfig conf) {
        return conf.toVlanMap();
    }
}
