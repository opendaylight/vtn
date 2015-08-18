/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.flow.action.FlowAction;
import org.opendaylight.vtn.manager.flow.action.SetDlDstAction;
import org.opendaylight.vtn.manager.flow.action.SetDlSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetDscpAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeAction;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4DstAction;
import org.opendaylight.vtn.manager.flow.action.SetInet4SrcAction;
import org.opendaylight.vtn.manager.flow.action.SetTpDstAction;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcAction;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpAction;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;
import org.opendaylight.vtn.manager.XmlDataType;
import org.opendaylight.vtn.manager.flow.action.SetDscpActionTest;
import org.opendaylight.vtn.manager.flow.action.SetIcmpCodeActionTest;
import org.opendaylight.vtn.manager.flow.action.SetIcmpTypeActionTest;
import org.opendaylight.vtn.manager.flow.action.SetTpDstActionTest;
import org.opendaylight.vtn.manager.flow.action.SetTpSrcActionTest;
import org.opendaylight.vtn.manager.flow.action.SetVlanPcpActionTest;

/**
 * JUnit test for {@link FlowFilter}.
 */
public class FlowFilterTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowFilter} class.
     */
    private static final String  XML_ROOT = "flowfilter";

    /**
     * A list of {@link FilterType} instances for test.
     */
    private List<FilterType>  filterTypes;

    /**
     * A list of {@link FlowAction} instance lists for test.
     */
    private List<List<FlowAction>>  actionLists;

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link FlowFilter} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        String[] p = XmlDataType.addPath(
            "actions", XmlDataType.addPath(name, parent));
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        dlist.add(new XmlAttributeType(name, "index", Integer.class).
                  add(parent));
        dlist.addAll(SetVlanPcpActionTest.getXmlDataTypes("vlanpcp", p));
        dlist.addAll(SetDscpActionTest.getXmlDataTypes("dscp", p));
        dlist.addAll(SetTpSrcActionTest.getXmlDataTypes("tpsrc", p));
        dlist.addAll(SetTpDstActionTest.getXmlDataTypes("tpdst", p));
        dlist.addAll(SetIcmpTypeActionTest.getXmlDataTypes("icmptype", p));
        dlist.addAll(SetIcmpCodeActionTest.getXmlDataTypes("icmpcode", p));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        int[] indices = {1, 100, 1000, 65535};
        for (int index: indices) {
            for (String cond: createStrings("cnd")) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = new FlowFilter(cond, type, actions);
                        List<FlowAction> exacts = actions;
                        if (exacts != null && exacts.isEmpty()) {
                            exacts = null;
                        }
                        assertEquals(null, ff.getIndex());
                        assertEquals(cond, ff.getFlowConditionName());
                        assertEquals(type, ff.getFilterType());
                        assertEquals(exacts, ff.getActions());

                        ff = new FlowFilter(index, cond, type, actions);
                        assertEquals(Integer.valueOf(index), ff.getIndex());
                        assertEquals(cond, ff.getFlowConditionName());
                        assertEquals(type, ff.getFilterType());

                        List<FlowAction> list = ff.getActions();
                        assertEquals(exacts, list);

                        // Ensure that a copy of action list is returned.
                        if (list != null) {
                            list.clear();
                            assertEquals(exacts, ff.getActions());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link FlowFilter#equals(Object)} and
     * {@link FlowFilter#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        int[] indices = {0, 1, 65535};
        List<String> conditions = createStrings("cnd");
        List<FilterType> types = createFilterTypes();
        List<List<FlowAction>> actLists = createActionLists();
        for (Iterator<List<FlowAction>> it = actLists.iterator();
             it.hasNext();) {
            List<FlowAction> act = it.next();
            if (act == null) {
                it.remove();
                break;
            }
        }

        for (int index: indices) {
            for (String cond: conditions) {
                for (FilterType type: types) {
                    for (List<FlowAction> actions: actLists) {
                        FlowFilter ff1, ff2;

                        if (index == 0) {
                            ff1 = new FlowFilter(cond, type, actions);
                            ff2 = new FlowFilter(
                                copy(cond), copy(type),
                                copy(actions, FlowAction.class));
                        } else {
                            ff1 = new FlowFilter(index, cond, type, actions);
                            ff2 = new FlowFilter(
                                index, copy(cond), copy(type),
                                copy(actions, FlowAction.class));
                        }
                        testEquals(set, ff1, ff2);
                    }
                }
            }
        }

        int required = indices.length * conditions.size() * types.size() *
            actLists.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link FlowFilter#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "FlowFilter[";
        String suffix = "]";
        int[] indices = {0, 1, 1000, 65535};
        for (int index: indices) {
            for (String cond: createStrings("cnd")) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = (index == 0)
                            ? new FlowFilter(cond, type, actions)
                            : new FlowFilter(index, cond, type, actions);
                        String i = (index == 0)
                            ? null : "index=" + index;
                        String c = (cond == null) ? null : "cond=" + cond;
                        String t = (type == null) ? null : "type=" + type;
                        String a = (actions == null || actions.isEmpty())
                            ? null : "actions=" + actions;
                        String required = joinStrings(prefix, suffix, ",",
                                                      i, c, t, a);
                        assertEquals(required, ff.toString());
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowFilter} is serializable.
     */
    @Test
    public void testSerialize() {
        int[] indices = {0, 100, 65535};
        for (int index: indices) {
            for (String cond: createStrings("cnd")) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = (index == 0)
                            ? new FlowFilter(cond, type, actions)
                            : new FlowFilter(index, cond, type, actions);
                        serializeTest(ff);
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link FlowFilter} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        int[] indices = {0, 100, 65535};
        for (int index: indices) {
            for (String cond: createStrings("cnd")) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = (index == 0)
                            ? new FlowFilter(cond, type, actions)
                            : new FlowFilter(index, cond, type, actions);
                        jaxbTest(ff, FlowFilter.class, XML_ROOT);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(FlowFilter.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link FlowFilter} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        int[] indices = {0, 100, 65535};
        for (int index: indices) {
            for (String cond: createStrings("cnd")) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = (index == 0)
                            ? new FlowFilter(cond, type, actions)
                            : new FlowFilter(index, cond, type, actions);
                        jsonTest(ff, FlowFilter.class);
                    }
                }
            }
        }
    }

    /**
     * Create a list of {@link FilterType} instances.
     *
     * @return  A list of {@link FilterType} instances.
     */
    private List<FilterType> createFilterTypes() {
        List<FilterType> list = filterTypes;
        if (list == null) {
            list = new ArrayList<FilterType>();
            list.add(null);
            list.add(new PassFilter());
            list.add(new DropFilter());
            list.add(new RedirectFilter(
                         new VBridgeIfPath(null, "bname", "if1"), true));
            list.add(new RedirectFilter(
                         new VTerminalIfPath("vtn", "vtname", "if2"), false));
            filterTypes = list;
        }

        return list;
    }

    /**
     * Create lists of {@link FlowAction} instances.
     *
     * @return  A list of {@link FlowAction} instance lists.
     */
    private List<List<FlowAction>> createActionLists() {
        List<List<FlowAction>> list = actionLists;
        if (list == null) {
            list = new ArrayList<List<FlowAction>>();
            byte[] mac1 = {
                (byte)0x00, (byte)0x01, (byte)0x02,
                (byte)0x03, (byte)0x04, (byte)0x05,
            };
            byte[] mac2 = {
                (byte)0x10, (byte)0x20, (byte)0x30,
                (byte)0x40, (byte)0x50, (byte)0x60,
            };
            byte[] mac3 = {
                (byte)0xa0, (byte)0xbb, (byte)0xcc,
                (byte)0xdd, (byte)0xee, (byte)0xff,
            };
            byte[] mac4 = {
                (byte)0xfa, (byte)0xf0, (byte)0xf1,
                (byte)0xfa, (byte)0xfb, (byte)0xfc,
            };

            InetAddress addr1, addr2;
            try {
                addr1 = InetAddress.getByName("192.168.100.1");
                addr2 = InetAddress.getByName("192.168.200.123");
            } catch (Exception e) {
                unexpected(e);
                return null;
            }

            list.add(null);
            list.add(new ArrayList<FlowAction>());
            list.add(createActions(new SetDscpAction((byte)63)));
            list.add(createActions(new SetDlSrcAction(mac1),
                                   new SetDlDstAction(mac2),
                                   new SetVlanPcpAction((byte)3)));

            // Create all-in-one list.
            list.add(createActions(new SetVlanPcpAction((byte)7),
                                   new SetDscpAction((byte)4),
                                   new SetTpSrcAction(65535),
                                   new SetTpDstAction(0),
                                   new SetDlSrcAction(mac3),
                                   new SetDlDstAction(mac4),
                                   new SetIcmpTypeAction((short)255),
                                   new SetIcmpCodeAction((short)128),
                                   new SetInet4SrcAction(addr1),
                                   new SetInet4DstAction(addr2)));

            actionLists = list;
        }

        return list;
    }

    /**
     * Create a list of {@link FlowAction} instances.
     *
     * @param actions  An array of {@link FlowAction} instances to be added
     *                 to the list.
     * @return  A list of {@link FlowAction} instances.
     */
    private List<FlowAction> createActions(FlowAction ... actions) {
        List<FlowAction> list = new ArrayList<FlowAction>();
        for (FlowAction act: actions) {
            list.add(act);
        }

        return list;
    }
}
