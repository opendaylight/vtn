/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;
import org.opendaylight.vtn.manager.XmlDataType;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link Inet4Match}.
 */
public class Inet4MatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link Inet4Match} class.
     */
    private static final String  XML_ROOT = "inet4match";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link Inet4Match} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        ArrayList<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlAttributeType(name, "srcsuffix", Short.class).add(parent),
            new XmlAttributeType(name, "dstsuffix", Short.class).add(parent),
            new XmlAttributeType(name, "protocol", Short.class).add(parent),
            new XmlAttributeType(name, "dscp", Byte.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Short[] srcsuffs = {
            null, Short.valueOf((short)1), Short.valueOf((short)31),
        };
        Short[] dstsuffs = {
            null, Short.valueOf((short)2), Short.valueOf((short)31),
        };
        Short[] protocols = {
            null, Short.valueOf((short)1), Short.valueOf((short)17),
        };
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)63),
        };

        for (InetAddress src: createInet4Addresses(3, true)) {
            for (InetAddress dst: createInet4Addresses(3, true)) {
                for (Short srcsuff: srcsuffs) {
                    for (Short dstsuff: dstsuffs) {
                        for (Short proto: protocols) {
                            for (Byte dscp: dscps) {
                                Inet4Match im =
                                    new Inet4Match(src, srcsuff, dst, dstsuff,
                                                   proto, dscp);
                                assertEquals(src, im.getSourceAddress());
                                assertEquals(srcsuff, im.getSourceSuffix());
                                assertEquals(dst, im.getDestinationAddress());
                                assertEquals(dstsuff,
                                             im.getDestinationSuffix());
                                assertEquals(proto, im.getProtocol());
                                assertEquals(dscp, im.getDscp());
                                assertEquals(null, im.getValidationStatus());
                                assertEquals(Inet4Address.class,
                                             im.getAddressClass());
                            }
                        }
                    }
                }
            }
        }

        // Specifying unexpected address.
        InetAddress v4 = null;
        InetAddress v6 = null;
        try {
            v4 = InetAddress.getByName("127.0.0.1");
            v6 = InetAddress.getByName("::1");
        } catch (Exception e) {
            unexpected(e);
        }

        try {
            new Inet4Match(v6, null, v4, null, null, null);
            unexpected();
        } catch (IllegalArgumentException e) {
        }

        try {
            new Inet4Match(v4, null, v6, null, null, null);
            unexpected();
        } catch (IllegalArgumentException e) {
        }
    }

    /**
     * Test case for {@link Inet4Match#equals(Object)} and
     * {@link Inet4Match#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<InetAddress> srcs = createInet4Addresses(3, true);
        List<InetAddress> dsts = createInet4Addresses(3, true);
        Short[] srcsuffs = {
            null, Short.valueOf((short)1), Short.valueOf((short)31),
        };
        Short[] dstsuffs = {
            null, Short.valueOf((short)2), Short.valueOf((short)31),
        };
        Short[] protocols = {
            null, Short.valueOf((short)1), Short.valueOf((short)17),
        };
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)63),
        };

        for (InetAddress src: srcs) {
            for (InetAddress dst: dsts) {
                for (Short srcsuff: srcsuffs) {
                    for (Short dstsuff: dstsuffs) {
                        for (Short proto: protocols) {
                            for (Byte dscp: dscps) {
                                Inet4Match im1 =
                                    new Inet4Match(src, srcsuff, dst, dstsuff,
                                                   proto, dscp);
                                Inet4Match im2 =
                                    new Inet4Match(copy(src), copy(srcsuff),
                                                   copy(dst), copy(dstsuff),
                                                   copy(proto), copy(dscp));
                                testEquals(set, im1, im2);
                            }
                        }
                    }
                }
            }
        }

        int required = srcs.size() * dsts.size() * srcsuffs.length *
            dstsuffs.length * protocols.length * dscps.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link Inet4Match#toString()}.
     */
    @Test
    public void testToString() {
        Short[] srcsuffs = {
            null, Short.valueOf((short)1), Short.valueOf((short)31),
        };
        Short[] dstsuffs = {
            null, Short.valueOf((short)2), Short.valueOf((short)31),
        };
        Short[] protocols = {
            null, Short.valueOf((short)1), Short.valueOf((short)17),
        };
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)63),
        };

        String prefix = "Inet4Match[";
        String suffix = "]";
        for (InetAddress src: createInet4Addresses(3, true)) {
            for (InetAddress dst: createInet4Addresses(3, true)) {
                for (Short srcsuff: srcsuffs) {
                    for (Short dstsuff: dstsuffs) {
                        for (Short proto: protocols) {
                            for (Byte dscp: dscps) {
                                Inet4Match im =
                                    new Inet4Match(src, srcsuff, dst, dstsuff,
                                                   proto, dscp);
                                String s = (src == null) ? null
                                    : "src=" + src.getHostAddress();
                                String ss = (srcsuff == null) ? null
                                    : "srcsuff=" + srcsuff;
                                String d = (dst == null) ? null
                                    : "dst=" + dst.getHostAddress();
                                String ds = (dstsuff == null) ? null
                                    : "dstsuff=" + dstsuff;
                                String p = (proto == null) ? null
                                    : "proto=" + proto;
                                String c = (dscp == null) ? null
                                    : "dscp=" + dscp;

                                String required = joinStrings(
                                    prefix, suffix, ",", s, ss, d, ds, p, c);
                                assertEquals(required, im.toString());
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link Inet4Match} is serializable.
     */
    @Test
    public void testSerialize() {
        Short[] srcsuffs = {
            null, Short.valueOf((short)1), Short.valueOf((short)31),
        };
        Short[] dstsuffs = {
            null, Short.valueOf((short)2), Short.valueOf((short)31),
        };
        Short[] protocols = {
            null, Short.valueOf((short)1), Short.valueOf((short)17),
        };
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)63),
        };

        for (InetAddress src: createInet4Addresses(3, true)) {
            for (InetAddress dst: createInet4Addresses(3, true)) {
                for (Short srcsuff: srcsuffs) {
                    for (Short dstsuff: dstsuffs) {
                        for (Short proto: protocols) {
                            for (Byte dscp: dscps) {
                                Inet4Match im =
                                    new Inet4Match(src, srcsuff, dst, dstsuff,
                                                   proto, dscp);
                                serializeTest(im);
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link Inet4Match} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        Short[] srcsuffs = {
            null, Short.valueOf((short)1), Short.valueOf((short)31),
        };
        Short[] dstsuffs = {
            null, Short.valueOf((short)2), Short.valueOf((short)31),
        };
        Short[] protocols = {
            null, Short.valueOf((short)1), Short.valueOf((short)17),
        };
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)63),
        };

        for (InetAddress src: createInet4Addresses(3, true)) {
            for (InetAddress dst: createInet4Addresses(3, true)) {
                for (Short srcsuff: srcsuffs) {
                    for (Short dstsuff: dstsuffs) {
                        for (Short proto: protocols) {
                            for (Byte dscp: dscps) {
                                Inet4Match im =
                                    new Inet4Match(src, srcsuff, dst, dstsuff,
                                                   proto, dscp);
                                Inet4Match im1 = (Inet4Match)
                                    jaxbTest(im, Inet4Match.class, XML_ROOT);
                                assertEquals(null, im1.getValidationStatus());
                            }
                        }
                    }
                }
            }
        }

        // Specifying invalid IP address.
        String[] badAddrs = {
            "", "192.168.100.256", "::1", "bad_address",
        };
        Unmarshaller um = createUnmarshaller(Inet4Match.class);
        for (String addr: badAddrs) {
            for (String attr: new String[]{"src", "dst"}) {
                StringBuilder builder = new StringBuilder(XML_DECLARATION);
                builder.append('<').append(XML_ROOT).append(' ').append(attr).
                    append("=\"").append(addr).append("\" />");
                String xml = builder.toString();
                try {
                    Inet4Match im = unmarshal(um, xml, Inet4Match.class);
                    Status st = im.getValidationStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }

        // Ensure that broken values in XML can be detected.
        List<XmlDataType> dlist = getXmlDataTypes(XML_ROOT);
        jaxbErrorTest(um, Inet4Match.class, dlist);
        jaxbErrorTest(createUnmarshaller(InetMatch.class), Inet4Match.class,
                      dlist);
    }

    /**
     * Ensure that {@link Inet4Match} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        Short[] srcsuffs = {
            null, Short.valueOf((short)1), Short.valueOf((short)31),
        };
        Short[] dstsuffs = {
            null, Short.valueOf((short)2), Short.valueOf((short)31),
        };
        Short[] protocols = {
            null, Short.valueOf((short)1), Short.valueOf((short)17),
        };
        Byte[] dscps = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)63),
        };

        for (InetAddress src: createInet4Addresses(3, true)) {
            for (InetAddress dst: createInet4Addresses(3, true)) {
                for (Short srcsuff: srcsuffs) {
                    for (Short dstsuff: dstsuffs) {
                        for (Short proto: protocols) {
                            for (Byte dscp: dscps) {
                                Inet4Match im =
                                    new Inet4Match(src, srcsuff, dst, dstsuff,
                                                   proto, dscp);
                                Inet4Match im1 = jsonTest(im, Inet4Match.class);
                                assertEquals(null, im1.getValidationStatus());
                            }
                        }
                    }
                }
            }
        }

        // Specifying invalid IP address.
        String[] badAddrs = {
            "", "192.168.100.256", "::1", "bad_address",
        };
        ObjectMapper mapper = getJsonObjectMapper();
        for (String addr: badAddrs) {
            for (String attr: new String[]{"src", "dst"}) {
                try {
                    JSONObject json = new JSONObject();
                    json.put(attr, addr);
                    Inet4Match im =
                        mapper.readValue(json.toString(), Inet4Match.class);
                    Status st = im.getValidationStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }
    }
}
