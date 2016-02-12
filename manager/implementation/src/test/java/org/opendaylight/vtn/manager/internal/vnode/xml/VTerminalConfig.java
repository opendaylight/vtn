/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import static org.junit.Assert.assertEquals;

import static org.opendaylight.vtn.manager.internal.vnode.xml.VInterfaceConfigList.toVinterfaceList;
import static org.opendaylight.vtn.manager.internal.vnode.xml.XmlVTerminalTest.XML_ROOT;

import java.util.Random;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.VterminalBuilder;

/**
 * {@code VTerminalConfig} describes configuration about a vTerminal.
 */
public final class VTerminalConfig extends VirtualBridgeConfig {
    /**
     * Construct a new instance.
     *
     * @param name  The name of the vTerminal.
     */
    public VTerminalConfig(String name) {
        super(name, null);
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the vTerminal.
     * @param desc  Description about the vTerminal.
     */
    public VTerminalConfig(String name, String desc) {
        super(name, desc);
    }

    /**
     * Construct a new instance using the given random number generator.
     *
     * @param name  The name of the vTerminal.
     * @param rand  A pseudo random number generator.
     */
    public VTerminalConfig(String name, Random rand) {
        super(name, "vTerminal: " + rand.nextInt(), rand);
    }

    /**
     * Convert this instance into a vterminal-config instance.
     *
     * @return  A {@link VterminalConfig} instance.
     */
    public VterminalConfig toVterminalConfig() {
        return new VterminalConfigBuilder().
            setDescription(getDescription()).
            build();
    }

    /**
     * Convert this instance into a vterminal instance.
     *
     * @return  A {@link Vterminal} instance.
     */
    public Vterminal toVterminal() {
        return new VterminalBuilder().
            setName(getVnodeName()).
            setVterminalConfig(toVterminalConfig()).
            setBridgeStatus(newBridgeStatus()).
            setVinterface(toVinterfaceList(getInterfaces())).
            build();
    }

    /**
     * Ensure that the given {@link XmlVTerminal} instance is identical to
     * this instance.
     *
     * @param xvterm  A {@link XmlVTerminal} instance.
     * @param jaxb    {@code true} indicates {@code xvterm} was deserialized
     *                from XML.
     */
    public void verify(XmlVTerminal xvterm, boolean jaxb) {
        verify((XmlVNode)xvterm);
        VInterfaceConfigList.
            verify(getInterfaces(), xvterm.getInterfaces(), jaxb);
    }

    /**
     * Test case for {@link XmlVTerminal#toVterminalBuilder()}.
     *
     * @param xvterm  A {@link XmlVTerminal} instance that contains the same
     *                configuration as this instance.
     * @throws Exception  An error occurred.
     */
    public void testToVterminalBuilder(XmlVTerminal xvterm) throws Exception {
        VterminalBuilder builder = xvterm.toVterminalBuilder();
        assertEquals(getName(), builder.getName().getValue());

        VterminalConfig vtmc = builder.getVterminalConfig();
        assertEquals(getDescription(), vtmc.getDescription());

        // Other fields should be always null.
        assertEquals(null, builder.getBridgeStatus());
        assertEquals(null, builder.getVinterface());
    }

    /**
     * Convert this instance into a XML node.
     *
     * @return  A {@link XmlNode} instance that indicates the vTerminal
     *          configuration in this instance.
     */
    public XmlNode toXmlNode() {
        XmlNode xnode = new XmlNode(XML_ROOT);
        setXml(xnode);
        setInterfacesAsXml(xnode);
        return xnode;
    }
}
