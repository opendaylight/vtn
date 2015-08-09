/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonElement;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

@JsonObject
public class Match {

    /**
     * Unique Id for for Match
     */
    @JsonElement(name = "index")
    private int index = 0;

    /**
     * ethernet reference for Match
     */
    @JsonObjectRef(name = "ethernet")
    Ethernet ethernet = new Ethernet();

    /**
     * InetMatch reference for Match
     */
    @JsonObjectRef(name = "inetMatch")
    InetMatch inetMatch = new InetMatch();

    /**
     * L4Match reference for Match
     */
    @JsonObjectRef(name = "l4Match")
    L4Match l4match = new L4Match();

    public Match() {
    }

    public Match(int index, Ethernet ethernet, InetMatch inetMatch,
            L4Match l4match) {
        this.index = index;
        this.ethernet = ethernet;
        this.inetMatch = inetMatch;
        this.l4match = l4match;
    }

    /**
     * getIndex - function to get the index values for this object.
     *
     * @return The index of this match.
     */
    public int getIndex() {
        return index;
    }

    /**
     * setIndex - function to set the index values for this object.
     *
     * @param index
     */
    public void setIndex(int index) {
        this.index = index;
    }

    /**
     * getEthernet - function to get the ethernet values for this object.
     *
     * @return {@link Ethernet}
     */
    public Ethernet getEthernet() {
        return ethernet;
    }

    /**
     * setEthernet - function to set the ethernet values for this object.
     *
     * @param ethernet
     */
    public void setEthernet(Ethernet ethernet) {
        this.ethernet = ethernet;
    }

    /**
     * getInetMatch - function to get the inetMatch values for this object.
     *
     * @return {@link InetMatch}
     */
    public InetMatch getInetMatch() {
        return inetMatch;
    }

    /**
     * setInetMatch - function to set the inetMatch values for this object.
     *
     * @param inetMatch
     */
    public void setInetMatch(InetMatch inetMatch) {
        this.inetMatch = inetMatch;
    }

    /**
     * getL4match - function to get the l4match values for this object.
     *
     * @return {@link L4Match}
     */
    public L4Match getL4match() {
        return l4match;
    }

    /**
     * setL4match - function to set the l4match values for this object.
     *
     * @param l4match
     */
    public void setL4match(L4Match l4match) {
        this.l4match = l4match;
    }

    /**
     * String representation of the object.
     *
     */
    @Override
    public String toString() {
        return "Match [index = " + index + ", ethernet = " + ethernet
                + ", inetMatch = " + inetMatch + ", l4match = " + l4match + "]";
    }
}
