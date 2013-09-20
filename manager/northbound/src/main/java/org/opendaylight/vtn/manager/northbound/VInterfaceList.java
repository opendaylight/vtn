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

import org.opendaylight.vtn.manager.VInterface;

/**
 * A JAXB class which represents a list of virtual interfaces.
 */
@XmlRootElement(name = "interfaces")
@XmlAccessorType(XmlAccessType.NONE)
public class VInterfaceList {
    /**
     * A list of virtual interface information.
     */
    @XmlElement(name = "interface")
    private List<VInterface>  ifList;

    /**
     * Default constructor.
     */
    public VInterfaceList() {
    }

    /**
     * Construct a interface list.
     *
     * @param list  A list of interface information.
     */
    public VInterfaceList(List<VInterface> list) {
        ifList = list;
    }

    /**
     * Return a list of virtual interface information.
     *
     * @return A list of virtual interface information.
     */
    List<VInterface> getList() {
        return ifList;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VInterfaceList)) {
            return false;
        }

        List<VInterface> list = ((VInterfaceList)o).ifList;
        if (ifList == null || ifList.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return ifList.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (ifList != null && !ifList.isEmpty()) {
            h ^= ifList.hashCode();
        }

        return h;
    }
}
