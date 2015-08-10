/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

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
import org.opendaylight.vtn.manager.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.flow.filter.FilterType;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;
import org.opendaylight.vtn.manager.flow.filter.PassFilter;
import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

/**
 * JUnit test for {@link FlowFilterList}.
 */
public class FlowFilterListTest extends TestBase {
    /**
     * Root XML element name associated with {@link FlowFilterList} class.
     */
    private static final String  XML_ROOT = "flowfilters";

    /**
     * A list of {@link FilterType} instances for test.
     */
    private List<FilterType>  filterTypes;

    /**
     * A list of {@link FlowAction} instance lists for test.
     */
    private List<List<FlowAction>>  actionLists;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        // Null list.
        FlowFilterList nullList = new FlowFilterList(null);
        assertNull(nullList.getFilters());

        // Empty list.
        List<FlowFilter> filters = new ArrayList<FlowFilter>();
        FlowFilterList emptyList = new FlowFilterList(filters);
        assertEquals(filters, emptyList.getFilters());

        int[] indices = {0, 1, 65535};
        for (int index: indices) {
            for (String cond: createStrings("cnd")) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = (index == 0)
                            ? new FlowFilter(cond, type, actions)
                            : new FlowFilter(index, cond, type, actions);
                        filters.add(ff);

                        List<FlowFilter> list =
                            new ArrayList<FlowFilter>(filters);
                        FlowFilterList fl = new FlowFilterList(list);
                        assertEquals(list, fl.getFilters());
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link FlowFilterList#equals(Object)} and
     * {@link FlowFilterList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // Null list.
        FlowFilterList nullList = new FlowFilterList(null);
        testEquals(set, nullList, new FlowFilterList(null));

        // Empty list should be treated as null list.
        List<FlowFilter> filters1 = new ArrayList<FlowFilter>();
        List<FlowFilter> filters2 = new ArrayList<FlowFilter>();
        FlowFilterList emptyList = new FlowFilterList(filters1);
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));

        int[] indices = {0, 1, 65535};
        String[] conditions = {null, "condition"};
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
                        List<FlowAction> alist =
                            new ArrayList<FlowAction>(actions);
                        if (index == 0) {
                            ff1 = new FlowFilter(cond, type, actions);
                            ff2 = new FlowFilter(copy(cond), type, alist);
                        } else {
                            ff1 = new FlowFilter(index, cond, type, actions);
                            ff2 = new FlowFilter(index, copy(cond), type,
                                                 alist);
                        }

                        filters1.add(ff1);
                        filters2.add(ff2);
                        List<FlowFilter> list1 =
                            new ArrayList<FlowFilter>(filters1);
                        List<FlowFilter> list2 =
                            new ArrayList<FlowFilter>(filters2);
                        FlowFilterList fl1 = new FlowFilterList(list1);
                        FlowFilterList fl2 = new FlowFilterList(list2);
                        testEquals(set, fl1, fl2);
                    }
                }
            }
        }

        int required = indices.length * conditions.length * types.size() *
            actLists.size() + 1;
        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link FlowFilterList} is mapped to both XML root element
     * and JSON object.
     */
    @Test
    public void testJAXB() {
        // Null list.
        FlowFilterList fl = new FlowFilterList(null);
        jaxbTest(fl, FlowFilterList.class, XML_ROOT);
        jsonTest(fl, FlowFilterList.class);

        // Empty list.
        List<FlowFilter> filters = new ArrayList<FlowFilter>();
        fl = new FlowFilterList(filters);
        jaxbTest(fl, FlowFilterList.class, XML_ROOT);
        jsonTest(fl, FlowFilterList.class);

        int[] indices = {0, 1, 65535};
        String[] conditions = {null, "condition"};
        for (int index: indices) {
            for (String cond: conditions) {
                for (FilterType type: createFilterTypes()) {
                    for (List<FlowAction> actions: createActionLists()) {
                        FlowFilter ff = (index == 0)
                            ? new FlowFilter(cond, type, actions)
                            : new FlowFilter(index, cond, type, actions);
                        filters.add(ff);

                        List<FlowFilter> list =
                            new ArrayList<FlowFilter>(filters);
                        fl = new FlowFilterList(list);
                        jaxbTest(fl, FlowFilterList.class, XML_ROOT);
                        jsonTest(fl, FlowFilterList.class);
                    }
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(FlowFilterList.class,
                      // FlowFilter
                      new XmlAttributeType("flowfilter", "index",
                                           Integer.class).add(XML_ROOT),

                      // FlowAction
                      new XmlAttributeType("vlanpcp", "priority", byte.class).
                      add(XML_ROOT, "flowfilter", "actions"),
                      new XmlAttributeType("dscp", "dscp", byte.class).
                      add(XML_ROOT, "flowfilter", "actions"),
                      new XmlAttributeType("tpsrc", "port", int.class).
                      add(XML_ROOT, "flowfilter", "actions"),
                      new XmlAttributeType("tpdst", "port", int.class).
                      add(XML_ROOT, "flowfilter", "actions"),
                      new XmlAttributeType("icmptype", "type", short.class).
                      add(XML_ROOT, "flowfilter", "actions"),
                      new XmlAttributeType("icmpcode", "code", short.class).
                      add(XML_ROOT, "flowfilter", "actions"));
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
            list.add(createActions(new SetDscpAction((byte)63),
                                   new SetDlSrcAction(mac1),
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
