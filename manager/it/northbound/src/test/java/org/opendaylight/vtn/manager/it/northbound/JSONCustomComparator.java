/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.northbound;

import java.util.Deque;

import org.codehaus.jettison.json.JSONException;
import org.codehaus.jettison.json.JSONObject;

/**
 * {@code JSONCustomComparator} describes a comparison function for JSON
 * objects.
 */
public interface JSONCustomComparator {
    /**
     * Compares the given JSON objects.
     *
     * @param path   A sequence of JSON keys that indicates the path to the
     *               given JSON objects.
     * @param json1  A {@link JSONObject} to be compared.
     * @param json2  A {@link JSONObject} to be compared.
     * @return  {@code true} only if the given objects are identical.
     * @throws JSONException  An error occurred.
     */
    boolean equals(Deque<String> path, JSONObject json1, JSONObject json2)
        throws JSONException;
}
