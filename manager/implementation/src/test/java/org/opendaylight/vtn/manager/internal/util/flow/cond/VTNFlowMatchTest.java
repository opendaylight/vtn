/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.cond;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.util.VTNIdentifiableComparator;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlNode;
import org.opendaylight.vtn.manager.internal.XmlValueType;
import org.opendaylight.vtn.manager.internal.util.flow.match.VTNMatchTest;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnFlowMatchConfig;

/**
 * JUnit test for {@link VTNFlowMatch}.
 */
public class VTNFlowMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link VTNFlowMatch} class.
     */
    private static final String  XML_ROOT = "vtn-flow-match";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link VTNFlowMatch} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = VTNMatchTest.getXmlDataTypes(name, parent);
        dlist.add(new XmlValueType("index",
                                   Integer.class).add(name).prepend(parent));
        return dlist;
    }

    /**
     * Test case for constructors and getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        List<VTNFlowMatch> list = new ArrayList<>();

        // Test for the upper limit of index.
        final int maxId = 65535;
        FlowMatchParams params = new FlowMatchParams().setIndex(maxId);
        VTNFlowMatch vfmatch = params.toVTNFlowMatch();
        params.verify(vfmatch);
        list.add(vfmatch);

        Map<FlowMatchParams, FlowMatchParams> cases =
            FlowMatchParams.createFlowMatches();
        for (Map.Entry<FlowMatchParams, FlowMatchParams> entry:
                 cases.entrySet()) {
            params = entry.getKey();
            FlowMatchParams expected = entry.getValue();
            vfmatch = params.toVTNFlowMatch();
            expected.verify(vfmatch);
            list.add(vfmatch);
        }

        // Ensure that VTNFlowMatch can be sorted by VTNIdentifiableComparator.
        VTNIdentifiableComparator<Integer> comparator =
            new VTNIdentifiableComparator<>(Integer.class);
        Collections.sort(list, comparator);
        int prev = 0;
        for (VTNFlowMatch vf: list) {
            int id = vf.getIdentifier().intValue();
            assertTrue(id > prev);
            prev = id;
        }
        assertEquals(maxId, prev);

        // Null argument.
        try {
            new VTNFlowMatch((FlowMatch)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("Flow match cannot be null", st.getDescription());
        }

        try {
            new VTNFlowMatch((VtnFlowMatchConfig)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            Status st = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, st.getCode());
            assertEquals("VTN flow match cannot be null", st.getDescription());
        }

        // Invalid index.
        Integer[] badIndices = {
            null, Integer.MIN_VALUE, -10000000, -3000, -200, -3, -2, -1, 0,
            65536, 65537, 70000, 1000000, Integer.MAX_VALUE,
        };
        for (Integer idx: badIndices) {
            String msg;
            RpcErrorTag etag;
            if (idx == null) {
                msg = "Match index cannot be null";
                etag = RpcErrorTag.MISSING_ELEMENT;
            } else {
                msg = "Invalid match index: " + idx;
                etag = RpcErrorTag.BAD_ELEMENT;
            }

            params = new FlowMatchParams().setIndex(idx);
            FlowMatch fm = params.toFlowMatch();
            try {
                new VTNFlowMatch(fm);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }
    }

    /**
     * Test case for object identity.
     *
     * <ul>
     *   <li>{@link VTNFlowMatch#equals(Object)}</li>
     *   <li>{@link VTNFlowMatch#hashCode()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<>();

        Map<FlowMatchParams, FlowMatchParams> cases =
            FlowMatchParams.createFlowMatches();
        for (Map.Entry<FlowMatchParams, FlowMatchParams> entry:
                 cases.entrySet()) {
            FlowMatchParams params = entry.getKey();
            FlowMatchParams expected = entry.getValue();
            VTNFlowMatch vfmatch1 = params.toVTNFlowMatch();
            VTNFlowMatch vfmatch2 = expected.toVTNFlowMatch();
            testEquals(set, vfmatch1, vfmatch2);

            Integer idx = params.getIndex().intValue() + 1;
            FlowMatchParams p = params.clone().setIndex(idx);
            vfmatch1 = p.toVTNFlowMatch();
            vfmatch2 = p.toVTNFlowMatch();
            testEquals(set, vfmatch1, vfmatch2);
        }

        assertEquals(cases.size() * 2, set.size());
    }

    /**
     * Test case for {@link VTNFlowMatch#verify()} and JAXB mapping.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        // Normal case.
        Class<VTNFlowMatch> type = VTNFlowMatch.class;
        Unmarshaller um = createUnmarshaller(type);
        Map<FlowMatchParams, FlowMatchParams> cases =
            FlowMatchParams.createFlowMatches();
        for (Map.Entry<FlowMatchParams, FlowMatchParams> entry:
                 cases.entrySet()) {
            FlowMatchParams params = entry.getKey();
            FlowMatchParams expected = entry.getValue();
            String xml = params.toXmlNode(XML_ROOT).toString();
            VTNFlowMatch vfmatch = unmarshal(um, xml, type);
            vfmatch.verify();
            expected.verify(vfmatch);

            xml = expected.toXmlNode(XML_ROOT).toString();
            VTNFlowMatch vfmatch1 = unmarshal(um, xml, type);
            vfmatch1.verify();
            assertEquals(vfmatch, vfmatch1);
        }

        // Invalid index.
        Integer[] badIndices = {
            null, Integer.MIN_VALUE, -10000000, -3000, -200, -3, -2, -1, 0,
            65536, 65537, 70000, 1000000, Integer.MAX_VALUE,
        };
        for (Integer idx: badIndices) {
            XmlNode xn = new XmlNode(XML_ROOT);
            String msg;
            RpcErrorTag etag;
            if (idx == null) {
                msg = "Match index cannot be null";
                etag = RpcErrorTag.MISSING_ELEMENT;
            } else {
                msg = "Invalid match index: " + idx;
                etag = RpcErrorTag.BAD_ELEMENT;
                xn.add(new XmlNode("index", idx));
            }

            VTNFlowMatch vfmatch = unmarshal(um, xn.toString(), type);
            try {
                vfmatch.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                Status st = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, st.getCode());
                assertEquals(msg, st.getDescription());
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));
    }
}
