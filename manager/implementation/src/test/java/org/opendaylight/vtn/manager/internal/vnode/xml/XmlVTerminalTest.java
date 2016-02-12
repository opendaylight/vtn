/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfigBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.Vbridge;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.list.VbridgeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.VtnVterminalInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.list.Vterminal;

/**
 * JUnit test for {@link XmlVTerminal}.
 */
public class XmlVTerminalTest extends TestBase {
    /**
     * Root XML element name associated with {@link XmlVTerminal} class.
     */
    public static final String  XML_ROOT = "vterminal";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link XmlVTerminal} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(
            "vinterfaces", XmlDataType.addPath(name, parent));
        return XmlVInterfaceTest.getXmlDataTypes("vinterface", p);
    }

    /**
     * Test case for {@link XmlVTerminal#XmlVTerminal(VtnVterminalInfo)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructor() throws Exception {
        // Test case for name and description.
        for (int i = 0; i < 5; i++) {
            String name = "vterm_" + i;
            String desc = (i == 0) ? null : "vTerminal " + i;
            VTerminalConfig vtconf = new VTerminalConfig(name, desc);
            VtnVterminalInfo vterm = vtconf.toVterminal();
            XmlVTerminal xvterm = new XmlVTerminal(vterm);
            vtconf.verify(xvterm, false);
        }
    }

    /**
     * Test case for {@link XmlAbstractBridge#getInterfaces()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetInterfaces() throws Exception {
        String name = "vterm";
        String desc = "vTerminal";
        VTerminalConfig vtconf = new VTerminalConfig(name, desc);

        Random rand = new Random(987654321L);
        int[] counts = {0, 1, 20};
        for (int count: counts) {
            VInterfaceConfigList iconfList =
                new VInterfaceConfigList(rand, count);
            vtconf.setInterfaces(iconfList);
            Vterminal vterm = vtconf.toVterminal();
            XmlVTerminal xvterm = new XmlVTerminal(vterm);
            vtconf.verify(xvterm, false);
        }
    }

    /**
     * Test case for {@link XmlAbstractBridge#equals(Object)} and
     * {@link XmlAbstractBridge#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        Set<Object> set = new HashSet<>();

        String[] names = {
            "vterm_1", "vterm_2", "vterminal_3",
        };
        String[] descs = {
            null, "desc 1", "desciption 2",
        };
        int count = 0;
        for (String name1: names) {
            String name2 = new String(name1);
            for (String desc1: descs) {
                String desc2 = (desc1 == null) ? null : new String(desc1);
                VTerminalConfig vtconf1 = new VTerminalConfig(name1, desc1);
                VTerminalConfig vtconf2 = new VTerminalConfig(name2, desc2);
                Vterminal vterm1 = vtconf1.toVterminal();
                Vterminal vterm2 = vtconf2.toVterminal();
                XmlVTerminal xvterm1 = new XmlVTerminal(vterm1);
                XmlVTerminal xvterm2 = new XmlVTerminal(vterm2);
                testEquals(set, xvterm1, xvterm2);
                count++;
            }
        }

        Random rand = new Random(3141592L);
        VInterfaceConfigList iconfList = new VInterfaceConfigList(rand, 20);
        VInterfaceConfigList ilist1 = new VInterfaceConfigList();
        VInterfaceConfigList ilist2 = new VInterfaceConfigList();
        String name1 = names[0];
        String desc1 = descs[1];
        String name2 = new String(name1);
        String desc2 = new String(desc1);
        for (VInterfaceConfig iconf: iconfList.getAll()) {
            ilist1.add(iconf);
            ilist2.add(iconf);
            VTerminalConfig vtconf1 = new VTerminalConfig(name1, desc1);
            VTerminalConfig vtconf2 = new VTerminalConfig(name2, desc2);
            vtconf1.setInterfaces(ilist1);
            vtconf2.setInterfaces(ilist2);
            Vterminal vterm1 = vtconf1.toVterminal();
            Vterminal vterm2 = vtconf2.toVterminal();
            XmlVTerminal xvterm1 = new XmlVTerminal(vterm1);
            XmlVTerminal xvterm2 = new XmlVTerminal(vterm2);
            testEquals(set, xvterm1, xvterm2);
            count++;
        }

        // vBridge should be distinguished from vTerminal.
        VbridgeConfig vbrc1 = new VbridgeConfigBuilder().
            setDescription(desc1).
            build();
        VbridgeConfig vbrc2 = new VbridgeConfigBuilder().
            setDescription(desc2).
            build();
        Vbridge vbr1 = new VbridgeBuilder().
            setName(new VnodeName(name1)).
            setVbridgeConfig(vbrc1).
            build();
        Vbridge vbr2 = new VbridgeBuilder().
            setName(new VnodeName(name2)).
            setVbridgeConfig(vbrc2).
            build();
        XmlVBridge xvbr1 = new XmlVBridge(vbr1);
        XmlVBridge xvbr2 = new XmlVBridge(vbr2);
        testEquals(set, xvbr1, xvbr2);
        count++;

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link XmlVTerminal#toVterminalBuilder()} and XML binding.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        Class<XmlVTerminal> type = XmlVTerminal.class;
        Unmarshaller um = createUnmarshaller(type);

        String[] names = {"vterm_1", "vterminal_1"};
        String[] descs = {null, "vterm desc", "vTerminal 1"};
        for (String name: names) {
            for (String desc: descs) {
                VTerminalConfig vtconf = new VTerminalConfig(name, desc);
                XmlNode xnode = vtconf.toXmlNode();
                XmlVTerminal xvterm = unmarshal(um, xnode.toString(), type);
                vtconf.verify(xvterm, true);
                jaxbTest(xvterm, type, XML_ROOT);
                vtconf.testToVterminalBuilder(xvterm);
            }
        }

        String name = "vterm";
        String desc = "vTerminal";
        VTerminalConfig vtconf = new VTerminalConfig(name, desc);

        Random rand = new Random(141421356L);
        VInterfaceConfigList iconfList = new VInterfaceConfigList(rand, 10);
        vtconf.setInterfaces(iconfList);
        XmlNode xnode = vtconf.toXmlNode();
        XmlVTerminal xvterm = unmarshal(um, xnode.toString(), type);
        vtconf.verify(xvterm, true);
        jaxbTest(xvterm, type, XML_ROOT);
        vtconf.testToVterminalBuilder(xvterm);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // Node name is missing.
        xnode = new XmlNode(XML_ROOT).
            add(new XmlNode("description", desc));
        xvterm = unmarshal(um, xnode.toString(), type);
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String msg = "vTerminal name cannot be null";
        try {
            xvterm.toVterminalBuilder();
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(msg, e.getMessage());
        }
    }
}
