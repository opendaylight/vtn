/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.HashSet;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

/**
 * JUnit test for {@link VNodeLocation}.
 */
public class VNodeLocationTest extends TestBase {
    /**
     * Root XML element name associated with {@link VNodeLocation} class.
     */
    private static final String  XML_ROOT = "vnodelocation";

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String tname: createStrings("tnt")) {
            for (String bname: createStrings("brg")) {
                VBridgePath bpath = new VBridgePath(tname, bname);
                VNodeLocation vloc = new VNodeLocation(bpath);
                assertEquals(tname, vloc.getTenantName());
                assertEquals(bname, vloc.getBridgeName());
                assertEquals(null, vloc.getRouterName());
                assertEquals(null, vloc.getTerminalName());
                assertEquals(null, vloc.getInterfaceName());

                VTerminalPath tmpath = new VTerminalPath(tname, bname);
                vloc = new VNodeLocation(tmpath);
                assertEquals(tname, vloc.getTenantName());
                assertEquals(null, vloc.getBridgeName());
                assertEquals(null, vloc.getRouterName());
                assertEquals(bname, vloc.getTerminalName());
                assertEquals(null, vloc.getInterfaceName());

                for (String iname: createStrings("vif")) {
                    VBridgeIfPath bipath = new VBridgeIfPath(bpath, iname);
                    vloc = new VNodeLocation(bipath);
                    assertEquals(tname, vloc.getTenantName());
                    assertEquals(bname, vloc.getBridgeName());
                    assertEquals(null, vloc.getRouterName());
                    assertEquals(null, vloc.getTerminalName());
                    assertEquals(iname, vloc.getInterfaceName());

                    VTerminalIfPath tipath =
                        new VTerminalIfPath(tmpath, iname);
                    vloc = new VNodeLocation(tipath);
                    assertEquals(tname, vloc.getTenantName());
                    assertEquals(null, vloc.getBridgeName());
                    assertEquals(null, vloc.getRouterName());
                    assertEquals(bname, vloc.getTerminalName());
                    assertEquals(iname, vloc.getInterfaceName());
                }
            }
        }
    }

    /**
     * Test case for {@link VNodeLocation#equals(Object)} and
     * {@link VNodeLocation#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        String[] tenantNames = {null, "tenant_1", "tenant_2"};
        String[] bridgeNames = {null, "bridge_1", "bridge_2"};
        String[] routerNames = {null, "router_1", "router_2"};
        String[] terminalNames = {null, "term_1", "term_2"};
        String[] ifNames = {null, "if1", "if2"};

        for (String tname: tenantNames) {
            for (String bname: bridgeNames) {
                for (String rname: routerNames) {
                    for (String tmname: terminalNames) {
                        for (String iname: ifNames) {
                            VNodeLocation vloc1 = createVNodeLocation(
                                tname, bname, rname, tmname, iname);
                            VNodeLocation vloc2 = createVNodeLocation(
                                tname, bname, rname, tmname, iname);
                            testEquals(set, vloc1, vloc2);
                        }
                    }
                }
            }
        }

        int required = tenantNames.length * bridgeNames.length *
            routerNames.length * terminalNames.length * ifNames.length;
        assertEquals(required, set.size());
    }

    /**
     * Ensure that {@link VNodeLocation} is serializable.
     */
    @Test
    public void testSerialize() {
        String[] tenantNames = {null, "tenant_1", "tenant_2"};
        String[] bridgeNames = {null, "bridge_1", "bridge_2"};
        String[] routerNames = {null, "router_1", "router_2"};
        String[] terminalNames = {null, "term_1", "term_2"};
        String[] ifNames = {null, "if1", "if2"};

        for (String tname: tenantNames) {
            for (String bname: bridgeNames) {
                for (String rname: routerNames) {
                    for (String tmname: terminalNames) {
                        for (String iname: ifNames) {
                            VNodeLocation vloc = createVNodeLocation(
                                tname, bname, rname, tmname, iname);
                            serializeTest(vloc);
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VNodeLocation} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        String[] tenantNames = {null, "tenant_1", "tenant_2"};
        String[] bridgeNames = {null, "bridge_1", "bridge_2"};
        String[] routerNames = {null, "router_1", "router_2"};
        String[] terminalNames = {null, "term_1", "term_2"};
        String[] ifNames = {null, "if1", "if2"};

        for (String tname: tenantNames) {
            for (String bname: bridgeNames) {
                for (String rname: routerNames) {
                    for (String tmname: terminalNames) {
                        for (String iname: ifNames) {
                            VNodeLocation vloc = createVNodeLocation(
                                tname, bname, rname, tmname, iname);
                            jaxbTest(vloc, VNodeLocation.class, XML_ROOT);
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link VNodeLocation} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        String[] tenantNames = {null, "tenant_1", "tenant_2"};
        String[] bridgeNames = {null, "bridge_1", "bridge_2"};
        String[] routerNames = {null, "router_1", "router_2"};
        String[] terminalNames = {null, "term_1", "term_2"};
        String[] ifNames = {null, "if1", "if2"};

        for (String tname: tenantNames) {
            for (String bname: bridgeNames) {
                for (String rname: routerNames) {
                    for (String tmname: terminalNames) {
                        for (String iname: ifNames) {
                            VNodeLocation vloc = createVNodeLocation(
                                tname, bname, rname, tmname, iname);
                            jsonTest(vloc, VNodeLocation.class);
                        }
                    }
                }
            }
        }
    }

    /**
     * Construct an instance of {@link VNodeLocation} using JAXB.
     *
     * @param tname   The name of the VTN.
     * @param bname   The name of the vBridge.
     * @param rname   The name of the vRouter.
     * @param tmname  The name of the vTerminal.
     * @param iname   The name of the virtual interface.
     * @return  A {@link VNodeLocation} instance.
     */
    private VNodeLocation createVNodeLocation(String tname, String bname,
                                              String rname, String tmname,
                                              String iname) {
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT);
        if (tname != null) {
            builder.append(" tenant=\"").append(tname).append("\"");
        }
        if (bname != null) {
            builder.append(" bridge=\"").append(bname).append("\"");
        }
        if (rname != null) {
            builder.append(" router=\"").append(rname).append("\"");
        }
        if (tmname != null) {
            builder.append(" terminal=\"").append(tmname).append("\"");
        }
        if (iname != null) {
            builder.append(" interface=\"").append(iname).append("\"");
        }

        String xml = builder.append(" />").toString();
        Unmarshaller um = createUnmarshaller(VNodeLocation.class);
        VNodeLocation vloc = null;
        try {
            vloc = unmarshal(um, xml, VNodeLocation.class);
            assertEquals(tname, vloc.getTenantName());
            assertEquals(bname, vloc.getBridgeName());
            assertEquals(rname, vloc.getRouterName());
            assertEquals(tmname, vloc.getTerminalName());
            assertEquals(iname, vloc.getInterfaceName());
        } catch (Exception e) {
            unexpected(e);
        }

        return vloc;
    }
}
