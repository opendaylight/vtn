/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import com.fasterxml.jackson.databind.ObjectMapper;

import org.codehaus.jettison.json.JSONObject;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlAttributeType;
import org.opendaylight.vtn.manager.XmlDataType;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link EthernetMatch}.
 */
public class EthernetMatchTest extends TestBase {
    /**
     * Root XML element name associated with {@link EthernetMatch} class.
     */
    private static final String  XML_ROOT = "ethermatch";

    /**
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link EthernetMatch} instance.
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
            new XmlAttributeType(name, "type", Integer.class).add(parent),
            new XmlAttributeType(name, "vlan", Short.class).add(parent),
            new XmlAttributeType(name, "vlanpri", Byte.class).add(parent));
        return dlist;
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        Integer[] types = {
            null, Integer.valueOf(0x800), Integer.valueOf(0x86dd),
        };
        Short[] vlans = {
            null, Short.valueOf((short)0), Short.valueOf((short)1),
            Short.valueOf((short)4095),
        };
        Byte[] priorities = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)7),
        };

        for (EthernetAddress src: createEthernetAddresses(3, true)) {
            EtherAddress srcAddr = EtherAddress.create(src);
            for (EthernetAddress dst: createEthernetAddresses(3, true)) {
                EtherAddress dstAddr = EtherAddress.create(dst);
                for (Integer type: types) {
                    for (Short vlan: vlans) {
                        for (Byte pri: priorities) {
                            EthernetMatch em =
                                new EthernetMatch(src, dst, type, vlan, pri);
                            EthernetAddress esrc = em.getSourceAddress();
                            EthernetAddress edst = em.getDestinationAddress();
                            assertEquals(src, esrc);
                            if (src != null) {
                                assertNotSame(src, esrc);
                            }
                            assertEquals(dst, edst);
                            if (dst != null) {
                                assertNotSame(dst, edst);
                            }
                            assertEquals(type, em.getType());
                            assertEquals(vlan, em.getVlan());
                            assertEquals(pri, em.getVlanPriority());
                            assertEquals(srcAddr, em.getSourceEtherAddress());
                            assertEquals(dstAddr,
                                         em.getDestinationEtherAddress());
                            assertEquals(null, em.getValidationStatus());

                            EthernetMatch em1 = new EthernetMatch(
                                srcAddr, dstAddr, type, vlan, pri);
                            assertEquals(src, em.getSourceAddress());
                            assertEquals(dst, em.getDestinationAddress());
                            assertEquals(type, em.getType());
                            assertEquals(vlan, em.getVlan());
                            assertEquals(pri, em.getVlanPriority());
                            assertEquals(srcAddr, em.getSourceEtherAddress());
                            assertEquals(dstAddr,
                                         em.getDestinationEtherAddress());
                            assertEquals(null, em.getValidationStatus());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link EthernetMatch#equals(Object)} and
     * {@link EthernetMatch#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<EthernetAddress> srcs = createEthernetAddresses(3, true);
        List<EthernetAddress> dsts = createEthernetAddresses(3, true);
        Integer[] types = {
            null, Integer.valueOf(0x800), Integer.valueOf(0x86dd),
        };
        Short[] vlans = {
            null, Short.valueOf((short)0), Short.valueOf((short)1),
            Short.valueOf((short)4095),
        };
        Byte[] priorities = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)7),
        };

        for (EthernetAddress src: srcs) {
            for (EthernetAddress dst: dsts) {
                for (Integer type: types) {
                    for (Short vlan: vlans) {
                        for (Byte pri: priorities) {
                            EthernetMatch em1 =
                                new EthernetMatch(src, dst, type, vlan, pri);

                            EthernetAddress src1 = (EthernetAddress)copy(src);
                            EthernetAddress dst1 = (EthernetAddress)copy(dst);
                            EthernetMatch em2 =
                                new EthernetMatch(src1, dst1, copy(type),
                                                  copy(vlan), copy(pri));
                            testEquals(set, em1, em2);
                        }
                    }
                }
            }
        }

        int required = srcs.size() * dsts.size() * types.length *
            vlans.length * priorities.length;
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link EthernetMatch#toString()}.
     */
    @Test
    public void testToString() {
        Integer[] types = {
            null, Integer.valueOf(0x800), Integer.valueOf(0x86dd),
        };
        Short[] vlans = {
            null, Short.valueOf((short)0), Short.valueOf((short)1),
            Short.valueOf((short)4095),
        };
        Byte[] priorities = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)7),
        };

        String prefix = "EthernetMatch[";
        String suffix = "]";
        for (EthernetAddress src: createEthernetAddresses(3, true)) {
            for (EthernetAddress dst: createEthernetAddresses(3, true)) {
                for (Integer type: types) {
                    for (Short vlan: vlans) {
                        for (Byte pri: priorities) {
                            EthernetMatch em =
                                new EthernetMatch(src, dst, type, vlan, pri);
                            String s = (src == null) ? null
                                : "src=" + src.getMacAddress();
                            String d = (dst == null) ? null
                                : "dst=" + dst.getMacAddress();
                            String t = (type == null) ? null
                                : "type=0x" +
                                Integer.toHexString(type.intValue());
                            String v = (vlan == null) ? null
                                : "vlan=" + vlan;
                            String p = (pri == null) ? null
                                : "pri=" + pri;

                            String required = joinStrings(prefix, suffix, ",",
                                                          s, d, t, v, p);
                            assertEquals(required, em.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link EthernetMatch} is serializable.
     */
    @Test
    public void testSerialize() {
        Integer[] types = {
            null, Integer.valueOf(0x800), Integer.valueOf(0x86dd),
        };
        Short[] vlans = {
            null, Short.valueOf((short)0), Short.valueOf((short)1),
            Short.valueOf((short)4095),
        };
        Byte[] priorities = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)7),
        };

        for (EthernetAddress src: createEthernetAddresses(3, true)) {
            for (EthernetAddress dst: createEthernetAddresses(3, true)) {
                for (Integer type: types) {
                    for (Short vlan: vlans) {
                        for (Byte pri: priorities) {
                            EthernetMatch em =
                                new EthernetMatch(src, dst, type, vlan, pri);
                            serializeTest(em);
                        }
                    }
                }
            }
        }
    }

    /**
     * Ensure that {@link EthernetMatch} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        Integer[] types = {
            null, Integer.valueOf(0x800), Integer.valueOf(0x86dd),
        };
        Short[] vlans = {
            null, Short.valueOf((short)0), Short.valueOf((short)1),
            Short.valueOf((short)4095),
        };
        Byte[] priorities = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)7),
        };

        for (EthernetAddress src: createEthernetAddresses(3, true)) {
            for (EthernetAddress dst: createEthernetAddresses(3, true)) {
                for (Integer type: types) {
                    for (Short vlan: vlans) {
                        for (Byte pri: priorities) {
                            EthernetMatch em =
                                new EthernetMatch(src, dst, type, vlan, pri);
                            EthernetMatch em1 = (EthernetMatch)
                                jaxbTest(em, EthernetMatch.class, XML_ROOT);
                            assertEquals(null, em1.getValidationStatus());
                        }
                    }
                }
            }
        }

        // Specifying invalid MAC address.
        String[] badAddrs = {
            "", "aa:bb:cc:dd:ee:ff:11", "00:11", "bad_address",
        };

        Unmarshaller um = createUnmarshaller(EthernetMatch.class);
        try {
            for (String addr: badAddrs) {
                for (String attr: new String[]{"src", "dst"}) {
                    StringBuilder builder = new StringBuilder(XML_DECLARATION);
                    String xml = builder.append('<').append(XML_ROOT).
                        append(' ').append(attr).append("=\"").append(addr).
                        append("\" />").toString();
                    EthernetMatch em = unmarshal(um, xml, EthernetMatch.class);
                    Status st = em.getValidationStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(EthernetMatch.class, getXmlDataTypes(XML_ROOT));
    }

    /**
     * Ensure that {@link EthernetMatch} is mapped to JSON object.
     */
    @Test
    public void testJSON() {
        Integer[] types = {
            null, Integer.valueOf(0x800), Integer.valueOf(0x86dd),
        };
        Short[] vlans = {
            null, Short.valueOf((short)0), Short.valueOf((short)1),
            Short.valueOf((short)4095),
        };
        Byte[] priorities = {
            null, Byte.valueOf((byte)0), Byte.valueOf((byte)7),
        };

        for (EthernetAddress src: createEthernetAddresses(3, true)) {
            for (EthernetAddress dst: createEthernetAddresses(3, true)) {
                for (Integer type: types) {
                    for (Short vlan: vlans) {
                        for (Byte pri: priorities) {
                            EthernetMatch em =
                                new EthernetMatch(src, dst, type, vlan, pri);
                            EthernetMatch em1 =
                                jsonTest(em, EthernetMatch.class);
                            assertEquals(null, em1.getValidationStatus());
                        }
                    }
                }
            }
        }

        // Specifying invalid MAC address.
        ObjectMapper mapper = getJsonObjectMapper();
        String[] badAddrs = {
            "", "aa:bb:cc:dd:ee:ff:11", "00:11", "bad_address",
        };

        try {
            for (String addr: badAddrs) {
                for (String attr: new String[]{"src", "dst"}) {
                    JSONObject json = new JSONObject();
                    json.put(attr, addr);
                    EthernetMatch em =
                        mapper.readValue(json.toString(), EthernetMatch.class);
                    Status st = em.getValidationStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                }
            }
        } catch (Exception e) {
            unexpected(e);
        }
    }
}
