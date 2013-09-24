/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.VBridge;

/**
 * A JAXB class which represents a list of virtual L2 bridge information.
 */
@XmlRootElement(name = "vbridges")
@XmlAccessorType(XmlAccessType.NONE)
public class VBridgeList {
    /**
     * A list of virtual L2 bridge information.
     */
    @XmlElement(name = "vbridge")
    private List<VBridge>  bridgeList;

    /**
     * Default constructor used by JAXB.
     */
    @SuppressWarnings("unused")
    private VBridgeList() {
    }

    /**
     * Construct a bridge list.
     *
     * @param list  A list of bridge information.
     */
    public VBridgeList(List<VBridge> list) {
        bridgeList = list;
    }

    /**
     * Return a list of virtual L2 bridge information.
     *
     * @return A list of virtual L2 bridge information.
     */
    List<VBridge> getList() {
        return bridgeList;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VBridgeList)) {
            return false;
        }

        List<VBridge> list = ((VBridgeList)o).bridgeList;
        if (bridgeList == null || bridgeList.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return bridgeList.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (bridgeList != null && !bridgeList.isEmpty()) {
            h ^= bridgeList.hashCode();
        }

        return h;
    }
}
