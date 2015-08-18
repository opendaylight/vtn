/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code XmlAttributeType} describes the data type of the attribute in the
 * specified XML node.
 */
public final class XmlAttributeType extends XmlDataType {
    /**
     * The name of the target attribute.
     */
    private final String  attributeName;

    /**
     * Construct a new instance.
     *
     * @param name  The name of the target node.
     * @param attr  The name of the attribute in the node specified by
     *              {@code name}.
     * @param type  A class which represents the type of data.
     */
    public XmlAttributeType(String name, String attr, Class<?> type) {
        super(name, type);
        attributeName = attr;
    }

    // XmlDataType

    /**
     * {@inheritDoc}
     */
    @Override
    protected XmlNode createNode(String name, String value) {
        return new XmlNode(name).setAttribute(attributeName, value);
    }
}
