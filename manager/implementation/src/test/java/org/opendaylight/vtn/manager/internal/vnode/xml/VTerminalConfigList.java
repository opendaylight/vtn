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

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * {@code VTerminalConfigList} describes configuration about a list of
 * vTerminal.
 */
public final class VTerminalConfigList
    extends AbstractConfigList<XmlVTerminal, VTerminalConfig,
                               Vterminal, VTerminalConfigList> {
    /**
     * Convert the given instance into a list of vterminal instances.
     *
     * @param list  A {@link VTerminalConfigList} instance.
     * @return  A list of vterminal or {@code null}.
     */
    public static List<Vterminal> toVterminalList(VTerminalConfigList list) {
        return (list == null)
            ? null
            : list.toDataObjects();
    }

    /**
     * Verify the given vTerminal list.
     *
     * @param expected  A {@link VTerminalConfigList} instance that contains
     *                  the expected vTerminal configuration.
     * @param xvterms   A list of {@link XmlVTerminal} instances.
     * @param jaxb      {@code true} indicates {@code xvterms} was deserialized
     *                  from XML.
     */
    public static void verify(VTerminalConfigList expected,
                              List<XmlVTerminal> xvterms, boolean jaxb) {
        if (expected == null) {
            assertEquals(null, xvterms);
        } else {
            expected.verify(xvterms, jaxb);
        }
    }

    /**
     * Construct an empty instance.
     */
    public VTerminalConfigList() {
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param rand  A pseudo random number generator.
     */
    public VTerminalConfigList(Random rand) {
        super(rand);
    }

    /**
     * Construct a new instance that contains the given number of vTerminal
     * configurations using the given random number generator.
     *
     * @param rand   A pseudo random number generator.
     * @param count  The number of vTerminals to add.
     */
    public VTerminalConfigList(Random rand, int count) {
        super(rand, count);
    }

    // AbstractConfigList

    /**
     * Set the vTerminal configuration in this instance into the specified
     * XML node.
     *
     * @param xnode  A {@link XmlNode} instance.
     */
    @Override
    public void setXml(XmlNode xnode) {
        XmlNode xvterms = new XmlNode("vterminals");
        for (VTerminalConfig vtconf: getConfigMap().values()) {
            xvterms.add(vtconf.toXmlNode());
        }
        xnode.add(xvterms);
    }

    /**
     * Ensure that the given {@link XmlVTerminal} instances are identical
     * to this instance.
     *
     * @param xvterms  A list of {@link XmlVTerminal} instances.
     * @param jaxb     {@code true} indicates {@code xvifs} was deserialized
     *                 from XML.
     */
    @Override
    public void verify(List<XmlVTerminal> xvterms, boolean jaxb) {
        Map<Object, VTerminalConfig> cfMap = getConfigMap();
        if (cfMap.isEmpty()) {
            assertEquals(null, xvterms);
        } else {
            assertNotNull(xvterms);
            Iterator<XmlVTerminal> it = xvterms.iterator();
            for (VTerminalConfig vtconf: cfMap.values()) {
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
    protected VTerminalConfigList getThis() {
        return this;
    }

    /**
     * Construct a new vTerminal configuration using the given random number
     * generator.
     *
     * @param rand  A pseudo random number generator.
     * @return  A new vTerminal configuration.
     */
    @Override
    protected VTerminalConfig newConfig(Random rand) {
        String name = "vterm_" + rand.nextInt(1000000);
        return new VTerminalConfig(name, rand);
    }

    /**
     * Return the key of the given vTerminal configuration.
     *
     * @param conf  An instance that specifies the vTerminal configuration.
     * @return  The key of the given configuration.
     */
    @Override
    protected String getKey(VTerminalConfig conf) {
        return conf.getName();
    }

    /**
     * Convert the given configuration into a vterminal instance.
     *
     * @param conf  An instance that specifies the vTerminal configuration.
     * @return  A {@link Vterminal} instance.
     */
    @Override
    protected Vterminal toDataObject(VTerminalConfig conf) {
        return conf.toVterminal();
    }
}
