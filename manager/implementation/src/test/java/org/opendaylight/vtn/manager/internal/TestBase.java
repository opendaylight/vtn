/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import static org.opendaylight.vtn.manager.internal.util.packet.EtherHeader.VLAN_NONE;
import static org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag.MISSING_ELEMENT;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.UnmarshalException;
import javax.xml.bind.Unmarshaller;
import javax.xml.bind.annotation.XmlRootElement;

import org.junit.After;
import org.junit.Assert;

import com.google.common.base.Optional;
import com.google.common.util.concurrent.CheckedFuture;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Ethernet;
import org.opendaylight.vtn.manager.packet.IEEE8021Q;
import org.opendaylight.vtn.manager.packet.IPv4;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.EtherTypes;
import org.opendaylight.vtn.manager.util.Ip4Network;

import org.opendaylight.vtn.manager.internal.util.concurrent.VTNFuture;
import org.opendaylight.vtn.manager.internal.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;

import org.opendaylight.controller.md.sal.common.api.data.ReadFailedException;
import org.opendaylight.controller.md.sal.common.api.data.TransactionCommitFailedException;

import org.opendaylight.yangtools.yang.binding.DataObject;
import org.opendaylight.yangtools.yang.common.RpcError.ErrorType;
import org.opendaylight.yangtools.yang.common.RpcError;
import org.opendaylight.yangtools.yang.common.RpcResult;
import org.opendaylight.yangtools.yang.common.RpcResultBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.filter.rev150907.vtn.flow.filter.list.VtnFlowFilter;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.AllowedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.config.DeniedHosts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.info.MacMapConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticEdgePorts;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology.StaticSwitchLinks;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static._switch.links.StaticSwitchLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.topology._static.rev150801.vtn._static.topology._static.edge.ports.StaticEdgePort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;

/**
 * Abstract base class for JUnit tests.
 */
public abstract class TestBase extends Assert {
    /**
     * XML declaration.
     */
    public static final String  XML_DECLARATION =
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>";

    /**
     * A relative path to the base configuration file directory.
     */
    public static final String CONFIG_DIR_BASE = "configuration";

    /**
     * A path to the VTN configuration file directory relative to
     * {@link #CONFIG_DIR_BASE}.
     */
    private static final String  VTN_CONFIG_DIR = "startup/vtn";

    /**
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    protected static void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
    }

    /**
     * Throw an error which indicates the test code should never reach here.
     */
    protected static void unexpected() {
        fail("Should never reach here.");
    }

    /**
     * Create a copy of the specified string.
     *
     * @param str  A string to be copied.
     * @return     A copied string.
     */
    protected static String copy(String str) {
        if (str != null) {
            str = new String(str);
        }
        return str;
    }

    /**
     * Create a copy of the specified integer.
     *
     * @param i   An integer to be copied.
     * @return  A copied integer.
     */
    protected static Integer copy(Integer i) {
        if (i != null) {
            i = new Integer(i.intValue());
        }
        return i;
    }

    /**
     * Create a copy of the specified {@link Long} object.
     *
     * @param num  An {@link Long} object to be copied.
     * @return     A copied boolean object.
     */
    protected static Long copy(Long num) {
        if (num != null) {
            num = new Long(num.longValue());
        }
        return num;
    }

    /**
     * Create a deep copy of the specified {@link VnodeName}.
     *
     * @param vname  A {@link VnodeName} instance to be copied.
     * @return  A copied {@link VnodeName} instance.
     */
    protected static VnodeName copy(VnodeName vname) {
        VnodeName ret = vname;
        if (ret != null) {
            ret = new VnodeName(copy(vname.getValue()));
        }
        return ret;
    }

    /**
     * Create a deep copy of the specified {@link MacVlan}.
     *
     * @param mv  A {@link MacVlan} instance to be copied.
     * @return  A copied {@link MacVlan} instance.
     */
    protected static MacVlan copy(MacVlan mv) {
        MacVlan ret = mv;
        if (ret != null) {
            ret = new MacVlan(mv.getEncodedValue());
        }
        return ret;
    }

    /**
     * Create a deep copy of the specified {@link Packet} instance.
     *
     * @param src  The source {@link Packet} instance.
     * @param dst  The destination {@link Packet} instance.
     * @param <T>  Type of the packet.
     * @return  {@code dst}.
     */
    protected static <T extends Packet> T copy(T src, T dst) {
        if (src != null) {
            try {
                byte[] raw = src.serialize();
                int nbits = raw.length * Byte.SIZE;
                dst.deserialize(raw, 0, nbits);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return dst;
    }

    /**
     * Create a list of {@link EtherAddress} and a {@code null}.
     *
     * @return A list of {@link EtherAddress}.
     */
    protected static List<EtherAddress> createEtherAddresses() {
        return createEtherAddresses(true);
    }

    /**
     * Create a list of {@link EtherAddress}.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link EtherAddress}.
     */
    protected static List<EtherAddress> createEtherAddresses(boolean setNull) {
        List<EtherAddress> list = new ArrayList<>();
        if (setNull) {
            list.add(null);
        }

        Collections.addAll(
            list,
            new EtherAddress(0x000000000001L),
            new EtherAddress(0x123456789abcL),
            new EtherAddress(0xfedcba987654L));
        return list;
    }

    /**
     * Create an Ethernet frame.
     *
     * @param src      Source MAC address.
     * @param dst      Destination MAC address.
     * @param type     Ethernet type.
     * @param vid      A VLAN ID.
     * @param pcp      VLAN priority.
     * @param payload  A {@link Packet} instance to be set as payload.
     * @return  An {@link Ethernet} instance.
     */
    protected static Ethernet createEthernet(
        EtherAddress src, EtherAddress dst, int type, int vid, byte pcp,
        Packet payload) {
        Ethernet pkt = new Ethernet().
            setSourceMACAddress(src.getBytes()).
            setDestinationMACAddress(dst.getBytes());
        if (vid == VLAN_NONE) {
            pkt.setEtherType((short)type).setPayload(payload);
        } else {
            IEEE8021Q tag = new IEEE8021Q();
            tag.setCfi((byte)0).setPcp(pcp).setVid((short)vid).
                setEtherType((short)type).setPayload(payload);
            pkt.setEtherType(EtherTypes.VLAN.shortValue()).
                setPayload(tag);
        }

        return pkt;
    }

    /**
     * Create an Ethernet frame.
     *
     * @param src   Source MAC address.
     * @param dst   Destination MAC address.
     * @param type  Ethernet type.
     * @param vid   A VLAN ID.
     * @param pcp   VLAN priority
     * @param raw   A byte array to be set as payload.
     * @return  An {@link Ethernet} instance.
     */
    protected static Ethernet createEthernet(
        EtherAddress src, EtherAddress dst, int type, int vid, byte pcp,
        byte[] raw) {
        Ethernet pkt = new Ethernet().
            setSourceMACAddress(src.getBytes()).
            setDestinationMACAddress(dst.getBytes());
        if (vid == VLAN_NONE) {
            pkt.setEtherType((short)type).setRawPayload(raw);
        } else {
            IEEE8021Q tag = new IEEE8021Q();
            tag.setCfi((byte)0).setPcp(pcp).setVid((short)vid).
                setEtherType((short)type).setRawPayload(raw);
            pkt.setEtherType(EtherTypes.VLAN.shortValue()).
                setPayload(tag);
        }

        return pkt;
    }

    /**
     * Create an untagged Ethernet frame that contains the given IPv4 packet.
     *
     * <p>
     *   Fixed values are used for the source and destination MAC address.
     * </p>
     *
     * @param ipv4  An {@link IPv4} to be configured as payload.
     * @return  An {@link Ethernet} instance.
     */
    protected static Ethernet createEthernet(IPv4 ipv4) {
        EtherAddress src = new EtherAddress(0x001122334455L);
        EtherAddress dst = new EtherAddress(0xa0b0c0ddeeffL);
        return createEthernet(src, dst, EtherTypes.IPV4.intValue(),
                              VLAN_NONE, (byte)0, ipv4);
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * @param src    Source IP address.
     * @param dst    Destination IP address.
     * @param proto  IP protocol number.
     * @param dscp   DSCP field value.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(Ip4Network src, Ip4Network dst,
                                     short proto, byte dscp) {
        IPv4 pkt = new IPv4();
        return pkt.setSourceAddress(src).setDestinationAddress(dst).
            setProtocol((byte)proto).setDiffServ(dscp).
            setTtl((byte)64);
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * @param src      The source IP address.
     * @param dst      The destination IP address.
     * @param proto    IP protocol number.
     * @param dscp     DSCP field value.
     * @param payload  A {@link Packet} instance to be configured as payload.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(Ip4Network src, Ip4Network dst,
                                     short proto, byte dscp, Packet payload) {
        IPv4 pkt = createIPv4(src, dst, proto, dscp);
        pkt.setPayload(payload);

        return pkt;
    }

    /**
     * Create an {@link IPv4} instance.
     *
     * <p>
     *   Fixed values are used for the source and destination IP address,
     *   and DSCP field.
     * </p>
     *
     * @param proto    IP protocol number.
     * @param payload  A {@link Packet} instance to be configured as payload.
     * @return  An {@link IPv4} instance.
     */
    protected static IPv4 createIPv4(short proto, Packet payload) {
        Ip4Network src = new Ip4Network("10.1.2.30");
        Ip4Network dst = new Ip4Network("127.0.0.1");
        return createIPv4(src, dst, proto, (byte)0, payload);
    }


    /**
     * create a {@link Ethernet} object of IPv4 Packet.
     *
     * @param src       A source MAC address
     * @param dst       A destination MAC address
     * @param sender    A sender address
     * @param target    A target address
     * @param vid       A VLAN ID. VLAN tag is added only if {@code vid} is
     *                  greater than zero.
     * @return  A {@link Ethernet} object.
     */
    protected Ethernet createIPv4Packet(
        EtherAddress src, EtherAddress dst, Ip4Network sender,
        Ip4Network target, int vid) {
        IPv4 ip = new IPv4();
        ip.setVersion((byte)4).
            setIdentification((short)5).
            setDiffServ((byte)0).
            setECN((byte)0).
            setTotalLength((short)84).
            setFlags((byte)2).
            setFragmentOffset((short)0).
            setTtl((byte)64).
            setDestinationAddress(target).
            setSourceAddress(sender);

        Ethernet eth = new Ethernet().
            setSourceMACAddress(src.getBytes()).
            setDestinationMACAddress(dst.getBytes());

        if (vid >= 0) {
            eth.setEtherType(EtherTypes.VLAN.shortValue());

            IEEE8021Q vlantag = new IEEE8021Q();
            vlantag.setCfi((byte)0x0).setPcp((byte)0x0).setVid((short)vid).
                setEtherType(EtherTypes.IPV4.shortValue());
            eth.setPayload(vlantag);

            vlantag.setPayload(ip);

        } else {
            eth.setEtherType(EtherTypes.IPV4.shortValue()).setPayload(ip);
        }
        return eth;
    }

    /**
     * Join the separated strings with inserting a separator.
     *
     * @param prefix     A string to be prepended to the string.
     * @param suffix     A string to be appended to the string.
     * @param separator  A separator string.
     * @param args       An array of objects. Note that {@code null} in this
     *                   array is always ignored.
     */
    protected static String joinStrings(String prefix, String suffix,
            String separator, Object... args) {
        StringBuilder builder = new StringBuilder();
        if (prefix != null) {
            builder.append(prefix);
        }

        boolean first = true;
        for (Object o : args) {
            if (o != null) {
                if (first) {
                    first = false;
                } else {
                    builder.append(separator);
                }
                builder.append(o.toString());
            }
        }

        if (suffix != null) {
            builder.append(suffix);
        }

        return builder.toString();
    }

    /**
     * Ensure that the given two objects are identical.
     *
     * @param set  A set of tested objects.
     * @param o1   An object to be tested.
     * @param o2   An object to be tested.
     */
    protected static void testEquals(Set<Object> set, Object o1, Object o2) {
        assertEquals(o1, o2);
        assertEquals(o2, o1);
        assertEquals(o1, o1);
        assertEquals(o2, o2);
        assertEquals(o1.hashCode(), o2.hashCode());
        assertFalse(o1.equals(null));
        assertFalse(o1.equals(new Object()));
        assertFalse(o1.equals("string"));
        assertFalse(o1.equals(set));

        for (Object o : set) {
            assertFalse("o1=" + o1 + ", o=" + o, o1.equals(o));
            assertFalse(o.equals(o1));
        }

        assertTrue(set.add(o1));
        assertFalse(set.add(o1));
        assertFalse(set.add(o2));
    }

    /**
     * Ensure that the given object is mapped to XML root element.
     *
     * @param o         An object to be tested.
     * @param cls       The type of the given object.
     * @param rootName  The name of expected root element.
     * @param <T>       The type of the given object.
     * @return  Deserialized object.
     */
    protected static <T> T jaxbTest(T o, Class<T> cls, String rootName) {
        return jaxbTest(o, cls, createMarshaller(cls), createUnmarshaller(cls),
                        rootName);
    }

    /**
     * Ensure that the given object is mapped to XML root element.
     *
     * @param o         An object to be tested.
     * @param cls       The type of the given object.
     * @param m         XML marshaller.
     * @param um        XML unmarshaller.
     * @param rootName  The name of expected root element.
     * @param <T>       The type of the given object.
     * @return  Deserialized object.
     */
    protected static <T> T jaxbTest(T o, Class<T> cls, Marshaller m,
                                    Unmarshaller um, String rootName) {
        // Ensure that the class of the given class has XmlRootElement
        // annotation.
        XmlRootElement xmlRoot = cls.getAnnotation(XmlRootElement.class);
        assertNotNull(xmlRoot);
        assertEquals(rootName, xmlRoot.name());

        // Marshal the given object into XML.
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        try {
            m.marshal(o, out);
        } catch (Exception e) {
            unexpected(e);
        }

        byte[] bytes = out.toByteArray();
        assertTrue(bytes.length != 0);

        // Construct a new Java object from XML.
        ByteArrayInputStream in = new ByteArrayInputStream(bytes);
        Object newobj = null;
        try {
            newobj = um.unmarshal(in);
        } catch (Exception e) {
            unexpected(e);
        }

        assertNotSame(o, newobj);
        assertEquals(o, newobj);

        assertTrue(cls.isInstance(newobj));
        return cls.cast(newobj);
    }

    /**
     * Create JAXB marshaller for the given JAXB class.
     *
     * @param cls  A class mapped to XML root element.
     * @return  An JAXB marshaller.
     */
    protected static Marshaller createMarshaller(Class<?> cls) {
        try {
            JAXBContext jc = JAXBContext.newInstance(cls);
            Marshaller m = jc.createMarshaller();
            m.setEventHandler(new TestXmlEventHandler());
            return m;
        } catch (Exception e) {
            unexpected(e);
            return null;
        }
    }

    /**
     * Create JAXB unmarshaller for the given JAXB class.
     *
     * @param cls  A class mapped to XML root element.
     * @return  An JAXB unmarshaller.
     */
    protected static Unmarshaller createUnmarshaller(Class<?> cls) {
        try {
            JAXBContext jc = JAXBContext.newInstance(cls);
            Unmarshaller um = jc.createUnmarshaller();
            um.setEventHandler(new TestXmlEventHandler());
            return um;
        } catch (Exception e) {
            unexpected(e);
            return null;
        }
    }

    /**
     * Marshal the given instance into XML.
     *
     * @param m         A {@link Marshaller} instance.
     * @param o         An object to be tested.
     * @param cls       A class which specifies the type of the given object.
     * @param rootName  The name of expected root element.
     * @return  A string which contains converted XML.
     * @throws JAXBException  Failed to marshal.
     */
    protected static String marshal(Marshaller m, Object o, Class<?> cls,
                                    String rootName) throws JAXBException {
        // Ensure that the class of the given class has XmlRootElement
        // annotation.
        XmlRootElement xmlRoot = cls.getAnnotation(XmlRootElement.class);
        assertNotNull(xmlRoot);
        assertEquals(rootName, xmlRoot.name());

        // Marshal the given object into XML.
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        m.marshal(o, out);

        String xml = out.toString();
        assertFalse(xml.isEmpty());

        return xml;
    }

    /**
     * Unmarshal the given XML using the given unmarshaller.
     *
     * @param um   An {@link Unmarshaller} instance.
     * @param xml  A XML text.
     * @param cls  A class which indicates the type of object.
     * @param <T>  The type of the object to be deserialized.
     * @return  The deserialized object.
     * @throws JAXBException  Failed to unmarshal.
     */
    protected static <T> T unmarshal(Unmarshaller um, String xml,
                                     Class<T> cls) throws JAXBException {
        ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
        Object o = um.unmarshal(in);
        assertTrue(cls.isInstance(o));
        return cls.cast(o);
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param type    A class which indicates the type of object.
     * @param dtypes  An array of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Class<T> type,
                                            XmlDataType ... dtypes) {
        jaxbErrorTest(createUnmarshaller(type), type, dtypes);
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param type    A class which indicates the type of object.
     * @param dtypes  A list of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Class<T> type,
                                            List<XmlDataType> dtypes) {
        jaxbErrorTest(createUnmarshaller(type), type, dtypes);
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param um      An {@link Unmarshaller} instance.
     * @param type    A class which indicates the type of object.
     * @param dtypes  An array of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Unmarshaller um, Class<T> type,
                                            XmlDataType ... dtypes) {
        for (XmlDataType dtype: dtypes) {
            jaxbErrorTest(um, type, dtype);
        }
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param um      An {@link Unmarshaller} instance.
     * @param type    A class which indicates the type of object.
     * @param dtypes  A list of {@link XmlDataType} instances that creates
     *                invalid XML text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Unmarshaller um, Class<T> type,
                                            List<XmlDataType> dtypes) {
        for (XmlDataType dtype: dtypes) {
            jaxbErrorTest(um, type, dtype);
        }
    }

    /**
     * Ensure that broken values in XML can be detected by validation event.
     *
     * @param um     An {@link Unmarshaller} instance.
     * @param type   A class which indicates the type of object.
     * @param dtype  A {@link XmlDataType} instance that creates invalid XML
     *               text.
     * @param <T>  The type of the object to be deserialized.
     */
    protected static <T> void jaxbErrorTest(Unmarshaller um, Class<T> type,
                                            XmlDataType dtype) {
        for (XmlNode xn: dtype.createInvalidNodes()) {
            try {
                unmarshal(um, xn.toString(), type);
                fail("Broken XML has been unmarshalled: " + xn);
            } catch (UnmarshalException e) {
                Throwable rootCause = null;
                Throwable cause = e.getCause();
                if (cause != null) {
                    while (true) {
                        if (cause instanceof IllegalArgumentException) {
                            rootCause = cause;
                            break;
                        }
                        Throwable c = cause.getCause();
                        if (c == null) {
                            rootCause = cause;
                            break;
                        }
                        cause = c;
                    }
                }
                if (!(rootCause instanceof IllegalArgumentException)) {
                    fail("Unexpected exception: " + e + ", cause=" +
                         rootCause + ", xml=" + xn);
                }
            } catch (Exception e) {
                unexpected(e);
            }
        }
    }

    /**
     * Delete the specified directory recursively.
     *
     * @param file  A {@link File} instance which represents a file or
     *              directory to be removed.
     */
    protected static void delete(File file) {
        if (!file.exists()) {
            return;
        }

        File[] files = file.listFiles();
        if (files == null) {
            // Delete the specified file.
            file.delete();
            return;
        }

        // Make the specified directory empty.
        for (File f: files) {
            delete(f);
        }

        // Delete the specified directory.
        file.delete();
    }

    /**
     * Return a path to the default container configuration directory.
     *
     * @return  A {@link File} instance which represents the default container
     *          configuration directory.
     */
    protected static File getConfigDir() {
        return new File(new File(CONFIG_DIR_BASE), VTN_CONFIG_DIR);
    }

    /**
     * Delete configuration directory after test.
     */
    @After
    public void deleteConfigDir() {
        delete(new File(CONFIG_DIR_BASE));
    }

    /**
     * Return the value of the field configured in the given object.
     *
     * @param obj   The target object.
     * @param type  A class which specifies the type of the field value.
     * @param name  The name of the field.
     * @param <T>   The type of the field value.
     * @return  The value of the field configured in the given object.
     * @throws Exception  An error occurred.
     */
    public static <T> T getFieldValue(Object obj, Class<T> type, String name)
        throws Exception {
        return getFieldValue(obj, obj.getClass(), type, name);
    }

    /**
     * Return the value of the field configured in the given object.
     *
     * @param obj      The target object.
     * @param objType  A class which contains the declaration of the specified
     *                 field.
     * @param type     A class which specifies the type of the field value.
     * @param name     The name of the field.
     * @param <T>      The type of the field value.
     * @return  The value of the field configured in the given object.
     * @throws Exception  An error occurred.
     */
    public static <T> T getFieldValue(Object obj, Class<?> objType,
                                      Class<T> type, String name)
        throws Exception {
        Field field = objType.getDeclaredField(name);
        field.setAccessible(true);
        Object value = field.get(obj);

        assertTrue(value == null || type.isInstance(value));
        return type.cast(value);
    }

    /**
     * Ensure that the given future associated with RPC contains an error state
     * caused by a null RPC input.
     *
     * @param future  A future associated with RPC.
     * @param <T>     The type of RPC output.
     * @throws Exception  An error occurred.
     */
    public static <T> void verifyRpcInputNull(Future<RpcResult<T>> future)
        throws Exception {
        verifyRpcFailure(future, MISSING_ELEMENT, "BADREQUEST",
                         "RPC input cannot be null");
    }

    /**
     * Ensure that the given future associated with RPC contains an error
     * state.
     *
     * @param future  A future associated with RPC.
     * @param tag     The expected error tag.
     * @param apTag   The expected application error tag.
     * @param msg     The expected error message.
     * @param <T>     The type of RPC output.
     * @throws Exception  An error occurred.
     */
    public static <T> void verifyRpcFailure(Future<RpcResult<T>> future,
                                            RpcErrorTag tag, String apTag,
                                            String msg) throws Exception {
        RpcResult<T> result = future.get(1L, TimeUnit.SECONDS);
        assertEquals(false, result.isSuccessful());

        Collection<RpcError> errors = result.getErrors();
        assertEquals(1, errors.size());

        RpcError error = errors.iterator().next();
        assertEquals(ErrorType.APPLICATION, error.getErrorType());
        assertEquals(tag.getErrorTag(), error.getTag());
        assertEquals(apTag, error.getApplicationTag());
        assertEquals(msg, error.getMessage());
    }

    /**
     * Ensure that the tiven two collecitons are identical.
     *
     * <p>
     *   This method compares the given collections as set.
     * </p>
     *
     * @param expected  An expected collection.
     * @param c         An collection to be tested.
     */
    public static <T> void assertEqualsAsSet(Collection<T> expected,
                                             Collection<T> c) {
        if (expected == null) {
            assertEquals(null, c);
            return;
        }
        assertNotNull(c);

        Set<T> eset = new HashSet<>(expected);
        Set<T> set = new HashSet<>(c);
        assertEquals(eset, set);
    }

    /**
     * Ensure that the given two {@link StaticSwitchLinks} instances are
     * identical.
     *
     * @param expected  A {@link StaticSwitchLinks} instance which contains
     *                  expected values.
     * @param swlinks   A {@link StaticSwitchLinks} instance to be tested.
     */
    public static void assertEqualsStaticSwitchLinks(
        StaticSwitchLinks expected, StaticSwitchLinks swlinks) {
        if (expected == null) {
            assertEquals(null, swlinks);
            return;
        }

        List<StaticSwitchLink> elinks = expected.getStaticSwitchLink();
        List<StaticSwitchLink> links = swlinks.getStaticSwitchLink();
        Set<StaticSwitchLink> eset = new HashSet<>();
        Set<StaticSwitchLink> lset = new HashSet<>();
        if (elinks != null) {
            eset.addAll(elinks);
        }
        if (links != null) {
            lset.addAll(links);
        }
        assertEquals(eset, lset);
    }

    /**
     * Ensure that the given two {@link StaticEdgePorts} instances are
     * identical.
     *
     * @param expected  A {@link StaticEdgePorts} instance which contains
     *                  expected values.
     * @param edges     A {@link StaticEdgePorts} instance to be tested.
     */
    public static void assertEqualsStaticEdgePorts(StaticEdgePorts expected,
                                                   StaticEdgePorts edges) {
        if (expected == null) {
            assertEquals(null, edges);
            return;
        }

        List<StaticEdgePort> elist = expected.getStaticEdgePort();
        List<StaticEdgePort> list = edges.getStaticEdgePort();
        Set<StaticEdgePort> eset = new HashSet<>();
        Set<StaticEdgePort> lset = new HashSet<>();
        if (elist != null) {
            eset.addAll(elist);
        }
        if (list != null) {
            lset.addAll(list);
        }
        assertEquals(eset, lset);
    }

    /**
     * Ensure that the given two {@link VtnFlowFilter} instances are
     * identical.
     *
     * @param expected  A {@link VtnFlowFilter} instance which contains
     *                  expected values.
     * @param vff       A {@link VtnFlowFilter} instance to be tested.
     */
    public static void assertEqualsVtnFlowFilter(VtnFlowFilter expected,
                                                 VtnFlowFilter vff) {
        if (expected == null) {
            assertEquals(null, vff);
            return;
        }

        assertEquals(expected.getIndex(), vff.getIndex());
        assertEquals(expected.getCondition(), vff.getCondition());
        assertEquals(expected.getVtnFlowFilterType(),
                     vff.getVtnFlowFilterType());
        assertEqualsAsSet(expected.getVtnFlowAction(), vff.getVtnFlowAction());
    }

    /**
     * Ensure that the given two {@link MacMapConfig} instances are identical.
     *
     * @param expected  A {@link MacMapConfig} instance which contains expected
     *                  values.
     * @param mconf     A {@link MacMapConfig} instance to be tested.
     */
    public static void assertEqualsMacMapConfig(MacMapConfig expected,
                                                MacMapConfig mconf) {
        if (expected == null) {
            assertEquals(null, mconf);
            return;
        }
        assertNotNull(mconf);

        AllowedHosts exAllowed = expected.getAllowedHosts();
        if (exAllowed == null) {
            assertEquals(null, mconf.getAllowedHosts());
        } else {
            AllowedHosts allowed = mconf.getAllowedHosts();
            assertNotNull(allowed);
            assertEqualsAsSet(exAllowed.getVlanHostDescList(),
                              allowed.getVlanHostDescList());
        }

        DeniedHosts exDenied = expected.getDeniedHosts();
        if (exDenied == null) {
            assertEquals(null, mconf.getDeniedHosts());
        } else {
            DeniedHosts denied = mconf.getDeniedHosts();
            assertNotNull(denied);
            assertEqualsAsSet(exDenied.getVlanHostDescList(),
                              denied.getVlanHostDescList());
        }
    }

    /**
     * Create a response of read request on a MD-SAL transaction.
     *
     * @param obj  An object to be read.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the data object.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadResult(
        T obj) {
        Optional<T> opt = Optional.fromNullable(obj);
        return Futures.<Optional<T>, ReadFailedException>
            immediateCheckedFuture(opt);
    }

    /**
     * Create an error response of read request on a MD-SAL transaction.
     *
     * @param type   A class which indicates the type of the return value.
     * @param cause  A throwable which indicates the cause of error.
     * @return  A {@link CheckedFuture} instance.
     * @param <T>  The type of the return value.
     */
    protected static <T extends DataObject> CheckedFuture<Optional<T>, ReadFailedException> getReadFailure(
        Class<T> type, Throwable cause) {
        String msg = "DS read failed";
        RpcError err = RpcResultBuilder.newError(
            ErrorType.APPLICATION, "failed", msg, null, null, cause);
        ReadFailedException rfe = new ReadFailedException(msg, cause, err);
        return Futures.<Optional<T>, ReadFailedException>
            immediateFailedCheckedFuture(rfe);
    }

    /**
     * Create a future associated with successful completion of the MD-SAL
     * datastore submit procedure.
     *
     * @return  A {@link CheckedFuture} instance.
     */
    protected static CheckedFuture<Void, TransactionCommitFailedException> getSubmitFuture() {
        ListenableFuture<Void> f = Futures.immediateFuture(null);
        DataStoreExceptionMapper mapper =
            DataStoreExceptionMapper.getInstance();
        return Futures.makeChecked(f, mapper);
    }

    /**
     * Create a future associated with the MD-SAL datastore submit procedure
     * with failure.
     *
     * @param cause  A throwable which indicates the cause of error.
     * @return  A {@link CheckedFuture} instance.
     */
    protected static CheckedFuture<Void, TransactionCommitFailedException> getSubmitFuture(
        Throwable cause) {
        ListenableFuture<Void> f = Futures.immediateFailedFuture(cause);
        DataStoreExceptionMapper mapper =
            DataStoreExceptionMapper.getInstance();
        return Futures.makeChecked(f, mapper);
    }

    /**
     * Create a {@link VtnPortBuilder} instance corresponding to an enabled
     * edge port.
     *
     * @param sport    A {@link SalPort} instance.
     */
    protected static VtnPortBuilder createVtnPortBuilder(SalPort sport) {
        return createVtnPortBuilder(sport, Boolean.TRUE, true);
    }

    /**
     * Create a {@link VtnPortBuilder} instance.
     *
     * @param sport    A {@link SalPort} instance.
     * @param enabled  The state of the port.
     * @param edge     Determine edge port or not.
     */
    protected static VtnPortBuilder createVtnPortBuilder(
        SalPort sport, Boolean enabled, boolean edge) {
        String name = "sw" + sport.getNodeNumber() + "-eth" +
            sport.getPortNumber();
        VtnPortBuilder builder = new VtnPortBuilder();
        builder.setCost(Long.valueOf(1000)).
            setId(sport.getNodeConnectorId()).setName(name).
            setEnabled(enabled);

        if (!edge) {
            PortLinkBuilder pbuilder = new PortLinkBuilder();
            long n = sport.getNodeNumber() + 100L;
            NodeConnectorId peer = new SalPort(n, sport.getPortNumber()).
                getNodeConnectorId();
            pbuilder.setLinkId(new LinkId("isl:" + name)).
                setPeer(peer);
            List<PortLink> list = new ArrayList<>();
            list.add(pbuilder.build());
            builder.setPortLink(list);
        }

        return builder;
    }

    /**
     * Generate tri-state boolean value.
     *
     * @param b  A {@link Boolean} instance.
     * @return
     *   {@link Boolean#TRUE} if {@code b} is {@code null}.
     *   {@link Boolean#FALSE} if {@code b} is {@link Boolean#TRUE}.
     *   {@code null} if {@code b} is {@link Boolean#FALSE}.
     */
    protected static Boolean triState(Boolean b) {
        if (b == null) {
            return Boolean.TRUE;
        }
        if (Boolean.TRUE.equals(b)) {
            return Boolean.FALSE;
        }

        return null;
    }

    /**
     * let a thread sleep. used for dispatch other thread.
     *
     * @param millis    the length of time in millisecond.
     */
    protected static void sleep(long millis) {
        Thread.yield();
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
            unexpected(e);
        }
    }

    /**
     * Ensure that the specified VTN future does not complete yet.
     *
     * @param future  The future to be tested.
     * @param <T>     The type of the result of the future.
     * @throws Exception  An error occurred.
     */
    protected static <T> void verifyNotComplete(VTNFuture<T> future)
        throws Exception {
        assertEquals(false, future.isCancelled());
        assertEquals(false, future.isDone());

        try {
            future.get(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (TimeoutException e) {
        }

        try {
            future.checkedGet(1L, TimeUnit.NANOSECONDS);
            unexpected();
        } catch (VTNException e) {
            assertEquals(VtnErrorTag.TIMEOUT, e.getVtnErrorTag());
        }
    }
}
