/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import javax.xml.bind.JAXBContext;
import javax.xml.bind.JAXBException;
import javax.xml.bind.Marshaller;
import javax.xml.bind.UnmarshalException;
import javax.xml.bind.Unmarshaller;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.AnnotationIntrospector;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.introspect.JacksonAnnotationIntrospector;
import com.fasterxml.jackson.module.jaxb.JaxbAnnotationIntrospector;

import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.VInterfaceConfig;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

import org.junit.Assert;

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
     * Throw an error which indicates an unexpected throwable is caught.
     *
     * @param t  A throwable.
     */
    protected static void unexpected(Throwable t) {
        throw new AssertionError("Unexpected throwable: " + t, t);
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
     * Create a copy of the specified boolean object.
     *
     * @param bool  A boolean object to be copied.
     * @return      A copied boolean object.
     */
    protected static Boolean copy(Boolean bool) {
        if (bool != null) {
            bool = new Boolean(bool.booleanValue());
        }
        return bool;
    }

    /**
     * Create a copy of the specified {@link Node}.
     *
     * @param node  A {@link Node} object to be copied.
     * @return      A copied {@link Node} object.
     */
    protected static Node copy(Node node) {
        if (node != null) {
            try {
                node = new Node(node);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return node;
    }

    /**
     * Create a copy of the specified {@link NodeConnector}.
     *
     * @param nc  A {@link NodeConnector} object to be copied.
     * @return    A copied {@link NodeConnector} object.
     */
    protected static NodeConnector copy(NodeConnector nc) {
        if (nc != null) {
            try {
                nc = new NodeConnector(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return nc;
    }

    /**
     * Create a copy of the specified {@link SwitchPort}.
     *
     * @param port  A {@link SwitchPort} object to be copied.
     * @return      A copied {@link SwitchPort} object.
     */
    protected static SwitchPort copy(SwitchPort port) {
        if (port != null) {
            port = new SwitchPort(port.getName(), port.getType(),
                                  port.getId());
        }
        return port;
    }

    /**
     * Create a copy of the specified {@link VInterfaceConfig}.
     *
     * @param iconf  A {@link VInterfaceConfig} object to be copied.
     * @return       A copied {@link VInterfaceConfig} object.
     */
    protected static VInterfaceConfig copy(VInterfaceConfig iconf) {
        if (iconf != null) {
            iconf = new VInterfaceConfig(copy(iconf.getDescription()),
                                         copy(iconf.getEnabled()));
        }
        return iconf;
    }

    /**
     * Create a copy of the specified {@link DataLinkAddress}.
     *
     * @param addr  A {@link DataLinkAddress} object to be copied.
     * @return      A copied {@link DataLinkAddress} object.
     */
    protected static DataLinkAddress copy(DataLinkAddress addr) {
        if (addr != null) {
            addr = addr.clone();
        }
        return addr;
    }

    /**
     * Create a copy of the specified {@link EthernetHost}.
     *
     * @param ehost  A {@link EthernetHost} object to be copied.
     * @return       A copied {@link EthernetHost} object.
     */
    protected static EthernetHost copy(EthernetHost ehost) {
        if (ehost != null) {
            EthernetAddress addr = ehost.getAddress();
            short vlan = ehost.getVlan();
            ehost = new EthernetHost((EthernetAddress)copy(addr), vlan);
        }
        return ehost;
    }

    /**
     * Create a copy of the specified {@link TestDataLinkHost}.
     *
     * @param dlhost  A {@link TestDataLinkHost} object to be copied.
     * @return        A copied {@link TestDataLinkHost} object.
     */
    protected static TestDataLinkHost copy(TestDataLinkHost dlhost) {
        if (dlhost != null) {
            DataLinkAddress addr = dlhost.getAddress();
            dlhost = new TestDataLinkHost(copy(addr));
        }
        return dlhost;
    }

    /**
     * Create a copy of the specified {@link MacAddressEntry}.
     *
     * @param ment  A {@link MacAddressEntry} object to be copied.
     * @return      A copied {@link MacAddressEntry} object.
     */
    protected static MacAddressEntry copy(MacAddressEntry ment) {
        if (ment != null) {
            DataLinkAddress dladdr = copy(ment.getAddress());
            short vlan = ment.getVlan();
            NodeConnector nc = copy(ment.getNodeConnector());
            Set<InetAddress> ipaddrs =
                copy(ment.getInetAddresses(), InetAddress.class);
            ment = new MacAddressEntry(dladdr, vlan, nc, ipaddrs);
        }
        return ment;
    }

    /**
     * Create a copy of the specified {@link InetAddress}.
     *
     * @param addr  A {@link InetAddress} object to be copied.
     * @return      A copied {@link InetAddress} object.
     */
    protected static InetAddress copy(InetAddress addr) {
        if (addr != null) {
            try {
                String host = addr.getHostName();
                byte[] raw = addr.getAddress();
                addr = InetAddress.getByAddress(host, raw);
            } catch (Exception e) {
                unexpected(e);
            }
        }
        return addr;
    }

    /**
     * Create a deep copy of the specified set.
     *
     * @param set   A set of instances.
     * @param type  The type of elements in the list.
     * @param <T>   The type of elements in the set.
     * @return  A deep copy of the specified set.
     */
    protected static <T> Set<T> copy(Set<T> set, Class<T> type) {
        Set<T> newset = set;
        if (set != null) {
            newset = new HashSet<T>();
            copy(newset, set, type);
        }
        return newset;
    }

    /**
     * Create a deep copy of the specified list.
     *
     * @param list  A list of instances.
     * @param type  The type of elements in the list.
     * @param <T>   The type of elements in the list.
     * @return  A deep copy of the specified list.
     */
    protected static <T> List<T> copy(List<T> list, Class<T> type) {
        List<T> newlist = list;
        if (list != null) {
            newlist = new ArrayList<T>();
            copy(newlist, list, type);
        }
        return newlist;
    }

    /**
     * Create a deep copy of the specified collection.
     *
     * @param dst   A collection to store copies.
     * @param src   A collection of instances.
     * @param type  The type of elements in the collection.
     * @param <T>   The type of elements in the collection.
     */
    protected static <T> void copy(Collection<T> dst, Collection<T> src,
                                   Class<T> type) {
        for (T elem: src) {
            Object obj;
            if (elem instanceof InetAddress) {
                obj = copy((InetAddress)elem);
            } else if (elem instanceof EthernetHost) {
                obj = copy((EthernetHost)elem);
            } else if (elem instanceof TestDataLinkHost) {
                obj = copy((TestDataLinkHost)elem);
            } else if (elem instanceof MacAddressEntry) {
                obj = copy((MacAddressEntry)elem);
            } else {
                fail("Unexpected instanse in the collection: " + elem);
                break;
            }

            if (obj != null && !type.isInstance(obj)) {
                fail("Unexpected instance: src=" + elem + ", obj=" + obj);
                break;
            }

            dst.add(type.cast(obj));
        }
    }

    /**
     * Create a list of boolean values and a {@code null}.
     *
     * @return A list of boolean values.
     */
    protected static List<Boolean> createBooleans() {
        return createBooleans(true);
    }

    /**
     * Create a list of boolean values.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of boolean values.
     */
    protected static List<Boolean> createBooleans(boolean setNull) {
        ArrayList<Boolean> list = new ArrayList<Boolean>();
        if (setNull) {
            list.add(null);
        }

        list.add(Boolean.TRUE);
        list.add(Boolean.FALSE);
        return list;
    }

    /**
     * Create a list of strings and a {@code null}.
     *
     * @param base A base string.
     * @return A list of strings.
     */
    protected static List<String> createStrings(String base) {
        return createStrings(base, true);
    }

    /**
     * Create a list of strings.
     *
     * @param base     A base string.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of strings.
     */
    protected static List<String> createStrings(String base, boolean setNull) {
        ArrayList<String> list = new ArrayList<String>();
        if (setNull) {
            list.add(null);
        }

        for (int i = 0; i <= base.length(); i++) {
            list.add(base.substring(0, i));
        }

        return list;
    }

    /**
     * Create a list of {@link Node} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link Node}.
     */
    protected static List<Node> createNodes(int num) {
        return createNodes(num, true);
    }

    /**
     * Create a list of {@link Node}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link Node}.
     */
    protected static List<Node> createNodes(int num, boolean setNull) {
        ArrayList<Node> list = new ArrayList<Node>();
        if (setNull) {
            list.add(null);
            num--;
        }

        String[] types = {
            Node.NodeIDType.OPENFLOW,
            Node.NodeIDType.ONEPK,
            Node.NodeIDType.PRODUCTION,
        };

        for (int i = 0; i < num; i++) {
            int tidx = i % types.length;
            String type = types[tidx];
            Object id;
            if (type.equals(Node.NodeIDType.OPENFLOW)) {
                id = new Long((long)i);
            } else {
                id = "Node ID: " + i;
            }

            try {
                Node node = new Node(type, id);
                assertNotNull(node.getType());
                assertNotNull(node.getNodeIDString());
                list.add(node);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a list of {@link NodeConnector} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link NodeConnector}.
     */
    protected static List<NodeConnector> createNodeConnectors(int num) {
        return createNodeConnectors(num, true);
    }

    /**
     * Create a list of {@link NodeConnector}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link NodeConnector}.
     */
    protected static List<NodeConnector> createNodeConnectors(
        int num, boolean setNull) {
        ArrayList<NodeConnector> list = new ArrayList<NodeConnector>();
        if (setNull) {
            list.add(null);
            num--;
        }

        String[] nodeTypes = {
            Node.NodeIDType.OPENFLOW,
            Node.NodeIDType.ONEPK,
            Node.NodeIDType.PRODUCTION,
        };
        String[] connTypes = {
            NodeConnector.NodeConnectorIDType.OPENFLOW,
            NodeConnector.NodeConnectorIDType.ONEPK,
            NodeConnector.NodeConnectorIDType.PRODUCTION,
        };

        for (int i = 0; i < num; i++) {
            int tidx = i % nodeTypes.length;
            String nodeType = nodeTypes[tidx];
            String connType = connTypes[tidx];
            Object nodeId, connId;
            if (nodeType.equals(Node.NodeIDType.OPENFLOW)) {
                nodeId = new Long((long)i);
                connId = new Short((short)(i + 10));
            } else {
                nodeId = "Node ID: " + i;
                connId = "Node Connector ID: " + i;
            }

            try {
                Node node = new Node(nodeType, nodeId);
                assertNotNull(node.getType());
                assertNotNull(node.getNodeIDString());
                NodeConnector nc = new NodeConnector(connType, connId, node);
                assertNotNull(nc.getType());
                assertNotNull(nc.getNodeConnectorIDString());
                list.add(nc);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a list of {@link SwitchPort} and a {@code null}.
     *
     * @param num  The number of objects to be created.
     * @return A list of {@link SwitchPort}.
     */
    protected static List<SwitchPort> createSwitchPorts(int num) {
        return createSwitchPorts(num, true);
    }

    /**
     * Create a list of {@link SwitchPort}.
     *
     * @param num      The number of objects to be created.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link SwitchPort}.
     */
    protected static List<SwitchPort> createSwitchPorts(
        int num, boolean setNull) {
        ArrayList<SwitchPort> list = new ArrayList<SwitchPort>();
        if (setNull) {
            list.add(null);
            num--;
            if (num == 0) {
                return list;
            }
        }

        list.add(new SwitchPort(null, null, null));
        num--;
        if (num == 0) {
            return list;
        }

        list.add(new SwitchPort("name", "type", "id"));
        num--;

        for (; num > 0; num--) {
            String name = ((num % 2) == 0) ? null : "name:" + num;
            String type = ((num % 7) == 0) ? null : "type:" + num;
            String id = ((num % 9) == 0) ? null : "id:" + num;
            if (name == null && type == null && id == null) {
                name = "name:" + num;
            }
            list.add(new SwitchPort(name, type, id));
        }

        return list;
    }

    /**
     * Create a list of {@link EthernetAddress} and a {@code null}.
     *
     * @return A list of {@link EthernetAddress}.
     */
    protected static List<EthernetAddress> createEthernetAddresses() {
        return createEthernetAddresses(true);
    }

    /**
     * Create a list of {@link EthernetAddress}.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link EthernetAddress}.
     */
    protected static List<EthernetAddress> createEthernetAddresses(
        boolean setNull) {
        List<EthernetAddress> list = new ArrayList<EthernetAddress>();
        byte [][] addrbytes = {
            new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                        (byte)0x00, (byte)0x00, (byte)0x01},
            new byte[] {(byte)0x12, (byte)0x34, (byte)0x56,
                        (byte)0x78, (byte)0x9a, (byte)0xbc},
            new byte[] {(byte)0xfe, (byte)0xdc, (byte)0xba,
                        (byte)0x98, (byte)0x76, (byte)0x54}
        };

        if (setNull) {
            list.add(null);
        }

        for (byte[] addr: addrbytes) {
            try {
                EthernetAddress ea;
                ea = new EthernetAddress(addr);
                list.add(ea);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a list of {@link DataLinkAddress} and a {@code null}.
     *
     * @return  A list of {@link DataLinkAddress}.
     */
    protected static List<DataLinkAddress> createDataLinkAddresses() {
        return createDataLinkAddresses(true);
    }

    /**
     * Create a list of {@link DataLinkAddress}.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return  A list of {@link DataLinkAddress}.
     */
    protected static List<DataLinkAddress> createDataLinkAddresses(
        boolean setNull) {
        List<DataLinkAddress> list = new ArrayList<DataLinkAddress>();

        for (EthernetAddress ether: createEthernetAddresses(setNull)) {
            list.add(ether);
        }

        String[] addrs = {"addr1", "addr2", "addr3"};
        for (String a: addrs) {
            list.add(new TestDataLink(a));
        }

        return list;
    }

    /**
     * Create a list of {@link InetAddress} set and a {@code null}.
     *
     * @return A list of {@link InetAddress} set.
     */
    protected static List<Set<InetAddress>> createInetAddresses() {
        return createInetAddresses(true);
    }

    /**
     * Create a list of {@link InetAddress} set.
     *
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return A list of {@link InetAddress} set.
     */
    protected static List<Set<InetAddress>> createInetAddresses(
        boolean setNull) {
        List<Set<InetAddress>> list = new ArrayList<Set<InetAddress>>();
        String[][] arrays = {
            {"0.0.0.0"},
            {"255.255.255.255", "10.1.2.3"},
            {"0.0.0.0", "10.0.0.1", "192.255.255.1",
             "2001:420:281:1004:e123:e688:d655:a1b0"},
            {},
        };

        if (setNull) {
            list.add(null);
        }

        for (String[] array: arrays) {
            Set<InetAddress> iset = new HashSet<InetAddress>();
            try {
                for (String addr: array) {
                    assertTrue(iset.add(InetAddress.getByName(addr)));
                }
                list.add(iset);
            } catch (Exception e) {
                unexpected(e);
            }
        }

        return list;
    }

    /**
     * Create a list of set of {@link EthernetHost} instances.
     *
     * @param limit    The upper limit of the number of ethernet addresses.
     * @return  A list of set of {@link EthernetHost} instances.
     */
    protected static List<Set<EthernetHost>> createEthernetHostSet(int limit) {
        return createEthernetHostSet(limit, true);
    }

    /**
     * Create a list of set of {@link EthernetHost} instances.
     *
     * @param limit    The upper limit of the number of ethernet addresses.
     * @param setNull  Set {@code null} to returned list if {@code true}.
     * @return  A list of set of {@link EthernetHost} instances.
     */
    protected static List<Set<EthernetHost>> createEthernetHostSet(
        int limit, boolean setNull) {
        List<Set<EthernetHost>> list = new ArrayList<Set<EthernetHost>>();
        if (setNull) {
            list.add(null);
        }

        HashSet<EthernetHost> set = new HashSet<EthernetHost>();
        short[] vlans = {0, 4095};
        for (short vlan: vlans) {
            for (EthernetAddress ether: createEthernetAddresses()) {
                set.add(new EthernetHost(ether, vlan));
                list.add(new HashSet<EthernetHost>(set));
                if (list.size() >= limit) {
                    break;
                }
            }
        }

        return list;
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
                                        String separator, Object ... args) {
        StringBuilder builder = new StringBuilder();
        if (prefix != null) {
            builder.append(prefix);
        }

        boolean first = true;
        for (Object o: args) {
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

        for (Object o: set) {
            assertFalse("o1=" + o1 + ", o=" + o, o1.equals(o));
            assertFalse(o.equals(o1));
        }

        assertTrue(set.add(o1));
        assertFalse(set.add(o1));
        assertFalse(set.add(o2));
    }

    /**
     * Ensure that the given object is serializable.
     *
     * @param o  An object to be tested.
     */
    protected static void serializeTest(Object o) {
        // Serialize the given object.
        byte[] bytes = null;
        try {
            ByteArrayOutputStream bout = new ByteArrayOutputStream();
            ObjectOutputStream out = new ObjectOutputStream(bout);

            out.writeObject(o);
            out.close();
            bytes = bout.toByteArray();
        } catch (Exception e) {
            unexpected(e);
        }
        assertTrue(bytes.length != 0);

        // Deserialize the object.
        Object newobj = null;
        try {
            ByteArrayInputStream bin = new ByteArrayInputStream(bytes);
            ObjectInputStream in = new ObjectInputStream(bin);
            newobj = in.readObject();
            in.close();
        } catch (Exception e) {
            unexpected(e);
        }

        assertNotSame(o, newobj);
        assertEquals(o, newobj);
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
        // Ensure that the class of the given class has XmlRootElement
        // annotation.
        XmlRootElement xmlRoot = cls.getAnnotation(XmlRootElement.class);
        assertNotNull(xmlRoot);
        assertEquals(rootName, xmlRoot.name());

        // Marshal the given object into XML.
        Marshaller m = createMarshaller(cls);
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
        Unmarshaller um = createUnmarshaller(cls);
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
                            return;
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
     * Ensure that the given object is mapped to JSON object.
     *
     * @param o     An object to be tested.
     * @param type  A {@link Class} instance associated with the type of the
     *              given object.
     * @param <T>   The type of the given object.
     * @return  Deserialized object.
     */
    protected static <T> T jsonTest(T o, Class<T> type) {
        ObjectMapper mapper = getJsonObjectMapper();

        try {
            // Marshal the given object into JSON.
            String json = mapper.writeValueAsString(o);
            assertNotNull(json);
            assertTrue(json.length() != 0);

            // Unmarshal the JSON notation.
            T newobj = mapper.readValue(json, type);
            assertNotSame(o, newobj);
            assertEquals(o, newobj);
            return newobj;
        } catch (Exception e) {
            unexpected(e);
        }

        return null;
    }

    /**
     * Create Jackson's {@code ObjectMapper} instance.
     *
     * @return  A {@code ObjectMapper} instance.
     */
    protected static ObjectMapper getJsonObjectMapper() {
        // Create Jackson object mapper with enabling JAXB annotations.
        ObjectMapper mapper = new ObjectMapper();
        AnnotationIntrospector introspector =
            AnnotationIntrospector.pair(new JacksonAnnotationIntrospector(),
                                        new JaxbAnnotationIntrospector());
        mapper.setAnnotationIntrospector(introspector);

        return mapper;
    }
}
