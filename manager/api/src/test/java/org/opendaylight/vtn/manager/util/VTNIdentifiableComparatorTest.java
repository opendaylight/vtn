/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link VTNIdentifiableComparator}.
 */
public class VTNIdentifiableComparatorTest extends TestBase {
    /**
     * A pseudo random number generator.
     */
    private Random  random = new Random();

    /**
     * A comparator for integers.
     */
    private VTNIdentifiableComparator<Integer> intComparator =
        new VTNIdentifiableComparator<Integer>(Integer.class);

    /**
     * A comparator for strings.
     */
    private VTNIdentifiableComparator<String> stringComparator =
        new VTNIdentifiableComparator<String>(String.class);

    /**
     * Describes a test data associated with an identifier.
     *
     * @param <T>  The type of the identifier.
     */
    private static final class TestData<T extends Comparable>
        implements VTNIdentifiable<T> {
        /**
         * The identifier assigned to this instance.
         */
        private final T  identifier;

        /**
         * Construct a new instance.
         *
         * @param ident  The identifier for this instance.
         */
        private TestData(T ident) {
            identifier = ident;
        }

        // VTNIdentifiable

        /**
         * Return the identifier assigned for this instance.
         *
         * @return  The identifier assigned for this instance.
         */
        @Override
        public T getIdentifier() {
            return identifier;
        }
    }

    /**
     * Counter for identifiers.
     *
     * @param <T>  The type of identifier.
     */
    private static final class Counter<T extends Comparable> {
        /**
         * A map that keeps the number of identifiers.
         */
        private final Map<T, Integer>  counter = new HashMap<>();

        /**
         * The number of identifiers.
         */
        private int  size;

        /**
         * Add the given identifier into this instance.
         *
         * @param id  An identifier to be counted.
         */
        private void add(T id) {
            Integer value = counter.get(id);
            int count = (value == null) ? 0 : value.intValue();
            counter.put(id, Integer.valueOf(count + 1));
            size++;
        }

        /**
         * Remove the given identifoer from this instance.
         *
         * @param id  An identifier to be removed.
         * @return  {@code true} only if the given identifier has been actually
         *          removed.
         */
        private boolean remove(T id) {
            Integer value = counter.get(id);
            if (value == null) {
                return false;
            }

            int count = value.intValue();
            if (count == 1) {
                counter.remove(id);
            } else {
                counter.put(id, Integer.valueOf(count - 1));
            }
            size--;

            return true;
        }

        /**
         * Return the number of identifiers in this instance.
         *
         * @return  The number of identifiers.
         */
        private int getSize() {
            return size;
        }
    }

    /**
     * Ensure that instances identified by an integer can be compared.
     */
    @Test
    public void testInteger() {
        List<TestData<Integer>> list = new ArrayList<>();
        Counter<Integer> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            Integer index = Integer.valueOf(random.nextInt());
            list.add(new TestData<Integer>(index));
            counter.add(index);
        }

        assertEquals(Integer.class, intComparator.getIdentifierType());
        Collections.sort(list, intComparator);

        int prev = Integer.MIN_VALUE;
        for (TestData<Integer> data: list) {
            Integer index = data.getIdentifier();
            int idx = index.intValue();
            assertTrue(prev <= idx);
            assertEquals(true, counter.remove(index));
            prev = idx;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that instances identified by a string can be compared.
     */
    @Test
    public void testString() {
        List<TestData<String>> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String name = "id_" + index;
            list.add(new TestData<String>(name));
            counter.add(name);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (TestData<String> data: list) {
            String name = data.getIdentifier();
            if (prev != null) {
                assertTrue(prev.compareTo(name) <= 0);
            }
            assertEquals(true, counter.remove(name));
            prev = name;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Test case for {@link VTNIdentifiableComparator#equals(Object)} and
     * {@link VTNIdentifiableComparator#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<>();
        VTNIdentifiableComparator<?> comp1 =
            new VTNIdentifiableComparator<Long>(Long.class);
        VTNIdentifiableComparator<?> comp2 =
            new VTNIdentifiableComparator<Long>(Long.class);
        testEquals(set, comp1, comp2);

        comp1 = new VTNIdentifiableComparator<Integer>(Integer.class);
        comp2 = new VTNIdentifiableComparator<Integer>(Integer.class);
        testEquals(set, comp1, comp2);

        comp1 = new VTNIdentifiableComparator<Short>(Short.class);
        comp2 = new VTNIdentifiableComparator<Short>(Short.class);
        testEquals(set, comp1, comp2);

        comp1 = new VTNIdentifiableComparator<Byte>(Byte.class);
        comp2 = new VTNIdentifiableComparator<Byte>(Byte.class);
        testEquals(set, comp1, comp2);

        comp1 = new VTNIdentifiableComparator<String>(String.class);
        comp2 = new VTNIdentifiableComparator<String>(String.class);
        testEquals(set, comp1, comp2);

        comp1 = new VTNIdentifiableComparator<BigInteger>(BigInteger.class);
        comp2 = new VTNIdentifiableComparator<BigInteger>(BigInteger.class);
        testEquals(set, comp1, comp2);

        assertEquals(false, set.add(intComparator));
        assertEquals(false, set.add(stringComparator));
        assertEquals(true, set.contains(intComparator));
        assertEquals(true, set.contains(stringComparator));

        assertEquals(6, set.size());

        assertEquals(true, set.remove(intComparator));
        assertEquals(false, set.remove(intComparator));
        assertEquals(5, set.size());

        assertEquals(true, set.remove(stringComparator));
        assertEquals(false, set.remove(stringComparator));
        assertEquals(4, set.size());
    }

    /**
     * Ensure that {@link VTNIdentifiableComparator} is serializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        List<VTNIdentifiableComparator<?>> list = new ArrayList<>();
        list.add(new VTNIdentifiableComparator<Long>(Long.class));
        list.add(new VTNIdentifiableComparator<Integer>(Integer.class));
        list.add(new VTNIdentifiableComparator<Short>(Short.class));
        list.add(new VTNIdentifiableComparator<Byte>(Byte.class));
        list.add(new VTNIdentifiableComparator<String>(String.class));
        list.add(new VTNIdentifiableComparator<BigInteger>(BigInteger.class));

        for (VTNIdentifiableComparator<?> o: list) {
            VTNIdentifiableComparator<?> o1 =
                serializeTest(o, VTNIdentifiableComparator.class);
            assertEquals(o.getIdentifierType(), o1.getIdentifierType());
        }
    }
}
