/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

/**
 * {@code XmlValueType} describes the data type of the value in the specified
 * XML node.
 */
public final class XmlValueType extends XmlDataType {
    /**
     * Construct a new instance.
     *
     * @param name  The name of the target node.
     * @param type  A class which represents the type of data.
     */
    public XmlValueType(String name, Class<?> type) {
        super(name, type);
    }

    // XmlDataType

    /**
     * {@inheritDoc}
     */
    @Override
    protected XmlNode createNode(String name, String value) {
        return new XmlNode(name, value);
    }
}
