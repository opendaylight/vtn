/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link ObjectPair}.
 */
public class ObjectPairTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        for (String str: createStrings("Test String")) {
            for (Integer i: createIntegers(0, 10)) {
                ObjectPair<String, Integer> pair =
                    new ObjectPair<String, Integer>(str, i);
                assertEquals(str, pair.getLeft());
                assertEquals(i, pair.getRight());
            }
        }
    }

    /**
     * Test case for {@link ObjectPair#equals(Object)} and
     * {@link ObjectPair#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        List<String> strings = createStrings("Test String");
        List<Integer> integers = createIntegers(0, 10);
        for (String str: strings) {
            for (Integer i: integers) {
                ObjectPair<String, Integer> p1 =
                    new ObjectPair<String, Integer>(str, i);
                ObjectPair<String, Integer> p2 =
                    new ObjectPair<String, Integer>(copy(str), copy(i));
                testEquals(set, p1, p2);
            }
        }

        int required = strings.size() * integers.size();
        assertEquals(required, set.size());
    }

    /**
     * Test case for {@link ObjectPair#toString()}.
     */
    @Test
    public void testToString() {
        String prefix = "ObjectPair[";
        String suffix = "]";
        for (String str: createStrings("Test String")) {
            for (Integer i: createIntegers(0, 10)) {
                ObjectPair<String, Integer> pair =
                    new ObjectPair<String, Integer>(str, i);
                String l = "left=" + str;
                String r = "right=" + i;
                String required = joinStrings(prefix, suffix, ",", l, r);
                assertEquals(required, pair.toString());
            }
        }
    }

    /**
     * Ensure that {@link ObjectPair} is serializable.
     */
    @Test
    public void testSerialize() {
        for (String str: createStrings("Test String")) {
            for (Integer i: createIntegers(0, 10)) {
                ObjectPair<String, Integer> pair =
                    new ObjectPair<String, Integer>(str, i);
                serializeTest(pair);
            }
        }
    }
}
