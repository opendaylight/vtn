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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.list.Vinterface;

/**
 * {@code VInterfaceConfigList} describes configuration about a list of
 * virtual interfaces.
 */
public final class VInterfaceConfigList
    extends AbstractConfigList<XmlVInterface, VInterfaceConfig,
                               Vinterface, VInterfaceConfigList> {
    /**
     * Convert the given instance into a list of vinterface instances.
     *
     * @param list  A {@link VInterfaceConfigList} instance.
     * @return  A list of vinterface or {@code null}.
     */
    public static List<Vinterface> toVinterfaceList(VInterfaceConfigList list) {
        return (list == null)
            ? null
            : list.toDataObjects();
    }

    /**
     * Verify the given virtual interface list.
     *
     * @param expected  A {@link VInterfaceConfigList} instance that contains
     *                  the expected virtual interface configuration.
     * @param xvifs     A list of {@link XmlVInterface} instances.
     * @param jaxb      {@code true} indicates {@code xvifs} was deserialized
     *                  from XML.
     */
    public static void verify(VInterfaceConfigList expected,
                              List<XmlVInterface> xvifs, boolean jaxb) {
        if (expected == null) {
            assertEquals(null, xvifs);
        } else {
            expected.verify(xvifs, jaxb);
        }
    }

    /**
     * Construct an empty instance.
     */
    public VInterfaceConfigList() {
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public VInterfaceConfigList(Random rand) {
        super(rand);
    }

    /**
     * Construct a new instance that contains the given number of virtual
     * interfaces using the given random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of virtual interfaces to add.
     */
    public VInterfaceConfigList(Random rand, int count) {
        super(rand, count);
    }

    // AbstractConfigList

    /**
     * Set the virtual interface configuration in this instance into the
     * specified XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    @Override
    public void setXml(XmlNode xnode) {
        XmlNode xvifs = new XmlNode("vinterfaces");
        for (VInterfaceConfig iconf: getConfigMap().values()) {
            xvifs.add(iconf.toXmlNode());
        }
        xnode.add(xvifs);
    }

    /**
     * Ensure that the given {@link XmlVInterface} instances are identical
     * to this instance.
     *
     * @param xvifs  A list of {@link XmlVInterface} instances.
     * @param jaxb   {@code true} indicates {@code xvifs} was deserialized from
     *               XML.
     */
    @Override
    public void verify(List<XmlVInterface> xvifs, boolean jaxb) {
        Map<Object, VInterfaceConfig> cfMap = getConfigMap();
        if (cfMap.isEmpty()) {
            assertEquals(null, xvifs);
        } else {
            assertNotNull(xvifs);
            Iterator<XmlVInterface> it = xvifs.iterator();
            for (VInterfaceConfig iconf: cfMap.values()) {
                assertEquals(true, it.hasNext());
                iconf.verify(it.next(), jaxb);
            }
            assertEquals(false, it.hasNext());
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected VInterfaceConfigList getThis() {
        return this;
    }

    /**
     * Construct a new virtual interface configuration using the given random
     * number generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  A new virtual interface configuration.
     */
    @Override
    protected VInterfaceConfig newConfig(Random rand) {
        String name = "vif_" + rand.nextInt(100000);
        return new VInterfaceConfig(name, rand);
    }

    /**
     * Return the key of the given virtual interface configuration.
     *
     * @param conf  An instance that specifies the virtual interface
     *              configuration.
     * @return  The key of the given configuration.
     */
    @Override
    protected String getKey(VInterfaceConfig conf) {
        return conf.getName();
    }

    /**
     * Convert the given configuration into a vinterface instance.
     *
     * @param conf  An instance that specifies the virtual interface
     *              configuration.
     * @return  A {@link Vinterface} instance.
     */
    @Override
    protected Vinterface toDataObject(VInterfaceConfig conf) {
        return conf.toVinterface();
    }
}
