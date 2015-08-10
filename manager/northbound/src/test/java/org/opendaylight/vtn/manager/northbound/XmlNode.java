/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * {@code XmlNode} describes a XML node.
 */
public final class XmlNode {
    /**
     * The name of this XML node.
     */
    private final String  nodeName;

    /**
     * The value of this XML node.
     */
    private final String  nodeValue;

    /**
     * Attributes of this node.
     */
    private final Map<String, Object>  attributes = new HashMap<>();

    /**
     * A list of child nodes.
     */
    private final List<XmlNode>  children = new ArrayList<>();

    /**
     * Construct a new instance.
     *
     * @param name  The name of the XML node.
     */
    public XmlNode(String name) {
        this(name, null);
    }

    /**
     * Construct a new instance.
     *
     * @param name   The name of the XML node.
     * @param value  The value of the XML node.
     */
    public XmlNode(String name, Object value) {
        nodeName = name;
        nodeValue = (value == null) ? null : value.toString();
    }

    /**
     * Add the given node into the children list.
     *
     * @param node  A {@link XmlNode} instance.
     * @return  This instance.
     */
    public XmlNode add(XmlNode node) {
        children.add(node);
        return this;
    }

    /**
     * Set attributes of this node.
     *
     * @param attr   The name of the attribute.
     * @param value  The value of the attribute.
     * @return  This instance.
     */
    public XmlNode setAttribute(String attr, Object value) {
        attributes.put(attr, value);
        return this;
    }

    /**
     * Set a XML node which represents this node into the given string builder.
     *
     * @param builder  A string builder.
     */
    private void setXml(StringBuilder builder) {
        builder.append('<').append(nodeName);
        for (Map.Entry<String, Object> entry: attributes.entrySet()) {
            String key = entry.getKey();
            Object value = entry.getValue();
            builder.append(' ').append(key).append("=\"").
                append(String.valueOf(value)).append("\"");
        }
        builder.append('>');

        for (XmlNode child: children) {
            child.setXml(builder);
        }
        if (nodeValue != null) {
            builder.append(nodeValue);
        }

        builder.append("</").append(nodeName).append('>');
    }

    // Object

    /**
     * Return a string representation of this node.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(TestBase.XML_DECLARATION);
        setXml(builder);
        return builder.toString();
    }
}
