/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.app.run.config.rest.response.beans;

import org.opendaylight.vtn.app.run.config.json.annotations.JsonObject;
import org.opendaylight.vtn.app.run.config.json.annotations.JsonObjectRef;

/**
 * Action - Bean Representaion for Action object from the JSON Response.
 *
 */

@JsonObject
public class Action {

    /**
     * Atribute  Popvlan in Action Object.
     */
    @JsonObjectRef(name = "popvlan")
    private Popvlan pobj;

    /**
     * Atribute  DlSrc in Action Object.
     */
    @JsonObjectRef(name = "dlsrc")
    private DlSrc dobj;

    public Action() {
    }

    /**
     * getPobj - function to get the Popvlan refrence.
     * @return {@link Popvlan}
     */
    public Popvlan getPobj() {
        return pobj;
    }

    /**
     * setPobj - function to set the Popvlan refrence.
     * @param pobj
     */
    public void setPobj(Popvlan pobj) {
        this.pobj = pobj;
    }

    /**
     * getDobj - function to get the DlSrc refrence.
     * @return {@link DlSrc}
     */
    public DlSrc getDobj() {
        return dobj;
    }

    /**
     * setDobj - function to set the DlSrc refrence.
     * @param dobj
     */
    public void setDobj(DlSrc dobj) {
        this.dobj = dobj;
    }

    /**
     * String representation of the Action object.
     *
     */
    @Override
    public String toString() {
        return "Action [pobj = " + pobj + ", dobj = " + dobj + "]";
    }
}
