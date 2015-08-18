/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.VTerminal;

/**
 * {@code VTerminalList} class describes a list of vTerminal information.
 *
 * <p>
 *   This class is used to return a list of vTerminal information to
 *   REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vterminal": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "vterm_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"description": "Description about vTerminal 1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"faults": "0",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"state": "-1"
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "vterm_2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"faults": "2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"state": "0"
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vterminals")
@XmlAccessorType(XmlAccessType.NONE)
public class VTerminalList {
    /**
     * A list of {@link VTerminal} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more {@link VTerminal} instances which
     *     represent VTerminal information.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "vterminal")
    private List<VTerminal>  terminalList;

    /**
     * Default constructor used by JAXB.
     */
    @SuppressWarnings("unused")
    private VTerminalList() {
    }

    /**
     * Construct a vTerminal list.
     *
     * @param list  A list of vTerminal information.
     */
    public VTerminalList(List<VTerminal> list) {
        terminalList = list;
    }

    /**
     * Return a list of vTerminal information.
     *
     * @return A list of vTerminal information.
     */
    List<VTerminal> getList() {
        return terminalList;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VTerminalList)) {
            return false;
        }

        List<VTerminal> list = ((VTerminalList)o).terminalList;
        if (terminalList == null || terminalList.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return terminalList.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (terminalList != null && !terminalList.isEmpty()) {
            h ^= terminalList.hashCode();
        }

        return h;
    }
}
