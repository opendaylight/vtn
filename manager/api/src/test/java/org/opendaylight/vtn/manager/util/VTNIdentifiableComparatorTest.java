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

import org.opendaylight.vtn.manager.PathMap;
import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.VBridge;
import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VInterface;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VTenant;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTerminal;
import org.opendaylight.vtn.manager.VTerminalConfig;
import org.opendaylight.vtn.manager.VlanMap;
import org.opendaylight.vtn.manager.flow.cond.FlowCondition;
import org.opendaylight.vtn.manager.flow.cond.FlowMatch;
import org.opendaylight.vtn.manager.flow.filter.DropFilter;
import org.opendaylight.vtn.manager.flow.filter.FlowFilter;

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
     * Ensure that {@link PathMap} instances can be compared.
     */
    @Test
    public void testPathMap() {
        List<PathMap> list = new ArrayList<>();
        Counter<Integer> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            list.add(new PathMap(index, "cond", i));
            counter.add(index);
        }

        assertEquals(Integer.class, intComparator.getIdentifierType());
        Collections.sort(list, intComparator);

        int prev = Integer.MIN_VALUE;
        for (PathMap pmap: list) {
            int index = pmap.getIndex().intValue();
            assertTrue(prev <= index);
            assertEquals(true, counter.remove(index));
            prev = index;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link PathPolicy} instances can be compared.
     */
    @Test
    public void testPathPolicy() {
        List<PathPolicy> list = new ArrayList<>();
        Counter<Integer> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            list.add(new PathPolicy(index, 1L, null));
            counter.add(index);
        }

        assertEquals(Integer.class, intComparator.getIdentifierType());
        Collections.sort(list, intComparator);

        int prev = Integer.MIN_VALUE;
        for (PathPolicy pp: list) {
            int index = pp.getPolicyId().intValue();
            assertEquals(true, counter.remove(index));
            assertTrue(prev <= index);
            prev = index;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link VBridge} instances can be compared.
     */
    @Test
    public void testVBridge() {
        List<VBridge> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        VBridgeConfig cf = new VBridgeConfig(null);
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String name = "vbr" + index;
            list.add(new VBridge(name, null, 0, cf));
            counter.add(name);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (VBridge vbridge: list) {
            String name = vbridge.getName();
            if (prev != null) {
                assertTrue(prev.compareTo(name) <= 0);
            }
            assertEquals(true, counter.remove(name));
            prev = name;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link VInterface} instances can be compared.
     */
    @Test
    public void testVInterface() {
        List<VInterface> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        VInterfaceConfig cf = new VInterfaceConfig(null, null);
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String name = "if" + index;
            list.add(new VInterface(name, null, null, cf));
            counter.add(name);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (VInterface vif: list) {
            String name = vif.getName();
            if (prev != null) {
                assertTrue(prev.compareTo(name) <= 0);
            }
            assertEquals(true, counter.remove(name));
            prev = name;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link VTenant} instances can be compared.
     */
    @Test
    public void testVTenant() {
        List<VTenant> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        VTenantConfig cf = new VTenantConfig(null);
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String name = "vtn" + index;
            list.add(new VTenant(name, cf));
            counter.add(name);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (VTenant vtn: list) {
            String name = vtn.getName();
            if (prev != null) {
                assertTrue(prev.compareTo(name) <= 0);
            }
            assertEquals(true, counter.remove(name));
            prev = name;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link VTerminal} instances can be compared.
     */
    @Test
    public void testVTerminal() {
        List<VTerminal> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        VTerminalConfig cf = new VTerminalConfig(null);
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String name = "vterm" + index;
            list.add(new VTerminal(name, null, 0, cf));
            counter.add(name);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (VTerminal vterm: list) {
            String name = vterm.getName();
            if (prev != null) {
                assertTrue(prev.compareTo(name) <= 0);
            }
            assertEquals(true, counter.remove(name));
            prev = name;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link VlanMap} instances can be compared.
     */
    @Test
    public void testVlanMap() {
        List<VlanMap> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String id = "map." + index;
            list.add(new VlanMap(id, null, (short)0));
            counter.add(id);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (VlanMap vmap: list) {
            String id = vmap.getId();
            if (prev != null) {
                assertTrue(prev.compareTo(id) <= 0);
            }
            assertEquals(true, counter.remove(id));
            prev = id;
        }
    }

    /**
     * Ensure that {@link FlowCondition} instances can be compared.
     */
    @Test
    public void testFlowCondition() {
        List<FlowCondition> list = new ArrayList<>();
        Counter<String> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            String name = "fcond." + index;
            list.add(new FlowCondition(name, null));
            counter.add(name);
        }

        assertEquals(String.class, stringComparator.getIdentifierType());
        Collections.sort(list, stringComparator);

        String prev = null;
        for (FlowCondition fcond: list) {
            String name = fcond.getName();
            if (prev != null) {
                assertTrue(prev.compareTo(name) <= 0);
            }
            assertEquals(true, counter.remove(name));
            prev = name;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link FlowMatch} instances can be compared.
     */
    @Test
    public void testFlowMatch() {
        List<FlowMatch> list = new ArrayList<>();
        Counter<Integer> counter = new Counter<>();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            list.add(new FlowMatch(index, null, null, null));
            counter.add(index);
        }

        assertEquals(Integer.class, intComparator.getIdentifierType());
        Collections.sort(list, intComparator);

        int prev = Integer.MIN_VALUE;
        for (FlowMatch match: list) {
            int index = match.getIndex().intValue();
            assertTrue(prev <= index);
            assertEquals(true, counter.remove(index));
            prev = index;
        }
        assertEquals(0, counter.getSize());
    }

    /**
     * Ensure that {@link FlowFilter} instances can be compared.
     */
    @Test
    public void testFlowFilter() {
        List<FlowFilter> list = new ArrayList<>();
        Counter<Integer> counter = new Counter<>();
        DropFilter drop = new DropFilter();
        for (int i = 0; i < 100; i++) {
            int index = random.nextInt();
            list.add(new FlowFilter(index, "cond", drop, null));
            counter.add(index);
        }

        assertEquals(Integer.class, intComparator.getIdentifierType());
        Collections.sort(list, intComparator);

        int prev = Integer.MIN_VALUE;
        for (FlowFilter ff: list) {
            int index = ff.getIndex().intValue();
            assertTrue(prev <= index);
            assertEquals(true, counter.remove(index));
            prev = index;
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
