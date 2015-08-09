/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;

/**
 * JUnit test for {@link VNodeRoute}.
 */
public class VNodeRouteTest extends TestBase {
    /**
     * Root XML element name associated with {@link VNodeRoute} class.
     */
    private static final String  XML_ROOT = "vnoderoute";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        VirtualRouteReason[] reasons = VirtualRouteReason.values();
        for (VNodePath path: createVNodePaths(10)) {
            VNodeRoute vnr = new VNodeRoute(path, null);
            assertEquals(path, vnr.getPath());
            assertEquals(null, vnr.getReason());
            assertEquals(null, vnr.getReasonString());
            for (VirtualRouteReason reason: reasons) {
                vnr = new VNodeRoute(path, reason);
                assertEquals(path, vnr.getPath());
                assertEquals(reason, vnr.getReason());
                assertEquals(reason.name(), vnr.getReasonString());
            }
        }
    }

    /**
     * Test case for {@link VNodeRoute#equals(Object)} and
     * {@link VNodeRoute#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<VNodePath> paths = createVNodePaths(10);
        List<VirtualRouteReason> reasons = createReasons();
        for (VNodePath path: paths) {
            for (VirtualRouteReason reason: reasons) {
                VNodeRoute vnr1 = new VNodeRoute(path, reason);
                VNodePath path1 = (path == null)
                    ? null : (VNodePath)path.clone();
                VNodeRoute vnr2 = new VNodeRoute(path1, reason);
                testEquals(set, vnr1, vnr2);
            }
        }

        int required = paths.size() * reasons.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link VNodeRoute#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "VNodeRoute[";
        String suffix = "]";
        for (VNodePath path: createVNodePaths(10)) {
            String p = (path == null) ? null : "path=" + path;
            for (VirtualRouteReason reason: createReasons()) {
                String r = (reason == null) ? null : "reason=" + reason;
                VNodeRoute vnr = new VNodeRoute(path, reason);
                String required = joinStrings(prefix, suffix, ",", p, r);
                assertEquals(required, vnr.toString());
            }
        }
    }

    /**
     * Ensure that {@link VNodeRoute} is serializable.
     */
    @Test
    public void testSerialize() {
        for (VNodePath path: createVNodePaths(10)) {
            for (VirtualRouteReason reason: createReasons()) {
                VNodeRoute vnr = new VNodeRoute(path, reason);
                serializeTest(vnr);
            }
        }
    }

    /**
     * Ensure that {@link VNodeRoute} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        String[] invalidReasons = {
            "", "1", "UNKNOWN", "BAD_REASON",
        };
        for (VNodePath path: createVNodePaths(10)) {
            for (VirtualRouteReason reason: createReasons()) {
                VNodeRoute vnr = new VNodeRoute(path, reason);
                jaxbTest(vnr, VNodeRoute.class, XML_ROOT);
            }

            // Specifying invalid reason.
            for (String r: invalidReasons) {
                VNodeRoute vnr = createVNodeRoute(path, r);
                assertEquals(path, vnr.getPath());
                assertEquals(null, vnr.getReason());
            }
        }
    }

    /**
     * Ensure that {@link PathCost} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        for (VNodePath path: createVNodePaths(10)) {
            for (VirtualRouteReason reason: createReasons()) {
                VNodeRoute vnr = new VNodeRoute(path, reason);
                jsonTest(vnr, VNodeRoute.class);
            }
        }
    }

    /**
     * Test case for {@link VirtualRouteReason} class.
     */
    @Test
    public void testReason() {
        VirtualRouteReason[] values = VirtualRouteReason.values();
        assertEquals(6, values.length);
        assertEquals(VirtualRouteReason.PORTMAPPED, values[0]);
        assertEquals(VirtualRouteReason.VLANMAPPED, values[1]);
        assertEquals(VirtualRouteReason.MACMAPPED, values[2]);
        assertEquals(VirtualRouteReason.FORWARDED, values[3]);
        assertEquals(VirtualRouteReason.REDIRECTED, values[4]);
        assertEquals(VirtualRouteReason.LINKED, values[5]);
        assertEquals(0, VirtualRouteReason.PORTMAPPED.ordinal());
        assertEquals(1, VirtualRouteReason.VLANMAPPED.ordinal());
        assertEquals(2, VirtualRouteReason.MACMAPPED.ordinal());
        assertEquals(3, VirtualRouteReason.FORWARDED.ordinal());
        assertEquals(4, VirtualRouteReason.REDIRECTED.ordinal());
        assertEquals(5, VirtualRouteReason.LINKED.ordinal());

        for (VirtualRouteReason reason: values) {
            serializeTest(reason);
        }
    }

    /**
     * Create a list of {@link VirtualRouteReason} instances.
     *
     * @return  A list which contains {@link VirtualRouteReason} instances and
     *          a {@code null}.
     */
    private List<VirtualRouteReason> createReasons() {
        List<VirtualRouteReason> reasons = new ArrayList<>();
        reasons.add(null);
        for (VirtualRouteReason reason: VirtualRouteReason.values()) {
            reasons.add(reason);
        }

        return reasons;
    }

    /**
     * Construct an instance of {@link VNodeRoute} using JAXB.
     *
     * @param path    A {@link VNodePath} instance which specifies
     *                the location of the virtual node inside the VTN.
     * @param reason  A string representation of {@link VirtualRouteReason}
     *                instance.
     * @return  A {@link VNodeRoute} instance.
     */
    private VNodeRoute createVNodeRoute(VNodePath path, String reason) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (reason != null) {
            builder.append(" reason=\"").append(reason).append('"');
        }

        builder.append('>');
        if (path != null) {
            builder.append("<path");
            String tname = path.getTenantName();
            if (tname != null) {
                builder.append(" tenant=\"").append(tname).append('"');
            }

            String nodeName = path.getTenantNodeName();
            if (nodeName != null) {
                if (path instanceof VBridgePath) {
                    builder.append(" bridge=\"").append(nodeName).append('"');
                } else if (path instanceof VTerminalPath) {
                    builder.append(" terminal=\"").append(nodeName).
                        append('"');
                }
            }

            if (path instanceof VInterfacePath) {
                VInterfacePath ipath = (VInterfacePath)path;
                String iname = ipath.getInterfaceName();
                if (iname != null) {
                    builder.append(" interface=\"").append(iname).append('"');
                }
            }
            builder.append(" />");
        }

        String xml = builder.append("</").append(XML_ROOT).append('>').
            toString();
        Unmarshaller um = createUnmarshaller(VNodeRoute.class);
        VNodeRoute vnr = null;
        try {
            vnr = unmarshal(um, xml, VNodeRoute.class);
        } catch (Exception e) {
            unexpected(e);
        }

        return vnr;
    }
}
