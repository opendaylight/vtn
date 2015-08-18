/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.northbound;

import static org.junit.Assert.assertTrue;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Deque;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;

import org.codehaus.jettison.json.JSONArray;
import org.codehaus.jettison.json.JSONException;
import org.codehaus.jettison.json.JSONObject;

/**
 * Comparator for {@link JSONObject} and {@link JSONArray}.
 */
public final class JSONComparator {
    /**
     * A pseudo JSON key that indicates an element in a JSON array.
     */
    public static final String KEY_ARRAY = "[ARRAY]";

    /**
     * A set of paths to JSON array to be treated as unordered list.
     */
    private final Set<List<String>>  unorderedArrays = new HashSet<>();

    /**
     * Custom comparators for specific JSON objects.
     */
    private final Map<List<String>, JSONCustomComparator> customComparators =
        new HashMap<>();

    /**
     * Return a set of keys in the given {@link JSONObject} instance.
     *
     * @param json  A {@link JSONObject} instance.
     * @return  A set of keys in the given {@link JSONObject} instance.
     */
    public static Set<String> getKeys(JSONObject json) {
        Set<String> keys = new HashSet<>();
        for (Iterator<?> it = json.keys(); it.hasNext();) {
            keys.add(String.valueOf(it.next()));
        }

        return keys;
    }

    /**
     * Add the path to JSON array to be treated as unordered list.
     *
     * @param path  A sequence of JSON keys that specifies the path to the
     *              JSON array.
     * @return  This instance.
     */
    public JSONComparator addUnordered(String ... path) {
        List<String> list = new ArrayList<>();
        Collections.addAll(list, path);
        unorderedArrays.add(list);
        return this;
    }

    /**
     * Add the custom comparator used to compare JSON objects at the specified
     * path.
     *
     * @param comp  A {@link JSONCustomComparator} instance.
     * @param path  A sequence of JSON keys that specifies the path to the
     *              JSON object or array.
     * @return  This instance.
     */
    public JSONComparator addCustomComparator(JSONCustomComparator comp,
                                              String ... path) {
        List<String> list = new ArrayList<>();
        Collections.addAll(list, path);
        customComparators.put(list, comp);
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public JSONComparator reset() {
        unorderedArrays.clear();
        customComparators.clear();
        return this;
    }

    /**
     * Determine whether the given two JSON objects are identical.
     *
     * @param json1  A {@link JSONObject} to be compared.
     * @param json2  A {@link JSONObject} to be compared.
     * @return  {@code true} only if the given objects are identical.
     * @throws JSONException  An error occurred.
     */
    public boolean equals(JSONObject json1, JSONObject json2)
        throws JSONException {
        return equals(new LinkedList<String>(), json1, json2);
    }

    /**
     * Determine whether the given two JSON arrays are identical,
     *
     * @param jarray1  A {@link JSONArray} to be compared.
     * @param jarray2  A {@link JSONArray} to be compared.
     * @return  {@code true} only if the given arrays are identical.
     * @throws JSONException  An error occurred.
     */
    public boolean equals(JSONArray jarray1, JSONArray jarray2)
        throws JSONException {
        return equals(new LinkedList<String>(), jarray1, jarray2);
    }

    /**
     * Determine whether the given two JSON objects are identical.
     *
     * @param path   A sequence of JSON keys that indicates the path to the
     *               given JSON objects.
     * @param json1  A {@link JSONObject} to be compared.
     * @param json2  A {@link JSONObject} to be compared.
     * @return  {@code true} only if the given objects are identical.
     * @throws JSONException  An error occurred.
     */
    private boolean equals(Deque<String> path, JSONObject json1,
                           JSONObject json2) throws JSONException {
        if (json1 == null) {
            return (json2 == null);
        } else if (json2 == null) {
            return false;
        }

        JSONCustomComparator comp = customComparators.get(path);
        if (comp != null) {
            return comp.equals(path, json1, json2);
        }

        Set<String> keys1 = getKeys(json1);
        Set<String> keys2 = getKeys(json2);
        if (!keys1.equals(keys2)) {
            return false;
        }

        for (String key: keys1) {
            Object v1 = json1.get(key);
            Object v2 = json2.get(key);
            if (v1 instanceof JSONArray) {
                if (!(v2 instanceof JSONArray)) {
                    return false;
                }

                JSONArray jarray1 = (JSONArray)v1;
                JSONArray jarray2 = (JSONArray)v2;
                path.addLast(key);
                boolean result = (unorderedArrays.contains(path))
                    ? equalsUnordered(path, jarray1, jarray2)
                    : equals(path, jarray1, jarray2);
                path.removeLast();
                if (!result) {
                    return false;
                }
            } else if (v1 instanceof JSONObject) {
                if (!(v2 instanceof JSONObject)) {
                    return false;
                }

                path.addLast(key);
                boolean result = equals(path, (JSONObject)v1, (JSONObject)v2);
                path.removeLast();
                if (!result) {
                    return false;
                }
            } else if (v2 instanceof JSONObject || v2 instanceof JSONArray ||
                       !equalsScalar(v1, v2)) {
                return false;
            }
        }

        return true;
    }

    /**
     * Determine whether the given two JSON arrays are identical,
     *
     * @param path     A sequence of JSON keys that indicates the path to the
     *                 given JSON objects.
     * @param jarray1  A {@link JSONArray} to be compared.
     * @param jarray2  A {@link JSONArray} to be compared.
     * @return  {@code true} only if the given arrays are identical.
     * @throws JSONException  An error occurred.
     */
    private boolean equals(Deque<String> path, JSONArray jarray1,
                           JSONArray jarray2)
        throws JSONException {
        if (jarray1 == null) {
            return (jarray2 == null);
        } else if (jarray2 == null) {
            return false;
        }

        int len = jarray1.length();
        if (jarray2.length() != len) {
            return false;
        }

        path.addLast(KEY_ARRAY);
        try {
            for (int i = 0; i < len; i++) {
                Object v1 = jarray1.get(i);
                Object v2 = jarray2.get(i);
                if (v1 instanceof JSONObject) {
                    if (!(v2 instanceof JSONObject &&
                          equals(path, (JSONObject)v1, (JSONObject)v2))) {
                        return false;
                    }
                } else if (v1 instanceof JSONArray) {
                    if (!(v2 instanceof JSONArray &&
                          equals(path, (JSONArray)v1, (JSONArray)v2))) {
                        return false;
                    }
                } else if (v2 instanceof JSONObject ||
                           v2 instanceof JSONArray || !equalsScalar(v1, v2)) {
                    return false;
                }
            }
        } finally {
            path.removeLast();
        }

        return true;
    }

    /**
     * Compares the given two JSON arrays as unordered list.
     *
     * <p>
     *   Note that this method ignores element order in the given arrays.
     * </p>
     *
     * @param path     A sequence of JSON keys that indicates the path to the
     *                 given JSON objects.
     * @param jarray1  A {@link JSONArray} to be compared.
     * @param jarray2  A {@link JSONArray} to be compared.
     * @return  {@code true} only if the given arrays are identical.
     * @throws JSONException  An error occurred.
     */
    private boolean equalsUnordered(Deque<String> path, JSONArray jarray1,
                                    JSONArray jarray2)
        throws JSONException {
        if (jarray1 == null) {
            return (jarray2 == null);
        } else if (jarray2 == null) {
            return false;
        }

        int len = jarray1.length();
        if (jarray2.length() != len) {
            return false;
        }

        List<JSONObject> objs1 = new ArrayList<>();
        List<JSONObject> objs2 = new ArrayList<>();
        List<JSONArray> arrays1 = new ArrayList<>();
        List<JSONArray> arrays2 = new ArrayList<>();
        List<Object> others1 = new ArrayList<>();
        List<Object> others2 = new ArrayList<>();
        for (int i = 0; i < len; i++) {
            Object v1 = jarray1.get(i);
            if (v1 instanceof JSONObject) {
                objs1.add((JSONObject)v1);
            } else if (v1 instanceof JSONArray) {
                arrays1.add((JSONArray)v1);
            } else {
                others1.add(v1);
            }

            Object v2 = jarray2.get(i);
            if (v2 instanceof JSONObject) {
                objs2.add((JSONObject)v2);
            } else if (v2 instanceof JSONArray) {
                arrays2.add((JSONArray)v2);
            } else {
                others2.add(v2);
            }
        }

        if (objs1.size() != objs2.size() || arrays1.size() != arrays2.size() ||
            others1.size() != others2.size()) {
            return false;
        }

        path.addLast(KEY_ARRAY);
        try {
            for (JSONObject json1: objs1) {
                boolean found = false;
                for (Iterator<JSONObject> it = objs2.iterator();
                     it.hasNext();) {
                    JSONObject json2 = it.next();
                    if (equals(path, json1, json2)) {
                        it.remove();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }

            for (JSONArray a1: arrays1) {
                boolean found = false;
                for (Iterator<JSONArray> it = arrays2.iterator();
                     it.hasNext();) {
                    JSONArray a2 = it.next();
                    if (equals(path, a1, a2)) {
                        it.remove();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }

            for (Object v1: others1) {
                boolean found = false;
                for (Iterator<Object> it = others2.iterator(); it.hasNext();) {
                    Object v2 = it.next();
                    if (equalsScalar(v1, v2)) {
                        it.remove();
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }
        } finally {
            path.removeLast();
        }

        assertTrue(objs2.isEmpty());
        assertTrue(arrays2.isEmpty());
        assertTrue(others2.isEmpty());
        return true;
    }

    /**
     * Determine whether the given two objects are identical.
     *
     * <p>
     *   This method is used to compare scalar values in JSON object.
     * </p>
     *
     * @param o1  An {@link Object} to be compared.
     * @param o2  An {@link Object} to be compared.
     * @return  {@code true} only if the given objects are identical.
     */
    private boolean equalsScalar(Object o1, Object o2) {
        if (Objects.equals(o1, o2)) {
            return true;
        }
        if (o1 == null || o2 == null) {
            return false;
        }

        return String.valueOf(o1).equals(String.valueOf(o2));
    }
}
