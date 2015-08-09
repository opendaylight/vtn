/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import static org.junit.Assert.fail;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;

/**
 * {@code XmlDataType} describes the data type of the XML node value or
 * attribute.
 */
public abstract class XmlDataType {
    /**
     * A map that keeps strings that cannot be converted into number.
     */
    private static final Map<Class<?>, String[]> INVALID_STRINGS;

    /**
     * Initialize static fields.
     */
    static {
        Map<Class<?>, String[]> map = new HashMap<>();
        INVALID_STRINGS = map;

        String[] forLong = {
            "",
            "bad-long-number",
            "9223372036854775808",
            "-9223372036854775809",
            "1000000000000000000000000000000000",
            "-1000000000000000000000000000000000",
        };
        map.put(Long.class, forLong);
        map.put(long.class, forLong);

        String[] forInt = {
            "",
            "bad-integer",
            "2147483648",
            "-2147483649",
            "9223372036854775807",
            "-9223372036854775808",
            "1000000000000000000000000000000000",
            "-1000000000000000000000000000000000",
        };
        map.put(Integer.class, forInt);
        map.put(int.class, forInt);

        String[] forShort = {
            "",
            "bad-short",
            "32768",
            "-32769",
            "2147483647",
            "-2147483648",
            "9223372036854775807",
            "-9223372036854775808",
            "1000000000000000000000000000000000",
            "-1000000000000000000000000000000000",
        };
        map.put(Short.class, forShort);
        map.put(short.class, forShort);

        String[] forByte = {
            "",
            "bad-byte",
            "128",
            "-129",
            "32767",
            "-32768",
            "2147483647",
            "-2147483648",
            "9223372036854775807",
            "-9223372036854775808",
            "1000000000000000000000000000000000",
            "-1000000000000000000000000000000000",
        };
        map.put(Byte.class, forByte);
        map.put(byte.class, forByte);

        String[] forDouble = {
            "",
            "bad-double",
            "11.00.11",
            "-100.123.456E+200",
        };
        map.put(Double.class, forDouble);
        map.put(double.class, forDouble);

        String[] forEther = {
            "",
            "bad address",
            "aabccddeeff",
            "00:11:22:33:44:55:66",
            "00:11:222:33:44:55:66",
            "aa:bb:cc::dd:ee:ff",
        };
        map.put(EtherAddress.class, forEther);

        String[] forIp4 = {
            "Bad IP address",
            "Bad IP address/3",
            "123.456.789.abc",
            "123.456.789.abc/12",
            "abc::ddee::123",
            "abc::ddee::123/45",
            "::1",
            "::1/32",
            "10.1.2.3/",
            "10.1.2.3/0x123",
            "10.1.2.3/3.141592",
            "10.1.2.3/1/2/3",
        };
        map.put(Ip4Network.class, forIp4);
    }

    /**
     * Path to the paremt XML node.
     */
    private final Deque<String>  parentPath = new LinkedList<>();

    /**
     * The name of the target node.
     */
    private final String  nodeName;

    /**
     * A class which represents the type of data.
     */
    private final Class<?>  dataType;

    /**
     * Construct a node path by appending the given node name to the node path.
     *
     * @param name  The node name to be appended.
     * @param path  The node path.
     * @return  A new node path.
     */
    public static final String[] addPath(String name, String ... path) {
        String[] newPath = new String[path.length + 1];
        System.arraycopy(path, 0, newPath, 0, path.length);
        newPath[path.length] = name;
        return newPath;
    }

    /**
     * Construct a new instance.
     *
     * @param name  The name of the target node.
     * @param type  A class which represents the type of data.
     */
    protected XmlDataType(String name, Class<?> type) {
        nodeName = name;
        dataType = type;
    }

    /**
     * Append the given list of XML nodes to the parent node path.
     *
     * @param nodes  An array of XML node names.
     * @return  This instance.
     */
    public final XmlDataType add(String ... nodes) {
        Collections.addAll(parentPath, nodes);
        return this;
    }

    /**
     * Prepend the given node to the parent node path.
     *
     * @param nodes  An array of XML node names.
     * @return  This instance.
     */
    public final XmlDataType prepend(String ... nodes) {
        for (int i = nodes.length - 1; i >= 0; i--) {
            parentPath.addFirst(nodes[i]);
        }
        return this;
    }

    /**
     * Create a list of {@link XmlNode} instances that contain invalid
     * value in XML node value or attribute.
     *
     * @return  A list of {@link XmlNode} instances.
     */
    public final List<XmlNode> createInvalidNodes() {
        List<XmlNode> list = new ArrayList<>();
        String[] invalid = INVALID_STRINGS.get(dataType);
        if (invalid == null) {
            fail("Unsupported data type: " + dataType);
        } else {
            for (String value: invalid) {
                list.add(createNode(value));
            }
        }

        return list;
    }

    /**
     * Create a XML node that contains the given value.
     *
     * @param value   A value to be set.
     * @return  A {@link XmlNode} instance which specifies the root node.
     */
    private XmlNode createNode(String value) {
        // Create parent containers.
        XmlNode root = null;
        XmlNode parent = null;
        for (String name: parentPath) {
            XmlNode xn = new XmlNode(name);
            if (root == null) {
                root = xn;
            } else if (parent != null) {
                parent.add(xn);
            }
            parent = xn;
        }

        XmlNode target = createNode(nodeName, value);
        XmlNode result;
        if (root == null) {
            // The target node should be a root node.
            result = target;
        } else {
            parent.add(target);
            result = root;
        }

        return result;
    }

    /**
     * Create the target node.
     *
     * @param name   The name of the target node.
     * @param value  The value to be set.
     * @return  A {@link XmlNode} instance.
     */
    protected abstract XmlNode createNode(String name, String value);
}
